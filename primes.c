#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

int primes[8] = {2, 3, 5, 7, 11, 13, 17, 19};

void* routine(void* arg){
    sleep(1);
    int* argu = (int *) arg;
    int index = *argu;
    printf("%d\n", primes[index]);
}

int main(int argc, char *argv[])
{
    pthread_t th[8];
    for (int i = 0; i < 8; i++)
    {
        if (pthread_create(&th[i], NULL, &routine, &i) != 0){
            perror("Thread failed\n");
        }
    }

    for (int i = 0; i < 8; i++)
    {
        if (pthread_join(th[i], NULL) != 0){
            perror("Thread failed to stop executing.\n");
        }
    }
    
    return EXIT_SUCCESS;
}
