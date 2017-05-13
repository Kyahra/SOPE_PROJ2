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
#include "utils.h"

#define BUFFER_SIZE 512
#define ID_MAX_SIZE 10

int * denied_requests;
int fRequestsNum, mRequestsNum;
int fDeniedNum, mDeniedNum;
int fDiscardedRequests, fDiscardedRequests;
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

int processedRequests = 0; //ou servidos ou descartados
sem_t mutex;



char * sendRequest(int fd, int max_requests, int max_duration, int id);
void *denied_request_handler(void * null);
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
  
  pthread_create(&auxThread, NULL, getServedRequest,&max_requests);//thread que vais receber uma nota da sauna quando um pedido e servido
  denied_requests = (int *) malloc(sizeof(int)* max_requests);

  int fd = open("/tmp/entrada", O_WRONLY  | O_APPEND);

  mkfifo("/tmp/rejeitados",0660);
  int fdDenied=open("/tmp/rejeitados",O_RDONLY);
  
  if(fd< 0){
    perror("/tmp/entrada");
    exit(2);
  }


  int rc;
  pthread_t handler_tid;
  pthread_t generator_tid = pthread_self();

  rc = pthread_create(&handler_tid, NULL, denied_request_handler,&generator_tid);

  int i = 0;

  for(i; i < max_requests; i++){
    char* gend;

    gend = sendRequest(fd, max_requests, max_duration,i);

    total_requests++;
    if(gend == " M ") total_m_requests++;
    if(gend == " F ") total_f_requests++;

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
      char serial_number[10];

      int r = rand() % 2;
      sprintf(request, "%d", id);
      char * gend;
      if(r == 0) {
        strcat(request, " M ");
        gend = " M ";
      }
      else {
        strcat(request, " F ");
        gend = " F ";
      }

      int rd = rand() % max_duration;

      sprintf(duration, "%d", rd);
      strcat(request, duration);


    if(write(fd, request, strlen(request)+1) != strlen(request)+1){
      perror("/tmp/entrada");
      exit(3);
    }

    sprintf(serial_number, "%d", id);

      write(fd, "\n", 1);
      writeDescriptor("PEDIDO", id, gend, rd, init_time, "/tmp/ger.");
      sleep(2);


      return gend;

}

void *denied_request_handler(void * null){

}

void *getServedRequest(void * arg) {
  mkfifo("/tmp/aux",0660);
  int fdDenied=open("/tmp/aux",O_RDONLY);
  while(*((int *)arg) != processedRequests) {
     char str[100];
     int trash;
     trash = readLine(fd,str);
     if(trash > 0) {
       sem_wait(&mutex);
       processedRequests++; //incrementar tambem quando um pedido e rejeitado pela terceira vez
       sem_post(&mutex);
     }
  }
}

/*
void *denied_request_handler(void * null)
{

  int fd;
  mkfifo("/tmp/rejeitados",0660);
  fd = open("/tmp/rejeitados", O_RDONLY);

  if(fd){
    perror("/tmp/rejeitados");
    exit(2);
  }

  unsigned char buffer[BUFFER_SIZE];

  while(readLine(fd,buffer)){
    char delimiter[] = " ";
    char * serial_number_str, *gender_str, *duration_str;
    int bufferLength = strlen(buffer);
    char *bufferCopy = (char*) calloc(bufferLength + 1, sizeof(char));//copiar string porque strtok altera a mesma
    strncpy(bufferCopy, buffer, bufferLength);
    int serial_number, duration;

    printf("%d\n", 1);

    /*
    * extrair palavras separadamente da frase

    serial_number_str = strtok (bufferCopy, delimiter);
    serial_number = atoi(serial_number_str);
    gender_str = strtok (NULL, delimiter);
    duration_str = strtok (NULL, delimiter);
    duration = atoi (duration_str);

    writeDescriptor("REJEITADO", serial_number_str, gender_str, duration_str, init_time);
    if(strcmp(gender_str, "F") == 0) {
      fDeniedNum++;
    } else {
      mDeniedNum++;
    }
  printf("%d\n", 2);
    denied_requests[serial_number]++;

    if(denied_requests[serial_number] < 3) {
      //escrever no FIFO /tmp/entrada
      int fd_resend = open("/tmp/entrada", O_WRONLY | O_CREAT | O_APPEND, 0644);
      if(fd_resend == -1) {
        perror("/tmp/entrada");//erro a abrir
        close(fd_resend);
        exit(3); //TODO: mandar sinal para acabar com o programa
      }
      if(write(fd_resend, buffer, bufferLength) != bufferLength) {
        perror("/tmp/entrada");//erro a escrever
        close(fd_resend);
        exit(4);
      }
      close(fd_resend);
  printf("%d\n", 3);
      writeDescriptor("PEDIDO", serial_number_str, gender_str, duration_str, init_time);
    } else if(denied_requests[serial_number] == 3) {
      writeDescriptor("DESCARTADO", serial_number_str, gender_str, duration_str, init_time);
    }
  printf("%d\n", 4);
  }

 return NULL;

}*/
