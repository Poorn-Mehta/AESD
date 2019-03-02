host1: posix1.c
	gcc -o posix1 posix1.c -lpthread -lrt -I.

host2: posix2.c
	gcc -o posix2 posix2.c -lpthread -lrt -I.

bbg1: posix1.c
	arm-linux-gcc -o posix1 posix1.c -lpthread -lrt -I.

bbg2: posix2.c
	arm-linux-gcc -o posix2 posix2.c -lpthread -lrt -I.
