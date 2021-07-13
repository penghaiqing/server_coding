#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<iostream>
#include<string.h>

// 客户端
// 1、 验证阻塞模式下 send 函数的行为。
// 2、 验证阻塞模式下 recv 函数，不需要修改服务器端。、
//     如果服务器端不向客户端发送数据，则此时客户端调用recv 函数的执行流就会阻塞在recv函数调用处。

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
        std::cout << "connect server error." << std::endl;
        close(clientfd);
        return -1;
    }

    // 3. 不断向服务器发送数据，直到出错并退出。来测试 send 函数的执行
    // int count = 0;
    // while(1){
    //     int ret = send(clientfd, DATA, strlen(DATA), 0);
    //     if(ret != strlen(DATA)){
    //         std::cout << "send data error." << std::endl;
    //         break;
    //     }else{
    //         count++;
    //         std::cout << "send data successfully, count = " << count << std::endl;
    //     }
    // }

    // 3. 直接调用 recv 函数，程序会阻塞在 recv 函数调用处
    char recvbuff[32] = {0};
    int ret = recv(clientfd, recvbuff, 32, 0);
    if(ret > 0){
        std::cout << "recv successfully. recv message is : " << recvbuff << std::endl; 
    }else{
        std::cout << "recv data error." << std::endl;
    }


    // 4.关闭socket
    close(clientfd);

    return 0;
}