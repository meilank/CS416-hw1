#include "types.h"
#include "stat.h"
#include "user.h"
#include "signal.h"

int numTraps = 50000;	//number of traps we want to run before skipping the instruction

void handle_signal(siginfo_t signum)
{
	static int count = 0;
	count++;

	if (count == numTraps)
	{
		uint *ret = (uint*) (&signum + 4);
		*ret += 4;
	}

}

int main(int argc, char *argv[])
{
	signal(SIGFPE, handle_signal);
	
	int starting = uptime();

	int x = 5;
	int y = 0;

	x = x / y;

	int finished = uptime();
	double elapsed = finished-starting;
	double avgTime = (elapsed/numTraps) * 1000000;

	printf(1, "Traps Performed: %d\n", numTraps);
	printf(1, "Total Elapsed Time: %d ticks\n", (int) elapsed);
	printf(1, "Average Time Per Trap: %d microticks\n", (int) avgTime);

	exit();
}