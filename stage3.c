#include "types.h"
#include "stat.h"
#include "user.h"
#include "signal.h"

static int numFPE;
int y= 0;

void handle_signal(siginfo_t info)
{
	while (numFPE < 5)
	{
		numFPE++;
	}
	y= 100;
	exit();
}

int main(int argc, char *argv[])
{
	signal(SIGFPE, handle_signal);
	
	int x= 1;
	numFPE= 0;

	y= x/y;

	int timeTaken= uptime();

	printf(1, "Traps Performed: %d\n", numFPE);
	printf(1, "Total Elapsed Time: %d\n", timeTaken);
	printf(1, "Average Time Per Trap: %d\n", timeTaken/numFPE);

	exit();
}