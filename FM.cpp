#include"FM.h"
FM::FM(const char *filename,int block_size,int D,int shape)
{
	if(block_size<=0 || shape<0 || shape >2)
	{
		cout<<"FM::FM error parmater"<<endl;
		exit(0);
	}
	switch(shape)
	{
		//hutacker
		case 0: fm =new Hutacker_FM(filename,block_size,D);break;
		case 1: fm =new Huffman_FM(filename,block_size,D);break;
		case 2: fm =new Balance_FM(filename,block_size,D);break;
		default: fm=new Hutacker_FM(filename,block_size,D);break;
	}
	fm->BuildTree();
//	cout<<"BuildTree is ok"<<endl;
}


FM::FM()
{
	fm=NULL;
}
FM::~FM()
{
	delete fm;
}

int FM::GetN()
{
	return fm->GetN()-1;
}

int FM::SizeInByte()
{
	return fm->SizeInByte();
}

int FM::SizeInByte_count()
{
	return fm->SizeInByte_count();
}

int FM::Save(const char * indexfile)
{
	savekit s(indexfile);
	s.writeu64(198809102510);
	fm->Save(s);
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

	fm = new ABS_FM();
	fm->Load(s);
	s.close();
	cout<<"Load is ok"<<endl;
	return 0;
}


void FM::Counting(const char * pattern,int &num)
{
	fm->Counting(pattern,num);
}


void FM::Locating(const char * pattern,int & num,int * & pos)
{
	fm->Locating(pattern,num,pos);
}


void FM::Extracting(int pos,int len,char *sequence)
{
	fm->Extracting(pos,len,sequence);
}

