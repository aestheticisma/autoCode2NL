/*
 * @Author       : fan
 * @Date         : 2022-08-06
 * @copyleft Apache 2.0
 */
#include "response.h"

const std::unordered_map<int, std::string> HttpResponse::CODE_STATUS = {
    {200, "OK"},
    {400, "Bad Request"},
    {403, "Forbidden"},
    {404, "Not Found"},
};

// const std::string ErrorHtmlContent = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 3.2 Final//EN\">\n<title>404 Not Found</title>\n<h1>Not Found</h1>\n<p>The requested URL was not found on the server. If you entered the URL manually please check your spelling and try again.</p>";

const std::string HttpResponse::prefix_message = "{\"message\":[\"";
const std::string HttpResponse::suffix_message = "\"]}";

HttpResponse::HttpResponse() {
    code_ = -1;
    path_ = reply_ = "";
    isKeepAlive_ = false;
}

void HttpResponse::Init(const std::string& path, const std::string& reply, bool isKeepAlive, int code) {
    path_ = path;
    reply_ = reply;
    isKeepAlive_ = isKeepAlive;
    code_ = code;
}

void HttpResponse::MakeResponse(Buffer& buff) {
    if (path_ != "/summary") code_ = 404;
    else if (code_ == -1) code_ = 200;

    AddStateLine_(buff);
    AddHeader_(buff);
    AddContent_(buff);
}

void HttpResponse::AddStateLine_(Buffer& buff) {
    std::string status;
    if (CODE_STATUS.count(code_))
        status = CODE_STATUS.find(code_)->second;
    else {
        code_ = 400;
        status = CODE_STATUS.find(code_)->second;
    }
    buff.Append("HTTP/1.1 " + std::to_string(code_) + " " + status + "\r\n");
}

void HttpResponse::AddHeader_(Buffer& buff) {
    buff.Append("Connection: ");
    if (isKeepAlive_) {
        buff.Append("keep-alive\r\n");
        buff.Append("keep-alive: max=6, timeout=120\r\n");
    }
    else {
        buff.Append("close\r\n");
    }
    buff.Append(std::string("Content-type: application/json\r\n"));
}

void HttpResponse::AddContent_(Buffer& buff) {
    if (reply_.empty() || code_ != 200) {
        ErrorContent(buff, "404 Not Found");
    }
    else {
        std::string body = prefix_message + reply_ + suffix_message;
        buff.Append("Content-length: " + std::to_string(body.size()) + "\r\n\r\n");
        buff.Append(body);
    }
}

void HttpResponse::ErrorContent(Buffer& buff, std::string message) {
    std::string body;
    std::string status;
    body += "<html><title>Error</title>";
    body += "<body bgcolor=\"ffffff\">";
    if (CODE_STATUS.count(code_)) {
        status = CODE_STATUS.find(code_)->second;
    }
    else {
        status = "Bad Request";
    }
    body += std::to_string(code_) + " : " + status + "\n";
    body += "<p>" + message + "</p>";
    body += "<hr><em>TinyWebServer</em></body></html>";

    buff.Append("Content-length: " + std::to_string(body.size()) + "\r\n\r\n");
    buff.Append(body);
}