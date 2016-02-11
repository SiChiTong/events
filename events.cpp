#include <csignal>
#include <cassert>
#include "events.hpp"

#ifdef ASYNC_REDIS
extern "C" {
    #include <hiredis.h>
    #include <async.h>
    #include <adapters/libev.h>
}
#endif

events::events() {
    this->loop = ev_loop_new(EVBACKEND_POLL | EVBACKEND_SELECT);
}

template<class T>
shared_ptr<T> events::new_watcher(function<void(T*)> callback) {
    shared_ptr<T> e_spec = make_shared<T>();
    e_spec->callback = callback;
    e_spec->self = this;
    e_spec->watcher.data = e_spec.get();
    
    return e_spec;
}

shared_ptr<event_signal_watcher> events::onSignal(int signal, function<void(event_signal_watcher*)> callback) {
    auto e_spec = new_watcher<event_signal_watcher>(callback);
    ev_signal_init(&e_spec->watcher, events::signal_callback, SIGINT);
    ev_signal_start(this->loop, &e_spec->watcher);
    signal_watchers.push_back(e_spec);
    return e_spec;
}

shared_ptr<event_timer_watcher> events::onTimer(ev_tstamp after, ev_tstamp repeat, function<void(event_timer_watcher*)> callback) {
    auto e_spec = new_watcher<event_timer_watcher>(callback);
    ev_timer_init(&e_spec->watcher, events::timer_callback, after, repeat);
    ev_timer_start(this->loop, &e_spec->watcher);
    timer_watchers.push_back(e_spec);
    return e_spec;
}

shared_ptr<event_io_watcher> events::onRead(int fd, function<void(event_io_watcher*)> callback) {
    auto e_spec = new_watcher<event_io_watcher>(callback);
    ev_io_init(&e_spec->watcher, events::io_callback, fd, EV_READ);
    ev_io_start(this->loop, &e_spec->watcher);
    io_watchers.push_back(e_spec);
    return e_spec;
}

shared_ptr<event_async_watcher> events::onAsync(function<void(event_async_watcher*)> callback) {
    auto e_spec = new_watcher<event_async_watcher>(callback);
    ev_async_init(&e_spec->watcher, events::async_callback);
    ev_async_start(this->loop, &e_spec->watcher);
    async_watchers.push_back(e_spec);
    return e_spec;
}

void events::run() {
    assert(this->loop);
    ev_run(this->loop, 0);
}

void events::stop() {
    assert(this->loop);
    ev_break(this->loop, EVBREAK_ALL);
}

template<> void event_watcher<ev_signal>::stop() {
    /* TODO event shall be removed from signal_watchers */
    ev_signal_stop(this->self->loop, &this->watcher);
}

template<> void event_watcher<ev_timer>::stop() {
    /* TODO event shall be removed from timer_watchers */
    ev_timer_stop(this->self->loop, &this->watcher);
}

template<> void event_watcher<ev_io>::stop() {
    /* TODO event shall be removed from io_watchers */
    ev_io_stop(this->self->loop, &this->watcher);
}

template<> void event_watcher<ev_async>::stop() {
    /* TODO event shall be removed from async_watchers */
    ev_async_stop(this->self->loop, &this->watcher);
}

void events::sendAsync(shared_ptr<event_async_watcher> event) {
    ev_async_send(this->loop, &event->watcher);
}

void events::signal_callback(struct ev_loop* loop, ev_signal* signal, int event) {
    auto e_spec = static_cast<event_signal_watcher*>(signal->data);
    auto callback = static_cast <function<void(event_signal_watcher*)>> (e_spec->callback);
    callback(e_spec);
}

void events::timer_callback(struct ev_loop* loop, ev_timer *timer, int event) {
    auto e_spec = static_cast<event_timer_watcher*>(timer->data);
    auto callback = static_cast <function<void(event_timer_watcher*)>> (e_spec->callback);
    callback(e_spec);
}

void events::io_callback(struct ev_loop* loop, ev_io *io, int event) {
    auto e_spec = static_cast<event_io_watcher*>(io->data);
    auto callback = static_cast <function<void(event_io_watcher*)>> (e_spec->callback);
    callback(e_spec);
}

void events::async_callback(struct ev_loop* loop, ev_async *async, int event) {
    auto e_spec = static_cast<event_async_watcher*>(async->data);
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
    this->loop = ev_loop_new(EVBACKEND_POLL | EVBACKEND_SELECT);
    redisLibevAttach(this->loop, this->redis);
    redisLibevAttach(this->loop, this->redis_pubsub);
}

void events::redis_read_callback(redisAsyncContext *context, void *reply, void *data) {
    auto _reply = static_cast<redisReply*>(reply);
    auto callback = *static_cast<function<void(const string& value)>*>(data);
    callback(_reply->element[1]->str);

    redisAsyncCommand(context, redis_read_callback, data, "BLPOP %s 0 ", _reply->element[0]->str);
}

void events::redis_subscribe_callback(redisAsyncContext *context, void *reply, void *data) {
    auto _reply = static_cast<redisReply*>(reply);

    if (string(_reply->element[0]->str) == "message") {
        auto callback = *static_cast<function<void(const string& value)>*>(data);
        callback(_reply->element[2]->str);
    }
}

shared_ptr<event_redis_watcher> events::onListPop(const string& key, function<void(const string& value)> callback) {
    auto e_spec = make_shared<event_redis_watcher>();
    e_spec->callback = callback;
    redis_watchers.push_back(e_spec);

    redisAsyncCommand(this->redis, redis_read_callback, &e_spec->callback, "BLPOP %s 0", key.c_str());
    return e_spec;
}

shared_ptr<event_redis_watcher> events::onSubscribe(const string& key, function<void(const string& value)> callback) {
    auto e_spec = make_shared<event_redis_watcher>();
    e_spec->callback = callback;
    redis_watchers.push_back(e_spec);

    redisAsyncCommand(this->redis_pubsub, redis_subscribe_callback, &e_spec->callback, "SUBSCRIBE %s", key.c_str());
    return e_spec;
}

#endif
