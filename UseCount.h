/*============================================
# Filename: UseCount.h
# Ver 1.0 2014-06-08
# Copyright (C) 2014 ChenLonggang (chenlonggang.love@163.com)
#
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 or later of the License.
#
# Description: a class used for use-count.
  Ruminations on C++,Chapter 7.
=============================================*/
#ifndef USECOUNT_H
#define USECOUNT_H
class UseCount
{
	public:
		UseCount();
		UseCount(const UseCount &);
		~UseCount();
		bool only();
		bool reattach(const UseCount &);
	private:
		int *p;
		UseCount & operator = (const UseCount &);
};
#endif
