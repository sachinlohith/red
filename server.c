#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <math.h>
#include <arpa/inet.h>
#include <time.h>
#include <signal.h>
#include "queue.h"

#define max(A, B) (A >= B) ? A : B;

// Algorithm's parameter initialization
double avg = 0; // Average queue length
int count = -1; // Count of packets since last probabilistic drop
double wq = 0.002; // Queue weight; standard value of 0.002 for early congestion detection
int minThreshold, maxThreshold;
double maxp = 0.02; // Maximum probability of dropping a packet; standard value of 0.02
double pb = 0; // Probability of dropping a packet
time_t qTime; // Time since the queue was last idle
Queue *queue; // Queue to store the packets

// Handle CTRL+Z(stop signal) keyboard signal
void sig_handler(int signo)
{
  if (signo == SIGTSTP) {
      queue->size = 0;
      queue->front = 0;
      queue->rear = -1;
      qTime = time(NULL);
  }
}

void error(const char* msg) {
    perror(msg);
    exit(1);
}

// RED algorithm according to http://www.utdallas.edu/~jjue/cs6390/papers/red.pdf
void red(Queue *queue, char *buffer) {
    printf("Current packet : %c\n", buffer[0]);
    // Average queue length calculation
    if (queue->size == 0) {
        double m = (time(NULL) - qTime) / 0.001;
        avg = pow((1 - wq), m) * avg;
    } else {
        avg = ((1 - wq)*avg) + (wq*queue->size);
    }
    printf("Average queue length : %f\n", avg);
    // If the average length is in between minimum and maximum threshold,
    // Probabilistically drop a packet
    if(minThreshold <= avg && avg < maxThreshold) {
        count++;
        pb = avg - minThreshold;
        pb = pb * maxp;
        pb = pb / (maxThreshold - minThreshold);
        double pa = pb / (1 - (count*pb));
        if (count == 50) {
            printf("Count has reached 1/maxp; dropping the next packet\n");
            pa = 1.0;
        }
        float randomProb = (rand()%100)/100.0;
        if(randomProb < pa) {
            // Drop the packet with probability pa
            printf("Dropping packet : %c with probability : %f\n", buffer[0], pa);
            // Since this packet was dropped, count is reinitialized to 0
            count = 0;
        } else {
            // Add the packet to the queue
            add(queue, buffer[0]);
        }
    } else if (maxThreshold <= avg) {
        // Drop the packet
        printf("Packet Dropped : %c\n", buffer[0]);
        // Since this packet was dropped, count is reinitialized to 0
        count = 0;
    } else {
        // Average queue length is below minimum threhold, accept all packets
        // Add packet to the queue
        add(queue, buffer[0]);
        // Since the average queue length is below minimum threshold, initialize count to -1
        count = -1;
    }
}

// Write PID of the current process to a file for the Queue processor python script to read
void writePID(int pid) {
    FILE* fp;

    fp = fopen("pid", "w");
    fprintf(fp, "%d", pid);
    fclose(fp);
}

int main(int argc, char** argv) {
    int sockfd, finalSocket, portno;
    socklen_t clilen;
    char *buffer;
    struct sockaddr_in serv_addr, cli_addr;
    int index = 0;
    int n;
    if (argc < 4) {
        printf("Usage: ./server host port queueSize\n");
        exit(1);
    }
    // Seed the random number generator
    srand(time(NULL));
    writePID(getpid());

    // Create a socket file descriptor
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error("ERROR opening socket");
    }

    // Clear the server address structure contents
    bzero((char *) &serv_addr, sizeof(serv_addr));

    // Get port number from command line argument
    portno = atoi(argv[2]);

    // Intialize server address parameters
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(portno);

    // Bind the socket to the host_address + port_no given
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        error("ERROR on binding");
    }

    // Listen on the socket for incoming connections
    listen(sockfd,1);
    clilen = sizeof(cli_addr);

    // Accept a connection from a client
    finalSocket = accept(sockfd,
             (struct sockaddr *) &cli_addr,
             &clilen);
    if (finalSocket < 0) {
        error("ERROR on accept");
    }
    printf("Connection established\n");
    buffer = (char *) malloc(sizeof(char));
    queue = createQueue(atoi(argv[3]));

    minThreshold = 10;
    maxThreshold = 30;

    // Queue is idle when created
    qTime = time(NULL);

    // Initialize the signal handler
    if (signal(SIGTSTP, sig_handler) == SIG_ERR) {
        error("\ncan't catch SIGTSTP\n");
        exit(1);
    }

    while(1) {
        // Clear buffer
        bzero(buffer, 1);

        // Read from the socket
        n = read(finalSocket, buffer, 1);
        if (n < 0) error("ERROR reading from socket");

        // Process the packet according to the RED algorithm
        red(queue, buffer);

        // Display the contents of the queue
        display(queue);
        index++;

        // Delete 3 elements for every 5th packet added to the queue
        // to simulate a real server scenario
        if(index % 5 == 0) {
            index = 0;
            delete(queue);
            delete(queue);
            delete(queue);
        }
        printf("\n");
    }

    // Close the connection
    close(finalSocket);
    close(sockfd);
    return 0;
}
