#pragma once

#include <iostream>
#include <functional>
#include <vector>
#include <memory>

extern "C" {
#include <unistd.h>
#include <ev.h>
}

using namespace std;

class events {
public:
    template<class T>
    struct event_watcher {
        function<void(events*)> callback;
        events* self;
        T watcher;
    };
    using event_signal_watcher = event_watcher<ev_signal>;
    using event_timer_watcher = event_watcher<ev_timer>;
    using event_io_watcher = event_watcher<ev_io>;

    events();
    void onSignal(int signal, function<void(events*)> callback);
    void onTimer(ev_tstamp after, ev_tstamp repeat, function<void(events*)> callback);
    void onRead(int fd, function<void(events*)> callback);
    void run();
    void stop();

private:
    static void signal_callback(struct ev_loop* loop, ev_signal* signal, int event);
    static void timer_callback(struct ev_loop* loop, ev_timer* timer, int event);
    static void io_callback(struct ev_loop* loop,ev_io* io, int event);

    template<class EVENT_TYPE, class EVENT_WATCHER>
    static void callback(struct ev_loop* loop, EVENT_TYPE* event_handler, int event);

    template<class T>
    shared_ptr<T> new_watcher(function<void(events*)> callback);

    struct ev_loop *loop = NULL;    
    vector<shared_ptr<event_signal_watcher>> signal_watchers;
    vector<shared_ptr<event_timer_watcher>> timer_watchers;
    vector<shared_ptr<event_io_watcher>> io_watchers;
};
