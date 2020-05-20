//
//  common.hpp
//  cnetworking
//
//  Created by 周昊 on 2020/5/20.
//  Copyright © 2020 aiofwa. All rights reserved.
//

#ifndef common_hpp
#define common_hpp

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>


in_addr strToInAddr(char *input);

#endif /* common_hpp */
