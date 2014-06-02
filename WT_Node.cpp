#include"WT_Node.h"
#include<math.h>
#include<iostream>
using namespace std;
#define lookuptable
inline int popcnt(unsigned long long int x)
{
	x = x -((x & 0xAAAAAAAAAAAAAAAA)>>1);
	x = (x & 0x3333333333333333)+((x>>2) & 0x3333333333333333);
	x =((x+(x>>4)) & 0x0F0F0F0F0F0F0F0F);
	return (x*0x0101010101010101)>>56;
}

int blog(int x)
{
	int ans = 0;
	while(x>0)
	{
		ans++;
		x=x>>1;
	}
	return ans;
}

WT_Node::WT_Node(unsigned long long int * bitbuff,int bit_len,int level,int block_size,unsigned char label,uchar ** tables)
{
	this->data = bitbuff;
	this->bitLen = bit_len;
	this->memorysize = 0;
	this->level = level;
	this->block_size = block_size;
	this->block_width = blog(block_size);
	this->super_block_size = 16*block_size;
	this->label = label;
	left=NULL;
	right=NULL;
	superblock =NULL;
	block=NULL;
	coding_style=NULL;
	R=NULL;
	
	if(data!=NULL)
	{
		this->zerostable = tables[0];
		this->R = tables[1];
	//	this->R1 = tables[1];
	//	this->R2 = tables[2];
	//	this->R3 = tables[3];
	//	this->R4 = tables[4];
		Coding();
		buff =NULL;
	}

}

int WT_Node::SizeInByte()
{

	int size = 0;
	if(data!=NULL)
	{
		//return bitLen/8;
		size+= superblock->GetMemorySize();
		size+= block->GetMemorySize();
		size+= coding_style->GetMemorySize();
		size+= memorysize;
	
	}
	return size;
}


