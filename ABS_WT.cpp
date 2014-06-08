/*============================================
# Filename: ABS_WT.cpp
# Ver 1.0 2014-06-08
# Copyright (C) 2014 ChenLonggang (chenlonggang.love@163.com)
#
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 or later of the License.
#
# Description: 
=============================================*/
#include"ABS_WT.h"
#include"ds_ssort.h"
#include<string.h>
#include<map>

u64 GetBits(u64 * buff,int &index,int bits)
{
	if((index & 0x3f) + bits < 65)
		return (buff[index>>6] >>( 64 -((index&0x3f) + bits))) & ((0x01ull<<bits)- 1);
	int first = 64 - (index &0x3f);
	int second = bits - first;
	u64 high = (buff[index>>6] & ((0x01ull<<first)-1)) << second;
	return high + (buff[(index>>6)+1]>>(64-second));
}

int Zeros(u16 x,ABS_FM *t)
{
	if(t->Z[x>>8]==8)
		return t->Z[x>>8]+t->Z[(uchar)x];
	else
		return t->Z[x>>8];
}

int GammaDecode(u64 * buff,int & index,ABS_FM * t)
{
	u32 x = GetBits(buff,index,32);
	int runs = Zeros(x>>16,t);
	int bits = (runs<<1)+1;
	index = index + bits;
	return x>>(32-bits);
}

ABS_FM::ABS_FM(const char * filename,int block_size,int D)
{
	this->block_size = block_size;
	this->D =D;
	this->T=NULL;
	T = Getfile(filename);
	Inittable();
}

ABS_FM::~ABS_FM()
{
	DestroyWaveletTree();
	if(T)
		delete [] T;
	if(bwt)
		delete [] bwt;
	if(SAL)
		delete SAL;
	if(RankL)
		delete RankL;
	if(C)
		delete [] C;
	if(code)
		delete [] code;
	if(Z)
		delete [] Z;
	if(R)
		delete [] R;
}

int ABS_FM::SizeInByte()
{
	return TreeSizeInByte(root) + SAL->GetMemorySize() + RankL->GetMemorySize();
}

int ABS_FM::SizeInByte_count()
{
	return TreeSizeInByte(root);
}

int ABS_FM::TreeNodeCount(BitMap * r)
{
	if(r==NULL)
		return 0;
	return TreeNodeCount(r->Left()) + TreeNodeCount(r->Right()) + 1;
}

int ABS_FM::TreeSizeInByte(BitMap * r)
{
	int size = 0;
	if(r->Left())
		size += TreeSizeInByte(r->Left());
	if(r->Right())
		size+=TreeSizeInByte(r->Right());
	size = size + r->SizeInByte();
	return size;
}

void ABS_FM::DrawBackSearch(const char * pattern,int & Left,int &Right)
{
	int len = strlen(pattern);
	int occ_left=0;
	int occ_right=0;
	if(len <=0)
	{
		Left =1;
		Right = 0;
		return;
	}
	int i = len -1;
	unsigned char c = pattern[i];
	int coding = code[c];
	if (coding==-1)
	{
		Left = 1;
		Right = 0;
		return ;
	}
	Left = C[coding];
	Right = C[coding+1]-1;
	i=i-1;
	while ((Left <= Right) and (i>=0))
	{
		c = pattern[i];
		coding = code[c];
		if(coding == -1)
		{
			Left = 1;
			Right = 0;
			return;
		}
	/*	
	    Left = C[coding]+Occ(c,Left-1);
		Right =C[coding]+Occ(c,Right)-1;
		i=i-1;
	*/
		Occ(c,Left-1,Right,occ_left,occ_right);
		Left = C[coding]+occ_left;
		Right = C[coding]+occ_right-1;
		i=i-1;
	}
	if(Right < Left)
	{
		Left = 1;
		Right = 0;
		return ;
	}
	return;
}


void ABS_FM::Counting(const char * pattern,int & num)
{
	int Left=1;
	int Right =0;
	DrawBackSearch(pattern,Left,Right);
//	cout<<"Counting "<<Left<<" "<<Right<<endl;
	num = Right -Left +1;
}


