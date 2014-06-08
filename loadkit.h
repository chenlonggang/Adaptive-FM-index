/*============================================
# Filename: loadkit.h
# Ver 1.0 2014-06-08
# Copyright (C) 2014 ChenLonggang (chenlonggang.love@163.com)
#
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 or later of the License.
#
# Description: a simple toolkit for loading/reading data from a file
=============================================*/
#ifndef _LOADKIT
#define _LOADKIT
#include<stdlib.h>
#include<stdio.h>
#include"BaseClass.h"
#include<iostream>
using namespace std;
class loadkit
{
	public:
		loadkit(const char * file);
		~loadkit();
		i32 loadi64(i64 & value);
		i32 loadu64(u64 & value);
		i32 loadi32(i32 & value);
		i32 loadu32(u32 & value);
		i32 loadi16(i16 & value);
		i32 loadu16(u16 & value);
		i32 loadu8(u8 & value);

		i32 loadi64array(i64 * value,i32 len);
		i32 loadu64array(u64 * value,i32 len);
		i32 loadi32array(i32 * value,i32 len);
		i32 loadu32array(u32 * value,i32 len);
		i32 loadi16array(i16 * value,i32 len);
		i32 loadu16array(u16 * value,i32 len);
		i32 loadu8array(u8 * value,i32 len);

		void close();
	private:
		FILE * r;
};
#endif


