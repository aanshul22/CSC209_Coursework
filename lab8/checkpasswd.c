#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAXLINE 256
#define MAX_PASSWORD 10

#define SUCCESS "Password verified\n"
#define INVALID "Invalid password\n"
#define NO_USER "No such user\n"

int main(void) {
	char user_id[MAXLINE];
	char password[MAXLINE];

	/* The user will type in a user name on one line followed by a password 
       on the next.
       DO NOT add any prompts.  The only output of this program will be one 
	   of the messages defined above.
       Please read the comments in validate carefully.
	*/

    if(fgets(user_id, MAXLINE, stdin) == NULL) {
    	perror("fgets");
    	exit(1);
	}
	if(fgets(password, MAXLINE, stdin) == NULL) {
		perror("fgets");
    	exit(1);
	}

	user_id[sizeof(user_id) - 1] = '\0';
	password[sizeof(password) - 1] = '\0';

	if (strlen(user_id) > MAX_PASSWORD) {
		fprintf(stderr, "Size of user id too big!\n");
		// exit(1);
	}

	if (strlen(password) > MAX_PASSWORD) {
		fprintf(stderr, "Size of password too big!\n");
		// exit(1);
	}

	int fd[2];

	pipe(fd);

	int r = fork();

	if (r == -1) {
		perror("fork");
	}
	else if (r == 0) // If child
	{
		close(fd[1]);

		if (dup2(fd[0], STDIN_FILENO) == -1) {
			perror("dup2");
		}
		close(fd[0]);

		execl("./validate", "validate", NULL);
		perror("exec");
		exit(1);
	}
	else			// If parent
	{
		close(fd[0]);

		if (write(fd[1], user_id, MAX_PASSWORD) == -1)
		{
			perror("Write to pipe");
		}
		if (write(fd[1], password, MAX_PASSWORD) == -1)
		{
			perror("Write to pipe");
		}

		close(fd[1]);

		int status;
		if (wait(&status) != -1) {
			int exit_status = 1;
			if (WIFEXITED(status)) {
				exit_status = WEXITSTATUS(status);
			}

			if (exit_status == 0)
			{
				printf(SUCCESS);
			}
			else if (exit_status == 3) 
			{
				printf(NO_USER);
			}
			else if (exit_status == 2)
			{
				printf(INVALID);
			}
			else 
			{
				exit(1);
			}
		}
	}
	
	return 0;
}
