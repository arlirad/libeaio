#pragma once

#include <eaio/coro.hpp>

#include <errno.h>
#include <memory>
#include <string>

namespace eaio {
    class dispatcher;
    struct accept_result;

    struct io_result {
        union {
            ssize_t len;
            ssize_t value;
        };
        error_t error;

        std::string perror(const char* prefix);

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

        coro<io_result> read(char* buffer, size_t count);
        coro<io_result> read(void* buffer, size_t count);
        coro<io_result> write(const char* buffer, size_t count);
        coro<io_result> write(const void* buffer, size_t count);

        protected:
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

        friend class dispatcher;
    };
}