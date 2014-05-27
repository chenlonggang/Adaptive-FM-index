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
		WT_Handle(const char * filename,int block_size=256,int D=32,int shape = 1);
		WT_Handle(const WT_Handle &);
		WT_Handle & operator = (const WT_Handle & );
		~WT_Handle();
		
		void Counting(const char * pattern,int &num) { fm->Counting(pattern,num); };
		void Locating(const char * pattern,int &num,int * &pos){ fm->Locating(pattern,num,pos); };
		void Extracting(int pos,int len,char * &sequence){ fm->Extracting(pos,len,sequence);};
		int Load(loadkit & s) { return fm->Load(s);};
		int Save(savekit & s){ return fm->Save(s);};

		int GetN(){ return fm->GetN();}
		int SizeInByte(){ return fm->SizeInByte();};
		int SizeInByte_count() { return fm->SizeInByte_count();};
};
#endif
