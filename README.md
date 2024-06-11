# Ding-BLE

Ding蓝牙动态模拟（使用ESP32核心开发板实现模拟蓝牙，请勿用于Ding打卡等非法用途！）


## 前言

Ding考勤机蓝牙实现方案是通过TOTP算法计算一次性密码并通过蓝牙广播，该密码大约30s刷新一次，较近范围内（无遮挡10米内）的手机设备通过搜索蓝牙广播获取其广播的Manufacturer Data数据，从里面解析出一次性密码并上传服务器进行对比，对比通过即认为手机在考勤机附近。

本文的实现方法为：使用ESP32（1）放置在考勤机附近，持续搜索考勤机发送的广播，当检测到Manufacturer Data数据变化时，则将数据发送到服务器；需要远程模拟时，使用ESP32（2）从服务器获取Manufacturer Data数据并模拟，手机接收其发送的Manufacturer Data数据。


## 环境准备

### 硬件环境

- 服务器环境：能够外网访问的php网站部署环境，负责Manufacturer Data数据的存储和转发
- ESP32核心开发板*2：作者使用ESP32-S3-N16R8进行测试，选择的开发板ROM建议选择4MB及以上
- WiFi环境：打卡机附近有稳定且可连接的WiFi

### 软件环境

- Arduino IDE：安装有esp32开发环境（建议使用2.0.17和2.0.6版本）


## 部署方案

1. 将[send.php](https://github.com/yvyan/Ding-BLE/blob/main/send.php)文件部署在服务器上，使用https://example.com/send.php?password=xxx&data=xxxx 即可上传Manufacturer Data数据数据，使用https://example.com/send.php?password=xxx 即可获取Manufacturer Data数据
2. Arduino IDE安装**ESP32 2.0.17**环境，若使用ESP32-S3-N16R8核心开发板，请选择**ESP32S3 Dev Module**，修改为个人信息，编译[SendClient.ino](https://github.com/yvyan/Ding-BLE/blob/main/SendClient.ino) 文件并烧录，记为模块1
3. Arduino IDE安装**ESP32 2.0.6**环境，若使用ESP32-S3-N16R8核心开发板，请选择**ESP32S3 Dev Module**，修改为个人信息，编译[ReadClient.ino](https://github.com/yvyan/Ding-BLE/blob/main/ReadClient.ino) 文件并烧录，记为模块2
4. 模块1放置在打卡机附近（无遮挡10米内）
5. 模块2随身携带，需要使用时供电供网，即可模拟成功

## 局限性
- 考勤机蓝牙打卡设置页面有“**位置异常检测**”功能开关，若管理员开启后，会采集本机位置与设定考勤机位置进行对比，出现较大偏差会判断为异常。  解决方案为结合LSP框架和位置模拟组件共同使用，有兴趣的可以参考：[Xposed-Modules-Repo/com.fuck.android.rimet: 锤锤 (github.com)](https://github.com/Xposed-Modules-Repo/com.fuck.android.rimet)

## 已知问题
- 蓝牙模拟代码仅在2.0.6库测试通过，不适用于较新库版本
- 蓝牙采集代码存在约40%概率获取不到Manufacturer Data数据


## 鸣谢&参考

蓝牙模拟实现参考：[zanjie1999/dingBle: esp32修改蓝牙mac地址 模拟蓝牙打卡机 (github.com)](https://github.com/zanjie1999/dingBle)

蓝牙采集实现参考：[蓝牙打卡破解 - 丁少华 - 博客园 (cnblogs.com)](https://www.cnblogs.com/dingshaohua/p/17148091.html)

