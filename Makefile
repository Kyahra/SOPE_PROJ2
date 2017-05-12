all: sauna.c gerador.c
	gcc -o ./bin/sauna sauna.c -Wall -lpthread -lrt
	gcc -o ./bin/sauna sauna.c -Wall -lpthread -lrt
sauna:
	gcc -o ./bin/sauna sauna.c -Wall -lpthread -lrt
gerador:
	gcc -o ./bin/sauna sauna.c -Wall -lpthread -lrt

clean:
	rm -f /tmp/entrada /tmp/rejeitados

cleanLogs:
    rm -f /tmp/bal.* /tmp/ger.*