void WT_Node::Coding()
{
	int u64Len =0;
	if(bitLen%64 == 0)
		u64Len = bitLen/64;
	else
		u64Len = bitLen/64+1;
	
	//用来缓存编码的地方
	u64 * temp = new u64[u64Len];
	memset(temp,0,u64Len*8);
	//用来表示编码进程，即编码到index bit了。
	int index = 0;
	//编码后的大小
	int space=0;
	//超快大小，单位bit
	int step1 = block_size*16;
	//块大小，单位bit
	int step2 = block_size;
	int block_width = blog(block_size);
	superblock = new InArray(2*(bitLen/step1)+2,blog(bitLen));
	block      = new InArray(2*(bitLen/step2)+2,blog(step1-step2));
	coding_style      = new InArray(bitLen/step2+1,3);

	//超快rank
	//int super_rank=0;
	//块rank
	int rank=0;
	//一个块内，已处理的bits数。
	int bits =0;
	//一个块内开始的第一个bit，这决定这编码方案，important.
	int firstbit;
	//某个块用gamma编码编码的大小，单位bit。
	int rl_g=0;
	int rl_f=0;
	//某个块用plain方案编码的大小，固定的.
//	int plain_size = block_size;
	//游程长度.
	int runs = 0;
	//标示这个游程是关于1的，还是0的
	int bit=0;
	//保存游程的临时数组，最多block_size个，此时0,1交替出现.
	int * runs_tmp = new int[block_size];
	int k=0;

	//temp的索引
	int index2=0;

	//前一个superblock的offset.
	//int pre_superblock_offset = 0;
	
	int pre_rank=0;
	int pre_space =0 ;
	superblock->SetValue(0,0);
	superblock->SetValue(1,0);
	block->SetValue(0,0);
	block->SetValue(1,0);
	while(index < bitLen)
	{
		if(index == bitLen)
			break;
		rl_g = 0;
		rl_f = 0;
		bits = 0;
		firstbit = 0;
		runs = 0;
		firstbit = GetBit(data,index);
		memset(runs_tmp,0,block_size*4);
		k=0;
 		runs=0;
		while(bits < block_size && index < bitLen)
		{

			runs = GetRuns(data,index,bit);
//			rl_gamma_size +=2*blog(runs)-1;
			bits = bits +runs;
			if(bit ==1)
				rank=rank+runs;
			runs_tmp[k] = runs;
			k++;
		}
		
		if(bits > block_size)
		{
			int step =0;
			index = index -(bits - block_size);
			step = block_size+runs-bits;
//			rl_gamma_size = rl_gamma_size-(2*blog(runs)-1)+2*blog(step)-1;
			if(bit ==1)//rank要修改
				rank = rank -runs+step;
			runs_tmp[k-1] = step;
		}


		for(int i=0;i<k;i++)
			rl_g = rl_g + 2*blog(runs_tmp[i])-1;
		rl_f = k * block_width;


/*		for(int i=0;i<max(0,k-2);i++)
			rl_g = rl_g + 2*blog(runs_tmp[i])-1;
		rl_f = max(0,k-2) * block_width;
*/


		int thred=20;
		int len = min(rl_g,min(rl_f,block_size-thred));

		
		if(k==1)
		{
			if(firstbit==0)
				coding_style->SetValue((index-1)/block_size,3);
			else
				coding_style->SetValue((index-1)/block_size,4);
			space = space +0;
		}

		else if(len == (block_size-thred) || index == bitLen)//plain
		{
			coding_style->SetValue((index-1)/block_size,2);
			int j=0;
			int num=0;
			if(index == bitLen)
			{
				space = space + bits;
				j = (index-bits)/64;
				num = bits%64?bits/64+1:bits/64;
			}
			else
			{
				space = space + block_size;
				j = (index - block_size)/64;
				num = block_size/64;
			}
			for(int kk=0;kk<num;kk++,j++)
				BitCopy(temp,index2,data[j]);
		}

		else//rl_gamma
		{
			if(firstbit == 0)
				coding_style->SetValue((index-1)/block_size,0);
			else
				coding_style->SetValue((index-1)/block_size,1);
			space =space + rl_g;
			for(int i=0;i<k;i++)
			{
				//cout<<runs_tmp[i]<<endl;
				Append_g(temp,index2,runs_tmp[i]);
			}
		}
	
		//打表顺序，superblock在前,block在后.
		if(index % step1 == 0)
		{
			pre_rank = rank;
			superblock ->SetValue(2*(index/step1),pre_rank);

			pre_space = space;
			superblock->SetValue(2*(index/step1)+1,pre_space);
		}
		if(index % step2 ==0)
		{
			block->SetValue(2*(index/step2),rank - pre_rank);
			block->SetValue(2*(index/step2)+1,space - pre_space);
		}
	}

	//释放runs_tmp
	delete [] runs_tmp;

	int u64_len_real = 0;
	if(space % 64==0)
		u64_len_real = space /64+1;
	else
		u64_len_real = space /64 +1+1;
	
	this->memorysize = u64_len_real*8;
	delete [] data;
	data = new u64[u64_len_real];

	memset(data,0,u64_len_real*8);
	memcpy(data,temp,(u64_len_real-1)*8);
	delete [] temp;

}



WT_Node::~WT_Node()
{
	if(left)
		delete left;
	if(right)
		delete right;
	delete [] data;
	delete superblock;
	delete block;
	delete coding_style;
}



int WT_Node::Rank(int pos,int & bit)
{
	if(pos < 0 || pos > bitLen)
	{
		return 0;
	}

	if((pos+1)%block_size!=0)
	{
		int block_anchor = (pos+1)/block_size;
		int superblock_anchor = (pos+1)/super_block_size;
		int type = coding_style->GetValue(pos/block_size);
		int rank1 = superblock->GetValue(superblock_anchor*2);
		int offset1 = superblock->GetValue(superblock_anchor*2+1);
		int rank_base = rank1+block->GetValue(block_anchor*2);
		int offset = offset1 + block->GetValue(block_anchor*2+1);
		buff = data +(offset>>6);
		int overloop = (pos+1)%block_size ;
		int index = (offset &0x3f);
		int rank = 0;
		switch(type)
		{
			case 0:rank = RL0_Rank(buff,index,overloop,bit);break;
			case 1:rank = RL1_Rank(buff,index,overloop,bit);break;
			case 2:rank = Plain_Rank(buff,index,overloop,bit);break;
			case 3:rank = 0;bit=0;break;
			case 4:rank = overloop;bit = 1;break;
		}
		return rank_base + rank;
	}
	else
	{
		int rank1   = superblock->GetValue(((pos+1)/super_block_size)*2);
		int offset1 = superblock->GetValue((pos/super_block_size)*2+1);
		int rank = rank1 + block->GetValue(((pos+1)/block_size)*2);
		int offset = offset1 + block->GetValue((pos/block_size)*2+1);

		int index = (offset&0x3f);
		buff = data+(offset>>6);
		int type = coding_style->GetValue(pos/block_size);
		int overloop = block_size;
		switch(type)
		{
			case 0:bit=RL0_Bit(buff,index,overloop);break;
			case 1:bit=RL1_Bit(buff,index,overloop);break;
			case 2:bit=Plain_Bit(buff,index,overloop);break;
			case 3:bit=0;break;
			case 4:bit=1;break;
		}
		return rank;

	}
}



