all:
	gcc Client.c -o Client.bin
	gcc -pthread Server.c -o Server.bin