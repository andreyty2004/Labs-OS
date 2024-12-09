#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

#define NUM_READERS 10
#define ARRAY_SIZE 5

pthread_mutex_t mutex;
int shared_array[ARRAY_SIZE];
int counter = 0;

void *reader_function(void *arg) 
{
    while (counter < ARRAY_SIZE) 
    { 
        usleep(10000);
        pthread_mutex_lock(&mutex);
        printf("%ld: ", pthread_self());
        for(int i = 0; i < ARRAY_SIZE; i++)
            printf("%d ", shared_array[i]);
        usleep(50000);
        printf("\n"); 
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(NULL);
}

void *writer_function(void *arg) 
{
    for(int i = 0; i < ARRAY_SIZE; i++)
    {
        usleep(10000);
        pthread_mutex_lock(&mutex);
        printf("\n");
        counter++;
        shared_array[i] = counter;
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(NULL);
}

int main() 
{
    pthread_t readers[NUM_READERS], writer;
    pthread_mutex_init(&mutex, NULL);

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
    pthread_join(writer, NULL);

    pthread_mutex_destroy(&mutex);
    return 0;
}