void ABS_FM::Locating(const char * pattern,int &num,int *& pos)
{
	int Left=1;
	int Right = 0;
	DrawBackSearch(pattern,Left,Right);
	if(Right < Left)
	{	pos=NULL;
		return ;
	}
	else
	{
		num = Right - Left + 1;
		pos =new int[num];
		for(int i=0;i<num;i++)
		{
			pos[i]=Lookup(Left + i);
		}
	}
}


void ABS_FM::Extracting(int pos,int len,char * &sequence)
{
	if(pos + len > n-1 || pos <0)
	{
		cout<<pos<<"  "<<len<<endl;
		cout<<pos+len<<" "<<n-1<<" "<<pos<<endl;
		cout<<"ABS_FM::Extracting  error parmaters"<<endl;
		exit(0);
	}
	int end = pos + len -1;
	int anchor = 0;
	int overloop = 0;

	int step = this->D*16;

	overloop = (n-2-end)%step;
	anchor = (n-2-end)/step;
	
	int i= RankL->GetValue(anchor);
	for(int j=0;j<overloop;j++)
		i = LF(i);

	for(int j=0;j<len;j++)
	{
		sequence[len-1-j]=L(i);
	//	cout<<L(i);
		i = LF(i);
	}
}


int ABS_FM::Lookup(int i)
{
	int step = 0;
	int D = this->D;
	while(i%D!=0)
	{
		i=LF(i);
		step =step +1;
	}
	i=i/D;
	return (SAL->GetValue(i)+step)%n;
}

//返回L串中c字符在位置pos_left 和pos_right之前出现的次数，结果由rank_left 和rank_right带回.
void ABS_FM::Occ(unsigned char c,int pos_left,int pos_right,int &rank_left,int &rank_right)
{
	BitMap *r = root;
	int level=0;
	char code = '0';
	while(r->Left())
	{
		code = codeTable[c][level];
		
		if(code == '1')//编码是1,走右分支
		{
			if(pos_left>-1 && pos_right >-1) //left right 都有待查找
			{
			
				r->Rank(pos_left,pos_right,rank_left,rank_right);
				pos_left = rank_left -1;
				pos_right = rank_right -1;
			
			/*	pos_left = r->Rank(pos_left)-1;
				pos_right=r->Rank(pos_right)-1;
			*/
			}
			else if(pos_right > -1)//只查右分支
			{
				pos_right=r->Rank(pos_right)-1;
			}
			else//该跳出循环了,此时pos_left 和pos_right都是-1.
			{
				break;
			}
			r= r->Right();
		}
		else //编码是0,走左分支.
		{
			if(pos_left>-1 && pos_right >-1)
			{
				
				r->Rank(pos_left,pos_right,rank_left,rank_right);
				pos_left = (pos_left+1) - rank_left-1;
				pos_right= (pos_right+1)- rank_right-1;
			/*
				pos_left = (pos_left+1)-r->Rank(pos_left)-1;
				pos_right= (pos_right+1)-r->Rank(pos_right)-1;
			*/
			}
			else if(pos_right > -1)
			{
				pos_right = (pos_right+1)-r->Rank(pos_right)-1;
			}
			else
			{
				break;
			}
			r=r->Left();
		}
		level++;
	}
	rank_left = pos_left+1;
	rank_right= pos_right+1;
	return ;
}
int ABS_FM::Occ(unsigned char c,int pos)
{
	BitMap * r = root;
	int level = 0;
	char code ='0';
	while(r->Left() && pos > -1)
	{
		code = codeTable[c][level];
		if(code == '1')
		{
			pos = r->Rank(pos) - 1;
			r = r->Right();
		}
		else
		{
			pos = (pos+1) - r->Rank(pos)-1;
			r = r->Left();
		}
		level = level +1;
	}
	return pos+1;
}


int ABS_FM::LF(int i)
{
/*
	unsigned char c = L(i);
//	cout<<"L(i)  "<<c<<endl;zerostable = zerostable;
	int coding = code[c];
	return C[coding]+Occ(c,i)-1;
*/
	
	int occ =0;
	unsigned char label =0;
	Occ(occ,label,i);
	int coding = code[label];
	return occ + C[coding] -1;

}


