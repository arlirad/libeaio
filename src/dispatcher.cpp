#include "eaio.hpp"

#include <fcntl.h>
#include <unistd.h>

namespace eaio {
    dispatcher::dispatcher() {
        this->_fd = epoll_create1(0);
    }

    dispatcher::~dispatcher() {
        close(this->_fd);
    }

    void dispatcher::add(int fd, handle::shared* h) {
        epoll_event ev;

        ev.events   = EPOLLIN | EPOLLOUT | EPOLLET;
        ev.data.fd  = h->_fd;
        ev.data.ptr = reinterpret_cast<void*>(h);

        epoll_ctl(this->_fd, EPOLL_CTL_ADD, h->_fd, &ev);
    }

    void dispatcher::remove(int fd) {
        epoll_event ev;

        ev.events   = EPOLLIN | EPOLLOUT | EPOLLET;
        ev.data.fd  = fd;
        ev.data.ptr = nullptr;

        // Before Linux 2.6.9, the EPOLL_CTL_DEL operation required a non-
        // null pointer in event, even though this argument is ignored.
        epoll_ctl(this->_fd, EPOLL_CTL_DEL, fd, &ev);
    }

    void dispatcher::poll() {
        this->_resters_iterated = this->_resters;
        this->_resters.clear();

        for (auto& rester : _resters_iterated) {
            if (!rester)
                continue;

            rester.resume();
        }

        int nfds = epoll_wait(this->_fd, this->_events, this->MAX_EVENTS, -1);

        for (int i = 0; i < nfds; i++) {
            epoll_event& event = this->_events[i];
            auto         sh    = reinterpret_cast<handle::shared*>(event.data.ptr);

            if (sh->_cb) {
                sh->_cb(event.events);
                continue;
            }

            this->resume(event, sh);
        }

        for (auto handle : this->_to_cleanup) {
            if (!handle.done())
                handle.resume();

            handle.destroy();
        }

        this->_to_cleanup.clear();
    }

    void dispatcher::prepare_fd(int fd) {
        fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
    }

    void dispatcher::resume(dispatcher::event_slot& rv, handle::shared* h) {
        if (rv.events & FLAG_IN)
            h->in_done.notify();

        if (rv.events & FLAG_OUT)
            h->out_done.notify();
    }
}