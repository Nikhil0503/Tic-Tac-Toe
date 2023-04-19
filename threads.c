#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

int x = 0;
pthread_mutex_t mutex;

void* ajay(void *arg){
    printf("Testing from Ajay\n");
    sleep(3);
    for (int i = 0; i < 1000000; i++)
    {
        
    }
}

int main(int argc, char *argv[])
{
    pthread_t thread1, thread2, thread3;

    pthread_mutex_init(&mutex, NULL);

    if (pthread_create(&thread1, NULL, &ajay, NULL) != 0){
        printf("Failed to create a thread.\n");
        return 1;
        //The moment a thread is failed to create, should we just return
            //In the game, yea return a message to the client.
    }

    if (pthread_create(&thread2, NULL, &ajay, NULL) != 0){
        printf("Failed to create a thread.\n");
        return 1;
        //The moment a thread is failed to create, should we just return
        //In the game, yea return a message to the client.
    }

    if (pthread_create(&thread3, NULL, &ajay, NULL) != 0){
        printf("Failed to create a thread.\n");
        return 1;
        //The moment a thread is failed to create, should we just return
        //In the game, yea return a message to the client.
    }

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    pthread_join(thread3, NULL);

    printf("Final: %d\n", x);
    return EXIT_SUCCESS;
}