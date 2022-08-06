/*
 * @Author       : fan
 * @Date         : 2022-08-05
 * @copyleft Apache 2.0
 */

#ifndef REQUEST_H
#define REQUEST_H

#include <vector>
#include <unordered_map>
#include <cstring>
#include <regex>
#include <errno.h>
#include <cstring>
#include <unistd.h>
#include <dirent.h>
#include <algorithm>

#include "../buffer/buffer.h"
#include "../log/log.h"
#include "../grpc/grpc_client.h"

class HttpRequest {
public:
    enum PARSE_STATE {
        REQUEST_LINE,
        HEADERS,
        BODY,
        FINISH,
    };

    enum STATUS_CODE {
        NO_REQUEST = 0,
        GET_REQUEST,
        BAD_REQUEST,
        NO_RESOURCE,
        FORBIDDENT_REQUEST,
        FILE_REQUEST,
        INTERNAL_ERROR,
        CLOSED_CONNECTION,
    };

    void Init();
    HttpRequest() { 
        Init(); 
    }
    ~HttpRequest() = default;
    bool parse(Buffer& buff);

    const std::string path() const;
    // std::string& path();
    const std::string method() const;
    const std::string version() const;
    const std::string reply() const;
    // std::string GetPost(const std::string& key) const;
    // std::string GetPost(const char* key) const;

    bool IsKeepAlive() const;

private:
    bool ParseRequestLine_(const std::string& line);
    // void ParsePath_();
    void ParseHeader_(const std::string& line);
    void ParseBody_(const std::string& line);

    void ParsePost_();
    void JsonHelper_(std::string &str);

    PARSE_STATE state_;
    std::string method_, path_, version_, body_;
    std::string reply_;

    std::unordered_map<std::string, std::string> header_;
    // std::unordered_map<std::string, std::string> post_;

};

#endif