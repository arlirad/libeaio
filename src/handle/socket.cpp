#include "eaio.hpp"
#include "io.hpp"

#include <format>
#include <string.h>

namespace eaio {
    std::string accept_result::perror(const char* prefix) {
        return std::format("{}: {}.", prefix, strerror(this->error));
    }

    coro<io_result> socket::send(const char* buffer, size_t count) {
        co_return co_await wait(this->_shared->out_done, ::send, this->_fd, buffer, count, 0);
    }

    coro<io_result> socket::send(const void* buffer, size_t count) {
        co_return co_await wait(this->_shared->out_done, ::send, this->_fd, buffer, count, 0);
    }

    coro<io_result> socket::recv(char* buffer, size_t count) {
        co_return co_await wait(this->_shared->in_done, ::recv, this->_fd, buffer, count, 0);
    }

    coro<io_result> socket::recv(void* buffer, size_t count) {
        co_return co_await wait(this->_shared->in_done, ::recv, this->_fd, buffer, count, 0);
    }

    io_result socket::listen(int backlog) {
        return call(::listen, this->_fd, backlog);
    }

    coro<accept_result> socket::accept() {
        auto result = co_await wait(this->_shared->in_done, ::accept, this->_fd, nullptr, nullptr);

        if (!result) {
            co_return accept_result{
                .error = errno,
            };
        }

        co_return accept_result{
            this->_shared->_owner.wrap<socket>(std::move(result.value)),
        };
    }

    int socket::shutdown(int how) {
        return ::shutdown(this->_fd, how);
    }

    io_result socket::bind(const sockaddr* addr, socklen_t addrlen) {
        return call(::bind, this->_fd, addr, addrlen);
    }

    io_result socket::setsockopt(int level, int optname, const void* optval, socklen_t len) {
        return call(::setsockopt, this->_fd, level, optname, optval, len);
    }

}