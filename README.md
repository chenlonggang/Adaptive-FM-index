#Hybrid-FM-index
	 
###if you
	 have any ideas and need a listener,email me!
	 have any trouble in using it,email me!
	 have found any bugs,let me know,emial me!


##What is it?
	 FM-index is a Succinct Data Structure(SDS), SDS can represent an object as whole object
	 in logic,in space close to information-theoretic lower bound of the object while 
	 supporting operations of the original object efficiently.
	 FM-index and CSA(compressed suffix array) both are implicit expression about SA(suffix array),
	 has the ability of fast pattern maching,and nedds little space,Hybrid-FM-index is close to Bzip2,
	 and CSA is a little weaker.
	 Hybrid-FM-index is a advanced version of the original FM-index,it's faster and smaller,
	 as you can guess,we developed a set of compression methods,for each piece of data,
	 always select the best one,that's the bigest secret about my works.
	 It works like a mixture of KMP and Bzip2,you can build a fm-index for a document,and the you 
	 have mainly there operations in your hands:
	 counting: count how many a pattern occurs in the document.
	 locating: locate all the position where the pattern occurs.
	 extracting: decompress a piece of the document.
## How to use it
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
	return 0;
}
