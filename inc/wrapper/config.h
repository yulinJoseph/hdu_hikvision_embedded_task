#pragma once

#include "isp/isp_sensor_if.h"

/**
 * @brief 由于初始化内容比较多，多以构造时没有进行初始化，需要依次执行以下成员函数
 * 
 */
class Config {
private:
    int w;
    int h;
    int fps;
    int group_id;

protected:
public:
    explicit Config(int w, int h, int fps, int group_id);
    ~Config();

    bool init_media_cache();
    bool init_isp(isp_sensor_if *sensor_func, int vimod = 0, int vomod = 1);
    bool init_vpu();
};
