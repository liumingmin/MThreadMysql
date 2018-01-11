#ifndef _MT_Conn_Mgr_
#define _MT_Conn_Mgr_

#include <string>

typedef struct st_mysql MYSQL;
typedef unsigned long DWORD;

#ifndef WIN32
typedef unsigned int pthread_key_t;
#endif

class MTConnMgr
{
public:
	MTConnMgr();
	~MTConnMgr();

public:
	bool initConnection();
	MYSQL* getConnection();
	void closeConnection();
	int execQuery(MYSQL*& connection, const char* sql );

protected:
	int getDbPassword(std::string& dbPassword);

private:

#ifdef WIN32
	DWORD m_dwTlsIndex;
#else
	pthread_key_t m_threadKey;
#endif

};

class MTAutoConnMgr
{
public:
	MTAutoConnMgr(MTConnMgr* pMTConnMgr, bool& result)
	{
		m_pMTConnMgr = pMTConnMgr;
		result = m_pMTConnMgr->initConnection();
	};

	~MTAutoConnMgr()
	{
		m_pMTConnMgr->closeConnection();
	};
private:
	MTConnMgr* m_pMTConnMgr;
};

#endif