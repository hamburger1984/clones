#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void prompt(char*);
void handle_child(pid_t);
void become_child(char*);

int main(void)
{
    char *line = NULL;
    size_t len = 0;
    ssize_t read = 0;
    while(read != -1)
    {
        prompt("%a, %d. %b %H:%M:%S > ");
        read = getline(&line, &len, stdin);
        if(read < 2)
            continue;

        line[read-1] = '\0';

        pid_t cpid = fork();
        if (cpid == -1)
        {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        if (cpid == 0)
        {
            become_child(line);
        }
        else
        {
            handle_child(cpid);
        }
    }
    exit(EXIT_SUCCESS);
}

void prompt(char* prompt_format)
{
    time_t now;
    struct tm *tmp;
    now = time(NULL);
    tmp = localtime(&now);
    if(tmp==NULL)
    {
        perror("localtime");
        exit(EXIT_FAILURE);
    }

    char buffer[200];
    if (strftime(buffer, sizeof(buffer), prompt_format, tmp)==0){
        perror("strftime");
        exit(EXIT_FAILURE);
    }

    printf("%s", buffer);

}

void become_child(char* to_execute)
{
    printf("Child PID is %ld\n", (long)getpid());
    printf("..should execute: '%s'\n", to_execute);
    exit(EXIT_SUCCESS);
}

void handle_child(pid_t cpid)
{
    int status;
    do
    {
        pid_t w = waitpid(cpid, &status, 0);

        if (w == -1)
        {
            perror("waitpid");
            exit(EXIT_FAILURE);
        }

        if (WIFEXITED(status))
        {
            printf("> exited, status=%d\n", WEXITSTATUS(status));
        }
        else if (WIFSIGNALED(status))
        {
            printf("> killed by signal %d\n", WTERMSIG(status));
        }
        else if (WIFSTOPPED(status))
        {
            printf("> stopped by signal %d\n", WSTOPSIG(status));
        }
        else if (WIFCONTINUED(status))
        {
            printf("> continued\n");
        }
    }
    while(!WIFEXITED(status) && !WIFSIGNALED(status));
}
