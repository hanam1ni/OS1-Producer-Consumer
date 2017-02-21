#include<stdio.h>
#define MAX 20
#define CONSUMER 30
#define PROCEDUCE 20

char buffer[1000];
int head = 0,tail = 0;


// add_item
void add_item(){

    buffer[head] = 'a';

    if(head+1 == MAX){
        head = 0;
    }else{
        head++;
    }
    printf("add\ta head : %d // tail : %d\n",head,tail);
}

// add_item
void remove_item(){
    char ret = buffer[tail];

    if(tail+1 == MAX){
        tail = 0;
    }else{
        tail++;
    }

    printf("remove\ta head : %d // tail : %d\n",head,tail);
}

void *Append(){
    if(head+1 == tail || (head+1 == MAX && tail == 0)){
        printf("cannot add q is full\n");
    }else{
        add_item();
    }
}

void *Remove(){
    if(head == tail){
        printf("cannot remove q is empty\n");
    }else{
        remove_item();
    }
}

void main(){



    return 0;
}
