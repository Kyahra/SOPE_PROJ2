/*#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/file.h>
#include <errno.h>
#include <stdlib.h>


int readline(int fd, char *str){
int n;
do{
n = read(fd,str,1);
}while (n>0 && *str++ != '\0');
return (n>0);
}


int main(void) {
int   fd;
char  str[100];
if(mkfifo("/tmp/entrada",0660)<0)
if(errno==EEXIST) {
printf("FIFO '/tmp/entrada' already exists\n");
exit(1);
}
else  {
printf("Can't create FIFO\n");
exit(2);
}

fd=open("/tmp/entrada",O_RDONLY);

putchar('\n');
while(readline(fd,str)) printf("%s",str);

close(fd);

unlink("/tmp/entrada");
return 0;
}

*/

#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/file.h>

int readline(int fd,char *str);

struct request{
  double duration;
  char gender;
  int serial_number;
};

void  *time_update_sauna(void * null){

}

void readEntry(char str[argv[1]], int line){



}

int main(void){
  int   fd;
  char gender;

  if(argc != 3) {
    printf("usage: sauna <n. lugares> <un. tempo>\n");
    exit(1);
  }
  char  str[argv[1]];
  mkfifo("/tmp/entrada",0660);
  fd=open("/tmp/entrada",O_RDONLY);

  putchar('\n');
  while(readline(fd,str)) printf("%s",str);
  close(fd);

  int rc;
  pthread_t handler_tid;
  pthread_t sauna_tid = pthread_self();

  rc = pthread_create(&handler_tid, NULL, time_update_sauna,&generator_tid);


  return 0;
}

int readline(int fd,char *str)
{
  int n;
  do{
    n = read(fd,str,1);
  }while (n>0 && *str++ !='\0');

  return (n>0);
}
