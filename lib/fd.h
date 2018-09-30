#ifndef FD_H
#define FD_H

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "nocopy.h"

class FD : NoCopy {
    int fd = -1;

public:
    FD() = default;
    FD(const char *path, int flags, mode_t mode = 0666) : fd(open(path, flags, mode)) {
        if( fd<0 )
            throw std::runtime_error("Open failed");
    }

    FD(const std::string &path, int flags, mode_t mode = 0666 ) : FD(path.c_str(), flags, mode) {}

    // Move
    FD(FD &&rhs) : fd(rhs.fd) {
        rhs.fd = -1;
    }

    FD &operator=(FD &&rhs) {
        std::swap(fd, rhs.fd);

        return *this;
    }

    ~FD() {
        if( fd>=0 )
            close(fd);
    }

    int get() {
        return fd;
    }
};

#endif // FD_H
