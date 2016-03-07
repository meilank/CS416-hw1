#include "types.h"
#include "stat.h"
#include "user.h"
#include "signal.h"

static int numFPE;

void handle_signal(siginfo_t info)
{
	numFPE++;
	printf(1, "Here!\n");
	exit();
}

int main(int argc, char *argv[])
{
	signal(SIGFPE, handle_signal);
	
	int x= 1;
	int y=0;
	int timeTaken= uptime();
	numFPE= 0;
	while (numFPE< 5)
	{
		y= x/y;
		y=0;
	}

	printf(1, "Traps Performed: %d\n", numFPE);
	printf(1, "Total Elapsed Time: %d\n", timeTaken);
	printf(1, "Average Time Per Trap: %d\n", timeTaken/numFPE);

	exit();
}