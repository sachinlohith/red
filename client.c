#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <time.h>

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd, portno, n, burstSize;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    int index = 0, index2 = 0;

    char buffer[256] = "abcde";
    if (argc < 4) {
        printf("Usage: ./client host port burstSize\n");
        exit(1);
    }
    portno = atoi(argv[2]);
    burstSize = atoi(argv[3]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error("ERROR opening socket");
        exit(2);
    }
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(3);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        error("ERROR connecting");
    }
    n = 1;
    n = setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &n, sizeof(n));
    if (n < 0)  {
        error("ERROR enabling TCP_NODELAY");
        exit(1);
    }
    while(1) {
        n = write(sockfd, (void *) &buffer[index], 1);
        if (n < 0) error("ERROR writing to socket");
        index++;
        if (index % 5 == 0) {
            index = 0;
        }
        index2++;
        if (index2 % burstSize == 0) {
            sleep(1);
            index2 = 0;
        }
    }
    close(sockfd);
    return 0;
}
