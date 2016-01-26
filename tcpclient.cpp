#include <iostream>
#include <string>
#include <cstring>
#include "tcpclient.hpp"

using std::string;
using namespace net;

#define MAX_MESSAGE_SIZE 1024

tcpclient::tcpclient(const string& host, unsigned short port) : host(host), port(port) {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        throw exception("ERROR: opening socket");

    server = gethostbyname(host.c_str());
    if (server == NULL)
        throw exception("ERROR: no such host");

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    memcpy(server->h_addr, &servaddr.sin_addr.s_addr, server->h_length);
    servaddr.sin_port = htons(port);
}

void tcpclient::connect() {
    if (net::connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
        throw exception("ERROR: connecting to " + this->host);
    int flags = fcntl(sockfd, F_GETFL);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
}

tcpclient::~tcpclient() {
#ifdef __APPLE__
    ::close(this->sockfd);
#else
    net::close(this->sockfd);
#endif
}

ssize_t tcpclient::write(const std::string& msg) {
#ifdef __APPLE__
    return ::write(sockfd, msg.c_str(), msg.length());
#else
    return net::write(sockfd, msg.c_str(), msg.length());
#endif
}

ssize_t tcpclient::read(std::string& msg) {
    ssize_t retval = 0, bytes_read;
    char raw_msg[MAX_MESSAGE_SIZE];
    bzero(raw_msg, MAX_MESSAGE_SIZE);

    msg.clear();
    while((bytes_read =
#ifdef __APPLE__
           ::read(sockfd, raw_msg, MAX_MESSAGE_SIZE)) > 0
#else
           net::read(sockfd, raw_msg, MAX_MESSAGE_SIZE)) > 0
#endif
        ) {
        retval += bytes_read;
        msg += string(raw_msg, bytes_read);
    }
    return retval;
}

void tcpclient::close() {
#ifdef __APPLE__
    ::close(sockfd);
#else
    net::close(sockfd);
#endif
}

bool tcpclient::isValid() {
    return fcntl(sockfd, F_GETFD) != -1 || errno != EBADF;
}

