#include <csignal>
#include <cassert>
#include "events.hpp"

#ifdef ASYNC_REDIS
extern "C" {
    #include <hiredis.h>
    #include <async.h>
    #include <adapters/libuv.h>
}
#endif

events::events() {
    this->loop = uv_loop_new();
}

template<class T>
T* events::new_watcher(function<void(T*)> callback) {
    T* e_spec = new T();
    e_spec->callback = callback;
    e_spec->self = this;
    e_spec->watcher.data = e_spec;
    
    return e_spec;
}

event_signal_watcher* events::onSignal(int signal, function<void(event_signal_watcher*)> callback) {
    auto e_spec = new_watcher<event_signal_watcher>(callback);

    uv_signal_init(this->loop, &e_spec->watcher);
    uv_signal_start(&e_spec->watcher, events::signal_callback, signal);
    
    return e_spec;
}

event_timer_watcher* events::onTimer(uint64_t timeout,
                                     uint64_t repeat,
                                     function<void(event_timer_watcher*)> callback) {
    auto e_spec = new_watcher<event_timer_watcher>(callback);
    
    uv_timer_init(this->loop, &e_spec->watcher);
    uv_timer_start(&e_spec->watcher, events::timer_callback, timeout, repeat);
    
    return e_spec;
}

event_io_watcher* events::onRead(int fd,
                                            function<void(event_io_watcher*)> callback) {
    auto e_spec = new_watcher<event_io_watcher>(callback);
    
    uv_poll_init(this->loop, &e_spec->watcher, fd);
    uv_poll_start(&e_spec->watcher, UV_READABLE | UV_DISCONNECT, events::poll_callback);
    
    return e_spec;
}

event_async_watcher* events::onAsync(function<void(event_async_watcher*)> callback) {
    auto e_spec = new_watcher<event_async_watcher>(callback);
    
    uv_async_init(this->loop, &e_spec->watcher, events::async_callback);
    
    return e_spec;
}

void events::run() {
    assert(this->loop);
    uv_run(this->loop, UV_RUN_DEFAULT);
}

void events::stop() {
    assert(this->loop);
    uv_stop(this->loop);
}

template<> void event_watcher<uv_signal_t>::stop() {
    /* TODO event shall be removed from signal_watchers */
    uv_signal_stop(&this->watcher);
}

template<> void event_watcher<uv_timer_t>::stop() {
    /* TODO event shall be removed from timer_watchers */
    uv_timer_stop(&this->watcher);
}

template<> void event_watcher<uv_poll_t>::stop() {
    uv_poll_stop(&this->watcher);
}

template<> void event_watcher<uv_poll_t>::release() {
    this->stop();
    delete this;
}

void events::sendAsync(event_async_watcher* event) {
    uv_async_send(&event->watcher);
}

void events::signal_callback(uv_signal_t* signal, int event) {
    auto e_spec = static_cast<event_signal_watcher*>(signal->data);
    auto callback = static_cast <function<void(event_signal_watcher*)>> (e_spec->callback);
    callback(e_spec);
}

void events::timer_callback(uv_timer_t* timer) {
    auto e_spec = static_cast<event_timer_watcher*>(timer->data);
    auto callback = static_cast <function<void(event_timer_watcher*)>> (e_spec->callback);
    callback(e_spec);
}

void events::poll_callback(uv_poll_t* handle, int status, int events) {
    auto e_spec = static_cast<event_io_watcher*>(handle->data);
    auto callback = static_cast <function<void(event_io_watcher*)>> (e_spec->callback);
    callback(e_spec);
}

void events::async_callback(uv_async_t* handle) {
    auto e_spec = static_cast<event_async_watcher*>(handle->data);
    auto callback = static_cast <function<void(event_async_watcher*)>> (e_spec->callback);
    callback(e_spec);
}

#ifdef ASYNC_REDIS
events::events(const string& redis_host, unsigned short redis_port) {
    this->redis = redisAsyncConnect(redis_host.c_str(), redis_port);
    this->redis_pubsub = redisAsyncConnect(redis_host.c_str(), redis_port);
    if (this->redis->err || this->redis_pubsub->err) {
        throw string("Redis Async cannot connect.");
    }
    this->loop = uv_loop_new();
    redisLibuvAttach(this->redis, this->loop);
    redisLibuvAttach(this->redis_pubsub, this->loop);
}

void events::redis_read_callback(redisAsyncContext *context, void *reply, void *data) {
    auto _reply = static_cast<redisReply*>(reply);
    auto e_spec = static_cast<event_redis_watcher*>(data);

    e_spec->callback(context, _reply->element[1]->str);
    redisAsyncCommand(e_spec->context, redis_read_callback, data, "BLPOP %s 0 ", _reply->element[0]->str);
}

void events::redis_subscribe_callback(redisAsyncContext *context, void *reply, void *data) {
    auto _reply = static_cast<redisReply*>(reply);

    if (string(_reply->element[0]->str) == "message") {
        auto e_spec = static_cast<event_redis_watcher*>(data);
        e_spec->callback(e_spec->context, _reply->element[2]->str);
    }
}

void events::onSubscribe(const string& key, 
                         function<void(redisAsyncContext*,
                                       const string& value)> callback) {
    auto e_spec = new event_redis_watcher;
    e_spec->context = this->redis;
    e_spec->callback = callback;
    redis_watchers.push_back(e_spec);
    
    redisAsyncCommand(this->redis_pubsub, redis_subscribe_callback, (void*) redis_watchers.back(), "SUBSCRIBE %s", key.c_str());
}

void events::onPop(const string& key,
                   function<void(redisAsyncContext*,
                                 const string& value)> callback,
                   int timeout) {
    auto e_spec = new event_redis_watcher;
    e_spec->context = this->redis;
    e_spec->callback = callback;
    redis_watchers.push_back(e_spec);
    
    redisAsyncCommand(this->redis, redis_read_callback, (void*) redis_watchers.back(), "BLPOP %s %d", key.c_str(), timeout);
};

void events::unsubscribe(const string& key) {
    redisAsyncCommand(this->redis_pubsub, NULL, NULL, "UNSUBSCRIBE %s", key.c_str());
}

#endif
