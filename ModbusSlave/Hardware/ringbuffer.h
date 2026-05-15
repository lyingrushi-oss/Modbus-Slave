#ifndef __RINGBUFFER_H
#define __RINGBUFFER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef uint8_t rb_elem_t;

typedef struct ringbuffer_t{
    rb_elem_t* buf;//准备好的缓冲区地址
    uint32_t capacity;//缓冲区大小
    uint32_t head;//写指针
    uint32_t tail;//读指针
    uint32_t mask;//掩码，用于取模操作 数值等于capacity-1

}ringbuffer_t;

typedef enum rb_err_t{
    RB_OK,      //操作成功
    RB_EMPTY,   //缓冲区为空
    RB_FULL,    //缓冲区已满
    RB_INVALID  //非法值
}rb_err_t;

//初始化环形缓冲区
rb_err_t rb_init(ringbuffer_t* rb,rb_elem_t* buf,uint16_t capacity);

//判断缓冲区状态
bool rb_is_full(ringbuffer_t* rb);
bool rb_is_empty(ringbuffer_t* rb);

//查看缓冲区剩余的大小
uint32_t rb_free(ringbuffer_t* rb);

//查看缓冲区的元素个数
uint32_t rb_size(ringbuffer_t* rb);

//写入一个元素
rb_err_t rb_write_elem(ringbuffer_t* rb,rb_elem_t data);

//读一个元素
rb_err_t rb_read_elem(ringbuffer_t* rb,rb_elem_t* data);

//清空缓冲区
rb_err_t rb_flush(ringbuffer_t* rb);

//批量写入
uint32_t rb_write(ringbuffer_t* rb,rb_elem_t* scr,uint32_t len);

//批量读出
uint32_t rb_read(ringbuffer_t* rb,rb_elem_t* dst,uint32_t len);



#endif
