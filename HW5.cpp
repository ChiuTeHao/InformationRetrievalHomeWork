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
	double length=0;
	double similarity;
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
	Document()
	{
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
}
string integertoString(int num)
{
	stack<int>s;
	string str;
	do
	{
		s.push(num%10);
		num/=10;
	}while(num!=0);
	while(!s.empty())
	{
		str+=('0'+s.top());
		s.pop();
	}
	return str;
}
int getInteger(string str,int &ptr)
{
	while(str[ptr]<'0'||str[ptr]>'9')
		ptr++;
	int x=0;
	while(str[ptr]>='0'&&str[ptr]<='9')
	{
		x*=10;
		x+=str[ptr]-'0';
		ptr++;
	}
	return x;	
}
void parsepzdString(int &topicid,int &docid,string str)
{
	int ptr=0;
	topicid=getInteger(str,ptr);
	docid=getInteger(str,ptr);
}
string pzdtoString(int topic,int docid)
{
	string str;
	str+=integertoString(topic);
	str+="|";
	str+=integertoString(docid);
	return str;
}
void parsepwzString(int &wordid,int &topicid,string str)
{
	int ptr=0;
	wordid=getInteger(str,ptr);
	topicid=getInteger(str,ptr);
}
string pwztoString(int wordid,int topicid)
{
	string str;
	str+=integertoString(wordid);
	str+="|";
	str+=integertoString(topicid);
	return str;
}
void parsepzdwString(int &topicindex,int &docid,int &wordid,string str)
{
	int ptr=0;
	topicindex=getInteger(str,ptr);
	docid=getInteger(str,ptr);
	wordid=getInteger(str,ptr);
}
string pzdwtoString(int topicindex,int docid,int wordid)
{
	string str;
	str+=integertoString(topicindex);
	str+="|";
	str+=integertoString(docid);
	str+="|";
	str+=integertoString(wordid);
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
			pzd[pzdtoString(j,i)]=1.0/numofcluster;
	}
}
void Training(map<string,double> &pzd,map<string,double> &pwz,map<string,double> &pzdw,map<int,int> topicdic[],int numofcluster,Document *document,int doccnt)
{
	int frequencytotal[doccnt]={};
	for(int i=0;i<doccnt;i++)
	{
		for(map<int,int>::iterator it=document[i].frequency.begin();it!=document[i].frequency.end();it++)
		{
			frequencytotal[i]+=it->second;
		}
	}
	for(int t=0;t<10;t++)
	{
		printf("iteration %d\n",t);
		map<string,double> denopzdw;
		for(map<string,double>::iterator it=pwz.begin();it!=pwz.end();it++)
		{
			for(int i=0;i<doccnt;i++)
			{
				int wordindex=0,topicindex=0;
				parsepwzString(wordindex,topicindex,it->first);
				if(document[i].frequency.count(wordindex)!=0)
				{
					//printf("wordid:%d topicindex:%d docid:%d\n",wordindex,topicindex,i);
					string pzdwindex=pzdwtoString(topicindex,i,wordindex);
					pzdw[pzdwindex]=it->second*pzd[pzdtoString(topicindex,i)];
					string denopzdwindex=pwztoString(wordindex,topicindex);
					if(denopzdw.count(denopzdwindex)==0)
					denopzdw[denopzdwindex]=pzdw[pzdwindex];
					else
						denopzdw[denopzdwindex]+=pzdw[pzdwindex];
				}
				else
					continue;
			}
		}
		for(map<string,double>::iterator it=pzdw.begin();it!=pzdw.end();it++)
		{
			int topicindex=0,docindex=0,wordindex=0;
			parsepzdwString(topicindex,docindex,wordindex,it->first);
			it->second/=denopzdw[pwztoString(wordindex,topicindex)];
		}
		double topicsum[4]={};
		pwz.clear();
		pzd.clear();
		for(map<string,double>::iterator it=pzdw.begin();it!=pzdw.end();it++)
		{
			string pzdwindex=it->first;
			int wordindex=0,topicindex=0,docindex=0;
			parsepzdwString(topicindex,docindex,wordindex,pzdwindex);
			string pwzindex=pwztoString(wordindex,topicindex);
			if(pwz.count(pwzindex)==0)
				pwz[pwzindex]=document[docindex].frequency[wordindex]*pzdw[pzdwindex];
			else
				pwz[pwzindex]+=document[docindex].frequency[wordindex]*pzdw[pzdwindex];
			topicsum[topicindex]+=document[docindex].frequency[wordindex]*pzdw[pzdwindex];
		}
		for(map<string,double>::iterator it=pwz.begin();it!=pwz.end();it++)
		{
			int wordindex=0,topicindex=0;
			parsepwzString(wordindex,topicindex,it->first);
			it->second/=topicsum[topicindex];
		}
		for(map<string,double>::iterator it=pzdw.begin();it!=pzdw.end();it++)
		{
			string pzdwindex=it->first;
			int wordindex=0,topicindex=0,docindex=0;
			parsepzdwString(topicindex,docindex,wordindex,pzdwindex);
			string pzdindex=pzdtoString(topicindex,docindex);
			if(pzd.count(pzdindex)==0)
				pzd[pzdindex]=document[docindex].frequency[wordindex]*pzdw[pzdwindex];
			else
				pzd[pzdindex]+=document[docindex].frequency[wordindex]*pzdw[pzdwindex];	
		}
		for(map<string,double>::iterator it=pzd.begin();it!=pzd.end();it++)
		{
			string pzdindex=it->first;
			int topicindex=0,docindex=0;
			parsepzdString(topicindex,docindex,pzdindex);
			it->second/=frequencytotal[docindex];
		}
		printf("pwz size:%d pzd size:%d pzdw size:%d\n",(int)pwz.size(),(int)pzd.size(),(int)pzdw.size());
	}
	FILE *fptr=fopen("pwz.txt","w");
	for(map<string,double>::iterator it=pwz.begin();it!=pwz.end();it++)
	{
		int wordindex=0,topicindex=0;
		parsepwzString(wordindex,topicindex,it->first);
		fprintf(fptr,"wordindex : %d topicindex : %d %f\n",wordindex,topicindex,it->second);
	}
	fclose(fptr);
	fptr=fopen("pzd.txt","w");
	for(map<string,double>::iterator it=pzd.begin();it!=pzd.end();it++)
	{
		int topicindex=0,docindex=0;
		parsepzdString(topicindex,docindex,it->first);
		fprintf(fptr,"topicindex : %d docindex: %d %f\n",topicindex,docindex,it->second);
	}
	fclose(fptr);
	fptr=fopen("pzdw.txt","w");
	for(map<string,double>::iterator it=pzdw.begin();it!=pzdw.end();it++)
	{
		int wordindex=0,topicindex=0,docindex=0;
		parsepzdwString(topicindex,docindex,wordindex,it->first);
		fprintf(fptr,"topicindex : %d docindex : %d wordindex : %d %f\n",topicindex,docindex,wordindex,it->second);
	}
	fclose(fptr);
	return;
}
void readBackGroundModel(map<int,double> &model)
{
	FILE *fptr=fopen("Word_Unigram_Xinhua98Upper.txt","r");
	int id;
	double prob,totalprob=0;
	double delta=1e-11;
	while(fscanf(fptr,"%d %lf",&id,&prob)==2)
	{
		model[id]=prob;
		totalprob+=prob+delta;
	}
	for(map<int,double>::iterator it=model.begin();it!=model.end();it++)
	{
		model[it->first]=(it->second+delta)/totalprob;
	}
	fclose(fptr);
}
void calculateRank(Document &query,Document document[],int doccnt,map<int,double> &backgroundmodel,map<string,double> &pwz,map<string,double> &pzd,double alpha,double beta,int numofcluster)
{
	for(int i=0;i<doccnt;i++)
	{
		int wordindex;
		double prob=1;
		for(map<int,int>::iterator it=query.frequency.begin();it!=query.frequency.end();it++)
		{
			wordindex=it->first;
			double plsaprob=0;
			for(int j=0;j<numofcluster;j++)
			{
				string pwzindex=pwztoString(wordindex,j),pzdindex=pzdtoString(j,i);
				if(pwz.count(pwzindex)!=0&&pzd.count(pzdindex)!=0)
					plsaprob=pwz[pwzindex]*pzd[pzdindex];
			}
			if(query.frequency.count(wordindex)==0)
			{
				prob*=(beta*backgroundmodel[wordindex]+(1-alpha-beta)*plsaprob);
			}
			else
			{
				prob*=((alpha*document[i].frequency[wordindex]/document[i].length)+(beta)*backgroundmodel[wordindex]+(1-alpha-beta)*plsaprob);
			}
		}
		document[i].similarity=prob;
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
	map<int,double> backgroundmodel;
	readBackGroundModel(backgroundmodel);
	FILE *ansptr=fopen("result.txt","w");
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
		calculateRank(query,document,doccnt,backgroundmodel,pwz,pzd,0.2,0.4,numofcluster);
		sort(document,document+doccnt);
		querycnt++;
		fprintf(ansptr,"Query %d %s %d\n",querycnt,filename[t].c_str(),doccnt);
		for(int i=0;i<doccnt;i++)
		{
			fprintf(ansptr,"%s %f\n",document[i].filename,document[i].similarity);
		}
		query.frequency.clear();
		query.weight.clear();
		query.length=0;
		query.similarity=0;	
	}
	fclose(ansptr);
	exit(0);
}
