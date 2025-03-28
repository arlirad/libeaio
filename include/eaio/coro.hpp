#pragma once

#include <coroutine>
#include <optional>
#include <utility>

namespace eaio {
    struct background;

    template <typename T>
    struct coro {
        struct final_awaiter {
            bool await_ready() noexcept {
                return false;
            }

            template <typename U>
            auto await_suspend(std::coroutine_handle<U> caller) noexcept {
                return caller.promise().next;
            }

            void await_resume() noexcept {}
        };

        struct promise_type {
            T                       value;
            std::coroutine_handle<> next;
            bool                    done = false;

            coro<T> get_return_object() {
                return {std::coroutine_handle<promise_type>::from_promise(*this)};
            }

            std::suspend_always initial_suspend() noexcept {
                return {};
            }

            final_awaiter final_suspend() noexcept {
                this->done = true;
                return {};
            }

            void return_value(T v) noexcept {
                value = std::move(v);
            }

            void unhandled_exception() noexcept {}
        };

        struct awaiter {
            std::coroutine_handle<promise_type> handle;

            bool await_ready() noexcept {
                return this->handle.promise().done;
            }

            template <typename U>
            auto await_suspend(std::coroutine_handle<U> caller) noexcept {
                this->handle.promise().next = caller;
                return this->handle;
            }

            T await_resume() noexcept {
                return std::move(this->handle.promise().value);
            }
        };

        std::coroutine_handle<promise_type> handle;

        coro(std::coroutine_handle<promise_type> h) : handle(h) {}

        ~coro() {
            if (this->handle)
                this->handle.destroy();
        }

        T get_value() {
            return std::move(this->handle.promise().value);
        }

        auto operator co_await() {
            return awaiter{this->handle};
        }
    };

    template <>
    struct coro<void> {
        struct final_awaiter {
            bool await_ready() noexcept {
                return false;
            }

            template <typename U>
            auto await_suspend(std::coroutine_handle<U> caller) noexcept {
                return caller.promise().next;
            }

            void await_resume() noexcept {}
        };

        struct promise_type {
            std::coroutine_handle<> next;
            bool                    done = false;

            coro<void> get_return_object() {
                return {std::coroutine_handle<promise_type>::from_promise(*this)};
            }

            std::suspend_always initial_suspend() noexcept {
                return {};
            }

            final_awaiter final_suspend() noexcept {
                this->done = true;
                return {};
            }

            void return_void() noexcept {}
            void unhandled_exception() noexcept {}
        };

        struct awaiter {
            std::coroutine_handle<promise_type> handle;

            bool await_ready() noexcept {
                return this->handle.promise().done;
            }

            template <typename U>
            auto await_suspend(std::coroutine_handle<U> caller) noexcept {
                this->handle.promise().next = caller;
                return this->handle;
            }

            void await_resume() noexcept {}
        };

        std::coroutine_handle<promise_type> handle;

        coro(std::coroutine_handle<promise_type> h) : handle(h) {}

        ~coro() {
            if (this->handle)
                this->handle.destroy();
        }

        auto operator co_await() {
            return awaiter{this->handle};
        }
    };

    template <>
    struct coro<background> {
        struct final_awaiter {
            bool await_ready() noexcept {
                return false;
            }

            template <typename U>
            auto await_suspend(std::coroutine_handle<U> caller) noexcept {}

            void await_resume() noexcept {}
        };

        struct promise_type {
            coro<background> get_return_object() {
                return {std::coroutine_handle<promise_type>::from_promise(*this)};
            }

            std::suspend_never initial_suspend() noexcept {
                return {};
            }

            final_awaiter final_suspend() noexcept {
                return {};
            }

            void return_void() noexcept {}
            void unhandled_exception() noexcept {}
        };

        std::coroutine_handle<promise_type> handle;

        coro(std::coroutine_handle<promise_type> h) : handle(h) {}
        ~coro() {}
    };

    template <typename T>
    struct await_handle {
        std::coroutine_handle<> chained = {};
        std::optional<T>        value;

        void notify() {
            if (!this->chained)
                return;

            auto handle = this->chained;

            this->chained = nullptr;
            handle.resume();
        }

        void notify(T value) {
            this->value = value;
            this->notify();
        }

        bool await_ready() {
            return this->value.has_value();
        }

        template <typename U>
        void await_suspend(std::coroutine_handle<U> caller) {
            this->chained = caller;
        }

        std::optional<T> await_resume() {
            auto val = std::move(this->value);
            this->value.reset();
            return std::move(val);
        }
    };

    template <>
    struct await_handle<void> {
        std::coroutine_handle<> chained = {};

        void notify() {
            if (!this->chained)
                return;

            auto handle = this->chained;

            this->chained = nullptr;
            handle.resume();
        }

        bool await_ready() {
            return false;
        }

        template <typename U>
        void await_suspend(std::coroutine_handle<U> caller) {
            this->chained = caller;
        }

        void await_resume() {}
    };

    struct coro_get_handle {
        std::coroutine_handle<> handle;

        bool await_ready() noexcept {
            return false;
        }

        bool await_suspend(std::coroutine_handle<> caller) noexcept {
            handle = caller;
            return false;
        }

        std::coroutine_handle<> await_resume() noexcept {
            return handle;
        }
    };
}