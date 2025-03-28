#pragma once

#include <eaio/handle.hpp>
#include <sys/signalfd.h>

namespace eaio {
    struct get_result;

    class signal : public handle {
        public:
        using handle::handle;

        io_result read()  = delete;
        io_result write() = delete;

        coro<get_result> get();

        private:
        using handle::read;
    };

    struct get_result {
        bool             valid;
        signalfd_siginfo info;
        error_t          error;

        std::string perror(const char* prefix);

        operator signalfd_siginfo() {
            return info;
        }

        operator bool() {
            return valid;
        }
    };
}