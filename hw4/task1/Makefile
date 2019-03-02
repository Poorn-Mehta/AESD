host: problem1.c
	gcc -o problem1 problem1.c -lpthread -lrt -I.

bbg: problem1.c
	arm-linux-gcc -o problem1 problem1.c -lpthread -lrt -I.

.PHONY: clean

clean:
	rm -f problem1