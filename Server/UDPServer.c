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

typedef enum {
	IGNORE_REQUEST = -1,
	PERFORM_SERVICE = -2,
	CLIENT_NEW = -3
} response_type;

typedef struct client_table {
	char processor[16]; /* Client Processor ID/IP address*/
	int inc; /* Client Incarnation number*/
	int client; /* Client number */
	int *req; /* Request numbers for the client */
	char *response; /* Stored responses for the client */
} client_table_t;

client_table_t *clientTable;
char clientString[STR_SIZE] = "     \0"; /* Buffer for echo string */
int sock; /* Socket */
struct sockaddr_in clientAddr; /* Client address */

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

void printClientTable() {
	int i, j, k;
	int numberOfClientTableEntries = sizeof(clientTable) / sizeof(client_table_t*);
	int numberOfReqResponse;
	
	printf("----------CLIENT TABLE----------\n");
	
	for (i = 0; i < numberOfClientTableEntries; i++) {
		printf("Entry [%d]\n", i + 1);
		printf("==========\n");
		printf("Processor ID: ");
		
		for (j = 0; j < 16; j++) {
			printf("%c", clientTable[i].processor[j]);
		}
		
		printf("\n");
		printf("Incarnation number: %d\n", clientTable[i].inc);
		printf("Client number: %d\n", clientTable[i].client);
		numberOfReqResponse = sizeof(clientTable[i].req) / sizeof(int *);
		
		for (j = 0; j < numberOfReqResponse; j++) {
			printf("Request %d, Response: ", clientTable[i].req[j]);
			
			for (k = j * STR_SIZE; k < j * STR_SIZE + STR_SIZE; k++) {
				printf("%c", clientTable[i].response[k]);
			}
			
			printf("\n");
		}
		
		for (j = 0; j < 1000; j++) {
			if (clientTable[i].response[k] != NULL) {
				printf("%c", clientTable[i].response[k]);
			} else {
				break;
			}
		}
	}

	printf("Number of requests/responses: %d\n", numberOfReqResponse);
	printf("\n--------------------------------\n");
}

/**
 * FUNCTION: requestResponse
 * DESCR:    Searches through the client table and returns the response type for the server to perform
 * RETURNS:  Enumerated type for server response, either IGNORE_REQUEST (R < r),
 *	     PERFORM_SERVICE (R > r), or CLIENT_NEW (client not in client table). If (R == r), returns
 *	     the index in the client table that the response is stored at.
 */
response_type requestResponse(struct request *p_request) {
	int i, j;
	int maxRequestIndex;
	int maxRequestNumber = -1;
	int processorMatches;

	if (clientTable == NULL) return CLIENT_NEW;
	
	for (i = 0; i < sizeof(clientTable) / sizeof(client_table_t*); i++) {
		/* Match up (P,I,C) in the client table with the given p_request */
		if (clientTable[i].inc == p_request->inc && clientTable[i].client == p_request->client) {
			for (j = 0; j < 16; j++) {
				if (clientTable[i].processor[j] != p_request->client_ip[j]) {
					processorMatches = FALSE;
					break;
				}
				
				if (j == 15) processorMatches = TRUE;
			}
			
			if (processorMatches == TRUE) {
				/* Check all requests for the given request number */
				for (j = 0; j < sizeof(clientTable[i].req) / sizeof(int*); j++) {
					if (clientTable[i].req[j] > maxRequestNumber) {
						maxRequestNumber = clientTable[i].req[j];
						maxRequestIndex = j;
					}
				}
			}
		}
	}

	printf("Max request number: %d\n", maxRequestNumber);
	
	/* (P,I,C) not found in client table */
	if (maxRequestNumber == -1) {
		return CLIENT_NEW;
	} else if (p_request->req < maxRequestNumber) {
		/* (P,I,C) found */
		return IGNORE_REQUEST;
	} else if (p_request->req == maxRequestNumber) {
		return maxRequestIndex;
	} else if (p_request->req > maxRequestNumber) {
		return PERFORM_SERVICE;
	} else {
		return -4;
	}
}

int PICIndex(struct request* p_request) {
	int i, j;
	
	if (clientTable == NULL) return -1;
	
	for (i = 0; i < sizeof(clientTable) / sizeof(client_table_t*); i++) {
		/* Match up (P,I,C) in the client table with the given p_request */
		if (clientTable[i].inc == p_request->inc && clientTable[i].client == p_request->client) {
			for (j = 0; j < 16; j++) {
				if (clientTable[i].processor[j] != p_request->client_ip[j]) break;
				if (j == 15) return i;
			}
		}
	}
	
	return -1;
}

void performRequest(struct request* p_request) {
	int i;
	srand(time(NULL));
	
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

			clientString[0] = p_request->c;

			if (DEBUG) printRequest(p_request);
			if (DEBUG) printf("CLIENT STRING: %s\n\n", clientString);

			printf("Dropping performed request...\n");
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

			clientString[0] = p_request->c;

			if (DEBUG) printRequest(p_request);
			if (DEBUG) printf("CLIENT STRING: %s\n\n", clientString);

			/* Send String back to the client */
			if (sendto(sock, clientString, STR_SIZE, 0, (struct sockaddr *) &clientAddr, sizeof(clientAddr)) != STR_SIZE)
				printError("sendto() sent a different number of bytes than expected", TRUE);

			printf("Request Sent...\n");
			printf("\n");
			printf("========================================================\n");
			break;
	}
}

