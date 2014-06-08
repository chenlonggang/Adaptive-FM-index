#include"BitMap.h"
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

BitMap::BitMap(unsigned long long int * bitbuff,int bit_len,int level,int block_size,unsigned char label,uchar ** tables)
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
	Z=NULL;
	
	if(data!=NULL)
	{
		this->Z = tables[0];
		this->R = tables[1];
		Coding();
		buff =NULL;
	}

}

int BitMap::SizeInByte()
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


void BitMap::Coding()
{
	int u64Len =0;
	if(bitLen%64 == 0)
		u64Len = bitLen/64;
	else
		u64Len = bitLen/64+1;
	u64 * temp = new u64[u64Len];
	memset(temp,0,u64Len*8);
	
	int index = 0;
	int step1 = block_size*16;
	int step2 = block_size;
	//int block_width = blog(block_size);
	superblock = new InArray(2*(bitLen/step1)+2,blog(bitLen));
	block      = new InArray(2*(bitLen/step2)+2,blog(step1-step2));
	coding_style      = new InArray(bitLen/step2+1,3);

	int rank=0;
	int space=0;
	int bits =0;
	int firstbit;
	int rl_g=0;
	int runs = 0;
	int bit=0;
	int * runs_tmp = new int[block_size];
	int k=0;
	int index2=0;
	
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
			if(bit ==1)
				rank = rank -runs+step;
			runs_tmp[k-1] = step;
		}

		for(int i=0;i<k;i++)
			rl_g = rl_g + 2*blog(runs_tmp[i])-1;

		int thred=20;
		int len = min(rl_g,block_size-thred);

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

BitMap::~BitMap()
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


