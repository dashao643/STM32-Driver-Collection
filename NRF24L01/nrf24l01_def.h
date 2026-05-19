#ifndef __NRF24L01_DEF_H
#define __NRF24L01_DEF_H

// 指令码
#define NRF24L01_CMD_R_REGISTER			      0x00	// 读寄存器，高3位为指令码，低5位为寄存器地址，后续跟1~5字节读数据
#define NRF24L01_CMD_W_REGISTER			      0x20	// 写寄存器，高3位为指令码，低5位为寄存器地址，后续跟1~5字节写数据
#define NRF24L01_CMD_R_RX_PAYLOAD		      0x61	// 读Rx有效载荷，后续跟1~32字节读数据
#define NRF24L01_CMD_W_TX_PAYLOAD		      0xA0	// 写Tx有效载荷，后续跟1~32字节写数据
#define NRF24L01_CMD_FLUSH_TX			        0xE1	// 清空Tx FIFO所有数据，单独指令
#define NRF24L01_CMD_FLUSH_RX			        0xE2	// 清空Rx FIFO所有数据，单独指令
#define NRF24L01_CMD_REUSE_TX_PL		      0xE3	// 重新使用最后一次发送的有效载荷，单独指令
#define NRF24L01_CMD_R_RX_PL_WID		      0x60	// 读取Rx FIFO最前面一个数据包的宽度，后续跟1字节读数据，仅适用于动态包长模式
#define NRF24L01_CMD_W_ACK_PAYLOAD		    0xA8	// 写应答附带的有效载荷，高5位为指令码，低3位为通道号，后续跟1~32字节写数据，仅适用于应答附带载荷模式
#define NRF24L01_CMD_W_TX_NOACK	          0xB0	// 写Tx有效载荷，不要求应答，后续跟1~32字节写数据，仅适用于不要求应答模式
#define NRF24L01_CMD_NOP				          0xFF	// 空操作，单独指令，可以用读取状态寄存器
 
// 寄存器地址 
#define NRF24L01_ADDR_CONFIG				      0x00	// 配置寄存器，1字节
#define NRF24L01_ADDR_EN_AA				        0x01	// 使能自动应答，1字节
#define NRF24L01_ADDR_EN_RXADDR			      0x02	// 使能接收通道，1字节
#define NRF24L01_ADDR_SETUP_AW			      0x03	// 设置地址宽度，1字节
#define NRF24L01_ADDR_SETUP_RETR			    0x04	// 设置自动重传，1字节
#define NRF24L01_ADDR_RF_CH				        0x05	// 射频通道，1字节
#define NRF24L01_ADDR_RF_SETUP			      0x06	// 射频相关参数设置，1字节
#define NRF24L01_ADDR_STATUS				      0x07	// 状态寄存器，1字节
#define NRF24L01_ADDR_OBSERVE_TX			    0x08	// 发送观察寄存器，1字节
#define NRF24L01_ADDR_RPD				          0x09	// 接收功率检测，1字节
#define NRF24L01_ADDR_RX_ADDR_P0			    0x0A	// 接收通道0地址，5字节
#define NRF24L01_ADDR_RX_ADDR_P1			    0x0B	// 接收通道1地址，5字节
#define NRF24L01_ADDR_RX_ADDR_P2			    0x0C	// 接收通道2地址，1字节，高位地址与接收通道1相同
#define NRF24L01_ADDR_RX_ADDR_P3			    0x0D	// 接收通道3地址，1字节，高位地址与接收通道1相同
#define NRF24L01_ADDR_RX_ADDR_P4			    0x0E	// 接收通道4地址，1字节，高位地址与接收通道1相同
#define NRF24L01_ADDR_RX_ADDR_P5			    0x0F	// 接收通道5地址，1字节，高位地址与接收通道1相同
#define NRF24L01_ADDR_TX_ADDR			        0x10	// 发送地址，5字节
#define NRF24L01_ADDR_RX_PW_P0			      0x11	// 接收通道0有效载荷数据宽度，1字节
#define NRF24L01_ADDR_RX_PW_P1			      0x12	// 接收通道1有效载荷的数据宽度，1字节
#define NRF24L01_ADDR_RX_PW_P2			      0x13	// 接收通道2有效载荷的数据宽度，1字节
#define NRF24L01_ADDR_RX_PW_P3			      0x14	// 接收通道3有效载荷的数据宽度，1字节
#define NRF24L01_ADDR_RX_PW_P4			      0x15	// 接收通道4有效载荷的数据宽度，1字节
#define NRF24L01_ADDR_RX_PW_P5			      0x16	// 接收通道5有效载荷的数据宽度，1字节
#define NRF24L01_ADDR_FIFO_STATUS		      0x17	// 发送和接收FIFO状态，1字节
#define NRF24L01_ADDR_DYNPD				        0x1C	// 使能接收通道的动态包长模式，1字节
#define NRF24L01_ADDR_FEATURE			        0x1D	// 使能高级功能，1字节

#endif
