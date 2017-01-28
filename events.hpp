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
#include <uv.h>
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
    union {
        T watcher;
        T* watcher_ptr;
    };
    void stop();     /* Stop the watcher but does not release it */
    void release();  /* Stop and release the watcher */
};

typedef uv_stream_t event_stream;
struct event_tcp_watcher : event_watcher<uv_tcp_t> {
    function<void(event_tcp_watcher*)> callback;
    event_stream* accept();
};
struct event_stream_watcher : event_watcher<uv_stream_t> {
    function<void(event_stream_watcher*)> callback;
    ssize_t nread;
    const uv_buf_t* buffer;
    uv_stream_t* handle;
    void stop() { uv_read_stop(handle); }
};
struct event_connect_watcher : event_watcher<uv_connect_t> {
    function<void(event_connect_watcher*)> callback;
    uv_stream_t* handle;
};
using event_signal_watcher = event_watcher<uv_signal_t>;
using event_timer_watcher = event_watcher<uv_timer_t>;
using event_io_watcher = event_watcher<uv_poll_t>;
using event_async_watcher = event_watcher<uv_async_t>;
using event_write_watcher = event_watcher<uv_write_t>;

#ifdef ASYNC_REDIS
struct event_redis_watcher {
    redisAsyncContext* context;
    function<void(redisAsyncContext*, const string& value)> callback;
};
#endif

class events {
public:
    events();

    event_signal_watcher* onSignal(int signal,
                                   function<void(event_signal_watcher*)> callback);
    event_timer_watcher* onTimer(uint64_t timeout,
                                 uint64_t repeat,
                                 function<void(event_timer_watcher*)> callback);
    event_io_watcher* onRead(int fd,
                             function<void(event_io_watcher*)> callback);
    event_async_watcher* onAsync(function<void(event_async_watcher*)> callback);
    
    event_tcp_watcher* onListen(const std::string& iface_addr,
                                unsigned short port,
                                function<void(event_tcp_watcher*)> callback);
    static int write(uv_stream_t* tcp, const char* buf, size_t bytes);
    int onRead(event_stream* stream,
               function<void(event_stream_watcher*)> callback);
    int onConnect(const std::string& addr,
                  unsigned short port,
                  function<void(event_connect_watcher*)> callback);

#ifdef ASYNC_REDIS
    events(const string& redis_host, unsigned short redis_port);
    void onPop(const string& key,
               function<void(redisAsyncContext*,
                             const string& value)> callback,
               int timeout = 0);
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
    void sendAsync(event_async_watcher* event);

private:
    friend event_signal_watcher;
    friend event_timer_watcher;
    friend event_io_watcher;
    friend event_async_watcher;
    friend event_tcp_watcher;

    static void signal_callback(uv_signal_t* signal, int event);
    static void timer_callback(uv_timer_t* timer);
    static void poll_callback(uv_poll_t* handle, int status, int events);
    static void async_callback(uv_async_t* handle);
    static void listen_callback(uv_stream_t* server, int status);
    static void read_stream_callback(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buffer);
    static void connect_callback(uv_connect_t* request, int status);
    static void write_callback(uv_write_t* request, int status);

#ifdef ASYNC_REDIS
    static void redis_read_callback(redisAsyncContext *context, void *reply, void *data);
    static void redis_subscribe_callback(redisAsyncContext *context, void *reply, void *data);
#endif
    template<class EVENT_TYPE, class EVENT_WATCHER>
    static void callback(struct uv_loop* loop, EVENT_TYPE* event_handler, int event);

    template<class T>
    T* new_watcher(function<void(T*)> callback);

    uv_loop_t *loop = NULL;    

#ifdef ASYNC_REDIS
    vector<event_redis_watcher*> redis_watchers;    
    redisAsyncContext *redis, *redis_pubsub;
#endif
};
