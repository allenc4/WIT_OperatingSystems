#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define TRUE 1

int main()
{
	struct timeval tt;

	while (TRUE)
	{
		(void) gettimeofday(&tt, NULL);
		srand(tt.tv_sec * 1000000 + tt.tv_usec);
		long rand_num = ((rand() % 10) + 5) * 100000;

		printf("%lu\n", rand_num);
		usleep(rand_num);
	}

	return 0;
}
