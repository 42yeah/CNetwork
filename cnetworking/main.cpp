//
//  main.cpp
//  cnetworking
//
//  Created by 周昊 on 2020/5/20.
//  Copyright © 2020 aiofwa. All rights reserved.
//

#include <iostream>
#include "tcp.hpp"
#include "udp.hpp"


int main(int argc, const char * argv[]) {
    std::cout << "选择要启动的项目:" << std::endl;
    std::cout << "1. TCP" << std::endl
              << "2. UDP" << std::endl;
    
    int selection;
    std::cin >> selection;
    switch (selection) {
        case 1: tcpMain(); break;
        case 2: udpMain(); break;
        default: std::cout << "未知选项" << std::endl; break;
    }

    return 0;
}
