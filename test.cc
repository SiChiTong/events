#include "events.h"
#include "tcpclient.h"

int main() {
    try {
        tcpclient client("localhost", 5555);
        client.connect();

        events ev;
        ev.onSignal(SIGINT, 
                    [](events* self) {
                        cout << "SIGINT received." << endl;
                        self->stop();
                    });
        ev.onTimer(0, 2,
                   [](events* self) {
                       cout << "Timer triggered." << endl;
                   });
        ev.onRead(client.fd(),
                  [&client](events* self) {
                      char buf[1];
                      read(client.fd(), &buf, 1);
                      cout << buf[0];
                  });
        ev.run();
       
    } catch(string& exception) {
        cerr << exception << endl;
    }

    return 0;
}
