#pragma once

namespace net {
    extern "C" {
        #include <sys/types.h>
        #include <sys/socket.h>
        #include <netinet/in.h>
        #include <netdb.h>
        #include <unistd.h>
        #include <fcntl.h>
    }
}
#include <string>

class tcpserver {
public:
    tcpserver(unsigned short port);
    ~tcpserver();
    int fd() { return this->sockfd; }
    int accept();

private:
    unsigned short port;

    int sockfd;
    struct net::sockaddr_in serveraddr;
    struct net::hostent *server;
};
