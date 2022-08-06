/*
 * @Author       : fan
 * @Date         : 2022-08-05
 * @copyleft Apache 2.0
 */

#include "buffer.h"

Buffer::Buffer(int initBufferSize): buffer_(initBufferSize), readPos_(0), writePos_(0) {}

char* Buffer::BeginPtr_() {
    return &*buffer_.begin();
}

const char* Buffer::BeginPtr_() const {
    return &*buffer_.begin();
}

void Buffer::MakeSpace_(size_t len) {
    // 若空间不足 则resize buffer
    if (WritableBytes() + HasReadBytes() < len)
        buffer_.resize(writePos_ + len + 1);
    // 整体左shift 覆盖已读空间
    else {
        size_t readable = ReadableBytes();
        std::copy(BeginPtr_() + readPos_, BeginPtr_() + writePos_, BeginPtr_());
        readPos_ = 0;
        writePos_ = readPos_ + readable;

        assert(readable == ReadableBytes());
    }
}

size_t Buffer::WritableBytes() const {
    return buffer_.size() - writePos_;
}

size_t Buffer::ReadableBytes() const {
    return writePos_ - readPos_;
}

size_t Buffer::HasReadBytes() const {
    return readPos_;
}

const char* Buffer::Peek() const {
    return BeginPtr_() + readPos_;
}

void Buffer::ToWritten(size_t len) {
    assert(len <= WritableBytes());
    writePos_ += len;
}

void Buffer::EnsureWritable(size_t len) {
    if (WritableBytes() < len) MakeSpace_(len);
    assert(WritableBytes() >= len);
}

void Buffer::ToRead(size_t len) {
    assert(len <= ReadableBytes());
    readPos_ += len;
}

void Buffer::ToReadUtil(const char *end) {
    assert(Peek() <= end);
    ToRead(end - Peek());
}

void Buffer::ToReadAll() {
    bzero(&buffer_[0], buffer_.size());
    readPos_ = writePos_ = 0;
}

std::string Buffer::ReadAll2Str() {
    std::string str(Peek(), ReadableBytes());
    ToReadAll();
    return str;
}

char* Buffer::BeginWrite() {
    return BeginPtr_() + writePos_;
}

const char* Buffer::BeginWriteConst() const {
    return BeginPtr_() + writePos_;
}

void Buffer::Append(const char *str, size_t len) {
    assert(str);
    EnsureWritable(len);
    std::copy(str, str + len, BeginWrite());
    ToWritten(len);
}

void Buffer::Append(const void *data, size_t len) {
    assert(data);
    Append(static_cast<const char*>(data), len);
}

void Buffer::Append(const std::string str) {
    Append(str.c_str(), str.size());
}

void Buffer::Append(const Buffer &buff) {
    Append(buff.Peek(), buff.ReadableBytes());
}

ssize_t Buffer::WriteFd(int fd, int *saveErrno) {
    ssize_t len = write(fd, Peek(), ReadableBytes());

    if (len < 0) *saveErrno = errno;
    else readPos_ += len;

    return len;
}

ssize_t Buffer::ReadFd(int fd, int *saveErrno) {
    char buff[65535];
    struct iovec iov[2];
    const size_t writeSize = WritableBytes();

    iov[0].iov_base = BeginWrite();
    iov[0].iov_len = writeSize;
    iov[1].iov_base = buff;
    iov[1].iov_len = sizeof(buff);

    const ssize_t len = readv(fd, iov, 2);

    if (len < 0) 
        *saveErrno = errno;
    else if (static_cast<size_t>(len) <= writeSize)
        ToWritten(static_cast<size_t>(len));
    else {
        writePos_ = buffer_.size();
        Append(buff, len - writeSize);
    }

    return len;
}