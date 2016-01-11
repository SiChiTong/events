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

class tcpclient {
public:
    tcpclient(const std::string& host, unsigned short port);
    ~tcpclient();

    void connect();
    int fd() { return this->sockfd; }

    size_t write( const std::string& msg);
    size_t read(std::string& msg);
    void close();
    bool isValid();

private:
    std::string host;
    unsigned short port;

    int sockfd;
    struct net::sockaddr_in servaddr;
    struct net::hostent *server;
};
