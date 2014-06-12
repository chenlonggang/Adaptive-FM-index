/*============================================
# Filename: WT_Handle.cpp
# Ver 1.0 2014-06-08
# Copyright (C) 2014 ChenLonggang (chenlonggang.love@163.com)
#
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 or later of the License.
#
# Description: 
=============================================*/
#include"WT_Handle.h"
#include"Huffman_WT.h"
#include"Balance_WT.h"
#include"Hutacker_WT.h"

WT_Handle::WT_Handle():fm(new ABS_FM()),u(){}

WT_Handle::WT_Handle(const char * filename,int block_size,int D,int shape,int speedlevel)
{
	if(block_size<=0 || shape<0 || shape >2)
	{
		cout<<"WT_Handle::WT_handle error parmater"<<endl;
		exit(0);
	}

	switch(shape)
	{
		case 0: fm =new Hutacker_FM(filename,block_size,D);break;
		case 1: fm =new Huffman_FM(filename,block_size,D);break;
		case 2: fm =new Balance_FM(filename,block_size,D);break;
		default: fm=new Hutacker_FM(filename,block_size,D);break;
	}
	fm->BuildTree(speedlevel);
}

WT_Handle::WT_Handle(const WT_Handle &h):fm(h.fm),u(h.u){}

WT_Handle & WT_Handle::WT_Handle:: operator =(const WT_Handle & h)
{
	if(u.reattach(h.u))
	{
		delete fm;
	}
	fm = h.fm;
	return * this;
}

WT_Handle::~WT_Handle()
{
	if(u.only())
		delete fm;
}


