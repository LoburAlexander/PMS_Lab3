all:
	gcc Client.c -o client.bin
	gcc -lpthread Server.c  -o server.bin
threads:
	gcc Client.c -o client.bin
	gcc -lpthread Server.c -DTHREAD -o server.bin
clean:
	rm -f client.bin
	rm -f server.bin