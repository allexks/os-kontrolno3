#include <err.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>
#include <string.h> //strlen
#include <errno.h>

int main (const int argc, char const * argv[]) {
	if (argc < 3)
		errx(1, "Usage: $ %s <THRESHOLD> <command> [<params>...]", argv[0]);

	const int THRESHOLD = atoi(argv[1]);
	if (THRESHOLD < 1 || THRESHOLD > 9)
		errx(1, "Parameter THRESHOLD should be a number from 1 to 9 inclusively");

	int fd = open("run.log", O_CREAT | O_WRONLY | O_APPEND, 0644);
	if (fd == -1)
		err(1, "could not open log file for writing");
	
	bool flag = false;

	while (true) {
		time_t start = time(NULL);

		pid_t pid = fork();
		if (pid < 0)
			err(1, "could not fork process");
		else if (pid == 0) {
			execvp(argv[2], (char * const *) &argv[2]);
			exit(errno);
		}

		int wstatus;
		wait(&wstatus);
	
		time_t end = time(NULL);
		
		int exitcode = WIFEXITED(wstatus) ? WEXITSTATUS(wstatus) : 129;
		//dprintf(fd, "%ld %ld %d\n", start, end, exitcode);
		char buf[27]; // "<epochtime> <epochtime> <errorcode>\n"
		snprintf(buf, 27, "%ld %ld %d\n", (unsigned long int)start, (unsigned long int)end, exitcode);
		if (write(fd, buf, strlen(buf)) == -1)
			err(1, "error while writing to log file");


		bool condition = exitcode != 0 && end - start < THRESHOLD;
		if (flag && condition)
			break;
		else if (condition)
			flag = true;
		else
			flag = false;
	}
	close(fd);
	exit(0);
}
