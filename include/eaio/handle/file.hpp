#pragma once

#include <eaio/handle.hpp>

#include <unistd.h>

namespace eaio {
    class file : public handle {
        public:
        using handle::handle;

        ssize_t tellg();
        ssize_t seekg(off_t offset);
        ssize_t seekg(off_t offset, int whence);
        ssize_t size();
        int     truncate(size_t length);
    };
}