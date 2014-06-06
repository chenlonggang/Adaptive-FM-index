#include "InArray.h"
#include<string.h>
#include<stdlib.h>
#include<iostream>
using namespace std;


InArray::~InArray(void)
{
	delete [] data;
}
InArray::InArray()
{
}
//构造函数,表示有len个“整数”需要存储，每个整数的位宽为size。
InArray::InArray(i32 len, i32 size) 
{
	if(len<0||size<=0)
		cout<<"InArray构造参数输入错误"<<endl;
	else
	{
		this->datanum =len;
		this->datawidth =size;
	    i64 totlesize=datanum*datawidth;
		if(totlesize%32==0)
			totlesize=totlesize/32+1;
		else
			totlesize=(totlesize/32)+2;
		this->data =new u32[totlesize];
		memset(data,0,4*totlesize);
		mask=((1<<datawidth)-1);
	}
}

//数组中下标为index的位置设置值为value
void InArray::SetValue (i32 index, i32 v)
{

	if(index>datanum-1|| index<0)
	{
		cerr<<"InArray:index out of boundary"<<endl;
		exit(0) ;
	}
	else if(v>((1<<datawidth)-1))
	{
		cerr<<"InArray:value is out of boundary"<<endl;
		exit(0) ;
	}
	else
	{
		u64 value=v;
		u64 anchor=(index*datawidth)>>5;
		u64 temp1=data[anchor];
		u32 temp2=data[anchor+1];
		temp1=(temp1<<32)+temp2;
		i32 overloop=((anchor+2)<<5)-(index+1)*datawidth;
		value=(value<<overloop);
		temp1=temp1+value;
		data[anchor+1]=(temp1&(0xffffffff));
		data[anchor]=(temp1>>32)&(0xffffffff);

	}


}

i32 InArray::GetNum ()
{
	return datanum;
}
i32 InArray::GetMemorySize() 
{
	return (datanum*datawidth)/8;
}

		
i32 InArray::GetDataWidth() 
{
	return datawidth;
}

i32 InArray::GetValue(i32 index)
{
	if(index>datanum-1||index<0)
	{
		cerr<<"InArray:GetValue: index out of boundary"<<endl;
		exit(0);
	}
	
	u64 anchor=(index*datawidth)>>5;
	u64 temp1=data[anchor];
	u32 temp2=data[anchor+1];
	temp1=(temp1<<32)+temp2;
	i32 overloop=((anchor+2)<<5)-(index+1)*datawidth;
	return (temp1>>overloop)&mask;



}
i32 InArray::write(savekit & s)
{
	s.writei32(datanum);
	s.writei64(datawidth);
	i64 len=(datanum*datawidth);
	if(len%32==0)
		len=len/32+1;
	else
		len=len/32+2;

	s.writei32(len);
	s.writeu32array(data,len);
	return 1;
}
i32 InArray::load(loadkit & s)
{
	s.loadi32(datanum);
	s.loadi64(datawidth);
	i32 len=0;
	s.loadi32(len);
	data=new u32[len];
	s.loadu32array(data,len);
	mask=((1<<datawidth)-1);
	return 1;
}



