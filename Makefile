all:
	gcc client.c -o client.bin
	gcc -lpthread server.c  -o server.bin
threads:
	gcc client.c -o client.bin
	gcc -lpthread server.c -DTHREAD -o server.bin
clean:
	rm -f client.bin
	rm -f server.bin