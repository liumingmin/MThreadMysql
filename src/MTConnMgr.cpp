

#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#include <process.h>
#else
#include <pthread.h>
#include <dlfcn.h>
#endif

#include "mysql.h"
#include "SimpleIni.h"
#include "Common.h"
#include "MTConnMgr.h"


//**************************************************************************************************
MTConnMgr::MTConnMgr()
{

#ifdef WIN32
	LogText(SYS_INFO3, "MTConnMgr::MTConnMgr: TlsAlloc to m_dwTlsIndex");

	if((m_dwTlsIndex = TlsAlloc()) == TLS_OUT_OF_INDEXES)
	{
		LogText(SYS_ERROR, "MTConnMgr::MTConnMgr:Error in TlsAlloc()");
	}
#else
	LogText(SYS_INFO3, "MTConnMgr::MTConnMgr: pthread_key_create to m_threadKey");

	pthread_key_create (&m_threadKey, NULL);
#endif

}

MTConnMgr::~MTConnMgr()
{

#ifdef WIN32
	LogText(SYS_INFO3, "MTConnMgr::~MTConnMgr: TlsFree  m_dwTlsIndex");

	TlsFree(m_dwTlsIndex);
#endif

}

//**************************************************************************************************

bool MTConnMgr::initConnection()
{

	MYSQL* connection = new MYSQL();

	if(mysql_init(connection) == NULL)
	{
		mysql_close(connection);
		return false;//=====>>>>
	}
	
	//读取数据库配置
	CSimpleIni tIni;
	if (SI_OK != tIni.LoadFile("./config.ini"))
	{
		LogText(SYS_ERROR, "fail to load config.ini...\n");
	}

	const char* dbIp = tIni.GetValue("DATABASE", "dbIp", "127.0.0.1");
	int dbPort = tIni.GetLongValue("DATABASE", "dbPort", 30001);
	const char* dbUser = tIni.GetValue("DATABASE", "dbUser", "root");
	string dbPassword = tIni.GetValue("DATABASE", "dbPassword", "123456abcdef");
	const char* dbName = tIni.GetValue("DATABASE", "dbName", "bms");

	int nRet = getDbPassword(dbPassword);
	if (nRet < 0)
	{
		LogText(SYS_ERROR, "MTConnMgr::initConnection:Error in GetDbPassword():%d", nRet);
		mysql_close(connection);
		return false;//======>>>>>
	}
	
	//数据库配置输出
	LogText(SYS_INFO3, "MTConnMgr::initConnection:mysql_real_connect: host:%s, port:%d , username:%s, password:%s, dbname:%s ",
		dbIp, dbPort,dbUser , dbPassword.c_str() , dbName);

	//设置字符集
	mysql_options(connection, MYSQL_SET_CHARSET_NAME, "utf8");
	
	//连接数据库
	if( mysql_real_connect(connection, dbIp, dbUser, dbPassword.c_str(), dbName, dbPort, NULL, 0) == NULL)
	{
		LogText(SYS_ERROR, "MTConnMgr::initConnection:mysql_real_connect failed!");
		mysql_close(connection);
		return false;//=======>>>>
	}

	//设置到线程本地变量中
#ifdef WIN32
	LogText(SYS_INFO3, "MTConnMgr::initConnection:TlsSetValue Set Connection %d To Thread %d 's ThreadLocal !",(unsigned int)connection,(unsigned int)GetCurrentThreadId());

	if (!TlsSetValue(m_dwTlsIndex, connection))
	{
		LogText(SYS_ERROR, "MTConnMgr::initConnection TlsSetValue Set Connection To ThreadLocal Failed !");
		mysql_close(connection);
		return false;//=======>>>>
	}

#else
	LogText(SYS_INFO3, "MTConnMgr::initConnection:pthread_setspecific Set Connection %d  To Thread %d 's ThreadLocal !",(unsigned int)connection,(unsigned int)pthread_self());

	pthread_setspecific(m_threadKey,connection);

#endif

	return true;
}


MYSQL* MTConnMgr::getConnection()
{

#ifdef WIN32
	LPVOID  args = TlsGetValue(m_dwTlsIndex);
#else
	void* args = pthread_getspecific(m_threadKey);
#endif
	MYSQL* connection = (MYSQL*)args;

	return connection;
}


void MTConnMgr::closeConnection()
{

#ifdef WIN32
	LPVOID  args = TlsGetValue(m_dwTlsIndex);
#else
	void* args = pthread_getspecific(m_threadKey);
#endif

	MYSQL* connection = (MYSQL*)args;

#ifdef WIN32
	LogText(SYS_INFO3, "MTConnMgr::closeConnection: Close Connection %d From Thread %d 's ThreadLocal !",(unsigned int)connection,(unsigned int)GetCurrentThreadId());
#else
	LogText(SYS_INFO3, "MTConnMgr::closeConnection: Close Connection %d  From Thread %d 's ThreadLocal !",(unsigned int)connection,(unsigned int)pthread_self());
#endif

	if(connection != NULL)
	{
		mysql_close((MYSQL*)connection);
	}

}

int MTConnMgr::execQuery(MYSQL*& connection, const char* sql )
{
	int retCode = mysql_query(connection,sql);

	//执行失败
	if(retCode != 0)
	{
		int errcode = mysql_errno(connection);
		if(errcode == 2006 || errcode == 2013)
		{
			LogText(SYS_INFO3, "MTConnMgr::execQuery: Lost Connect ,Reconnect  !");

			mysql_close(connection);
			initConnection();

			connection = getConnection();

			return mysql_query(connection,sql);
		}
	}

	return retCode;
}


//**********************************************************************************************************
int MTConnMgr::getDbPassword(std::string& dbPassword)
{
	int nRst = 0;

#ifdef WIN32
	// WINDOWS下使用配置文件的密码，不做操作
#else
	// LINUX下的数据库密码获取
	// 不用编译链接的方式调用动态库，保证编译出来的程序能在安全和非安全版本的NTX下使用
	void * pLibHandle = NULL;
	typedef int (*PFN_GETPSD)(char *, int);
	PFN_GETPSD pfnGetPassword;

	// NTX不是安全的版本
	pLibHandle = dlopen("libntxsecurity.so", RTLD_LAZY);
	if (pLibHandle == NULL)
	{
		LogText(SYS_ERROR, "MTConnMgr::GetDbPassword:Can not find libntxsecurity.so");
		return -1;
	}

	// 获取函数指针
	pfnGetPassword = (PFN_GETPSD)dlsym(pLibHandle, "NTXSECURITY_GetDbPasswd");
	if (pfnGetPassword == NULL)
	{
		// 出错了
		LogText(SYS_ERROR, "MTConnMgr::GetDbPassword:Can not find NTXSECURITY_GetDbPasswd");
		dlclose(pLibHandle);
		return -2;
	}

	// 获取密码
	char dbPasswordStr[128];
	nRst = pfnGetPassword(dbPasswordStr, sizeof(dbPasswordStr));

	dlclose(pLibHandle);

	dbPassword = dbPasswordStr;

#endif

	return nRst;
}

