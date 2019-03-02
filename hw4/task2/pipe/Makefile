host: pipe.c
	gcc -o pipe pipe.c -lpthread -lrt -I.

bbg: pipe.c
	arm-linux-gcc -o pipe pipe.c -lpthread -lrt -I.

.PHONY: clean

clean:
	rm -f pipe
