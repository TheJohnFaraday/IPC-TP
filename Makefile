all: master slave view

master: master.c
	gcc -Wall $< -o $@ -std=c99 -lm -pthread

slave: slave.c
	gcc -Wall $< -o $@ -std=c99 -lm -pthread

view: view.c
	gcc -Wall $< -o $@ -std=c99 -lm -pthread

clean:
	rm -f master slave view

.PHONY: all clean