#include <stdio.h>
#include <stdlib.h>
#include <wait.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <semaphore.h>
#include "utils.h"

#define BUFFER_SIZE 512
#define ID_MAX_SIZE 10


int * count_rejection;
struct timespec init_time;

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

int processedRequests = 0; //ou serao servidos ou foram descartados
sem_t mutex;



char * sendRequest(int fd, int max_requests, int max_duration, int id);
void *denied_request_handler(void * null);
void *getServedRequest(void * arg);
void printStats();

int main(int argc, char* argv[]){

  clock_gettime(CLOCK_MONOTONIC_RAW, &init_time);
  sem_init(&mutex, 0, 1);

  if(argc != 3) {
    printf("usage: gerador <n. pedidos> <max. utilização>\n");
    exit(1);
  }

  int max_requests = atoi(argv[1]);
  int max_duration = atoi(argv[2]);

  pthread_t auxThread;


  count_rejection = (int *) malloc(sizeof(int)* max_requests);
  memset(count_rejection, 0, sizeof count_rejection); //comecar array todo a zero

  int fd = open("/tmp/entrada", O_WRONLY  | O_APPEND);

  if(fd< 0){
    perror("/tmp/entrada");
    exit(2);
  }


  int rc;
  pthread_t handler_tid;
  pthread_t generator_tid = pthread_self();

  rc = pthread_create(&handler_tid, NULL, denied_request_handler,&fd);

  int i = 0;

  for(i; i < max_requests; i++){
    char* gend;

    gend = sendRequest(fd, max_requests, max_duration,i);

    total_requests++;
    if(gend == "M") total_m_requests++;
    if(gend == "F") total_f_requests++;



  }


  close(fd);

  printStats();

  return 0;
}

void printStats(){

  printf("Generated requests: %d\n", total_requests);
  printf("Female generated requests: %d\n", total_f_requests);
  printf("Male generated requests: %d\n", total_m_requests);

  printf("Rejected requests: %d\n", rejected_requests);
  printf("Female rejected requests: %d\n", rejected_f_requests);
  printf("Male rejected requests: %d\n", rejected_m_requests);

  printf("Discarded requests: %d\n", discarded_requests);
  printf("Female discarded requests: %d\n", discarded_f_requests);
  printf("Male discarded requests: %d\n", discarded_m_requests);

}



char * sendRequest(int fd, int max_requests, int max_duration, int id){


  srand(time(NULL));
  char request[100];
  char duration[15];

  int r = rand() % 2;
  sprintf(request, "%d", id);
  char * gend;
  if(r == 0) {
    strcat(request, " M ");
    gend = "M";
  }
  else {
    strcat(request, " F ");
    gend = "F";
  }

  int rd = rand() % max_duration;

  sprintf(duration, "%d", rd);
  strcat(request, duration);


  if(write(fd, request, strlen(request)+1) != strlen(request)+1){
    perror("/tmp/entrada");
    exit(3);
  }

  write(fd, "\n", 1);
  writeDescriptor("PEDIDO", id, gend, rd, init_time, "/tmp/ger.");

  sleep(2);

  return gend;
}

void *denied_request_handler(void * arg){
  mkfifo("/tmp/rejeitados",0660);
  int fdDenied = open("/tmp/rejeitados",O_RDONLY);

  if(fdDenied < 0){
    perror("/tmp/rejeitados");
    exit(3);
  }
  //TODO: rejeitar pedidos

  char str[100];
  while(readLine(fdDenied,str)) {
    struct Request r = getRequest(str);
    count_rejection[r.serial_number]++;
    if(count_rejection[r.serial_number] > 2) {//pedido sera descartado
      sem_wait(&mutex);
      processedRequests++; //incrementar tambem quando um pedido e rejeitado pela terceira vez
      sem_post(&mutex);
    } else {
      sendBackRequest(*((int *)arg), r.serial_number, r.gender, r.duration, "/tmp/entrada");
      writeDescriptor("PEDIDO", r.serial_number, r.gender, r.duration, init_time, "/tmp/ger.");
    }
  }
  close(fdDenied);
}
