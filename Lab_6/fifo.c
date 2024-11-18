#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>

int main(int argc, char** argv)
{
    (void)argc; (void)argv;

    char* fifoName = "testFifo";
    int fifoRes = mkfifo(fifoName, S_IRUSR | S_IWUSR);

    if(fifoRes != 0)
    {
        int err = errno;
        fprintf(stderr, "Error: %s (%d)\n", strerror(err), err);
        return 1;
    }

    int res = 0;
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
            int fd = open(fifoName, O_RDONLY);
            if(fd == -1)
            {
                fprintf(stderr, "Error opening fifo\n");
                return 1;
            }

            char buf[128];

            time_t s_time = time(NULL);
            struct tm* c_time = localtime(&s_time);
            
            printf("CHILD process time: %s", asctime(c_time));

            int len = 0;
            while((len = read(fd, buf, sizeof(buf))) != 0)
                write(2, buf, len);

            close(fd);
            break;
        }
        default:
        {
            int fd = open(fifoName, O_WRONLY);
            if(fd == -1)
            {
                fprintf(stderr, "Error opening fifo\n");
                return 1;
            }

            sleep(5);
            char buf[128];

            time_t s_time = time(NULL);
            struct tm* c_time = localtime(&s_time);

            sprintf(buf, "PARENT process time: %sPARENT pid: %d\n", asctime(c_time), getpid());

            write(fd, buf, strlen(buf));

            close(fd);
            break;
        }
    }
    unlink(fifoName);
    return 0;
}