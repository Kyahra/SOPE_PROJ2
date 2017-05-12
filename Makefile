CC = gcc
CFLAGS = -Wall -lpthread 
RM = rm

all: sauna gerador

sauna:
	$(CC) $(CFLAGS) sauna.c -o bin/sauna

gerador:
	$(CC) $(CFLAGS) gerador.c -o bin/gerador

clean:
	$(RM) bin/*;