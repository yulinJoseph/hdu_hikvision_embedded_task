#pragma once

#include <string>

class LedCtrl {
private:
    std::string name;
    int fd;

protected:
public:
    LedCtrl();
    ~LedCtrl();

    void on();
    void off();
};
