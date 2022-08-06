/*
 * @Author       : fan
 * @Date         : 2022-08-06
 * @copyleft Apache 2.0
 */

#include "webserver.h"

WebServer::WebServer(
    int port, int trigMode, int timeoutMS,
    bool OptLinger, int threadNum, bool openLog,
    int logLevel, int logQueSize):port_(port), openLinger_(OptLinger), timeoutMS_(timeoutMS),
    isClose_(false), timer_(new HeapTimer()), threadpool_(new ThreadPool(threadNum)), epoller_(new Epoller()) {
    
    HttpConn::userCount = 0;
    InitEventMode_(trigMode);
    if (!InitSocket_()) { isClose_ = true; } 
    if (openLog) {
        Log::Instance()->Init(logLevel, "./log", ".log", logQueSize);
        if (isClose_) {
            LOG_ERROR("========== Server Init Error! ==========");
        }
        else {
            LOG_INFO("========== Server Init ==========");
            LOG_INFO("Port: %d, OpenLinger: %s", port_, OptLinger ? "true" : "false");
            LOG_INFO("Listen Mode: %s, OpenConn Mode: %s", 
                        (listenEvent_ & EPOLLET ? "ET" : "LT"),
                        (connEvent_ & EPOLLET ? "ET" : "LT"));
            LOG_INFO("LogSys level: %d", logLevel);
        }
    }
}

WebServer::~WebServer() {
    close(listenFd_);
    isClose_ = true;
}

void WebServer::InitEventMode_(int trigMode) {
    listenEvent_ = EPOLLRDHUP;
    connEvent_ = EPOLLONESHOT | EPOLLRDHUP;
    switch (trigMode)
    {
    case 0:
        break;
    case 1:
        connEvent_ |= EPOLLET;
        break;
    case 2:
        listenEvent_ |= EPOLLET;
        break;
    case 3:
        listenEvent_ |= EPOLLET;
        connEvent_ |= EPOLLET;
        break;
    default:
        listenEvent_ |= EPOLLET;
        connEvent_ |= EPOLLET;
        break;
    }
    HttpConn::isET = (connEvent_ & EPOLLET);
}

bool WebServer::InitSocket_() {
    int ret;
    struct sockaddr_in addr;
    if (port_ > 65535 || port_ < 1024) {
        LOG_ERROR("Port: %d error!", port_);
        return false;
    }
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port_);

    struct linger optLinger = { 0 };
    // TCP优雅关闭连接，直到数据发送完毕或超时
    if (openLinger_) {
        optLinger.l_onoff = 1;
        optLinger.l_linger = 1;
    }
    listenFd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (listenFd_ < 0) {
        LOG_ERROR("Create socket error!", port_)
        return false;
    }

    ret = setsockopt(listenFd_, SOL_SOCKET, SO_LINGER, &optLinger, sizeof(optLinger));
    if (ret < 0) {
        close(listenFd_);
        LOG_ERROR("Init linger error!", port_);
        return false;
    }

    // 端口复用: 只有最后一个套接字会正常接收数据
    int optval = 1;
    ret = setsockopt(listenFd_, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(int));
    if (ret == -1) {
        LOG_ERROR("set socket setsockopt error!");
        close(listenFd_);
        return false;
    }

    ret = bind(listenFd_, (struct sockaddr*)&addr, sizeof(addr));
    if (ret < 0) {
        LOG_ERROR("Bind Port: %d error", port_);
        close(listenFd_);
        return false;
    }

    ret = listen(listenFd_, 6);
    if (ret < 0) {
        LOG_ERROR("Listen Port: %d error", port_);
        close(listenFd_);
        return false;
    }

    ret = epoller_->AddFd(listenFd_, listenEvent_ | EPOLLIN);
    if (ret == 0) {
        LOG_ERROR("Add listen error");
        close(listenFd_);
        return false;
    }
    // 边缘触发需要设置为非阻塞fd
    SetFdNonblock_(listenFd_);
    LOG_INFO("Server port: %d", port_);
    return true;
}

int WebServer::SetFdNonblock_(int fd) {
    assert(fd > 0);
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
}

