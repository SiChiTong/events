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

#define TELNET_CLRSCR "\u001B[2J"

class tcpserver {
public:
    tcpserver(unsigned short port);
    ~tcpserver();
    int fd() { return this->sockfd; }
    int accept();

    size_t write( const std::string& msg);
    size_t read(std::string& msg);
    void close();
    bool isValid();

private:
    unsigned short port;

    int sockfd;
    struct net::sockaddr_in serveraddr;
    struct net::hostent *server;
};
