#include "types.h"
#include "stat.h"
#include "user.h"
#include "signal.h"

static int numFPE;
int timeTaken;
int y;
int x;

void handle_signal(siginfo_t info)
{
	numFPE++;
	if (numFPE== 8)
	{
		printf(1, "here!\n");
		y= 1;
	}
}

int main(int argc, char *argv[])
{
	signal(SIGFPE, handle_signal);

	x= 5;
	y= 0;
	numFPE= 0;

	timeTaken= uptime();

	while (y == 0)
		x= x/y;

	printf(1, "Traps Performed: %d\n", numFPE);
	printf(1, "Total Elapsed Time: %dms\n", timeTaken);
	printf(1, "Average Time Per Trap: %dms\n", timeTaken/numFPE);

	exit();
}