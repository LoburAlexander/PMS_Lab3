#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#define PORT 5000
#define BUFFER_SIZE 256
#define MAX_FILENAME_SIZE 256


int main(int argc, char* argv[])
{
	char filename[MAX_FILENAME_SIZE];
	unsigned char buffer[BUFFER_SIZE] = { 0 };
	int socketId = 0, clientId = 0, bytesSend = 0, bytesRead = 0;
	struct sockaddr_in serverSocket;
	FILE *file;
	
	if ((socketId = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("Error : Could not create socket.\n");
		return 1;
	}

	printf("Socket retrieve success\n");

	memset(&serverSocket, '0', sizeof(serverSocket));
	serverSocket.sin_family = AF_INET;
	serverSocket.sin_addr.s_addr = INADDR_ANY;
	serverSocket.sin_port = htons(PORT);

	if ((bind(socketId, (struct sockaddr_in*)&serverSocket, sizeof(serverSocket))) < 0)
	{
		printf("Error : Binding socket failed.\n");
		return 1;
	}

	if (listen(socketId, 10) == -1)
	{
		printf("Error : Listening socket failed.\n");
		return -1;
	}

	for (;;)
	{
		clientId = accept(socketId, (struct sockaddr*)NULL, NULL);

		if (clientId < 0)
		{
			printf("Error : Accept client failed.\n");
			continue;
			//return 1;
		}

		bytesRead = read(clientId, filename, sizeof(filename) - 1);
		if (bytesRead < 0)
		{
			printf("Error reading filename.\n");
			continue;
		}

		/* Open the file to transfer */
		filename[bytesRead] = 0;
		file = fopen(filename, "rb");
		if (file == NULL)
		{
			printf("File open error.\n");
			return 1;
		}

		/* Read file and send it */
		for (;;)
		{
			bytesRead = fread(buffer, 1, BUFFER_SIZE, file);
			printf("Bytes read: %d\n", bytesRead);

			/* If read was success, send data. */
			if (bytesRead > 0)
			{
				printf("Sending \n");
				bytesSend = write(clientId, buffer, bytesRead); //clientId?

				if (bytesSend < bytesRead)
					printf("Error sending file.\n");
			}

			if (bytesRead < BUFFER_SIZE)
			{
				if (feof(file))
					printf("File send.\n");
				if (ferror(file))
					printf("Error reading from file.\n");
				break;
			}
		}
		close(clientId); // clientId?
		sleep(1);
	}

	return 0;
}