#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define BUFFER_IN_SIZE 20000
#define BUFFER_CIRC_SIZE 50
#define BUFFER_OUT_SIZE 19998

int buffer_in[BUFFER_IN_SIZE];
int buffer_circ[BUFFER_CIRC_SIZE];
int buffer_out[BUFFER_OUT_SIZE];

pthread_mutex_t buffer_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t buffer_full_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t buffer_empty_cond = PTHREAD_COND_INITIALIZER;

int producer_count = 0;
int consumer_count = 0;

void *producer(void *arg)
{
    int thread_id = *(int *)arg;

    for (int i = 0; i < BUFFER_IN_SIZE; i++)
    {
        pthread_mutex_lock(&buffer_mutex);

        // Espera até que haja espaço disponível no buffer circular
        while (((consumer_count + BUFFER_CIRC_SIZE) % BUFFER_OUT_SIZE) == producer_count)
        {
            pthread_cond_wait(&buffer_empty_cond, &buffer_mutex);
        }

        // Calcula a média dos 3 elementos consecutivos e adiciona ao buffer circular
        int average = (buffer_in[i] + buffer_in[(i + 1) % BUFFER_IN_SIZE] + buffer_in[(i + 2) % BUFFER_IN_SIZE]) / 3;
        buffer_circ[producer_count] = average;

        producer_count = (producer_count + 1) % BUFFER_OUT_SIZE;

        // Notifica a thread consumidora que há novos elementos no buffer circular
        pthread_cond_signal(&buffer_full_cond);
        pthread_mutex_unlock(&buffer_mutex);
    }

    pthread_exit(NULL);
}

void *consumer(void *arg)
{
    while (1)
    {
        pthread_mutex_lock(&buffer_mutex);

        // Espera até que haja elementos no buffer circular
        while (producer_count == consumer_count)
        {
            pthread_cond_wait(&buffer_full_cond, &buffer_mutex);
        }

        // Copia o elemento do buffer circular para o buffer de saída e imprime
        buffer_out[consumer_count] = buffer_circ[consumer_count];
        printf("Elemento consumido: %d\n", buffer_out[consumer_count]);

        consumer_count = (consumer_count + 1) % BUFFER_OUT_SIZE;

        // Notifica as threads produtoras que há espaço disponível no buffer circular
        pthread_cond_signal(&buffer_empty_cond);
        pthread_mutex_unlock(&buffer_mutex);
    }
}

int main()
{
    // Inicializa o buffer de entrada de 1 a 20000
    for (int i = 0; i < BUFFER_IN_SIZE; i++)
    {
        buffer_in[i] = i + 1;
    }

    pthread_t producer_thread1, producer_thread2, consumer_thread;
    int thread_id1 = 1, thread_id2 = 2;

    // Cria as threads produtoras e a thread consumidora
    pthread_create(&producer_thread1, NULL, producer, (void *)&thread_id1);
    pthread_create(&producer_thread2, NULL, producer, (void *)&thread_id2);
    pthread_create(&consumer_thread, NULL, consumer, NULL);

    // Espera pela conclusão das threads produtoras e da thread consumidora
    pthread_join(producer_thread1, NULL);
    pthread_join(producer_thread2, NULL);
    pthread_join(consumer_thread, NULL);

    printf("Quantidade de elementos produzidos por cada thread produtora: %d\n", BUFFER_IN_SIZE);
    printf("Quantidade de elementos consumidos pela thread consumidora: %d\n", BUFFER_OUT_SIZE);

    return 0;
}
