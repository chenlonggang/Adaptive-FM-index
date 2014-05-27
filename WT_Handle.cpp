#include"WT_Handle.h"

WT_handle::WT_Handle(const char * filename,int block_size=256,int D=32,int shape = 1)
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
	fm->BuildTree();
}

WT_Handle::WT_Handle(const WT_Handle &h):u(h.u),fm(h.fm){}

WT_Handle::WT_handle:: operator =(const WT_Handle & h)
{
	if(u.reattach(h.u))
		delete fm;
	fm = h.fm;
	return * this;
}

WT_Handle::~WT_Handle()
{
	if(u.only())
		delete fm;
}


