# ROBOCON 2023

## 北京科技大学 MEI 竞技机器人团队

### University of Science and Technology

### MEI Contest Robot Team

## 传感器组 R2摄像头工程源码

### Project R2 Camera of Sensor Department

## 项目简介

## 更新日志

* **2023/11/10**
  * 上传四摄像头4cls模型代码
* **2023/11/14**
  * Ball添加isInBasket_，适配新的7cls模型
  * BackDataProcessor添加逻辑：删除框内球
  * FrontDataProcessor修改画图逻辑为：先画所有物体（红色），再画有效物体（绿色）
  * TrtEngineLoader优化部分变量，添加逻辑：判断模型classNum_，若为7cls模型则对标签索引进行处理
* **2023/11/15**
  * BackDataProcessor修改错误：判断pickedBallsIndex_是否为空
  * FrontDataProcessor修改frontDataProcess的逻辑错误，简化逻辑

## 使用说明

## 功能列表

## 外部依赖

## 关于作者