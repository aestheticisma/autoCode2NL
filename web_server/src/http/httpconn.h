/*
 * @Author       : fan
 * @Date         : 2022-08-06
 * @copyleft Apache 2.0
 */

#ifndef HTTP_CONN_H
#define HTTP_CONN_H

#include <arpa/inet.h> // sockaddr_in
#include <stdlib.h> // atoi()
#include <unistd.h>
#include <errno.h>
#include <atomic>

#include "../log/log.h"
#include "../buffer/buffer.h"
#include "request.h"
#include "response.h"

class HttpConn {
public:
    HttpConn();
    ~HttpConn();

    void Init(int sockFd, const sockaddr_in& addr);
    void Close();

    int GetFd() const;
    int GetPort() const;
    const char* GetIP() const;
    sockaddr_in GetAddr() const;

    ssize_t read(int* saveErrno);
    ssize_t write(int* saveErrno);

    bool process();

    bool IsKeepAlive() const {
        return request_.IsKeepAlive();
    }

    static bool isET;
    static std::atomic<int> userCount;

private:
    int fd_;
    struct sockaddr_in addr_;

    bool isClose_;

    Buffer readBuff_;
    Buffer writeBuff_;
    HttpRequest request_;
    HttpResponse response_;
};

#endif