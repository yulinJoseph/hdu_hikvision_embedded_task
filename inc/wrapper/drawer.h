#pragma once

#ifndef __yulin_DRAWER_H__
#define __yulin_DRAWER_H__

#include <string>

#include "dsp/fh_vpu_mpi.h"
#include "dsp_ext/FHAdv_OSD_mpi.h"

class Drawer {
private:
    FHT_OSD_Layer_Config_t osd_layer_conf[4];
    FHT_OSD_TextLine_t text_line_conf[4];
    FH_CHAR text_data[4][128];

    FH_VPU_MASK mask_conf;

protected:
public:
    Drawer();
    ~Drawer();

    bool init();
    bool put_text(int id, std::string &&text, int x, int y);
    bool draw_mask(int id, int x, int y, int w, int h, int color);
};

#endif
