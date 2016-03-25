#include "types.h"
#include "stat.h"
#include "user.h"
#include "signal.h"

//int x = 5;
//int y = 0;

void handle_signal(siginfo_t signum)
{
	static int count = 0;
	count++;

	printf(1, "infinite?\n");

}

int main(int argc, char *argv[])
{
	signal(SIGFPE, handle_signal);
	
	int x = 5;
	int y = 0;

	x = x / y;

	printf(1, "Traps Performed: XXXX\n");
	printf(1, "Total Elapsed Time: XXXX\n");
	printf(1, "Average Time Per Trap: XXXXX\n");

	exit();
}