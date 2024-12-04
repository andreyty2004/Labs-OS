.SILENT:
all: transmitter.out receiver.out

transmitter.out: transmitter.c
	gcc transmitter.c -o transmitter

receiver.out: receiver.c
	gcc receiver.c -o receiver

clean:
	rm -f transmitter
	rm -f receiver