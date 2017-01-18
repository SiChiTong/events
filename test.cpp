
#include "events.hpp"
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
        events ev;
        ev.onListen("127.0.0.1", 12345, [&ev](event_tcp_watcher* server) {
                event_stream* client = server->accept();
                cout << "Connection" << endl;
                ev.onRead(client, [](event_stream_watcher* stream) {
                        cout << "Reading" << endl;
                        if (stream->nread > 0)
                            cout << stream->buffer->base << endl;
                        else
                            cout << "Disconnected" << endl;
                    });
            });
        ev.onConnect("127.0.0.1", 12345, [&ev](event_connect_watcher* watcher) {
                /* TODO how to close the connection */
                /* TODO check memory leaks */
                cout << "Connected" << endl;
                ev.onTimer(1, 2000, [&ev, watcher](event_timer_watcher*) {
                        ev.onWrite(watcher->watcher_ptr->handle, "hello", 5);
                    });
            });
        ev.onTimer(1, 1000, [](event_timer_watcher*) {
                cout << "Sec elapsed..." << endl;
            });
        ev.onSignal(SIGINT, [](event_signal_watcher* watcher) {
                watcher->self->stop();
            });
#endif
        ev.run();
       
    } catch(string& exception) {
        cerr << exception << endl;
    }

    return 0;
}