unsigned char ABS_FM::L(int i)
{
	BitMap * r = root;
	int bit =0;
	int rank = 0;

/*	//Bit(i)和Rank(i)可以在一步内计算出来，可以减少一次检索BitMap的功夫.
	while(r->Left() || r->Right())
	{
		//bit = r->GetBit(data,i);
		bit = r->Rank(i) - r->Rank(i-1);
//		cout<<"L "<<bit<<endl;
		if(bit ==1)
		{
			i = r->Rank(i)-1;
			r=r->Right();
		}
		else
		{
			i = (i+1) - r->Rank(i)-1;
			r=r->Left();
		}
	}
	return r->Label();
*/

	
	while(r->Left())
	{
	//	bit = r->Rank(i) - r->Rank(i-1);
		rank = r->Rank(i,bit);
		if(bit ==1)
		{
			//i = r->Rank(i)-1;
			i = rank -1;
			r=r->Right();
		}
		else
		{
			//i = (i+1) - r->Rank(i)-1;
			i = (i+1) - rank -1;
			r=r->Left();
		}
	}
	return r->Label();

}

int ABS_FM::Occ(int & occ , unsigned char & label,int pos)
{
	BitMap * r = root;
	int bit =0;
	int rank =0;
	while(r->Left())
	{
		rank = r->Rank(pos,bit);
		if(bit==1)
		{
			pos = rank -1;
			r =r ->Right();
		}
		else
		{
			pos = (pos+1) - rank -1;
			r = r->Left();
		}
	}
	occ = pos +1;
	label = r->Label();
	return 0;
}


unsigned char * ABS_FM::Getfile(const char *filename)
{
	FILE * fp = fopen(filename,"r+");
	if(fp==NULL)
	{
		cout<<"Be sure the file is available"<<endl;
		exit(0);
	}
	fseek(fp,0,SEEK_END);
	this->n = ftell(fp)+1;
//	cout<<n<<endl;

	int overshot = init_ds_ssort(500,2000);
	unsigned char * T = new unsigned char[n+overshot];
	fseek(fp,0,SEEK_SET);

	int e=0;
	int num=0;
	while((e=fread(T+num,sizeof(uchar),n-1-num,fp))!=0)
		num = num +e;
	if(num!=n-1)
	{
		cout<<"Read source file failed"<<endl;
		exit(0);
	}
	T[n-1]=0;
	fclose(fp);

	memset(charFreq,0,256*sizeof(int));
	memset(charMap,0,256*sizeof(bool));
	for(int i=0;i<n;i++)
		charFreq[T[i]]++;
	this->alphabetsize = 0;
	for(i32 i=0;i<256;i++)
		if(charFreq[i])
		{
			this->alphabetsize++;
			this->charMap[i]=true;
		}
	this->code = new uchar[256];
	this->C = new int[alphabetsize+1];
	memset(C,0,(alphabetsize+1)*4);
	this->C[alphabetsize] = n;
	this->C[0] = 0;
	int k=1;
	i32 pre =0;
	for(int i=0;i<256;i++)
	{
		if(charFreq[i]!=0)
		{
			code[i]=k-1;
			C[k]=pre + charFreq[i];
			pre = C[k];
			k++;
		}
		else
			code[i]=-1;
	}
	return T;
}


int ABS_FM::BWT(unsigned char *T,int * SA,unsigned char * bwt,int len)
{
	int i=0;
	int index=0;
	for(i=0;i<len;i++)
	{
		index = (SA[i]-1+len)%len;
		bwt[i]=T[index];
	}
	return 0;
}


