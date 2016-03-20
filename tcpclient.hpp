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

    ssize_t write(const std::string& msg);
    ssize_t read(std::string& msg);
    void close();
    bool isValid();

    class exception {
    public:
        exception(const std::string& msg) : msg(msg) { /* Left blank intentionally */ }
        std::string msg;
    };

private:
    std::string host;
    unsigned short port;

    int sockfd;
    struct net::sockaddr_in servaddr;
    struct net::hostent *server;
};
