#include "eaio/handle.hpp"

#include "eaio.hpp"

namespace eaio {
    template <typename F, typename... Args>
    io_result call(F&& func, Args... args) {
        auto ret = func(args...);
        int  err = errno;

        if (ret >= 0)
            err = 0;

        return {
            .value = static_cast<ssize_t>(ret),
            .error = errno,
        };
    }

    template <typename F, typename... Args>
    coro<io_result> wait(await_handle<void>& on_what, F&& func, Args... args) {
        while (true) {
            auto ret = func(args...);

            if (ret < 0 && (errno == EWOULDBLOCK || errno == EAGAIN)) {
                co_await on_what;
                continue;
            }
            else {
                co_return {
                    .value = static_cast<ssize_t>(ret),
                    .error = errno,
                };
            }
        }
    }

    handle::handle() {}
    handle::handle(int fd, dispatcher& o)
        : _fd(fd), _shared(std::make_shared<handle::shared>(fd, o)) {}

    handle::~handle() {}

    coro<io_result> handle::send(void* buffer, size_t count) {
        co_return co_await wait(this->_shared->in_done, ::send, this->_fd, buffer, count, 0);
    }

    coro<io_result> handle::recv(void* buffer, size_t count) {
        co_return co_await wait(this->_shared->in_done, ::recv, this->_fd, buffer, count, 0);
    }

    io_result handle::listen(int backlog) {
        return call(::listen, this->_fd, backlog);
    }

    coro<accept_result> handle::accept() {
        auto result = co_await wait(this->_shared->in_done, ::accept, this->_fd, nullptr, nullptr);

        if (!result)
            co_return accept_result{
                .error = errno,
            };


        co_return accept_result{
            this->_shared->_owner.wrap(std::move(result.value)),
        };
    }

    io_result handle::bind(const sockaddr* addr, socklen_t addrlen) {
        return call(::bind, this->_fd, addr, addrlen);
    }

    io_result handle::setsockopt(int level, int optname, const void* optval, socklen_t len) {
        return call(::setsockopt, this->_fd, level, optname, optval, len);
    }

    handle::shared::shared(int fd, dispatcher& o) : _fd(fd), _owner(o) {
        o.add(this->_fd, this);
    }

    handle::shared::~shared() {
        this->_owner.remove(this->_fd);
        close(this->_fd);
    }
}