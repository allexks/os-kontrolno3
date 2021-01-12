/* find .. -type f -printf '%p\t%T@\n' | sort -k2 -n -r | cut -f1 | head -n1 */

#include <err.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int main(const int argc, char const * argv[]) {
	if (argc != 2)
		errx(1, "Usage: $ %s <directory_name>\n", argv[0]);
	
	int fd1[2];
	int fd2[2];
	int fd3[2];

	if (pipe(fd1) == -1)
		err(1, "pipe 1 could not be made");
	if (pipe(fd2) == -1)
		err(1, "pipe 2 could not be made");
	if (pipe(fd3) == -1)
		err(1, "pipe 3 could not be made");

	pid_t find_pid = fork();
	if (find_pid < 0) {
		err(1, "could not fork find process");
	} else if (find_pid == 0) {
		close(fd1[0]);

		if (dup2(fd1[1], 1) == -1)
			err(1, "dup2 writefd for pipe 1 failed");

		execlp("find", "find", argv[1], "-type", "f", "-printf", "%p\t%T@\n", NULL);
		err(1, "could not execute find command");
	}

	close(fd1[1]);
	pid_t sort_pid = fork();
	if (sort_pid < 0) {
		err(1, "could not fork sort process");
	} else if (sort_pid == 0) {
		close(fd2[0]);

		if (dup2(fd1[0], 0) == -1)
			err(1, "dup2 readfd for pipe 2 failed");

		if (dup2(fd2[1], 1) == -1)
			err(1, "dup2 writefd for pipe 2 failed");

		execlp("sort", "sort", "-k2", "-n", "-r", NULL);
		err(1, "could not execute sort command");
	}

	close(fd2[1]);
	pid_t cut_pid = fork();
	if (cut_pid < 0) {
		err(1, "could not fork cut process");
	} else if (cut_pid == 0) {
		close(fd3[0]);

		if (dup2(fd2[0], 0) == -1)
			err(1, "dup2 readfd for pipe 3 failed");

		if (dup2(fd3[1], 1) == -1)
			err(1, "dup2 writefd fo pipe 3 failed");

		execlp("cut", "cut", "-f1", NULL);
		err(1, "could not execute cut command");
	}

	close(fd3[1]);
	if (dup2(fd3[0], 0) == -1)
		err(1, "dup2 readfd for head failed");

	execlp("head", "head", "-n1", NULL);
	err(1, "could not execute head command");
}
