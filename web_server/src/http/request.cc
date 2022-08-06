/*
 * @Author       : fan
 * @Date         : 2022-08-05
 * @copyleft Apache 2.0
 */

#include "request.h"

void HttpRequest::Init() {
    method_ = path_ = version_ = body_ = "";
    reply_ = "";
    state_ = REQUEST_LINE;
    header_.clear();
    // post_.clear();
}

bool HttpRequest::IsKeepAlive() const {
    if (header_.count("Connection") == 1) {
        return header_.find("Connection")->second == "keep-alive" \
            && version_ == "1.1";
    }
    return false;
}

bool HttpRequest::parse(Buffer& buff) {
    const char CRLF[] = "\r\n";
    if (buff.ReadableBytes() <= 0) return false;
    while (buff.ReadableBytes() && state_ != FINISH) {
        const char* lineEnd = std::search(buff.Peek(), buff.BeginWriteConst(), CRLF, CRLF+2);
        std::string line(buff.Peek(), lineEnd);

        switch (state_) {
            case REQUEST_LINE:
                if (!ParseRequestLine_(line)) return false;
                // ParsePath_();
                break;
            case HEADERS:
                ParseHeader_(line);
                if (buff.ReadableBytes() <= 2) state_ = FINISH;
                break;
            case BODY:
                ParseBody_(line);
                break;
            default:
                break;
        }

        if (lineEnd == buff.BeginWrite()) break;
        buff.ToReadUtil(lineEnd+2);
    }
    LOG_DEBUG("[%s], [%s], [%s]", method_.c_str(), path_.c_str(), version_.c_str());
    return true;
}

bool HttpRequest::ParseRequestLine_(const std::string& line) {
    std::regex patten("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
    std::smatch subMatch;
    if (std::regex_match(line, subMatch, patten)) {
        method_ = subMatch[1];
        path_ = subMatch[2];
        version_ = subMatch[3];
        state_ = HEADERS;
        return true;
    }
    LOG_ERROR("RequestLine Error");
    return false;
}

void HttpRequest::ParseHeader_(const std::string& line) {
    std::regex patten("^([^:]*): ?(.*)$");
    std::smatch subMatch;
    if (regex_match(line, subMatch, patten)) {
        header_[subMatch[1]] = subMatch[2];
    }
    else {
        state_ = BODY;
    }
}

void HttpRequest::ParseBody_(const std::string& line) {
    body_ = line;
    ParsePost_();
    state_ = FINISH;
    LOG_DEBUG("Body: %s, len: %d", line.c_str(), line.size());
}

// To Do: 解析POST请求, json格式
// Be a grpc client
void HttpRequest::ParsePost_() {
    if (method_ == "POST" && header_["Content-Type"].find("application/json") != std::string::npos) {
        // cout << body_ << endl;
        std::string source = "";
        JsonHelper_(source);
        reply_ = SummaryClient::Instance()->summarize(source);
        LOG_DEBUG("REPLY: %s", reply_.c_str());
        // std::cout << reply_ << std::endl;
    }
}

void HttpRequest::JsonHelper_(std::string &str) {
    int l = 0, n = body_.size();
    while (l < n && body_[l] != ':') ++l;
    ++l;
    while (l < n && body_[l] != '"') ++l;
    if (l >= n) return;
    str = body_.substr(l, n - l - 2);
}

const std::string HttpRequest::path() const {
    return path_;
}

// std::string& HttpRequest::path() {
//     return path_;
// }

const std::string HttpRequest::method() const {
    return method_;
}

const std::string HttpRequest::version() const {
    return version_;
}

const std::string HttpRequest::reply() const {
    return reply_;
}
