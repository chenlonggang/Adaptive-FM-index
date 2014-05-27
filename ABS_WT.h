#ifndef ABS_FM_H
#define ABS_FM_H
#define CHAR_SET_SIZE 256
#define CODE_MAX_LEN 256
#include<string.h>
#include"WT_Node.h"
#include"InArray.h"
#include"loadkit.h"
#include"savekit.h"
#include"ds_ssort.h"
class ABS_FM
{
	public:
		//D:SA数组采样步长,Rank采样步长D*16
		ABS_FM(const char * filename,int block_size=1024,int D=32);
		ABS_FM(){};
		virtual ~ABS_FM();
		void Counting(const char * partten,int &num);
		void Locating(const char * pattern,int &num,int * &pos);
		void Extracting(int pos,int len,char * &sequence);
		int Load(loadkit & s);
		int Save(savekit & s);
		int BuildTree();


		int GetN(){return n;}
		int SizeInByte();
		int SizeInByte_count();
	protected:
		WT_Node * root;
		uchar * zerostable;
	//	uchar * R1;//16bits表示的gamma编码的编码值的和
	//	uchar * R2;//16bits可以完整解码的bits数
	//	uchar * R3;//假设是1-runs时的rank值
	//	uchar * R4;//runs数目
	 	uchar * R;

		void Inittable();
		//int BuildTree();

		unsigned char * T;
		unsigned char * bwt;
		unsigned char * filename;
		int  n;//file size in byte
		int block_size;//the block_size of wavelettree's node.default 1024
		int D;//step of SAL
		InArray * SAL;//the sampling of SA array
		InArray * RankL;// The sampling of the Rank Array
		
		bool charMap[256];
		// if charMap[i] is true,char[i] is a member of alphabet
		
		int * C;// Cumulative Frequency sum
		uchar *code;

		int charFreq[CHAR_SET_SIZE];
		int alphabetsize;
		char codeTable[CHAR_SET_SIZE][CODE_MAX_LEN];

		int Occ(int &rank,unsigned char &label,int pos);
		int Occ(unsigned char c,int pos);
		int LF(int i);
		unsigned char L(int i);
		void DrawBackSearch(const char * pattern,int &Left,int & Right);
		int Lookup(int i);

		virtual int TreeCode(){return -1;};
		//每种树形只有TreeCode方法不一样.
		//该方法创建编码表，即初始化codeTable表，之后
		//BuildTree方法利用该标创建小波树.

		int BWT(unsigned char * T,int * SA,unsigned char * bwt,int n);
		WT_Node * CreateWaveletTree(unsigned char * bwt,int n);
		WT_Node * FullFillWTNode(unsigned char * bwt,int len,int level);
		int DestroyWaveletTree();
		int blog(int);
		unsigned char * Getfile(const char * filename);

		//只保存小波树该有的东西,不包快查找表
		//保存查找表是ABS_WT类的save的工作
		int SaveNodePosition(WT_Node *,u32, savekit &);
		int SaveNodeData(WT_Node *,savekit &s);
		int SaveWTTree(savekit & s);
		int LoadWTTree(loadkit & s,uchar ** tables=NULL);
		
		int TreeNodeCount(WT_Node * root);

		int TreeSizeInByte(WT_Node * r);
	//	int TreeSizeInByte();
		friend int GammaDecode(u64 * buff,int & index,ABS_FM * t);
		void Test_L();
		void Test_Occ();
		void Test_Shape(WT_Node *);

};
#endif

