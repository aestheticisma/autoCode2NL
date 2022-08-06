/*
 * @Author       : fan
 * @Date         : 2022-08-06
 * @copyleft Apache 2.0
 */

#include "httpconn.h"

bool HttpConn::isET;
std::atomic<int> HttpConn::userCount;

HttpConn::HttpConn(): fd_(-1), addr_({0}), isClose_(true) {}

HttpConn::~HttpConn() {
    Close();
}

void HttpConn::Close() {
    if (isClose_ = false) {
        isClose_ = true;
        --userCount;
        close(fd_);
        LOG_INFO("Client[%d](%s:%d) quit, UserCount:%d", fd_, GetIP(), GetPort(), (int)userCount);
    }
}

int HttpConn::GetFd() const {
    return fd_;
}

int HttpConn::GetPort() const {
    return addr_.sin_port;
}

const char* HttpConn::GetIP() const {
    return inet_ntoa(addr_.sin_addr);
}

sockaddr_in HttpConn::GetAddr() const {
    return addr_;
}

void HttpConn::Init(int fd, const sockaddr_in& addr) {
    assert(fd > 0);
    isClose_ = false;
    ++userCount;
    fd_ = fd;
    addr_ = addr;
    writeBuff_.ToReadAll();
    readBuff_.ToReadAll();
    LOG_INFO("Client[%d](%s:%d) in, UserCount:%d", fd_, GetIP(), GetPort(), (int)userCount);
}

ssize_t HttpConn::read(int* saveErrno) {
    ssize_t len = -1;
    do {
        len = readBuff_.ReadFd(fd_, saveErrno);
        if (len <= 0) break;
    } while (isET);
    return len;
}

ssize_t HttpConn::write(int* saveErrno) {
    ssize_t len = -1;
    do {
        len = writeBuff_.WriteFd(fd_, saveErrno);
        if (len <= 0) break;
    } while (isET);
    return len;
}

bool HttpConn::process() {
    request_.Init();
    if (readBuff_.ReadableBytes() <= 0) return false;
    else if (request_.parse(readBuff_)) {
        response_.Init(request_.path(), request_.reply(), request_.IsKeepAlive(), 200);
    }
    else {
        response_.Init(request_.path(), request_.reply(), false, 400);
    }

    response_.MakeResponse(writeBuff_);
    return true;
}