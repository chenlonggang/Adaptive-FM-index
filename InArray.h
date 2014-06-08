/*============================================
# Filename: InArray.h
# Ver 1.0 2014-06-08
# Copyright (C) 2014 ChenLonggang (chenlonggang.love@163.com)
#
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 or later of the License.
#
# Description:A simple int-vector,data_width bits each cell
=============================================*/
#ifndef _Inarray
#define _Inarray
#include"BaseClass.h"
#include"savekit.h"
#include"loadkit.h"
class InArray
{
public:
	InArray();
	InArray(i32 data_num,i32 data_width);
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
