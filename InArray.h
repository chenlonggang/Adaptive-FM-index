#ifndef _Inarray
#define _Inarray
#include"BaseClass.h"
#include"savekit.h"
#include"loadkit.h"
//本类表示一个有len个整数组成的数组，每个数组元素的大小为size位。若存入数组中的值的范围为0~len-1，则
//size 的大小为log(len)，若数据范围为1~len，则size的大小为log(len)+1。本类默认的数组下标从0开始。
class InArray
{
public:
	InArray();
	InArray(i32 len,i32 size);
	~InArray(void);
	i32 GetValue(i32 index);
	void SetValue(i32 index,i32 value);
	i32 GetNum();
	i32 GetDataWidth();
	i32 GetMemorySize();
    i32 GetValue2(i32 index);
	i32 write(savekit & s);
	i32 load(loadkit & s);
private:
	u32 * data;
    i32 datanum;
	i64 datawidth;
	u32 mask;
};
#endif
