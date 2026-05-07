#include "modbus.h"
#include "uart_transport.h"
#include "register.h"

#define FRAME_TIMEOUT 4//定义帧超时时间：4ms
#define RX_BUF_LEN 256




//定义缓冲区和全局变量

static uint8_t rx_buf[RX_BUF_LEN];//接收缓冲区
static uint8_t rx_len=0;//接收缓冲区长度
static uint8_t frame_ready=0;


//内部函数
static void modbus_frame_process(void);
static void handle_read_hold(void);
static void handle_read_input(void);
static void handle_write_single_hold(void);
static void handle_write_single_coil(void);
static void send_exception(modbus_func_code func_code, modbus_ex_code ex_code);
static void send_response(uint8_t *buf, uint8_t len);

void modbus_init(UART_HandleTypeDef* huart)
{
 uart_transport_init(huart);
}

//@brief:modbus crc校验函数
uint16_t modbus_crc16(uint8_t* buf,uint16_t len)
{
	//#1.初始化
	uint16_t crc=0xFFFF;
	//#2.来一个字节异或一次
	for(uint16_t i=0;i<len;i++)
	{
		crc^=buf[i];
		//#3.对crc每一位做一次循环
		for(uint8_t j=0;j<8;j++)
		{
			//#4.看最低位
			if(crc&0x0001)
			{
			 //是1：右移+异或多项式
				crc=(crc>>1)^0xA001;
			}
			else
			{
			 //是0：右移
				crc>>=1;
			}
			
		}
	}
	return crc;
}



void modbus_poll(void)
{
	uart_transport_poll(rx_buf,&rx_len,&frame_ready);
	if(frame_ready==1)
	{
		//帧结束，可以开始进行接收了
		modbus_frame_process();
		frame_ready=0;//处理完清零接收完成标志
		rx_len=0;//对接收缓冲区长度清零
	}
	
}

static void modbus_frame_process(void)
{
	// --- 防御：最短合法帧是8字节（地址+功能码+2字节数据+CRC）---
    if (rx_len < 4) return;
	
	//#1.CRC校验，确认信息是否有误
	uint16_t crc_calc=modbus_crc16(rx_buf,(rx_len-2));//计算结果
	//从数据帧里面解析出crc
	uint16_t crc_rx;
	crc_rx=((uint16_t)rx_buf[rx_len-1]<<8)|(rx_buf[rx_len-2]);//注意小端模式
	
	if(crc_rx!=crc_calc)
	{
		//计算得到的crc码与接收到的不相等
		// CRC错了，说明数据在传输中损坏
    // Modbus规定：CRC错误不响应，直接丢弃
		//....
		return;//退出解析函数

	}
	
	//#2.通过CRC校验，开始过滤从机ID
	// rx_buf[0] 是目标从机地址
  // 地址0x00是广播地址，所有从机都接收但不响应
	uint8_t addr=rx_buf[0];
	if(addr!=SLAVE_ID&&addr!=0x00)
	{
		//不是发给自己的报文，直接退出即可
		return;
	}
	
	//#3.分拣功能码
	modbus_func_code func_code = (modbus_func_code)rx_buf[1];
	switch (func_code)
	{
		case FC_READ_HOLD:  	//0x03 读保持寄存器
			handle_read_hold();
		  break;
		
		case FC_READ_INPUT:		//0x04 读输入寄存器
			handle_read_input();
		break;
		case FC_WRITE_HOLD:		//0x06 写保持寄存器
			handle_write_single_hold();
		break;
		
		case FC_WRITE_COIL:		//0x05 写单个线圈
			handle_write_single_coil();
		break;
		
		default://未定义的无效功能码
			
			 // 广播地址不响应异常
		
		//不是广播模式从机上报异常功能码
		if (addr != 0x00)//判断不是广播
    send_exception(func_code, EX_ILLEGAL_FUNC);//发送异常码：功能码异常
		break;
	}
		
}

