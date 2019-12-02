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
int main()
{
	doneCore = true;
	// make two cores for computer that have separate process ques for ease of use
	thread threads[2];
	//run first
	threads[0] = thread(runner(), core1);
	//run second core
	threads[1] = thread(runner(), core2);
  // Make a computer
  Shell::Computer c;
  // run it.
  c.run();
  threads[0].join();
  threads[1].join();
  

       

  return 0;
}
