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

struct request{
  double duration;
  char gender;
  int serial_number;
};


void sendRequest(int fd, int max_requests, int max_duration);
void *denied_request_handler(void * null);

int main(int argc, char* argv[]){

  clock_gettime(CLOCK_MONOTONIC_RAW, &init_time);

  if(argc != 3) {
    printf("usage: gerador <n. pedidos> <max. utilização>\n");
    exit(1);
  }

  int max_requests = atoi(argv[1]);
  int max_duration = atoi(argv[2]);

  denied_requests = (int *) malloc(sizeof(int)* max_requests);

  int fd = open("/tmp/entrada", O_WRONLY  | O_APPEND);

  if(fd< 0){
    perror("/tmp/entrada");
    exit(2);
  }


    int rc;
    pthread_t handler_tid;
    pthread_t generator_tid = pthread_self();

    //rc = pthread_create(&handler_tid, NULL, denied_request_handler,&generator_tid);

  sendRequest(fd, max_requests, max_duration);

  close(fd);

  return 0;
}



void sendRequest(int fd, int max_requests, int max_duration){

    srand(time(NULL));

    int i = 0;

    for(i; i < max_requests; i++){

      char request[100];
      char duration[15];
      char serial_number[10];

      int r = rand() % 2;
      sprintf(request, "%d", i);
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

    sprintf(serial_number, "%d", i);

      write(fd, "\n", 1);
      writeDescriptor("PEDIDO", i, gend, rd, init_time, "/tmp/ger.");
      sleep(2);
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