int WT_Node::Rank(int pos)
{
	if (pos<0 || pos > bitLen)
	{
		cout<<"WT_Node::Rank  error paramater"<<endl;
		cout<<pos<<" "<<bitLen<<endl;
		exit(0);
	}

	int block_anchor = (pos+1)/block_size;
	int superblock_anchor = (pos+1)/super_block_size;
	int type = coding_style->GetValue(pos/block_size);

	int rank1 = superblock->GetValue(superblock_anchor<<1);
	int offset1 = superblock->GetValue((superblock_anchor<<1)+1);
	int rank_base = rank1 + block->GetValue(block_anchor<<1);
	int offset    = offset1 + block->GetValue((block_anchor<<1)+1);
	
	buff = data + (offset>>6);
	int overloop = (pos+1)%block_size ;
	int index = (offset & 0x3f);
	int rank = 0;
	if(overloop > 0)
	{
		switch(type)
		{
			case 0:rank = RL0_Rank(buff,index,overloop);break;
			case 1:rank = RL1_Rank(buff,index,overloop);break;
			case 2:rank = Plain_Rank(buff,index,overloop);break;
			case 3:rank=0;break;
			case 4:rank = overloop;break;
		}
	}
	return rank_base + rank;

}


int WT_Node::GetBit(u64 * data,int index)
{
	int anchor = index/64;
	int pos = 63-index%64;
	return ((data[anchor] &(0x01ull<<pos))>>pos);
}


//2014.5.8:16:53:这三段程序的性能相当.
int WT_Node::GetRuns(u64 * data,int &index,int &bit)
{
/*
    bit = GetBit(data,index);
	index=index+1;
	int runs =1;
	int currentbit = 0;
	while(runs < block_size)
	{
		currentbit = GetBit(data,index);
		if(currentbit == bit)
			runs++;
		else
			break;
		index++;
	}
	return runs;
*/


	bit = GetBit(data,index);
	index = index +1;
	int totle_runs = 1;
	int runs=0;
	
	while(totle_runs < block_size)
	{
		u16 x= GetBits(data,index,16);//index不联动
		if(bit==1)
			x=(~x);
		runs = zerostable[x];
		totle_runs +=runs;
		index+=runs;
		if(runs < 16)
			break;
	}
	return totle_runs;


/*	这两个while的i性能相当
	bit= GetBit(data,index);
	index = index +1;
	int totle_runs = 1;
	int runs=0;
	while(totle_runs < block_size)
	{
		u64 x = GetBits(data,index,64);
		if(bit == 1)
			x= (~x);
		runs = zerostable[(x>>48)&0xffff];
		totle_runs +=runs;
		index+=runs;
		if(runs < 16)
			break;
		runs = zerostable[(x>>32)&0xffff];
		totle_runs +=runs;
		index+=runs;
		if(runs < 16)
			break;
		runs = zerostable[(x>>16)&0xffff];
		totle_runs +=runs;
		index+=runs;
		if(runs < 16)
			break;
		runs = zerostable[x&0xffff];
		totle_runs +=runs;
		index+=runs;
		if(runs < 16)
			break;
	}

	return totle_runs;
*/


}


