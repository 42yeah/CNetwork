//
//  udp.cpp
//  cnetworking
//
//  Created by 周昊 on 2020/5/20.
//  Copyright © 2020 aiofwa. All rights reserved.
//

#include "udp.hpp"
#include <iostream>
#include <random>
#include <thread>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/select.h>
#include "common.hpp"


class App {
public:
    App() {}
    
    void init() {
        sock = socket(AF_INET, SOCK_DGRAM, 0);
        bool done = false;
        std::uniform_int_distribution<> distrib(49152, 65535);
        std::random_device dev;
        sockaddr_in sin;
        sin.sin_family = AF_INET;
        sin.sin_addr.s_addr = INADDR_ANY;
        port = -1;
        while (!done) {
            port = distrib(dev);
            sin.sin_port = htons(port);
            int res = bind(sock, (sockaddr *) &sin, sizeof(sin));
            if (res >= 0) { done = true; }
        }
        running = true;
    }
    
    std::pair<sockaddr_in, std::string> recv() {
        sockaddr_in sin = { 0 };
        if (!running) { return std::pair<sockaddr_in, std::string>(sin, ""); }

        socklen_t slen = sizeof(sin);
        char buf[512] = { 0 };
        recvfrom(sock, buf, sizeof(buf), 0, (sockaddr *) &sin, &slen);
        return std::pair<sockaddr_in, std::string>(sin, std::string(buf));
    }
    
    void send(sockaddr_in sin, std::string msg) {
        if (!running) { return; }
        sendto(sock, msg.c_str(), msg.length(), 0, (sockaddr *) &sin, sizeof(sin));
    }
    
    int port;
    int sock;
    bool running;
};


void udpHandle(App *app) {
    while (app->running) {
        std::pair<sockaddr_in, std::string> info = app->recv();
        std::cout << "来自 " << inet_ntoa(info.first.sin_addr) << ":" << ntohs(info.first.sin_port) << " 的消息: " << info.second << std::endl;
    }
}


sockaddr_in sinify(char *str) {
    sockaddr_in ret;
    ret.sin_family = AF_INET;
    ret.sin_addr = strToInAddr(strtok(str, ":"));
    ret.sin_port = htons(atoi(strtok(nullptr, ":")));
    return ret;
}


int udpMain() {
    App app;

    app.init();
    std::cout << "本 UDP Socket 端口是 " << app.port << std::endl;
    std::thread background(udpHandle, &app);
    background.detach();

    std::cout << "发送消息要采用格式 地址|消息。" << std::endl
              << "例如: 127.0.0.1:12345|你好!" << std::endl;
    fflush(stdin);
    std::string input;
    while (app.running) {
        std::cout << "> ";
        std::getline(std::cin, input, '\n');
        if (input.length() <= 0) { continue; }
        char buf[512] = { 0 };
        memcpy(buf, input.c_str(), input.size());
        
        char *addr = strtok(buf, "|");
        char *msg = strtok(nullptr, "|");
        sockaddr_in sin = sinify(addr);
        app.send(sin, std::string(msg));
    }
    return 0;
}
