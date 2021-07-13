#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<iostream>
#include<string.h>

// 服务器端
// 1. 验证非阻塞模式下 send 函数的行为。
// 2. 验证非阻塞模式下 recv 函数的行为。


int main(){
    // 1.创建一个监听 socket
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenfd == -1){
        std::cout << "create listen socket failed!" << std::endl;
        return -1;
    }
    // 2.初始化服务器地址
    struct sockaddr_in bindaddr;
    bindaddr.sin_family = AF_INET;
    bindaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    bindaddr.sin_port = htons(3000);
    if(bind(listenfd, (struct sockaddr*)&bindaddr, sizeof(bindaddr)) == -1){
        std::cout << "bind listen socket error." << std::endl;
        close(listenfd);
        return -1;
    }

    // 3.启动监听
    if(listen(listenfd, SOMAXCONN) == -1){
        std::cout << "listen error." << std::endl;
        close(listenfd);
        return -1;
    }

    while(1){
        struct sockaddr_in clientaddr;
        socklen_t clientaddrlen = sizeof(clientaddr);
        // 4.接收客户端的连接
        int clientfd = accept(listenfd, (struct sockaddr*)&clientaddr, &clientaddrlen);
        if(clientfd != -1){
            // 只是接收客户端的连接，不调用 recv 收取任何数据。
            std::cout << "accept a client connection." << std::endl;
        }
    }
    // 5. 关闭监听 socket
    close(listenfd);

    return 0;
}