int BitMap::Rank(int pos,int & bit)
{
	if(pos < 0 || pos > bitLen)
	{
		return 0;
	}

	if((pos+1)%block_size!=0)
	{
		int block_anchor = (pos+1)/block_size;
		int superblock_anchor = (pos+1)/super_block_size;
		int type =  coding_style->GetValue(pos/block_size);
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
			case 0:rank = RL_Rank(buff,index,overloop,0,bit);break;
			case 1:rank = RL_Rank(buff,index,overloop,1,bit);break;
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


void BitMap::Rank(int pos_left,int pos_right,int &rank_left,int &rank_right)
{
/*	
	rank_left= Rank(pos_left);
	rank_right=Rank(pos_right);
*/
	
	if(pos_left<0 || pos_right <0 || pos_left > bitLen || pos_right > bitLen)
	{
		cerr<<"BitMap::Rank(int,int,int&,int&) error parmater"<<endl;
		exit(0);
	}
	int block_anchor = (pos_left+1)/block_size;
	if(block_anchor==(pos_right+1)/block_size)
	{
		int superblock_anchor=(pos_left+1)/super_block_size;
		int rank_base = superblock->GetValue(superblock_anchor<<1)+block->GetValue(block_anchor<<1);
		int offset = superblock->GetValue((superblock_anchor<<1)+1)+block->GetValue((block_anchor<<1)+1);
		int type = coding_style->GetValue(block_anchor);
		int overloop_left = (pos_left+1)%block_size;
		int overloop_right= (pos_right+1)%block_size;
		buff = data+ (offset>>6);
		int index = (offset&0x3f);
		rank_left = rank_right =rank_base;
		if(overloop_left!=0)
		{
			switch(type)
			{
				case 0:RL_Rank(buff,index,overloop_left,overloop_right,rank_left,rank_right,0);break;
				case 1:RL_Rank(buff,index,overloop_left,overloop_right,rank_left,rank_right,1);break;
				case 2:Plain_Rank(buff,index,overloop_left,overloop_right,rank_left,rank_right);break;
				case 3:break;
				case 4:rank_left +=overloop_left;rank_right += overloop_right;break;
			}
			return ;
		}
		switch(type)
		{   
			case 0:rank_right += RL_Rank(buff,index,overloop_right,0);break;
			case 1:rank_right += RL_Rank(buff,index,overloop_right,1);break;
			case 2:rank_right += Plain_Rank(buff,index,overloop_right);break;
			case 3:break;
			case 4:rank_right += overloop_right;break;
		}
	}
	else
	{
		rank_left= Rank(pos_left);
		rank_right=Rank(pos_right);
	}

}


int BitMap::Rank(int pos)
{
	if (pos<0 || pos > bitLen)
	{
		cerr<<"BitMap::Rank  error paramater"<<endl;
		cerr<<pos<<" "<<bitLen<<endl;
		exit(0);
	}

	int block_anchor = (pos+1)/block_size;
	int superblock_anchor  = ((pos+1)/super_block_size);
	int type = coding_style->GetValue(block_anchor);
	
	int rank1 = superblock->GetValue(superblock_anchor<<1);
	int offset1 = superblock->GetValue((superblock_anchor<<1)+1);
	int rank_base = rank1 + block->GetValue(block_anchor<<1);
	int offset    = offset1 + block->GetValue((block_anchor<<1)+1);
	
	buff = data + (offset>>6);
	int overloop = (pos+1)%block_size ;
	int index = (offset & 0x3f);
	if(overloop > 0)
	{
		switch(type)
		{
			case 0:rank_base += RL_Rank(buff,index,overloop,0);break;
			case 1:rank_base += RL_Rank(buff,index,overloop,1);break;
			case 2:rank_base += Plain_Rank(buff,index,overloop);break;
			case 3:break;
			case 4:rank_base += overloop;break;
		}
	}
	return rank_base;

}


int BitMap::GetBit(u64 * data,int index)
{
	int anchor = index/64;
	int pos = 63-index%64;
	return ((data[anchor] &(0x01ull<<pos))>>pos);
}


//2014.5.8:16:53:这三段程序的性能相当.
int BitMap::GetRuns(u64 * data,int &index,int &bit)
{


	bit = GetBit(data,index);
	index = index +1;
	int totle_runs = 1;
	int runs=0;
	
	while(totle_runs < block_size)
	{
		u16 x= GetBits(data,index,16);//index不联动
		if(bit==1)
			x=(~x);
		runs = Zeros(x);
		totle_runs +=runs;
		index+=runs;
		if(runs < 16)
			break;
	}
	return totle_runs;
}


//gamma编码,index联动
void BitMap::Append_g(u64 *temp,int &index,u32 value)
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



void BitMap::BitCopy(u64 * temp,int & index,u64 value)
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

int BitMap::RL0_Rank(u64 *buff,int &index,int bits_num)
{
	int bit=0;
	return RL0_Rank(buff,index,bits_num,bit);
}

int BitMap::RL0_Rank(u64 * buff,int & index,int bits_num,int &bit)
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

int BitMap::RL0_Bit(u64 * buff,int & index,int bits_num)
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

int BitMap::RL1_Rank(u64 * buff,int &index,int bits_num)
{
	int bit =0;
	return RL1_Rank(buff,index,bits_num,bit);
}

int BitMap::RL1_Rank(u64 * buff,int &index,int bits_num,int & bit)
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

int BitMap::RL1_Bit(u64 * buff,int &index,int bits_num)
{
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


//palin类型的比例较低，所以两种方式的区别不大.
void BitMap::Plain_Rank(u64 *buff,int &index,int bits_left,int bits_right,int & rank_left,int &rank_right)
{
	
	int rank = Plain_Rank(buff,index,bits_left);
	rank_left += rank;
	index++;
	buff = buff +(index>>6);
	index = (index &0x3f);
	rank_right+= (Plain_Rank(buff,index,bits_right-bits_left)+rank);

/*	
   rank_left += Plain_Rank(buff,index,bits_left);
   index = index +1 - bits_left;
   rank_right+= Plain_Rank(buff,index,bits_right);
  */

}


int BitMap::Plain_Rank(u64 * buff,int &index,int bits_num,int &bit)
{
	if((index &0x3f) + bits_num < 65)
	{
		u64 temp = (buff[index>>6]<<(index&0x3f))>>(64-bits_num);
		index = index + bits_num -1;
		bit=(buff[index>>6]>>(63-(index&0x3f)))&0x01;
		return popcnt(temp);
	}
	int rank = 0;
	int head = 64 - (index&0x3f);
	u64 temp = (buff[index>>6]<<(index&0x3f));
	rank = rank + popcnt(temp);
	bits_num = bits_num - head;
	
	int times = bits_num>>6;
	int i=0;
	for(i=0;i<times;i++)
	rank = rank + popcnt(buff[i+(index>>6)+1]);
	
	if((bits_num&0x3f)!=0)
		rank = rank + popcnt((buff[i+(index>>6)+1] >> (64-(bits_num&0x3f))));
	
	index = index + head + bits_num - 1;
	bit=(buff[index>>6]>>(63-(index&0x3f)))&0x01;
	return rank;
}

int BitMap::Plain_Bit(u64 * buff,int &index,int bits_num)
{
	index = index + bits_num - 1;
	return (buff[index>>6]>>(63-(index&0x3f)))&0x01;
}



int BitMap::Plain_Rank(u64 * buff,int &index,int bits_num)
{
	int bit=0;
	return Plain_Rank(buff,index,bits_num,bit);
}

int BitMap::GammaDecode(u64 * buff,int & index)
{
	u32 x = GetBits(buff,index,32);
	int runs = Zeros(x>>16);
	int bits = (runs<<1)+1;
	index = index + bits;
	return x>>(32-bits);
}


//从buff的index位置开始,读取bits位数据,返回.
u64 BitMap::GetBits(u64 * buff,int &index,int bits)
{

	if((index & 0x3f) + bits < 65)
		return (buff[index>>6]<<(index &0x3f))>>(64-bits);

	int first = 64 - (index &0x3f);
	int second = bits - first;
	u64 high = (buff[index>>6] & ((0x01ull<<first)-1)) << second;
	return high + (buff[(index>>6)+1]>>(64-second));


}

int BitMap::GetZerosRuns(u64 * buff,int &index)
{
	
	u32 x = GetBits(buff,index,16);
	int runs = Zeros(x);
	index = index + runs;
	return runs;

}


void BitMap::Left(BitMap * left)
{
     this->left = left;
}


void BitMap::Right(BitMap * right)
{
     this->right = right;
}
 
unsigned char BitMap::Label()
{
    return label;
}


int BitMap::Load(loadkit & s)
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


int BitMap::Save(savekit & s)
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

int BitMap::RL_Rank(u64 * buff,int &index,int bits_num,int rl_type)
{
	int bit=0;
	return RL_Rank(buff,index,bits_num,rl_type,bit);
}


int BitMap::RL_Rank(u64 * buff,int &index,int bits_num,int rl_type,int &bit)
{
	int rank=0;
	int r=0;
	int already = 0;
	u64 x = GetBits(buff,index,64);
	int bits = 0;//通过查找表可以解码的被编码的0,1串的长度
	int step = 0;//
	int runs =0 ;//本次解码的runs数目
	int runs_num = 0;//累计的runs数目
	u32 anchor=0;
	rl_type=1-rl_type;
	if(bits_num > 32)
	{
		while(true)
		{
			anchor = (x>>48)<<2;
			runs =R[anchor];
			if(runs >0)
			{
				step =R[1+anchor];
				already = already + step;
				if(already > 64)
				{
					index = index + (already -step);
					x = GetBits(buff,index,64);
					already = 0;
					continue;
				}
				bits = R[2+anchor];
				r=R[3+anchor];
				bits=(bits==0)?256:bits;
				if((runs_num & 0x01) ==rl_type)
					rank = rank + r;
				else
					rank = rank + ( bits-r);
				bits_num = bits_num - bits;
				runs_num = runs_num + runs;
				if(bits_num <=0)
					break;
				x = (x<<step);
			}
			else
			{
				step = 1 + (Zeros(x>>48)<<1);
				already = already + step;
				if(already > 64)
				{
					index = index + (already - step);
					x = GetBits(buff,index,64);
					step = 1 + (Zeros(x>>48)<<1);
					already = step;
				}
				bits_num = bits_num - (x>>( 64 - step));
				if((runs_num &0x01) ==rl_type)
					rank = rank + (x>>( 64 - step));
				if(bits_num <=0)
				{
					if((runs_num &0x01)==rl_type)
					{
						bit=1;
						return rank + bits_num;
					}
					else
					{
						bit=0;
						return rank;
					}
				}
				runs_num++;
				x = (x<<step);
			}
		}
	}
	index = index + (already - step);
	bits_num = bits_num + bits;
	runs_num = runs_num - runs;
	if((runs_num &0x01) ==rl_type)
		rank = rank - r;
	else
		rank = rank - (bits -r );
	already = 0;
	x = GetBits(buff,index,64);
	while(true)
	{
		step = 1+ (Zeros(x>>48)<<1);
		already = already + step;
		if(already > 64)
		{
			index = index + (already - step);
			x =GetBits(buff,index,64);
			step = 1+ (Zeros(x>>48)<<1);
			already = step;
		}
		bits_num = bits_num - (x>>( 64 - step));
		if((runs_num &0x01) ==rl_type)
			rank = rank + (x>>( 64 - step));
		if(bits_num <= 0)
		{
			if((runs_num&0x01)==rl_type)
			{
				bit =1;
				return rank + bits_num;
			}
			else
			{
				bit=0;
				return rank;
			}
		}
		runs_num++;
		x=(x<<step);
	}
}

void BitMap::RL_Rank(u64 *buff,int &index,int bits_left,int bits_right,int &rank_left,int &rank_right,int rl_type)
{
	int old_index = index;
	rank_left+=RL_Rank(buff,old_index,bits_left,rl_type);
	//index = old_index;
	rank_right+=RL_Rank(buff,index,bits_right,rl_type);
}
