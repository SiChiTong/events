#include <csignal>
#include <cassert>

#include "events.hpp"

events::events() {
    this->loop = ev_default_loop(EVBACKEND_POLL | EVBACKEND_SELECT);
}

template<class T>
shared_ptr<T> events::new_watcher(function<void(T*)> callback) {
    shared_ptr<T> e_spec = make_shared<T>();
    e_spec->callback = callback;
    e_spec->self = this;
    e_spec->watcher.data = e_spec.get();
    
    return e_spec;
}

template<class EVENT_TYPE, class EVENT_WATCHER>
void events::callback(struct ev_loop* loop, EVENT_TYPE* event_handler, int event) {
    auto e_spec = static_cast<EVENT_WATCHER*>(event_handler->data);
    auto _callback = static_cast<function<void(events*)>>(e_spec->callback);
    _callback(e_spec->self);
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

void events::run() {
    assert(this->loop);
    ev_run(this->loop, 0);
}

void events::stop() {
    assert(this->loop);
    ev_break(this->loop, EVBREAK_ALL);
}

void events::stopTimer(event_timer_watcher* watcher) {
    ev_timer_stop(watcher->self->loop, &watcher->watcher);
}

void events::stopTimer(shared_ptr<event_timer_watcher> watcher) {
    ev_timer_stop(watcher->self->loop, &watcher->watcher);
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
