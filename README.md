# jetson-h264-decode
jetson硬件平台解码代码

主要功能是将jetson的一系列解码调用封装成decode库，方便二次调用。

调用InitDecode函数+DoDecodeProcess函数，即可实现从H264解码成rgb图像（参考test.cpp）

代码暂时不方便公开，可以参考https://docs.nvidia.com/jetson/l4t-multimedia/l4t_mm_00_video_decode.html

本人也是从该代码修改而来
