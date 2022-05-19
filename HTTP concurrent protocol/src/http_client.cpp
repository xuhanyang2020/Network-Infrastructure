/*
** client.c -- a stream socket client demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string>

#include <arpa/inet.h>
#include <iostream>
#include <fstream>

#define MAXDATASIZE 1000 // max number of bytes we can get at once

using namespace std;
// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


int main(int argc, char *argv[])
{
    int sockfd, numbytes;
    char recvbuffer[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];
    string target, protocol, filepath, host_ip;

    ofstream fp;

    if (argc != 2) {
        fprintf(stderr,"usage: client hostname\n");
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    // parse url
    target = argv[1];

    int deli = target.find_first_of("//");
    // delimite the protocol name
    protocol = target.substr(0, deli - 1);
    // change the value of the target
    target = target.substr(deli + 2);
    deli = target.find('/');
    // target path of the file
    filepath = target.substr(deli);
    host_ip = target.substr(0, deli);
    
    string host, port;
    if (host_ip.find(':') == host_ip.npos) {
        // specify the port implicitly
        port = "80";
        host = host_ip;
    } else {
        deli = host_ip.find(':');
        port = host_ip.substr(deli + 1);
        host = host_ip.substr(0, deli);
    }
    cout << host << "\n" << port << "\n" << filepath << endl;

    if ((rv = getaddrinfo(host.data(), port.data(), &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
              s, sizeof s);
    printf("client: connecting to %s\n", s);

    freeaddrinfo(servinfo); // all done with this structure

    // assemble http request
    auto request = "GET " + filepath + " HTTP/1.1\r\n"
                   + "User-Agent: Wget/1.12(linux-gnu)\r\n"
                   + "Host: " + host + ":" + port + "\r\n"
                   + "Connection: Keep-Alive\r\n\r\n";
    // send http request
    printf("%s\n", request.data());
    send(sockfd, request.c_str(), request.size(), 0);
    // receive http response
    fp.open("output", ios::binary);
    int flag = 1;
    while(1) {
        memset(recvbuffer, '\0', MAXDATASIZE);
        numbytes = recv(sockfd, recvbuffer, MAXDATASIZE, 0);
        if (numbytes > 0){
            if (!flag) {
                fp.write(recvbuffer, sizeof(char) * numbytes);
            } else {
                char* start = strstr(recvbuffer, "\r\n\r\n") + 4;
                flag = 0; // next round do not need to test /r/n/r/n
                fp.write(start,strlen(start));
            }

        } else {
            fp.close();
            break;
        }
    }


    close(sockfd);

    return 0;
}
