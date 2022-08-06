/*
 * @Author       : fan
 * @Date         : 2022-08-05
 * @copyleft Apache 2.0
 */

#ifndef BLOCKQUEUE_H
#define BLOCKQUEUE_H

#include <mutex>
#include <condition_variable>
#include <deque>
#include <sys/time.h>
#include <assert.h>

template<typename T>
class BlockDeque {
public:
    explicit BlockDeque(size_t MaxCapacity = 1000);
    ~BlockDeque();

    void clear();
    bool empty();
    bool full();

    void Close();

    size_t size();
    size_t capacity();

    T front();
    T back();

    void push_back(const T &item);
    void push_front(const T &item);

    bool pop(T &item);
    bool pop(T &item, int timeout);

    void flush();


private:
    std::deque<T> dq_;
    size_t capacity_;
    std::mutex mtx_;
    bool isClosed_;
    std::condition_variable condConsumer_;
    std::condition_variable condProducer_;
};

template<typename T>
BlockDeque<T>::BlockDeque(size_t MaxCapacity): capacity_(MaxCapacity), isClosed_(false) {
    assert(MaxCapacity > 0);
}

template<typename T>
BlockDeque<T>::~BlockDeque() {
    Close();
}

template<typename T>
void BlockDeque<T>::Close() {
    {
        std::lock_guard<std::mutex> locker(mtx_);
        dq_.clear();
        isClosed_ = true;
    }
    condProducer_.notify_all();
    condConsumer_.notify_all();
}

template<typename T>
void BlockDeque<T>::clear() {
    std::lock_guard<std::mutex> locker(mtx_);
    dq_.clear();
}

template<typename T>
bool BlockDeque<T>::empty() {
    std::lock_guard<std::mutex> locker(mtx_);
    return dq_.empty();
}

template<typename T>
bool BlockDeque<T>::full() {
    std::lock_guard<std::mutex> locker(mtx_);
    return dq_.size() >= capacity_;
}

template<typename T>
size_t BlockDeque<T>::size() {
    std::lock_guard<std::mutex> locker(mtx_);
    return dq_.size();
}

template<typename T>
size_t BlockDeque<T>::capacity() {
    std::lock_guard<std::mutex> locker(mtx_);
    return capacity_;
}

template<typename T>
T BlockDeque<T>::front() {
    std::lock_guard<std::mutex> locker(mtx_);
    return dq_.front();
}

template<typename T>
T BlockDeque<T>::back() {
    std::lock_guard<std::mutex> locker(mtx_);
    return dq_.back();
}

template<typename T>
void BlockDeque<T>::push_back(const T &item) {
    std::unique_lock<std::mutex> locker(mtx_);
    while (dq_.size() >= capacity_) {
        condProducer_.wait(locker);
        if (isClosed_) return;
    }
    dq_.push_back(item);
    condConsumer_.notify_one();
}

template<typename T>
void BlockDeque<T>::push_front(const T &item) {
    std::unique_lock<std::mutex> locker(mtx_);
    while (dq_.size() >= capacity_) {
        condProducer_.wait(locker);
        if (isClosed_) return;
    }
    dq_.push_front(item);
    condConsumer_.notify_one();
}

template<typename T>
bool BlockDeque<T>::pop(T &item) {
    std::unique_lock<std::mutex> locker(mtx_);
    while (dq_.empty()) {
        condConsumer_.wait(locker);
        if (isClosed_) return false;
    }
    item = dq_.front();
    dq_.pop_front();
    condProducer_.notify_one();
    return true;
}

template<typename T>
bool BlockDeque<T>::pop(T &item, int timeout) {
    std::unique_lock<std::mutex> locker(mtx_);
    while (dq_.empty()) {
        if (condConsumer_.wait_for(locker, std::chrono::seconds(timeout))
            == std::cv_status::timeout) return false;
        if (isClosed_) return false;
    }
    item = dq_.front();
    dq_.pop_front();
    condProducer_.notify_one();
    return true;
}

template<typename T>
void BlockDeque<T>::flush() {
    condConsumer_.notify_all();
}


#endif

