#include <iostream>

#ifdef WIN32
#include <windows.h>
#include <process.h>
#else
#include <pthread.h>
#endif

#include "mysql.h"
#include "MTConnMgr.h"
#include "Common.h"
#include "MtMutex.h"

using namespace std;

MTConnMgr connectionMgr;

void doSomething()
{
     MYSQL* connection = connectionMgr.getConnection();

	 cout<<(unsigned int)connection<<endl;

	 int retCode = connectionMgr.execQuery(connection, " select * from rates");

	 MYSQL_RES *res  = NULL;
	 MYSQL_ROW row = NULL;

	 cout<<"reuslt"<<retCode<<":";

	 if(retCode == 0)
	 {
		 res = mysql_store_result(connection);
		 row = NULL;

		while( (row = mysql_fetch_row(res)) != NULL )
		{
			cout<<"rates is :"<< row[0] <<row[1]<<endl;
		}

		mysql_free_result(res);
	 }
}

MtMutex g_mutex;
int g_foo =10;

#ifdef WIN32
void
#else
void*
#endif
 thread_function(void* args)
{
	//connectionMgr.initConnection();

	//MTAutoConnMgr autoConnMgr(&connectionMgr);
    //doSomething();

	//connectionMgr.closeConnection();

	

	synchronized(g_mutex)
	{
		printf("%d \n",++g_foo);
		printf("%d \n",++g_foo);
		printf("%d \n",++g_foo);
		printf("%d \n",++g_foo);
		printf("%d \n",++g_foo);
	}

#ifndef WIN32
	return NULL;
#endif

}

#ifdef WIN32
void
#else
void*
#endif
 thread_function2(void* args)
{
	synchronized(g_mutex)
	{
		printf("%d \n",--g_foo);
		printf("%d \n",--g_foo);
		printf("%d \n",--g_foo);
		printf("%d \n",--g_foo);
		printf("%d \n",--g_foo);
	}
#ifndef WIN32
	return NULL;
#endif
}


int main()
{


#ifdef WIN32
	for (int i = 0; i < 1; ++i)
	{
		_beginthread(thread_function, 0, NULL);
		_beginthread(thread_function2, 0, NULL);
		//Sleep(10);
	}

	Sleep(100000);
#else
	pthread_t threads[5];

	for (int i = 0; i < 5; ++i)
	{
		pthread_create (&(threads[i]), NULL, thread_function, NULL);
	}
	 sleep(100);
#endif

    return 0;
}
