#include "schedulers.h"
#include <random>
//Round Robin scheduler implementation. In general, this function maintains a double ended queue
//of processes that are candidates for scheduling (the ready variable) and always schedules
//the first process on that list, if available (i.e., if the list has members)
#include "schedulers.h"

//Round Robin scheduler implementation. In general, this function maintains a double ended queue
//of processes that are candidates for scheduling (the ready variable) and always schedules
//the first process on that list, if available (i.e., if the list has members)
int RoundRobin(const int& curTime, const vector<Process>& procList, const int& timeQuantum)
{
    static int timeToNextSched = timeQuantum;  //keeps track of when we should actually schedule a new process
    static deque<int> ready;  //keeps track of the processes that are ready to be scheduled

    int idx = -1, chk;
    bool done;
    bool found;

    // first look through the process list and find any processes that are newly ready and
    // add them to the back of the ready queue
    for(int i = 0, i_end = procList.size(); i < i_end; ++i)
    {
        if(procList[i].startTime < curTime)
        {
            found = false;
            for(int j = 0; j < ready.size(); j++)
            {
                if(ready[j] == i)
                {
                    found = true;
                }
            }
            if(!found)
            {
                ready.push_back(i);
            }
            
        }
    }

    // now take a look the head of the ready queue, and update if needed
    // (i.e., if we are supposed to schedule now or the process is done)
    if(timeToNextSched == 0 || procList[ready[0]].isDone)
    {
        // the process at the start of the ready queue is being taken off of the
        // processor

        // if the process isn't done, add it to the back of the ready queue
        if(!procList[ready[0]].isDone)
        {
            ready.push_back(ready[0]);
        }

        // remove the process from the front of the ready queue and reset the time until
        // the next scheduling
        ready.pop_front();
        timeToNextSched = timeQuantum;
    }

    // if the ready queue has any processes on it
    if(ready.size() > 0)
    {
        // grab the front process and decrement the time to next scheduling
        idx = ready[0];
        --timeToNextSched;
    }
    // if the ready queue has no processes on it
    else
    {
        // send back an invalid process index and set the time to next scheduling
        // value so that we try again next time step
        idx = -1;
        timeToNextSched = 0;
    }

    // return back the index of the process to schedule next
    return idx;
}


int SPN(const int& curTime, const vector<Process>& procList, const int& timeQuantum)
{
    static int timeToNextSched = timeQuantum;  //keeps track of when we should actually schedule a new process
    static vector<int> ready;  //keeps track of the processes that are ready to be scheduled

    int idx = -1, chk;
    bool done;
    bool found;

    int i_end;
    for(int i = 0, i_end = procList.size(); i < i_end; ++i)
    {
        if(procList[i].startTime < curTime)
        {
            found = false;
            for(int j = 0; j < ready.size(); j++)
            {
                if(ready[j] == i)
                {
                    found = true;
                }
            }
            if(!found)
            {
                ready.push_back(i);
            }
            
        }
    }
    if(procList[ready[0]].isDone)
    {
   
        ready.erase(ready.begin());
        int size = procList[ready[0]].totalTimeNeeded;
        int place = 0;
        int input = ready[0];
        int start = procList[ready[0]].startTime;

    

    if(ready.size() > 0)
    {

        for(int i = 0; i < ready.size(); i++)
        {
            if(procList[ready[i]].totalTimeNeeded < size || ((procList[ready[i]].totalTimeNeeded == size) && procList[ready[i]].startTime < start))
            {
                
                place = i;
                size = procList[ready[i]].totalTimeNeeded;
                input = ready[i];
                start = procList[ready[i]].startTime;
            }
        }
        ready.emplace(ready.begin(), input);
        ready.erase(ready.begin() + place +1);
        

        }
    }

    if(ready.size() > 0)
    {
       
        idx = ready[0];
     
    }
    else
    {
        
        idx = -1;
    
    }

    return idx;
}

int FCFS(const int& curTime, const vector<Process>& procList, const int& timeQuantum)
{
    static int timeToNextSched = timeQuantum;  //keeps track of when we should actually schedule a new process
    static vector<int> ready;  //keeps track of the processes that are ready to be scheduled

    int idx = -1, chk;
    bool done;
    bool found;

    int i_end;
    for(int i = 0, i_end = procList.size(); i < i_end; ++i)
    {
        if(procList[i].startTime < curTime)
        {
            found = false;
            for(int j = 0; j < ready.size(); j++)
            {
                if(ready[j] == i)
                {
                    found = true;
                }
            }
            if(!found)
            {
                ready.push_back(i);
            }
            
        }
    }
    if(procList[ready[0]].isDone)
    {
   
        ready.erase(ready.begin());
    }


    if(ready.size() > 0)
    {
       
        idx = ready[0];
    }
    else
    {
        
        idx = -1;
    
    }

    return idx;
}



