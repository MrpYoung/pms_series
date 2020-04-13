# pms_series

## 1、介绍

​         pms_series 软件包是 攀藤 pm2.5系列的驱动软件包，是基于激光散射原理的数字式通用颗粒物浓度传感器，可连续采集并计算单位体积内空气中不同粒径的悬浮颗粒物个数，即颗粒物浓度分布，进而换算成为质量浓度，并以通用数字接口形式输出。可嵌入各种与空气中悬浮颗粒物浓度相关的仪器仪表或环境改善设备，为其提供及时准确的浓度数据。

本软件包支持uart接口，目前仅支持主动式传输协议，支持的具体型号为下表：

| 名称      | 说明                          | 测试 |
| --------- | ----------------------------- | ---- |
| PMS5003ST | PM2.5、PM10、温度、湿度、甲醛 | √    |
| PMS5003S  | PM2.5、PM10、甲醛             |      |
| PMS5003T  | PM2.5、PM10、温度、湿度       | √    |
| PMS5003   | PM2.5、PM10                   |      |
| PMS7003   | PM2.5、PM10                   |      |
| PMS7003M  | PM2.5、PM10                   |      |
| PMS3003   | PM2.5、PM10                   |      |
| PMS1003   | PM2.5、PM10                   |      |
| PMSA003   | PM2.5、PM10                   |      |

​			手头只有PMS5003ST，所以只测试了这个，其他的型号协议大同小异，有条件的可以测测。

### 1.1 目录结构

| 名称 | 说明 |
| ---- | ---- |
| docs  | 文档目录 |
| examples | 例子目录，并有相应的一些说明 |
| inc  | 头文件目录 |
| src  | 源代码目录 |

### 1.2 许可证

pms_series 遵循 MIT许可，详见 `LICENSE` 文件。

### 1.3 依赖

- RT-Thread 4.0.1+
- 使用串口DMA需要开启相关接口

## 2、如何打开 pms_series

使用 pms_series package 需要在 RT-Thread 的包管理器中选择它，具体路径如下：

```
RT-Thread online packages --->                                                
	peripheral libraries and drivers  --->                                   		
	[*] pms_series: Digital universal particle concentration ...
                  	Model (PMS5003ST)  --->                                     
             	[*]  Enable pms series sample                           
             	[*]  Enable pms series sample dma 
                	 Enable pms series sample uart (UART2)
                  	Version (v1.0.0)  --->
```

**Model**: 选择PMS系列的型号，总共包含段落1中的表格的型号；

**Enable pms series sample **: 使用例程代码

**Enable pms series sample dma **：使用例程中串口DMA

**Enable pms series sample uart **：选择例程中使用的串口

然后让 RT-Thread 的包管理器自动更新，或者使用 `pkgs --update` 命令更新包到 BSP 中。

## 3、使用 pms_series 软件包

### 3.1 版本说明

| 版本   | 说明                                  |
| ------ | ------------------------------------- |
| v1.0.0 | 支持多种PMS系列传感器，支持主动式传输 |
| latest | 进一步优化                            |

### 3.2 API 说明

**初始化传感器**

初始化传感器

```
pms_device_t pms_init(const char *uart_name);
```

| 参数         | 描述           |
| ------------ | -------------- |
| uart_name    | 串口名称       |
| **返回**     | ——             |
| pms_device_t | 传感器对象句柄 |

**协议解析**

```
rt_err_t frame_check(pms_device_t dev,rt_uint8_t *buf,rt_uint16_t len);
```

协议解析函数，整包解析，直接应用在DMA接收上

| 参数     | 描述             |
| -------- | ---------------- |
| dev      | 传感器对象结构体 |
| buf      | 解析数据的缓存   |
| len      | 解析数据的长度   |
| **返回** | ——               |
| RT_EOK   | 解析成功         |
| RT_ERROR | 解析失败         |

**按字节解析**

```
rt_err_t pms_get_byte(pms_device_t dev, char data);
```

按字节解析函数，可用于串口字节接收上，接收到一帧数据后，会调用frame_check进行整包解析

| 参数     | 描述               |
| -------- | ------------------ |
| dev      | 传感器对象结构体   |
| data     | 接收的一个字节数据 |
| **返回** | ——                 |
| RT_EOK   | 解析成功           |
| RT_ERROR | 正在解析           |

**失能传感器**

```
void pms_deinit(pms_device_t dev);
```

释放申请的内存


## 4、注意事项

1. 例程中，有两种采集方式一种是字节解析，使用信号量，收到特定一帧后解析，另外一种是DMA，收到一帧数据然后进行解析。
2. 协议解析出来的数据统一为 uint16 方便用户用作其他处理，数据具体的单位以及转换请参考/doc文件夹下的手册。

## 5、联系方式 & 感谢

* 维护：623216350@qq.com
* 主页：https://github.com/MrpYoung/pms_series
