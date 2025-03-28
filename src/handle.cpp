#include "eaio/handle.hpp"

#include "eaio.hpp"

namespace eaio {
    handle::handle() {}
    handle::handle(int fd, dispatcher& o)
        : _fd(fd), _shared(std::make_shared<handle::shared>(fd, o)) {}

    handle::~handle() {}

    handle::shared::shared(int fd, dispatcher& o) : _fd(fd), _owner(o) {
        o.add(this->_fd, this);
    }

    handle::shared::~shared() {
        this->_owner.remove(this->_fd);
        close(this->_fd);
    }
}