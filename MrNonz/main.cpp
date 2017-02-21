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
void add_item(thread_data *, char);
void remove_item(thread_data *);

thread_data circular_queue;
pthread_mutex_t lock_head, lock_tail;

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

    add_item(&circular_queue, 'a');
    add_item(&circular_queue, 'b');
    remove_item(&circular_queue);

    printf("Last head and tail address is %d %d", circular_queue.head, circular_queue.tail);
    return 0;
}

void initial_buffet(thread_data *temp_buffer)
{
    temp_buffer->head = 0;
    temp_buffer->tail = 0;
}

void add_item(thread_data *temp_buffer, char temp_data)
{
    //Make sure you locked of Head
    temp_buffer->data_list[temp_buffer->head++] = temp_data;

    if (temp_buffer->head == buffer_size)
    {
        temp_buffer->head = 0;
    }
}

void remove_item(thread_data *temp_buffer)
{
    //Make sure you locked of Tail
    temp_buffer->tail++;

    if (temp_buffer->tail == buffer_size)
    {
        temp_buffer->tail = 0;
    }
}