int ABS_FM::BuildTree()
{
	int *SA = new int[n];
	ds_ssort(T,SA,n);	
	
	//SA和Rank数组的采样
	int step1 =this->D;
	int step2 =this->D*16;
	SAL=new InArray(n/step1+1,blog(n));
	RankL=new InArray(n/step2+1,blog(n));

	int i=0;
	int j=0;
	for(i=0,j=0;i<n;i=i+step1,j++)
		SAL->SetValue(j,SA[i]);
	
	for(i=0;i<n;i++)
	{
		if(SA[i]==0)
			continue;
		if((n-2-(SA[i]-1))%step2 == 0)
		{
			RankL->SetValue((n-2-(SA[i]-1))/step2,i);
		}
	}

	bwt = new unsigned char[n];

	BWT(T,SA,bwt,n);
	TreeCode();
	root=CreateWaveletTree(bwt,n);
//	cout<<"CreatWaveletTree"<<endl;
	
	//Test_L();
	//Test_Occ();
	//Test_Shape(root);

	delete [] T;
	T=NULL;
	delete [] SA;
	SA=NULL;
	delete[] bwt;
	bwt=NULL;

	return 0;
}

void ABS_FM::Test_Shape(BitMap * r)
{
	if(r->Left() && r->Right())
	{
		Test_Shape(r->Left());
		Test_Shape(r->Right());
	}
	else if(r->Left() || r->Right())
	{
		cout<<"one child"<<endl;
	}
}
	
void ABS_FM::Test_Occ()
{
	int count[256]={0};
	unsigned long long int mis=0;
	for(int i=0;i<n;i++)
	{
		count[bwt[i]]++;
		if(Occ(bwt[i],i) != count[bwt[i]])
		{
			cout<<count[bwt[i]]<<" "<<Occ(bwt[i],i)<<" "<<(int)bwt[i]<<" "<<(char)bwt[i]<<" "<<i<<endl;
			cout<<codeTable[bwt[i]]<<endl;
			mis++;
		}
		if(i%100000==0)
			cout<<(i*1.0)/n<<endl;
	}
	cout<<"missing :"<<mis<<endl;

}

void ABS_FM::Test_L()
{
	int mis=0;
	for(int i=0;i<n;i++)
	{
		if(bwt[i]!=L(i))
		{
			cout<<bwt[i]<<" "<<L(i)<<" "<<i<<endl;
			mis++;;
		}
	}
	cout<<"mis: "<<mis<<endl;
}


BitMap * ABS_FM::CreateWaveletTree(unsigned char * bwt,int n)
{
	BitMap * root = NULL;

	root = FullFillWTNode(bwt,n,0);
	if(!root)
	{
		cout<<"FullfillWTNode failed"<<endl;
		exit(0);
	}
	return root;
}


BitMap * ABS_FM::FullFillWTNode(unsigned char * buff,int len,int level)
{
//	cout<<level<<endl;
	int CurrentLevel = level;
	unsigned int CurrentBitLen = len;
	unsigned char CurrentLabel = '\0';
	unsigned long long int *CurrentBitBuff = NULL;
	if ((int)strlen((const char*)codeTable[buff[0]])==level)
	{
		CurrentLabel = buff[0];
		CurrentBitBuff = NULL;
		//uchar * tables[5]={this->zerostable,this->R1,this->R2,this->R3,this->R4};
		uchar * tables[2] ={this->Z,this->R};
		BitMap * node = new BitMap(CurrentBitBuff,CurrentBitLen,CurrentLevel,block_size,CurrentLabel,tables);
		node->Left(NULL);
		node->Right(NULL);
		return node;
	}
	
	int u64Len=0;
	if(len%64==0)
		u64Len = len/64+1;
	else
		u64Len = len/64+2;
	CurrentBitBuff = new unsigned long long int[u64Len];
	memset(CurrentBitBuff,0,u64Len*8);
	unsigned char * lptr=NULL;
	unsigned char * rptr=NULL;
	int leftLen=0;
	int rightLen=0;

	lptr = new unsigned char[len];
	rptr = new unsigned char[len];
	memset(lptr,0,len);
	memset(rptr,0,len);

	//computer bitvect;

	int i=0;
	int bytePos=0;
	int bitOffset=0;
	u64 last = 0;
	for(i=0;i<len;i++)
	{
		if(codeTable[buff[i]][level]=='1')
		{
			//CurrentBitBuff[bytePos] |= (0x01<<(7-bitOffset));
			CurrentBitBuff[bytePos] |= (0x01ull<<(63-bitOffset));
			//construct right data buff
			rptr[rightLen++]=buff[i];
			last = 0;
		}
		else
		{
			lptr[leftLen++]=buff[i];
			last = 1;
		}
		bitOffset++;
		//if(bitOffset == 8)
		if(bitOffset == 64)
		{
			bytePos++;
			bitOffset = 0;
		}
	}
	CurrentBitBuff[bytePos] |= (last<<(63-bitOffset));

	//uchar * tables[5] = {this->zerostable,this->R1,this->R2,this->R3,this->R4};
	uchar * tables[2] = {this->Z,this->R};
	BitMap * node = new BitMap(CurrentBitBuff,CurrentBitLen,CurrentLevel,block_size,CurrentLabel,tables);

	if(leftLen !=0)
	{
		BitMap * left =FullFillWTNode(lptr,leftLen,level+1);
		node->Left(left);
		delete [] lptr;
		lptr=NULL;
	}
	if(rightLen!=0)
	{
		BitMap * right = FullFillWTNode(rptr,rightLen,level+1);
		node->Right(right);
		delete [] rptr;
		rptr=NULL;
	}
	return node;
}


