CC= gcc
CFLAGS= -std=c99 -Wall -g
AUX_FLAGS = -lrt -pthread

all: master slave view

master: master.c
	$(CC) $(CFLAGS) -o master master.c $(AUX_FLAGS)

slave: slave.c
	$(CC) $(CFLAGS) -o slave slave.c 

view: view.c
	$(CC) $(CFLAGS) -o view view.c $(AUX_FLAGS)

clean:
	rm -f master slave view

.PHONY: all clean