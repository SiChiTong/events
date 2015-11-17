#include "events.h"

int main() {
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
    ev.onRead(STDIN_FILENO,
              [](events* self) {
                  char buf[1024];
                  cin >> buf;
                  cout << "Reading: " << buf << endl;
              });
    ev.run();

    return 0;
}
