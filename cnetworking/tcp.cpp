//
//  tcp.cpp
//  cnetworking
//
//  Created by 周昊 on 2020/5/20.
//  Copyright © 2020 aiofwa. All rights reserved.
//

#include "tcp.hpp"
#include <iostream>
#include <thread>
#include <vector>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include "common.hpp"


enum AppType {
    UNKNOWN, SERVER, CLIENT
};

class TCPApp {
public:
    TCPApp() : type(UNKNOWN) {}
    
    bool init() {
        sock = socket(AF_INET, SOCK_STREAM, 0);
        sockaddr_in sin;
        sin.sin_family = AF_INET;
        sin.sin_addr = address;
        sin.sin_port = htons(port);
        int result;

        switch (type) {
            case UNKNOWN:
                // You can't just waltz in uninitlialized!
                exit(2);
                break;
                
            case SERVER:
                result = bind(sock, (sockaddr *) &sin, sizeof(sin));
                listen(sock, 5);
                break;
                
            case CLIENT:
                result = connect(sock, (sockaddr *) &sin, sizeof(sin));
                break;
        }
        highest = sock + 1;
        running = !(result < 0);
        return running;
    }
    
    void stop() {
        switch (type) {
            case UNKNOWN:
                return;
                
            case SERVER:
                shutdown(sock, SHUT_RDWR);
                close(sock);
                break;
                
            case CLIENT:
                close(sock);
                break;
        }
        running = false;
    }
    
    std::string getTerm() {
        switch (type) {
            case SERVER:
                return "侦听错误";
                break;
                
            case CLIENT:
                return "连接错误";
                break;
                
            default:
                return "未知";
                break;
        }
    }
    
    void send(const char *msg) {
        if (!running) { return; }
        int len = (int) strlen(msg);
        switch (type) {
            case SERVER:
                for (int i = 0; i < connections.size(); i++) {
                    const int conn = connections[i];
                    ::send(conn, &len, sizeof(int), 0);
                    ::send(conn, msg, len, 0);
                }
                break;

            case CLIENT:
                ::send(sock, &len, sizeof(int), 0);
                ::send(sock, msg, len, 0);
                break;
                
            default:
                break;
        }
    }
    
    // CLIENT ONLY!
    std::vector<std::string> recv() {
        std::vector<std::string> messages;
        
        if (!running) { return messages; }
        
        switch (type) {
            case SERVER: {
                fd_set readfds;
                FD_ZERO(&readfds);
                FD_SET(sock, &readfds);
                for (int i = 0; i < connections.size(); i++) {
                    FD_SET(connections[i], &readfds);
                }
                timeval tv;
                tv.tv_sec = 0;
                tv.tv_usec = 50;
                int ret = select(highest, &readfds, nullptr, nullptr, &tv);
                if (ret > 0) {
                    if (FD_ISSET(sock, &readfds)) {
                        sockaddr_in sin;
                        socklen_t len = sizeof(sin);
                        int client = accept(sock, (sockaddr *) &sin, &len);
                        std::cout << "新连接已和服务端建立: " << inet_ntoa(sin.sin_addr) << ":" << sin.sin_port << std::endl;
                        connections.push_back(client);
                        highest = client + 1;
                    }
                    for (int i = 0; i < connections.size(); i++) {
                        const int conn = connections[i];
                        int expected;
                        char buf[512] = { 0 };
                        if (FD_ISSET(connections[i], &readfds)) {
                            ::recv(conn, &expected, sizeof(int), 0);
                            ssize_t len = ::recv(conn, buf, expected, 0);
                            if (len <= 0) {
                                connections.erase(connections.begin() + i, connections.begin() + i + 1);
                                i--;
                                std::cout << "有客户端和服务端已断开连接" << std::endl;
                                continue;
                            }
                            messages.push_back(std::string(buf));
                        }
                    }
                }
                break;
            }

            case CLIENT: {
                int expected = 0;
                char buf[512] = { 0 };
                ::recv(sock, &expected, sizeof(int), 0);
                ssize_t len = ::recv(sock, buf, expected, 0);
                if (len <= 0) {
                    std::cout << "和服务器的连接已经断开" << std::endl;
                    stop();
                    break;
                }
                messages.push_back(std::string(buf));
                break;
            }

            default:
                break;
        }
        return messages;
    }
    
    AppType type;
    in_addr address;
    int port;
    int sock;
    int highest;
    std::vector<int> connections;
    bool running;
};


void tcpHandle(TCPApp *app) {
    std::vector<std::string> messages;
    while (app->running) {
        messages = app->recv();
        for (std::string m : messages) {
            std::cout << "新消息: " << m << std::endl;
        }
    }
}


int getPort(char *input) {
    if (input == nullptr) { return -1; }
    return atoi(input);
}


int tcpMain() {
    TCPApp app;
    std::cout << "选择要启动的东西:" << std::endl;
    std::cout << "1. 服务器" << std::endl
              << "2. 客户端" << std::endl;
    
    int selection;
    char input[512] = { 0 };
    std::cin >> selection;
    switch (selection) {
        case 1:
            app.type = SERVER;
            std::cout << "侦听的端口: ";
            std::cin >> app.port;
            app.address.s_addr = INADDR_ANY;
            break;
            
        case 2: {
            app.type = CLIENT;
            std::cout << "地址 (用 地址:端口 形式): ";
            std::cin >> input;
            app.address = strToInAddr(strtok(input, ":"));
            app.port = getPort(strtok(nullptr, ":"));
            break;
        }
            
        default:
            std::cout << "啥？" << std::endl;
            exit(2);
            break;
    }
    if (!app.init()) {
        std::cout << "发生致命错误: " << app.getTerm() << "。正在停止" << std::endl;
        app.stop();
        return -1;
    }
    std::thread background(tcpHandle, &app);
    background.detach();

    std::cout << "连接已经建立。回车键发送消息。" << std::endl;
    fflush(stdin);
    while (app.running) {
        std::string msg;
        std::cout << "> ";
        std::getline(std::cin, msg, '\n');
        app.send(msg.c_str());
    }
    return 0;
}
