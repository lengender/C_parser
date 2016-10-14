#include<stdio.h>

int fibonacci(int i)
{
    if(i <= 1)
	{
		return 1;
	}
	
	return fibonacci(i - 1) + fibonacci(i - 2);
}
int main()
{
    int i, j;
	i = 10;
	printf("hello world, i = %d\n". i);

	while(i >= 5)
	{
		printf("fibonacci(%2d) = %d\n", i, fibonacci(i));
		i--;
	}


	return 0;
}
