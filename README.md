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
* **2024/5/2**
  * 添加BallPosition类，用于记录同一个球在多摄像头下的位置
  * 修改Ball类及其相关类
* **2024/5/9**
  * CameraLoader合并为CameraManager，为每个CameraLoader分配连续的cameraId
  * EngineLoader适配多batch和动态batch模型
  * 俯仰角和偏航角集成到Parameters，不再作为CameraLoader成员变量
  * 宏定义优化，区分R2_GEN2_VER1和R2_GEN2_VER2
  * 其他的相关优化
* **2024/5/25**
  * 命名空间优化
  * TrtEngineLoader改用cv-cuda加速图像预处理，改用libtorch和torchvision加速推理后处理
  * OvEngineLoader停止维护。相关接口类和宏定义保留。
* **2024/5/30**
  * RsCamera启动优化
  * 添加常量宏定义，日志记录优化
  * FrontDataProcessor修改逻辑，识别一个框和球作为近框决策
* **2024/6/4**
  * 项目整体结构重构与优化
  * 为每个相机单独创建一个线程，防止断连阻塞主线程
* **2024/6/5**
  * RsCameraLoader添加尝试重连相机的逻辑
* **2024/6/6**
  * 串口相关函数优化
  * 日志与输出优化
* **2024/6/17**
  * DataCenter修正cameraImages_的索引逻辑错误
  * RsCameraLoader优化reconnect异常处理逻辑
* **2024/6/22**
  * SAVE_VIDEO宏定义优化
* **2024/6/24**
  * 添加多线程版本的WideFieldCameraLoader
  * 添加ICameraLoader接口便于CameraManager管理
  * CameraManager添加检测单目相机的逻辑
  * DataCenter处理前场信息时去除紫球，删除识别一个框和球作为近框决策的逻辑
  * 其他与ICameraLoader接口相关的优化
  * 类型定义优化、命名优化和输出优化
* **2024/6/28**
  * 修改TrtEngineLoader，适配带有TensorRT Efficient NMS Plugin的模型
* **2024/7/4**
  * TrtEngineLoader预处理修改，适配不同大小的图像输入
  * TrtEngineLoader后处理优化
  * Ball修改merge方法
  *

## 使用说明

## 功能列表

## 外部依赖

## 关于作者