void addReqAndResponse(struct request* p_request, int p_PICIndex, char p_response[STR_SIZE]) {
	int i;
	int j;
	int numberOfReqResponse = sizeof(clientTable[p_PICIndex].req) / sizeof(int *);
	int* tempReq = (int *) realloc(clientTable[p_PICIndex].req, (numberOfReqResponse + 1) * sizeof(int *));
	clientTable[p_PICIndex].req = tempReq;
	clientTable[p_PICIndex].req[numberOfReqResponse] = p_request->req;

	char* tempResponse = (char *) realloc(clientTable[p_PICIndex].response, (numberOfReqResponse + 1) * (sizeof(char *)));
	clientTable[p_PICIndex].response = tempResponse;
	
	for (i = STR_SIZE* numberOfReqResponse, j = 0; i < STR_SIZE* numberOfReqResponse + STR_SIZE; i++, j++) {
		clientTable[p_PICIndex].response[i] = p_response[j];
	}
}

void addEntryToClientTable(struct request* p_request, char p_response[STR_SIZE]) {
	int numberOfClientTableEntries = sizeof(clientTable) / sizeof(client_table_t*);

	int i;
	int j;

	printf("Number of client table entries: %d\n", numberOfClientTableEntries);
	
	if (numberOfClientTableEntries != 1) {
		client_table_t *tempClientTable = (client_table_t *) realloc(clientTable, (numberOfClientTableEntries + 1) * sizeof(client_table_t *));
		clientTable = tempClientTable;
		
		for (i = 0; i < 16; i++) {
			clientTable[numberOfClientTableEntries].processor[i] = p_request->client_ip[i];
		}
		
		clientTable[numberOfClientTableEntries].inc = p_request->inc;
		clientTable[numberOfClientTableEntries].client = p_request->client;

		int numberOfReqResponse = sizeof(clientTable[numberOfClientTableEntries].req) / sizeof(int *);
		int *tempReq = (int *) realloc(clientTable[numberOfClientTableEntries].req, (numberOfReqResponse + 1) * sizeof(int *));
		clientTable[numberOfClientTableEntries].req = tempReq;
		clientTable[numberOfClientTableEntries].req[numberOfReqResponse] = p_request->req;
		free(tempReq);

		char *tempResponse = (char *) realloc(clientTable[numberOfClientTableEntries].response, (numberOfReqResponse + 1) * (sizeof(char *) * STR_SIZE));
		clientTable[numberOfClientTableEntries].response = tempResponse;
		
		for (i = STR_SIZE * numberOfClientTableEntries, j = 0; i < STR_SIZE * numberOfClientTableEntries + STR_SIZE; i++, j++) {
			clientTable[numberOfClientTableEntries].response[i] = p_response[j];
		}
		
		free(tempResponse);
		free(tempClientTable);
	} else {
		clientTable = (client_table_t *) malloc(sizeof(client_table_t *));

		for (i = 0; i < 16; i++) {
			clientTable[0].processor[i] = p_request->client_ip[i];
		}

		clientTable[0].inc = p_request->inc;
		clientTable[0].client = p_request->client;

		clientTable[0].req = (int *) malloc(sizeof(int *));
		clientTable[0].req[0] = p_request->req;
		printf("New request: %d\n", clientTable[0].req[0]);

		clientTable[0].response = (char *) malloc(STR_SIZE * sizeof(char *));
		
		for (i = 0; i < STR_SIZE; i++) {
			clientTable[0].response[i] = p_response[i];
		}
	}
}

int main(int argc, char* argv[]) {
	struct sockaddr_in echoServAddr; /* Local address */
	unsigned int cliAddrLen; /* Length of incoming message */
	unsigned short echoServPort; /* Server port */
	struct request* req; /* Declare Client's Request Structure */
	char* clientIP; /* Declare the Client's IP String Pointer */
	int i; /* Declare Counter for Loops */
	char storedResponse[STR_SIZE];
	int storedPICIndex;

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

		/* Determine response to request */
		response_type currentResponse = requestResponse(req);
		
		if (currentResponse == IGNORE_REQUEST) {
			printf("Response: IGNORE_REQUEST\n");
		} else if (currentResponse == PERFORM_SERVICE) {
			printf("Response: PERFORM_SERVICE\n");
			performRequest(req);
			addReqAndResponse(req, PICIndex(req), clientString);
			printClientTable();
		} else if (currentResponse == CLIENT_NEW) {
			printf("Response: CLIENT_NEW\n");
			performRequest(req);
			addEntryToClientTable(req, clientString);
			printClientTable();
		} else {
			printf("Response: SEND_STORED_RESPONSE\n");
			storedPICIndex = PICIndex(req);
			
			for (i = 0; i < STR_SIZE - 1; i++) {
				storedResponse[i] = clientTable[storedPICIndex].response[currentResponse * STR_SIZE + i];
			}
			
			storedResponse[STR_SIZE-1] = '\0';
			
			/* Send String back to the client */
			if (sendto(sock, storedResponse, STR_SIZE, 0, (struct sockaddr *) &clientAddr, sizeof(clientAddr)) != STR_SIZE)
				printError("sendto() sent a different number of bytes than expected", TRUE);

			printf("Request Sent...\n");
			printf("\n");
			printf("========================================================\n");
		}

		/* Deallocate the Structure Memory */
		free(req);
	}
	/* NOT REACHED */
}