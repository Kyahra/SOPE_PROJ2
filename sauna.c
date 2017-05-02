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
int readline(int fd,char *str);int

 main(void){
   int   fd;
   char  str[100];
      mkfifo("/tmp/entrada",0660);
      fd=open("/tmp/entrada",O_RDONLY);

      putchar('\n');
      while(readline(fd,str)) printf("%s",str);
      close(fd);
      return
      0;
}

int
 readline(int fd,char *str)
{
    int n;
    do{
    n = read(fd,str,1);
      }while (n>0 && *str++ !='\0');
return (n>0);
}
