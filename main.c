#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(void)
{
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    while((read = getline(&line, &len, stdin)) != -1)
    {
        pid_t cpid = fork();
        if (cpid == -1)
        {
            perror("Failed to fork. Exit.");
            exit(EXIT_FAILURE);
        }

        if (cpid == 0)
        {
            printf("Child PID is %ld\n", (long)getpid());
            printf("..should execute: %s", line);
            exit(EXIT_SUCCESS);
        }
        else
        {
            int status;
            do
            {
                pid_t w = waitpid(cpid, &status, 0);

                if (w == -1)
                {
                    perror("Failed to wait for pid status change.");
                    exit(EXIT_FAILURE);
                }

                if (WIFEXITED(status))
                {
                    printf("exited, status=%d\n", WEXITSTATUS(status));
                }
                else if (WIFSIGNALED(status))
                {
                    printf("killed by signal %d\n", WTERMSIG(status));
                }
                else if (WIFSTOPPED(status))
                {
                    printf("stopped by signal %d\n", WSTOPSIG(status));
                }
                else if (WIFCONTINUED(status))
                {
                    printf("continued\n");
                }
            }
            while(!WIFEXITED(status) && !WIFSIGNALED(status));
        }
    }
}
