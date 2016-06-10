#pragma once

#include <iostream>
#include <functional>
#include <vector>
#include <memory>
#ifdef ASYNC_REDIS
#include <async.h>
#include <hiredis.h>
#endif
extern "C" {
#include <unistd.h>
#include <ev.h>
}

using namespace std;

class events; /* Forward declaration */

#ifdef ASYNC_REDIS
typedef void (*event_redis_callback)(redisAsyncContext*, void*, void*);
#endif

template<class T>
struct event_watcher {
    function<void(event_watcher<T>*)> callback;
    events* self;
    T watcher;
    void stop();
};

using event_signal_watcher = event_watcher<ev_signal>;
using event_timer_watcher = event_watcher<ev_timer>;
using event_io_watcher = event_watcher<ev_io>;
using event_async_watcher = event_watcher<ev_async>;
#ifdef ASYNC_REDIS
struct event_redis_watcher {
    redisAsyncContext* context;
    function<void(redisAsyncContext*, const string& value)> callback;
};
#endif

class events {
public:
    events();
    shared_ptr<event_signal_watcher> onSignal(int signal,
                                              function<void(event_signal_watcher*)> callback);
    shared_ptr<event_timer_watcher> onTimer(ev_tstamp after,
                                            ev_tstamp repeat,
                                            function<void(event_timer_watcher*)> callback);
    shared_ptr<event_io_watcher> onRead(int fd,
                                        function<void(event_io_watcher*)> callback);
    shared_ptr<event_async_watcher> onAsync(function<void(event_async_watcher*)> callback);

#ifdef ASYNC_REDIS
    events(const string& redis_host, unsigned short redis_port);
    void onSubscribe(const string& key, 
                     function<void(redisAsyncContext*,
                                   const string& value)> callback);
    void unsubscribe(const string& key);
    redisAsyncContext* getAsyncContext() {
        return this->redis;
    }
    redisAsyncContext* getPubSubAsyncContext() {
        return this->redis_pubsub;
    }
#endif

    void run();
    void stop();
    void sendAsync(shared_ptr<event_async_watcher> watcher);

private:
    friend event_signal_watcher;
    friend event_timer_watcher;
    friend event_io_watcher;
    friend event_async_watcher;

    static void signal_callback(struct ev_loop* loop, ev_signal* signal, int event);
    static void timer_callback(struct ev_loop* loop, ev_timer* timer, int event);
    static void io_callback(struct ev_loop* loop,ev_io* io, int event);
    static void async_callback(struct ev_loop* loop, ev_async* async, int event);

#ifdef ASYNC_REDIS
    static void redis_read_callback(redisAsyncContext *context, void *reply, void *data);
    static void redis_subscribe_callback(redisAsyncContext *context, void *reply, void *data);
#endif
    template<class EVENT_TYPE, class EVENT_WATCHER>
    static void callback(struct ev_loop* loop, EVENT_TYPE* event_handler, int event);

    template<class T>
    shared_ptr<T> new_watcher(function<void(T*)> callback);

    struct ev_loop *loop = NULL;    
    vector<shared_ptr<event_signal_watcher>> signal_watchers;
    vector<shared_ptr<event_timer_watcher>> timer_watchers;
    vector<shared_ptr<event_io_watcher>> io_watchers;
    vector<shared_ptr<event_async_watcher>> async_watchers;

#ifdef ASYNC_REDIS
    vector<event_redis_watcher*> redis_watchers;    
    redisAsyncContext *redis, *redis_pubsub;
#endif
};
