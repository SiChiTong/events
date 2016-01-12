#include "events.hpp"
#include "tcpclient.hpp"
#include "tcpserver.hpp"

int main() {
    try {
        tcpserver server(5555);
        tcpclient client("localhost", 5555);

        events ev;
        auto event = ev.onAsync([](event_async_watcher* watcher) {
                cout << "ASYNC received." << endl;
            });
        ev.onSignal(SIGINT, 
                    [](event_signal_watcher* watcher) {
                        cout << "SIGINT received." << endl;
                        watcher->self->stop();
                    });
        ev.onTimer(0, 5,
                   [&ev, event](event_timer_watcher* watcher) {
                       cout << "Timer triggered 5secs." << endl;
                       ev.sendAsync(event);
                   });

        ev.onRead(server.fd(),
                  [&server, &ev](event_io_watcher* watcher) {
                      int childfd = server.accept();
                      cout << "Client connected." << endl;
                      ev.onTimer(0, 1,
                                 [childfd](event_timer_watcher* watcher) {
                                     string msg = "Hello";
                                     write(childfd, msg.data(), msg.length());
                                 });
                  });

        ev.onTimer(1, 0,
                   [&client, &ev](event_timer_watcher* watcher) {
                       client.connect();
                       ev.onRead(client.fd(),
                                 [&client](event_io_watcher* watcher) {
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
