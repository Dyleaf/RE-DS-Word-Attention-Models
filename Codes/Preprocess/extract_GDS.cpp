#include <cstring>
#include <cstdlib>
#include <vector>
#include <map>
#include <string>
#include <cstdio>
#include <float.h>
#include <cmath>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <iostream>

using namespace std;
int output_model = 0;

string version = "";

float wt = 0.5;

int num_threads = 12;
int trainTimes = 15;
float alpha = 0.03;
float reduce = 0.98;
int tt,tt1;
int dimensionC = 230;//1000;
int dimensionWPE = 5;//25;
int window = 3;
int limit = 30;
float marginPositive = 2.5;
float marginNegative = 0.5;
float margin = 2;
float Belt = 0.001;
float *matrixB1, *matrixRelation, *matrixW1, *matrixRelationDao, *matrixRelationPr, *matrixRelationPrDao;
float *matrixB1_egs, *matrixRelation_egs, *matrixW1_egs, *matrixRelationPr_egs;
float *matrixB1_exs, *matrixRelation_exs, *matrixW1_exs, *matrixRelationPr_exs;
float *wordVecDao,*wordVec_egs,*wordVec_exs;
float *positionVecE1, *positionVecE2, *matrixW1PositionE1, *matrixW1PositionE2;
float *positionVecE1_egs, *positionVecE2_egs, *matrixW1PositionE1_egs, *matrixW1PositionE2_egs, *positionVecE1_exs, *positionVecE2_exs, *matrixW1PositionE1_exs, *matrixW1PositionE2_exs;
float *matrixW1PositionE1Dao;
float *matrixW1PositionE2Dao;
float *positionVecDaoE1;
float *positionVecDaoE2;
float *matrixW1Dao;
float *matrixB1Dao;
double mx = 0;
int batch = 16;
int npoch;
int len;
float rate = 1;
FILE *logg;

float *wordVec;
long long wordTotal, dimension, relationTotal;
long long PositionMinE1, PositionMaxE1, PositionTotalE1,PositionMinE2, PositionMaxE2, PositionTotalE2;
map<string,int> wordMapping;
vector<string> wordList;
map<string,int> relationMapping;
vector<int *> trainLists, trainPositionE1, trainPositionE2;
vector<int> trainLength;
vector<int> headList, tailList, relationList, ldist, rdist,testldist, testrdist, devldist, devrdist;
vector<int *> testtrainLists, testPositionE1, testPositionE2;
vector<int *> devtrainLists, devPositionE1, devPositionE2;
vector<int> testtrainLength, devtrainLength;
vector<int> testheadList, testtailList, testrelationList;
vector<int> devheadList, devtailList, devrelationList;
vector<string> testheadLists, testtailLists, testrelationLists;
vector<string> devheadLists, devtailLists, devrelationLists;
vector<std::string> nam;

map<string,vector<int> > bags_train, bags_test, bags_dev;

