/*
This is program use for Assignment 1.
OS in Computer Engineering of KMITL

Version 1.59
Developed by Micky & MrNonz & DragonKorn
*/
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>
#define WAIT_TIME_SECONDS   1
typedef struct thread_data
{
    long head;
    long tail;
    long space_buffer;
    char data_list[1005];
} thread_data;

//Function Prototype
void initial_buffer();

void add_item(void*);
void remove_item();

void* append_buffer(void*);
void* remove_buffer(void*);

int timeval_subtract(struct timeval*, struct timeval*, struct timeval*);

thread_data circular_queue;
pthread_mutex_t mutex_for_lock_buffer_queue;
pthread_cond_t condition_append,condition_remove;

long temp_request_size;
long fail_removed = 0;

char template_data[500];

long success_appended = 0;
long success_removed = 0;

int producer_num = 0;

int buffer_size;
int producer_size;
int consumer_size;

long request_size;
long append_per_thread;

double elapsed_time;

int main()
{
    printf("Insert your Producer, Consumer, Buffer, Request\n\n");
    scanf("%d %d %d %ld", &producer_size, &consumer_size, &buffer_size, &request_size);
    /*
    producer_size = 20;
    consumer_size = 20;
    buffer_size = 1000;
    request_size = 20000000;
    */

    producer_num = producer_size;
    temp_request_size = request_size;
    circular_queue.space_buffer = buffer_size;

    struct timeval tvBegin, tvEnd, tvDiff;

    int i,return_code;
    char *temp_char_data = (char*)malloc(sizeof(*temp_char_data));
    *temp_char_data = 'a';

    //define Producer and Consumer threads
    pthread_t producer_threads[producer_size];
    pthread_t consumer_threads[consumer_size];

    //Init mutex
    pthread_mutex_init(&mutex_for_lock_buffer_queue, NULL);

    pthread_cond_init(&condition_append,NULL);
    pthread_cond_init(&condition_remove,NULL);

    //set default value of circular_queue
    initial_buffer();

    printf("\nInitializing...\n");
    printf("Producers %d, Consumers %d\n", producer_size, consumer_size);
    printf("Buffer size %d\n", buffer_size);
    printf("Requests %ld\n\n", request_size);

    append_per_thread = temp_request_size / producer_size;

    printf("\tStart Benchmark Timer : \n");

    // Start Clock
    gettimeofday(&tvBegin, NULL);

    for (i = 0; i < producer_size; i++)
    {
        return_code = pthread_create(&producer_threads[i], NULL, append_buffer, temp_char_data);

        if (return_code)
        {
            printf("Producer; return code from pthread_create() is %d\n", return_code);
        }
    }
    for (i = 0; i <consumer_size; i++)
    {
        return_code = pthread_create(&consumer_threads[i], NULL, remove_buffer, &circular_queue);

        if (return_code)
        {
            printf("Consumer; return code from pthread_create() is %d\n", return_code);
        }
    }
    // Waiting all of threads completed
    for (i=0; i < producer_size; i++)
    {
        pthread_join(producer_threads[i], NULL);
    }

    for (i=0; i < consumer_size; i++)
    {
        pthread_join(consumer_threads[i], NULL);
    }

    // Stop Clock
    gettimeofday(&tvEnd, NULL);
    timeval_subtract(&tvDiff, &tvEnd, &tvBegin);

    // Clean up all of pthread.h lib and exit
    pthread_mutex_destroy(&mutex_for_lock_buffer_queue);

    pthread_cond_destroy(&condition_append);
    pthread_cond_destroy(&condition_remove);

    elapsed_time = tvDiff.tv_sec + (double)(tvDiff.tv_usec/1000000.0);

    long success_request = request_size - fail_removed;
    double throughput = (success_removed)/elapsed_time;

    printf("\tSuccessfullly Consumed \t:\t %ld (%.2lf%%)\n",success_removed,(double)(success_removed*100/request_size));
    printf("\t\tElapsed \t:\t %ld.%06ld s\n", tvDiff.tv_sec, tvDiff.tv_usec);
    printf("\t\tThroughput \t:\t %.2lf \tSuccessful Request/s\n", throughput);
    return 0;
}

