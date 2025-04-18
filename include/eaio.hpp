#pragma once

#include <eaio/coro.hpp>
#include <eaio/handle.hpp>
#include <eaio/handle/file.hpp>
#include <eaio/handle/signal.hpp>
#include <eaio/handle/socket.hpp>
#include <sys/epoll.h>

#include <atomic>
#include <functional>
#include <vector>

namespace eaio {
    constexpr auto FLAG_IN  = ::EPOLLIN;
    constexpr auto FLAG_OUT = ::EPOLLOUT;
    constexpr auto FLAG_ERR = ::EPOLLERR;

    class custom : public handle {
        public:
        using handle::handle;

        custom(int fd, dispatcher& o, std::function<void(uint32_t)> cb) : handle(fd, o) {
            this->_shared->_cb = cb;
        }

        io_result read()  = delete;
        io_result write() = delete;
    };

    class dispatcher {
        public:
        struct rester {
            dispatcher& owner;

            rester(dispatcher& d) : owner(d) {}

            bool await_ready() {
                return false;
            }

            void await_suspend(std::coroutine_handle<> caller) {
                owner._resters.push_back(caller);
            }

            void await_resume() {}
        };

        static const int      MAX_EVENTS    = 128;
        static const uint32_t REST_INTERVAL = 1024;
        std::atomic<uint32_t> suspend_counter;
        rester                event_loop_rest = {*this};

        dispatcher();
        ~dispatcher();

        void add(int fd, handle::shared* h);
        void remove(int fd);

        template <typename T, typename... U>
        void spawn(T&& f, U&&... args) {
            call_def(f, args...).handle.resume();
        }

        template <typename T>
        T wrap(int fd) {
            this->prepare_fd(fd);
            return T(fd, *this);
        }

        custom wrap(int fd, std::function<void(uint32_t)> fun) {
            this->prepare_fd(fd);
            return custom(fd, *this, fun);
        }

        void poll();

        private:
        using event_slot = epoll_event;

        int                                  _fd;
        event_slot                           _events[MAX_EVENTS];
        std::vector<std::coroutine_handle<>> _to_cleanup;
        std::vector<std::coroutine_handle<>> _resters;
        std::vector<std::coroutine_handle<>> _resters_iterated;

        template <typename T, typename... U>
        coro<background> call_def(T&& f, U&&... args) {
            auto c = f(args...);
            co_await c;

            auto handle = co_await coro_get_handle();
            _to_cleanup.push_back(handle);
        }

        void resume(event_slot& ev, handle::shared* h);
        void prepare_fd(int fd);
    };
}