//gamma编码,index联动
void WT_Node::Append_g(u64 *temp,int &index,u32 value)
{
	u64 y=value;
	int zerosnum = blog(value)-1;
	index+=zerosnum;
	int onesnum = zerosnum+1;
	if(index%64 + onesnum < 65)
	{
		temp[index/64] = (temp[index/64] | (y<<(64-(index%64 + onesnum))));
	}
	else
	{
		int first = 64 - index%64;
		int second = onesnum - first;
		temp[index/64] = (temp[index/64] | (y>>second));
		temp [index/64 +1] = (temp[index/64+1] | (y<<(64-second)));
	}
	index = index + onesnum;
}
/*
//定长编码,index联动
void WT_Node::Append_f(u64 * temp,int & index,u32 value)
{
	u64 y=value;
	if(index % 64 + block_width < 65)
	{
		temp[index/64] = (temp[index/64] | (y<<(64-(index%64 + block_width))));
	}
	else
	{
		int first  = 64 - index%64;
		int second = block_width - first;
		temp[index/64] = (temp[index/64] | (y>>second));
		temp [index/64 +1] = (temp[index/64+1] | (y<<(64-second)));
	}
	index = index + block_width;
}
*/

void WT_Node::BitCopy(u64 * temp,int & index,u64 value)
{
	if(index%64!=0)
	{
		int first = 64 - index % 64;
		int second = 64 - first;
		temp[index/64] = (temp[index/64] | (value>>second));
		temp[index/64 + 1] = (temp[index/64+1] | (value<<first));
	}
	else
		temp[index/64]  = value;
	index = index +64;
}

//由压缩格式的0,1串buff的index位置开始, bits_num个bits内有几个1
int WT_Node::RL0_Rank(u64 * buff ,int &index,int bits_num)
{
/*
	int rank = 0;
	int bits = 0;
	while(true)
	{
		bits=GammaDecode(buff,index);
		bits_num = bits_num - bits;
		if(bits_num<=0)
			return rank;
		bits=GammaDecode(buff,index);
		bits_num = bits_num - bits;
		rank = rank + bits;
		if(bits_num<=0)
			return rank + bits_num;
	}
*/

#ifndef lookuptable	 
	int rank = 0;
	int already = 0;
	u64 x= GetBits(buff,index,64);
	int bits = 0;
	while(true)
	{
		bits =1+ (((int)zerostable[x>>48])<<1);

		already = already +bits;
		if(already > 64)
		{
			index = index + (already - bits);
			x = GetBits(buff,index,64);
			bits =1+ (((int)zerostable[x>>48])<<1);
			already = bits;
		}

		bits_num = bits_num - (x>>( 64 - bits));
		if(bits_num <= 0)
			return rank;
		x=(x<<bits);

		 bits =1+ (((int)zerostable[x>>48])<<1);
		 already = already +bits;
		 if(already > 64)
		 {
			 index = index + (already - bits);
			 x = GetBits(buff,index,64);
			 bits =1+ (((int)zerostable[x>>48])<<1);
			 already = bits;
		 }

		 bits_num = bits_num -(x>>( 64 - bits));
		 rank = rank + (x>>( 64- bits));
		 if(bits_num <= 0)
			 return rank + bits_num;
		 x = (x<<bits);
	}
#endif

#ifdef lookuptable	
	int rank = 0;
	int r=0;
	int already = 0;
	u64 x = GetBits(buff,index,64);
	int bits = 0;//通过查找表可以解码的被编码的0,1串的长度
	int step = 0;//
	int runs =0 ;//本次解码的runs数目
	int runs_num = 0;//累的runs数目.
	//cout<<bits_num<<endl;
	if(bits_num > 32)
	{
		while(true)
		{
			//runs = R4[x>>48]; //查找表可以解码的runs数目.
			runs = R[(x>>48)<<2];
			//cout<<runs<<endl;
			if(runs >0)
			{
				//cout<<runs<<endl;
				//step = R2[x>>48];
				step = R[1+((x>>48)<<2)];
				//cout<<step<<endl;
				already = already + step;
				if(already > 64)
				{
					index = index + (already -step);
					x = GetBits(buff,index,64);
					already = 0;
					continue;
				}
				//bits = R1[x>>48];
				bits = R[2+((x>>48)<<2)];
				//r = R3[x>>48];
				bits=(bits==0)?256:bits;
				r = R[3+((x>>48)<<2)];
				if((runs_num & 0x01) ==1)
					rank = rank + r;
				else
					rank = rank + ( bits-r);

				runs_num = runs_num + runs;
				bits_num = bits_num - bits;
				if(bits_num <=0)
					break;
				x = (x<<step);
			}
			else
			{
				step = 1 + (((int)zerostable[x>>48])<<1);
				already = already + step;
				if(already > 64)
				{
					index = index + (already - step);
					x = GetBits(buff,index,64);
					step = 1 + (((int)zerostable[x>>48])<<1);
					already  = step;
				}
				bits_num = bits_num - (x>>( 64 - step));
				if((runs_num &0x01) ==1)
					rank = rank + (x>>( 64 - step));
				
				//bits_num = bits_num - (x>>( 64 - step));
				if(bits_num <=0)
				{
					if((runs_num &0x01)==1)
						return rank + bits_num;
					else
						return rank;
				}
				runs_num++;
				x = (x<<step);
			}
		}
	
	}
	index = index + (already - step);
	bits_num = bits_num + bits;
	runs_num = runs_num - runs;
	if((runs_num &0x01) ==1)
		rank = rank - r;
	else
		rank = rank - (bits -r );
	already = 0;
	x = GetBits(buff,index,64);
	while(true)
	{
		//cout<<"fuck "<<endl;
		step = 1+ (((int)zerostable[x>>48])<<1);
		already  = already + step;

		if(already > 64)
		{
			index = index + (already - step);
			x =GetBits(buff,index,64);
			step  = 1+ (((int)zerostable[x>>48])<<1);
			already = step;
		}
		bits_num = bits_num - (x>>( 64 - step));
		
		if((runs_num &0x01) ==1)
			rank = rank + (x>>( 64 - step));
		
	//	bits_num = bits_num - (x>>( 64 - step));
		if(bits_num <= 0)
		{
			if((runs_num&0x01)==1)
				return rank + bits_num;
			else
				return rank;
		}
		runs_num++;
		x=(x<<step);
	}
#endif
}