int ABS_FM::DestroyWaveletTree()
{
	delete root ;
	root=NULL;
	return 0;
}


int ABS_FM::blog(int x)
{
	int ans=0;
	while(x>0)
	{
		ans++;
		x=(x>>1);
	}
	return ans;
}


void ABS_FM::Inittable()
{
	this -> Z = new uchar[1<<8];
	int tablewidth = 8;
	for(int i=0;i<tablewidth;i++)
		for(int j=(1<<i);j<(2<<i);j++)
			Z[j] = tablewidth-1-i;
	Z[0] = tablewidth;
	
	u64 tablesize = (1<<16);
	R  = new uchar[tablesize<<2];
	
	//查找表的初始化：在16bits的0,1串上模拟gamma解码的过程，得到
	//这些表
	u64 B[2]={0xffffffffffffffffull,0xffffffffffffffffull};
	int sum =0;//gamma编码的和,含义为原串被编码的bits数目。
	int step=0;//16bits可以完整解码的bits数,该值不会大于16.
	int rank = 0;//16bits表示的几个完整的runs,假设第一个runs是1-runs,这几个runs的rank值。
	int runs = 0 ;//runs 个数.
	
	int x = 0;//工作变量，保存本次的gamma解码值.
	int prestep = 0;//前一次正确解码的bits数(累加),<=16.
	for(u64 i=0;i<tablesize;i++)
	{
		B[0] = (i<<48);
		sum  =0 ;
		step = 0;
		prestep=0;
		rank = 0;
		runs = 0;
		while(1)
		{
			x = GammaDecode(B,step,this);//step会联动.
			if(step > 16)
				break;
			sum = sum + x;
			prestep = step;
			runs ++;
			if(runs%2==1)
				rank = rank + x;
		}
		R[i<<2] = runs;//r4
		R[(i<<2)+1] = prestep;//r2
		R[(i<<2)+2] = sum; //r1;
		R[(i<<2)+3] = rank;//r3

	}
}

//递归保存节点的编号信息
int ABS_FM::SaveNodePosition(BitMap * r,u32 position,savekit &s)
{
	if(!r)
		return 1;
	s.writei32(position);
	SaveNodePosition(r->Left(), 2 * position,s);
	SaveNodePosition(r->Right(),2 * position +1,s);
	return 0;
}

//递归保存节点的数据信息
int ABS_FM::SaveNodeData(BitMap *r,savekit &s)
{
	if(!r)
		return 1 ;
	r->Save(s);
	SaveNodeData(r->Left(),s);
	SaveNodeData(r->Right(),s);
	return 0;
}

int ABS_FM::SaveWTTree(savekit &s)
{
	//保存编号信息
	//int nodecount = 2*alphabetsize -1;
	//s.writei32(nodecount);
	SaveNodePosition(root,1,s);

	//保存节点数据信息
	SaveNodeData(root,s);
	return 0;
}

