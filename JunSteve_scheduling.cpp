#include <iostream>
#include <fstream>
using namespace std;

/*
 * Scheduling C++
 * 
 * parameters on dev c++:
 * copy paste these parameters in to make your life easier!
 ************************************************
 * "Data_JobTimes.txt" "Data_DependencyPairs.txt"
 ************************************************
 *
 * @author Steve Jun
*/
struct jobNode{
	int id;
	jobNode* next;
	
	jobNode(){			id=-1;		next=NULL;}
	jobNode(int i){		id=i;		next=NULL;}
};
//*******************************
int main(int argc, char* argv[]){
	
	if (argc < 3) {
		cerr<<"usage: "<<argv[0]<<" <input file> <output file> "<<endl;
		return 0;
	}
	
	//step 0: prepare and initialize everything
	ifstream jobTimeFile;
	jobTimeFile.open(argv[1]);
	
	ifstream jobDependFile;
	jobDependFile.open(argv[2]);
	
	int j, t, d, maxTime, numOfJobs, numOfDepends;
	jobNode* openList = new jobNode();
	jobNode* job;
	jobNode* pointer;
	jobNode* LH;
	
	int** table;//schedule table
	jobNode* dependencyGraph;
	int* processJob;	//current job on process;
	int* processTime;	//less than or equal to 0 means available;
	int* parentCount;	//tracks num of parents for job; -1 deleted
	int* jobTime;		//job's time requirement;
	int* jobStatus;		//jobDone; 1 deleted from graph; 0 still in graph; -1 processing
	
	cout<<"Reading file 1"<<endl;
	jobTimeFile>>skipws>>numOfJobs;
	maxTime=0;
	jobTime=new int[numOfJobs+1];
	while(jobTimeFile>>skipws>>j){
		jobTimeFile>>skipws>>t;
		jobTime[j]=t;
		maxTime+=t;
		cout<<"job: "<<j<<" time: "<<t<<endl;
	}
	
	//table[numOfJobs][maxTime] //worse case
	table=new int*[numOfJobs];
	for(int i=1; i<=numOfJobs; ++i)
		table[i]=new int[maxTime];	
	for(int p=1; p<=numOfJobs; ++p)
		for(int t=0; t<=maxTime; ++t)
			table[p][t]=0;	
	
	dependencyGraph=new jobNode[numOfJobs+1];
	parentCount=new int[numOfJobs+1];
	jobStatus=new int[numOfJobs+1];
	processJob=new int[numOfJobs+1];
	processTime=new int[numOfJobs+1];
	for(int i=1; i<=numOfJobs; ++i){
		parentCount[i]=0;
		dependencyGraph[i]=0;
		jobStatus[i]=0;
		processJob[i]=0;
		processTime[i]=0;
	}
	
	cout<<endl<<"Reading file 2"<<endl;
	jobDependFile>>skipws>>d;
	while(jobDependFile>>skipws>>j){
		jobDependFile>>skipws>>d;
		
		job=new jobNode(d);
		pointer=&dependencyGraph[j];
		while(pointer->next!=NULL)
			pointer=pointer->next;
		pointer->next=job;
		++parentCount[j];
		cout<<"job: "<<j<<" depends on job: "<<d<<endl;
	}
//*******************************
	int time = 0;
	bool graphNotEmpty = true;
	while(graphNotEmpty){
		openList = new jobNode();
		LH = openList;
		graphNotEmpty=false;
	/*
	step 1: find jobs that do not have any parent 
		(ie., check parentCount[i] == 0)
		and place them, one-by-one, onto OPEN list
	*/
		for(int i=1; i<=numOfJobs; ++i){
			if (parentCount[i]==0 && jobStatus[i]==0){
				job=new jobNode(i);
				LH->next=job;
				LH=LH->next;
				parentCount[i]=-1;
				jobStatus[i]=-1;
			}
		}
		LH=openList;
	/*
	step 2: 2.1: newJob <-- remove from OPEN 
	        2.2: availProc <-- the next available processTime (looking
		     into processjob[i] <= 0)
	        2.3: place newJob on the processJob[availProc], 
		     place newJob's time on processTime[availProc]
		     update the scheduling table under availProc,
			(with respect to TIME status and job's time requiement).
	        2.4: repeat 2.1 and 2.3 until OPEN is empty
	*/
		int newJob;
		int availProc;
		while(LH->next!=NULL){
			//2.1
			newJob=LH->next->id;
			//2.2
			bool foundProc=false;
			for(int i=1; i<=numOfJobs; ++i){
				if (processTime[i]<=0){
					availProc=i;
					foundProc=true;
				}
				if (foundProc) break;
			}
			//2.3
			processJob[availProc]=newJob;
			processTime[availProc]=jobTime[newJob];
			jobStatus[newJob]=-1;
			LH=LH->next;
		}//2.4 repeat	
	//update scheduling table
		for(int p=1; p<=numOfJobs; ++p)
			for(int t=0; t<=maxTime; ++t)
				table[p][time]=processJob[p];

	//step 3: print the scheduling table, TIME, all 1-D arrays with proper heading.
		cout<<endl<<"At start of Time: "<<time<<endl;
		
		cout<<"|Processor|"<<'\t'<<"processJob"<<'\t'<<"processTime"<<'\t'
			<<"|Job|"<<'\t'<<"parentCount"<<'\t'<<"jobTime"<<'\t'<<'\t'<<"jobStatus"<<endl;
		for (int i=1; i<numOfJobs; ++i){
				cout<<' '<<'|'<<i<<'|'<<'\t'<<'\t'
				<<processJob[i]<<'\t'<<'\t'
				<<processTime[i]<<'\t'<<'\t'
				<<' '<<'|'<<i<<'|'<<'\t'
				<<parentCount[i]<<'\t'<<'\t'
				<<jobTime[i]<<'\t'<<'\t'
				<<jobStatus[i]<<endl;
		}
		cout<<endl<<"Scheduling Table: "<<endl;
		for(int p=1; p<=numOfJobs; ++p){
			for(int t=0; t<=maxTime; ++t){
				if (table[p][t]==0)	cout<<"  ";
				else {
					if (table[p][t]<10)
						cout<<' '<<table[p][t];
					else cout<<table[p][t];
				}
				cout<<' ';
			}
			cout<<endl;
		}
			
	//step 4: track the TIME (ie, decrease the processTime[i] by 1 and Time++)
		++time;
		for(int i=1; i<=numOfJobs; ++i){
				if (processTime[i]>0) 
					--processTime[i];
		}
	/*
	step 5: job <-- find a job that is done, ie., processTIME [i] == 0 ;
	        5.1: delete the job from the processJob[i]
	        5.2: delete the job from the graph (update jobDone[job])
	        5.3: delete all it's outgoing arcs (decrease by 1, the paraentCount[job] of its dependents)
			5.4: jobDone[job] <-- 1
			5.5: repeat 5.1 to 5.4 until no more finished job
	*/
		for(int i=1; i<=numOfJobs; ++i){
			//job is done if
			//time on proc==0 && there exists a job on proc && jobStatus: isProcessing (-1)
			int jobID=processJob[i];
			if (processTime[i]==0 && processJob[i]!=0 && jobStatus[jobID]==-1){
				//5.1
				processJob[i]=0;
				//5.2
				jobStatus[jobID]=1;
				//5.3
				for(int j=1; j<=numOfJobs; ++j){
					pointer=&dependencyGraph[j];
					while(pointer->next!=NULL){
						if (pointer->next->id==jobID){
							//remove dependency on hash table
							if (pointer->next->next!=NULL)
								pointer->next=pointer->next->next;
							else 
								pointer->next==NULL;
							--parentCount[j];		
						}
						pointer=pointer->next;	
					}
				}
			}
		}//5.5 repeat
	//step 6: print the scheduling table, TIME, all 1-D arrays with proper heading.
		cout<<endl<<"After 1 time, time is now: "<<time<<endl;
		
		cout<<"|Processor|"<<'\t'<<"processJob"<<'\t'<<"processTime"<<'\t'
			<<"|Job|"<<'\t'<<"parentCount"<<'\t'<<"jobTime"<<'\t'<<'\t'<<"jobStatus"<<endl;
		for (int i=1; i<numOfJobs; ++i){
				cout<<' '<<'|'<<i<<'|'<<'\t'<<'\t'
				<<processJob[i]<<'\t'<<'\t'
				<<processTime[i]<<'\t'<<'\t'
				<<' '<<'|'<<i<<'|'<<'\t'
				<<parentCount[i]<<'\t'<<'\t'
				<<jobTime[i]<<'\t'<<'\t'
				<<jobStatus[i]<<endl;
		}
		cout<<endl<<"Scheduling Table: "<<endl;
		for(int p=1; p<=numOfJobs; ++p){
			for(int t=0; t<=maxTime; ++t){
				if (table[p][t]==0)	cout<<"  ";
				else {
					if (table[p][t]<10)
						cout<<' '<<table[p][t];
					else cout<<table[p][t];
				}
				cout<<' ';
			}
			cout<<endl;
		}
		
		for (int i=1; i<=numOfJobs; ++i)
			if (jobStatus[i]!=1) graphNotEmpty=true;
		delete openList;
}
//step 7: repeat step 1 to step 6 until graph is empty (looking into the 1-D array of jobs'status)
	
	
	delete dependencyGraph, parentCount,jobStatus,processJob, processTime;
	delete job, pointer, LH;
	
	cout<<endl<<"Total number of jobs: "<<numOfJobs<<endl;
	cout<<"Total time taken to finish: "<<time<<endl;
	/***END***/
	jobTimeFile.close();
	jobDependFile.close();
	cout<<"Assignment Complete!"<<endl;

	system("pause");
    return 0;  
}

