# hdu-hikvision embedded class

基础要求：
- [x] 旋转 180 度
- [x] osd 或 mask 改变颜色
- [x] osd 内容写成小组队名
- [x] 设备端存 1 分钟录像

加分项目：
- [x] osd 从勾边改为自动反色
- [x] 设置标准时间
- [x] 开始发流的时候，灯亮起来
- [x] pc 端保存 1 分钟录像

## 说明

1. mask 可以有多块，但颜色都一样
2. osd 内容还未修改
3. 设备端保存的录像为 `/home/h264.ps`
4. 使用 c/c++ 内置函数获取时间，并不是现实时间
5. 运行程序之前需要以下操作
```shell
cd ./driver && ./load_modules_FH8862.sh && cd ..
cp ./lib/imx415_mipi_attr.hex /home
（前两行命令可用 ./went2 执行）
cd mod_led && make && insmod led_ctrl.ko
mknod /dev/led_ctrl c 251 0
# 数字 `251` 需要根据 `cat /proc/devices` 中的 `led_ctrl` 的主设备号来确定
```
6. 由于字库 `inc/font_array.h` 中对中文只有 `gb2312` 的支持，所以除了 `main.cpp` 使用 `gb2312` 之外，其他文件都使用 `utf-8` 编码

7. `tcp` 文件夹下的代码需拷贝到 pc 下 作为服务端使用，由于 Ubuntu 的用户权限，需要自己先创建储存位置的文件
