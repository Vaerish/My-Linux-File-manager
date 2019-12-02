/*
 * Name: Cash
 * Class: Operating Systems
 * Assignment Shell Emulator
 */

#include <iostream>
#include <thread>
#include <mutex>         
#include <condition_variable>
#include "computer.h"
#include "node.h"

#include "schedulers.h"    // scheduler function(s) and data objects

int main()
{
  // Make a computer
  Shell::Computer c;
  // run it.
  c.run();
  

       

  return 0;
}
