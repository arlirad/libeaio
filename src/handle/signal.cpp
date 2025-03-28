#include "eaio.hpp"
#include "io.hpp"

namespace eaio {
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