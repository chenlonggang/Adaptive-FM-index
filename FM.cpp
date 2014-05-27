#include"FM.h"

FM::FM(const char *filename,int block_size,int D,int shape):wt(filename,block_size,D,shape){}

FM::FM():wt(){}

int FM::GetN()
{
	return wt.GetN()-1;
}

int FM::SizeInByte()
{
	return wt.SizeInByte();
}

int FM::SizeInByte_count()
{
	return wt.SizeInByte_count();
}

int FM::Save(const char * indexfile)
{
	savekit s(indexfile);
	s.writeu64(198809102510);
	wt.Save(s);
	s.close();
	return 0;
}


int FM::Load(const char * indexfile)
{
	loadkit s(indexfile);
	unsigned long long int magicnum=0;
	s.loadu64(magicnum);
	if(magicnum!=198809102510)
	{
		cerr<<"Not a FM_Index file"<<endl;
		exit(0);
	}
	wt.Load(s);
	s.close();
	cout<<"Load is ok"<<endl;
	return 0;
}


void FM::Counting(const char * pattern,int &num)
{
	wt.Counting(pattern,num);
}


void FM::Locating(const char * pattern,int & num,int * & pos)
{
	wt.Locating(pattern,num,pos);
}


void FM::Extracting(int pos,int len,char *sequence)
{
	wt.Extracting(pos,len,sequence);
}

