#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/file.h>
#include<stdlib.h>
#include <pthread.h>
#include <string.h>
#include <semaphore.h>
#include "utils.h"


int num_seats;
int available_seats;
struct Request * request_list;
struct timespec init_time;
/*
* semaphore controla as entradas na sauna;
* semaphore2 controla os conflitos entre as pessoas dentro da sauna
*/
sem_t semaphore, semaphore2;

// STATS

int total_requests =0;
int total_f_requests =0;
int total_m_requests=0;

int rejected_requests =0;
int rejected_f_requests =0;
int rejected_m_requests =0;

int discarded_requests =0;
int discarded_f_requests =0;
int discarded_m_requests =0;

//int readline(int fd,char *str);
//void writeDescriptor(char *type, int id, char * gender, int dur);
int checkEntrance(char ** sauna_gender, char * request_gender, int available_seats);
void  *time_update_sauna(void * r);
//struct Request getRequest(char * request_str);
//void sendBackRequest(int fd, int id, char * gender, int dur);

int main(int argc, char* argv[]){
  clock_gettime(CLOCK_MONOTONIC_RAW, &init_time);

  if(argc != 2) {
    printf("usage: sauna <n. lugares>\n");
    exit(1);
  }

  num_seats = atoi(argv[1]);
  available_seats = num_seats;
  sem_init(&semaphore, 0, num_seats);
  sem_init(&semaphore2, 0, 1);

  char * currGender = "null";//ainda nao esta decidido o genero de pessoas que podem entrar na sauna
  request_list = malloc(sizeof(struct Request) * num_seats);//array com os lugares da sauna (ocupados ou livres)
  struct Request null_request;//equivalente a uma vaga livre na sauna

  null_request.duration = 0;

  int i = 0;
  for(i; i < num_seats; i++) {
    request_list[i] = null_request;
  }

  mkfifo("/tmp/entrada",0660);
  int fd=open("/tmp/entrada",O_RDONLY);

  int fdDenied = open("/tmp/rejeitados", O_WRONLY  | O_APPEND);
  if(fdDenied < 0){
    perror("/tmp/rejeitados");
    exit(2);
  }

  char str[100];

  putchar('\n');

  pthread_t threads_ids[1000];//array de id das threads "pedidos aceites"
  int count_ids = 0;

  /*
  * leitura de um fifo aberto para escrita, mas atualmente vazio ==> processo que le o fifo fica bloqueado a espera
  * leitura de um fifo nao aberto para escrita e vazio ==> processo que le o fifo retorna 0 (end of file)
  * quando ha bytes no fifo para serem lidos, o read() retorna o numero de bytes lidos
  */



  while(readLine(fd,str)){//este while acaba quando o programa "gerador.c" fechar o fifo em modo de escrita (/tmp/entrada)

    struct Request r = getRequest(str);
    writeDescriptor("PEDIDO", r.serial_number, r.gender, r.duration, init_time, "/tmp/bal.");

    if(checkEntrance(&currGender,r.gender,available_seats)){

      int i = 0;
      for(; i < num_seats ; i++){//procurar por um lugar vago na sauna
        if(request_list[i].duration ==  0)
          request_list[i] = r;
      }

      sem_wait(&semaphore2);
      available_seats--;
      sem_post(&semaphore2);




      int rc;
      pthread_t handler_tid;
      pthread_t sauna_tid = pthread_self();
      void * ret;

      rc = pthread_create(&handler_tid, NULL, time_update_sauna,&r);//thread equivalente a um pedido aceite pela sauna

      threads_ids[count_ids] = handler_tid;//id do thread "pedido aceite" guardado
      count_ids++;

    } else {//pedido do genero oposto ao que se encontra na sauna
      writeDescriptor("REJEITADO", r.serial_number, r.gender, r.duration, init_time, "/tmp/bal.");
      sendBackRequest(fdDenied, r.serial_number, r.gender, r.duration, "/tmp/rejeitados");
    }

}

  close(fdDenied);

  int j = 0;
  for(j; j <= count_ids; j++) {
    pthread_join(threads_ids[j], NULL);//esperar pelos pedidos aceites que ainda nao foram servido ate ao fim
  }

  close(fd);

  return 0;
}


void  *time_update_sauna(void * r){

  struct Request r_copy = *((struct Request *) r);


  sem_wait(&semaphore);

  usleep(r_copy.duration*1000);

  sem_wait(&semaphore2);
  available_seats++;

  int i =0;
  for(i; i<num_seats; i++){

    if(request_list[i].serial_number == r_copy.serial_number)
      request_list[i].duration =0;//lugar passa a estar livre
  }
  writeDescriptor("SERVIDO", r_copy.serial_number, r_copy.gender, r_copy.duration, init_time, "/tmp/bal.");
  sem_post(&semaphore2);
  sem_post(&semaphore);


}

int checkEntrance(char ** sauna_gender, char * request_gender, int available_seats){
  if(available_seats == num_seats){//sauna vazia
    if(strcmp(request_gender, " F ")) {
      *sauna_gender = "F";
    } else {
      *sauna_gender = "M";
    }
    return 1;
  }
  if(strcmp(request_gender, *sauna_gender) == 0) {
    return 1;
  }

    return 0;

}
