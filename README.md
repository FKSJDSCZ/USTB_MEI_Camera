# ROBOCON 2024

## 北京科技大学 MEI 竞技机器人团队

### MEI Contest Robot Team, University of Science and Technology Beijing

## 传感器组 R2摄像头工程源码

### Project R2 Camera, Sensor Department

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
* **2024/3/25**
  * 项目整体结构重构与优化
* **2024/4/1**
  * TrtEngineLoader与OvEngineLoader修改为适配YOLOv8模型
  * WideFieldCameraGroup取消了二次识别逻辑
* **2024/4/19**
  * EngineLoader修改推理逻辑错误
  * 整体代码优化
* **2024/4/20**
  * 代码合并到r2-gen2分支，该分支停止维护

#### R2-GEN2

* **2024/1/23**
  * 新增r2-gen2分支，部分参数优化
* **2024/3/20**
  * FrontDataProcessor与r2-gen1同步
* **2024/4/5**
  * 项目整体结构重构与优化，与r2-gen1同步
* **2024/4/17**
  * BackDataProcessor添加逻辑：判断前进路线上是否有球
* **2024/4/20**
  * 接受来自r2-gen1的合并
* **2024/4/22**
  * CameraLoader添加逻辑，将推理后的图片作为视频流保存到本地
  * CameraManager优化，分离画图、显示和保存逻辑
  * TrtEngineLoader改用智能指针和新的TensorRT API，修改后处理逻辑错误
  * 添加Logger单例类，用于记录报错信息
  * 使用C++20标准
* **2024/4/23**
  * OvEngineLoader修改后处理逻辑错误
  * main函数捕获所有中断信号和异常，记录并正常退出程序
* **2024/5/1**
  * Ball优化了参数校准的逻辑
  * BackDataProcessor添加去重逻辑，优化了部分逻辑

## 使用说明

## 功能列表

## 外部依赖

## 关于作者