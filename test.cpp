#include "events.hpp"
#include "tcpclient.hpp"
#include "tcpserver.hpp"

int main() {
    try {
        tcpserver server(5555);
        tcpclient client("localhost", 5555);

        events ev;
        ev.onSignal(SIGINT, 
                    [](events* self) {
                        cout << "SIGINT received." << endl;
                        self->stop();
                    });
        ev.onTimer(0, 5,
                   [](events* self) {
                       cout << "Timer triggered 5secs." << endl;
                   });

        ev.onRead(server.fd(),
                  [&server, &ev](events* self) {
                      int childfd = server.accept();
                      cout << "Client connected." << endl;
                      ev.onTimer(0, 1,
                                 [childfd](events* self) {
                                     string msg = "Hello";
                                     write(childfd, msg.data(), msg.length());
                                 });
                  });

        ev.onTimer(1, 0,
                   [&client, &ev](events* self) {
                       client.connect();
                       ev.onRead(client.fd(),
                                 [&client](events* self) {
                                     char buf[100];
                                     read(client.fd(), &buf, sizeof(buf));
                                     cout << "Message received: " << buf << endl;
                                     fflush(NULL);
                                 });
                   });

        ev.run();
       
    } catch(string& exception) {
        cerr << exception << endl;
    }

    return 0;
}
