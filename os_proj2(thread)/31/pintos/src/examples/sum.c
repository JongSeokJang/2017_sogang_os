#include <stdio.h>
#include <syscall.h>
#include <stdlib.h>
#include "../lib/user/syscall.h"

int main(int argc, char **argv)
{
    int a,b,c,d,sum,fibo;

    a = atoi(argv[1]); 
	b = atoi(argv[2]); 
	c = atoi(argv[3]); 
	d = atoi(argv[4]);

    fibo = fibonacci(a);
    sum = sum_of_four_integers(a,b,c,d);

    printf("%d %d\n",fibo,sum);

    return 1;
}
