#include <stdio.h>
#include <stdlib.h>
inline void prt(int i) {
printf("The power of %d is %d\n", i, i*i); 
}
void main(){
int max_num = 1000;
for (int i = 0; i < max_num; i++)
prt(i); 

}
