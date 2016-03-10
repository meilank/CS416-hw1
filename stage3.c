#include "types.h"
#include "stat.h"
#include "user.h"
#include "signal.h"

static int numFPE;
int timeTaken;
int y;
int x;
int cont= 1;

void handle_signal(siginfo_t info)
{
	numFPE++;
	if (numFPE==5)
	{
		cont= 0;
	}
}

int main(int argc, char *argv[])
{
	signal(SIGFPE, handle_signal);

	x= 1;
	y=0;
	numFPE= 0;

	timeTaken= uptime();

	while (cont)
		x= x/y;

	printf(1, "Traps Performed: %d\n", numFPE);
	printf(1, "Total Elapsed Time: %dms\n", timeTaken);
	printf(1, "Average Time Per Trap: %dms\n", timeTaken/numFPE);

	exit();
}