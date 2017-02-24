/*
This is program use for Assignment 1.
OS in Computer Engineering of KMITL

Version 1.4
Developed by Micky & MrNonz & DragonKorn
*/
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include  <errno.h>
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

long timediff(clock_t,clock_t);

thread_data circular_queue;
pthread_mutex_t mutex_producer,mutex_consumer;
pthread_cond_t cond_a,cond_b;
bool can_append = true;
bool can_remove = true;
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

int main()
{
    printf("Insert your Producer, Consumer, Buffer, Request\n\n");
    scanf("%d %d %d %ld", &producer_size, &consumer_size, &buffer_size, &request_size);

    producer_num = producer_size;
    temp_request_size = request_size;
    circular_queue.space_buffer = buffer_size;

    clock_t start_time, end_time;
    long elapsed;

    int i,return_code;
    char *temp_char_data = (char*)malloc(sizeof(*temp_char_data));
    *temp_char_data = 'a';

    //define Producer and Consumer threads
    pthread_t producer_threads[producer_size];
    pthread_t consumer_threads[consumer_size];

    //Init mutex
    pthread_mutex_init(&mutex_producer, NULL);
    pthread_mutex_init(&mutex_consumer, NULL);

    pthread_cond_init(&cond_a,NULL);
    pthread_cond_init(&cond_b,NULL);
    //set default value of circular_queue
    initial_buffer();

    printf("\nInitializing...\n");
    printf("Producers %d, Consumers %d\n", producer_size, consumer_size);
    printf("Buffer size %d\n", buffer_size);
    printf("Requests %ld\n\n", request_size);

    append_per_thread = temp_request_size / producer_size;

    printf("\tStart Benchmark Timer : \n");

    // Start Clock
    start_time = clock();

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

    while(circular_queue.space_buffer < buffer_size)
    {
        remove_item();
        success_removed++;
    }

    // Stop Clock
    end_time = clock();

    elapsed = timediff(start_time, end_time);


    // Clean up all of pthread.h lib and exit
    pthread_mutex_destroy(&mutex_producer);
    pthread_mutex_destroy(&mutex_consumer);

    long success_request = request_size - fail_removed;
    double throughput = (success_removed)/(double)(elapsed/1000.0);

    printf("\tSuccessfullly Consumed \t:\t %ld (%.2lf%%)\n",success_removed,(double)(success_removed*100/request_size));
    printf("\t\tElapsed \t:\t %.2lf s\n", (double)elapsed/1000.0);
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
    //circular_queue.data_list[circular_queue.head++] = *(char*)temp_data;

    circular_queue.head++;
    if (circular_queue.head == buffer_size)
    {
        circular_queue.head = 0;
    }
    circular_queue.space_buffer--;
}

void remove_item()
{
    //Make sure you locked of Tail

    circular_queue.tail++;
    if (circular_queue.tail == buffer_size)
    {
        circular_queue.tail = 0;
    }
    circular_queue.space_buffer++;
}

void* append_buffer(void* temp_data)
{
    int               return_code;
    struct timespec   ts;
    struct timeval    tp;

    int try_time = 0;
    while(1)
    {
        try_time = 0;
        while(pthread_mutex_trylock(&mutex_producer) != 0)
        {
            try_time+=25;
            usleep(try_time);
        }

        // CheckBuffer is not Full
        //printf("Before Lock\n");
        if(temp_request_size > 0)
        {
            //printf("Try Append\n");
            append_point:
            if (circular_queue.space_buffer > 0)
            {
                //printf("Append Success %d\n",temp_request_size);
                add_item(temp_data);
                success_appended++;
                pthread_cond_signal(&cond_b);
                temp_request_size--;
                pthread_mutex_unlock(&mutex_producer);
            }
            else
            {
                ts.tv_sec  = tp.tv_sec;
                ts.tv_nsec = tp.tv_usec * 1000;
                ts.tv_sec += WAIT_TIME_SECONDS;
                return_code = pthread_cond_timedwait(&cond_a,&mutex_producer,&ts);
                if(return_code == ETIMEDOUT){
                    pthread_mutex_unlock(&mutex_producer);
                }else{
                    goto append_point;
                }
            }
        }
        else
        {
            producer_num--;
            //printf("%d\n",producer_num);
            break;
        }
    }
    pthread_mutex_unlock(&mutex_producer);
    //printf("Done\n");
    pthread_exit(NULL);
}

void* remove_buffer(void* temp_queue)
{
    int               return_code;
    struct timespec   ts;
    struct timeval    tp;

    int try_time = 0;
    while(1)
    {
        try_time = 0;
        while(pthread_mutex_trylock(&mutex_producer) != 0)
        {
            try_time+=25;
            usleep(try_time);
        }
        // Check Buffer is not Empty
        //printf("Lock\n");
        if(success_removed < request_size)
        {
            remove_point:
            if(circular_queue.space_buffer < buffer_size)
            {
                //printf("Remove Success %d\n",temp_request_size);
                remove_item();
                success_removed++;
                pthread_cond_signal(&cond_a);
                pthread_mutex_unlock(&mutex_producer);
            }
            else
            {
                ts.tv_sec  = tp.tv_sec;
                ts.tv_nsec = tp.tv_usec * 1000;
                ts.tv_sec += WAIT_TIME_SECONDS;
                return_code = pthread_cond_timedwait(&cond_b,&mutex_producer,&ts);
                if(return_code == ETIMEDOUT){
                    pthread_mutex_unlock(&mutex_producer);
                }else{
                    //printf("Goto\n");
                    goto remove_point;
                }

            }
        }
        else
        {
            break;
        }
    }
    pthread_mutex_unlock(&mutex_producer);

    /*
    pthread_mutex_lock(&mutex_consumer);
    pthread_mutex_lock(&mutex_producer);

    while(circular_queue.space_buffer < buffer_size)
    {
        circular_queue.space_buffer--;
        success_removed++;
    }

    pthread_mutex_unlock(&mutex_producer);
    pthread_mutex_unlock(&mutex_consumer);
    */
    pthread_exit(NULL);
}

long timediff(clock_t start_time, clock_t end_time)
{
    long elapsed;
    elapsed = ((double)end_time - start_time) / CLOCKS_PER_SEC * 1000;
    return elapsed;
}
