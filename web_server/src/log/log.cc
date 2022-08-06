/*
 * @Author       : fan
 * @Date         : 2022-08-05
 * @copyleft Apache 2.0
 */

#include "log.h"

Log::Log(): lineCount_(0), isAsync_(false), writeThread_(nullptr),
    dq_(nullptr), toDay_(0), fp_(nullptr) {}

Log::~Log() {
    if (writeThread_ && writeThread_->joinable()) {
        while (!dq_->empty()) dq_->flush();
        dq_->Close();
        writeThread_->join();
    }
    if (fp_) {
        std::lock_guard<std::mutex> locker(mtx_);
        flush();
        fclose(fp_);
    }
    isOpen_ = false;
}

void Log::flush() {
    if (isAsync_) dq_->flush();
    fflush(fp_);
}

Log* Log::Instance() {
    static Log inst;
    return &inst;
}

int Log::GetLevel() {
    std::lock_guard<std::mutex> locker(mtx_);
    return level_;
}

void Log::SetLevel(int level) {
    std::lock_guard<std::mutex> locker(mtx_);
    level_ = level;
}

bool Log::IsOpen() {
    std::lock_guard<std::mutex> locker(mtx_);
    return isOpen_;
}

void Log::Init(
        int level = 1, const char *path,
        const char *suffix,
        int maxQueueCapacity
) {
    isOpen_ = true;
    level_ = level;
    if (maxQueueCapacity > 0) {
        isAsync_ = true;
        if (dq_ == nullptr) {
            std::unique_ptr<BlockDeque<std::string>> newDeque(new BlockDeque<std::string>(maxQueueCapacity));
            dq_ = std::move(newDeque);

            std::unique_ptr<std::thread> newThread(new std::thread(FlushLogThread));
            writeThread_ = std::move(newThread);
        }
    }
    else isAsync_ = false;

    lineCount_ = 0;
    time_t timer = time(nullptr);
    struct tm *sysTime = localtime(&timer);
    path_ = path;
    suffix_ = suffix;
    char fileName[LOG_NAME_LEN] = {0};
    snprintf(fileName, LOG_NAME_LEN-1, "%s/%04d_%02d_%02d%s",
            path_, sysTime->tm_year + 1900, sysTime->tm_mon + 1, sysTime->tm_mday, suffix_);
    toDay_ = sysTime->tm_mday;

    {
        std::lock_guard<std::mutex> locker(mtx_);
        buff_.ToReadAll();
        if (fp_) {
            flush();
            fclose(fp_);
        }
        fp_ = fopen(fileName, "a");
        if (fp_ == nullptr) {
            mkdir(path_, 0777);
            fp_ = fopen(fileName, "a");
        }
        assert(fp_ != nullptr);
    }
}

void Log::AsyncWrite_() {
    std::string str = "";
    while (dq_->pop(str)) {
        std::lock_guard<std::mutex> locker(mtx_);
        fputs(str.c_str(), fp_);
    }
}

// 异步写线程
void Log::FlushLogThread() {
    Log::Instance()->AsyncWrite_();
}

void Log::AppenLogLevelTitle_(int level) {
    switch (level) {
        case 0:
            buff_.Append("[Debug]: ", 9);
            break;
        case 1:
            buff_.Append("[Info] : ", 9);
            break;
        case 2:
            buff_.Append("[Warn] : ", 9);
            break;
        case 3:
            buff_.Append("[Error]: ", 9);
            break;
        default:
            buff_.Append("[info] : ", 9);
    }
}

void Log::write(int level, const char *format, ...) {
    struct timeval now = {0, 0};
    gettimeofday(&now, nullptr);
    time_t tSec = now.tv_sec;
    struct tm *sysTime = localtime(&tSec);
    struct tm t = *sysTime;

    va_list vaList;

    // 打开新文件
    if (toDay_ != t.tm_mday || (lineCount_ && (lineCount_ % MAX_LINES == 0))) {
        std::unique_lock<std::mutex> locker(mtx_);
        locker.unlock();

        char newFile[LOG_NAME_LEN] = {0};
        char tail[36] = {0};
        snprintf(tail, 36, "%04d_%02d_%02d", t.tm_year+1900, t.tm_mon+1, t.tm_mday);

        if (toDay_ != t.tm_mday) {
            snprintf(newFile, LOG_NAME_LEN-72, "%s/%s%s", path_, tail, suffix_);
            toDay_ = t.tm_mday;
            lineCount_ = 0;
        }
        else snprintf(newFile, LOG_NAME_LEN-72, "%s/%s-%d%s", path_, tail, (lineCount_ / MAX_LINES), suffix_);

        locker.lock();
        flush();
        fclose(fp_);
        fp_ = fopen(newFile, "a");
        assert(fp_ != nullptr);
    }

    // 写 fp_
    {
        std::lock_guard<std::mutex> locker(mtx_);
        ++lineCount_;
        char timeLines[128] = {0};
        snprintf(timeLines, 127, "%04d-%02d-%02d %02d:%02d:%02d.%06ld ",
                t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, 
                t.tm_hour, t.tm_min, t.tm_sec, now.tv_usec);
        buff_.Append(timeLines, strlen(timeLines));
        AppenLogLevelTitle_(level);

        va_start(vaList, format);
        char messages[128];
        vsnprintf(messages, 127, format, vaList);
        va_end(vaList);

        buff_.Append(messages, strlen(messages));
        buff_.Append("\n\0", 2);

        if (isAsync_ && dq_ && !dq_->full()) {
            dq_->push_back(buff_.ReadAll2Str());
        }
        else {
            fputs(buff_.Peek(), fp_);
            buff_.ToReadAll();
        }
    }
}