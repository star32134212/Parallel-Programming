IP：140.113.215.195  
Port：37031 ~ 37034  
`ssh -p 37031 0513404@140.113.215.195`  
`gcc -pthread -std=gnu99 -O2 -s hw1.c -o pi`  
`time ./pi 2 1000000000` : 加time可以順便看時間  

```
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

pthread_mutex_t mutex;
unsigned long long number_in_circle;
unsigned long long  number_of_cpu, number_of_tosses;

void* randtoss(void* rank){
    srand((unsigned int)time(NULL));
    double x, y, distance_squared;
    long my_rank = (long) rank;
    unsigned long long my_toss = number_of_tosses/number_of_cpu;
    unsigned long long incircle, toss;
    incircle = 0;
    for (toss = 0; toss < my_toss; toss++) {
        x = ((float)rand())/RAND_MAX;
        y = ((float)rand())/RAND_MAX;
        distance_squared = x*x + y*y;
        if (distance_squared <= 1)
            incircle++;
    }
    pthread_mutex_lock(&mutex);
    number_in_circle += incircle;
    pthread_mutex_unlock(&mutex);

    return NULL;
}

int main(int argc, char **argv)
{
    double pi_estimate;
    long thread;
    pthread_t* thread_handle;
    if ( argc < 2) {
        exit(-1);
    }
    number_of_cpu = atoi(argv[1]);
    number_of_tosses = atoi(argv[2]);
    if (( number_of_cpu < 1) || ( number_of_tosses < 0)) {
        exit(-1);
    }
    number_in_circle = 0;
    thread_handle = (pthread_t*) malloc (number_of_cpu*sizeof(pthread_t));
    pthread_mutex_init(&mutex, NULL);
    for (thread = 0; thread < number_of_cpu; thread++) {
        pthread_create(&thread_handle[thread], NULL, randtoss, (void*)thread);
    }
    for (thread = 0; thread < number_of_cpu; thread++) {
        pthread_join(thread_handle[thread], NULL);
    }

    pi_estimate = 4*number_in_circle/((double) number_of_tosses);
    printf("%f\n",pi_estimate);
    pthread_mutex_destroy(&mutex);
    free(thread_handle);
    return 0;
}
```
