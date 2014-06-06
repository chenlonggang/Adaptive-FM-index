#Hybrid-FM-index

##What is it?
	 FM-index is a Succinct Data Structure(SDS), SDS can represent an object as implicitly,
	 and in space close to information-theoretic lower bound of the object while 
	 supporting operations of the original object efficiently.
	 FM-index and CSA(compressed suffix array) both are implicit expression about SA(suffix array),
	 has the ability of fast pattern matching,and needs little space, and Hybrid-FM-index is more
	 space efficient, comparable with Bzip2,but CSA is a little weake in space.
	 Hybrid-FM-index is a advanced version of the original FM-index,it's faster and smaller,
	 as you can guess,we developed a set of compression methods,for each pieces of data,
	 always select the best one,that's the bigest secret about my works.
	 It works like a mixture of KMP and Bzip2,you can build a fm-index for a document,and the you 
	 have mainly there operations in your hands:
	 counting: count how many a pattern occurs in the document.
	 locating: locate all the positions where the pattern occurs.
	 extracting: decompress a piece of the document.
## How to use it?
###just for fun
	 step 1:download it or clone it
	 step 2:make
	 step 3:run my_fm
###build your own program
	 step 1:download or clone it
	 step 2:make
	 step 3:include FM.h
	 step 4: g++ your_program.cpp -o xx -fm.a

###example
	```cpp
	#include"fm.h"
	#include<iostream>
	using namespace std;
	int main()
	{
		FM fm("filename");
		int num;
		fm.Counting("the",num);
		cout<<"pattern the occs "<<num<<" times"<<endl;
		int *pos;
		fm.Locating("love",num,pos);
		cout<<"pattern love occs "<<num<<" times"<<endl;
		cout<<"all the positions are:";
		for(int i=0;i<num;i++)
			cout<<pos[i]<<endl;
		delete [] pos;//it's your duty to delete pos.
		pos=NULL;

		char * sequence;
		int start=0;
		int len =20;
		fm.Extracting(start,len,sequence);
		cout<<"T[start...start+len-1] is "<<sequence<<endl;
		delete [] sequence;//it's your duty to delete sequence.
    	sequence =NULL;

		fm.Save("index.fm");//serialize the fm object to file index.fm
		FM fm2;
		fm2->Load("index.fm");//restore the fm object from file index.fm

		return 0;
	}
	```

##ChangeLog
2014.5.25:   
Use the LookupTables,and cross them,it looks helpful,count works faster about 25~35%.  

2014.5.27:   
If gamma coding don't save much space,only very few,using plain first.it helps,not very significant.  

2014.5.28:   
Complete the copy constructor,assigment operator,for class FM by using UseCount Tec. It seems WT-Node is not a good name,BitMap is better.  

2014.5.30:  
Want to computer from head or tail,depending which points is good,working on it Now!

2014.6.1:  
It seems that Rank(int pos) works now,and the answer seems right.That's good!. Now working on Rank(int pos,int &bit).
And change rename 'WT-Node' to BitMap    

2014.6.2:  
Computer Rank from head or tail adaptivly, but the result is not good,this may be a result of expanded lookuptables,or bit-reverse function.
So we can drop the drawback-lookup tables,and drop the bit-reverse function,only holds:forward-lookuptables and reverse-table,this will reduce
space of tables,and bit-reverse can use the reverse-table,so it's may be faster.working on it!  

2014.6.3  
It does not work to drop backforward-lookup tables and bit-reverse function,So,expanded lookuptables and bit-reverse function is not the suspect of
fading performance  

2014.6.4  
After a long time thinking,it's no need to use reverse-function or reverse-tables. we can map all the gamma value to odd numbers,so the lowest bit will be 1,
and it can denote the boundary. The mapping function: f(x)=x+x-1 or f(x)=x+x-3 or f(x) = x+x-5...  

2014.6.6:  
Rank from head or tail will not save time for any kind of document.if the file is random-like file,the needed block should be small,in this situation,from 
both points will not help,because time used for decoding is not very significant,and time for prepare-operations will increase.for highly-repetive data the runs
are big,the decoding process is good enough,for example,if the block-size is 1024,the runs will be hundreds,so only a few decoding steps is needed.In this situation,
form both points will not help too.So let it go,bye!
	
2014.6.6:  
drawbacksearch function has the potential to works faster.Woking on it