// ================================================================
// 功能码 0x03：读保持寄存器
// 请求格式：[addr][0x03][起始地址H][起始地址L][数量H][数量L][CRC]
// 响应格式：[addr][0x03][字节数][数据H][数据L]...[CRC]（CRC低位在前）
// ================================================================
static void handle_read_hold(void) 
{
	//解析起始地址和寄存器数量
	uint16_t start=(rx_buf[2]<<8)|rx_buf[3];
	uint16_t count=(rx_buf[4]<<8)|rx_buf[5];
	
	//越界处理：发送异常码 读保持寄存器的最大数量不能超过125
	if(count==0||count>MODBUS_READ_HOLD_MAX_COUNT||(count+start)>REG_HOLD_NUM)
	{
		send_exception(FC_READ_HOLD,EX_ILLEGAL_ADDR);//发送错误码
		return;//直接返回
	}
	
	//不越界的正常处理 发送响应报文
	
	uint8_t tx[256];//从机发送响应缓冲区
	tx[0]=SLAVE_ID;//从机地址
	tx[1]=FC_READ_HOLD;//功能码：读保持寄存器
	tx[2]=count*2;//字节数=保持寄存器数量*2（uint16_t类型）
	for(uint16_t i=0;i<count;i++)
	{
	  //打包寄存器数据：高位在前，低位在后
		tx[3+i*2]=hold_reg[start+i]>>8;//高位在前
		tx[3+i*2+1]=hold_reg[start+i]&0xFF;//低位在后
		
	}
	uint16_t tx_len=3+count*2;//除去CRC校验码的数据个数
	uint16_t crc=modbus_crc16(tx,tx_len);
	tx[tx_len]=crc&0xFF;//低位在前
	tx[tx_len+1]=crc>>8;//高位在后
	
	//发送响应报文
	send_response(tx,tx_len+2);
	
}
// ================================================================
// 功能码 0x04：读输入寄存器
// 格式和0x03完全相同，只是操作input_reg数组
// ================================================================
static void handle_read_input(void) 
{
	//#1.解析出起始地址和寄存器数量
	uint16_t start=(rx_buf[2]<<8)|rx_buf[3];//高位在前 低位在后
	uint16_t count=(rx_buf[4]<<8)|rx_buf[5];
	
	//#2.越界处理
	if(count==0||count>MODBUS_READ_INPUT_MAX_COUNT||start+count>REG_INPUT_NUM)
	{
		send_exception(FC_READ_INPUT,EX_ILLEGAL_ADDR);
		return;
	}
	
	//#3.读输入寄存器
	uint8_t tx[256];
	tx[0]=SLAVE_ID;//从机地址
	tx[1]=FC_READ_INPUT;//功能码：读输入寄存器
	tx[2]=count*2;//字节数
	for(uint16_t i=0;i<count;i++)
	{
		tx[3+i*2]=input_reg[start+i]>>8;//高位在前
		tx[3+i*2+1]=input_reg[start+i]&0xFF;//低位在后
		
	}
	
	//#4.计算校验码
	uint16_t tx_len=3+count*2;
	uint16_t crc=modbus_crc16(tx,tx_len);
	tx[tx_len]=crc&0xFF;//低位在前
	tx[tx_len+1]=crc>>8;//高位在后
	
	//#5.打包发送
	send_response(tx,(uint8_t)tx_len+2);
	
}

// ================================================================
// 功能码 0x06：写单个保持寄存器
// 请求格式：[addr][0x06][地址H][地址L][值H][值L][CRC]
// 响应格式：把请求原样echo回去（Modbus规定）
// ================================================================
static void handle_write_single_hold(void) 
{
		//#1.解析出起始地址和要写入的值
	uint16_t addr=(rx_buf[2]<<8)|rx_buf[3];
	uint16_t value=(rx_buf[4]<<8)|rx_buf[5];
	
	//#2.越界处理
	if(addr>=REG_HOLD_NUM)
	{
		send_exception(FC_WRITE_HOLD,EX_ILLEGAL_ADDR);
		return;
	}
	
	//#3.写保持寄存器
	hold_reg[addr]=value;
	
	//#4.echo回原报文
	send_response(rx_buf,rx_len);
	
}

// ================================================================
// 功能码 0x05：写单个线圈
// 请求格式：[addr][0x05][地址H][地址L][值H][值L][CRC]
// 值：0xFF00 = ON，0x0000 = OFF，其他值非法
// ================================================================
static void handle_write_single_coil(void)
{
	//#1.解析出起始地址和要写入的值
	uint16_t addr=(rx_buf[2]<<8)|rx_buf[3];
	uint16_t value=(rx_buf[4]<<8)|rx_buf[5];
	
	//#2.越界处理
	if(addr>=COIL_NUM)
	{
	 send_exception(FC_WRITE_COIL,EX_ILLEGAL_ADDR);
		return;
	}
	
	//#3.写线圈
	//modbus规定线圈只有两个有效值：0xFF00和0x0000
	if(value!=0xFF00&&value!=0x0000)
	{
		send_exception(FC_WRITE_COIL,EX_ILLEGAL_VALUE);
		return;
	}
	coil_reg[addr]=(value==0xFF00?1:0);
	
	//#4.echo响应原报文
	send_response(rx_buf,rx_len);
}

// ================================================================
// 发送异常响应
// 格式：[addr][功能码|0x80][异常码][CRC]
// 功能码最高位置1表示这是一个异常响应，是Modbus的规定
// ================================================================
static void send_exception(modbus_func_code func_code, modbus_ex_code ex_code)
{
	uint8_t tx[5];
	tx[0]=SLAVE_ID;
	tx[1]=func_code|0x80;
	tx[2]=ex_code;
	
	//#2.计算CRC
	uint16_t crc=modbus_crc16(tx,3);
	tx[3]=crc&0xFF;
	tx[4]=crc>>8;//高位在后
	
	send_response(tx,5);//发送
}

static void send_response(uint8_t *buf, uint8_t len)
{
	rs485_send(buf,len);
}