int WT_Node::RL0_Rank(u64 * buff,int & index,int bits_num,int &bit)
{
	int rank = 0;
	int bit_count = 0;
	int bits = 0;
	while(true)
	{
		 bits=GammaDecode(buff,index);
		 bit_count = bit_count + bits;
		 if(bit_count >= bits_num)
		 {
			 bit = 0;
			 return rank;
		 }
		 bits=GammaDecode(buff,index);
		 bit_count = bit_count + bits;
		 rank = rank + bits;
		 if(bit_count >= bits_num)
		 {
			 bit = 1;
			 return rank - (bit_count-bits_num);
		 }
	}
}

int WT_Node::RL0_Bit(u64 * buff,int & index,int bits_num)
{
	//int rank = 0;
	int bit_count =0;
	int bits = 0;
	while(true)
	{
		bits=GammaDecode(buff,index);
		bit_count = bit_count + bits;
		if(bit_count >= bits_num)
			return  0;
		bits=GammaDecode(buff,index);
		bit_count = bit_count + bits;
		if(bit_count >= bits_num)
			return 1;
	}
}


//
int WT_Node::RL1_Rank(u64 * buff,int &index,int bits_num)
{
/*
	int rank = 0;
	int bit_count = 0 ;
	int  bits = 0;
	while(true)
	{
		bits = GammaDecode(buff,index);
		bit_count = bit_count + bits;
		rank = rank + bits;
		if(bit_count >= bits_num)
			return rank - (bit_count-bits_num);
		bits = GammaDecode(buff,index);
		bit_count = bit_count + bits;
		if(bit_count >= bits_num)
			return rank;
	}
*/
#ifndef lookuptable
	int rank = 0;
	int already = 0;
	u64 x = GetBits(buff,index,64);
	int bits = 0;
	while(true)
	{
		bits =1+ (((int)zerostable[x>>48])<<1);
		already = already +bits;
		if(already > 64)
		{
			index = index + (already - bits);
			x = GetBits(buff,index,64);
			bits =1+ (((int)zerostable[x>>48])<<1);
			already = bits;
		}
		bits_num = bits_num -(x>>( 64 - bits));
		rank = rank + (x>>( 64- bits));
		if(bits_num <= 0)
			return rank + bits_num;
		x = (x<<bits);

		bits =1+ (((int)zerostable[x>>48])<<1);
		already = already +bits;
		if(already > 64)
		{
			index = index + (already - bits);
			x = GetBits(buff,index,64);
			bits =1+ (((int)zerostable[x>>48])<<1);
			already = bits;
		}
		bits_num = bits_num - (x>>( 64 - bits));
		if(bits_num <= 0)
			return rank;
		x=(x<<bits);
	}

#endif

#ifdef lookuptable
	int rank = 0;
	int r=0;
	int already = 0;
	u64 x = GetBits(buff,index,64);
	int bits = 0;//通过查找表可以解码的被编码的0,1串的长度
	int step = 0;//
	int runs =0 ;//本次解码的runs数目
	int runs_num = 0;//累计的runs数目
	if(bits_num > 32)
	{
	while(true)
	{
		//runs = R4[x>>48]; //查找表可以解码的runs数目.
		runs = R[(x>>48)<<2];
		if(runs >0)
		{
			//step = R2[x>>48];
			step = R[1+((x>>48)<<2)];
			already = already + step;
			if(already > 64)
			{
				index = index + (already -step);
				x = GetBits(buff,index,64);
				already = 0;
				continue;
			}
			//bits = R1[x>>48];
			bits = R[2+((x>>48)<<2)];
			//r = R3[x>>48];
			bits=(bits==0)?256:bits;
			r = R[3+((x>>48)<<2)];
			if((runs_num & 0x01) ==0)
				rank = rank + r;
			else
				rank = rank + ( bits-r);
			bits_num = bits_num - bits;
			runs_num = runs_num + runs;
		//	bits_num = bits_num - bits;
			if(bits_num <=0)
				break;
			x = (x<<step);
		}
		else
		{
			step = 1 + (((int)zerostable[x>>48])<<1);
			already = already + step;
			if(already > 64)
			{
				index = index + (already - step);
				x = GetBits(buff,index,64);
				step = 1 + (((int)zerostable[x>>48])<<1);
				already = step;
			}
			bits_num = bits_num - (x>>( 64 - step));
			if((runs_num &0x01) ==0)
				rank = rank + (x>>( 64 - step));
			if(bits_num <=0)
			{
				if((runs_num &0x01)==0)
					return rank + bits_num;
				else
					return rank;
			}
			runs_num++;
			x = (x<<step);

		}
	}
	}
	index = index + (already - step);
	bits_num = bits_num + bits;
	runs_num = runs_num - runs;
	if((runs_num &0x01) ==0)
		rank = rank - r;
	else
		rank = rank - (bits -r );
	already = 0;
	x = GetBits(buff,index,64);
	while(true)
	{
		//cout<<"fuck "<<endl;
		step = 1+ (((int)zerostable[x>>48])<<1);
		already  = already + step;

		if(already > 64)
		{
			index = index + (already - step);
			x =GetBits(buff,index,64);
			step  = 1+ (((int)zerostable[x>>48])<<1);
			already = step;
		}
		bits_num = bits_num - (x>>( 64 - step));
		
		if((runs_num &0x01) ==0)
			rank = rank + (x>>( 64 - step));
		
		if(bits_num <= 0)
		{
			if((runs_num&0x01)==0)
				return rank + bits_num;
			else
				return rank;
		}
		runs_num++;
		x=(x<<step);
	}
#endif 
}

