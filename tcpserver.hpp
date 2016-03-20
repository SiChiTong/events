#pragma once

namespace net {
    extern "C" {
        #include <sys/types.h>
        #include <sys/socket.h>
        #include <netinet/in.h>
        #include <arpa/inet.h>
        #include <netdb.h>
        #include <unistd.h>
        #include <fcntl.h>
    }
}
#include <string>

#define TELNET_CLRSCR "\u001B[2J"

class tcpserver {
public:
    tcpserver(const std::string& host,
              unsigned short port);
    ~tcpserver();
    int fd() { return this->sockfd; }
    int accept();

    ssize_t write(int childfd, const std::string& msg);
    ssize_t read(int childfd, std::string& msg);
    void close();
    bool isValid();
    bool isValid(int childfd);

    class exception {
    public:
        exception(const std::string& msg)
            : msg(msg)
            { /* Left blank intentionally */ }
        std::string msg;
    };

private:
    const std::string host;
    unsigned short port;

    int sockfd;
    struct net::sockaddr_in serveraddr;
    struct net::hostent *server;
};
