#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

void prompt(const char*);
const unsigned long hash(const char*);
bool is_builtin(unsigned long);
void handle_builtin(unsigned long, const char*);
void handle_child(pid_t);
void become_child(const char*);

#define PWD 193502992
#define CD 5863276
#define EXIT 6385204799

int main(int argc, char* argv[])
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

        unsigned long hash_value = hash(line);
        printf(" -- %s -- hash: %ld\n", line, hash_value);
        if (is_builtin(hash_value)){
            handle_builtin(hash_value, line);
            continue;
        }

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

void prompt(const char* prompt_format)
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
    if (strftime(buffer, sizeof(buffer), prompt_format, tmp)==0)
    {
        perror("strftime");
        exit(EXIT_FAILURE);
    }

    printf("%s", buffer);

}

bool is_builtin(unsigned long hash_value)
{
    switch(hash_value){
        case PWD:
        case CD:
        case EXIT:
            return true;
        default:
            return false;
    }
}

void handle_builtin(unsigned long hash_value, const char* value){

    switch(hash_value){
        case EXIT:
            printf("..done.\n");
            exit(EXIT_SUCCESS);
            break;
        default:
            printf("handle_builtin (%s).\n", value);
            break;
    }

}

// reused from: http://stackoverflow.com/a/37121071
const unsigned long hash(const char* value)
{
    unsigned long hash = 5381;
    int c;

    while ((c = *value++))
        hash = ((hash << 5) + hash ) + c;

    return hash;
}

void become_child(const char* to_execute)
{
    printf("Child PID is %ld\n", (long)getpid());
    printf("..should execute: '%s'\n", to_execute);

    /* TODO:
     *  parse to_execute -> parts
     *    1. dump (split by space)
     *    2. better (escapes, quotes, ..)
     *  check for builtin via hash(parts[0])
     *  execute builtin if appropiate
     *  else execve(parts[0], parts)
     *
     */

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