int WT_Node::RL1_Rank(u64 * buff,int &index,int bits_num,int & bit)
{
	int rank = 0;
	int bit_count = 0 ;
	int  bits = 0;
	while(true)
	{
		 bits = GammaDecode(buff,index);
		 bit_count = bit_count + bits;
		 rank = rank + bits;
		 if(bit_count >= bits_num)
		 {
			 bit = 1;
			 return rank - (bit_count-bits_num);
		 }
		 bits = GammaDecode(buff,index);
		 bit_count = bit_count + bits;
		 if(bit_count >= bits_num)
		 {
			 bit = 0;
			 return rank;
		 }
	}
}

int WT_Node::RL1_Bit(u64 * buff,int &index,int bits_num)
{
	//int rank = 0;
	int bit_count = 0 ;
	int  bits = 0;
	while(true)
	{
		 bits = GammaDecode(buff,index);
		 bit_count = bit_count + bits;
		 if(bit_count >= bits_num)
			 return 1;
		 bits = GammaDecode(buff,index);
		 bit_count = bit_count + bits;
		 if(bit_count >= bits_num)
			 return 0;
	}
}

int WT_Node::Plain_Rank(u64 * buff,int &index,int bits_num,int &bit)
{
	if(index %64 + bits_num < 65)
	{
		u64 temp = buff[index/64];
		temp = (temp >>(64-(index%64 + bits_num)));
		if(bits_num!=64)
			temp = temp &((0x01ull<<bits_num)-1);

		index = index + bits_num -1;
		bit = (buff[index/64] & (0x01ull<<(63-index%64))) >>(63-index%64);
		return popcnt(temp);
	}
	int rank = 0;
	int head = 64 - index%64;
	u64 temp = buff[index/64];
	if(head!=64)
		temp = temp &((0x01ull<<head)-1);
	rank = rank + popcnt(temp);
	bits_num = bits_num - head;
	int times = bits_num/64;
	int i=0;
	for(i=0;i<times;i++)
		rank = rank + popcnt(buff[i+index/64+1]);
	if(bits_num%64!=0)
		rank = rank + popcnt((buff[i+index/64+1] >> (64-bits_num%64)));
	index  = index + head + bits_num - 1;
	bit = ((buff[index/64] & (0x01ull<<(63-index%64))) >>(63-index%64));
	return rank;
/*
	if((index&0x3f ) + bits_num < 65)
	{
		u64 temp = buff[index>>6];
		temp = (temp >>(64-((index&0x3f) + bits_num)));
		if(bits_num!=64)
			temp = temp &((0x01ull<<bits_num)-1);

		index = index + bits_num -1;
		bit = (buff[index/64] & (0x01ull<<(63-(index&0x3f)))) >>(63-(index&0x3f));
		return popcnt(temp);
	}
	int rank = 0;
	int head = 64 - (index&0x3f);
	u64 temp = buff[index>>6];
	if(head!=64)
		temp = temp &((0x01ull<<head)-1);
	rank = rank + popcnt(temp);
	bits_num = bits_num - head;
	int times = (bits_num>>6);
	int i=0;
	for(i=0;i<times;i++)
		rank = rank + popcnt(buff[i+(index>>6)+1]);
	if(bits_num%64!=0)
		rank = rank + popcnt((buff[i+(index>>6)+1] >> (64-(bits_num&0x3f))));
	index  = index + head + bits_num - 1;
	bit = ((buff[index>>6] & (0x01ull<<(63-(index&0x3f)))) >>(63-(index&0x3f)));
	return rank;
*/
}

