#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/file.h>
#include<stdlib.h>
#include <pthread.h>
#include <string.h>

int readline(int fd,char *str);
void writeDescriptor(char *type, int id, char * gender, int dur);
void sigkill_handler(int signo);


struct Request{
  int duration;
  char *gender;
  int serial_number;
};

void  *time_update_sauna(void * r){

  struct Request r_copy = *((struct Request *) r);

  sleep(r_copy.duration);

  void * ret = malloc(sizeof(int));
  *(int*)ret = r_copy.serial_number;
  return ret;

}

struct Request getRequest(char * request_str);


int main(int argc, char* argv[]){
  int   fd;
  char gender;

  if(argc != 2) {
    printf("usage: sauna <n. lugares>\n");
    exit(1);
  }

  int num_places = atoi(argv[1]);

  mkfifo("/tmp/entrada",0660);
  fd=open("/tmp/entrada",O_RDONLY);

  char str[100];

  putchar('\n');
  while(readline(fd,str)){
    printf("%s",str);

    struct Request r = getRequest(str);
    writeDescriptor("PEDIDO", r.serial_number, r.gender, r.duration);

    int pid;

    pid = fork();

    if(pid > 0){

      // algortimo para verficar se é possivel colocar o pedido no array

      int rc;
      pthread_t handler_tid;
      pthread_t sauna_tid = pthread_self();
      void * ret;

      rc = pthread_create(&handler_tid, NULL, time_update_sauna,&r);
      pthread_join(handler_tid, &ret);

      // tirar pedido da sauna


      free(ret);

    }

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

void writeDescriptor(char *type, int id, char * gender, int dur){

   FILE * fp;
   char file_name[100];


   time_t rawtime;
   struct tm * timeinfo;
   time(& rawtime);
   timeinfo = localtime( &rawtime);

   sprintf(file_name, "/tmp/bal.%d", (int) getpid());

  fp = fopen (file_name, "a+");
  fprintf(fp, "%d:%d:%d - %d - %d: %s - %d - %s\n",timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec,
  getpid(),id,gender,dur,type);

  fclose(fp);

}

void sigkill_handler(int signo);
{
  writeDescriptor("SERVIDO")
}


int readline(int fd,char *str)
{
  int n;
  do{
    n = read(fd,str,1);
  }while (n>0 && *str++ !='\0');

  return (n>0);
}
