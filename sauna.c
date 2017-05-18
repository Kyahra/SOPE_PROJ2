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
char  currGender[2];//ainda nao esta decidido o genero de pessoas que podem entrar na sauna

int end =0;
int discarded_requests =0;
int max_requests;

int fdInput;
int fdRejected;
int fdDiscarded;


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

int served_requests =0;
int served_f_requests =0;
int served_m_requests =0;


int checkEntrance(const char * request_gender, int available_seats);
void  *time_update_sauna(void * r);
void  *discarded_handler(void * null);
void printStats();
void terminate();


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


  request_list = malloc(sizeof(struct Request) * num_seats);//array com os lugares da sauna (ocupados ou livres)
  struct Request null_request;//equivalente a uma vaga livre na sauna

  null_request.duration = 0;

  int i = 0;
  for(; i < num_seats; i++) {
    request_list[i] = null_request;
  }

  mkfifo("/tmp/entrada",0660);
  fdInput=open("/tmp/entrada",O_RDONLY);

  fdRejected = open("/tmp/rejeitados", O_WRONLY  | O_APPEND);

  if(fdRejected < 0){
    perror("/tmp/rejeitados");
    exit(2);
  }


  pthread_t discarded_handler_tid;
  pthread_create(&discarded_handler_tid, NULL, discarded_handler,NULL);

  char str[100];

  putchar('\n');

  readLine(fdInput,str);
  max_requests = atoi(str);


  while(!end){//este while acaba quando o programa "gerador.c" fechar o fifo em modo de escrita (/tmp/entrada)

    readLine(fdInput,str);

    struct Request r = getRequest(str);
    writeDescriptor("RECEBIDO", r.serial_number, r.gender, r.duration, init_time, "/tmp/bal.",pthread_self());

    total_requests++;
    if(strcmp(r.gender, "M")==0) total_m_requests++;
    if(strcmp(r.gender, "F")==0) total_f_requests++;

    if(checkEntrance(r.gender,available_seats)){

      int i = 0;
      for(; i < num_seats ; i++){//procurar por um lugar vago na sauna
        if(request_list[i].duration ==  0)
          request_list[i] = r;
      }

      sem_wait(&semaphore2);
      available_seats--;
      sem_post(&semaphore2);


      pthread_t handler_tid;
      pthread_create(&handler_tid, NULL, time_update_sauna,&r);//thread equivalente a um pedido aceite pela sauna


    } else {//pedido do genero oposto ao que se encontra na sauna
      writeDescriptor("REJEITADO", r.serial_number, r.gender, r.duration, init_time, "/tmp/bal.",pthread_self());
      sendBackRequest(fdRejected, r.serial_number, r.gender, r.duration, "/tmp/rejeitados");

      rejected_requests++;
      if(strcmp(r.gender, "M")==0) rejected_m_requests++;
      if(strcmp(r.gender, "F")==0) rejected_f_requests++;

    }



}

  return 0;
}


void  *time_update_sauna(void * r){


  struct Request r_copy = *((struct Request *) r);


  sem_wait(&semaphore);

  usleep(r_copy.duration*1000);

  sem_wait(&semaphore2);
  available_seats++;

  int i =0;
  for(; i<num_seats; i++){

    if(request_list[i].serial_number == r_copy.serial_number)
      request_list[i].duration =0;//lugar passa a estar livre

  }


  writeDescriptor("SERVIDO", r_copy.serial_number, r_copy.gender, r_copy.duration, init_time, "/tmp/bal.",  pthread_self());

  served_requests++;

  if(discarded_requests + served_requests == max_requests)
    terminate();



  if(strcmp(r_copy.gender, "M")==0) served_m_requests++;
  if(strcmp(r_copy.gender, "F")==0) served_f_requests++;


  sem_post(&semaphore2);
  sem_post(&semaphore);

  return 0;

}


void  *discarded_handler(void * null){
    char str[100];

    mkfifo("/tmp/descartados",0660);
    fdDiscarded = open("/tmp/descartados", O_RDONLY);

    if(fdDiscarded < 0){
      perror("/tmp/descartados");
      exit(2);
    }


    while(readLine(fdDiscarded,str)){
      discarded_requests++;

      if(discarded_requests + served_requests == max_requests)
         terminate();

    }

    close(fdDiscarded);

    return NULL;


}

int checkEntrance(const char * request_gender, int available_seats){
  if(available_seats == num_seats){
    strcpy(currGender,request_gender);

      return 1;
  }

  if(strcmp(request_gender, currGender) == 0) {
    return 1;
  }

    return 0;

}

void printStats(){

  printf("\n  RECEIVED REQUESTS\n");
  printf("    Total: %d\n", total_requests);
  printf("    Female: %d\n", total_f_requests);
  printf("    Male: %d\n", total_m_requests);

  printf("\n  REJECTED REQUESTS\n");
  printf("    Total: %d\n", rejected_requests);
  printf("    Female: %d\n", rejected_f_requests);
  printf("    Male: %d\n", rejected_m_requests);


  printf("\n  SERVED REQUESTS\n");
  printf("    Total: %d\n", served_requests);
  printf("    Female: %d\n", served_f_requests);
  printf("    Male: %d\n\n", served_m_requests);

}


void terminate(){

    close(fdInput);
    close(fdRejected);
    close(fdDiscarded);

    if(unlink("/tmp/descartados")<0)
      printf("Error when destroying FIFO '/tmp/descartados'\n");

    if(unlink("/tmp/entrada")<0)
        printf("Error when destroying FIFO '/tmp/entrada'\n");

    printStats();

    exit(0);

}
