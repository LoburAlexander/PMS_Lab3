#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>

#define PORT 5000
#define BUFFER_SIZE 256
#define MAX_FILENAME_SIZE 256
#define MAX_THREAD_COUNT 5
//#define THREAD

void* sendFile(void* param)
{
	char filename[MAX_FILENAME_SIZE];
	unsigned char buffer[BUFFER_SIZE] = { 0 };
	FILE *file;
	int bytesRead = 0, bytesSend = 0;
	int clientId = (int)param;
	

	bytesRead = read(clientId, filename, sizeof(filename)-1);
	if (bytesRead < 0)
	{
		printf("Error reading filename.\n");
		close(clientId);
		return 1;
	}

	/* Open the file to transfer */
	filename[bytesRead] = 0;
	if (access(filename, F_OK) == -1) 
	{
		printf("File not found.\n");
		close(clientId);
		return 1;
	}

	file = fopen(filename, "rb");
	if (file == NULL)
	{
		printf("File open error.\n");
		close(clientId);
		return 1;
	}

	/* Read file and send it */
	for (;;)
	{
		bytesRead = fread(buffer, 1, BUFFER_SIZE, file);

		/* If read was success, send data. */
		if (bytesRead > 0)
		{
			bytesSend = write(clientId, buffer, bytesRead);

			if (bytesSend < bytesRead)
			{
				printf("Error sending file.\n");
				fclose(file);
				close(clientId);
				return 1;
			}
			
			printf("Bytes send: %d\n", bytesSend);
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

	fclose(file);
	close(clientId);
	return 0;
}

int main(int argc, char* argv[])
{
	int socketId = 0, clientId = 0;
	struct sockaddr_in serverSocket;

#ifdef THREAD
	pthread_t threads[MAX_THREAD_COUNT] = { NULL };
	int error = 0;
	int i = 0;
#else
	pid_t procId = NULL;
#endif


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
		}

#ifdef THREAD
		
		for (i = 0; i < MAX_THREAD_COUNT; i++)
		if ((threads[i] == NULL) || (pthread_kill(threads[i], 0) != ESRCH))
			break;

		if(i >= MAX_THREAD_COUNT)
		{
			printf("Error : There's no free threads.\n");
			continue;
		}

		error = pthread_create(&threads[i], NULL, sendFile, (void*)clientId);
		if (error)
		{
			printf("Error : Thread create failed.\n");
			continue;
		}

#else
		switch(procId = fork()) 
		{
		case -1:
			printf("Error : Process create failed.\n");
			break;
		case 0:
			sendFile((void*)clientId);
			return 0;
		default:
		  //wait();
		  close(clientId);
			break;
		}
		
#endif
	}

	return 0;
}