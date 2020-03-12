#include <stdio.h>
#include <process.h>
#include <windows.h>


int count=0;

void thread1(void* pvoid)
{
	while(count<1000000000)
	{
       count++;
	}
}

void thread2(void* pvoid)
{
	while(count>-1000000000)
	{
       count--;
	}
}

int main()
{
	
	_beginthread(thread1,0,NULL);
	_beginthread(thread2,0,NULL);

	_sleep(100);
	printf("%d\n",count);

	return 0;
}
