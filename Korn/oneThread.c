#include<stdio.h>
#define MAX 20
#define CONSUMER 30
#define PROCEDUCE 20

char buffer[1000];
int head = 0,tail = 0;


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

void add_item(){
    circular_queue.data_list[circular_queue.head] = 'a';

    if(circular_queue.data_list[circular_queue.head]+1 == buffer_size){
        circular_queue.head = 0;
    }else{
        circular_queue.head++;
    }
    printf("add\ta head : %d // tail : %d\n",circular_queue.head,circular_queue.tail);
}
void remove_item(){
    char ret = circular_queue.data_list[circular_queue.tail];

    if(circular_queue.tail+1 == buffer_size){
        circular_queue.tail = 0;
    }else{
        circular_queue.tail++;
    }
    printf("remove\ta head : %d // tail : %d\n",circular_queue.head,circular_queue.tail);
}

void *Append(){
    if(circular_queue.head+1 == circular_queue.tail || (circular_queue.head+1 == buffer_size && circular_queue.tail == 0)){
        printf("cannot add q is full\n");
    }else{
        add_item();
    }
}

void *Remove(){
    if(circular_queue.head == circular_queue.tail){
        printf("cannot remove q is empty\n");
    }else{
        remove_item();
    }
}

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

    /*test Append and Remove*/
    *Append();
    *Append();
    *Append();
    *Remove();
    *Remove();
    *Append();
    *Append();
    *Remove();
    *Append();
    *Append();
    *Remove();
    *Append();
    *Append();
    *Remove();
    *Remove();
    *Append();

    return 0;
}

void initial_buffet(thread_data *temp_buffer)
{
    temp_buffer->head = 0;
    temp_buffer->tail = 0;
}


