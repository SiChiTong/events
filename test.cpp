#include "events.hpp"
#include "tcpclient.hpp"
#include "tcpserver.hpp"

int main() {
    try {
        string a = "Queue A";
        string b = "Queue B";
        events ev("localhost", 6379);
        ev.onSubscribe("a",
                       [&a](redisAsyncContext* context, const string& value) {
                           cout << a << " : " << value << endl;
                       });
        ev.onListPop("b",
                     [&b](redisAsyncContext* context, const string& value) {
                         redisAsyncCommand(context, NULL, NULL, "publish a movida");
                         cout << b << " : " << value << endl;
                     });
        ev.onTimer(1, 1,
                   [](event_timer_watcher*) {
                       cout << "Dance in the rain" << endl;
                   });
        ev.run();
       
    } catch(string& exception) {
        cerr << exception << endl;
    }

    return 0;
}
