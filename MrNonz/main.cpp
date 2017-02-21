/*
Coding by MrNonz
This is version 1.0
Lang C++
*/
#include <stdio.h>
#include <pthread.h>
#include <cstdlib>

#define buffer_size 1000
#define producer_size 20
#define consumer_size 30
#define request_size 100000

typedef struct thread_data
{
    long head;
    long tail;
    long space_buffer;
    char data_list[buffer_size];
}thread_data;

//function prototype
void initial_buffer();

void add_item(void*);
void remove_item();

void* append_buffer(void*);
void* remove_buffer(void*);

thread_data circular_queue;
pthread_mutex_t mutex_head, mutex_tail, mutex_space_buffer;
pthread_cond_t head_threshold, tail_threshold, space_buffer_threshold;

long temp_request_size = request_size;
long fails_request = 0;

int main()
{
    int i, return_code;
    char *temp_char_data = (char*)malloc(sizeof(*temp_char_data));
    *temp_char_data = 'a';
    pthread_t threads_append[producer_size];
    pthread_t threads_remove[consumer_size];

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
    initial_buffer();

    /*
    add_item(&circular_queue, 'a');
    add_item(&circular_queue, 'b');
    remove_item(&circular_queue);

    printf("Last head and tail address is %d %d", circular_queue.head, circular_queue.tail);
    */

    //create thread for append and remove
    /*append_buffer('a');
    append_buffer('b');
    append_buffer('c');
    remove_buffer();*/
    for (i = 0; i < producer_size; i++)
    {
        return_code = pthread_create(&threads_append[i], NULL, append_buffer, temp_char_data);

        if (return_code){
          printf("Producer; return code from pthread_create() is %d\n", return_code);
       }
    }
    for (i = 0; i <consumer_size; i++)
    {
        return_code = pthread_create(&threads_remove[i], NULL, remove_buffer, &circular_queue);

        if (return_code){
          printf("Consumer; return code from pthread_create() is %d\n", return_code);
       }
    }

    // Waiting all of threads completed
    for (i=0; i < producer_size; i++)
    {
        pthread_join(threads_append[i], NULL);
    }
    for (i=0; i < consumer_size; i++)
    {
        pthread_join(threads_remove[i], NULL);
    }


    // Clean up all of pthread.h lib and exit
    pthread_mutex_destroy(&mutex_head);
    pthread_mutex_destroy(&mutex_tail);
    pthread_mutex_destroy(&mutex_space_buffer);
    pthread_cond_destroy(&head_threshold);
    pthread_cond_destroy(&tail_threshold);
    pthread_cond_destroy(&space_buffer_threshold);
    //pthread_exit(NULL);

    long success_request = request_size - fails_request;
    printf("Successfully consumed %ld requests (%.2lf%)", success_request, (double)(success_request*100)/request_size);

    return 0;
}

void initial_buffer()
{
    // -1 is mean not data in buffer and not even insert data
    circular_queue.head = 0;
    circular_queue.tail = 0;
    circular_queue.space_buffer = buffer_size;
}

void add_item(void* temp_data)
{
    //Make sure you locked of Head
    circular_queue.data_list[circular_queue.head++] = *(char*)temp_data;

    if (circular_queue.head == buffer_size)
    {
        circular_queue.head = 0;
    }
}

void remove_item()
{
    //Make sure you locked of Tail
    circular_queue.tail++;

    if (circular_queue.tail == buffer_size)
    {
        circular_queue.tail = 0;
    }
}

void* append_buffer(void* temp_data)
{
    /*
    Check your buffer is not Full !!
    then lock your Head_mutex
    and call add_item then unlock your Head_mutex
    */
    while(temp_request_size > 0) {
        pthread_mutex_lock(&mutex_space_buffer);
        pthread_cond_signal(&space_buffer_threshold);

        //printf("Received signal space_buffer\n");

        //Check buffer is not Full !!
        if (circular_queue.space_buffer != 0)
        {
            pthread_mutex_lock(&mutex_head);
            pthread_cond_signal(&head_threshold);

            //add item to buffer
            add_item(temp_data);

            pthread_mutex_unlock(&mutex_head);

            //decrease space of buffer
            circular_queue.space_buffer--;

            //printf("Add item is success\n");
        } else {
            //printf("Fail to add item\n");
            fails_request++;
        }

        pthread_mutex_unlock(&mutex_space_buffer);
        //pthread_exit(NULL);
        temp_request_size--;
    }
}

void* remove_buffer(void* temp_queue)
{
    /*
    Check your buffer is not Empty !!
    then lock your Tail_mutex
    and call remove_item then unlock your Tail_mutex
    */
    while(temp_request_size > 0) {
        pthread_mutex_lock(&mutex_space_buffer);
        pthread_cond_signal(&space_buffer_threshold);

        //printf("Received signal space_buffer\n");

        //Check buffer is Empty !!
        if (circular_queue.space_buffer < buffer_size)
        {
            pthread_mutex_lock(&mutex_tail);
            pthread_cond_signal(&tail_threshold);

            //remove item from buffer
            remove_item();

            pthread_mutex_unlock(&mutex_tail);

            //increase space of buffer
            circular_queue.space_buffer++;

            //printf("Remove item is success\n");
        } else {
            //printf("Fail to remove item %ld\n", circular_queue.space_buffer);
            fails_request++;
        }

        pthread_mutex_unlock(&mutex_space_buffer);
        //pthread_exit(NULL);
        temp_request_size--;
    }
}
