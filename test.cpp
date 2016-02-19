#include "events.hpp"
#include "tcpclient.hpp"
#include "tcpserver.hpp"

int main() {
    try {
        events ev("localhost", 6379);
        ev.onSubscribe("A",
                       [](redisAsyncContext* context, const string& value) {
                           cout << "A: " << value << endl;
                           redisAsyncCommand(context, NULL, NULL, "RPUSH REP:A %s", value.c_str());
                       });
        ev.onSubscribe("B",
                       [](redisAsyncContext* context, const string& value) {
                           cout << "B: " << value << endl;
                           redisAsyncCommand(context, NULL, NULL, "RPUSH REP:B %s", value.c_str());
                       });
        ev.run();
       
    } catch(string& exception) {
        cerr << exception << endl;
    }

    return 0;
}
