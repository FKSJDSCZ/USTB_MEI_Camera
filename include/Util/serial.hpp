#pragma once

#include <cstdio>      /*标准输入输出定义*/
#include <cstdlib>     /*标准函数库定义*/
#include <unistd.h>     /*Unix 标准函数定义*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>      /*文件控制定义*/
#include <termios.h>    /*PPSIX 终端控制定义*/
#include <cerrno>      /*错误号定义*/
#include <cstring>


//宏定义  
#define FALSE  (-1)
#define TRUE   0

/**
 * @brief 打开串口
 * @param fd 串口文件描述符
 * @param port 串口号(/dev/ttyUSB*)
 * @return 打开串口成功或失败
 */
int UART0_Open(int fd, const char *port);

/**
 * @brief 关闭串口
 * @param fd 文件描述符
 */
void UART0_Close(int fd);

/**
 * @brief 串口初始化
 * @param fd 串口文件描述符
 * @param speed 串口波特率
 * @param flow_ctrl 数据流控制符
 * @param databits 数据位(7 or 8)
 * @param stopbits 停止位(1 or 2)
 * @param parity 奇偶效验类型(N, E, O or S)
 * @return 设置成功或失败
 */
int UART0_Init(int fd, int speed, int flow_ctrl, int databits, int stopbits, int parity);

/**
 * @brief 接收串口数据
 * @param fd 串口文件描述符
 * @param rcv_buf 串口数据缓冲区
 * @param data_len 单帧数据的长度
 * @return 接收成功或失败
 */
int UART0_Recv(int fd, char *rcv_buf, int data_len);

/**
 * @brief 发送串口数据
 * @param fd 串口文件描述符
 * @param send_buf 串口数据缓冲区
 * @param data_len 单帧数据的长度
 * @return 发送成功或失败
 */
int UART0_Send(int fd, unsigned char *send_buf, int data_len);
