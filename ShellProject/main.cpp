/*
 * Name: Cash
 * Class: Operating Systems
 * Assignment Shell Emulator
 */

#include <iostream>
#include <thread>
#include <condition_variable>
#include "computer.h"
#include "node.h"

#include "schedulers.h"    // scheduler function(s) and data objects

using namespace std;
vector<Process> core1;
vector<Process> core2;
vector<string> schedHist;

bool doneCore;
int times = 0;
int schedCh = 1;
bool accessed = false;
void runner(vector<Process>& procList) //this is our two threads
{
  int schedChoice,  timeQuantum;
  unsigned int procIdx;
  timeQuantum = 2;
  procIdx = -1;
  schedChoice = schedCh;

    //while not all processes have completed: or while computer is not shut down
    while(doneCore)
    {
        //get the process to schedule next using the indicated scheduler
        procIdx = -1;
        int sizeNotDone = 0;
        for(int i = 0; i < procList.size(); i++)
        {
        	if(!procList[i].isDone)
        	{
        		sizeNotDone++;
        	}
        }
        
        if(sizeNotDone > 0) //to ensure there is actually something to schedule
        {
        switch(schedChoice)
        {
            //Round Robin
            case 1:
                procIdx = RoundRobin(times, procList, timeQuantum);
                break;

            //Shortest Process Next
            case 2:


                
                procIdx = SPN(times, procList, timeQuantum);
            


                break;

            //Shortest Remaining Time
            case 3:


                procIdx = SRT(times, procList, timeQuantum);


                break;

            //Highest Response Ratio Next
            case 4:


                // TODO set procIdx to the proper index for the next process to be scheduled using HRRN
                procIdx = HRR(times, procList, timeQuantum);


                break;
            case 5:
            	procIdx = FCFS(times, procList, timeQuantum);
            	break;
        }
        
    	}
        
        
        //if we were given a valid process index
        if(procIdx >= 0 && procIdx < procList.size())
        {
            //update the details for the scheduled process
            ++procList[procIdx].timeScheduled;
            if(procList[procIdx].totalTimeNeeded == procList[procIdx].timeScheduled)
            {
                procList[procIdx].isDone = true;
                procList[procIdx].timeFinished = times;
            }
            while(accessed)
            {
            	int i = 1;
            	i++;
            }
            accessed = true;
            schedHist.push_back(to_string(times) + " - " + procList[procIdx].id);
            accessed = false;


        }
    	
    	if(sizeNotDone > 0 )
    	{
    		times++;
    	}
    }
   
}
int main()
{
	doneCore = true;
	// make two cores for computer that have separate process ques for ease of use
	thread threads[2];
	//run first
	threads[0] = thread(runner, std::ref(core1));
	//run second core
	threads[1] = thread(runner, std::ref(core2));
  // Make a computer
  Shell::Computer c;
  // run it.
  c.run();
  threads[0].join();
  threads[1].join();
  

       

  return 0;
}
