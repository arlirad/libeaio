#include "eaio.hpp"
#include "eaio/coro.hpp"
#include "eaio/handle.hpp"

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
    coro<io_result> wait(dispatcher& d, await_handle<void>& on_what, F&& func, Args... args) {
        while (true) {
            // otherwise we risk a stack overflow
            if (d.suspend_counter++ % dispatcher::REST_INTERVAL == dispatcher::REST_INTERVAL - 1)
                co_await d.event_loop_rest;

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
}
