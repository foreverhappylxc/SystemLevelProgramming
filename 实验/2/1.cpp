#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>
 int NegativeNum(int x)  
{  
	return ~((~x + 1) & (~0x80000000)); 
} 

int main( ) 
{ 
printf("%d\n",NegativeNum(5));  
return 0; 
}


