#ifndef FM_H
#define FM_H
#include"loadkit.h"
#include"savekit.h"
#include"ABS_WT.h"
#include"Huffman_WT.h"
#include"Balance_WT.h"
#include"Hutacker_WT.h"
#include"WT_Handle.h"
class FM
{
	public:
		FM(const char * filename,int block_size = 1024,int D=32,int shape=1);
		FM();
		~FM(){};
		FM(const FM & h):wt(h.wt){}
		FM& operator =(const FM&h){wt=h.wt;return *this;};
		
		void Counting(const char *pattern,int &num);
		void Locating(const char *pattern,int & num,int *&pos);
		void Extracting(int pos,int len,char *sequence);
		int Load(const char * indexfile);
		int Save(const char * indexfile);

		int GetN();
		int SizeInByte();
		int SizeInByte_count();
	private:
		WT_Handle wt;
};
#endif

