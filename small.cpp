#include "sv.h"
#include<iostream>
#include "seqIO.h"
/////////////////////////////////////////////////////////////////////////////
void storeCordsCm(vector<vector<qord> > & mRef, mI & mi)
{
	int refC = mi.x1;
//	int ci = refC * (-1); //ci minus i
	int ci = 0; //assuming that the first base in a mum will not have an insertion in query
	qord temp;
	vector<int>:: iterator it;
	if(mi.y1 < mi.y2)
	{
		int qC = mi.y1;
		while( refC < mi.x2+1)
		{
			if((find(mi.mv.begin(),mi.mv.end(),refC) == mi.mv.end()) && (find(mi.mv.begin(),mi.mv.end(),ci) == mi.mv.end()))//if this position does not have a indel
			{
				temp.name = mi.qn;
				temp.cord = qC;
				mRef[refC-1].push_back(temp);
				refC++;
				qC++;
				ci = refC * (-1);
				if(find(mi.mv.begin(),mi.mv.end(),ci) != mi.mv.end())//if the next is a del
				{
					temp.name = mi.qn;
					temp.cord = qC;
					mRef[refC-1].push_back(temp);
				}
			}
			if(find(mi.mv.begin(),mi.mv.end(),refC) != mi.mv.end()) //insertion in reference
			{
				temp.name = mi.qn;
				temp.cord = qC-1; //because when insertion begins, query cord does not increase
				mRef[refC-1].push_back(temp);
				refC++;
				ci = refC * (-1);
			}
			if(find(mi.mv.begin(),mi.mv.end(),ci) != mi.mv.end())
			{
				qC++;
				it = find(mi.mv.begin(),mi.mv.end(),ci);
				it++;
				--ci;

				while((*it == -1) && (it != mi.mv.end()))
				{
					qC++;
					it++;
				}
				if((*it != -1) || (it == mi.mv.end()))
				{
					refC++;
					qC++;
					ci = refC * (-1);
				}
			}
		}
	}
	if(mi.y1 > mi.y2 )
	{
		int qC = mi.y1; //y1 is bigger than y2
		while(refC<mi.x2+1)
		{
			if((find(mi.mv.begin(),mi.mv.end(),refC) == mi.mv.end()) && (find(mi.mv.begin(),mi.mv.end(),ci) == mi.mv.end())) //if this position does not have a indel
			{
				temp.name = mi.qn;
				temp.cord = qC;
				mRef[refC-1].push_back(temp);
				refC++;
				qC--;
				ci = refC * (-1);
				if(find(mi.mv.begin(),mi.mv.end(),ci) != mi.mv.end())
				{
					temp.name = mi.qn;
					temp.cord = qC;
					mRef[refC-1].push_back(temp);
				}
			}
			if(find(mi.mv.begin(),mi.mv.end(),refC) != mi.mv.end())  //position has insertion
			{
				temp.name = mi.qn;
				temp.cord = qC+1;
				mRef[refC-1].push_back(temp);
				refC++;
				ci = refC * (-1);
			}
			if(find(mi.mv.begin(),mi.mv.end(),ci) != mi.mv.end()) //position has deletion
			{
				--qC;
				it = find(mi.mv.begin(),mi.mv.end(),ci);
				it++;
				--ci;
				while((*it == -1) && (it != mi.mv.end()))
				{
					//qC++;
					--qC;
					it++;
				}
				if((*it != -1) || (it == mi.mv.end()))//stretch of -1 has ended
				{
					refC++;
					--qC;
					ci = refC * (-1);
				}
			}
		}
	}
}
//////////////////////////////////////////////////////////
void readUniq(ifstream & fin,vector<mI> & cm, vector<vector<qord> > & umRef,vector<short int> & masterHQ) //records one to one mapping
{
	string refName,qName,indexAln,line;
	size_t pos1,pos2,namePos;
	int count =0,indelPos =0, refStart =0, qStart =0, refEnd =0, qEnd = 0;
	vector<int> vi;
	
	mI tempmi;

	while(getline(fin,line))
	{
		if(line.find('>') != string::npos)//start of an aligning chromosome description
		{
			refName = line.substr(1,line.find(' ')-1);
			pos1 = line.find(' '); //position of the first space
			pos2 = line.find(' ',pos1+1);//position of the second space
			qName = line.substr(pos1+1, pos2-pos1-1); //up to the second space
			pos1 = line.find(' ',pos2+1); //recycling pos1 to find pos3
			indexAln = refName + qName;
			count = -1;

		}
		if((line.size() <10) && (refName != "") && (count > -1))
		{

			indelPos = stoi(line);
			//refStart = refStart + abs(indelPos);
			
			if(indelPos < -1)
			{
				refStart = refStart + abs(indelPos) -1;
				vi.push_back(refStart*-1);
			}
			if(indelPos == -1)
			{
				vi.push_back(-1);
			}
			if(indelPos > 0)
			{
				refStart = refStart + abs(indelPos);
				vi.push_back(refStart);
			}
			if(indelPos ==0) //reached the end of the indel description
			{
				tempmi.mv = vi;
				if(find(cm.begin(),cm.end(),tempmi) != cm.end()) //if tempmi is present within the unique alignments
				{
					storeCordsCm(umRef,tempmi); //add the new mRef or umRef here
//cout<<tempmi.rn<<" "<<tempmi.x1<<" "<<tempmi.x2<<" "<<tempmi.qn<<" "<<tempmi.y1<<" "<<tempmi.y2<<endl;
				}
				if((cm[0].rn == tempmi.rn) && (cm[0].qn == tempmi.qn))
				{
					storeCords(masterHQ,tempmi);
				}
				vi.clear();//reset it once its values are used
			}
			count++;
		}
		if((line.find('>') == string::npos) && (line.size() >10) && (refName != "")) //when describing alignment segments
		{
			tempmi.rn = refName;
			tempmi.qn = qName;
			refStart = stoi(line,&pos1);
			refEnd = stoi(line.substr(pos1),&pos2);
			qStart = stoi(line.substr(pos1+pos2), &namePos);
			qEnd = stoi(line.substr(pos1+pos2+namePos));
			tempmi.x1 = refStart;
			tempmi.x2 = refEnd;
			tempmi.y1 = qStart;
			tempmi.y2 = qEnd;
			count = 0;
			--refStart;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void readfasta(ifstream & fin,map<string, string> & fastaseq) //reading fasta files
{
	string str,index;
	size_t pos;

	while(getline(fin,str))
	{
		if(str[0] == '>')
		{
			// Getting the substring from the start until the first whitespace
			if((pos = str.find_first_of(" \t")) != string::npos) {
				index = str.substr(1, pos - 1);
			}
			else {
				index = str.substr(1);
			}
		}
		if(str[0] != '>')
		{
			fastaseq[index].append(str);
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void callSmall(mI & mi,vector<vector<qord> > & umRef, string & refseq, string & qseq,vector<int> & seqLen, ofstream & fsmall) //just get the individual sequences passed
{
	int refPos =0,refGap =0,qGap =0;
	refPos = mi.x1 -2;
	
	qord lq,delStart;//lq is last qord :P
	sort(umRef[mi.x1-1].begin(),umRef[mi.x1-1].end());
	lq.cord = umRef[mi.x1-1][0].cord -1;

	vector<qord> tq; //creating one element
	vector<int> ti;
	mI tempmi;

	string refName = mi.rn;
//	for(map<int,vector<qord> >::iterator it= umRef.begin();it != umRef.end();it++)
	for(int pos = mi.x1-1;pos < mi.x2;pos++)//pos is the coordinates from a cm
	{
		//pos = it->first;
//cout<<"small\t"<<mi.rn<<'\t'<<mi.x1<<'\t'<<mi.x2<<'\t'<<pos<<endl;
		if(umRef[pos].size() >1)
		{
//			sort(umRef[pos].begin(),umRef[pos].end());//sort the coordinates to remain consistent

		}
		if(umRef[pos].size() >0) //a mapping is present
		{
			refGap = (pos - refPos);
			qGap = abs(lq.cord - umRef[pos][0].cord);
			if((refGap == 1) && (qGap == 1))
			{
				if(tq.size()>0)
				{
					fsmall<<refName<<"\t"<<ti[0]<<"\t"<<ti[ti.size()-1]<<"\tDEL\t"<<tq[0].name<<"\t"<<tq[0].cord<<"\t"<<tq[tq.size()-1].cord<<"\ts"<<setfill('0')<<setw(10)<<ti[0]<<refName<<"\t"<<abs(ti[0] - ti[ti.size()-1])+1<<"\t"<<"NA"<<"\t"<<"NA"<<endl;
					tq.clear();
					ti.clear();
				}
				if(mi.y1 > mi.y2) //if inverted
				{
//cout<<pos<<"\t"<<umRef[pos][0].cord<<"\t"<<qseq.size()<<"\t"<<refseq.size()<<endl;
					if(((int(qseq[umRef[pos][0].cord -1]) - int(comp(refseq[pos]))) == 0) || (abs(int(qseq[umRef[pos][0].cord -1]) - int(comp(refseq[pos]))) == 32)) //both are same case or different case
					{
						qseq[umRef[pos][0].cord -1] = refseq[pos];
					}
				}
				if((int(refseq[pos])-int(qseq[umRef[pos][0].cord -1]) != 0) && (abs(int(refseq[pos])-int(qseq[umRef[pos][0].cord -1])) != 32))
				{
					fsmall<<refName<<"\t"<<pos+1<<"\t"<<refseq[pos]<<"\tSNP\t"<<umRef[pos][0].name<<"\t"<<umRef[pos][0].cord<<"\t"<<qseq[umRef[pos][0].cord -1]<<endl;
				}
			}
			if((refGap == 1) && (qGap ==0))
			{
				tq.push_back(lq);
				ti.push_back(pos);
			}
			if((refGap == 1) && (qGap>1))
			{
				//fout<<"INS "<<refName<<" "<<refPos+1<<" "<<refPos+1<<" "<<umRef[pos][0].name<<" "<<lq.cord+1<<" "<<umRef[pos][0].cord-1<<" "<<endl;
				if(mi.y1 < mi.y2) //if forward oriented
				{
					fsmall<<refName<<" "<<refPos+1<<"\t"<<refPos+1<<"\tINS\t"<<umRef[pos][0].name<<"\t"<<lq.cord+1<<"\t"<<umRef[pos][0].cord-1<<"\ts"<<setfill('0')<<setw(10)<<refPos+1<<refName<<"\t"<<abs(lq.cord+1-umRef[pos][0].cord+1)+1<<"\t"<<"NA\tNA"<<endl;
				}
				if(mi.y1 > mi.y2)
				{
				//	cout<<"INS\t"<<refName<<"\t"<<refPos+1<<" "<<refPos+1<<"\t"<<umRef[pos][0].name<<"\t"<<lq.cord-1<<"\t"<<umRef[pos][0].cord +1<<"\t"<<endl;
					fsmall<<refName<<"\t"<<refPos+1<<" "<<refPos+1<<"\tINS\t"<<umRef[pos][0].name<<"\t"<<lq.cord-1<<"\t"<<umRef[pos][0].cord +1<<"\ts"<<setfill('0')<<setw(10)<<refPos+1<<refName<<"\t"<<abs(lq.cord-1- umRef[pos][0].cord-1)+1<<"\t"<<"NA\tNA"<<endl;
				}
				if(abs((lq.cord+1) - (umRef[pos][0].cord-1)) > 100)
				{
					tempmi.x1 = refPos;
					tempmi.x2 = refPos;
					tempmi.y1 = lq.cord+1;
					tempmi.y2 = umRef[pos][0].cord -1;
					tempmi.rn = refName;
					tempmi.qn = umRef[pos][0].name;
				//	gap.push_back(tempmi);
//cout<<tempmi.rn<<" "<<tempmi.x1<<" "<<tempmi.x2<<" "<<tempmi.qn<<" "<<tempmi.y1<<" "<<tempmi.y2<<endl;
				}
			
				//if(refseq[pos] != qseq[umRef[pos][0].cord -1])
				if(mi.y1 > mi.y2) //if inverted
				{
					if(((int(qseq[umRef[pos][0].cord -1]) - int(comp(refseq[pos]))) == 0) || (abs(int(qseq[umRef[pos][0].cord -1]) - int(comp(refseq[pos]))) == 32)) //both are same case or different case
					{
						qseq[umRef[pos][0].cord -1] = refseq[pos];
					}
				}
				if((int(refseq[pos])-int(qseq[umRef[pos][0].cord -1]) != 0) && (abs(int(refseq[pos])-int(qseq[umRef[pos][0].cord -1])) != 32))
				{
					fsmall<<refName<<"\t"<<pos+1<<"\t"<<refseq[pos]<<"\tSNP\t"<<umRef[pos][0].name<<"\t"<<umRef[pos][0].cord<<"\t"<<qseq[umRef[pos][0].cord -1]<<endl;
				}				
			}
			lq = umRef[pos][0];
			refPos = pos;
			if(umRef[pos].size() >1)
			{
				umRef[pos].erase(umRef[pos].begin(),umRef[pos].begin()+1);//remove the first element
			}
		}
	}
	
	
}
/////////////////////////////////////////////////////////////////////////
char comp(char & N) //return the complementary nucleotide
{
	char c;
	if(N == 'N')
	{
		c = 'N';
	}
	if(N == 'n')
	{
		c = 'n';
	}
	if(N == 'A')
	{
		c = 'T';
	}
	if(N == 'a')
	{
		c = 't';
	}
	if(N == 'T')
	{
		c = 'A';
	}
	if(N == 't')
	{
		c = 'a';
	}
	if(N == 'G')
	{
		c = 'C';
	}
	if(N == 'g')
	{
		c = 'c';
	}	
	if(N == 'C')
	{
		c = 'G';
	}
	if(N == 'c')
	{
		c = 'g';
	}

return c;
}		
//////////////////////////////////////////////////////////////////////
void storeCords(vector<short int> & masterHQ, mI & mi)
{

        int ty1 = 0, ty2 =0;

        if(mi.y1 > mi.y2)//if reverse oriented
        {
                ty1 = mi.y2;
                ty2 = mi.y1;
        }
        if(mi.y1 < mi.y2)//forward oriented
        {
                ty1 = mi.y1;
                ty2 = mi.y2;
        }
      	for(int j = ty1-1; j<ty2;j++)
        {
                masterHQ[j]++;
        }
}
	
