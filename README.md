# ROBOCON 2023

## 北京科技大学 MEI 竞技机器人团队

### University of Science and Technology

### MEI Contest Robot Team

## 传感器组 R2摄像头工程源码

### Project R2 Camera of Sensor Department

## 项目简介

## 更新日志

#### R2-GEN1

* **2023/11/10**
  * 新增r2-gen1分支，上传四摄像头4cls模型代码
* **2023/11/14**
  * Ball添加isInBasket_，适配新的7cls模型
  * BackDataProcessor添加逻辑：删除框内球
  * FrontDataProcessor修改画图逻辑为：先画所有物体（红色），再画有效物体（绿色）
  * TrtEngineLoader优化部分变量，添加逻辑：判断模型classNum_，若为7cls模型则对标签索引进行处理
* **2023/11/15**
  * BackDataProcessor修改错误：判断pickedBallsIndex_是否为空
  * FrontDataProcessor修改frontDataProcess的逻辑错误，简化逻辑
* **2023/12/2**
  * RsCameraGroup修改r2-gen1下方摄像头到码盘偏移量
  * RsCameraLoader添加偏航角，添加逻辑：获取深度坐标时邻近采样防止深度“黑洞”
  * BackDataProcessor添加逻辑：特判球被举起的情况
* **2023/12/12**
  * TrtEngineLoader代码格式优化
  * 添加OvEngineLoader类，实现基于OpenVINO库调用Intel CPU/GPU进行推理的功能
  * RsCameraGroup与WideFieldCameraGroup添加OvEngineLoader相关重载函数
  * 宏定义优化
* **2024/1/23**
  * BackDataProcessor修改backDataProcess逻辑错误，添加吸起球-整排球-散球逻辑
  * DataSender发送长度拓展为32
  * 部分参数优化
* **2024/1/24**
  * BackDataProcessor修改backDataProcess逻辑错误，优化逻辑
  * FrontDataProcessor修改outputPosition参数错误
  * 宏定义优化
* **2024/1/30**
  * BackDataProcessor修改backDataProcess逻辑，大于两个球成排都认为是一排球
* **2024/2/3**
  * BackDataProcessor整体逻辑修改
* **2024/3/4**
  * BackDataProcessor修改backDataProcess逻辑，大于两个球成排都认为是一排球
  * BackDataProcessor添加逻辑：checkDistance和positionRevise
* **2024/3/13**
* BackDataProcessor整体回滚
* FrontDataProcessor修改frontDataProcess的逻辑错误，简化逻辑
* 部分参数优化

#### R2-GEN2

* **2024/1/23**
  * 新增r2-gen2分支，部分参数优化

## 使用说明

## 功能列表

## 外部依赖

## 关于作者