#include <stdio.h>      /* for printf(), fprintf(), and remove() */
#include <sys/socket.h> /* for socket(), connect(), sendto(), and recvfrom() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi(), exit(), and rand() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <time.h>       /* for time() */
#include <fcntl.h>		/* for fcntl() */

#define DEBUG 1 /* Boolean to Enable/Disable Debugging Output */
#define STR_SIZE 6 /* Length of String to represent States */
#define TIMEOUT_SEC 1 /* Length of the UDP recvfrom() Timeout (ms) */
#define TIMEOUT_MSEC 0 /* Length of the UDP recvfrom() Timeout (ms) */
#define TRUE 1 /* Boolean TRUE value */
#define FALSE 0 /* Boolean FALSE value */

int requestNumber = 0; /* Counter for the Request Number */

/* Data Structure to Send through UDP */
struct request {
	char client_ip[16]; /* To hold client IP address in dotted decimal */
	int inc; /* Incarnation number of clnFileient */
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
 * randomRequest Method
 * 
 * Allocates memory for the random requenFilest. Then grabs the
 * Global counter for the request number (Starting at 0). Then
 * assigns a random character (a-z or 97-122 ASCII). Increments Request Number.
 */
struct request* randomRequest() {
	struct request* req = malloc(sizeof *req); /* Allocate Space for Random Request */
	req->req = requestNumber; /* Use Global Counter for Request Number */
	req->c = (char) ((rand() % 26) + 97); /* Generate Random Number (97-122) Cast to Char */
	requestNumber++; /* Increment Request Number */
	return req;
}

/**
 * getIncarnationNumber Method
 * 
 * Searches stable storage for the previous incarnations of the request.
 * If found increment number and return. Else return 0 and store request.
 */
int getIncarnationNumber() {
	return 0;
	
	int inc = 1;
	FILE* fp;
	
	fp = fopen("./inc.txt", "w+");
	flock("./inc.txt", LOCK_SH);
	fscanf(fp, "%d", &inc);
	rewind(fp);
	fprintf(fp, "%d", (inc+1));
	flock("./inc.txt", LOCK_UN);
	fclose(fp);
	
	return inc;
}

void removeIncarnation() {
	remove("./inc.txt");
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
	printf("REQUEST: %d\n", req->req);
	printf("INCREMENT: %d\n", req->inc);
	printf("CHARACTER: %c\n", req->c);
	printf("\n");
}

int main(int argc, char* argv[]) {
	int sock; /* Socket descriptor */
	struct sockaddr_in serverAddr; /* Echo server address */
	struct sockaddr_in fromAddr; /* Source address of echo */
	unsigned short serverPort; /* Echo server port */
	unsigned int fromSize; /* In-out of address size for recvfrom() */
	int flags; /* Declare flags for UDP Socket */
	char* servIP; /* IP address of server */
	char clientString[STR_SIZE] = "$$$$$\0"; /* Buffer for receiving Client String */
	struct request* req; /* Pointer fornFile Random Request Structure */
	struct timeval tv; /* Structure to Modify UDP Socket Timeout */
	int packetSent; /* Boolean to determine if Server sent response */
	int i; /* Declare Counter for Loops */
	
	if (argc != 4) {
		/* Test for correct number of arguments */
		fprintf(stderr, "Usage: %s <Server IP> <Echo Port> <Client Number>\n", argv[0]);
		exit(1);
	}

	/* Make Random Code Based clock_ton Time */
	srand(time(NULL));
	
	/* Initialize Timeout Value */
	tv.tv_sec = TIMEOUT_SEC;
	tv.tv_usec = TIMEOUT_MSEC;
	
	/* non-blocking I/O on the UDP socket */
	flags = fcntl(sock, F_GETFL);
	flags |= O_NONBLOCK;
	fcntl(sock, F_SETFL, flags);

	/* Server IP anFileddress (dotted quad) */
	servIP = argv[1];

	/* Use given port */
	serverPort = atoi(argv[2]);

	/* Create a datagram/UDP socket */
	if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
		printError("socket() failed", TRUE);
	
	if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
		printError("setsockopt() failed", TRUE);
	}

	/* Construct the server address structure */
	memset(&serverAddr, 0, sizeof(serverAddr)); /* Zero out structure */
	serverAddr.sin_family = AF_INET; /* Internet addr family */
	serverAddr.sin_addr.s_addr = inet_addr(servIP); /* Server IP address */
	serverAddr.sin_port = htons(serverPort); /* Server port */

	// Loop 20 times and send random packets
	for (i = 0; i < 20; i++) {
		packetSent = FALSE;
		
		req = randomRequest(); /* Generate a random Request */
		req->client = atoi(argv[3]); /* Use given Client ID */
		req->inc = getIncarnationNumber(); /* Retrieve Incarnation Number */

		if (DEBUG) printRequest(req);

		do {
			/* Grab next IncarnationNumber */
			req->inc = getIncarnationNumber();
			/* Send the Structure to the server */
			if (sendto(sock, req, sizeof(*req), 0, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0)
				printError("sendto() sent a different number of bytes than expected", TRUE);

			/* Recv a response */
			fromSize = sizeof(fromAddr);
			if (recvfrom(sock, clientString, STR_SIZE, 0, (struct sockaddr *) &fromAddr, &fromSize) != STR_SIZE) {
				printError("recvfrom() failed", FALSE);
			} else if (serverAddr.sin_addr.s_addr != fromAddr.sin_addr.s_addr) {
				fprintf(stderr, "Error: received a packet from unknown source.\n");
				exit(1);
			} else {
				packetSent = TRUE;
			}
			
			sleep(10);
		} while (packetSent == FALSE);

		/* null-terminate the received data */
		printf("\n");
		printf("Received: %s\n", clientString); /* Print the Client String */
		printf("\n");
		printf("========================================================\n");

		/* Deallocate Memory of Pointers */
		free(req);
	}

	close(sock);
	removeIncarnation();
	exit(0);
}