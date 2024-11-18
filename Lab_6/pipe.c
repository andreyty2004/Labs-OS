#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include <time.h>

int main(int argc, char** argv)
{
    (void)argc; (void)argv;

    int res = 0;
    int pipdesc[2];
    int pipeRes = pipe(pipdesc);

    if(pipeRes != 0)
    {
        int err = errno;
        fprintf(stderr, "Error: %s (%d)\n", strerror(err), err);
        return 1;
    }

    switch(res = fork())
    {
        case -1:
        {
            int err = errno;
            fprintf(stderr, "Fork error: %s (%d)\n", strerror(err), err);
            break;
        }
        case 0:
        {
            char buf[128];

            time_t s_time = time(NULL);
            struct tm* c_time = localtime(&s_time);
            
            printf("CHILD process time: %s", asctime(c_time));
            close(pipdesc[1]);

            int len = 0;
            while((len = read(pipdesc[0], buf, sizeof(buf))) != 0)
                write(2, buf, len);

            close(pipdesc[0]);
            break;
        }
        default:
        {
            sleep(5);
            char buf[128];

            time_t s_time = time(NULL);
            struct tm* c_time = localtime(&s_time);

            sprintf(buf, "PARENT process time: %sPARENT pid: %d\n", asctime(c_time), getpid());
            close(pipdesc[0]);

            write(pipdesc[1], buf, strlen(buf));

            close(pipdesc[1]);
            break;
        }
    }

    return 0;
}