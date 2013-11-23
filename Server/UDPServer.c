#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket() and bind() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>     /* for atoi(), exit(), and rand() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <time.h>       /* for time() */

#define DEBUG 1 /* Boolean to Enable/Disable Debugging Output */
#define STR_SIZE 6 /* Length of String to represent States */
#define TIMEOUT_SEC 1 /* Length of the UDP recvfrom() Timeout (ms) */
#define TIMEOUT_MSEC 0 /* Length of the UDP recvfrom() Timeout (ms) */
#define TRUE 1 /* Boolean TRUE value */
#define FALSE 0 /* Boolean FALSE value */

/* Data Structure to Send through UDP */
struct request {
	char client_ip[16]; /* To hold client IP address in dotted decimal */
	int inc; /* Incarnation number of client */
	int client; /* Client number */
	int req; /* Request number */
	char c; /* Random character client sends to server */
};

/**
 * printError Method
 * 
 * Displays an error message. If flag is set for die, the quit program
 */
void printError(const char *errorMessage, int die) {
	/* External error handling function */
	perror(errorMessage);
	if (die == TRUE) exit(1);
}

/**
 * printRequest Method
 * 
 * Displays the Request Information
 */
void printRequest(struct request* req) {
	printf("\n");
	printf("NEW REQUEST\n");
	printf("===========\n");
	printf("CLIENT IP: %s\n", req->client_ip);
	printf("CLIENT ID: %d\n", req->client);
	printf("REQUEST: %d\n", req->req);
	printf("INCREMENT: %d\n", req->inc);
	printf("CHARACTER: %c\n", req->c);
	printf("\n");
}

//struct request* performRequest() {}

int main(int argc, char* argv[]) {
	int sock; /* Socket */
	struct sockaddr_in echoServAddr; /* Local address */
	struct sockaddr_in clientAddr; /* Client address */
	unsigned int cliAddrLen; /* Length of incoming message */
	unsigned short echoServPort; /* Server port */
	struct request* req; /* Declare Client's Request Structure */
	char clientString[STR_SIZE] = "     \0"; /* Buffer for echo string */
	char* clientIP; /* Declare the Client's IP String Pointer */
	int i; /* Declare Counter for Loops */

	if (argc != 2) {
		/* Test for correct number of parameters */
		fprintf(stderr, "Usage:  %s <UDP SERVER PORT>\n", argv[0]);
		exit(1);
	}

	/* Make Random Code Based on Time */
	srand(time(NULL));

	echoServPort = atoi(argv[1]); /* Local Port */

	/* Create socket for sending/receiving datagrams */
	if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
		printError("socket() failed", TRUE);

	/* Construct local address structure */
	memset(&echoServAddr, 0, sizeof(echoServAddr)); /* Zero out structure */
	echoServAddr.sin_family = AF_INET; /* Internet address family */
	echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
	echoServAddr.sin_port = htons(echoServPort); /* Local port */

	/* Bind to the local address */
	if (bind(sock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
		printError("bind() failed", TRUE);

	/* Run forever */
	for (;;) {
		/* Set the size of the in-out parameter */
		cliAddrLen = sizeof(clientAddr);

		/* Allocate the Structure Memory */
		req = malloc(sizeof *req);

		/* Block until receive message from a client */
		if (recvfrom(sock, req, sizeof(*req), 0, (struct sockaddr *) &clientAddr, &cliAddrLen) < 0)
			printError("recvfrom() failed", TRUE);

		/* Gather Client IP Address */
		clientIP = inet_ntoa(clientAddr.sin_addr);
		
		/* Assign Client IP Address to Structure Char Array */
		for (i = 0; i < strlen(clientIP); i++) {
			req->client_ip[i] = clientIP[i];
		}
		
		printf("\n");
		printf("Handling client %s\n", clientIP);

		switch (rand() % 10) {
			case 0:
				printf("Performing Request...\n");
				
				/* Modify Client String */
				for (i = 4; i > 0; i--) {
					// Loop through characters in String in reverse order
					// assign previous character to current position
					// moves character down an index
					clientString[i] = clientString[i - 1];
				}

				clientString[0] = req->c;

				if (DEBUG) printRequest(req);
				if (DEBUG) printf("CLIENT STRING: %s\n\n", clientString);
				
				printf("Dropping request...\n");
				printf("\n");
				printf("========================================================\n");
				break;
			case 1:
				printf("Dropping request without performing the request...\n");
				printf("\n");
				printf("========================================================\n");
				break;
			default:
				printf("Performing Request...\n");
				
				/* Modify Client String */
				for (i = 4; i > 0; i--) {
					// Loop through characters in String in reverse order
					// assign previous character to current position
					// moves character down an index
					clientString[i] = clientString[i - 1];
				}

				clientString[0] = req->c;

				if (DEBUG) printRequest(req);
				if (DEBUG) printf("CLIENT STRING: %s\n\n", clientString);
				
				/* Send String back to the client */
				if (sendto(sock, clientString, STR_SIZE, 0, (struct sockaddr *) &clientAddr, sizeof(clientAddr)) != STR_SIZE)
					printError("sendto() sent a different number of bytes than expected", TRUE);

				printf("Request Sent...\n");
				printf("\n");
				printf("========================================================\n");
				break;
		}
		
		/* Deallocate the Structure Memory */
		free(req);
	}
	/* NOT REACHED */
}