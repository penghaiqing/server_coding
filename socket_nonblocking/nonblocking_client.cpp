#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<iostream>
#include<string.h>
#include<fcntl.h>

// 客户端
// 主要是 验证非阻塞模式下 send 函数的行为。

#define ADDRSS_SERVER "127.0.0.1"
#define PORT_SERVER 3000
#define DATA "hello ya"

int main(){
    // 1. 创建一个 socket
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if(clientfd == -1){
        std::cout << "create client socket error." << std::endl;
        return -1;
    }
    // 2. 获得服务器地址，并连接服务器(客户端不需要 bind 和 listen，直接去连接服务器端就可以了)
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(ADDRSS_SERVER); // inet_addr把点分十进制的IP转换成u_long
    serveraddr.sin_port = htons(PORT_SERVER);

    if(connect(clientfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) == -1){
        std::cout << "connect socket error." << std::endl;
        close(clientfd);
        return -1;
    }

    // 连接成功后，再将 clientfd 设置为非阻塞模式
    // 不能在创建时就设置，这样会影响到connect 函数的行为
    int oldSocketFlag = fcntl(clientfd, F_GETFL, 0);
    int newSocketFlag = oldSocketFlag | O_NONBLOCK;
    if(fcntl(clientfd, F_SETFL, newSocketFlag) == -1){
        close(clientfd);
        std::cout << "set socket to noblock error." << std::endl;
        return -1;
    }


    // 3. 不断向服务器发送数据，直到出错并退出。来测试 send 函数的执行
    // int count = 0;
    // while(1){
    //     int ret = send(clientfd, DATA, strlen(DATA), 0);
    //     if(ret == -1){
    //         // 非阻塞模式下，send 函数由于TCP窗口太小发不出去数据，错误码为 EWOULDBLOCK
    //         if(errno == EWOULDBLOCK){
    //             std::cout << "send data error as TCP window size is too small." << std::endl;
    //             // continue;
    //             break;
    //         }
    //         else if(errno == EINTR){
    //             // 如果信号被中断，则继续重试
    //             std::cout << "sending data interrupted by signal." << std::endl;
    //             continue;
    //         }else{
    //             std::cout << "send data error." << std::endl;
    //             break;
    //         }     
    //     }else if(ret == 0){
    //         // 对端关闭了连接，客户端也关闭
    //         std::cout << "send data error." << std::endl;
    //         close(clientfd);
    //         break;
    //     }
    //     else {
    //         count++;
    //         std::cout << "send data successfully, count = " << count << std::endl;
    //     }
    // }

    // 3. 直接调用 recv 函数，程序会阻塞在 recv 函数调用处
    while(1){
        char recvbuff[32] = {0};
        // 因为 clientfd 已经被设置为 非阻塞模式，所以无论有无数据，recv 函数都不会阻塞程序
        int ret = recv(clientfd, recvbuff, 32, 0);
        if(ret > 0){
            // 收到数据
            std::cout << "recv sucessfully." << std::endl;
        }
        else if(ret == 0){
            // 对端关闭
            std::cout << "peer close the socket." << std::endl;
            break;
        }
        else if(ret == -1){
            if(errno == EWOULDBLOCK){
                std::cout << "There is no data available now." << std::endl;
                break;
            }else if(errno == EINTR){
                // 如果被信号中断，则继续重试 recv函数
                std::cout << "recv data interrupted by signal." << std::endl;
            }else{
                // 真的出错
                break;
            }
        }
    }


    // 4.关闭socket
    close(clientfd);

    return 0;
}