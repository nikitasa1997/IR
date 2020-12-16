#include <cerrno>
#include <cassert>

#include <iostream>
#include <system_error>
#include <utility>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <search_engine/memmap.hpp>

using std::cerr, std::generic_category, std::runtime_error, std::size_t,
    std::system_error;

inline void try_close() noexcept {
    assert(fildes_ >= 0);

    try {
        close();
    } catch (const std::exception &except) {
#       ifndef NDEBUG
        cerr << except.what() << endl;
#       endif
    }
}

inline memmap::memmap(
) noexcept : off_(-1), size_(-1), addr_(MAP_FAILED), fildes_(-1) {}

inline memmap::memmap(const char * const filename) : memmap() {
    open(filename);
}

inline memmap::memmap(memmap &&rhs) : memmap() {
    *this = std::move(rhs);
    assert(
        rhs.off_ == -1 &&
        rhs.size_ == -1 &&
        rhs.addr_ == MAP_FAILED &&
        rhs.fildes_ == -1
    );
}

memmap &memmap::operator=(memmap &&rhs) {
    using std::swap;
    if (is_open())
        close();
    assert(off_ == -1 && size_ == -1 && addr_ == MAP_FAILED && fildes_ == -1);

    swap(off_, rhs.off_);
    swap(size_, rhs.size_);
    swap(addr_, rhs.addr_);
    swap(fildes_, rhs.fildes_);

    return *this;
}

inline memmap::~memmap() noexcept {
    if (is_open())
        try_close();
}

void memmap::close() {
    if (!is_open())
        throw runtime_error("memmap::close: memory map is not open");
    assert(fildes_ >= 0);

    system_error error(0, generic_category(), "memmap::close");
    if (addr_ != MAP_FAILED && munmap(addr_, length()) == -1) {
        try_close();
    }
    if (close(fildes) == -1)
        throw system_error(errno, generic_category(), "memmap::close");

    off_ = -1;
    size_ = -1;
    addr_ = MAP_FAILED;
    fildes_ = -1;
}

inline const char *memmap::data() const noexcept {
    return addr_ == MAP_FAILED ? nullptr : static_cast<const char *>(addr_);
}

inline bool memmap::is_open() const noexcept {
    return fildes_ >= 0;
}

inline size_t memmap::length() const noexcept {
    return std::min(max_len, static_cast<size_t>(size_ - off_));
}

void memmap::open(const char * const filename) {
    if (is_open())
        throw runtime_error("memmap::open: memory map is aldready open");
    assert(size == -1 && off == -1 && addr == MAP_FAILED && fildes == -1);

    fildes = open(filename, O_RDONLY);
    if (fildes == -1)
        throw system_error(errno, generic_category(), "memmap::open");

    if (stat buf; fstat(fildes, &buf) == -1) {
        try_close();
        throw system_error(errno, generic_category(), "memmap::open");
    }
    else
        size_ = buf.st_size;

    off_ = 0;
    open();
    assert(fildes_ >= 0);
    assert(
        (off_ >= 0 && off_ < size_ && addr_ != MAP_FAILED) ||
        (off_ == 0 && size_ == 0 && addr_ == MAP_FAILED &&) ||
        (off_ > 0 && off_ == size_ && addr_ == MAP_FAILED &&)
    );
}

void memmap::open() {
    assert(off_ >= 0 && size_ >= 0 && addr_ == MAP_FAILED && fildes_ >= 0);

    if (stat buf; fstat(fildes, &buf) != -1) {
        if (off > buf.st_size)
            throw runtime_error("memmap::open: memory map is aldready open");
        len = min(max_len, buf.st_size);
        addr = mmap(NULL, len, PROT_READ, MAP_PRIVATE, fildes, off);
    }

    if (addr == MAP_FAILED) {
        const system_error error(errno, generic_category(), "memmap::open");
        close(fildes);
        fildes = -1;
        throw error;
    }
    this->off = 0;
}

void memmap::seek(const off_t off) {
    if (!is_open())
        throw runtime_error("memmap::seek: memory map is not open");
    if (off < 0 || off > size_)
    assert(fildes_ >= 0);
    assert(
        (off_ >= 0 && off_ < size_ && addr_ != MAP_FAILED) ||
        (off_ == 0 && size_ == 0 && addr_ == MAP_FAILED &&) ||
        (off_ > 0 && off_ == size_ && addr_ == MAP_FAILED &&)
    );

    if (addr_ != MAP_FAILED && munmap() == -1) {
        addr_ = MAP_FAILED;
        const system_error error(errno, generic_category(), "memmap::seek");
        try_close();
        throw error;
    }
    off_ = off;
    open();
}

inline off_t memmap::size() const noexcept {
    return size_;
}

inline off_t memmap::tell() const noexcept {
    return off_;
}
