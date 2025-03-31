#include "eaio.hpp"
#include "io.hpp"

#include <format>
#include <string.h>

namespace eaio {
    ssize_t file::tellg() {
        return ::lseek(this->_fd, 0, SEEK_CUR);
    }

    ssize_t file::seekg(off_t offset) {
        return ::lseek(this->_fd, offset, SEEK_SET);
    }

    ssize_t file::size() {
        off_t   current = this->tellg();
        ssize_t size    = ::lseek(this->_fd, 0, SEEK_END);

        this->seekg(current);
        return size;
    }

    int file::truncate(size_t length) {
        return ::ftruncate(this->_fd, length);
    }
}