#include "ringbuffer.h"

//判断缓冲区大小是否是2的幂次
static inline bool is_power_of_two(uint32_t n)
{
    return (n>0)&&((n&(n-1))==0);
}

//初始化环形缓冲区
rb_err_t rb_init(ringbuffer_t* rb,rb_elem_t* buf,uint16_t capacity)
{
    //传入的环形缓冲区指针无效
    if(rb==NULL||buf==NULL)return RB_INVALID ;
    //缓冲区大小不是2的幂次
    if(is_power_of_two(capacity)==false)
    return RB_INVALID;

    rb->buf=buf;
    rb->capacity=capacity;
    rb->head=0;//写指针初始化
    rb->tail=0;//读指针初始化
    rb->mask=capacity-1;//取模位运算使用的掩码

    return RB_OK;//初始化完成

}

//判断缓冲区状态
bool rb_is_full(ringbuffer_t* rb)
{
    //读写指针只自增 不做其他运算
    //如果读写指针相差一整个capacity，说明缓冲区满了
    return rb->head-rb->tail==rb->capacity;
}
bool rb_is_empty(ringbuffer_t* rb)
{
    //读写指针相等
    return rb->head==rb->tail;
}

//查看缓冲区的元素个数
uint32_t rb_size(ringbuffer_t* rb)
{
    return rb->head-rb->tail;
}

//查看缓冲区剩余的大小
uint32_t rb_free(ringbuffer_t* rb)
{
    return rb->capacity-rb_size(rb);
}

//写入一个元素
rb_err_t rb_write_elem(ringbuffer_t* rb,rb_elem_t data)
{
    if(rb==NULL) return RB_INVALID;
    if(rb_is_full(rb)) return RB_FULL;

    //开始写入
    rb->buf[rb->head&rb->mask]=data;//记得取模操作

    //更新写指针的值
    rb->head++;

    return RB_OK;

}

//读一个元素
rb_err_t rb_read_elem(ringbuffer_t* rb,rb_elem_t* data)
{
    if(rb==NULL||data==NULL) return RB_INVALID;
    if(rb_is_empty(rb)) return RB_EMPTY;

    //读元素操作
    *data=rb->buf[rb->tail&rb->mask];
    //更新缓冲区的读指针
    rb->tail++;

    return RB_OK;

}

//清空缓冲区
rb_err_t rb_flush(ringbuffer_t* rb)
{
    if(rb==NULL) return RB_INVALID;

    rb->head=0;
    rb->tail=0;

    return RB_OK;

}

//批量写入
uint32_t rb_write(ringbuffer_t* rb,rb_elem_t* scr,uint32_t len)
{
    if(rb==NULL||scr==NULL) return 0;//无效指针，写入0个值

    //先判断剩余空间够不够用：够用写入，不够用写满缓冲区，写不下的不进行覆盖操作
    uint32_t space=rb_free(rb);
    if(len>space)
    {
        len=space;//space就是最大写入的长度
    }
    
    uint32_t head_pos=rb->head&rb->mask;
    uint32_t to_end=rb->capacity-head_pos;
    uint32_t first_len=(len<to_end)?len:to_end;
    uint32_t second_len=len-first_len;

    //写入第一段
    for(uint32_t i=0;i<first_len;i++)
    {
        rb->buf[head_pos+i]=scr[i];
    }
   
    //写入第二段
    if(second_len!=0)
    {
        for(uint32_t i=0;i<second_len;i++)
        {
            rb->buf[i]=scr[first_len+i];
        }
    }

    //写操作完成一行记得更新写指针的值
    rb->head+=len;

    return len;//返回写入的字节数
}

//批量读出
uint32_t rb_read(ringbuffer_t* rb,rb_elem_t* dst,uint32_t len)
{
    if(rb==NULL||dst==NULL) return 0;//一个也没读出来

    //获取当前缓冲区可以读的元素个数
    uint32_t size=rb_size(rb);

    if(len>size)
    {
        len=size;
    }

    //分两段读出
    uint32_t tail_pos=rb->tail&rb->mask;
    uint32_t to_end=rb->capacity-tail_pos;
    uint32_t first_len=(len<to_end)?len:to_end;
    uint32_t second_len=len-first_len;

    for(uint32_t i=0;i<first_len;i++)
    {
        dst[i]=rb->buf[tail_pos+i];
    }

    //第二段
    if(second_len!=0)
    {
        for(uint32_t i=0;i<second_len;i++)
        {
            dst[first_len+i]=rb->buf[i];
        }
    }

    //读操作结束，记得更新读指针的值
    rb->tail+=len;
    return len;//返回读出的元素个数
}