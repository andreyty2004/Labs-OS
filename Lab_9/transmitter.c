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
#include <sys/sem.h>

#define SHM_SIZE 128

char* shm_filename = "shmem";
char* shm_message = NULL;
int shm_id = 0;
int sem_id = 0;

void signal_handler(int signal)
{
    printf("[SIGNAL HANDLER]: %d received\n", signal);

    int res = shmdt(shm_message);   
    if(shm_message != NULL)
    {
        if(res < 0)
        {
            int err = errno;
            fprintf(stderr, "shmdt: %s (%d)\n", strerror(err), err);
            exit(1);
        }
    }

    res = shmctl(shm_id, IPC_RMID, NULL);
    if(res < 0)
    {
        int err = errno;
        fprintf(stderr, "shmctl: %s (%d)\n", strerror(err), err);
        exit(1);
    }

    res = semctl(sem_id, 0, IPC_RMID);
    if(sem_id != 0)
    {
        if(res < 0)
        {
            int err = errno;
            fprintf(stderr, "semctl: %s (%d)\n", strerror(err), err);
            exit(1);
        }
    }

    if(remove(shm_filename) == -1)
    {
        int err = errno;
        fprintf(stderr, "remove: %s (%d)\n", strerror(err), err);
        exit(1);
    }

    exit(0);
}

int main() 
{
    struct sembuf sem_lock = {0, -1, 0}, sem_open = {0, 1, 0};

    int fd = open(shm_filename, O_CREAT | O_EXCL, S_IWUSR | S_IRUSR);
    if(fd == -1)
    {
        fprintf(stderr, "Error creating file\n");
        return 1;
    }
    close(fd);

    key_t key = ftok(shm_filename, 1);
    if(key < 0)
    {
        int err = errno;
        fprintf(stderr, "ftok: %s (%d)\n", strerror(err), err);
        return 1;
    }

    shm_id = shmget(key, SHM_SIZE, 0666 | IPC_CREAT);
    if (shm_id < 0) 
    {
        int err = errno;
        fprintf(stderr, "shmget: %s (%d)\n", strerror(err), err);
        return 1;
    }

    sem_id = semget(key, 1, 0666 | IPC_CREAT);
    semop(sem_id, &sem_open, 1);
    if (sem_id < 0) 
    {
        int err = errno;
        fprintf(stderr, "semget: %s (%d)\n", strerror(err), err);
        return 1;
    }

    shm_message = shmat(shm_id, NULL, 0);
    if (shm_message == (void*)-1) 
    {
        int err = errno;
        fprintf(stderr,"shmat: %s (%d)\n", strerror(err), err);
        return 1;
    }

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    while (1) 
    {
        semop(shm_id, &sem_lock, 1);
        time_t s_time = time(NULL);
        struct tm* c_time = localtime(&s_time);
        char message[SHM_SIZE];
        sprintf(message, "TRANSMITTING process time: %s--->TPID: %d\n", asctime(c_time), getpid());
        strcpy(shm_message, message);
        semop(shm_id, &sem_open, 1);
        sleep(1);
    }
    return 0;
}