#include<stdlib.h>
#include<sys/time.h>
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
		}
		cout<<"pattern is ready"<<endl;

		double totnum =0;
		int num = 0;

		struct timeval start;
		struct timeval end;
		gettimeofday(&start,NULL);

		for(int i=0;i<times;i++)
		{
			fm.Counting(p[i],num);
			totnum = totnum + num;
		}

		gettimeofday(&end,NULL);
		//time_t t2 =clock();
		//cout<<"count-time: "<<(t2-t1)/1000000.0<<endl;
		cout<<"count-time: "<<((end.tv_sec-start.tv_sec)*1000000.0+end.tv_usec - start.tv_usec)/times<<endl;
		cout<<"avera-nums: "<<totnum/times<<endl;
		return 0;
	}


}

