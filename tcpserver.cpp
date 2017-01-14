#include <iostream>
#include <string>
#include <cstring>
#include <stdexcept>
#include "tcpserver.hpp"

using std::string;
using std::runtime_error;

#define MAX_LISTENS 10
#define MAX_MESSAGE_SIZE 1024

tcpserver::tcpserver(const std::string& host,
                     unsigned short port)
    : host(host),
      port(port) {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        throw runtime_error("ERROR: opening socket");

    int optval = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));

    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(host.c_str());
    serveraddr.sin_port = htons((unsigned short)port);

    if (bind(sockfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0)
        throw runtime_error("ERROR: binding socket");
    
    if (listen(sockfd, MAX_LISTENS) < 0)
        throw runtime_error("ERROR: listening");
}

tcpserver::~tcpserver() {
#ifdef __APPLE__
    ::close(this->sockfd);
#else
    close(this->sockfd);
#endif
}

int tcpserver::accept(bool nonblocking) {
    struct sockaddr_in clientaddr;
    socklen_t clientlen;
#ifdef __APPLE__
    int childfd = ::accept(this->sockfd, (struct sockaddr *) &clientaddr, &clientlen);
#else
    int childfd = accept(this->sockfd, (struct sockaddr *) &clientaddr, &clientlen);
#endif
    if (childfd < 0) 
        throw runtime_error("ERROR: accept(" + string(strerror(errno)) + ")");

    if (nonblocking) {
        int flags = fcntl(childfd, F_GETFL);
        fcntl(childfd, F_SETFL, flags | O_NONBLOCK);
    }
    return childfd;
}

ssize_t tcpserver::write(int childfd, const std::string& msg) {
#ifdef __APPLE__
    return ::write(childfd, msg.c_str(), msg.length());
#else
    return write(childfd, msg.c_str(), msg.length());
#endif
}

ssize_t tcpserver::read(int childfd, std::string& msg) {
    ssize_t retval = 0, bytes_read;
    char raw_msg[MAX_MESSAGE_SIZE];
    bzero(raw_msg, MAX_MESSAGE_SIZE);

    msg.clear();
    while((bytes_read =
#ifdef __APPLE__
           ::read(childfd, raw_msg, MAX_MESSAGE_SIZE)) > 0
#else
          read(childfd, raw_msg, MAX_MESSAGE_SIZE)) > 0
#endif
        ) {
        retval += bytes_read;
        msg += string(raw_msg, bytes_read);
    }
    return retval;
}

void tcpserver::close() {
#ifdef __APPLE__
    ::close(sockfd);
#else
    close(sockfd);
#endif
}

bool tcpserver::isValid() {
    return fcntl(sockfd, F_GETFD) != -1 || errno != EBADF;
}

bool tcpserver::isValid(int childfd) {
    return fcntl(childfd, F_GETFD) != -1 || errno != EBADF;
}
