#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<dirent.h>
#include<map>
#include<stack>
#include<math.h>
#include<string>
#include<algorithm>
#include<string.h>
#include<vector>
#include<set>
#include<time.h>
#include<string>
using namespace std;
const double eps=1e-5;
struct Document
{
	int clusterid;
	double distance;
	char filename[50];
	map<int,int> frequency;
	map<int,double> weight;
	double length=0,similarity=0;
	bool operator<(const Document &rhs)
	{
		return similarity>rhs.similarity;
	}
	double Length()
	{
		double length=0;
		map<int,int>::iterator it;
		for(it=frequency.begin();it!=frequency.end();it++)
			length+=it->second*it->second;
		return sqrt(length);
	}
};
struct Center
{
	map<int,double> frequency;
	Center()
	{
		frequency.clear();
	}
	double Length()
	{
		map<int,double>::iterator it;
		double dis=0;
		for(it=frequency.begin();it!=frequency.end();it++)
		{
			dis+=it->second*it->second;
		}
		return sqrt(dis);
	}
};
bool operator==(const Center &lhs,const Center &rhs)
{
	if(lhs.frequency.size()!=rhs.frequency.size())
		return false;
	map<int,double>::const_iterator it,it2;
	for(it=lhs.frequency.begin();it!=lhs.frequency.end();it++)
	{
		if(rhs.frequency.count(it->first)==0)
			return false;
		it2=rhs.frequency.find(it->first);
		if(abs(it->second-it2->second)>eps)
			return false;
	}
	return true;
}
bool operator!=(const Center &lhs,const Center &rhs)
{
	return !(lhs==rhs);
}
void Print(char str[],Document &doc,Document &query)
{
	char filename[50];
	strcpy(filename,str);
	strcat(filename,"oddcase");
	FILE *fptr=fopen(filename,"a");
	double dot=0;
	for(map<int,double>::iterator it=query.weight.begin();it!=query.weight.end();it++)
	{
		fprintf(fptr,"%d %f %f %f\n",it->first,it->second,doc.weight[it->first],it->second*doc.weight[it->first]);
		dot+=it->second*doc.weight[it->first];
	}
	fprintf(fptr,"dot : %f query length : %f document length : %f\n",dot,query.length,doc.length);
	fprintf(fptr,"===========================================\n");
	fclose(fptr);
}
void readDocument(const char dirname[],Document *document,map<int,int> &appear,int &doccnt)
{
	DIR *dir=opendir(dirname);
	struct dirent *entry;
	while((entry=readdir(dir))!=NULL)
	{
		if(entry->d_type==DT_REG)
		{
			strcpy(document[doccnt].filename,entry->d_name);
			int word;
			char str[100];
			strcpy(str,dirname);
			strcat(str,"/");
			strcat(str,entry->d_name);
			FILE *fptr=fopen(str,"r");
			if(fptr==NULL)
			{
				printf("open fail\n");
				continue;
			}
			for(int i=0;i<3;i++)
			{
				char tmpstr[100];
				fgets(tmpstr,100,fptr);
			}
			while(fscanf(fptr,"%d",&word)==1)
			{
				if(word!=-1)
				{
					if(document[doccnt].frequency.count(word)==0)
						document[doccnt].frequency[word]=1;
					else
						document[doccnt].frequency[word]+=1;
				}
			}
			for(map<int,int>::iterator iter=document[doccnt].frequency.begin();iter!=document[doccnt].frequency.end();iter++)
			{
				if(appear.count(iter->first)==0)
					appear[iter->first]=1;
				else
					appear[iter->first]+=1;
			}
			doccnt++;
			fclose(fptr);			
		}
	}
}
void readQuery(const char queryfile[],Document &query)
{
	int word;
	FILE *fptr=fopen(queryfile,"r");
	if(fptr==NULL)
	{
		printf("open fail\n");
	}
	while(fscanf(fptr,"%d",&word)==1)
	{
		if(word!=-1)
		{
			if(query.frequency.count(word)==0)
				query.frequency[word]=1;
			else
				query.frequency[word]++;
		}
	}
	fclose(fptr);
}
double Distance(Document &lhs,Center &rhs)
{
	double length1=lhs.Length(),length2=rhs.Length(),distance;
	distance=length1*length1+length2*length2;
	map<int,int>::iterator it;
	for(it=lhs.frequency.begin();it!=lhs.frequency.end();it++)
	{
		if(rhs.frequency.count(it->first)!=0)
			distance-=2*(it->second)*(rhs.frequency[it->first]);
	}
	return sqrt(distance);
}
void copyFrequency(Center &center,Document &document)
{
	map<int,int>::iterator it;
	for(it=document.frequency.begin();it!=document.frequency.end();it++)
		center.frequency[it->first]=(double)it->second;
}
void calculateNewCenter(Document *document,int doccnt,vector<Center> &center,int numofcluster,vector<Center> &newcenter)
{
	for(int i=0;i<numofcluster;i++)
	{
		Center tmp;
		int membercnt=0;
		for(int j=0;j<doccnt;j++)
		{
			if(document[j].clusterid==i)
			{
				map<int,int>::iterator it;
				for(it=document[j].frequency.begin();it!=document[j].frequency.end();it++)
				{
					if(tmp.frequency.count(it->first)==0)
						tmp.frequency[it->first]=(double)it->second;
					else
						tmp.frequency[it->first]+=(double)it->second;
				}
				membercnt++;
			}
		}
		map<int,double>::iterator it;
		for(it=tmp.frequency.begin();it!=tmp.frequency.end();it++)
		{
			it->second/=(double)membercnt;
		}
		newcenter.push_back(tmp);
	}
}
void kmeans(Document document[],int doccnt,int numofcluster)
{
	vector<Center>center;
	set<int>centerindex;
	centerindex.clear();
	while((int)center.size()<numofcluster)
	{
		int index=rand()%doccnt;
		if(centerindex.count(index)!=0)
			continue;
		else
			centerindex.insert(index);
		Center tmpcenter;
		copyFrequency(tmpcenter,document[index]);
		center.push_back(tmpcenter);
	}
	int sameresult=0,time=0;
	do
	{
		int membercnt[numofcluster]={};
		for(int i=0;i<doccnt;i++)
		{
			document[i].distance=Distance(document[i],center[0]);
			document[i].clusterid=0;
			for(int j=1;j<numofcluster;j++)
			{
				double tmpdis=Distance(document[i],center[j]);
				if(tmpdis<document[i].distance)
				{
					document[i].distance=tmpdis;
					document[i].clusterid=j;
				}
			}
			membercnt[document[i].clusterid]++;
		}
		printf("finish clustering\n");
		for(int i=0;i<numofcluster;i++)
		{
			if(membercnt[i]==0)
			{
				int index=rand()%doccnt;
				membercnt[document[index].clusterid]--;
				document[index].clusterid=i;
				membercnt[i]++;
				document[index].distance=Distance(document[index],center[i]);
			}
		}
		time++;
		printf("iteration %d\n",time);
		vector<Center> newcenter;
		newcenter.clear();
		calculateNewCenter(document,doccnt,center,numofcluster,newcenter);
		for(int i=0;i<numofcluster;i++)
		{
			printf("%f %f\n",center[i].Length(),newcenter[i].Length());
		}
		printf("===================================================\n");
		bool flag=true;
		for(int i=0;i<numofcluster;i++)
		{
			if(center[i]!=newcenter[i])
				flag=false;
			center[i]=newcenter[i];
		}
		if(flag)
			sameresult++;
		else
			sameresult=0;
	}while(sameresult<10);
	string name[4]={"cluster1.txt","cluster2.txt","cluster3.txt","cluster4.txt"};
	for(int i=0;i<4;i++)
	{
		FILE *fptr=fopen(name[i].c_str(),"w");
		for(int j=0;j<doccnt;j++)
		{
			if(document[j].clusterid==i)
				fprintf(fptr,"%s\n",document[j].filename);
		}
		fclose(fptr);
	}
}
void parsepzdString(int &topicid,char filename[],string str)
{
	int ptr=0;
	while(str[ptr]<'0'||str[ptr]>'9')
		ptr++;
	while(str[ptr]>='0'&&str[ptr]<='9')
	{
		topicid*=10;
		topicid+=str[ptr]-'0';
		ptr++;
	}
	while(str[ptr]!='|')
		ptr++;
	ptr++;
	int cnt=0,stop=(int)str.size();
	while(ptr<stop)
	{
		filename[cnt]=str[ptr];
		cnt++;
		ptr++;
	}
}
string pzdtoString(int topic,char filename[])
{
	string str;
	stack<int> s;
	do
	{
		s.push(topic%10);
		topic/=10;
	}while(topic!=0);
	while(!s.empty())
	{
		str+=(s.top()+'0');
		s.pop();
	}
	str+="|";
	str+=filename;
	return str;
}
void parsepwzString(int &wordid,int &topicid,string str)
{
	int ptr=0;
	while(str[ptr]<'0'||str[ptr]>'9')
		ptr++;
	while(str[ptr]>='0'&&str[ptr]<='9')
	{
		wordid*=10;
		wordid+=str[ptr]-'0';
		ptr++;
	}
	while(str[ptr]<'0'||str[ptr]>'9')
		ptr++;
	while(str[ptr]>='0'&&str[ptr]<='9')
	{
		topicid*=10;
		topicid+=str[ptr]-'0';
		ptr++;
	}
}
string pwztoString(int a,int b)
{
	stack<int> tmp;
	string str;
	do
	{
		tmp.push(a%10);
		a/=10;
	}while(a!=0);
	while(!tmp.empty())
	{
		str+=(tmp.top()+'0');
		tmp.pop();
	}
	str+="|";
	do
	{
		tmp.push(b%10);
		b/=10;
	}while(b!=0);
	while(!tmp.empty())
	{
		str+=(tmp.top()+'0');
		tmp.pop();
	}
	return str;
}
string pzdwtoString(int topicindex,char filename[],int wordid)
{
	stack<int> tmp;
	string str;
	do
	{
		tmp.push(topicindex%10);
		topicindex/=10;
	}while(topicindex!=0);
	while(!tmp.empty())
	{
		str+=(tmp.top()+'0');
		tmp.pop();
	}
	str+="|";
	str+=filename;
	str+="|";
	do
	{
		tmp.push(wordid%10);
		wordid/=10;
	}while(wordid!=0);
	while(!tmp.empty())
	{
		str+=(tmp.top()+'0');
		tmp.pop();
	}
	return str;

}
void buildTopicDic(Document *document,int doccnt,map<int,int> topicdic[],int numofcluster)
{
	for(int i=0;i<numofcluster;i++)
		topicdic[i][-1]=0;
	for(int i=0;i<doccnt;i++)
	{
		map<int,int>::iterator it;
		int group=document[i].clusterid;
		for(it=document[i].frequency.begin();it!=document[i].frequency.end();it++)
		{
			if(topicdic[group].count(it->first)==0)
				topicdic[group][it->first]=it->second;
			else
				topicdic[group][it->first]+=it->second;
			topicdic[group][-1]+=it->second;	
		}
	}
}
void Initialize(map<string,double> &pzd,map<string,double> &pwz,map<string,double> &pzdw,map<int,int> topicdic[],int numofcluster,Document *document,int doccnt)
{
	pzd.clear();
	pwz.clear();
	pzdw.clear();
	for(int i=0;i<numofcluster;i++)
	{
		map<int,int>::iterator it;
		for(it=topicdic[i].begin();it!=topicdic[i].end();it++)
		{
			if(it->first!=-1)
				pwz[pwztoString(it->first,i)]=(double)(it->second)/topicdic[i][-1];
		}
	}
	for(int i=0;i<doccnt;i++)
	{
		for(int j=0;j<numofcluster;j++)
			pzd[pzdtoString(j,document[i].filename)]=1.0/numofcluster;
	}
	FILE *fptr=fopen("topicdictionary.txt","w");
	for(int i=0;i<numofcluster;i++)
	{
		map<string,double>::iterator it;
		for(it=pwz.begin();it!=pwz.end();it++)
		{
			fprintf(fptr,"%s %f\n",it->first.c_str(),it->second);
		}
	}
	fclose(fptr);
	fptr=fopen("pzd.txt","w");
	map<string,double>::iterator it;
	for(it=pzd.begin();it!=pzd.end();it++)
	{
		fprintf(fptr,"%s %f\n",it->first.c_str(),it->second);
	}
	fclose(fptr);
}
void Training(map<string,double> pzd,map<string,double> pwz,map<string,double> pzdw,map<int,int> topicdic[],int numofcluster,Document *document,int doccnt)
{
	map<string,double>::iterator it;
	for(it=pwz.begin();it!=pwz.end();it++)
	{
		int wordindex,topicindex;
		parsepwzString(wordindex,topicindex,it->first);
		for(int i=0;i<1000;i++)
		{
			pzdw[pzdwtoString(topicindex,document[i].filename,wordindex)]=it->second*pzd[pzdtoString(topicindex,document[i].filename)];
		}
	}
}
int main()
{
	srand(time(NULL));
	Document document[3000];
	map<int,int> appear;
	int doccnt=0,numofcluster=4;
	readDocument("./Document",document,appear,doccnt);
	for(int i=0;i<doccnt;i++)
	{
		map<int,int>::iterator iter;
		double doclength=0;
		for(iter=document[i].frequency.begin();iter!=document[i].frequency.end();iter++)
		{
			doclength+=document[i].frequency[iter->first];
		}
		document[i].length=doclength;
	}
	kmeans(document,doccnt,numofcluster);
	map<string,double> pzd,pwz,pzdw;
	map<int,int> topicdic[numofcluster];
	buildTopicDic(document,doccnt,topicdic,numofcluster);
	Initialize(pzd,pwz,pzdw,topicdic,numofcluster,document,doccnt);
	Training(pzd,pwz,pzdw,topicdic,numofcluster,document,doccnt);
	/*FILE *ansptr=fopen("result.txt","w");
	DIR *querydir=opendir("./shortquery");
	struct dirent *queryentry;
	int querycnt=0;
	vector<string>filename;
	while((queryentry=readdir(querydir))!=NULL)
	{
		if(queryentry->d_type==DT_REG)
			filename.push_back(queryentry->d_name);
	}
	sort(filename.begin(),filename.end());
	int totalquery=(int)filename.size();
	for(int t=0;t<totalquery;t++)
	{
		Document query;
		char str[50]={'.','/','s','h','o','r','t','q','u','e','r','y','/'};
		strcat(str,filename[t].c_str());
		readQuery(str,query);
		sort(document,document+doccnt);
		querycnt++;
		fprintf(ansptr,"Query %d %s %d\n",querycnt,filename[t].c_str(),doccnt);
		for(int i=0;i<doccnt;i++)
		{
			fprintf(ansptr,"%s %f\n",document[i].filename,document[i].similarity);
			if(document[i].similarity>1)
			{
				Print(queryentry->d_name,document[i],query);
			}
		}
		query.frequency.clear();
		query.weight.clear();
		query.length=query.similarity=0;	
	}
	fclose(ansptr);*/
	exit(0);
}
