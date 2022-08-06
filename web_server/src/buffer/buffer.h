/*
 * @Author       : fan
 * @Date         : 2022-08-05
 * @copyleft Apache 2.0
 */

#ifndef BUFFER_H
#define BUFFER_H

#include <iostream>
#include <cstring>
#include <vector>
#include <atomic>
#include <assert.h>
#include <unistd.h> // write
#include <sys/uio.h> // iovec readv

class Buffer {
public:
    Buffer(int initBufferSize = 1024);
    ~Buffer() = default;

    size_t WritableBytes() const; // 可写字节数
    size_t ReadableBytes() const; // 可读字节数
    size_t HasReadBytes() const;  // 已读字节数

    const char* Peek() const;     // 读指针位置
    void ToWritten(size_t len);   // 移动写指针位置
    // 保证Buffer有空间可写，没有则resize buffer;
    void EnsureWritable(size_t len);

    void ToRead(size_t len); // 移动读指针位置
    void ToReadUtil(const char *end);
    void ToReadAll();
    std::string ReadAll2Str();

    void Append(const char *str, size_t len);
    void Append(const void *data, size_t len);
    void Append(const std::string str);
    void Append(const Buffer &buff);

    ssize_t ReadFd(int fd, int *saveErrno);
    ssize_t WriteFd(int fd, int *saveErrno);

    char* BeginWrite();
    const char* BeginWriteConst() const;

private:
    char* BeginPtr_();
    const char* BeginPtr_() const;

    void MakeSpace_(size_t len);

    std::vector<char> buffer_;
    std::atomic<size_t> readPos_;
    std::atomic<size_t> writePos_;
};


#endif //BUFFER_H