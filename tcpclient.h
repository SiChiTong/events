#pragma once

namespace net {
    extern "C" {
        #include <sys/types.h>
        #include <sys/socket.h>
        #include <netinet/in.h>
        #include <netdb.h>
    }
}
#include <string>

class tcpclient {
public:
    tcpclient(const std::string& host, unsigned int port);
    void connect();
    int fd() { return this->sockfd; }

private:
    std::string host;
    unsigned int port;

    int sockfd;
    struct net::sockaddr_in servaddr;
    struct net::hostent *server;
};
