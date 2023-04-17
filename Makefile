CFLAGS = -D_XOPEN_SOURCE=500 -Wall -std=c99 -lrt -pthread
IDIR = ./include
ODIR = ./obj
BDIR = .
SRCDIR = ./src

all: $(BDIR)/master $(BDIR)/view $(BDIR)/slave

$(BDIR)/master: $(ODIR) $(BDIR) $(ODIR)/shmManager.o $(ODIR)/semManager.o $(ODIR)/master.o
	gcc $(ODIR)/master.o $(ODIR)/semManager.o $(ODIR)/shmManager.o $(CFLAGS) -o $(BDIR)/master

$(BDIR)/view: $(ODIR)/view.o $(ODIR)/shmManager.o $(ODIR)/semManager.o
	gcc $(ODIR)/view.o $(ODIR)/semManager.o $(ODIR)/shmManager.o $(CFLAGS) -o $(BDIR)/view

$(BDIR)/slave: $(ODIR)/slave.o 
	gcc $(ODIR)/slave.o $(CFLAGS) -o $(BDIR)/slave

$(ODIR):
	mkdir -p $(ODIR)

$(BDIR):
	mkdir -p $(BDIR)	

$(ODIR)/%.o : $(SRCDIR)/%.c
	gcc -c $(CFLAGS) $< -o $@

clean:
	@rm -rf ./master ./view ./slave
	@rm -rf $(ODIR)
	@rm -rf result

.PHONY: all clean