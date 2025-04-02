#include "eaio.hpp"
#include "io.hpp"

#include <format>
#include <string.h>
#include <unistd.h>

namespace eaio {
    std::string io_result::perror(const char* prefix) {
        return std::format("{}: {}.", prefix, strerror(this->error));
    }

    handle::handle() {}
    handle::handle(int fd, dispatcher& o)
        : _fd(fd), _shared(std::make_shared<handle::shared>(fd, o)) {}

    handle::~handle() {}

    coro<io_result> handle::read(char* buffer, size_t count) {
        co_return co_await wait(this->_shared->_owner, this->_shared->in_done, ::read, this->_fd,
                                buffer, count);
    }

    coro<io_result> handle::read(void* buffer, size_t count) {
        co_return co_await wait(this->_shared->_owner, this->_shared->in_done, ::read, this->_fd,
                                buffer, count);
    }

    coro<io_result> handle::write(const char* buffer, size_t count) {
        co_return co_await wait(this->_shared->_owner, this->_shared->out_done, ::write, this->_fd,
                                buffer, count);
    }

    coro<io_result> handle::write(const void* buffer, size_t count) {
        co_return co_await wait(this->_shared->_owner, this->_shared->out_done, ::write, this->_fd,
                                buffer, count);
    }

    int handle::close() {
        return ::close(this->_fd);
    }

    handle::shared::shared(int fd, dispatcher& o) : _fd(fd), _owner(o) {
        o.add(this->_fd, this);
    }

    handle::shared::~shared() {
        this->_owner.remove(this->_fd);
        ::close(this->_fd);
    }
}