/*
 * @Author       : fan
 * @Date         : 2022-08-06
 * @copyleft Apache 2.0
 */

#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unordered_map>

#include "epoller.h"
#include "../log/log.h"
#include "../timer/heaptimer.h"
#include "../pool/threadpool.h"
#include "../http/httpconn.h"

class WebServer {
public:
    WebServer(
        int port, int trigMode, int timeoutMS, bool OptLinger,
        int threadNum, bool openLog, int logLevel, int logQueSize
    );
    ~WebServer();
    void Start();

private:
    void InitEventMode_(int trigMode);
    bool InitSocket_();
    static int SetFdNonblock_(int fd);

    void DealListen_();
    void DealRead_(HttpConn *client);
    void DealWrite_(HttpConn *client);
    void CloseConn_(HttpConn *client);

    void OnRead_(HttpConn* client);
    void OnWrite_(HttpConn* client);
    void OnProcess_(HttpConn* client);

    void SendError_(int fd, const char* info);
    void AddClient_(int fd, sockaddr_in addr);
    void ExtentTime_(HttpConn* client);

private:
    int port_;
    bool openLinger_;
    int timeoutMS_;
    bool isClose_;
    int listenFd_;

    static const int MAX_FD = 65536;

    uint32_t listenEvent_;
    uint32_t connEvent_;

    std::unique_ptr<HeapTimer> timer_;
    std::unique_ptr<ThreadPool> threadpool_;
    std::unique_ptr<Epoller> epoller_;
    std::unordered_map<int, HttpConn> users_;
};

#endif