/*
 * @Author       : fan
 * @Date         : 2022-08-05
 * @copyleft Apache 2.0
 */

#ifndef LOG_H
#define LOG_H

#include <mutex>
#include <cstring>
#include <thread>
#include <sys/time.h>
#include <stdarg.h>
#include <sys/stat.h> // mkdir

#include "blockqueue.h"
#include "../buffer/buffer.h"

class Log {
public:
    void flush();
    int GetLevel();
    void SetLevel(int level);
    bool IsOpen();
    void Init(
        int level, const char *path = "./log",
        const char *suffix = ".log",
        int maxQueueCapacity = 1024
    );
    void write(int level, const char *format, ...);

    static Log* Instance();
    static void FlushLogThread();

private:
    Log();
    void AppenLogLevelTitle_(int level);
    virtual ~Log();
    void AsyncWrite_();

    static const int LOG_PATH_LEN = 256;
    static const int LOG_NAME_LEN = 256;
    static const int MAX_LINES = 50000;

    const char *path_;
    const char *suffix_;

    int lineCount_;
    int toDay_;

    bool isOpen_;

    Buffer buff_;
    int level_;
    bool isAsync_;

    FILE *fp_;
    std::unique_ptr<BlockDeque<std::string>> dq_;
    std::unique_ptr<std::thread> writeThread_;
    std::mutex mtx_;
};

#define LOG_BASE(level, format, ...) \
    do {\
        Log *log = Log::Instance();\
        if (log->IsOpen() && log->GetLevel() <= level) {\
            log->write(level, format, ##__VA_ARGS__);\
            log->flush();\
        }\
    } while (0);

#define LOG_DEBUG(format, ...) do {LOG_BASE(0, format, ##__VA_ARGS__)} while(0);
#define LOG_INFO(format, ...) do {LOG_BASE(1, format, ##__VA_ARGS__)} while(0);
#define LOG_WARN(format, ...) do {LOG_BASE(2, format, ##__VA_ARGS__)} while(0);
#define LOG_ERROR(format, ...) do {LOG_BASE(3, format, ##__VA_ARGS__)} while(0);

#endif