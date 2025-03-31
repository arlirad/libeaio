#pragma once

#include <eaio/handle.hpp>
#include <sys/socket.h>
#include <sys/types.h>

namespace eaio {
    struct accept_result;

    class socket : public handle {
        public:
        using handle::handle;

        io_result read()  = delete;
        io_result write() = delete;

        coro<io_result> send(const char* buffer, size_t count);
        coro<io_result> send(const void* buffer, size_t count);
        coro<io_result> recv(char* buffer, size_t count);
        coro<io_result> recv(void* buffer, size_t count);

        io_result           listen(int backlog);
        coro<accept_result> accept();

        int shutdown(int how);

        template <typename T>
        io_result bind(const T& addr) {
            return this->bind(reinterpret_cast<const sockaddr*>(&addr), sizeof(addr));
        }

        template <typename T>
        io_result setsockopt(int level, int optname, const T& val) {
            return this->setsockopt(level, optname, &val, sizeof(val));
        }

        io_result setsockopt(int level, int optname, const bool& val) {
            int v = val ? 1 : 0;
            return this->setsockopt(level, optname, &v, sizeof(v));
        }

        io_result setsockopt(int level, int optname, const void* optval, socklen_t len);
        io_result bind(const sockaddr* addr, socklen_t addrlen);
    };

    struct accept_result {
        socket  returned_handle;
        error_t error;

        std::string perror(const char* prefix);

        operator socket() {
            return returned_handle;
        }

        operator bool() {
            return returned_handle.is_valid();
        }
    };
}