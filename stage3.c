#include "types.h"
#include "stat.h"
#include "user.h"
#include "signal.h"

int timeTaken;
int y;
int x;

void handle_signal(siginfo_t info)
{
	static int numFPE= 0;
	numFPE++;
	if (numFPE== 1000)
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

	timeTaken= uptime();

	x= x/y;

	printf(1, "Traps Performed: 1000\n");
	printf(1, "Total Elapsed Time: %dticks\n", timeTaken);
	printf(1, "Average Time Per Trap: %dticks/trap\n", timeTaken/1000);

	exit();
}