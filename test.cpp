#include "events.hpp"
#include "tcpclient.hpp"
#include "tcpserver.hpp"

int main() {
    try {
        events ev("localhost", 6379);
        ev.onSubscribe("a",
                       [](const string& value) {
                           cout << "A: " << value << endl;
                       });
        ev.onSubscribe("b",
                       [](const string& value) {
                           cout << "B: " << value << endl;
                       });
        ev.onListPop("c",
                     [](const string& value) {
                         cout << value << endl;
                     });
        ev.run();
       
    } catch(string& exception) {
        cerr << exception << endl;
    }

    return 0;
}
