#pragma once

#include <eaio/coro.hpp>
#include <eaio/handle.hpp>
#include <sys/epoll.h>

#include <vector>

namespace eaio {
    class dispatcher {
        public:
        static const int MAX_EVENTS = 32;

        dispatcher();
        ~dispatcher();

        void add(int fd, handle::shared* h);
        void remove(int fd);

        template <typename T, typename... U>
        void spawn(T&& f, U&&... args) {
            call_def(f, args...);
        }

        handle wrap(int fd);
        void   poll();

        private:
        using event_slot = epoll_event;

        int                                  _fd;
        event_slot                           _events[MAX_EVENTS];
        std::vector<std::coroutine_handle<>> _to_cleanup;

        template <typename T, typename... U>
        coro<background> call_def(T&& f, U&&... args) {
            auto c = f(args...);
            co_await c;

            auto handle = co_await coro_get_handle();
            _to_cleanup.push_back(handle);
        }

        void resume(event_slot& ev, handle::shared* h);
    };
}