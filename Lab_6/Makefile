.SILENT:
all: pipe.out fifo.out

pipe.out: pipe.c
	gcc pipe.c -o mypipe

fifo.out: fifo.c
	gcc fifo.c -o myfifo

clean:
	rm -f mypipe
	rm -f myfifo