int ABS_FM::LoadWTTree(loadkit &s,uchar **tables)
{
	//读取数据，map的int域对应该节点的位置
	int nodecount = 2*alphabetsize -1;
//	cout<<alphabetsize<<endl;
//	s.loadi32(nodecount);
	int * p = new int[nodecount];
	s.loadi32array(p,nodecount);
	map<int,BitMap * > pmap;
	BitMap * r=NULL;
	for(int i=0;i<nodecount;i++)
	{
		if(tables)
			r = new BitMap(tables);
		else
			r = new BitMap();
		r->Load(s);
		pmap[p[i]] = r;
	}

//	cout<<"748"<<endl;

	//挂链
	map<int ,BitMap *>::iterator iter;
	map<int ,BitMap *>::iterator f_iter;
	for(iter = pmap.begin();iter!=pmap.end();iter++)
	{
		f_iter = pmap.find(2*iter->first);
		if(f_iter != pmap.end())
			iter->second->Left(f_iter->second);
		else
			iter->second->Left(NULL);
		
		f_iter = pmap.find(2*iter->first +1);
		if(f_iter!=pmap.end())
			iter->second->Right(f_iter->second);
		else
			iter->second->Right(NULL);
	}
//	cout<<"767"<<endl;	
	f_iter = pmap.find(1);
	if(f_iter !=pmap.end())
		this->root = f_iter->second;
	else
	{
		cerr<<"Load WTTree error"<<endl;
		this->root = NULL;
		exit(0);
	}
//	cout<<"778"<<endl;
	return 0;
}

int ABS_FM::Load(loadkit &s)
{
	s.loadi32(this->n);
	s.loadi32(this->alphabetsize);
	s.loadi32(this->D);

	//for C
	this->C = new int[alphabetsize+1];
	s.loadi32array(this->C,alphabetsize+1);
	//for code
	this->code = new uchar[256];
	s.loadu8array(this->code,256);
	//for codeTable;
	memset(codeTable,0,sizeof(codeTable));
	for(int i=0;i<256;i++)
	{
		uchar len=0;
		s.loadu8(len);
		if(len!=0)
		{
			int bytes = len%8?len/8+1:len/8;
			uchar * bits = new uchar[bytes];
			s.loadu8array(bits,bytes);
			int in_index =0;
			int off_index =0;
			for(int j=0;j<len;j++)
			{
				if(bits[off_index] & (0x01<<(7-in_index)))
					codeTable[i][j] = '1';
				else
					codeTable[i][j] = '0';
				in_index++;
				if(in_index==8)
				{
					in_index =0;
					off_index ++;
				}
			}
		}
	}
	
	//for SAL
	this->SAL = new InArray();
	this->SAL->load(s);
	//for Rankl
	this->RankL = new InArray();
	this->RankL->load(s);

	Inittable();
	uchar * par[2]={Z,R};
	//cout<<"cs"<<endl;
	LoadWTTree(s,par);
//	cout<<"835"<<endl;
	T=NULL;
	bwt=NULL;
	return 0;
}
int ABS_FM::Save(savekit &s)
{
	s.writei32(n);
	s.writei32(alphabetsize);
	s.writei32(D);//SA的采样率
	
	//C表
	//s.writei32(alphabetsize+1);
	s.writei32array(C,alphabetsize+1);
	//code表
	//s.writei32(256);
	s.writeu8array(code,256);
	
	//codeTable
	for(int i=0;i<256;i++)
	{
		uchar len = strlen(codeTable[i]);
		s.writeu8(len);
		if(0!=len)
		{
			int bytes = len%8?len/8+1:len/8;
			uchar *bits = new uchar[bytes];
			memset(bits,0,bytes);
			int off_index=0;
			int in_index =0;
			for(int j=0;j<len;j++)
			{
				if(codeTable[i][j]=='1')
					bits[off_index] = bits[off_index]|(0x01<<(7-in_index));
				in_index++;
				if(8==in_index)
				{
					in_index=0;
					off_index++;
				}
			}
			s.writeu8array(bits,bytes);
		}
	}
	
	//for SAL
	SAL->write(s);
	//for RankL
	RankL->write(s);
	//for WT tree
//	cout<<"SaveWTTree"<<endl;
	SaveWTTree(s);
	return 0;
}