void WebServer::Start() {
    int timeMS = -1;
    if (!isClose_) {
        LOG_INFO("========== Server Start ==========");
    }
    while (!isClose_) {
        if (timeoutMS_ > 0) timeMS = timer_->GetNextTick();
        int eventCnt = epoller_->Wait(timeMS);
        for (int i = 0; i < eventCnt; ++i) {
            int fd = epoller_->GetEventFd(i);
            uint32_t events = epoller_->GetEvents(i);
            if (fd == listenFd_) DealListen_();
            else if (events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                assert(users_.count(fd) > 0);
                CloseConn_(&users_[fd]);
            }
            else if (events & EPOLLIN) {
                assert(users_.count(fd) > 0);
                DealRead_(&users_[fd]);
            }
            else if (events & EPOLLOUT) {
                assert(users_.count(fd) > 0);
                DealWrite_(&users_[fd]);
            }
            else {
                LOG_ERROR("Unexpected event");
            }
        }
    }
    std::cout << "test isClosed" << " " << isClose_ << std::endl;
}

void WebServer::DealListen_() {
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    do {
        int fd = accept(listenFd_, (struct sockaddr*)&addr, &len);
        if (fd < 0) { return; }
        else if (HttpConn::userCount >= MAX_FD) {
            SendError_(fd, "Server Busy!");
            LOG_WARN("Clients is full!");
            return;
        }
        AddClient_(fd, addr);
    } while (listenEvent_ & EPOLLET);
}

void WebServer::SendError_(int fd, const char* info) {
    assert(fd > 0);
    int ret = send(fd, info, strlen(info), 0);
    if (ret < 0) {
        LOG_WARN("Send Error to Client[%d] error!", fd);
    }
    close(fd);
}

void WebServer::AddClient_(int fd, sockaddr_in addr) {
    assert(fd);
    users_[fd].Init(fd, addr);
    if (timeoutMS_ > 0) {
        timer_->add(fd, timeoutMS_, std::bind(&WebServer::CloseConn_, this, &users_[fd]));
    }
    epoller_->AddFd(fd, EPOLLIN | connEvent_);
    SetFdNonblock_(fd);
    LOG_INFO("Client[%d] in!", users_[fd].GetFd());
}

void WebServer::ExtentTime_(HttpConn* client) {
    assert(client);
    if (timeoutMS_ > 0) {
        timer_->adjust(client->GetFd(), timeoutMS_);
    }
}

void WebServer::CloseConn_(HttpConn *client) {
    assert(client);
    LOG_INFO("Client[%d] quit!", client->GetFd());
    epoller_->DelFd(client->GetFd());
    client->Close();
}

void WebServer::DealWrite_(HttpConn* client) {
    assert(client);
    ExtentTime_(client);
    threadpool_->AddTask(std::bind(&WebServer::OnWrite_, this, client));
}

void WebServer::DealRead_(HttpConn* client) {
    assert(client);
    ExtentTime_(client);
    threadpool_->AddTask(std::bind(&WebServer::OnRead_, this, client));
}

void WebServer::OnRead_(HttpConn* client) {
    assert(client);
    int ret = -1;
    int readErrno = 0;
    ret = client->read(&readErrno);
    if (ret <= 0 && readErrno != EAGAIN) {
        CloseConn_(client);
        return;
    }
    OnProcess_(client);
}

void WebServer::OnWrite_(HttpConn* client) {
    assert(client);
    int ret = -1;
    int writeErrno = 0;
    ret = client->write(&writeErrno);
    if (ret < 0) {
        if (writeErrno == EAGAIN) {
            // 继续传输
            epoller_->ModFd(client->GetFd(), connEvent_ | EPOLLOUT);
        }
    }
    // else if (client->IsKeepAlive()) {
    //     OnProcess_(client);
    // }
    else {
        CloseConn_(client);
    }
}

void WebServer::OnProcess_(HttpConn* client) {
    if (client->process()) {
        epoller_->ModFd(client->GetFd(), connEvent_ | EPOLLOUT);
    }
    else {
        epoller_->ModFd(client->GetFd(), connEvent_ | EPOLLIN);
    }
}


