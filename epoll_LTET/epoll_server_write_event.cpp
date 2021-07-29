/*
 * @Author: your name
 * @Date: 2021-07-27 06:53:41
 * @LastEditTime: 2021-07-27 07:25:55
 * @LastEditors: Please set LastEditors
 * @Description: 测试 epoll LT 模式下处理 写事件的情况。当使用 nc -v 127.0.0.1 3000 连接后，服务器端会疯狂输出可写事件的触发
 * @FilePath: /network_program/server_coding/epoll_LTET/epoll_server_write_event.cpp
 */

#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/epoll.h>
#include<poll.h>
#include<iostream>
#include<string.h>
#include<errno.h>
#include<vector>

using namespace std;

int main(){
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenfd == -1){
        cout << "create listen socket error" << endl;
    }
    // 设置重用 IP地址和端口号
    int on = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on));
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEPORT, (char*)&on, sizeof(on));

    // 设置监听socket 为非阻塞
    int oldSocketFlag = fcntl(listenfd, F_GETFL, 0);
    int newSocketFlag = oldSocketFlag | O_NONBLOCK;
    if(fcntl(listenfd, F_SETFL, newSocketFlag) == -1){
        cout << "set listenfd to noblock error" << endl;
        return -1;
    }

    // 初始化服务器地址
    struct sockaddr_in bindaddr;
    bindaddr.sin_family = AF_INET;
    bindaddr.sin_addr.s_addr = htons(INADDR_ANY);
    bindaddr.sin_port = htons(3000); // 默认指定为 3000  端口号

    if(bind(listenfd, (struct sockaddr*)& bindaddr, sizeof(bindaddr)) == -1){
        cout << "bind listenfd to port error" << endl;
        close(listenfd);
        return -1;
    }
    // 启动监听
    if(listen(listenfd, SOMAXCONN) == -1){
        cout << "listen error. " << endl;
        close(listenfd);
        return -1;
    }

    // 创建 epollfd
    int epollfd = epoll_create(1024);
    if(epollfd == -1){
        cout << "crreate epollfd error." << endl;
        close(listenfd);
        return -1;
    }

    epoll_event listen_fd_event;
    listen_fd_event.data.fd = listenfd;
    listen_fd_event.events = EPOLLIN;
    
    //listen_fd_event.events |= EPOLLET; // 此行注释掉则，使用 LT模式（水平触发）

    // 将监听socket绑定到 epollfd 上
    if(epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &listen_fd_event) == -1){
        cout << "epoll_ctl error." << endl;
        close(listenfd);
        return -1;
    }
    

    int n ;
    // 服务器循环等待事件的到来，进行处理
    while(1){
        epoll_event epoll_events[1024];
        n = epoll_wait(epollfd, epoll_events, 1024, 1000);
        if(n < 0){
            if(errno == EINTR){
                continue; // 信号中断，
            }
            // 出错 退出
            break;
        }

        else if(n == 0){
            continue; // 超时，继续
        }

        for(size_t i=0; i<n; ++i){
            // 事件可读
            if(epoll_events[i].events & EPOLLIN){
                if(epoll_events[i].data.fd == listenfd){
                    // 监听socket，接收新连接
                    struct sockaddr_in clientaddr;
                    socklen_t clientaddrlen = sizeof(clientaddr);
                    // accept4() 函数返回的socket就直接是 非阻塞式的，但需要在第4个参数设置为 SOCK_NONBLOCK
                    int clientfd = accept4(listenfd, (struct sockaddr*) &clientaddr, &clientaddrlen, SOCK_NONBLOCK);
                    if(clientfd == -1){
                        cout << "accept4 error. " << endl;
                        close(listenfd);
                    }
                    epoll_event client_fd_event;
                    client_fd_event.data.fd = clientfd;
                    client_fd_event.events = EPOLLIN;

                    client_fd_event.events |= EPOLLOUT; // 同时监听新来的 socket 的读和写事件

                    if(epoll_ctl(epollfd, EPOLL_CTL_ADD, clientfd, &client_fd_event) != -1){
                        cout << "new client accepted, clientfd: " << clientfd << endl;
                    }
                    else{
                        cout << "add client fd to epollfd error." << endl;
                        close(clientfd);
                    }
                }
                else{
                    cout << "client fd : " << epoll_events[i].data.fd << " recv data." << endl;
                    // 普通 clientfd
                    char ch;
                    // 这里为了看出 ET模式触发和 LT 模式触发的不同，选择每次只接收一个字节
                    int m = recv(epoll_events[i].data.fd, &ch, 1, 0);
                    if(m == 0){
                        // 对端关闭了连接，从epollfd上移除clientfd
                        if(epoll_ctl(epollfd, EPOLL_CTL_DEL, epoll_events[i].data.fd, NULL) != -1){
                            cout << "client disconnected, clientfd: " << epoll_events[i].data.fd << endl;
                        }
                        close(epoll_events[i].data.fd);
                    }
                    else if (m < 0)
                    {
                        // 出错
                        if(errno != EWOULDBLOCK && errno != EINTR){
                            if(epoll_ctl(epollfd, EPOLL_CTL_DEL, epoll_events[i].data.fd, NULL) != -1){
                                cout << "client disconnected, clientfd: " << epoll_events[i].data.fd << endl;
                            }
                            close(epoll_events[i].data.fd);
                        }
                    }
                    else
                    {
                        // 正常接收数据
                        cout << "recv from client: " << epoll_events[i].data.fd << ", " << ch << endl;
                    }
                }
            
            }
            else if (epoll_events[i].events & EPOLLOUT)
            {
                // 只处理客户端fd 的可写事件
                if(epoll_events[i].data.fd != listenfd){
                    // 打印结果
                    cout << "EPOLLOUT triggered, clientfd : " << epoll_events[i].data.fd << endl;
                }
            }

            else if (epoll_events[i].events & EPOLLERR)
            {
                // TODO: 暂不处理
            }
        }
    }

    close(listenfd);
    return 0;
}
