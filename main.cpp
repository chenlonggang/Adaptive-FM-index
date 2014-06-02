#include<stdlib.h>
#include"FM.h"
#include<iostream>
using namespace std;
#define  times 10000
int main(int argc,char ** argvs)
{
	
	if(argc ==5)
	{

		if(argc!=5)
		{
			cout<<"error parmater"<<endl;
			exit(0);
		}
		cout<<"Pattern: "<<argvs[2]<<endl;
		FM fm(argvs[1]);
		int num=0;
		fm.Counting(argvs[2],num);
		cout<<"Counting: "<<num<<endl;
		int * pos=NULL;
		cout<<"Locating:"<<endl;
		fm.Locating(argvs[2],num,pos);
		for(int i=0;i<num && i<10;i++)
			cout<<pos[i]<<endl;
		char * p= new char[atoi(argvs[4])+1];
		p[atoi(argvs[4])]='\0';
		cout<<"Extracting :"<<"["<<argvs[3]<<" "<<argvs[4]<<"]"<<endl;
		fm.Extracting(atoi(argvs[3]),atoi(argvs[4]),p);
		cout<<p<<endl;
	
		return 0;
	}
	else
	{

		if(argc!=2)
		{
			cout<<"use it like:   ./my_fm file"<<endl;
			exit(0);
		}

		time_t t3 = clock();
		FM fm(argvs[1]);
		time_t t4 = clock();
		cout<<"build time: "<<(t4-t3)/1000000.0<<endl;
		cout<<"fm is ready"<<endl;
		int n =fm.GetN();
		int size = fm.SizeInByte_count();
		cout<<"szie "<<size/(n*1.0)<<endl;
		char p[times][21]={'\0'};
		for(int i=0;i<times;i++)
		{
			int x = rand()%( n - 50);
			fm.Extracting(x,20,p[i]);
//			cout<<p[i]<<endl;
		}
		cout<<"pattern is ready"<<endl;

		double totnum =0;
		int num = 0;
		time_t t1 =clock();
		for(int i=0;i<times;i++)
		{
			fm.Counting(p[i],num);
			totnum = totnum + num;
		}
		time_t t2 =clock();
		cout<<"count-time: "<<(t2-t1)/1000000.0<<endl;
		cout<<"avera-nums: "<<totnum/times<<endl;
		cout<<"totnum :"<<totnum<<endl;

/*

		time_t t5 = clock();
		int * pos=NULL;
		char temp[21]={'\0'};
		for(int i=0;i<times;i++)
		{
			fm.Locating(p[i],num,pos);

			delete [] pos;
		}
		time_t t6 = clock();
		cout<<"locat-time: "<<(t6-t5)/1000000.0<<endl;
*/		
//		cout<<n<<endl;
/*
		char * pp =new  char[n];
		memset(pp,0,n);
		fm.Extracting(0,n,pp);
		for(int i=0;i<n;i++)
			cout<<pp[i];
*/		

		// for Save and Load
/*		fm.Save("fm.index");
		cout<<"Save is ok"<<endl;
		FM fm1;
		fm1.Load("fm.index");
		cout<<"fm1 is load ok"<<endl;
		fm1.Counting("the",num);
		cout<<num<<endl;
		char  s[21]={'\0'};
		fm1.Extracting(0,20,s);
		cout<<s<<endl;
		int * sa;
		fm1.Locating("the",num,sa);
		for(int i=0;i<10;i++)
			cout<<sa[i]<<endl;
*/

		return 0;
	}


}

