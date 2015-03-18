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
	int bytes_received = 0, socket_id = 0;
	char buffer[BUF_SIZE];
	struct sockaddr_in server_socket;
	FILE *file;

	memset(buffer, '0', sizeof(buffer));
	
	printf("Input file name: ");
	scanf("%s", filename);

	/* Create socket*/
	if ((socket_id = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Error : Could not create socket.\n");
		return 1;
	}

	/* Initialize sockaddr_in data structure */
	server_socket.sin_family = AF_INET;
	server_socket.sin_port = htons(PORT); 
	server_socket.sin_addr.s_addr = inet_addr("127.0.0.1"); //localhost

	/* Attempt a connection */
	if (connect(socket_id, (struct sockaddr_in*)&server_socket, sizeof(server_socket)) < 0) {
		printf("Error : Connection failed.\n");
		close(socket_id);
		return 1;
	}

	/* Send file name */
	if (send(socket_id, filename, strlen(filename), 0) < 0) {
		printf("Error : Sending file name failed.\n");
		close(socket_id);
		return 1;
	}
	printf("Filename send.\n");

	/* Receive data */
	if ((bytes_received = read(socket_id, buffer, BUF_SIZE)) > 0) {
		file = fopen(filename, "wb");
		if (file == NULL) {
			printf("Error opening file.\n");
			close(socket_id);
			return 1;
		}
		
		fwrite(buffer, 1, bytes_received, file);
	} else {
		printf("Error : File not found.\n");
		close(socket_id);
		return 1;
	}
	
	while ((bytes_received = read(socket_id, buffer, BUF_SIZE)) > 0)
		fwrite(buffer, 1, bytes_received, file);

	if (bytes_received < 0)
		printf("Error : Reading file failed.\n");
	
	printf("File reading finished.\n");
	fclose(file);
	close(socket_id);
	return 0;
}

