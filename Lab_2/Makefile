.SILENT:
all: mycat.out mygrep.out

mycat.out: mainCat.c
	gcc mainCat.c -o mycat

mygrep.out: mainGrep.c
	gcc mainGrep.c -o mygrep

clean:
	rm -f mycat
	rm -f mygrep