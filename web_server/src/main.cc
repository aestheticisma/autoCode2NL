#include <unistd.h>
#include "server/webserver.h"

int main() {
    // 守护进程 后台运行

    WebServer server (
        8888, 3, 60000, true, 16, true, 0, 1024
    );
    server.Start();
}