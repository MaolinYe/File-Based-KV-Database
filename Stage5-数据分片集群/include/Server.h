//
// Created by YEZI on 2024/5/24.
//

#ifndef SEVER_H
#define SEVER_H
#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <unordered_map>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sstream>
#include "application.h"
#include "logger.h"
#include "HashRing.h"
#define MAX_EVENTS 8
#define PORT 8888
#define BUFFER_SIZE 512
#define BACKLOG_SIZE 16 // 请求队列最大长度

class Server {
private:
    uint16_t port;
    int server_fd = -1;
    int epoll_fd = -1;
    sockaddr_in server_addr{}, client_addr{};
    socklen_t client_addr_len = sizeof(client_addr);
    epoll_event event{}, events[MAX_EVENTS]{};
    Application *application = nullptr;
    std::unordered_map<size_t, int> servers{};

public:
    explicit Server(uint16_t port = PORT): port(port) {
        // 初始化日志
        Logger::InitLogger("server.log");
        // 初始化服务器节点间连接
        for (const auto &serverPort: ServerPorts) {
            if (serverPort == this->port)
                continue;
            servers.insert({serverPort, -1});
        }
        // 创建KVDB应用程序
        application = new Application;

        // 创建套接字
        // AF_INET :   表示使用 IPv4 地址		可选参数
        // SOCK_STREAM 表示使用面向连接的数据传输方式，
        // IPPROTO_TCP 表示使用 TCP 协议
        server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (server_fd == -1) {
            std::cerr << "Failed to create socket\n";
            LOG(LogLevel::ERROR, "Failed to create socket");
            exit(EXIT_FAILURE);
        }

        // 设置服务器地址
        server_addr.sin_family = AF_INET; // IPv4
        server_addr.sin_addr.s_addr = INADDR_ANY; // INADDR_ANY：0.0.0.0 表示本机所有IP地址
        server_addr.sin_port = htons(port);

        // 绑定套接字
        if (bind(server_fd, (sockaddr *) &server_addr, sizeof(server_addr)) == -1) {
            std::cerr << "Failed to bind socket\n";
            LOG(LogLevel::ERROR, "Failed to bind socket");
            exit(EXIT_FAILURE);
        }

        // 监听套接字
        if (listen(server_fd, BACKLOG_SIZE) == -1) {
            std::cerr << "Failed to listen on socket\n";
            LOG(LogLevel::ERROR, "Failed to listen on socket");
            exit(EXIT_FAILURE);
        }

        // 创建 epoll 实例
        epoll_fd = epoll_create1(0); // flag设置为0同epoll_create()
        if (epoll_fd == -1) {
            std::cerr << "Failed to create epoll instance\n";
            LOG(LogLevel::ERROR, "Failed to create epoll instance");
            exit(EXIT_FAILURE);
        }

        // 将服务器套接字添加到 epoll 实例中
        event.events = EPOLLIN | EPOLLET; // 监听事件类型 EPOLLIN表示有数据可读 EPOLLET表示边缘触发仅在状态变化时通知
        event.data.fd = server_fd;
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event) == -1) {
            std::cerr << "Failed to add server socket to epoll\n";
            LOG(LogLevel::ERROR, "Failed to add server socket to epoll");
            exit(EXIT_FAILURE);
        }
        const std::string log = "Server started. Listening on port " + std::to_string(port);
        std::cout << log << std::endl;
        LOG(LogLevel::INFO, log.c_str());
    }

    void run() {
        while (true) {
            // 使用 epoll 等待事件 参数timeout为等待时间，-1等死
            int num_ready = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
            if (num_ready == -1) {
                std::cerr << "Error in epoll_wait\n";
                LOG(LogLevel::ERROR, "Error in epoll_wait");
                exit(EXIT_FAILURE);
            }

            for (int i = 0; i < num_ready; ++i) {
                if (events[i].data.fd == server_fd) {
                    // 有新的连接请求
                    const int client_fd = accept(server_fd, (sockaddr *) &client_addr, &client_addr_len);
                    if (client_fd == -1) {
                        std::cerr << "Failed to accept client connection\n";
                        LOG(LogLevel::ERROR, "Failed to accept client connection");
                        continue;
                    }
                    std::ostringstream log;
                    log << "New connection from " << inet_ntoa(client_addr.sin_addr)
                            << ":" << ntohs(client_addr.sin_port);
                    std::cout << log.str() << std::endl;
                    LOG(LogLevel::INFO, log.str().c_str());

                    // 将新的客户端套接字添加到 epoll 实例中
                    event.events = EPOLLIN | EPOLLET;
                    event.data.fd = client_fd;
                    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event) == -1) {
                        std::cerr << "Failed to add client socket to epoll\n";
                        LOG(LogLevel::ERROR, "Failed to add client socket to epoll");
                        exit(EXIT_FAILURE);
                    }
                } else {
                    // 有数据到达现有客户端套接字
                    const int client_fd = events[i].data.fd;
                    char buffer[BUFFER_SIZE]{};
                    ssize_t bytes_received = recv(client_fd, buffer, BUFFER_SIZE, 0);
                    if (bytes_received <= 0) {
                        if (bytes_received == 0) {
                            // 客户端关闭连接
                            getpeername(client_fd, (sockaddr *) &client_addr, &client_addr_len);
                            std::ostringstream log;
                            log << "Client disconnected " << inet_ntoa(client_addr.sin_addr)
                                    << ":" << ntohs(client_addr.sin_port);
                            std::cout << log.str() << std::endl;
                            LOG(LogLevel::INFO, log.str().c_str());
                        } else {
                            std::cerr << "Error in recv\n";
                            LOG(LogLevel::ERROR, "Error in recv");
                        }
                        // 关闭客户端套接字，并从 epoll 实例中移除
                        close(client_fd);
                        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, nullptr);
                    } else {
                        // 接收到数据解析key寻找正确数据分片节点
                        std::istringstream iss(buffer);
                        std::string todo, key;
                        iss >> todo;
                        todo = Application::Standardize(todo);
                        if (todo == "set" || todo == "get" || todo == "del" || todo == "expire") {
                            iss >> key;
                        }
                        int rightServerPort = GetHashRingServer(key);
                        // 如果是本节点处理的数据
                        if (key.empty() || rightServerPort == port) {
                            // 转发给KVDB应用解析处理
                            std::string response = application->ParseInstruction(buffer);
                            send(client_fd, response.c_str(), response.size(), 0);
                        } else {
                            // 转发其他节点处理
                            if (servers[rightServerPort] == -1) {
                                // 还没连接其他节点需要建立连接
                                servers[rightServerPort] = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
                                if (servers[rightServerPort] == -1) {
                                    std::cerr << "Failed to create socket\n";
                                    LOG(LogLevel::ERROR, "Failed to create socket");
                                }
                                sockaddr_in serverNode_addr{};
                                serverNode_addr.sin_family = AF_INET; // IPv4
                                serverNode_addr.sin_addr.s_addr = INADDR_ANY; // INADDR_ANY：0.0.0.0 表示本机所有IP地址
                                serverNode_addr.sin_port = htons(rightServerPort);
                                if (connect(servers[rightServerPort], (sockaddr *) &serverNode_addr,
                                            sizeof(serverNode_addr)) < 0) {
                                    std::string log = "Failed to connect Server:" + std::to_string(rightServerPort);
                                    std::cerr << log << std::endl;
                                    LOG(LogLevel::ERROR, log.c_str());
                                } else {
                                    std::ostringstream log;
                                    log << "Success to connect Server:" << rightServerPort;
                                    std::cout << log.str() << std::endl;
                                    LOG(LogLevel::INFO, log.str().c_str());
                                }
                            }
                            // 已经连接过直接转发到相应节点
                            send(servers[rightServerPort], buffer, bytes_received, 0);
                            std::ostringstream log;
                            log << "Transmit to Server:" << rightServerPort;
                            std::cout << log.str() << std::endl;
                            LOG(LogLevel::INFO, log.str().c_str());
                            char response[BUFFER_SIZE]{};
                            ssize_t bytes_response = recv(servers[rightServerPort], response, BUFFER_SIZE, 0);
                            send(client_fd, response, bytes_response, 0);
                        }
                    }
                }
            }
        }
    }

    ~Server() {
        // 关闭服务器套接字和 epoll 实例
        close(server_fd);
        close(epoll_fd);
    }
};
#endif //SEVER_H
