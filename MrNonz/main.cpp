#include <stdio.h>
#include <pthread.h>
#include <cstdlib>
#include <time.h>

#include <windows.h>

#define buffer_size 1000
#define producer_size 20
#define consumer_size 30
#define request_size 10000000
#define NUM_TRY 3

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
pthread_mutex_t mutex_op,mutex_size, mutex_head, mutex_tail;

long temp_request_size = request_size;
long fails_request = 0;

char template_data[500];
int num = 0;
long num_a = 0;
long num_r = 0;
int main()
{

	printf("Initializing...\n");

    clock_t t1, t2;
    int c;
    long elapsed;

    int i, return_code;
    char *temp_char_data = (char*)malloc(sizeof(*temp_char_data));
    *temp_char_data = 'a';
    pthread_t threads_append[producer_size];
    pthread_t threads_remove[consumer_size];

    //Init mutex
    pthread_mutex_init(&mutex_op, NULL);
    pthread_mutex_init(&mutex_size, NULL);

    pthread_mutex_init(&mutex_head, NULL);
    pthread_mutex_init(&mutex_tail, NULL);
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
    for (i=0; i < consumer_size; i++)
    {
        pthread_join(threads_remove[i], NULL);
    }

    // Stop Clock
    t2 = clock();

    elapsed = timediff(t1, t2);
    printf("Elapsed: %ld s\n", elapsed/1000);

    // Clean up all of pthread.h lib and exit
    pthread_mutex_destroy(&mutex_op);

    long success_request = request_size - fails_request;
    double throughput = (success_request)/(double)(elapsed/1000.0);

    printf("Successfully consumed \t:\t %ld \trequests (%.2lf%%)\n", success_request, (double)(success_request)*100/(double)request_size);
    printf("\tThroughput \t:\t %.2lf \tSuccessful Request/s", throughput);
    printf("\nAppend: %ld , Remove: %ld , Failed: %ld , Total Request: %ld",num_a,num_r,fails_request,num_a+num_r+fails_request);

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
    pthread_mutex_lock(&mutex_head);

    circular_queue.head++;
    if (circular_queue.head == buffer_size)
    {
        circular_queue.head = 0;
    }
    circular_queue.space_buffer--;
    pthread_mutex_unlock(&mutex_head);
}

void remove_item()
{
    //Make sure you locked of Tail
    pthread_mutex_lock(&mutex_tail);

    circular_queue.tail++;
    if (circular_queue.tail == buffer_size)
    {
        circular_queue.tail = 0;
    }
    circular_queue.space_buffer++;
    pthread_mutex_unlock(&mutex_tail);
}

void* append_buffer(void* temp_data)
{
    while(temp_request_size-- > 0) {
    		// CheckBuffer is not Full
    		//printf("Try Append\n");
	        if (circular_queue.space_buffer != 0)
	        {
                num_a++;
	            //printf("add, %d\n", num);
	            //printf("Append Success %d\n",temp_request_size);
	            add_item(temp_data);
	        } else {
	            //printf("Append Failed Halt %d\n",temp_request_size);
	            fails_request++;
	        }
    }
    pthread_exit(NULL);
}

void* remove_buffer(void* temp_queue)
{
    while(temp_request_size-- > 0) {

    		// Check Buffer is not Empty
    		//printf("Try Remove\n");
	        if (circular_queue.space_buffer != buffer_size)
	        {
	            num_r++;
	            /*printf("remove, %d\n", num);*/
	            //printf("Remove Success %d\n",temp_request_size);
	            remove_item();
	        } else {
	            //printf("Remove Failed Halt %d\n",temp_request_size);
	            fails_request++;
	        }
     	//printf("%d\n",temp_request_size);
    }
    pthread_exit(NULL);
}

long timediff(clock_t t1, clock_t t2) {
    long elapsed;
    elapsed = ((double)t2 - t1) / CLOCKS_PER_SEC * 1000;
    return elapsed;
}