void initial_buffer()
{
    // -1 is mean not data in buffer and not even insert data
    circular_queue.head = 0;
    circular_queue.tail = 0;
}

void add_item(void* temp_data)
{
    //Make sure you locked of Head
    circular_queue.data_list[circular_queue.head++] = *(char*)temp_data;

    if (circular_queue.head == buffer_size)
    {
        circular_queue.head = 0;
    }
    circular_queue.space_buffer--;
}

void remove_item()
{
    //Make sure you locked of Tail
    circular_queue.data_list[circular_queue.tail++] = '\0';

    if (circular_queue.tail == buffer_size)
    {
        circular_queue.tail = 0;
    }
    circular_queue.space_buffer++;
}

void* append_buffer(void* temp_data)
{
    int               return_code;
    struct timespec   time_spec;
    struct timeval    time_value;

    int try_time = 0;
    while(1)
    {
        try_time = 0;
        while(pthread_mutex_trylock(&mutex_for_lock_buffer_queue) != 0)
        {
            try_time+=25;
            usleep(try_time);
        }

        // CheckBuffer is not Full
        if(temp_request_size > 0)
        {
append_point:
            if (circular_queue.space_buffer > 0)
            {
                add_item(temp_data);

                success_appended++;
                temp_request_size--;

                pthread_cond_signal(&condition_remove);
                pthread_mutex_unlock(&mutex_for_lock_buffer_queue);
            }
            else
            {
                time_spec.tv_sec  = time_value.tv_sec;
                time_spec.tv_nsec = time_value.tv_usec * 1000;
                time_spec.tv_sec += WAIT_TIME_SECONDS;

                return_code = pthread_cond_timedwait(&condition_append,&mutex_for_lock_buffer_queue,&time_spec);

                if(return_code == ETIMEDOUT)
                {
                    pthread_mutex_unlock(&mutex_for_lock_buffer_queue);
                }
                else
                {
                    goto append_point;
                }
            }
        }
        else
        {
            producer_num--;
            break;
        }
    }

    pthread_mutex_unlock(&mutex_for_lock_buffer_queue);
    pthread_exit(NULL);
}

void* remove_buffer(void* temp_queue)
{
    int               return_code;
    struct timespec   time_spec;
    struct timeval    time_value;

    int try_time = 0;
    while(1)
    {
        try_time = 0;
        while(pthread_mutex_trylock(&mutex_for_lock_buffer_queue) != 0)
        {
            try_time+=25;
            usleep(try_time);
        }

        // Check Buffer is not Empty
        if(success_removed < request_size)
        {
remove_point:
            if(circular_queue.space_buffer < buffer_size)
            {
                remove_item();
                success_removed++;

                pthread_cond_signal(&condition_append);
                pthread_mutex_unlock(&mutex_for_lock_buffer_queue);
            }
            else
            {
                time_spec.tv_sec  = time_value.tv_sec;
                time_spec.tv_nsec = time_value.tv_usec * 1000;
                time_spec.tv_sec += WAIT_TIME_SECONDS;

                return_code = pthread_cond_timedwait(&condition_remove,&mutex_for_lock_buffer_queue,&time_spec);
                if(return_code == ETIMEDOUT)
                {
                    pthread_mutex_unlock(&mutex_for_lock_buffer_queue);
                }
                else
                {
                    goto remove_point;
                }

            }
        }
        else
        {
            break;
        }
    }

    pthread_mutex_unlock(&mutex_for_lock_buffer_queue);
    pthread_exit(NULL);
}

int timeval_subtract(struct timeval *result, struct timeval *t2, struct timeval *t1)
{
    long int diff = (t2->tv_usec + 1000000 * t2->tv_sec) - (t1->tv_usec + 1000000 * t1->tv_sec);
    result->tv_sec = diff / 1000000;
    result->tv_usec = diff % 1000000;

    return (diff<0);
}
