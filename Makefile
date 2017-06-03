np = -np
CC = gcc
CFLAGS = -std=c99

MPICC = mpicc
dest = e0011845@tembusu2.comp.nus.edu.sg:./
dest2 = e0011845@xcnc1.comp.nus.edu.sg:./othellox
params = initialbrd.txt evalparams.txt
subfiles = board.c util.c solver.c board.h util.h solver.h

all: clean othelloxMPI

MPI: clean othelloxMPI runM

othelloxMPI: othelloxMPI.c
	$(CC) $(CFLAGS) -c board.c util.c solver.c -I./ && $(MPICC) $(CFLAGS) -o othellox othelloxMPI.c -I./ board.o util.o solver.o

runM: othelloxMPI
	mpirun $(np) $(NP) ./othellox $(params)

clean: 
	rm -f othellox othelloxMPI errS.txt errM.txt *.o

tembusu: zip scp ## Zip and send to tembusu2

zip:
	zip othellox othelloxMPI.c $(params) $(subfiles)

scp:
	scp Makefile othellox.zip $(dest) && rm othellox.zip

scp2:
	scp Makefile othellox.zip $(dest2)

unzip:
	unzip -o othellox.zip && rm othellox.zip

runSS:
	screen -R othellox -d -m make runS 2>errS.txt

runMS:
	screen -R othelloxMPI -d -m make runM 2>errM.txt

runBoth: runSS runMS