#include <stdio.h>
#include <pthread.h>
#include <time.h>

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

//define pthread
pthread_t producer,consumer;
//define mutex variable
pthread_mutex_t mutex_t;

void add_item(int tid){
    circular_queue.data_list[circular_queue.head] = 'a';

    if(circular_queue.head+1 == buffer_size){
        circular_queue.head = 0;
    }else{
        circular_queue.head++;
    }
    printf("thread %d\tadd\ta head : %d // tail : %d\n",tid,circular_queue.head,circular_queue.tail);
}
void remove_item(int tid){
    char ret = circular_queue.data_list[circular_queue.tail];
    if(circular_queue.tail+1 == buffer_size){
        circular_queue.tail = 0;
    }else{
        circular_queue.tail++;
    }
    printf("thread %d\tremove\ta head : %d // tail : %d\n",tid,circular_queue.head,circular_queue.tail);
}

void *Append(void *threadid){
    pthread_mutex_lock (&mutex_t);
    int tid = (int)threadid;
    if(circular_queue.head+1 == circular_queue.tail || (circular_queue.head+1 == buffer_size && circular_queue.tail == 0)){
        printf("cannot add q is full\n");
    }else{
        add_item(tid);
    }
    pthread_mutex_unlock(&mutex_t);
    pthread_exit(NULL);
}

void *Remove(void *threadid){
    pthread_mutex_lock (&mutex_t);
    int tid = (int)threadid;
    if(circular_queue.head == circular_queue.tail){
        printf("cannot remove q is empty\n");
    }else{
        remove_item(tid);
    }
    pthread_mutex_unlock(&mutex_t);
    pthread_exit(NULL);
}

int main()
{
    int rc,rd;
    int t;
    //define Producer and Consumer threads
    pthread_t producer_threads[producer_size];
    pthread_t consumer_threads[consumer_size];

    printf("Producers %d, Consumers %d\n", producer_size, consumer_size);
    printf("Buffer size %d\n", buffer_size);
    printf("Requests %d\n", request_size);

    //set default value of circular_queue
    initial_buffet(&circular_queue);

    /*test Append and Remove*/
    printf("\tcreate thread\n");
    for(t = 0;t<request_size;t++){
        rc = pthread_create(&producer, NULL, Append, (void *)t);
        if(rc){
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
        rd = pthread_create(&consumer, NULL, Remove, (void *)t);
        if(rd){
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }



    return 0;
}

void initial_buffet(thread_data *temp_buffer)
{
    temp_buffer->head = 0;
    temp_buffer->tail = 0;
}


