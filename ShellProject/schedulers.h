#ifndef SCHEDULERS_H
#define SCHEDULERS_H

#include<vector>  //process vector
#include<deque>   //for ready double ended queue
#include<fstream>  // file i/o
#include<iostream> // cerr

using namespace std;

extern bool doneCore;
extern int times;


struct Process
{
    Process() : isDone(false), totalTimeNeeded(-1), timeScheduled(0), startTime(-1), timeFinished(-1) {}

    // Given data
    string id;            //The process id
    int startTime;        //The time at which the process becomes available for scheduling
    int totalTimeNeeded;  //The total amount of time needed by the process

    // Process details
    bool isDone;          //Indicates if the process is complete
    int timeScheduled;    //The amount of time the process has been scheduled so far
    int timeFinished;     //The time that the process completed
};


inline void readInProcList(const string& fname, vector<Process>& procList)
{
    ifstream in(fname.c_str());
    int numProcs;

    if(in.fail())
    {
        cerr << "Unable to open file \"" << fname << "\", terminating" << endl;
        exit(-1);
    }

    in >> numProcs;
    procList.resize(numProcs);
    for(auto& p:procList)
    {
        in >> p.id >> p.startTime >> p.totalTimeNeeded;
    }
    in.close();
}


int RoundRobin(const int& curTime, const vector<Process>& procList, const int& timeQuantum);

int SPN(const int& curTime, const vector<Process>& procList, const int& timeQuantum);

int SRT(const int& curTime, const vector<Process>& procList, const int& timeQuantum);

int HRR(const int& curTime, const vector<Process>& procList, const int& timeQuantum);

#endif