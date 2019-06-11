#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

void error(const char *msg) { perror(msg); exit(0); } // Error function used for reporting issues
int sendBuffer(int, char *, int);
int recvBuffer(int, char *, int);

int main(int argc, char *argv[])
{
	int socketFD, portNumber, charsWritten, charsRead;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char messageBuffer[70000];
	char keyBuffer[70000];
	char ciphertext[70000];
	memset(messageBuffer, '\0', sizeof(messageBuffer));
	memset(keyBuffer, '\0', sizeof(keyBuffer));
	memset(ciphertext, '\0', sizeof(ciphertext));
//	char key[70000];

	if (argc < 3) { fprintf(stderr,"USAGE: %s hostname port\n", argv[0]); exit(0); } // Check usage & args

	// Set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[3]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverHostInfo = gethostbyname("localhost"); // Convert the machine name into a special form of address
	if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); }
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address

	// Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) error("CLIENT: ERROR opening socket");

	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to address
		error("CLIENT: ERROR connecting");


	FILE* inputFile = fopen(argv[1], "r");
	if (inputFile < 0 ){printf("error opening file\n"); exit(0);}
	fgets(messageBuffer, sizeof(messageBuffer), inputFile); // Get input from the user, trunc to buffer - 1 chars, leaving \0
	messageBuffer[strcspn(messageBuffer, "\n")] = '\0'; // Remove the trailing \n that fgets adds
	fclose(inputFile);

	FILE* keyFile = fopen(argv[2], "r");
	if (keyFile < 0){printf("error opening file\n"); exit(0);}
	fgets(keyBuffer, sizeof(keyBuffer), keyFile);
	keyBuffer[strcspn(keyBuffer, "\n")] = '\0'; // Remove the trailing \n that fgets adds
	fclose(keyFile);



	char alfaArr[27] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
	int i, j, charCheck;


	for(i = 0; i < strlen(messageBuffer); i++)
	{
		charCheck = 0;
		for (j = 0; j < strlen(alfaArr); j++)
		{
			if (messageBuffer[i] == alfaArr[j])
			{
				charCheck = 1;
			}
		}
		if (charCheck != 1)
		{
				fprintf(stderr, "Bad File\n");
				exit(1);
		}
	}

	if(strlen(keyBuffer) < strlen(messageBuffer))
	{
		fprintf(stderr, "Key is too short\n");
	}


	else
	{
		int length = strlen(messageBuffer);
		int totalSentMessage, totalSentKey;

		send(socketFD, &length, sizeof(length), 0);
		totalSentMessage = sendBuffer(socketFD, messageBuffer, length);

		totalSentKey = sendBuffer(socketFD, keyBuffer, length);

		int numInCiph;
		recv(socketFD, &numInCiph, sizeof(numInCiph), 0);

		int recived;
		recived = recvBuffer(socketFD, ciphertext, numInCiph);


		printf("%s\n", ciphertext);

	}

	close(socketFD); // Close the socket
	return 0;
}

int sendBuffer(int socketFD, char * buffer, int length){
	int sent = 0;
	int left = length;
	int charsWritten;

	while (sent != length)
	{
		charsWritten = send(socketFD, buffer+sent, left, 0);
		if (charsWritten == -1){
			exit(1);
		}
		sent += charsWritten;
		left -= charsWritten;
	}
	length = sent;
	return length;
}

int recvBuffer(int socketFD, char *buffer, int length){
	char tmpBuffer[length];
	int numRecv = 0;
	int charsRead;
	int left = length;

	while(numRecv != length){

		charsRead = recv(socketFD, tmpBuffer+numRecv, left, 0);
		if (charsRead == -1)
		{
			exit(1);
		}

		numRecv += charsRead;
		left -= charsRead;

	}

	memcpy(buffer, tmpBuffer, sizeof(tmpBuffer));
	return numRecv;

}
