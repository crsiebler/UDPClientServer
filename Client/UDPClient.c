#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), sendto(), and recvfrom() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */

#define DEBUG 1	/* Boolean to Enable/Disable Debugging Output */
#define STR_SIZE 6	/* Length of String to represent States */

struct request {
	char client_ip[16]; /* To hold client IP address in dotted decimal */
	int inc; /* Incarnation number of client */
	int client; /* Client number */
	int req; /* Request number */
	char c; /* Random character client sends to server */
};

void DieWithError(const char *errorMessage) {
	/* External error handling function */
	perror(errorMessage);
	exit(1);
}

struct request* randomRequest() {
	struct request* req = malloc(sizeof *req);
	
	return req;
}

int main(int argc, char *argv[]) {
	int sock; /* Socket descriptor */
	struct sockaddr_in serverAddr; /* Echo server address */
	struct sockaddr_in fromAddr; /* Source address of echo */
	unsigned short serverPort; /* Echo server port */
	unsigned int fromSize; /* In-out of address size for recvfrom() */
	char *servIP; /* IP address of server */
	char echoBuffer[STR_SIZE]; /* Buffer for receiving echoed string */
	int echoStringLen; /* Length of string to echo */
	int respStringLen; /* Length of received response */

	if ((argc < 2) || (argc > 3)) {
		/* Test for correct number of arguments */
		fprintf(stderr, "Usage: %s <Server IP> [<Echo Port>]\n", argv[0]);
		exit(1);
	}

	servIP = argv[1]; /* First arg: server IP address (dotted quad) */

	if (argc == 3) {
		/* Use given port, if any */
		serverPort = atoi(argv[2]);
	} else {
		/* 7 is the well-known port for the echo service */
		serverPort = 7;
	}

	/* Create a datagram/UDP socket */
	if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
		DieWithError("socket() failed");

	/* Construct the server address structure */
	memset(&serverAddr, 0, sizeof(serverAddr)); /* Zero out structure */
	serverAddr.sin_family = AF_INET; /* Internet addr family */
	serverAddr.sin_addr.s_addr = inet_addr(servIP); /* Server IP address */
	serverAddr.sin_port = htons(serverPort); /* Server port */
	
	struct request* req = malloc(sizeof *req);
//	strcpy(req->client_ip, "10.0.1.50\0");
	req->c = 'A';
	req->client = 0;
	req->inc = 0;
	req->req = 0;
	
	/* Send the Structure to the server */
	if (sendto(sock, req, sizeof(*req), 0, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0)
		DieWithError("sendto() sent a different number of bytes than expected");

	/* Recv a response */
//	fromSize = sizeof(fromAddr);
//	if ((respStringLen = recvfrom(sock, echoBuffer, ECHOMAX, 0, (struct sockaddr *) &fromAddr, &fromSize)) != echoStringLen)
//		DieWithError("recvfrom() failed");

//	if (serverAddr.sin_addr.s_addr != fromAddr.sin_addr.s_addr) {
//		fprintf(stderr, "Error: received a packet from unknown source.\n");
//		exit(1);
//	}

	/* null-terminate the received data */
//	echoBuffer[respStringLen] = '\0';
//	printf("Received: %s\n", echoBuffer); /* Print the echoed arg */

	close(sock);
	exit(0);
}