#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define BUFFER_IN_SIZE 20000 //Tamanho do buffer inicial
#define BUFFER_CIRC_SIZE 35 //Tamanho do tapete circular
#define BUFFER_OUT_SIZE 20000 //Tamanho do buffer de saida

int tapetecircular_buffer[BUFFER_CIRC_SIZE]; //Array -> tapete circular
int saida_buffer[BUFFER_OUT_SIZE]; //Array -> buffer de saida

pthread_mutex_t buffer_mutex = PTHREAD_MUTEX_INITIALIZER; //Variável mutex com atributos padrão
pthread_cond_t buffer_full_cond = PTHREAD_COND_INITIALIZER; //Variável de condição para sinalisar buffer cheio
pthread_cond_t buffer_empty_cond = PTHREAD_COND_INITIALIZER; //Variável de condição para sinalisar buffer vazio

int producer_count = 0; //Index do tapete para os produtores
int consumer_count = 0; //Index do tapete para os consumidores
int elements_consumed = 0; //Contador de elementos consumidos
int elements_produced = 0; //Contador de elementos produzidos
int elemento = 20000; //Elemento atual

int threads_counter[5] = {0,0,0,0,0}; //Array -> armazenar contador para cada thread

void *producer(void *arg)
{
    int thread_id = *(int *)arg; //id da thread atual

    while(1){
        pthread_mutex_lock(&buffer_mutex);

	//Termina a thread se todos os elementos foram produzidos
        if (elemento > 40000)
        {
            pthread_mutex_unlock(&buffer_mutex);
            break;
        }

	//Espera até que tenha espaço disponivel no tapete circular
        while (((producer_count + 1) % BUFFER_CIRC_SIZE) == consumer_count){
		pthread_cond_wait(&buffer_empty_cond, &buffer_mutex);
        }

	//Copia o elemento atual para o tapete circular
        tapetecircular_buffer[producer_count] = elemento;
	//Incrementa para o proximo elemento
	elemento++;

	//Vai para a proxima posição no tapete circular
        producer_count = (producer_count + 1) % BUFFER_CIRC_SIZE;

	//Incrementa o contador de elementos consumidos da thread atual
	if(threads_counter[0] + threads_counter[1] + threads_counter[2] != 20000){
	        threads_counter[thread_id]++;
	}

	//Incrementa o contador de elementos produzidos
	elements_produced++;

	//Notifica as threads consumirdoras que tem novos elementos no tapete circular
        pthread_cond_signal(&buffer_full_cond);
        pthread_mutex_unlock(&buffer_mutex);
    }

    pthread_exit(NULL);
}

void *consumer(void *arg){

    int thread_id = *(int *)arg; //Id da thread atual

    while (1)
    {

	//Termina a thread se todos os elementos foram consumidos
	if(elements_consumed >= BUFFER_OUT_SIZE){
		pthread_mutex_unlock(&buffer_mutex);
		break;
	}

        pthread_mutex_lock(&buffer_mutex);

	//Espera até que tenha elementos no tapete circular
        while (producer_count == consumer_count)
        {
            pthread_cond_wait(&buffer_full_cond, &buffer_mutex);
        }

	//Copia o elemento do tapete circular para o buffer de saida, subtari 20000 e dá print do elmento e da thread
        saida_buffer[consumer_count] = tapetecircular_buffer[consumer_count]-20000;
        printf("Elemento consumido: %d", saida_buffer[consumer_count]);
	printf(" | por COMILAO_%d\n", thread_id - 3);

	//Vai para a proxima posição do tapete circular
        consumer_count = (consumer_count + 1) % BUFFER_CIRC_SIZE;

	//Incrementa o contador de elementos consumidos da thread atual
	if(threads_counter[3] + threads_counter[4] != 20000){
        	threads_counter[thread_id]++;
	}

	//Incrementa o contador de elementos consumidos
	elements_consumed++;

        // Notify the producer threads that there is space available in the circular buffer
        pthread_cond_signal(&buffer_empty_cond);
        pthread_mutex_unlock(&buffer_mutex);
    }
    pthread_exit(NULL);
}

int main()
{

    //Define as threads
    pthread_t PCOOK_1, PCOOK_2, PCOOK_3, COMILAO_1, COMILAO_2;
    int thread_id1 = 0, thread_id2 = 1, thread_id3 = 2, thread_id4 = 3, thread_id5 = 4;

    //Cria as threads produtoras e consumidoras
    pthread_create(&PCOOK_1, NULL, producer, (void *)&thread_id1);
    pthread_create(&PCOOK_2, NULL, producer, (void *)&thread_id2);
    pthread_create(&PCOOK_3, NULL, producer, (void *)&thread_id3);
    pthread_create(&COMILAO_1, NULL, consumer, (void *)&thread_id4);
    pthread_create(&COMILAO_2, NULL, consumer, (void *)&thread_id5);

    //Espera que as threads produtoras e as consumidoras acabem
    pthread_join(PCOOK_1, NULL);
    pthread_join(PCOOK_2, NULL);
    pthread_join(PCOOK_3, NULL);
    pthread_join(COMILAO_1, NULL);
    pthread_join(COMILAO_2, NULL);

    //Da print dos resultados estatisticos para cada thread
    printf("-----------------------------------------------------\n");
    printf("Quantidade de elementos produzidos pelo PCOOK_1: %d\n", threads_counter[0]);
    printf("Quantidade de elementos produzidos pelo PCOOK_2: %d\n", threads_counter[1]);
    printf("Quantidade de elementos produzidos pelo PCOOK_3: %d\n", threads_counter[2]);
    printf("Quantidade de elementos consumidos pelo COMILAO_1: %d\n", threads_counter[3]);
    printf("Quantidade de elementos consumidos pelo COMILAO_1: %d\n", threads_counter[4]);

    return 0;
}
