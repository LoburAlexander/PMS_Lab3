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

void* send_file(void* param)
{
	char filename[MAX_FILENAME_SIZE];
	unsigned char buffer[BUFFER_SIZE] = { 0 };
	FILE *file;
	int bytes_read = 0, bytes_send = 0;
	int client_id = (int)param;
	

	bytes_read = read(client_id, filename, sizeof(filename)-1);
	if (bytes_read < 0) {
		printf("Error reading filename.\n");
		close(client_id);
		return 1;
	}

	/* Open the file to transfer */
	filename[bytes_read] = 0;
	if (access(filename, F_OK) == -1) {
		printf("File not found.\n");
		close(client_id);
		return 1;
	}

	file = fopen(filename, "rb");
	if (file == NULL) {
		printf("File open error.\n");
		close(client_id);
		return 1;
	}

	/* Read file and send it */
	for (;;) {
		bytes_read = fread(buffer, 1, BUFFER_SIZE, file);

		/* If read was success, send data. */
		if (bytes_read > 0) {
			bytes_send = write(client_id, buffer, bytes_read);

			if (bytes_send < bytes_read) {
				printf("Error sending file.\n");
				fclose(file);
				close(client_id);
				return 1;
			}
			
			printf("Bytes send: %d\n", bytes_send);
		}

		if (bytes_read < BUFFER_SIZE) {
			if (feof(file))
				printf("File send.\n");
			if (ferror(file))
				printf("Error reading from file.\n");
			break;
		}
	}

	fclose(file);
	close(client_id);
	return 0;
}

int main(int argc, char* argv[])
{
	int socket_id = 0, client_id = 0, yes = 1;
	struct sockaddr_in server_socket;

#ifdef THREAD
	pthread_t threads[MAX_THREAD_COUNT] = { NULL };
	int error = 0;
	int i = 0;
#else
	pid_t proc_id = NULL;
#endif


	if ((socket_id = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Error : Could not create socket.\n");
		return 1;
	}

	printf("Socket retrieve success\n");

	memset(&server_socket, '0', sizeof(server_socket));
	server_socket.sin_family = AF_INET;
	server_socket.sin_addr.s_addr = INADDR_ANY;
	server_socket.sin_port = htons(PORT);

	
	if (setsockopt(socket_id, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
    		printf("Error : Binding socket failed.\n");
		return 1;
	}

	if ((bind(socket_id, (struct sockaddr_in*)&server_socket, sizeof(server_socket))) < 0) {
		printf("Error : Binding socket failed.\n");
		return 1;
	}

	if (listen(socket_id, 10) == -1) {
		printf("Error : Listening socket failed.\n");
		return -1;
	}

	for (;;) {
		client_id = accept(socket_id, (struct sockaddr*)NULL, NULL);

		if (client_id < 0) {
			printf("Error : Accept client failed.\n");
			continue;
		}

#ifdef THREAD
		
		for (i = 0; i < MAX_THREAD_COUNT; i++)
			if ((threads[i] == NULL) || (pthread_kill(threads[i], 0) != ESRCH))
				break;

		if (i >= MAX_THREAD_COUNT) {
			printf("Error : There's no free threads.\n");
			continue;
		}

		error = pthread_create(&threads[i], NULL, send_file, (void*)client_id);
		if (error) {
			printf("Error : Thread create failed.\n");
			continue;
		}

#else
		switch (proc_id = fork()) {
		case -1:
			printf("Error : Process create failed.\n");
			break;
		case 0:
			send_file((void*)client_id);
			return 0;
		default:
		  	close(client_id);
			break;
		}
		
#endif
	}

	return 0;
}

