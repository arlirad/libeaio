#pragma once

#include <eaio/coro.hpp>
#include <eaio/handle.hpp>
#include <sys/socket.h>
#include <sys/types.h>

#include <errno.h>
#include <memory>

namespace eaio {
    class dispatcher;
    struct accept_result;

    struct io_result {
        union {
            ssize_t len;
            ssize_t value;
        };
        error_t error;

        operator bool() {
            return len >= 0;
        }
    };

    class handle {
        public:
        handle();
        ~handle();

        constexpr bool is_valid() {
            return this->_fd != -1;
        }

        coro<io_result> read(void* buffer, size_t count);
        coro<io_result> write(const void* buffer, size_t count);

        coro<io_result> send(void* buffer, size_t count);
        coro<io_result> recv(void* buffer, size_t count);

        io_result           listen(int backlog);
        coro<accept_result> accept();

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

        private:
        struct shared {
            dispatcher& _owner;
            int         _fd = -1;

            await_handle<void> in_done;
            await_handle<void> out_done;
            await_handle<void> errored;

            shared(int fd, dispatcher& o);
            ~shared();
        };

        std::shared_ptr<shared> _shared;
        int                     _fd;

        handle(int fd, dispatcher& o);

        io_result setsockopt(int level, int optname, const void* optval, socklen_t len);
        io_result bind(const sockaddr* addr, socklen_t addrlen);

        friend class dispatcher;
    };

    class socket : public handle {
        io_result read()  = delete;
        io_result write() = delete;
    };

    struct accept_result {
        handle  returned_handle;
        error_t error;

        operator handle() {
            return returned_handle;
        }

        operator bool() {
            return returned_handle.is_valid();
        }
    };

}