//Vicente Martinez Franco
//Alberto Gil Valverde


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>

//Variables glovales
int tam_buffer;//tamaño del buffer
int ultimo_leido;//ultimo numero consumido
int n_numeros;//cantidad de numeros que se deben producir
int *buffer;//puntero al primer elemento del buffer


//Semaforos
sem_t s_variables;//semaforo de exclusion mutua
sem_t s_escribir;//semaforo para evitar que el productor sobreescriba datos
sem_t s_leer;//semaforo para que los consumidores no intenten leer datos no producidos




int isPrimo(int candidato){
	int i,contador=0;
	float j=sqrt(candidato);
	for(i=1;i<j;i++){
		if(candidato%i==0){
			contador++;
		}
	}
	if(contador==1){
		return (true);
	}else{
		return (false);
	}
}

void *escribir(){
	int cont;
	int *pos_escribir;//puntero a la posicion de memoria en la que se va a escribir el dato
	for(cont=0;cont<n_numeros;cont++){
	
		sem_wait(&s_escribir);//espera para no sobreescribir un dato no leido.
		int random=rand() %99999;//un numero aleatorio entre 0 y 99999
		pos_escribir=buffer+cont%tam_buffer;
		*pos_escribir=random;
		sem_post(&s_leer);
	}
	pthread_exit(NULL);
	
}

void *leer(void *num){
	int *n=(int *) num;//numero de hilo
	int *pos_leer;//puntero a la posicion de memoria que se quiere leer
	int leer,n_leido;
	
	while (ultimo_leido<n_numeros){
		sem_wait(&s_variables);//accede a la variable ultimo_leido en exclusion mutua
		sem_wait(&s_leer);
		leer=ultimo_leido;
		ultimo_leido+=1;
		sem_post(&s_variables);
		
		pos_leer=buffer+leer%tam_buffer;
		n_leido=*pos_leer;
		*pos_leer=0;
		if(isPrimo(n_leido)){
			printf("HILO: %d Numero valor: %d Numero leido: %d Es primo\n",*n,leer,n_leido);
			fflush(stdout);
		}
		else{
			printf("HILO: %d Numero valor: %d Numero leido: %d No es primo\n",*n,leer,n_leido);
			fflush(stdout);
		}	
		sem_post(&s_escribir);
	}
	pthread_exit(NULL);
}




int main(int argc, char *argv[]) {
	int n_hilos;
	//comprueba que el numero de agumentos pasado es correcto
	if (argc != 4) {
		printf ("Numero de argumentos pasados incorrecto\n");
		exit(-1);
	}
	n_hilos=atoi(argv[1]);//transforma los argumentos recibidos a enteros
	n_numeros=atoi(argv[2]);
	tam_buffer=atoi(argv[3]);
	int buff[tam_buffer];
	buffer=&buff[0];//apunta el puntero buffer a primer elemento de este
	//comprueba que el tamaño del buffer es correcto
	if (tam_buffer>n_numeros/2){
		printf("tamaño del buffer incorrecto");
		exit (1);
	}
	//inicializacion de los semaforos
	sem_init(&s_escribir,0,tam_buffer);//para que no pueda escribir sin haber leido antes.
	sem_init(&s_leer,0,0);//para que no pueda leer sin que se haya escrito antes.
	sem_init(&s_variables,0,1);//Semaforo binario para que solo un hilo pueda leer del buffer al mismo tiempo.
	
	pthread_t tid[n_hilos];//tid hilos consumidores
	pthread_t tid_e;//tid hilo productor
	
	pthread_create (&tid_e,NULL,&escribir,NULL);//inicializa el productor
	
	int i;
	int num[n_hilos];
	
	for(i=0;i<n_hilos;i++){
		num[i]=i;
	}
	for(i=0;i<n_hilos;i++){
		pthread_create (&tid[i], NULL, &leer, (void *) &num[i]);//inicializa los consumidores
	}
	//espera a que acaben todos los hilos
	pthread_join(tid_e,NULL);
	for(i=0;i<n_hilos;i++){
		pthread_join(tid[i],NULL);
	}
	
	return 0;
}
