#include <stdio.h>
#include <pthread.h>
#include <cstdlib>
#include <time.h>

#include <windows.h>

#define buffer_size 1000
#define producer_size 20
#define consumer_size 30
#define request_size 1000000
#define WAIT_TIME_SECONDS       1

typedef struct thread_data
{
    long head;
    long tail;
    long space_buffer;
    char data_list[buffer_size];
}thread_data;

//Prototype
void initial_buffer();

void add_item(void*);
void remove_item();

void* append_buffer(void*);
void* remove_buffer(void*);

long timediff(clock_t,clock_t);

thread_data circular_queue;
pthread_mutex_t mutex_producer,mutex_consumer;
pthread_cond_t cond_not_empty,cond_not_full;

long temp_request_size = request_size;
long fails_request = 0;

char template_data[500];
int num = 0;
long num_a = 0;
long num_r = 0;

int producer_num = producer_size;
int consumer_num = 0;
int main()
{

	printf("Initializing...\n");

    clock_t t1, t2;
    int c;
    long elapsed;

    int i,return_code;
    char *temp_char_data = (char*)malloc(sizeof(*temp_char_data));
    *temp_char_data = 'a';
    pthread_t threads_append[producer_size];
    pthread_t threads_remove[consumer_size];

    //Init mutex
    pthread_mutex_init(&mutex_producer, NULL);
    pthread_mutex_init(&mutex_consumer, NULL);

    pthread_cond_init (&cond_not_empty, NULL);
    pthread_cond_init (&cond_not_full, NULL);
    //define Producer and Consumer threads
    pthread_t producer_threads[producer_size];
    pthread_t consumer_threads[consumer_size];

    printf("Producers %d, Consumers %d\n", producer_size, consumer_size);
    printf("Buffer size %d\n", buffer_size);
    printf("Requests %d\n\n", request_size);

    //set default value of circular_queue
    initial_buffer();

    //Init Data

    srand(time(NULL));
    for(i = 0;i < 500;i++){
    	template_data[i] = rand()*26 + 65;
    }

    circular_queue.space_buffer = buffer_size;

    printf("Start Benchmark Timer \n");
    // Start Clock
    t1 = clock();

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

    pthread_cond_broadcast(&cond_not_empty);

    for (i=0; i < consumer_size; i++)
    {
        pthread_join(threads_remove[i], NULL);
    }

    // Stop Clock
    t2 = clock();

    elapsed = timediff(t1, t2);
    printf("Elapsed: %ld s\n", elapsed/1000);

    // Clean up all of pthread.h lib and exit
    pthread_mutex_destroy(&mutex_producer);
    pthread_mutex_destroy(&mutex_consumer);

    pthread_cond_destroy(&cond_not_empty);
    pthread_cond_destroy(&cond_not_full);

    long success_request = request_size - fails_request;
    double throughput = (num_r)/(double)(elapsed/1000.0);

    printf("\tThroughput \t:\t %.2lf \tSuccessful Request/s", throughput);
    printf("\nAppend: %ld , Remove: %ld , Failed: %ld , Total Request: %ld",num_a,num_r,fails_request,num_r+fails_request);

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
    struct timespec   ts;
    struct timeval    tp;
    int return_code;
    while(1) {
    	// CheckBuffer is not Full
        //printf("Before Lock\n");
		pthread_mutex_lock(&mutex_producer);
		if(temp_request_size-- > 0){
		    //printf("Try Append\n");
			if (circular_queue.space_buffer != 0)
		       {
		           printf("Append Success %d\n",temp_request_size);
		           add_item(temp_data);
		           pthread_mutex_unlock(&mutex_producer);
		           pthread_cond_signal(&cond_not_empty);
		           num_a++;
		       } else {
		            printf("Append Failed Halt \n");
		       		ts.tv_sec  = tp.tv_sec;
				    ts.tv_nsec = tp.tv_usec * 1000;
				    ts.tv_sec += WAIT_TIME_SECONDS;
		           	//return_code = pthread_cond_timedwait(&cond_not_full, &mutex_producer, &ts);
		           	pthread_cond_wait(&cond_not_full, &mutex_producer);
		           	if(return_code == ETIMEDOUT){
                        pthread_mutex_unlock(&mutex_producer);
                        Sleep(10);
		           	}else{
                        add_item(temp_data);
                        pthread_mutex_unlock(&mutex_producer);
                        pthread_cond_signal(&cond_not_empty);
                        num_a++;
		           	}
		       }
		}else{
		    producer_num--;
		    //printf("%d\n",producer_num);
			break;
		}
    }
    pthread_mutex_unlock(&mutex_producer);
    pthread_exit(NULL);
}

void* remove_buffer(void* temp_queue)
{
    struct timespec   ts;
    struct timeval    tp;
    int return_code;
    while(1){
    	// Check Buffer is not Empty
    	pthread_mutex_lock(&mutex_consumer);
    	//printf("Lock\n");
    	if(producer_num > 0){
    		if (circular_queue.space_buffer != buffer_size)
	        {
	            /*printf("remove, %d\n", num);*/
	            //printf("Remove Success \n");
	            remove_item();
	            pthread_mutex_unlock(&mutex_consumer);
	            pthread_cond_signal(&cond_not_full);
	            num_r++;
	        } else if(producer_num > 0){
                ts.tv_sec  = tp.tv_sec;
                ts.tv_nsec = tp.tv_usec * 1000;
                ts.tv_sec += WAIT_TIME_SECONDS;
                return_code = pthread_cond_timedwait(&cond_not_empty, &mutex_consumer, &ts);
                pthread_cond_wait(&cond_not_empty, &mutex_consumer);
                printf("signal rcv\n");
                if(return_code == ETIMEDOUT){
                        printf("Timeout \n");
                        pthread_mutex_unlock(&mutex_consumer);
                        fails_request++;
                        Sleep(10);
                }else{
                        printf("Remove q \n");
                        remove_item();
                        pthread_mutex_unlock(&mutex_consumer);
                        pthread_cond_signal(&cond_not_full);
                        num_r++;
                }
	        }
    	}else{
    		break;
    	}
    }
    pthread_mutex_unlock(&mutex_consumer);
    consumer_num++;
    while(circular_queue.space_buffer < buffer_size){
        pthread_mutex_lock(&mutex_consumer);
        remove_item();
        pthread_mutex_unlock(&mutex_consumer);
        num_r++;
    }
    pthread_exit(NULL);
}

long timediff(clock_t t1, clock_t t2) {
    long elapsed;
    elapsed = ((double)t2 - t1) / CLOCKS_PER_SEC * 1000;
    return elapsed;
}
