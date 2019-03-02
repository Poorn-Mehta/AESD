host1: socket1.c
	gcc -o socket1 socket1.c -I.

host2: socket2.c
	gcc -o socket2 socket2.c -I.

bbg1: socket1.c
	arm-linux-gcc -o socket1 socket1.c -I.

bbg2: socket2.c
	arm-linux-gcc -o socket2 socket2.c -I.
