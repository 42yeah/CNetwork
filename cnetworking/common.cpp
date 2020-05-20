//
//  common.cpp
//  cnetworking
//
//  Created by 周昊 on 2020/5/20.
//  Copyright © 2020 aiofwa. All rights reserved.
//

#include "common.hpp"


in_addr strToInAddr(char *input) {
    hostent *he;
    in_addr result;
    result.s_addr = INADDR_LOOPBACK;

    if ((he = gethostbyname(input)) == nullptr) {
        return result;
    }
    memcpy(&result, he->h_addr_list[0], he->h_length);
    return result;
}
