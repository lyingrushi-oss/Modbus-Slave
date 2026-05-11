
#include "uart_transport.h"
#include "register.h"

#define FRAME_TIMEOUT 4//定义帧超时时间：4ms
#define RX_BUF_LEN 256




static UART_HandleTypeDef* _huart;//uart使用的串口
static DMA_HandleTypeDef* _dma;//使用的dma句柄

static uint8_t _rx_buf[RX_BUF_LEN];//接收缓冲区
static volatile uint16_t _rx_len=0;//接收缓冲区索引 数组下标
//static volatile uint8_t _rx_byte=0;//单字节接收缓冲区
//static volatile uint32_t _last_rx_time=0;

static void rs485_tx_en(void);//使能发送
static void rs485_rx_en(void);//使能接收

void uart_transport_init(UART_HandleTypeDef* huart,DMA_HandleTypeDef* dma)
{
  _huart=huart;
	_dma=dma;
	//开启接收中断
	//HAL_UART_Receive_IT(_huart,(uint8_t*)&_rx_byte,1);//开启串口接收中断
	//使用DMA+IDLE中断
	HAL_UARTEx_ReceiveToIdle_DMA(_huart,_rx_buf,RX_BUF_LEN);//开启DMA接收&空闲中断
	//关闭DMA过半中断
	__HAL_DMA_DISABLE_IT(_dma,DMA_IT_HT);
}


#if 0
//UART接收完成中断处理函数
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart==_huart)
	{
		_rx_buf[_rx_len++]=_rx_byte;
		
		//记录上一次接收的时间戳
		_last_rx_time=HAL_GetTick();
		
		// ★★★ 加这一行：收到一个字节，就发回去 ★★★
       // HAL_UART_Transmit(_huart, (uint8_t*)"R", 1, 100);  // 收到就打印字母 "R"
		
		//防止数组溢出
		if(_rx_len>=RX_BUF_LEN) _rx_len=0;
		
		//继续接收下一个字节
		HAL_UART_Receive_IT(_huart,(uint8_t*)&_rx_byte,1);
		
	}
		
}
#endif

//使用空闲中断进行接收
//@参数：size：接收到的字节数
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
 if(huart==_huart)
 {
	 //DMA已经把数据搬运到缓冲区了，无需重复搬运了
//  for(uint8_t i=0;i<size;i++)
//	 {
//		//将数据拷贝到
//	 }
	 
	 //在这里发送一个空闲中断确认接收结束就可以了
	 _rx_len=Size;
	 //测试代码，回环发送
	 //HAL_UART_Transmit(_huart,_rx_buf,_rx_len,1000);
	 //开启下一次DMA接收
	 HAL_UARTEx_ReceiveToIdle_DMA(_huart,_rx_buf,RX_BUF_LEN);
	 //关闭DMA过半中断
	__HAL_DMA_DISABLE_IT(_dma,DMA_IT_HT);
 }
}

//esp32重启电平不稳定产生脉冲噪声锁死stm32dma,会导致stm32必须要重启才能正常收发
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart == _huart) {
        __HAL_UART_CLEAR_OREFLAG(_huart);
        __HAL_UART_CLEAR_NEFLAG(_huart);
        __HAL_UART_CLEAR_FEFLAG(_huart);
        HAL_UARTEx_ReceiveToIdle_DMA(_huart, _rx_buf, RX_BUF_LEN);
        __HAL_DMA_DISABLE_IT(_dma, DMA_IT_HT);
    }
}

//传输层轮询
void uart_transport_poll(uint8_t* buf,uint8_t* buf_len,uint8_t* flag_ready)
{
	*flag_ready=0;
	//如果缓冲区有数据，且距离上一次发送的时间超过3.5个字符就确认帧结束
	if(_rx_len!=0)
	{
		//判定为帧结束 将缓冲区数据拷贝过来
		for(uint8_t i=0;i<_rx_len;i++)
		{
			buf[i]=_rx_buf[i];
		}
		*buf_len=_rx_len;
		*flag_ready=1;
		_rx_len=0;//对缓冲区长度进行清零，下次接收直接覆盖
	}
	
}

//发送函数
void rs485_send(uint8_t* buf,uint8_t len)
{
	rs485_tx_en();//使能发送
	HAL_UART_Transmit(_huart,buf,len,1000);
	rs485_rx_en();//改回接收模式

}

static void rs485_tx_en(void)
{
}

static void rs485_rx_en(void)
{


}
