#include <string>
#include <cstring>
#include "tcpserver.hpp"

using std::string;
using namespace net;

#define MAX_LISTENS 10

tcpserver::tcpserver(unsigned short port) : port(port) {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        throw string("ERROR: opening socket");

    int optval = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));

    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons((unsigned short)port);

    if (bind(sockfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0)
        throw string("ERROR: binding socket");
    
    if (listen(sockfd, MAX_LISTENS) < 0)
        throw string("ERROR: listening");
}

tcpserver::~tcpserver() {
#ifdef __APPLE__
    close(this->sockfd);
#else
    net::close(this->sockfd);
#endif
}

int tcpserver::accept() {
    struct net::sockaddr_in clientaddr;
    net::socklen_t clientlen;

    int childfd = net::accept(this->sockfd, (struct sockaddr *) &clientaddr, &clientlen);
    if (childfd < 0) 
        throw string("ERROR: accept");

    int flags = fcntl(childfd, F_GETFL);
    flags |= O_NONBLOCK;
    fcntl(childfd, F_SETFL, flags);
    return childfd;
}
