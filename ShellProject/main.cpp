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
vector<Process> core1;
vector<Process> core2;

bool doneCore;
int times;
void runner(vector<Process>& procList) //this is our two threads
{
  int curTime = 0, input, schedChoice, numProc, procIdx, timeQuantum;
  timeQuantum = 2;
  curTime = 0;
  procIdx = -1;
  schedChoice = -1;
   
    curTime = 0;

    //while not all processes have completed: or while computer is not shut down
    while(doneCore)
    {
        //get the process to schedule next using the indicated scheduler
        procIdx = -1;
        switch(schedChoice)
        {
            //Round Robin
            case 1:
                procIdx = RoundRobin(curTime, procList, timeQuantum);
                break;

            //Shortest Process Next
            case 2:


                
                procIdx = SPN(curTime, procList, timeQuantum);
            


                break;

            //Shortest Remaining Time
            case 3:


                procIdx = SRT(curTime, procList, timeQuantum);


                break;

            //Highest Response Ratio Next
            case 4:


                // TODO set procIdx to the proper index for the next process to be scheduled using HRRN
                procIdx = HRR(curTime, procList, timeQuantum);


                break;
        }
        curTime++;
        times = curTime;

        //if we were given a valid process index
        if(procIdx >= 0 && procIdx < procList.size())
        {
            //update the details for the scheduled process
            ++procList[procIdx].timeScheduled;
            if(procList[procIdx].totalTimeNeeded == procList[procIdx].timeScheduled)
            {
                procList[procIdx].isDone = true;
                procList[procIdx].timeFinished = curTime;
            }

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
