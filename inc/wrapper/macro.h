#pragma once

#include <fcntl.h>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define WR_PROC_DEV(device, cmd)                \
    {                                           \
        auto _tmp_fd = open(device, O_RDWR, 0); \
        if (_tmp_fd >= 0) {                     \
            write(_tmp_fd, cmd, sizeof(cmd));   \
            close(_tmp_fd);                     \
        }                                       \
    }

#define _IF_0(f)                                                   \
    {                                                              \
        auto res = f;                                              \
        if (res != 0) {                                            \
            std::cout << #f << " failed" << std::endl;             \
            std::cout << __FILE__ << ":" << __LINE__ << std::endl; \
            return false;                                          \
        }                                                          \
    }

#define _IF_bool(f)                                                \
    {                                                              \
        auto res = f;                                              \
        if (!res) {                                                \
            std::cout << #f << " failed" << std::endl;             \
            std::cout << __FILE__ << ":" << __LINE__ << std::endl; \
            return false;                                          \
        }                                                          \
    }

// ;\n
//
#define DEBUG std::cout << __FILE__ << ":" << __LINE__ << std::endl
