/*============================================
# Filename: WT_Handle.h
# Ver 1.0 2014-06-08
# Copyright (C) 2014 ChenLonggang (chenlonggang.love@163.com)
#
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 or later of the License.
#
# Description: 
  A handle class for pointer fm(ABS_FM *),ABS_FM will
  actual points to a specific subclass.

=============================================*/
#ifndef WTHANDLE_H
#define WTHANDLE_H
#include"UseCount.h"
#include"ABS_WT.h"
class WT_Handle
{
	private:
		ABS_FM * fm;
		UseCount u;
	public:
		WT_Handle();
		WT_Handle(const char * filename,int block_size=256,int D=32,int shape = 1,int speedlevel=1);
		WT_Handle(const WT_Handle &);
		WT_Handle & operator = (const WT_Handle & );
		~WT_Handle();
		
		void Counting(const char * pattern,int &num) { fm->Counting(pattern,num); };
		int * Locating(const char * pattern,int &num){ return fm->Locating(pattern,num); };
		unsigned char *Extracting(int pos,int len){ return fm->Extracting(pos,len);};
		int Load(loadkit & s) { return fm->Load(s);};
		int Save(savekit & s){ return fm->Save(s);};

		int GetN(){ return fm->GetN();}
		int SizeInByte(){ return fm->SizeInByte();};
		int SizeInByte_count() { return fm->SizeInByte_count();};
};
#endif
