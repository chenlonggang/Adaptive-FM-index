#include<stdlib.h>
#include<string.h>
#include"FM.h"
#include<ctime>
#include<fstream>
#include<iostream>
using namespace std;
void usage();
void helpbuild();
void helpload();
void helpsave();
void helpcount();
void helplocate();
void splitcommand(string command,string result[]);
void showpos(int * pos,int num);
int main(int argc, char* argv[])
{
	
	usage();
	string command;
	string  result[3];
	char filename[100]={'\0'};
	char indexname[100]={'\0'};
	FM *csa=NULL;
	while(1)
	{
		result[0]="";
		result[1]="";
		result[2]="";
		command="";
		cout<<">";
		getline(cin,command);
		splitcommand(command,result);
		if(result[0]=="quit")
			break;
		else if(result[0]=="build")
		{
			if(csa!=NULL)
				delete csa;
			csa=NULL;
			csa=new FM(result[1].data());
		}
		else if(result[0]=="count")
		{
			int num=0;
			if(csa!=NULL)
			{
				csa->Counting(result[1].data(),num);
				cout<<"occs: "<<num<<endl;
			}
			else
			{
				cout<<"build a csa first"<<endl;
			}
		}
		else if(result[0]=="locate")
		{
			int * pos;
			int num=0;
			if(csa!=NULL)
			{
				csa->Locating(result[1].data(),num,pos);
				showpos(pos,num);
				delete [] pos;
			}
			else
				cout<<"build a csa first"<<endl;
		}
		else if(result[0]=="del")
		{
			if(csa!=NULL)
				delete csa;
			csa=NULL;
		}
		else if(result[0]=="load")
		{
			if(csa!=NULL)
				delete csa;
			csa=new FM();
			csa->Load(result[1].data());
		}
		else if(result[0]=="save")
		{
			csa->Save(result[1].data());
		}
		else if(result[0]=="size")
		{
			if(csa!=NULL)
				cout<<csa->SizeInByte()<<endl;
			else
				cout<<"build a csa first"<<endl;
		}
		else if(result[0]=="help")
		{
			if(result[1]=="build")
				helpbuild();
			if(result[1]=="count")
				helpcount();
			if(result[1]=="locate")
				helplocate();
			if(result[1]=="load")
				helpload();
			if(result[1]=="save")
				helpsave();
		}
		else
			usage();
	}

	return 0;
}

void showpos(int * pos,int num)
{
	cout<<"occs:"<<num<<endl;
	for(int i=0;i<num;i++)
	{
		cout<<pos[i]<<endl;
		if((i+1)%10==0)
		{
			char command;
			cout<<"-----------------more---------------------";
			system("stty raw");
			command=getchar();
     		cout<<endl<<'\r';
			system("stty cooked");
			if(command==27)//按下空格，返回
			{
				cout<<endl;
				return ;
			}
		}
	}
}




void splitcommand(string command,string result[])
{

	int i=0;
	int j=0;
	int start=0;
//	command=command+" ";
	int len=command.length();
	result[0]=command;
	for(i=0;i<len;i++)
	{
		if(command.at(i)!=' ')
			continue;
		else
		{
			result[j]=command.substr(start,i-start);
			start=i+1;
			j++;
		}
	}
	result[j]=command.substr(start,len);
}
void usage()
{
	cout<<"--------------------------------------------------------------------------------------"<<endl;
	cout<<"The flowing commands are supported "<<endl;
	cout<<"	help XX: show the details for the command XX"<<endl;
	cout<<"	build XX : build the index of file XX"<<endl;
	cout<<"	load  XX: load the index file XX"<<endl;
	cout<<"	save XX: write the csa to a index file XX"<<endl;
	cout<<"	count XX: count the pattern XX's occs" <<endl;
	cout<<"	locate YY: enum eyery  position of the pattern"<<endl;
	cout<<"	size: the size of the csa"<<endl;
	cout<<"	del : delete the current csa"<<endl;
	cout<<"	quit: say goodbye"<<endl;

}
void helpbuild()
{
	cout<<"build XX [-rl|-RL]"<<endl;
	cout<<"	XX:the source file,it's you responsibility to provide a correct path"<<endl;
	cout<<"	If -rl or -RL is provided,runlength will be used"<<endl;
}
void helpcount()
{
	cout<<"count XX"<<endl;
	cout<<"	XX:the pattern.you have the responsibility to ensure the index csa is nearby,otherwise,nothing you will get"<<endl;
}
void helplocate()
{
	cout<<"locate XX"<<endl;
	cout<<"	XX: the pattern.it's like count, and you can scan the positions of all the occs in a manner like the system command -more-"<<endl;
}
void helpload()
{
	cout<<"load XX"<<endl;
	cout<<"	XX: the csa index file, the command will read the index file and build a csa secretly"<<endl;
}
void helpsave()
{
	cout<<"save XX"<<endl;
	cout<<"	XX: the csa index file, the command will save the csa in file XX"<<endl;
}
