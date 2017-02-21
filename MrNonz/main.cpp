/*
Coding by MrNonz
This is version 1.0
Lang C++
*/
#include <stdio.h>
#include <pthread.h>

#define buffer_size 1000
#define producer_size 20
#define consumer_size 30
#define request_size 100000
typedef struct thread_data
{
    int head;
    int tail;
    char data_list[buffer_size];
}thread_data;

//function prototype
void initial_buffet(thread_data *);

thread_data circular_queue;

int main()
{
    //define Producer and Consumer threads
    pthread_t producer_threads[producer_size];
    pthread_t consumer_threads[consumer_size];

    printf("Producers %d, Consumers %d\n", producer_size, consumer_size);
    printf("Buffer size %d\n", buffer_size);
    printf("Requests %d\n", request_size);

    //set default value of circular_queue
    initial_buffet(&circular_queue);


    return 0;
}

void initial_buffet(thread_data *temp_buffer)
{
    temp_buffer->head = 0;
    temp_buffer->tail = 0;
}
