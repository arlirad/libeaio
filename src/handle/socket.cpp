#include "eaio.hpp"
#include "io.hpp"

namespace eaio {
    coro<io_result> socket::send(void* buffer, size_t count) {
        co_return co_await wait(this->_shared->in_done, ::send, this->_fd, buffer, count, 0);
    }

    coro<io_result> socket::recv(void* buffer, size_t count) {
        co_return co_await wait(this->_shared->in_done, ::recv, this->_fd, buffer, count, 0);
    }

    io_result socket::listen(int backlog) {
        return call(::listen, this->_fd, backlog);
    }

    coro<accept_result> socket::accept() {
        auto result = co_await wait(this->_shared->in_done, ::accept, this->_fd, nullptr, nullptr);

        if (!result)
            co_return accept_result{
                .error = errno,
            };


        co_return accept_result{
            this->_shared->_owner.wrap<socket>(std::move(result.value)),
        };
    }

    io_result socket::bind(const sockaddr* addr, socklen_t addrlen) {
        return call(::bind, this->_fd, addr, addrlen);
    }

    io_result socket::setsockopt(int level, int optname, const void* optval, socklen_t len) {
        return call(::setsockopt, this->_fd, level, optname, optval, len);
    }

}