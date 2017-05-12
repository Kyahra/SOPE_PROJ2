#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/file.h>
#include<stdlib.h>
#include <pthread.h>
#include <string.h>
#include <semaphore.h>
#include <time.h>
#include "utils.h"


int checkEntrance(char * sauna_gender, char * request_gender, int available_seats);
void  *time_update_sauna(void * r);
struct Request getRequest(char * request_str);
struct timespec init_time;



struct Request{
  int duration;
  char *gender;
  int serial_number;
};

int num_seats;
int available_seats;
struct Request * request_list;
/*
* semaphore controla as entradas na sauna;
* semaphore2 controla os conflitos entre as pessoas dentro da sauna
*/
sem_t semaphore, semaphore2;

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

  int  fd;
  char gender ='0';
  request_list = (struct Request *) malloc(sizeof(struct Request)* num_seats);
  struct Request null_request;

  null_request.duration = 0;

  int i = 0;
  for(i; i < num_seats; i++) {
    request_list[i] = null_request;
  }

  mkfifo("/tmp/entrada",0660);
  fd=open("/tmp/entrada",O_RDONLY);

  char str[100];

  putchar('\n');
  pthread_t threads_ids[1000];
  int count_ids = 0;
  while(readline(fd,str)){


    struct Request r = getRequest(str);
    writeDescriptor("PEDIDO", r.serial_number, r.gender, r.duration,init_time, "/tmp/bal.");

    if(checkEntrance(&gender,r.gender,available_seats)){
      int i = 0;
      for(; i < num_seats ; i++){
        if(request_list[i].duration ==  0)
        request_list[i] = r;
      }


      int rc;
      pthread_t handler_tid;
      pthread_t sauna_tid = pthread_self();
      void * ret;
      rc = pthread_create(&handler_tid, NULL, time_update_sauna,&r);
      threads_ids[count_ids] = handler_tid;
      count_ids++;
      
    } else {
      writeDescriptor("REJEITADO", r.serial_number, r.gender, r.duration);
    }
    
       

}

int j = 0;
    for(j; j <= count_ids; j++) {
      pthread_join(threads_ids[j], NULL);
    }

  close(fd);

  return 0;
}

struct Request getRequest(char * request_str){

   const char delimiter[2] = " ";
   char *word;
   struct Request r;

   word = strtok(request_str, delimiter);
   r.serial_number = atoi(word);

   word = strtok(NULL, delimiter);
   r.gender = word;

    word = strtok(NULL, delimiter);
    r.duration = atoi(word);

     return r;

}



void  *time_update_sauna(void * r){

  struct Request r_copy = *((struct Request *) r);
  printf("entrou no thread\n");

  sem_wait(&semaphore);
  printf("passou o wait\n");
  usleep(r_copy.duration*1000);

  sem_wait(&semaphore2);
  available_seats++;

  int i =0;
  for(i; i<num_seats; i++){

    if(request_list[i].serial_number == r_copy.serial_number)
      request_list[i].duration =0;
  }
  writeDescriptor("SERVIDO", r_copy.serial_number, r_copy.gender, r_copy.duration,init_time, "/tmp/bal.");
  sem_post(&semaphore2);
  sem_post(&semaphore);
  printf("saiu do wait\n");

}

int checkEntrance(char * sauna_gender, char * request_gender, int available_seats){
  //printf("genero atual na sauna: %c \n", (*sauna_gender));
  if(available_seats == num_seats){
    sauna_gender = request_gender;
    return 1;
  }

  if((*request_gender) == (*sauna_gender)) {
    //printf("este pedido e do genero %c \n", (*request_gender));
    return 1;
  }
   
    return 0;
  
  

}
