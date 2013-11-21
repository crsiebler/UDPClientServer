#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), sendto(), and recvfrom() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */

#define DEBUG 1         /* Boolean to Enable/Disable Debugging Output */
#define STR_SIZE 6	/* Length of String to represent States */

int requestNumber = 0;  /* Counter for the Request Number */

/* Data Structure to Send through UDP */
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

/**
 * randomRequest Method
 * 
 * Allocates memory for the random request. Then grabs the
 * Global counter for the request number (Starting at 0). Then
 * assigns a random character (a-z or 97-122 ASCII). Increments Request Number.
 */
struct request* randomRequest() {
    struct request* req = malloc(sizeof *req); /* Allocate Space for Random Request */
    req->req = requestNumber; /* Use Global Counter for Request Number */
    req->c = (char) rand() % 26 + 97; /* Generate Random Number (97-122) Cast to Char */
    requestNumber++;
    return req;
}

/**
 * getIncarnationNumber Method
 * 
 * Searches stable storage for the previous incarnations of the request.
 * If found increment number and return. Else return 0 and store request.
 */
int getIncarnationNumber(int num) {
    return 0;
}

int main(int argc, char* argv[]) {
    int sock; /* Socket descriptor */
    struct sockaddr_in serverAddr; /* Echo server address */
    struct sockaddr_in fromAddr; /* Source address of echo */
    unsigned short serverPort; /* Echo server port */
    unsigned int fromSize; /* In-out of address size for recvfrom() */
    char* servIP; /* IP address of server */
    char echoBuffer[STR_SIZE]; /* Buffer for receiving echoed string */
    int respStringLen; /* Length of received response */
    struct request* req; /* Pointer for Random Request Structure */

    if (argc != 4) {
        /* Test for correct number of arguments */
        fprintf(stderr, "Usage: %s <Server IP> <Echo Port> <Client Number>\n", argv[0]);
        exit(1);
    }

    /* Server IP address (dotted quad) */
    servIP = argv[1];

    /* Use given port */
    serverPort = atoi(argv[2]);
    
    /* Create a datagram/UDP socket */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");

    /* Construct the server address structure */
    memset(&serverAddr, 0, sizeof(serverAddr)); /* Zero out structure */
    serverAddr.sin_family = AF_INET; /* Internet addr family */
    serverAddr.sin_addr.s_addr = inet_addr(servIP); /* Server IP address */
    serverAddr.sin_port = htons(serverPort); /* Server port */

    req = randomRequest(); /* Generate a random Request */
    req->client = atoi(argv[3]); /* Use given Client ID */
    req->inc = getIncarnationNumber(req->req); /* Retrieve Incarnation Number */
    
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
    echoBuffer[STR_SIZE] = '\0';
    printf("Received: %s\n", echoBuffer); /* Print the echoed arg */

    /* Deallocate Memory of Pointers */
    free(req);
    
    close(sock);
    exit(0);
}