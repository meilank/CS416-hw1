#include "types.h"
#include "stat.h"
#include "user.h"
#include "signal.h"

int numFPE;

void handle_signal(siginfo_t info)
{
	numFPE++;
	//printf(1, "\n");
	//__asm__ ("movl $0x0,%ebx\n\t");
	if (numFPE== 1000)
	{
		printf(1, "here!\n");
		__asm__ 
		(
			"mov $1, %ebx\n\t"
			"ret\n\t"
		);
	}
	printf(1, "here\n");
}

int main(int argc, char *argv[])
{
	signal(SIGFPE, handle_signal);
	register int ebx asm ("%ebx");

	ebx= 0;

	int x= 1;
	numFPE= 0;

	int timeTaken= uptime();
	x= x/ebx;

	printf(1, "%d\n", ebx);

	printf(1, "Traps Performed: %d\n", numFPE);
	printf(1, "Total Elapsed Time: %dms\n", timeTaken);
	printf(1, "Average Time Per Trap: %dms\n", timeTaken/numFPE);

	exit();
}