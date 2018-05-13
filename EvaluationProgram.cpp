#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<string>
#include<set>
#include<map>
#include<math.h>
using namespace std;
void fillup(double precisiontable[][11],int docid,double precision,int recall)
{
	for(int i=0;i<=10;i++)
	{
		if(i*10>recall)
			break;
		else
			precisiontable[docid][i]=max(precision,precisiontable[docid][i]);
	}
}
int main()
{
	char qname[100];
	int reldoccnt[16];
	set<string> reldoc[16];
	FILE *fptr=fopen("./assessment.txt","r");
	for(int i=0;i<16;i++)
	{
		fscanf(fptr,"%s %s %s%d",qname,qname,qname,&reldoccnt[i]);
		char docname[100];
		for(int j=0;j<reldoccnt[i];j++)
		{
			fscanf(fptr," %s",docname);
			reldoc[i].insert(string(docname));
		}
	}
	fclose(fptr);
	double precisiontable[16][11];
	int doccnt;
	for(int i=0;i<16;i++)
		for(int j=0;j<11;j++)
			precisiontable[i][j]=-1;
	fptr=fopen("result.txt","r");
	map<string,double> precisionmap[16];
	double cgavg[2500]={},icgavg[2500]={},ndcg[2500]={};
	for(int i=0;i<16;i++)
	{
		fscanf(fptr,"%s %s %s%d",qname,qname,qname,&doccnt);
		char docname[100],score[100];
		int relcnt=0;
		double gain[2500]={},ig[2500]={},cg[2500]={},icg[2500]={};
		for(int j=0;j<doccnt;j++)
		{
			fscanf(fptr," %s %s",docname,score);
			string docnamestr(docname);
			if(reldoc[i].count(docnamestr)==1)
			{
				relcnt++;
				int recall=relcnt*100/reldoc[i].size();
				double precision=(double)relcnt*100/(j+1);
				precisionmap[i][docnamestr]=precision;
				fillup(precisiontable,i,precision,recall);
				gain[j]=1;
			}
			if(j<(int)reldoc[i].size())
				ig[j]=1;
			if(j+1>1)
			{
				ig[j]/=log2(j+1);
				gain[j]/=log2(j+1);
			}
		}
		cg[0]=gain[0];
		icg[0]=ig[0];
		for(int j=1;j<doccnt;j++)
		{
			cg[j]+=gain[j]+cg[j-1];
			icg[j]+=ig[j]+icg[j-1];
		}
		for(int j=0;j<doccnt;j++)
		{
			cgavg[j]+=cg[j];
			icgavg[j]+=icg[j];
		}
	}
	for(int i=0;i<doccnt;i++)
	{
		cgavg[i]/=16;
		icgavg[i]/=16;
		ndcg[i]=cgavg[i]/icgavg[i];
	}
	fclose(fptr);
	fptr=fopen("Interpolated-Recall-Precision-curve.txt","w");	
	/*for(int i=0;i<16;i++)
	{
		fprintf(fptr,"Query %d:\n",i+1);
		for(int j=0;j<11;j++)
		{
			fprintf(fptr,"%f\n",precisiontable[i][j]);
		}
		fprintf(fptr,"===================================\n");
	}*/
	double avg[11]={};
	for(int i=0;i<=10;i++)
	{
		for(int j=0;j<16;j++)
			avg[i]+=precisiontable[j][i];	
		avg[i]/=16;
	}
	for(int i=0;i<=10;i++)
	{
		fprintf(fptr,"%f ",avg[i]);
	}
	fclose(fptr);
	double MAP[16]={},totalmap=0;
	fptr=fopen("Mean-Average-Precision.txt","w");
	for(int i=0;i<16;i++)
	{
		for(set<string>::iterator it=reldoc[i].begin();it!=reldoc[i].end();it++)
		{
			string docname=*it;
			if(precisionmap[i].count(docname)==1)
				MAP[i]+=precisionmap[i][docname];
		}
		MAP[i]/=reldoc[i].size();
		fprintf(fptr,"Query %d : %f\n",i+1,MAP[i]);
	}
	for(int i=0;i<16;i++)
		totalmap+=MAP[i];
	totalmap/=16;
	fprintf(fptr,"Total MAP : %f\n",totalmap);
	fclose(fptr);
	fptr=fopen("NDCG.txt","w");
	for(int i=0;i<doccnt;i++)
	{
		fprintf(fptr,"%f ",ndcg[i]);
	}
	fclose(fptr);
}
