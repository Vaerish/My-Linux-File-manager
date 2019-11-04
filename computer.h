#include <vector>
#include <iostream>
#include "node.h"
#include <limits>
#include <sstream>
#include <map>
#include <string>

#ifndef COMPUTER_H
#define COMPUTER_H
namespace Shell // 
{
  const std::string CMDS[] = {"", "ls", "pwd", "exit", "mkdir", "touch", "cd", "rm", "rmdir", "chmod"};

  // Computer class 
  // Represents the OS who controls the file System.
  class Computer
  {
    // Private vars
    private:
      // The root File
      Node* rootFile;
      // The current Directory the System is looking at
      Node* curDir;
      // The current logged in user.
      User* curUser;
      // The name of this computer
      std::string computerName;


    // Public functions
    public:

      // The constructor taking in the computer name
      Computer(std::string name)
      {
        // set the name and call the default constuctor
        computerName = name;
        Computer();
      }
      // Deconstructor, make sure we don't have memory leaks :)
      ~Computer()
      {
        delete curUser;
        // Delete the root : let its deconstructor handle deleting
        // the rest of the file system.
        delete rootFile;
        // Make sure we clear the pointers to prevent some accidental
        // pointer to mem we don't own.
        rootFile = nullptr;
        curDir = nullptr;
        curUser = nullptr;
      }
      // Main constructor - does the heavy lifting
      Computer()
      {
        // No user to start off with, need to login.
        curUser = nullptr;
        // Create the root of the file system.
        rootFile = new Node("", true, nullptr, 0, "root", "root");
        // Set the root's parent to itself - makes it auto handle ../ on root. 
        // simple hack to make my life easier down the line.
        rootFile->parent = rootFile;
        // Make the root user.
        curUser = new User("root", "root", true, "toor");
        // set the computer name.
        computerName = "computer";
        // move the current location to the root - this will change depending
        // on who logs in
        curDir = rootFile;
        // create home and root directory
        rootFile->AddChild(new Node("root", true, rootFile));
        rootFile->AddChild(new Node("home", true, rootFile));
      }

      // Running the computer. Handles all operations from here.
      void run()
      {
        // Start the console.
        console();
      }
      
    // Private functions
    private:
      // Console handles taking in input and outputting correct response.
      // is what emulates running the shell.
      void console()
      {
        // control var for machine
        bool looping = true;
        // storages variables
        std::string dir;
        std::string input;
        // While we haven't quit
        while(looping)
        {
          // Gets current working directory
          dir = pwd();
          //looks for user's name in wd
          int pos = dir.find(curUser->Username());
          // if found, use ~ insead the complete path.
          if(pos > -1)
            dir = "~" + dir.substr(pos + curUser->Username().length());
          // output the status line. 
          std::cout << curUser->Username() << "@" << computerName << ":" << dir
               << "#" << " ";
          // Get input from the user.
          std::getline(std::cin, input);
          // Parse it and handle it.
          looping = parser(input);
        }
      }
      // Parses input. returns true if the console should continue.
      bool parser(std::string input)
      {
        // storage vars for easier manipulation of data.
        std::stringstream stream(input);
        std::string command;
        std::vector<std::string> args;
        // Using a string stream to quickly break up input on spaces.
        stream >> command;
        // get the rest of the input
        std::string temp;
        // store it as an argument
        while(stream >> temp)
        {
          args.push_back(temp);
        }
        // return the results of commands func
        return commands(command, args);
      }

