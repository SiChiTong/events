#include <string>
#include <cstring>
#include "tcpclient.h"

using std::string;
using namespace net;

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
