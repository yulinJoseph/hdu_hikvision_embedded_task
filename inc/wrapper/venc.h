#pragma once

class Venc {
private:
    int in_w;
    int in_h;
    int fps;
    int out_w;
    int out_h;
    int group_id;
    int chan;

protected:
public:
    explicit Venc(int in_w, int in_h, int fps, int out_w, int out_h, int group_id, int chan);
    ~Venc();

    bool init();
};
