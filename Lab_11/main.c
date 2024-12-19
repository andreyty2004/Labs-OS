#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

#define NUM_READERS 10
#define ARRAY_SIZE 5

pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;
char shared_array[ARRAY_SIZE];
int counter = 0;

void *reader_function(void *arg) 
{
    while (counter < ARRAY_SIZE) 
    { 
        usleep(10000);
        pthread_rwlock_rdlock(&rwlock);
        printf("%ld: %s\n", pthread_self(), shared_array);
        pthread_rwlock_unlock(&rwlock);
    }
    pthread_exit(NULL);
}

void *writer_function(void *arg) 
{
    for(int i = 0; i < ARRAY_SIZE; i++)
    {
        pthread_rwlock_wrlock(&rwlock);
        usleep(100000);
        printf("\n");
        counter++;
        shared_array[i] = counter + '0';
        pthread_rwlock_unlock(&rwlock);
    }
    pthread_exit(NULL);
}

int main() 
{
    for(int i = 0; i < ARRAY_SIZE; i++)
        shared_array[i] = '0';

    pthread_t readers[NUM_READERS], writer;

    for (int i = 0; i < NUM_READERS; i++) 
    {
        int create_res = pthread_create(&readers[i], NULL, reader_function, NULL);
        if(create_res != 0)
        {
            int err = errno;
            printf("pthread_create error: %s (%d)\n", strerror(err), err);
        }
    }
    pthread_create(&writer, NULL, writer_function, NULL);
    
    for (int i = 0; i < NUM_READERS; i++) 
    {
        int join_res = pthread_join(readers[i], NULL);
        if(join_res != 0)
        {
            int err = errno;
            printf("pthread_join error: %s (%d)\n", strerror(err), err);
        }
    }
    
    int join_res = pthread_join(writer, NULL);
    if(join_res != 0)
    {
        int err = errno;
        printf("pthread_join error: %s (%d)\n", strerror(err), err);
    }

    return 0;
}

