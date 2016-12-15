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

double avg = 0;
int count = -1;
double wq = 0.002;
int minThreshold, maxThreshold;
double maxp = 0.02;
double pb = 0;
time_t qTime;
Queue *queue;

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

void red(Queue *queue, char *buffer) {
    printf("Current packet : %c\n", buffer[0]);
    if (queue->size == 0) {
        double m = (time(NULL) - qTime) / 0.001;
        avg = pow((1 - wq), m) * avg;
    } else {
        avg = ((1 - wq)*avg) + (wq*queue->size);
    }
    printf("Average queue length : %f\n", avg);
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
            printf("Dropping packet : %c with probability : %f\n", buffer[0], pa);
            count = 0;
        } else {
            add(queue, buffer[0]);
        }
    } else if (maxThreshold <= avg) {
        printf("Packet Dropped : %c\n", buffer[0]);
        count = 0;
    } else {
        add(queue, buffer[0]);
        count = -1;
    }
}

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
    srand(time(NULL));
    writePID(getpid());
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error("ERROR opening socket");
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[2]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        error("ERROR on binding");
    }
    listen(sockfd,1);
    clilen = sizeof(cli_addr);
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
    qTime = time(NULL);
    if (signal(SIGTSTP, sig_handler) == SIG_ERR) {
        error("\ncan't catch SIGTSTP\n");
        exit(1);
    }
    while(1) {
        bzero(buffer, 1);
        n = read(finalSocket, buffer, 1);
        if (n < 0) error("ERROR reading from socket");
        red(queue, buffer);
        display(queue);
        index++;
        if(index % 5 == 0) {
            index = 0;
            delete(queue);
            delete(queue);
            delete(queue);
        }
        printf("\n");
    }
    close(finalSocket);
    close(sockfd);
    return 0;
}
