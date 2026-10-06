# Overview
- **Name**: LSM6DSR-V1.0
- **MCU**: STM32C542CCT6
- **IDE**: STM32CUBEMX2+STM32CUBEIDE


# Buy Link
[https://shop192352884.taobao.com/](https://shop192352884.taobao.com/)





# Contact Information

- **Name**: Billy
- **交流群**: 925643491
- **Email**: a845656974@outlook.com
- **Phone**: +86 15622736378
- **CSDN Blog**: [Blog](https://blog.csdn.net/qq_24312945)
- **Video**: [Video](https://space.bilibili.com/26152390)



# Project Introduction
- **STM32C5_LSM6DSR_Project1**:STM32C5开发LSM6DSR(1)----轮询获取陀螺仪数据
- **CSDN Blog**:

本章介绍如何使用 STM32C5 通过 I²C 接口驱动 LSM6DSV320X，以轮询方式读取三轴角速度数据。程序首先完成设备 ID 检查、软件复位和基本参数配置，设置陀螺仪的输出数据率与量程，并使能 BDU（块数据更新）。主循环检查数据就绪状态，读取原始角速度数据并转换为对应的物理单位，最后通过串口输出采集结果。

This chapter introduces how to use the **STM32C5** to communicate with the **LSM6DSV320X** over **I²C** and acquire three-axis angular velocity data through polling. The program performs device ID verification, a software reset, and basic configuration, including the gyroscope output data rate, full-scale range, and **Block Data Update (BDU)**. The main loop checks the data-ready status, reads the raw angular velocity data, converts it into physical units, and outputs the results through UART.

- **STM32C5_LSM6DSR_Project2**:STM32C5开发LSM6DSR(2)----中断获取陀螺仪数据
- **CSDN Blog**:

本章节介绍基于 STM32C542 的 LSM6DSR 陀螺仪中断采集功能。通过配置 INT1 数据就绪中断，当陀螺仪产生新数据时触发 MCU 中断，读取三轴角速度数据并进行单位转换，实现高实时性的运动数据采集。 
This section introduces the **LSM6DSR gyroscope interrupt-based data acquisition** using the **STM32C542**. By configuring the **INT1 data-ready interrupt**, an MCU interrupt is triggered whenever new gyroscope data is available. The three-axis angular velocity data is then read and converted into physical units, enabling real-time motion data acquisition with low latency.


- **STM32C5_LSM6DSR_Project3**:STM32C5开发LSM6DSR(3)----配置单击与双击检测
- **CSDN Blog**:

本章介绍如何使用 STM32C542CCT6 驱动 LSM6DSR 实现单击（Single Tap）和双击（Double Tap）检测。LSM6DSR 内置硬件 Tap 检测功能，可通过低 G 加速度计对敲击动作进行识别，并支持将单击、双击事件映射到 INT1/INT2 中断引脚。 

This chapter introduces how to use the **STM32C542CCT6** to drive the **LSM6DSR** for **Single Tap** and **Double Tap** detection. The LSM6DSR features built-in hardware tap detection, which uses the low-g accelerometer to recognize tapping events and supports routing single-tap and double-tap events to the **INT1/INT2 interrupt pins**.

- **STM32C5_LSM6DSR_Project4**:STM32C5开发LSM6DSR(4)----高级计步器、步数检测与步数计数 
- **CSDN Blog**:
本章将在前面外部中断配置的基础上，使能计步器并设置 Debounce 参数，将 Step Detector 事件映射至 INT1；当检测到有效步数后，由 STM32C542 响应外部中断并读取芯片内部 Step Counter，实现步数检测与实时计数。  
Based on the external interrupt configuration introduced earlier, this chapter enables the **pedometer** function and configures the **debounce parameters**, then routes the **Step Detector** event to **INT1**. When a valid step is detected, the **STM32C542** responds to the external interrupt and reads the internal **Step Counter** of the LSM6DSR, enabling real-time step detection and counting.

- **STM32C5_LSM6DSR_Project5**:STM32C5开发LSM6DSR(5)----显著运动检测   
- **CSDN Blog**:
本章介绍如何使用 STM32C542CCT6 驱动 LSM6DSR 实现 Wake-up 运动检测。LSM6DSR 内置可配置的 Wake-up 标准中断，可利用 Low-G 加速度计对设备的动态运动进行检测，并在运动幅度超过设定阈值后通过 INT1/INT2 输出中断信号。  

This chapter introduces how to use the **STM32C542CCT6** to drive the **LSM6DSR** for **Wake-up motion detection**. The LSM6DSR integrates a configurable **Wake-up interrupt** function that uses the **low-g accelerometer** to detect device motion. When the detected acceleration exceeds the configured threshold, an interrupt signal can be generated through the **INT1/INT2 pins**.


- **STM32C5_LSM6DSR_Project6**:STM32C5开发LSM6DSR(6)----倾斜度检测
- **CSDN Blog**:

本章介绍如何使用 STM32C542CCT6 驱动 LSM6DSR 实现 Tilt 倾斜检测。程序使能并初始化芯片内部的倾斜检测算法，配置嵌入式功能中断锁存，并将 Tilt 事件映射到 INT1。当设备姿态变化满足内部算法判定条件时，MCU 读取事件状态并通过串口输出检测结果。该功能用于检测倾斜变化事件，不直接输出具体的倾斜角度。

This chapter introduces how to use the **STM32C542CCT6** to drive the **LSM6DSR** for **Tilt detection**. The program enables and initializes the embedded tilt algorithm, configures latched embedded-function interrupts, and routes Tilt events to **INT1**. When an orientation change meets the algorithm’s detection criteria, the MCU reads the event status and reports the result through UART. This function detects tilt events without directly providing numerical tilt angles.


- **STM32C5_LSM6DSR_Project7**:STM32C5开发LSM6DSR(7)----自由落体检测
- **CSDN Blog**:

本章介绍如何使用 STM32C542CCT6 驱动 LSM6DSR 实现自由落体检测。程序配置加速度阈值和持续时间，当三轴加速度的绝对值同时低于设定阈值并满足时间条件时，芯片产生自由落体事件，并通过 INT1 通知 MCU。主循环读取事件状态，通过串口输出检测结果。

This chapter introduces how to use the **STM32C542CCT6** to drive the **LSM6DSR** for **Free-fall detection**. The program configures an acceleration threshold and a duration requirement. When the absolute acceleration on all three axes remains below the threshold for the required duration, the sensor generates a free-fall event and notifies the MCU through **INT1**. The main loop reads the event status and reports the result through UART.


- **STM32C5_LSM6DSR_Project8**:STM32C5开发LSM6DSR(8)----6D方向检测
- **CSDN Blog**:

本章介绍如何使用 STM32C542CCT6 驱动 LSM6DSR 实现 6D 方向检测。程序关闭 4D 模式，设置方向识别阈值，并将 6D 事件映射到 INT1。当设备姿态满足方向判定条件时，MCU 读取方向状态，通过串口输出 X+、X−、Y+、Y−、Z+ 或 Z−，实现六种基本方向的识别。

This chapter introduces how to use the **STM32C542CCT6** to drive the **LSM6DSR** for **6D orientation detection**. The program disables 4D mode, configures the orientation threshold, and routes 6D events to **INT1**. When the device orientation meets the detection criteria, the MCU reads the direction flags and reports **X+, X−, Y+, Y−, Z+, or Z−** through UART.


- **STM32C5_LSM6DSR_Project9**:STM32C5开发LSM6DSR(9)----活动与静止检测
- **CSDN Blog**:

本章介绍如何使用 STM32C542CCT6 驱动 LSM6DSR 实现活动与静止检测。程序配置运动阈值和静止等待时间，根据经过滤波的加速度数据判断设备状态，并将活动与静止状态变化事件映射到 INT1。MCU 读取状态后，通过串口输出 Activity 或 Inactivity 提示。本例在静止状态下保持加速度计和陀螺仪的工作配置不变。

This chapter introduces how to use the **STM32C542CCT6** to drive the **LSM6DSR** for **Activity and Inactivity detection**. The program configures motion thresholds and an inactivity time window, determines the device state from filtered acceleration data, and routes state-change events to **INT1**. The MCU reads the status and reports **Activity** or **Inactivity** through UART. In this example, the accelerometer and gyroscope configurations remain unchanged during inactivity.


- **STM32C5_LSM6DSR_Project10**:STM32C5开发LSM6DSR(10)----FIFO水位中断与六轴数据读取
- **CSDN Blog**:

本章介绍如何使用 STM32C542CCT6 驱动 LSM6DSR 实现 FIFO 缓存与水位中断读取。程序将 Low-G 加速度计和陀螺仪数据分别以 60 Hz 的速率写入 FIFO，配置连续模式，并设置 128 条记录的水位阈值。达到水位后，INT1 通知 MCU 批量读取数据，根据 TAG 区分加速度和角速度，完成单位换算，并通过串口输出本批次的样本数量及三轴平均值。

This chapter introduces how to use the **STM32C542CCT6** to drive the **LSM6DSR** for **FIFO buffering and watermark interrupt-based acquisition**. Low-g accelerometer and gyroscope data are each written to the FIFO at **60 Hz**. The FIFO operates in continuous mode with a watermark of **128 records**. When the watermark is reached, **INT1** notifies the MCU to read the buffered data in a batch. The program identifies acceleration and angular velocity records by their **TAG**, converts the measurements into physical units, and outputs the sample counts and three-axis averages through UART.
