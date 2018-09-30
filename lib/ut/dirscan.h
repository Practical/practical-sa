#ifndef DIRSCAN_H
#define DIRSCAN_H

#include <sys/types.h>
#include <dirent.h>

#include <string.h>

#include "nocopy.h"

class DirScan : NoCopy {
    std::string path;
    std::string suffix;

    class Iterator {
        DIR *handle;
        std::string suffix;
        dirent *entry;

    public:
        Iterator() = default;

        Iterator(std::string path, std::string suffix) : handle(opendir(path.c_str())), suffix(suffix) {
            if( handle==nullptr ) {
                throw std::runtime_error("opendir failed");
            }

            ++(*this);
        }

        ~Iterator() {
            closehandle();
        }

        bool operator==(const Iterator &rhs) const {
            return handle==nullptr && rhs.handle==nullptr;
        }

        bool operator!=(const Iterator &rhs) const {
            return !((*this)==rhs);
        }

        const dirent &operator*() const {
            return *entry;
        }

        Iterator &operator++() {
            do {
                entry = readdir(handle);
            } while(entry!=nullptr && !suffixCorrect());

            if( entry==nullptr )
                closehandle();

            return *this;
        }
    private:
        void closehandle() {
            if( handle!=nullptr )
                closedir(handle);

            handle = nullptr;
            entry = nullptr;
        }

        bool suffixCorrect() const {
            size_t len = strlen(entry->d_name);
            if( len<suffix.length() )
                return false;

            return strcmp( entry->d_name+len-suffix.length(), suffix.c_str() )==0;
        }
    };
public:
    DirScan(const DirScan &rhs) = delete;
    DirScan &operator=(const DirScan &rhs) = delete;

    DirScan(std::string path, std::string suffix) : path(path), suffix(suffix) {
    }

    auto begin() const {
        return Iterator(path, suffix);
    }

    auto end() const {
        return Iterator();
    }
};

#endif // DIRSCAN_H
