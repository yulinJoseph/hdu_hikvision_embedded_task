#include <iostream>
#include <string.h>

#include "drawer.h"
#include "font_array.h"
#include "wrapper/macro.h"

Drawer::Drawer() {
    auto res = init();
    if (!res) {
        std::cout << "Drawer init failed" << std::endl;
        std::cout << __FILE__ << ":" << __LINE__ << std::endl;
        exit(1);
    }

    memset(&mask_conf, 0, sizeof(mask_conf));
}

Drawer::~Drawer() {
}

bool Drawer::init() {
    int graph_ctrl = 0 | FHT_OSD_GRAPH_CTRL_TOSD_AFTER_VP;

    _IF_0(FHAdv_Osd_Init(0, FHT_OSD_DEBUG_LEVEL_ERROR, graph_ctrl, 0, 0));

    // 设置 ASC 码表
    FHT_OSD_FontLib_t font_lib{};
    font_lib.pLibData = asc16;
    font_lib.libSize = sizeof(asc16);
    _IF_0(FHAdv_Osd_LoadFontLib(FHEN_FONT_TYPE_ASC, &font_lib));

    // 设置中文字库
    font_lib.pLibData = gb2312;
    font_lib.libSize = sizeof(gb2312);
    _IF_0(FHAdv_Osd_LoadFontLib(FHEN_FONT_TYPE_CHINESE, &font_lib));

    FHT_OSD_CONFIG_t osd_cfg{};

    memset(&osd_cfg, 0, sizeof(osd_cfg));
    for (auto i = 0; i < 4; ++i) {
        memset(&osd_layer_conf[i], 0, sizeof(FHT_OSD_Layer_Config_t));
        memset(&text_line_conf[i], 0, sizeof(FHT_OSD_TextLine_t));
    }
    memset(&text_data, 0, sizeof(text_data));
    // 旋转
    osd_cfg.osdRotate = 2;
    osd_cfg.pOsdLayerInfo = &osd_layer_conf[0];
    osd_cfg.nOsdLayerNum = 1;

    osd_layer_conf[0].layerStartX = 0;
    osd_layer_conf[0].layerStartY = 0;
    osd_layer_conf[0].osdSize = 64;
    osd_layer_conf[0].normalColor.fAlpha = 255;
    osd_layer_conf[0].normalColor.fRed = 255;
    osd_layer_conf[0].normalColor.fGreen = 255;
    osd_layer_conf[0].normalColor.fBlue = 255;
    osd_layer_conf[0].invertColor.fAlpha = 255;
    osd_layer_conf[0].invertColor.fRed = 0;
    osd_layer_conf[0].invertColor.fGreen = 0;
    osd_layer_conf[0].invertColor.fBlue = 0;
    osd_layer_conf[0].edgeColor.fAlpha = 255;
    osd_layer_conf[0].edgeColor.fRed = 0;
    osd_layer_conf[0].edgeColor.fGreen = 0;
    osd_layer_conf[0].edgeColor.fBlue = 0;
    osd_layer_conf[0].bkgColor.fAlpha = 0;
    osd_layer_conf[0].edgePixel = 1;
    osd_layer_conf[0].osdInvertEnable = FH_OSD_INVERT_BY_CHAR;
    osd_layer_conf[0].osdInvertThreshold.high_level = 180;
    osd_layer_conf[0].osdInvertThreshold.low_level = 160;
    osd_layer_conf[0].layerFlag = FH_OSD_LAYER_USE_TWO_BUF;
    osd_layer_conf[0].layerId = 0;
    _IF_0(FHAdv_Osd_Ex_SetText(0, 0, &osd_cfg));

    for (auto i = 0; i < 4; ++i) {
        text_line_conf[i].textInfo = text_data[i];
        text_line_conf[i].textEnable = 0;
        text_line_conf[i].timeOsdEnable = 0;
        text_line_conf[i].textLineWidth = (64 / 2) * 36;
        text_line_conf[i].lineId = i;
        text_line_conf[i].enable = 1;
    }
    return true;
}

bool Drawer::put_text(int id, std::string &&text, int x, int y) {
    sprintf(text_data[id], "%s", text.c_str());
    text_line_conf[id].textEnable = 1;
    text_line_conf[id].linePositionX = x;
    text_line_conf[id].linePositionY = y;

    _IF_0(FHAdv_Osd_SetTextLine(0, 0, osd_layer_conf[0].layerId, &text_line_conf[id]));

    return true;
}

// attention: 所有 mask_conf 的颜色都是一样的
// FH_VPSS_SetMask() 函数执行第二次时，会把之前的覆盖掉，所以 mask_conf 是数组也没用
bool Drawer::draw_mask(int id, int x, int y, int w, int h, int color) {
    mask_conf.mask_enable[id] = FH_TRUE;
    mask_conf.area_value[id].u32X = x;
    mask_conf.area_value[id].u32Y = y;
    mask_conf.area_value[id].u32Width = w;
    mask_conf.area_value[id].u32Height = h;
    mask_conf.masaic.masaic_enable = 0;
    mask_conf.color = color;

    _IF_0(FH_VPSS_SetMask(0, &mask_conf));

    return true;
}
