// 1.cpp : Defines the entry point for the console application.
//

#include <stdio.h> 
#include <stdlib.h> 


char* getmemory() 
{ char *p=(char *) malloc(100); strcpy(p,"hello world"); return &p; 
} 

int main(int argc, char* argv[])
{
	char *str=NULL; str = getmemory( ); printf("%s/n",str); free(str); return 0; 
}

