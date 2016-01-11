#include <string>
#include <cstring>
#include "tcpclient.hpp"

using std::string;
using namespace net;

#define MAX_MESSAGE_SIZE 1024

tcpclient::tcpclient(const string& host, unsigned short port) : host(host), port(port) {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        throw string("ERROR: opening socket");

    server = gethostbyname(host.c_str());
    if (server == NULL)
        throw string("ERROR: no such host");

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    memcpy(server->h_addr, &servaddr.sin_addr.s_addr, server->h_length);
    servaddr.sin_port = htons(port);
}

void tcpclient::connect() {
    if (net::connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
        throw string("ERROR: connecting to " + this->host);
}

tcpclient::~tcpclient() {
#ifdef __APPLE__
    close(this->sockfd);
#else
    net::close(this->sockfd);
#endif
}

size_t tcpclient::write(const std::string& msg) {
    return net::write(sockfd, msg.c_str(), msg.length());
}

size_t tcpclient::read(std::string& msg) {
    char raw_msg[MAX_MESSAGE_SIZE];
    bzero(raw_msg, MAX_MESSAGE_SIZE);
    size_t retval = net::read(sockfd, raw_msg, MAX_MESSAGE_SIZE);
    if (retval)
        msg = raw_msg;
    return retval;
}

void tcpclient::close() {
    net::close(sockfd);
}

bool tcpclient::isValid() {
    return fcntl(sockfd, F_GETFD) != -1 || errno != EBADF;
}

