#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/ioctl.h>

void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues
void encription(char *, char *, int, int);
int recvBuffer(int , char *, int);
int sendBuffer(int, char *, int);

int main(int argc, char *argv[])
{
	int listenSocketFD, establishedConnectionFD, portNumber, charsRead;
	socklen_t sizeOfClientInfo;
	struct sockaddr_in serverAddress, clientAddress;

	if (argc < 2) { fprintf(stderr,"USAGE: %s port\n", argv[0]); exit(1); } // Check usage & args

	// Set up the address struct for this process (the server)
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process

	// Set up the socket
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (listenSocketFD < 0) error("ERROR opening socket");

	// Enable the socket to begin listening
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to port
		error("ERROR on binding");

  listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections
  // Accept a connection, blocking if one is not available until one connects

  int childExitMethod;
  pid_t childPid;
  int numChildren = 0;
  char messageBuffer[100000];
  char keyBuffer[100000];

  while(1)
  {

    if (numChildren < 5)
    {
      childPid = fork();
      switch(childPid){

        case -1:
        {
          printf("Could not create child process\n");
        }
        case 0:
        {
					memset(messageBuffer, '\0', sizeof(messageBuffer));
					memset(keyBuffer, '\0', sizeof(keyBuffer));
          sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
          establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
          if (establishedConnectionFD < 0) error("ERROR on accept");
          // Get the message from the client and display it
          int numInMessage;
          recv(establishedConnectionFD, &numInMessage, sizeof(numInMessage), 0);

					int totalRecivedMessage;
					totalRecivedMessage = recvBuffer(establishedConnectionFD, messageBuffer, numInMessage);
					int totalRecivedKey;
					totalRecivedKey = recvBuffer(establishedConnectionFD, keyBuffer, numInMessage);
					//printf("Recv(): %d %d\n", totalRecivedMessage, totalRecivedKey);

					// printf("%d\n", numInMessage);
					//

					// charsRead = recv(establishedConnectionFD, messageBuffer, 1000, 0); // Read the client's message from the socket
					//
					//
					//
          // charsRead = recv(establishedConnectionFD, keyBuffer, 1000, 0); //Read the clients key from the socket
					//
          // if (charsRead < 0) error("ERROR reading from socket");

          encription(messageBuffer, keyBuffer, establishedConnectionFD, numInMessage);

        }
        default:
        {
          numChildren++;
        }
      }
    }
    else{
      waitpid(0, &childExitMethod, 0);
      numChildren = numChildren - 1;
    }

  }
	close(establishedConnectionFD); // Close the existing socket which is connected to the client
	close(listenSocketFD); // Close the listening socket
	return 0;
}

void encription(char * messageBuffer, char * keyBuffer, int establishedConnectionFD, int numInMessage){

	int i, j, charsWritten;
  char alfaArr[27] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
	int keyVal[strlen(keyBuffer)];
  int cipherVal[strlen(messageBuffer)];
  char ciphertext[strlen(messageBuffer)];
	int messageVal[strlen(messageBuffer)];

	memset(ciphertext, '\0', sizeof(ciphertext));
	memset(messageVal, '\0', sizeof(messageVal));
	memset(cipherVal, '\0', sizeof(cipherVal));
	memset(keyVal, '\0', sizeof(keyVal));


  for (i = 0; i < numInMessage; i++)
  {
    for (j = 0; j < 27; j++)
    {
      if (alfaArr[j] == messageBuffer[i])
      {
        messageVal[i] = j;
      }
      if (alfaArr[j] == keyBuffer[i])
      {
        keyVal[i] = j;
      }
    }
  }


   for (i = 0; i < numInMessage; i++)
   {
     cipherVal[i] = (messageVal[i] - keyVal[i]);
		 if (cipherVal[i] < 0){
			 cipherVal[i] = cipherVal[i] + 27;
		 }
		 cipherVal[i] = cipherVal[i] % 27;
   }

   for (i = 0; i < numInMessage; i++)
   {
     ciphertext[i] = alfaArr[cipherVal[i]];
   }

	int ciphLength = strlen(ciphertext);
	send(establishedConnectionFD, &ciphLength, sizeof(ciphLength), 0);

	int sent;
  sent = sendBuffer(establishedConnectionFD, ciphertext, strlen(ciphertext));
	//printf("sent server: %d\n", sent);

}

int sendBuffer(int establishedConnectionFD, char * buffer, int length){
	int sent = 0;
	int left = length;
	int charsWritten;

	while (sent < length)
	{
		charsWritten = send(establishedConnectionFD, buffer+sent, left, 0);
		if (charsWritten == -1){
			exit(1);
		}
		sent += charsWritten;
		left -= charsWritten;
	}
	length = sent;
	return length;
}



int recvBuffer(int establishedConnectionFD, char *buffer, int length){
	char tmpBuffer[length];
	int numRecv = 0;
	int charsRead;
	int left = length;

	while(numRecv < length){

		charsRead = recv(establishedConnectionFD, tmpBuffer+numRecv, left, 0);
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