int WT_Node::Plain_Bit(u64 * buff,int &index,int bits_num)
{
	index = index + bits_num - 1;
	return (buff[index/64] & (0x01ull<<(63 - (index&0x3f)))) >>(63 - (index&0x3f));

}



int WT_Node::Plain_Rank(u64 * buff,int &index,int bits_num)
{
	if(index %64 + bits_num < 65)
	{
		u64 temp = buff[index/64];
		temp = (temp >>(64-(index%64 + bits_num)));
		if(bits_num!=64)
			temp = temp &((0x01ull<<bits_num)-1);
		return popcnt(temp);
	}
	int rank = 0;
	int head = 64 - index%64;
	u64 temp = buff[index/64];
	if(head!=64)
		temp = (temp &((0x01ull<<head)-1));
	rank = rank + popcnt(temp);
	bits_num = bits_num - head;
	int times = bits_num/64;
	int i=0;
	for(i=0;i<times;i++)
		rank = rank + popcnt(buff[i+index/64+1]);
	if(bits_num%64!=0)
		rank = rank + popcnt((buff[i+index/64+1] >> (64-bits_num%64)));
	return rank;
/*
	if((index&0x3f) + bits_num < 65)
	{
		u64 temp = buff[index>>6];
		temp = (temp >>(64-((index&0x3f) + bits_num)));
		if(bits_num!=64)
			temp = temp &((0x01ull<<bits_num)-1);
		return popcnt(temp);
	}
	int rank = 0;
	int head = 64 - (index&0x3f);
	u64 temp = buff[index>>6];
	if(head!=64)
		temp = (temp &((0x01ull<<head)-1));
	rank = rank + popcnt(temp);
	bits_num = bits_num - head;
	int times = (bits_num>>6);
	int i=0;
	for(i=0;i<times;i++)
		rank = rank + popcnt(buff[i+(index>>6)+1]);
	//int left = bits_num % 64;
	if(bits_num%64!=0)
		rank = rank + popcnt((buff[i+(index>>6)+1] >> (64-(bits_num & 0x3f))));
		//rank = rank + popcnt(buff[i+index/64+1]& (~((0x01ull<<(64-bits_num%64))-1)));
	return rank;
*/
}
/*
int WT_Node::Fixed0_Rank(u64 * buff ,int &index,int bits_num)
{

	return 0;
}

int WT_Node::Fixed0_Rank(u64 * buff,int & index,int bits_num,int &bit)
{

	return 0;
}

int WT_Node::Fixed0_Bit(u64 * buff,int & index,int bits_num)
{
	return 0;
}

int WT_Node::Fixed1_Rank(u64 * buff ,int &index,int bits_num)
{

	return 0;
}
int WT_Node::Fixed1_Rank(u64 * buff,int & index,int bits_num,int &bit)
{
	return 0;
}
int WT_Node::Fixed1_Bit(u64 * buff,int & index,int bits_num)
{
	return 0;
}



int WT_Node::FixedDecode(u64 * buff,int & index)
{
	u32 x = GetBits(buff,index,block_width);
	index = index + block_width;
	return x;
}
*/
int WT_Node::GammaDecode(u64 * buff,int & index)
{
	u32 x = GetBits(buff,index,32);
	int runs = zerostable[x>>16];
	//int runs = 16 - blog(x>>16);
	int bits = (runs<<1)+1;
	index = index + bits;
	return x>>(32-bits);
}


