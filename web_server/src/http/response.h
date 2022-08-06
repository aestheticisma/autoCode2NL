/*
 * @Author       : fan
 * @Date         : 2022-08-06
 * @copyleft Apache 2.0
 */

#ifndef RESPONSE_H
#define RESPONSE_H

#include <unordered_map>
#include <string>

#include "../buffer/buffer.h"
#include "../log/log.h"

class HttpResponse {
public:
    HttpResponse();
    ~HttpResponse() = default;

    void Init(const std::string& path, const std::string& reply, bool isKeepAlive=false, int code=-1);
    void MakeResponse(Buffer& buff);

    void ErrorContent(Buffer& buff, std::string message);
    int Code() const { return code_; }

private:
    void AddStateLine_(Buffer& buff);
    void AddHeader_(Buffer& buff);
    void AddContent_(Buffer& buff);

    int code_;
    bool isKeepAlive_;

    std::string path_, reply_;
    static const std::unordered_map<int, std::string> CODE_STATUS;
    // static const std::string ErrorHtmlContent;
    static const std::string prefix_message;
    static const std::string suffix_message;
};

#endif