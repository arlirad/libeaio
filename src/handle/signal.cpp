#include "eaio.hpp"
#include "io.hpp"

#include <format>
#include <string.h>

namespace eaio {
    std::string get_result::perror(const char* prefix) {
        return std::format("{}: {}.", prefix, strerror(this->error));
    }

    coro<get_result> signal::get() {
        signalfd_siginfo info;

        auto result = co_await this->read(&info, sizeof(info));

        co_return {
            .valid = result.len == sizeof(info),
            .info  = info,
            .error = result.error,
        };
    }
}