//从buff的index位置开始,读取bits位数据,返回.
u64 WT_Node::GetBits(u64 * buff,int &index,int bits)
{

	if((index & 0x3f) + bits < 65)
		return (buff[index>>6] >>( 64 -((index&0x3f) + bits))) & ((0x01ull<<bits)- 1);

	int first = 64 - (index &0x3f);
	int second = bits - first;
	u64 high = (buff[index>>6] & ((0x01ull<<first)-1)) << second;
	return high + (buff[(index>>6)+1]>>(64-second));


}

int WT_Node::GetZerosRuns(u64 * buff,int &index)
{
/*	int runs = 0;
	int bit=0;
	while(true)
	{
		bit=GetBit(buff,index);
//		cout<<bit<<" ";
		if(bit==0)
			runs++;
		else
			break;
		index++;
	}
//	cout<<endl;
	return runs;
*/
	
	u32 x = GetBits(buff,index,16);
	int runs = zerostable[x];
	index = index + runs;
	return runs;

}


void WT_Node::Left(WT_Node * left)
{
     this->left = left;
}


void WT_Node::Right(WT_Node * right)
{
     this->right = right;
}
 
unsigned char WT_Node::Label()
{
    return label;
}


int WT_Node::Load(loadkit & s)
{
	s.loadi32(level);
	s.loadu8(label);
	s.loadi32(bitLen);
	s.loadi32(block_size);
	block_width = blog(block_size);
	s.loadi32(super_block_size);
	s.loadi32(memorysize);
	this->data=NULL;
	this->superblock=NULL;
	this->block=NULL;
	this->coding_style=NULL;
	if(memorysize!=0)
	{
		this->data = new u64[memorysize/8];
		s.loadu64array(data,memorysize/8);
	
		superblock = new InArray();
		superblock->load(s);
		
		block = new InArray();
		block->load(s);
		
		coding_style = new InArray();
		coding_style->load(s);
	}
    return 0;
}


int WT_Node::Save(savekit & s)
{
	s.writei32(level);
	s.writeu8(label);
	s.writei32(bitLen);
	s.writei32(block_size);
	s.writei32(super_block_size);
	s.writei32(memorysize);
	if(memorysize!=0)
	{
		s.writeu64array(data,memorysize/8);
	
		superblock->write(s);
		block->write(s);
		coding_style->write(s);
	}
	return 0;
}


