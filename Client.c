#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#define PORT 5000
#define BUF_SIZE 256
#define MAX_FILENAME_SIZE 256


int main(int argc, char* argv[])
{
	char filename[MAX_FILENAME_SIZE];
	int bytesReceived = 0, socketId = 0;
	char buffer[BUF_SIZE];
	struct sockaddr_in serverSocket;
	FILE *file;

	memset(buffer, '0', sizeof(buffer));

	printf("Input file name: ");
	scanf("%s", filename);

	/* Create socket*/
	if ((socketId = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("Error : Could not create socket.\n");
		return 1;
	}

	/* Initialize sockaddr_in data structure */
	serverSocket.sin_family = AF_INET;
	serverSocket.sin_port = htons(PORT);
	serverSocket.sin_addr.s_addr = inet_addr("127.0.0.1"); //localhost

	/* Attempt a connection */
	if (connect(socketId, (struct sockaddr_in*)&serverSocket, sizeof(serverSocket)) < 0)
	{
		printf("Error : Connection failed.\n");
		close(socketId);
		return 1;
	}

	/* Send file name */
	if (send(socketId, filename, strlen(filename), 0) < 0)
	{
		printf("Error : Sending file name failed.\n");
		close(socketId);
		return 1;
	}
	printf("Filename send.\n");

	/* Receive data */
	if ((bytesReceived = read(socketId, buffer, BUF_SIZE)) > 0)
	{
		file = fopen(filename, "ab");
		if (file == NULL)
		{
			printf("Error opening file.\n");
			close(socketId);
			return 1;
		}

		printf("Bytes received: %d\n", bytesReceived);
		fwrite(buffer, 1, bytesReceived, file);
	}
	else
	{
		printf("Error : File not found.\n");
		close(socketId);
		return 1;
	}

	while ((bytesReceived = read(socketId, buffer, BUF_SIZE)) > 0)
	{
		printf("Bytes received: %d\n", bytesReceived);
		fwrite(buffer, 1, bytesReceived, file);
	}

	if (bytesReceived < 0)
		printf("Error : Reading file failed.\n");

	printf("File reading finished.\n");
	fclose(file);
	close(socketId);
	return 0;
}