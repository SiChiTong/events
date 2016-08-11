#include "events.hpp"
#include "tcpclient.hpp"
#include "tcpserver.hpp"

int main() {
    try {
#ifdef ASYNC_REDIS
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
        ev.onPop("Tasks",
                 [](redisAsyncContext*, const string& value) {
                     cout << "Tasks: " << value << endl;
                 });
#else
        tcpserver server("127.0.0.1", 12345);
        events ev;
        ev.onRead(server.fd(),
                  [&server](event_io_watcher*) {
                      server.accept();
                      cout << "Client connected." << endl;
                  });
#endif
        ev.run();
       
    } catch(string& exception) {
        cerr << exception << endl;
    }

    return 0;
}