      // attempts to run a command using the command and the args for it.
      // returns true if console should keep running.
      bool commands(std::string command, std::vector<std::string> args)
      {
        // Handles ls command
        if(command == "ls")
        {
          // if command has args
          if(args.size() > 0)
          {
            // if it has only the correct arg.
            if(args.size() == 1 && args[0] == "-l") 
            {
              // display output
              for(auto childPair :curDir->Children())
              {
                Node* child = childPair.second;
                std::cout << child->PermsStr() << " " << child->NumDirs()
                          << " " << child->User() << " " << child->Group() << " "
                          << child->Size() << " " << child->TimeStr() << " "
                          // Adds blue color if it is a dir
                          << (child->isDir ? "\033[34m" : "")
                          << child->name << "\033[0m" << std::endl;
              }
            }
            else
            {
              // otherwise invalid useage
              std::cout << "Invalid use - For help use: help ls\n";
            }
            
          }
          // no args
          else
          {
            // display simple output
            for(auto child : curDir->Children())
            {
              std::cout << (child.second->IsDir() ? "\033[34m" : "") 
                        << child.first << "\033[0m ";
            }
            // adds a new line at the end only if there was something to print
            if(curDir->Children().size() > 0)
              std::cout << std::endl;
          }
          
        }
        // Handles pwd command
        else if(command == "pwd")
        {
          // outputs the current directory
          std::cout << pwd() << std::endl;
        }
        // Handles the exit command
        else if(command == "exit")
        {
          // return false to signal ending
          return false;
        }
        // Handles mkdir command
        else if(command == "mkdir")
        {
          // if there are no arguments
          if(args.size() == 0)
          {
            // error
            std::cout << "Invalid use - For help use: help mkdir\n";
          }
          // else 
          else
          {
            // iterate over arguments
            for(std::string arg : args)
            {
              // Attempt to add new directory if fails output such a message.
              if(!curDir->AddChild(new Node(arg, true, curDir)))
              {
                std::cout << "mkdir: cannot create directory '" << arg << "': File exits\n"; 
              }
            }
          }
        }
        // Handles touch command
        else if(command == "touch")
        {
          // if there are no args
          if(args.size() == 0)
          {
            // error
            std::cout << "touch: Invalid use - For help use: help touch\n";
          }
          // otherwise attempt do it
          else
          {
            // iterate over args
            for(std::string arg : args)
            {
              // try to add and if that fails, update the current timestamp
              if(!curDir->AddChild(new Node(arg, false, curDir)))
              {
                  curDir->children[arg]->UpdateTimeStamp();
              }
            }
          }
        }
        // Handles cd command
        else if (command == "cd")
        {
          // if no args
          if(args.size() == 0)
          {
            // set current directory to root's home
            curDir = rootFile->children["root"];

            // if their directory doesn't exist anymore, put them at the root.
            if(curDir == nullptr)
              curDir = rootFile;
          }
          // if there is args and it is more than one
          else if(args.size() > 1)
          {
            // error
            std::cout << "cd: too many arguments - For help use: help cd\n";
          }
          // else if only one arg
          else
          {
            // attempt to find the file.
            Node* file = findFile(args[0]);
            // if file exists and is not a directory error
            if(file != nullptr)
              if(!file->isDir)
                std::cout << "cd: " << args[0] << " Not a directory\n";
              else
                // else set curDir to it
                curDir = file;
            // else file isn't real
            else std::cout << "cd: Directory does not exist\n";
          }
        }
        // Handles rm command
        else if (command == "rm")
        {
          // if no args
          if(args.size() == 0)
          {
            // error
            std::cout << "rm: Invalid use - For help use: help rm\n";
          }
          // else if there are args
          else
          {
            // iterate over them
            for(auto arg : args)
            {
              // try to find the arg
              Node* file = findFile(arg);
              // if it doesn't exist, error
              if(file == nullptr)
              {
                std::cout << "rm: File '" << arg << "' not found\n";
              }
              // if file is a directory
              else if(file->isDir)
              {
                // error
                std::cout << "rm: cannot remove '" << arg 
                          << "': Is a directory\n";
              }
              // else is valid so delete
              else
              {
                file->parent->DeleteChild(file);
                delete file;
              }
            }
          }
        }
        // Handles rmdir command
        else if (command == "rmdir")
        {
          // if there are no args
          if(args.size() == 0)
          {
            // error
            std::cout << "rm: Invalid use - For help use: help rmdir\n";
          }
          // has args
          else
          {
            // iterate over args
            for(auto arg : args)
            {
              //find file
              Node* file = findFile(arg);
              // if not found
              if(file == nullptr)
              {
                // error
                std::cout << "rm: File '" << arg << "' not found\n";
              }
              // if file is not a directory
              else if(!file->isDir)
              {
                // error
                std::cout << "rm: failed to remove '" << arg 
                          << "': Not a directory\n";
              }
              // if there is stuff in the file,
              else if(file->children.size() > 0)
              {
                // error
                std::cout << "rm: failed to remove '" << file->Name() 
                          << "': Directory not empty\n";
              }
              // else delete files
              else
              {
                // if we try to delete this directory, move back a directory
                if(file == curDir)
                  curDir = file->parent;
                // delete the file if it isn't the root.
                if(file != rootFile)
                {
                  file->parent->DeleteChild(file);
                  delete file;
                }
                // else error
                else std::cout << "rm: Permission Denied\n";
              }
            }
          }
        }
        // Handle chmod command
        else if(command == "chmod")
        {
          // int for conversion
          int permInt;
          // if there are more than 2 args
          if(args.size() > 2)
          {
            // error
            std::cout << 
              "chmod: Too many arguments - For help use: help chmod\n";
          }
          // if args is less 2 
          else if(args.size() < 2)
          {
            // error
            std::cout << 
              "chmod: Not enough arguments - For help use: help chmod\n";
          }
          else
          {
            try
            {
              // convert the number to an int
              permInt = std::stoi(args[0]);
              // make sure it is a valid 
              if(permInt > 777 || permInt < 0)
              {
                // error
                std::cout <<
                  "chmod: Invalid permission number\n";
              }
              else 
              {
                // iterate over args
                for(auto arg : args)
                {
                  // if is the first ignore
                  if(arg == args[0]) continue;
                  // try to find file
                  Node* file = findFile(arg);
                  // if file doesn't exist
                  if(file == nullptr)
                  {
                    // error 
                    std::cout << "File '" << arg << "' does not exist\n";
                  }
                  // else
                  else
                  {
                    // break up the digit
                    file->perms = 
                    {
                      permInt / 100 % 10, 
                      permInt / 10 % 10, 
                      permInt % 10
                    };
                  }
                }
              }
            }
            catch(const std::exception&)
            {
              std::cout << "chmod: " << args[0] << " invalid Permissions\n";
            }
          }
        }
        // Handle help command
        else if(command == "help")
        {
          if(args.size() == 0 || args[0] == "")
          {
            std::cout << "Usage: help cmd : prints help for a given command\n";
            std::cout << "Usage: help -a : prints help for all avaliable commands\n";
          }
          else if(args[0] == "-a")
          {
            for(std::string cmd : CMDS)
            {
              commands("help", std::vector<std::string>{cmd});
            }
            
          }
          else if(args[0] == "ls")
          {
            std::cout << "Usage: ls : prints files in directory\n";
            std::cout << "Usage: ls -l : prints files in directory"
                      << " with extra information\n";
          }
          else if(args[0] == "pwd")
          {
            std::cout << "Usage: pwd : prints working file directory\n"; 
          }
          else if(args[0] == "exit")
          {
            std::cout << "Usage: exit : exits console\n";
          }
          else if(args[0] == "mkdir")
          {
            std::cout << "Usage: mkdir dir... : makes the directories listed\n";
          }
          else if(args[0] == "touch")
          {
            std::cout << "Usage: touch file... :will make a file if "
                      << "doesn't exist\n";
          }
          else if(args[0] == "cd")
          {
            std::cout << "Usage: cd dir : changes current working directory\n";
          }
          else if(args[0] == "rm")
          {
            std::cout << "Usage: rm file... : removes the files listed\n";
          }
          else if(args[0] == "rmdir")
          {
            std::cout << "Usage: rmdir dir... : removes the directories "
                      << "listed\n";
          }
          else if(args[0] == "chmod")
          {
            std::cout << "Usage: chmod ### file/dir... : changes permissions"
                      << " of files/directories listed\n";
          }
          else 
          {
            std::cout << "help: command doesn't exist\n";
          }
        }
        // else handle no command found
        else
        {
          std::cout << "Command '" << command << "' not found.\n";
        }
        
        return true;
      }
      // returns the current working directory
      std::string pwd()
      {
        std::string dir;
        // make a new tracker
        Node* traverse = curDir;
        // check to see if we are on the root.
        if(traverse == rootFile)
        {
          dir = "/";
        }
        // if we are not, work backwards
        while(traverse != rootFile)
        {
          if(traverse->IsDir())
            dir = "/" + traverse->Name() + dir; 

          traverse = traverse->Parent();
        }
        // return directory 
        return dir;
      }
      // Finds the file or not, takes a path and returns a pointer
      // pointer is null if it wasn't found
      Node* findFile(std::string path)
      {
        Node* next = curDir;
        // stream for parsing
        std::stringstream pathStream(path);
        bool succeed = true;
        // while there exists stuff to parse
        while(pathStream.peek() != std::char_traits<char>::eof())
        {
          std::string dir;
          // delimited on /
          getline(pathStream, dir, '/');
          // check if we are looking at the root of the directory
          if(dir == "" && pathStream.peek() == std::char_traits<char>::eof())
          {
            next = rootFile;
          }
          else
          {
            // if find .. then go to parent
            if(dir == "..")
            {
              next = next->parent;
            }
            // if . then stay
            else if(dir == ".")
            {
              // Nothing needed to do.
            }
            // else look for child.
            else
            {
              auto found = next->children.find(dir);
              succeed = found != next->children.end();
              // if looking and didn't find stop
              if(!succeed)
                break;
              else next = next->children[dir];
            }
          }
        }
        // return if we found it or no
        return succeed ? next : nullptr;
      }
  };
}
#endif
