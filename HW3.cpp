#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<dirent.h>
#include<map>
#include<math.h>
#include<string>
#include<algorithm>
#include<string.h>
#include<vector>
using namespace std;
struct Document
{
	char filename[50];
	map<int,int> frequency;
	map<int,double> weight;
	double length=0,similarity=0;
	bool operator<(const Document &rhs)
	{
		return similarity>rhs.similarity;
	}
};
struct Occurrence
{
	vector<int> occurrence;
};
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
struct Cluster
{
	int wordid;
	double similarity;
	bool operator<(const Cluster &rhs)
	{
		if(similarity!=rhs.similarity)
			return similarity>rhs.similarity;
		else
			return wordid<rhs.wordid;
	}
};
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
void calculateTermWeightandLength(Document &query,map<int,int> &appear,int doccnt)
{
	map<int,int>::iterator iter;
	for(iter=query.frequency.begin();iter!=query.frequency.end();iter++)
	{
		if(appear[iter->first]!=0)
		{
			query.weight[iter->first]=(1+log2((double)iter->second))*log2((double)doccnt/(double)appear[iter->first]);
		}
		else
		{
			query.weight[iter->first]=0;
		}
		query.length+=query.weight[iter->first]*query.weight[iter->first];
	}
	query.length=sqrt(query.length);
}
void calculateSimilarity(Document &query,Document *document,int doccnt)
{
	for(int i=0;i<doccnt;i++)
	{
		map<int,double>::iterator iter;
		document[i].similarity=0;
		for(iter=document[i].weight.begin();iter!=document[i].weight.end();iter++)
		{
			int word=iter->first;
			if(query.weight.count(word)!=0)
			{
				document[i].similarity+=iter->second*query.weight[word];
			}
		}
		if(document[i].length!=0&&query.length!=0)
		{
			document[i].similarity/=(document[i].length*query.length);
		}
		else
		{
			document[i].similarity=0;
		}
	}
}
void buildDictionary(Document document[],int doccnt,map<int,Occurrence> &dictionary)
{
	for(int i=0;i<doccnt;i++)
	{
		for(map<int,int>::iterator it=document[i].frequency.begin();it!=document[i].frequency.end();it++)
		{
			int word=it->first;
			if(dictionary.count(word)==0)
			{
				Occurrence tmp;
				for(int j=0;j<doccnt;j++)
				{
					if(document[i].frequency.count(word)==0)
					{
						tmp.occurrence.push_back(0);
					}
					else
					{
						tmp.occurrence.push_back(document[i].frequency[word]);
					}
				}
				dictionary[word]=tmp;
			}
		}
	}
}
void modifyQuery(Document &query,Document document[],int n)
{
	double alpha=0.47,beta=0.53,gama=0.0;
	for(map<int,double>::iterator it=query.weight.begin();it!=query.weight.end();it++)
	{
		it->second*=alpha;
	}
	for(int i=0;i<n;i++)
	{
		for(map<int,double>::iterator it=document[i].weight.begin();it!=document[i].weight.end();it++)
		{
			if(query.weight.count(it->first)==0)
			{
				query.weight[it->first]=beta*document[i].weight[it->first];
			}
			else
			{
				query.weight[it->first]+=beta*document[i].weight[it->first];
			}
		}
	}
	for(map<int,double>::iterator it=document[n].weight.begin();it!=document[n].weight.end();it++)
	{
		if(query.weight.count(it->first)==0)
		{
			query.weight[it->first]=(-1)*gama*it->second;
		}
		else
		{
			query.weight[it->first]-=gama*it->second;
		}
	}
	double len=0;
	for(map<int,double>::iterator it=query.weight.begin();it!=query.weight.end();it++)
	{
		len+=it->second*it->second;
	}
	query.length=sqrt(len);
}
int main()
{

	Document document[3000];
	map<int,int> appear;
	int doccnt=0;
	readDocument("./Document",document,appear,doccnt);
	for(int i=0;i<doccnt;i++)
	{
		map<int,int>::iterator iter;
		double doclength=0;
		for(iter=document[i].frequency.begin();iter!=document[i].frequency.end();iter++)
		{
			double word=(double)iter->first;
			double f=(double)iter->second;
			double n=(double)appear[word];
			document[i].weight[word]=(1+log2(f))*log2(doccnt/n);
			doclength+=document[i].weight[word]*document[i].weight[word];
		}
		doclength=sqrt(doclength);
		document[i].length=doclength;
	}
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
	map<int,Occurrence> dictionary;
	for(int t=0;t<totalquery;t++)
	{
		Document query;
		char str[50]={'.','/','s','h','o','r','t','q','u','e','r','y','/'};
		strcat(str,filename[t].c_str());
		readQuery(str,query);
		calculateTermWeightandLength(query,appear,doccnt);
		calculateSimilarity(query,document,doccnt);
		sort(document,document+doccnt);
		modifyQuery(query,document,6);
		calculateSimilarity(query,document,doccnt);
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
	fclose(ansptr);
	exit(0);
}