void init() {
  

	FILE *f = fopen("../../Data/GIDS/vec.bin", "rb");
	fscanf(f, "%lld", &wordTotal);
	fscanf(f, "%lld", &dimension);
	cout<<"wordTotal=\t"<<wordTotal<<endl;
	cout<<"Word dimension=\t"<<dimension<<endl;

	PositionMinE1 = 0;
	PositionMaxE1 = 0;
	PositionMinE2 = 0;
	PositionMaxE2 = 0;
	wordVec = (float *)malloc((wordTotal+1) * dimension * sizeof(float));
	wordList.resize(wordTotal+1);
	wordList[0] = "UNK";
	for (int b = 1; b <= wordTotal; b++) {
		string name = "";
		while (1) {
			char ch = fgetc(f);
			if (feof(f) || ch == ' ') break;
			if (ch != '\n') name = name + ch;
		}
		long long last = b * dimension;
		float smp = 0;
		for (int a = 0; a < dimension; a++) {
			fread(&wordVec[a + last], sizeof(float), 1, f);
			smp += wordVec[a + last]*wordVec[a + last];
		}
		smp = sqrt(smp);
		for (int a = 0; a< dimension; a++)
			wordVec[a+last] = wordVec[a+last] / smp;
		wordMapping[name] = b;
		wordList[b] = name;
	}
	wordTotal+=1;
	fclose(f);

	ofstream fout;
  	fout.open("word2vec.txt",ios::out);
	for (int i = 0; i < wordTotal; i++)
	{
		fout << wordList[i]<<"\t";
		for (int j=0; j<dimension; j++)
			fout << wordVec[i*dimension+j]<<",";
		fout <<"\n";
	}
	fout.close();
	char buffer[1000];
	f = fopen("../../Data/GIDS/relation2id.txt", "r");
	while (fscanf(f,"%s",buffer)==1) {
		int id;
		fscanf(f,"%d",&id);
		relationMapping[(string)(buffer)] = id;
		relationTotal++;
		nam.push_back((std::string)(buffer));
	}
	fclose(f);
	cout<<"relationTotal:\t"<<relationTotal<<endl;

      f = fopen("train_lower.tsv", "r");
    	while (fscanf(f,"%s",buffer)==1)  {
    		string e1 = buffer;
    		fscanf(f,"%s",buffer);
    		string e2 = buffer;
    		fscanf(f,"%s",buffer);
    		string head_s = (string)(buffer);
    		int head = wordMapping[(string)(buffer)];
    		fscanf(f,"%s",buffer);
    		int tail = wordMapping[(string)(buffer)];
    		string tail_s = (string)(buffer);
    		fscanf(f,"%s",buffer);
    		//cout << buffer;
    		//exit(0);
       //cout<<e1+"\t"+e2+"\t"+(string)(buffer);
    		bags_train[e1+"\t"+e2+"\t"+(string)(buffer)].push_back(headList.size());
    		int num = relationMapping[(string)(buffer)];
        int len = 0, lefnum = 0, rignum = 0;
    		std::vector<int> tmpp;
    		while (fscanf(f,"%s", buffer)==1) {
    			std::string con = buffer;
    			if (con=="###end###") break;
    			int gg = wordMapping[con];
    			if (con == head_s) lefnum = len;
    			if (con == tail_s) rignum = len;
    			len++;
    			tmpp.push_back(gg);
    		}
    		headList.push_back(head);
    		tailList.push_back(tail);
    		relationList.push_back(num);
    		trainLength.push_back(len);
    		ldist.push_back(lefnum);
    		rdist.push_back(rignum);
    		int *con=(int *)calloc(len,sizeof(int));
    		int *conl=(int *)calloc(len,sizeof(int));
    		int *conr=(int *)calloc(len,sizeof(int));
    		for (int i = 0; i < len; i++) {
    			con[i] = tmpp[i];
    			conl[i] = lefnum - i;
    			conr[i] = rignum - i;
    			if (conl[i] >= limit) conl[i] = limit;
    			if (conr[i] >= limit) conr[i] = limit;
    			if (conl[i] <= -limit) conl[i] = -limit;
    			if (conr[i] <= -limit) conr[i] = -limit;
    			if (conl[i] > PositionMaxE1) PositionMaxE1 = conl[i];
    			if (conr[i] > PositionMaxE2) PositionMaxE2 = conr[i];
    			if (conl[i] < PositionMinE1) PositionMinE1 = conl[i];
    			if (conr[i] < PositionMinE2) PositionMinE2 = conr[i];
    		}
    		trainLists.push_back(con);
    		trainPositionE1.push_back(conl);
    		trainPositionE2.push_back(conr);
    	}
    	fclose(f);

    	f = fopen("test_lower.tsv", "r");	
    	while (fscanf(f,"%s",buffer)==1)  {
    		string e1 = buffer;
    		fscanf(f,"%s",buffer);
    		string e2 = buffer;
    		bags_test[e1+"\t"+e2].push_back(testheadList.size());
    		fscanf(f,"%s",buffer);
    		string head_s = (string)(buffer);
    		int head = wordMapping[(string)(buffer)];
    		fscanf(f,"%s",buffer);
    		string tail_s = (string)(buffer);
    		int tail = wordMapping[(string)(buffer)];
    		fscanf(f,"%s",buffer);
    		int num = relationMapping[(string)(buffer)];
    		int len = 0 , lefnum = 0, rignum = 0;
    		std::vector<int> tmpp;
    		while (fscanf(f,"%s", buffer)==1) {
    			std::string con = buffer;
    			if (con=="###end###") break;
    			int gg = wordMapping[con];
    			if (head_s == con) lefnum = len;
    			if (tail_s == con) rignum = len;
    			len++;
    			tmpp.push_back(gg);
    		}
    		testheadList.push_back(head);
    		testtailList.push_back(tail);
    		testheadLists.push_back(head_s);
    		testtailLists.push_back(tail_s);
    		testrelationList.push_back(num);
    		testtrainLength.push_back(len);
    		testldist.push_back(lefnum);
    		testrdist.push_back(rignum);
    		int *con=(int *)calloc(len,sizeof(int));
    		int *conl=(int *)calloc(len,sizeof(int));
    		int *conr=(int *)calloc(len,sizeof(int));
    		for (int i = 0; i < len; i++) {
    			con[i] = tmpp[i];
    			conl[i] = lefnum - i;
    			conr[i] = rignum - i;
    			if (conl[i] >= limit) conl[i] = limit;
    			if (conr[i] >= limit) conr[i] = limit;
    			if (conl[i] <= -limit) conl[i] = -limit;
    			if (conr[i] <= -limit) conr[i] = -limit;
    			if (conl[i] > PositionMaxE1) PositionMaxE1 = conl[i];
    			if (conr[i] > PositionMaxE2) PositionMaxE2 = conr[i];
    			if (conl[i] < PositionMinE1) PositionMinE1 = conl[i];
    			if (conr[i] < PositionMinE2) PositionMinE2 = conr[i];
    		}
    		testtrainLists.push_back(con);
    		testPositionE1.push_back(conl);
    		testPositionE2.push_back(conr);
    	}
    	fclose(f);
    	

        f = fopen("dev_lower.tsv", "r"); 
        while (fscanf(f,"%s",buffer)==1)  {
            string e1 = buffer;
            fscanf(f,"%s",buffer);
            string e2 = buffer;
            bags_dev[e1+"\t"+e2].push_back(devheadList.size());
            fscanf(f,"%s",buffer);
            string head_s = (string)(buffer);
            int head = wordMapping[(string)(buffer)];
            fscanf(f,"%s",buffer);
            string tail_s = (string)(buffer);
            int tail = wordMapping[(string)(buffer)];
            fscanf(f,"%s",buffer);
            int num = relationMapping[(string)(buffer)];
            int len = 0 , lefnum = 0, rignum = 0;
            std::vector<int> tmpp;
            while (fscanf(f,"%s", buffer)==1) {
                std::string con = buffer;
                if (con=="###end###") break;
                int gg = wordMapping[con];
                if (head_s == con) lefnum = len;
                if (tail_s == con) rignum = len;
                len++;
                tmpp.push_back(gg);
            }
            devheadList.push_back(head);
            devtailList.push_back(tail);
            devheadLists.push_back(head_s);
            devtailLists.push_back(tail_s);
            devrelationList.push_back(num);
            devtrainLength.push_back(len);
            devldist.push_back(lefnum);
            devrdist.push_back(rignum);
            int *con=(int *)calloc(len,sizeof(int));
            int *conl=(int *)calloc(len,sizeof(int));
            int *conr=(int *)calloc(len,sizeof(int));
            for (int i = 0; i < len; i++) {
                con[i] = tmpp[i];
                conl[i] = lefnum - i;
                conr[i] = rignum - i;
                if (conl[i] >= limit) conl[i] = limit;
                if (conr[i] >= limit) conr[i] = limit;
                if (conl[i] <= -limit) conl[i] = -limit;
                if (conr[i] <= -limit) conr[i] = -limit;
                if (conl[i] > PositionMaxE1) PositionMaxE1 = conl[i];
                if (conr[i] > PositionMaxE2) PositionMaxE2 = conr[i];
                if (conl[i] < PositionMinE1) PositionMinE1 = conl[i];
                if (conr[i] < PositionMinE2) PositionMinE2 = conr[i];
            }
            devtrainLists.push_back(con);
            devPositionE1.push_back(conl);
            devPositionE2.push_back(conr);
        }
        fclose(f);

        cout<<PositionMinE1<<' '<<PositionMaxE1<<' '<<PositionMinE2<<' '<<PositionMaxE2<<endl;

    	for (int i = 0; i < trainPositionE1.size(); i++) {
    		int len = trainLength[i];
    		int *work1 = trainPositionE1[i];
    		for (int j = 0; j < len; j++)
    			work1[j] = work1[j] - PositionMinE1;
    		int *work2 = trainPositionE2[i];
    		for (int j = 0; j < len; j++)
    			work2[j] = work2[j] - PositionMinE2;
    	}
    
    	for (int i = 0; i < testPositionE1.size(); i++) {
    		int len = testtrainLength[i];
    		int *work1 = testPositionE1[i];
    		for (int j = 0; j < len; j++)
    			work1[j] = work1[j] - PositionMinE1;
    		int *work2 = testPositionE2[i];
    		for (int j = 0; j < len; j++)
    			work2[j] = work2[j] - PositionMinE2;
    	}

        for (int i = 0; i < devPositionE1.size(); i++) {
            int len = devtrainLength[i];
            int *work1 = devPositionE1[i];
            for (int j = 0; j < len; j++)
                work1[j] = work1[j] - PositionMinE1;
            int *work2 = devPositionE2[i];
            for (int j = 0; j < len; j++)
                work2[j] = work2[j] - PositionMinE2;
        }

    	PositionTotalE1 = PositionMaxE1 - PositionMinE1 + 1;
    	PositionTotalE2 = PositionMaxE2 - PositionMinE2 + 1;

    	fout.open(("train_temp.txt"),ios::out);
    	for (int i = 0; i < trainLists.size(); i++)
    	{
    		int len = trainLength[i];
    		//cout << i<< "\n";
    		int *work1 = trainLists[i];
    		for (int j=0; j<len; j++)
    			fout << work1[j]<<",";
    		fout <<"\n";
    
    		int *work2 = trainPositionE1[i];
    		for (int j=0; j<len; j++)
    			fout << work2[j]<<",";
    		fout <<"\n";
    
    		int *work3 = trainPositionE2[i];
    		for (int j=0; j<len; j++)
    			fout << work3[j]<<",";
    		fout <<"\n"<<"\n";
    	}
    	fout.close();

    	map<string, vector<int> >::iterator it;
    	fout.open(("bags_train.txt"),ios::out);
    	cout << "Number of bags "<< bags_train.size()<<'\t'<<bags_test.size()<<"\t"<<bags_dev.size()<<endl;
    	for (it = bags_train.begin(); it != bags_train.end(); it++ )
    	{
    		string bagname = it->first;
    		fout << bagname<<"\t";
    		vector<int> indices = it->second;
    		for(int i=0;i<indices.size();i++){
    			if(i != indices.size() -1){
    				fout << indices[i]<<",";
    			}else{
    				fout << indices[i];
    			}
    		}
    		fout <<"\n";
    		for(int i=0;i<indices.size();i++){
    			int len = trainLength[indices[i]];
    			fout <<headList[indices[i]]<<","<<tailList[indices[i]]<<","<<ldist[indices[i]]<<","<<rdist[indices[i]]<<","<<relationList[indices[i]]<<","<<trainLength[indices[i]]<<"\n";
    			int *work1 = trainLists[indices[i]];
    			for (int j=0; j<len; j++){
    				if(j!=len-1){
    					fout << work1[j]<<",";
    				}else{
    					fout << work1[j];
    				}
    			}
    			fout <<"\n";
    
    			int *work2 = trainPositionE1[indices[i]];
    			for (int j=0; j<len; j++){
    				if(j!=len-1){
    					fout << work2[j]<<",";
    				}else{
    					fout << work2[j];
    				}
    			}
    			fout <<"\n";
    
    			int *work3 = trainPositionE2[indices[i]];
    			for (int j=0; j<len; j++){
    				if(j!=len-1){
    					fout << work3[j]<<",";
    				}else{
    					fout << work3[j];
    				}
    			}
    			fout <<"\n";
    		}
    	}
    	fout.close();
    	fout.open(("bags_test.txt"),ios::out);
    	for (it = bags_test.begin(); it != bags_test.end(); it++ )
    	{
    		string bagname = it->first;
   		 fout << bagname<<"\t";
    		vector<int> indices = it->second;
    		for(int i=0;i<indices.size();i++){
    			if(i != indices.size() -1){
    				fout << indices[i]<<",";
    			}else{
    				fout << indices[i];
    			}
    		}
    		fout <<"\n";
    		for(int i=0;i<indices.size();i++){
    			int len = testtrainLength[indices[i]];
    			fout << testheadLists[indices[i]]<<","<<testtailLists[indices[i]]<<","<<testldist[indices[i]]<<","<<testrdist[indices[i]]<<","<<testrelationList[indices[i]]<<","<<testtrainLength[indices[i]]<<"\n";
    			int *work1 = testtrainLists[indices[i]];
    			for (int j=0; j<len; j++){
    				if(j!=len-1){
    					fout << work1[j]<<",";
    				}else{
    					fout << work1[j];
    				}
    			}
    			fout <<"\n";
    
    			int *work2 = testPositionE1[indices[i]];
    			for (int j=0; j<len; j++){
    				if(j!=len-1){
    					fout << work2[j]<<",";
    				}else{
    					fout << work2[j];
    				}
    			}
    			fout <<"\n";
    
    			int *work3 = testPositionE2[indices[i]];
    			for (int j=0; j<len; j++){
    				if(j!=len-1){
    					fout << work3[j]<<",";
    				}else{
    					fout << work3[j];
    				}
    			}
    			fout <<"\n";
  		}
  	}
  	fout.close();

    fout.open(("bags_dev.txt"),ios::out);
        for (it = bags_dev.begin(); it != bags_dev.end(); it++ )
        {
            string bagname = it->first;
         fout << bagname<<"\t";
            vector<int> indices = it->second;
            for(int i=0;i<indices.size();i++){
                if(i != indices.size() -1){
                    fout << indices[i]<<",";
                }else{
                    fout << indices[i];
                }
            }
            fout <<"\n";
            for(int i=0;i<indices.size();i++){
                int len = devtrainLength[indices[i]];
                fout << devheadLists[indices[i]]<<","<<devtailLists[indices[i]]<<","<<devldist[indices[i]]<<","<<devrdist[indices[i]]<<","<<devrelationList[indices[i]]<<","<<devtrainLength[indices[i]]<<"\n";
                int *work1 = devtrainLists[indices[i]];
                for (int j=0; j<len; j++){
                    if(j!=len-1){
                        fout << work1[j]<<",";
                    }else{
                        fout << work1[j];
                    }
                }
                fout <<"\n";
    
                int *work2 = devPositionE1[indices[i]];
                for (int j=0; j<len; j++){
                    if(j!=len-1){
                        fout << work2[j]<<",";
                    }else{
                        fout << work2[j];
                    }
                }
                fout <<"\n";
    
                int *work3 = devPositionE2[indices[i]];
                for (int j=0; j<len; j++){
                    if(j!=len-1){
                        fout << work3[j]<<",";
                    }else{
                        fout << work3[j];
                    }
                }
                fout <<"\n";
        }
    }
    fout.close();
   }
   
   int main(int argc, char ** argv) {
	cout<<"Init Begin."<<endl;
	init();
	cout<<"Init End."<<endl;
}

