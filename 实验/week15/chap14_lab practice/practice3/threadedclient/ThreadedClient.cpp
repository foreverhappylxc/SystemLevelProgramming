/*
	ThreadedClient.cpp

	A threaded database client.
 */

#include <iostream>

#include "servers.h"

#ifdef _MSC_VER
#include <windows.h>
#include <winbase.h>
#include <process.h>
typedef __int64 INT64_T;
typedef unsigned __int64 UINT64_T;

typedef LONGLONG time_ms_t;

time_ms_t getTimeInMilliseconds() {
	SYSTEMTIME stime;
	
	GetSystemTime( &stime );

	FILETIME ftime;
	LARGE_INTEGER time;

	SystemTimeToFileTime( &stime, &ftime );	/* if this fails... */

	time.HighPart = ftime.dwHighDateTime;
	time.LowPart = ftime.dwLowDateTime;

	/* FileTime is in 100ns intervals since 1/1/1601 */
	return time.QuadPart / 10000;
}	
#endif

/*
	ostream &operator <<( ostream &out, string *str ) {

	Send a string to an output stream.
 */
ostream &operator<<(ostream &out, string *str) {
	if (str)
		return out << str->data();
	else
		return out;
}

/*
    ostream & operator<< ( ostream &out, INT64_T num )

	Print a 64-bit unsigned integer to the given output stream.
  */
ostream &operator<<( ostream &out, INT64_T snum ) {
#define _OSTR_LL_LEN 21
	if (snum) {
		char buffer[_OSTR_LL_LEN];
		int i = _OSTR_LL_LEN - 1;
        UINT64_T num;
		if (snum < 0) num = -snum; else num = snum;

		buffer[i] = '\0';
		while (num) {
			buffer[--i] = ('0' + (int) (num % 10));
			num /= 10;
		}
		if (snum < 0) buffer[--i] = '-';
		return out << buffer + i;
	}else return out << '0';
}

/*
	int main( int argc, char *argv[]

	You should modify this function to use threads.
 */
int account;
Personal *pers;
AccountInfo *acct;

//获取账户名
void getName(void *pName) {
	pers = GetPersonalInformation( account );
	_endthread();
}

//获取账户信息
void getAccount(void *pAccount) {
	acct = GetAccountInformation( account );
	_endthread();
}

int main( int argc, char *argv[] ) {
	if (argc != 2) {
		cerr << "usage: " << argv[0] << " [account_number]" << endl;
		exit(1);
	}
	account = atoi( argv[1] );
	
	time_ms_t start = getTimeInMilliseconds();
	cout << "Retrieving...";
	cout.flush();

	HANDLE handles[2];
	//开创两个线程用于实现读取姓名和账户信息
	long thread1;
	long thread2;
	thread1 = _beginthread(getName,0,NULL);
	handles[0] = (void *)thread1;
	thread2 = _beginthread(getAccount,0,NULL);
	handles[1] = (void *)thread2;

	WaitForMultipleObjects(2,handles,true,INFINITE);


	time_ms_t end = getTimeInMilliseconds();
	cout << "done (" << end - start << "ms)" << endl;

	if (pers) {
		cout << account << ": " << pers->FirstName << " "
			<< pers->LastName << endl;
		cout << pers->Address << endl;
		cout << "Balance: " << acct->Balance << ", " << acct->Pending
			<< " pending, " << acct->Share << " share" << endl;

		delete pers;
		delete acct;
	}else cout << "No client matches that account number" << endl;
	return 0;
}
