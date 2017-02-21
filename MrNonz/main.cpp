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
    long space_buffer;
    char data_list[buffer_size];
}thread_data;

//function prototype
void initial_buffer(thread_data *);

void add_item(thread_data *, char);
void remove_item(thread_data *);

void append_buffer(thread_data *, char);
void remove_buffer(thread_data *);

thread_data circular_queue;
pthread_mutex_t mutex_head, mutex_tail, mutex_space_buffer;
pthread_cond_t head_threshold, tail_threshold, space_buffer_threshold;

int main()
{
    //Initialize mutex&condition objects
    pthread_mutex_init(&mutex_head, NULL);
    pthread_mutex_init(&mutex_tail, NULL);
    pthread_mutex_init(&mutex_space_buffer, NULL);
    pthread_cond_init (&head_threshold, NULL);
    pthread_cond_init (&tail_threshold, NULL);
    pthread_cond_init (&space_buffer_threshold, NULL);

    //define Producer and Consumer threads
    pthread_t producer_threads[producer_size];
    pthread_t consumer_threads[consumer_size];

    printf("Producers %d, Consumers %d\n", producer_size, consumer_size);
    printf("Buffer size %d\n", buffer_size);
    printf("Requests %d\n\n", request_size);

    //set default value of circular_queue
    initial_buffer(&circular_queue);

    /*
    add_item(&circular_queue, 'a');
    add_item(&circular_queue, 'b');
    remove_item(&circular_queue);

    printf("Last head and tail address is %d %d", circular_queue.head, circular_queue.tail);
    */
    append_buffer(&circular_queue, 'a');
    append_buffer(&circular_queue, 'b');
    append_buffer(&circular_queue, 'c');
    remove_buffer(&circular_queue);


    // Clean up all of pthread.h lib and exit
    pthread_mutex_destroy(&mutex_head);
    pthread_mutex_destroy(&mutex_tail);
    pthread_cond_destroy(&head_threshold);
    pthread_cond_destroy(&tail_threshold);
    //pthread_exit(NULL);
    return 0;
}

void initial_buffer(thread_data *temp_buffer)
{
    // -1 is mean not data in buffer and not even insert data
    temp_buffer->head = 0;
    temp_buffer->tail = 0;
    temp_buffer->space_buffer = buffer_size;
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

void append_buffer(thread_data *temp_buffer, char temp_data)
{
    /*
    Check your buffer is not Full !!
    then lock your Head_mutex
    and call add_item then unlock your Head_mutex
    */
    pthread_mutex_lock(&mutex_space_buffer);
    pthread_cond_signal(&space_buffer_threshold);

    printf("Received signal space_buffer\n");

    //Check buffer is not Full !!
    if (temp_buffer->space_buffer != 0)
    {
        pthread_mutex_lock(&mutex_head);
        pthread_cond_signal(&head_threshold);

        //add item to buffer
        add_item(temp_buffer, temp_data);

        pthread_mutex_unlock(&mutex_head);

        printf("Add item is success\n");
    } else {
        printf("Fail to add item\n");
    }

    //decrease space of buffer
    temp_buffer->space_buffer--;
    pthread_mutex_unlock(&mutex_space_buffer);
}

void remove_buffer(thread_data *temp_buffer)
{
    /*
    Check your buffer is not Empty !!
    then lock your Tail_mutex
    and call remove_item then unlock your Tail_mutex
    */

    pthread_mutex_lock(&mutex_space_buffer);
    pthread_cond_signal(&space_buffer_threshold);

    printf("Received signal space_buffer\n");

    //Check buffer is Empty !!
    if (temp_buffer->space_buffer != buffer_size)
    {
        pthread_mutex_lock(&mutex_tail);
        pthread_cond_signal(&tail_threshold);

        //remove item from buffer
        remove_item(temp_buffer);

        pthread_mutex_unlock(&mutex_tail);

        printf("Remove item is success\n");
    } else {
        printf("Fail to remove item %ld\n", temp_buffer->space_buffer);
    }

    //increase space of buffer
    temp_buffer->space_buffer++;
    pthread_mutex_unlock(&mutex_space_buffer);
}
