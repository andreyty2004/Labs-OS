#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>

#define SHM_SIZE 128

char* shm_filename = "shmem";
char* received_message = NULL;
int shm_id = 0;

void signal_handler(int signal)
{
    printf("[SIGNAL HANDLER]: %d received\n", signal);

    // if(received_message != NULL)
    //     free(sh_str);

    if(received_message != NULL)
    {
        int res = shmdt(received_message);
        if(res < 0)
        {
            int err = errno;
            fprintf(stderr, "shmdt: %s (%d)\n", strerror(err), err);
            exit(1);
        }
    }
    exit(0);
}

int main() 
{
    key_t key = ftok(shm_filename, 1);
    if(key < 0)
    {
        int err = errno;
        fprintf(stderr, "ftok: %s (%d)\n", strerror(err), err);
        return 1;
    }

    shm_id = shmget(key, SHM_SIZE, 0666);
    if (shm_id < 0) 
    {
        int err = errno;
        fprintf(stderr, "shmget: %s (%d)\n", strerror(err), err);
        return 1;
    }

    received_message = shmat(shm_id, NULL, SHM_RDONLY);
    if (received_message == (void*)-1) 
    {
        int err = errno;
        fprintf(stderr,"shmat: %s (%d)\n", strerror(err), err);
        return 1;
    }

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    while(1)
    {
        time_t s_time = time(NULL);
        struct tm* c_time = localtime(&s_time);
        printf("RECEIVING process time: %s--->RPID: %d\n", asctime(c_time), getpid());
        printf("message: %s\n", received_message);
        sleep(3);
    }

    return 0;
}

