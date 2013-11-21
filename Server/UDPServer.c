#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket() and bind() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */

#define ECHOMAX 255     /* Longest string to echo */
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

int main(int argc, char *argv[]) {
    int sock; /* Socket */
    struct sockaddr_in echoServAddr; /* Local address */
    struct sockaddr_in clientAddr; /* Client address */
    unsigned int cliAddrLen; /* Length of incoming message */
    char echoBuffer[ECHOMAX]; /* Buffer for echo string */
    unsigned short echoServPort; /* Server port */
    int recvMsgSize; /* Size of received message */

    if (argc != 2) {
        /* Test for correct number of parameters */
        fprintf(stderr, "Usage:  %s <UDP SERVER PORT>\n", argv[0]);
        exit(1);
    }

    echoServPort = atoi(argv[1]); /* First arg:  local port */

    /* Create socket for sending/receiving datagrams */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");

    /* Construct local address structure */
    memset(&echoServAddr, 0, sizeof (echoServAddr)); /* Zero out structure */
    echoServAddr.sin_family = AF_INET; /* Internet address family */
    echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
    echoServAddr.sin_port = htons(echoServPort); /* Local port */

    /* Bind to the local address */
    if (bind(sock, (struct sockaddr *) &echoServAddr, sizeof (echoServAddr)) < 0)
        DieWithError("bind() failed");

    /* Run forever */
    for (;;) {
        /* Set the size of the in-out parameter */
        cliAddrLen = sizeof (clientAddr);

        /* Allocate the Structure Memory */
        struct request* req = malloc(sizeof *req);

        /* Allocate the Client's String */
        char clientString[STR_SIZE] = "     \0";
        char* clientIP;
        int i;

        /* Block until receive message from a client */
        if (recvfrom(sock, req, sizeof (*req), 0, (struct sockaddr *) &clientAddr, &cliAddrLen) < 0)
            DieWithError("recvfrom() failed");

        /* Gather Client IP Address */
        clientIP = inet_ntoa(clientAddr.sin_addr);

        printf("Handling client %s\n", clientIP);

        /* Assign Client IP Address to Structure Char Array */
        for (i = 0; i < strlen(clientIP); i++) {
            req->client_ip[i] = clientIP[i];
        }

        if (DEBUG) {
            printf("\n");
            printf("NEW REQUEST\n");
            printf("===========\n");
            printf("CLIENT IP: %s\n", req->client_ip);
            printf("CLIENT ID: %d\n", req->client);
            printf("INCREMENT: %d\n", req->inc);
            printf("CHARACTER: %c\n", req->c);
            printf("\n");
            printf("\n");
        }

        /* Send String back to the client */
        //		if (sendto(sock, echoBuffer, recvMsgSize, 0, (struct sockaddr *) &clientAddr, sizeof (clientAddr)) != recvMsgSize)
        //			DieWithError("sendto() sent a different number of bytes than expected");

        /* Deallocate the Structure Memory */
        free(req);
        free(clientIP);
    }
    /* NOT REACHED */
}