int SRT(const int& curTime, const vector<Process>& procList, const int& timeQuantum)
{
    static int timeToNextSched = timeQuantum;  //keeps track of when we should actually schedule a new process
    static vector<int> ready;  //keeps track of the processes that are ready to be scheduled

    int idx = -1, chk;
    bool done;
    bool found;

    // first look through the process list and find any processes that are newly ready and
    // add them to the back of the ready queue
    for(int i = 0, i_end = procList.size(); i < i_end; ++i)
    {
        if(procList[i].startTime < curTime)
        {
            found = false;
            for(int j = 0; j < ready.size(); j++)
            {
                if(ready[j] == i)
                {
                    found = true;
                }
            }
            if(!found)
            {
                ready.push_back(i);
            }
            
        }
    }
    if(procList[ready[0]].isDone)
    {
        ready.erase(ready.begin());
    }
    int shortestTime = procList[ready[0]].totalTimeNeeded - procList[ready[0]].timeScheduled;
    int theirTime;
    int place = 0;
    bool switched = false;
    for(int i = 0; i < ready.size(); i++)
    {

        theirTime = procList[ready[i]].totalTimeNeeded - procList[ready[i]].timeScheduled;
        if(shortestTime > theirTime && theirTime != 0)
        {
            place = i;
            switched = true;
            shortestTime = theirTime; 
            
        }
        else if(shortestTime == 0)
        {
            place = i;
            switched = true;
            shortestTime = theirTime; 
            
        }
    }

    if(switched)
    {
      
        ready.insert(ready.begin(), ready[place]);

        
    }

    // if the ready queue has any processes on it
    if(ready.size() > 0)
    {
        // grab the front process and decrement the time to next scheduling
        idx = ready[0];
        --timeToNextSched;
    }
    // if the ready queue has no processes on it
    else
    {
        // send back an invalid process index and set the time to next scheduling
        // value so that we try again next time step
        idx = -1;
        timeToNextSched = 0;
    }

    // return back the index of the process to schedule next
    return idx;
}

int HRR(const int& curTime, const vector<Process>& procList, const int& timeQuantum)
{
    static int timeToNextSched = timeQuantum;  //keeps track of when we should actually schedule a new process
    static vector<int> ready;  //keeps track of the processes that are ready to be scheduled
    bool found;
    int idx = -1;

   for(int i = 0, i_end = procList.size(); i < i_end; ++i)
    {
        if(procList[i].startTime < curTime)
        {
            found = false;
            for(int j = 0; j < ready.size(); j++)
            {
                if(ready[j] == i)
                {
                    found = true;
                }
            }
            if(!found)
            {
                ready.push_back(i);
            }
            
        }
    }


    if(procList[ready[0]].isDone)
    {
        ready.erase(ready.begin());
        if(ready.size() > 0)
        {
        int size = procList[0].totalTimeNeeded;
        float chosenRatio = (float((procList[ready[0]].totalTimeNeeded)+(curTime -(procList[ready[0]].startTime)))/float(procList[ready[0]].totalTimeNeeded));
        float theirRatio;
        int place = 0;

 
        for(int i = 0; i < ready.size(); i++)
        {
            theirRatio = (float((procList[ready[i]].totalTimeNeeded) + (curTime-(procList[ready[i]].startTime))) / float(procList[ready[i]].totalTimeNeeded));
            if(theirRatio > chosenRatio || chosenRatio == 0)
            {
                idx = ready[i];
                place = i;
                chosenRatio = theirRatio;
            }
        }
        ready.insert(ready.begin(), ready[place]);
        ready.erase(ready.begin() + place + 1);
        }
    }
    // if the ready queue has no processes on it
    if(ready.size() > 0)
    {
        // grab the front process and decrement the time to next scheduling
        idx = ready[0];
        --timeToNextSched;
    }

    else
    {

        idx = -1;
        timeToNextSched = 0;
    }

    return idx;
}

