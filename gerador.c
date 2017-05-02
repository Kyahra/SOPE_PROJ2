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

#define BUFFER_SIZE 512
#define ID_MAX_SIZE 10

int * denied_requests;
int fRequestsNum, mRequestsNum;
int fDeniedNum, mDeniedNum;
int fDiscardedRequests, fDiscardedRequests;

struct request{
  double duration;
  char gender;
  int serial_number;
};

int readline(int fd, char *str);
void sendRequest(int fd, int max_requests, int max_duration);
void writeDescriptor(char *type, char * id, char * gender, char * dur);

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

  while(readline(fd,buffer)){
    char delimiter[] = " ";
    char * serial_number_str, *gender_str, *duration_str;
    int bufferLength = strlen(buffer);
    char *bufferCopy = (char*) calloc(bufferLength + 1, sizeof(char));//copiar string porque strtok altera a mesma
    strncpy(bufferCopy, buffer, bufferLength);
    int serial_number, duration;

    printf("%d\n", 1);

    /*
    * extrair palavras separadamente da frase
    */
    serial_number_str = strtok (bufferCopy, delimiter);
    serial_number = atoi(serial_number_str);
    gender_str = strtok (NULL, delimiter);
    duration_str = strtok (NULL, delimiter);
    duration = atoi (duration_str);

    writeDescriptor("REJEITADO", serial_number_str, gender_str, duration_str);
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
      writeDescriptor("PEDIDO", serial_number_str, gender_str, duration_str);
    } else if(denied_requests[serial_number] == 3) {
      writeDescriptor("DESCARTADO", serial_number_str, gender_str, duration_str);
    }
  printf("%d\n", 4);
  }

 return NULL;

}


int main(int argc, char* argv[]){

  if(argc != 4) {
    printf("usage: gerador <n. pedidos> <max. utilização> <un. tempo>\n");
    exit(1);
  }

  int max_requests = atoi(argv[1]);
  int max_duration = atoi(argv[2]);

  denied_requests = (int *) malloc(sizeof(int)* max_requests);

 // O FIFO vai  ter que ser criado em sauna.c
 // correr sauna.c primeiro
 // dá erro ao abrir para escrita


  int fd = open("/tmp/entrada", O_WRONLY | O_NONBLOCK | O_APPEND);

  if(fd< 0){
    perror("/tmp/entrada");
    exit(2);
  }

  sendRequest(fd, max_requests, max_duration);

  close(fd);

  int rc;
  pthread_t handler_tid;
  pthread_t generator_tid = pthread_self();

  rc = pthread_create(&handler_tid, NULL, denied_request_handler,&generator_tid );
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

      int rd = rand() % max_duration + 3600;

      sprintf(duration, "%d", rd);
      strcat(request, duration);


    if(write(fd, request, strlen(request)+1) != strlen(request)+1){
      perror("/tmp/entrada");
      exit(3);
    }

    sprintf(serial_number, "%d", i);

      write(fd, "\n", 1);
      writeDescriptor("PEDIDO", serial_number, gend, duration);
      sleep(2);
    }

}

void writeDescriptor(char *type, char * id, char * gender, char * dur){

   FILE * fp;
   char file_name[100];


   time_t rawtime;
   struct tm * timeinfo;
   time(& rawtime);
   timeinfo = localtime( &rawtime);

   sprintf(file_name, "/tmp/ger.%d", (int) getpid());

  fp = fopen (file_name, "a+");
  fprintf(fp, "%d:%d:%d - %d - %s: %s - %s - %s\n",timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec,
  getpid(),id,gender,dur,type);

  fclose(fp);

}

int readline(int fd, char *str){
  int n;
  do{
      n = read(fd,str,1);
    }while (n>0 && *str++ != '\0');
    return (n>0);
}
