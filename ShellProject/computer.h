#include <vector>
#include <iostream>
#include "node.h"
#include <limits>
#include <sstream>
#include <map>
#include <string>
#include <random>
#include "schedulers.h"

#ifndef COMPUTER_H
#define COMPUTER_H
extern bool doneCore;
extern vector<Process> core1;
extern vector<Process> core2;
extern vector<string> schedHist;
extern int schedCh;

namespace Shell //
{
  const std::string CMDS[] = {"", "ls", "pwd", "exit", "mkdir", "touch", "cd", "rm", "rmdir", "chmod", "useradd", "chuser", "groupadd", "usermod", "chown", "chgrp", "userdel", "groupdel", "groups", "users", "run", "ps", "kill", "schedHist", "algorithm"};

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
      // Creating lists to track users and groups
      std::vector<User> user_list;
      std::vector<std::string> group_list;
      // FLAG: SET TO FALSE BEFORE USING. DO NOT USE OUTSIDE THE PARSER.
      bool parser_flag = false;
      // String used to hold temporary strings
      std::string temp_string = "";
      std::string temp_token = "";
      vector<Process> procList;
      //whether it is in first or second core
      bool firstCore = true;
      //States if the current user is in the same group
      //as the file it is getting permissions from.
      bool InGroup = false;
      //String used to hold the permission section that
      //a specific user refrences to when deciding
      //if they have permission to do something or not
      std::string PermissionSection = "";

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
        //delete curUser;
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
        rootFile = new Node("", true, nullptr, 0, "root", "Users"); //Was "root" "root" // If this changes back then in the resetGroup function it would need to change back as well
        // Set the root's parent to itself - makes it auto handle ../ on root.
        // simple hack to make my life easier down the line.
        rootFile->parent = rootFile;
        // Make the root user.
        //curUser = new User("root", "root", true);
        user_list.push_back(User("root", "Users", true));
        group_list.push_back("Users");
        curUser = &user_list[0];
        // set the computer name.
        computerName = "computer";
        // move the current location to the root - this will change depending
        // on who logs in
        curDir = rootFile;
        // create home and root directory
        rootFile->AddChild(new Node("root", true, rootFile, curUser->Username(), curUser->Group()));
        rootFile->AddChild(new Node("home", true, rootFile, curUser->Username(), curUser->Group()));
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
          doneCore = looping;
        }
      }
      //Returns -1 if user cannot be found or returns the index
      int findUser(const std::string uname)
      {
        for (int i = 0; (unsigned) i < user_list.size(); i++)
        {
          if (user_list.at(i).Username() == uname)
            return i;
        }
        return -1;
      }
      //Returns -1 id the group cannot be found or returns the index
      int findGroup(const std::string gname)
      {
        for (int i = 0; (unsigned) i < group_list.size(); i++)
        {
          if (group_list.at(i) == gname)
            return i;
        }
        return -1;
      }
      //Function that sets all of the objects owned by a specified user back to root
      void resetUser(const std::string uname, Node* file)
      {
        if (file->User() == uname)
        {
          file->setUser("root");
        }
        // Make sure all child nodes are reset in the same way
        for (auto child : file->Children())
        {
          resetUser(uname, child.second);
        }
      }
      // Function that sets all of the objects owned by a specific group back to root
      void resetGroup(const std::string gname, Node* file)
      {
        if (file->Group() == gname)
        {
          file->setGroup("Users");
        }
        // Make sure all child nodes are reset in the same way
        for (auto child : file->Children())
        {
          resetGroup(gname, child.second);
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

              PermissionSection = curDir->PermsStr();
              //Finds if current User in in same group as object
              for (int i = 0; (unsigned) i < user_list.size(); i++)
              {
                if (user_list.at(i).Username() == curUser->Username())
                {
                  parser_flag = true;
                  InGroup = user_list.at(i).contains(curDir->Group());
                }
              }
              //Checks if root, owner, same group, or public and assigns PermissionSection accordingly
              if(curUser->Username() == "root")
              {
                PermissionSection = "rwx";
              }
              else if(curUser->Username() == curDir->User())
              {
                PermissionSection = PermissionSection.substr(1,3);
              }
              else if(InGroup)
              {
                PermissionSection = PermissionSection.substr(4,3);
              }
              else
              {
                PermissionSection = PermissionSection.substr(7,3);
              }
              if (PermissionSection.find('r') != std::string::npos)
              {
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
                std::cout << "Permission Denied due to not having read permissions" << std::endl;
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
            PermissionSection = curDir->PermsStr();
            //Finds if current User in in same group as object
            for (int i = 0; (unsigned) i < user_list.size(); i++)
            {
              if (user_list.at(i).Username() == curUser->Username())
              {
                parser_flag = true;
                InGroup = user_list.at(i).contains(curDir->Group());
              }
            }
            //Checks if root, owner, same group, or public and assigns PermissionSection accordingly
            if(curUser->Username() == "root")
            {
              PermissionSection = "rwx";
            }
            else if(curUser->Username() == curDir->User())
            {
              PermissionSection = PermissionSection.substr(1,3);
            }
            else if(InGroup)
            {
              PermissionSection = PermissionSection.substr(4,3);
            }
            else
            {
              PermissionSection = PermissionSection.substr(7,3);
            }
            if (PermissionSection.find('r') != std::string::npos)
            {
              // display simple output
              for(auto child : curDir->Children())
              {
                  Node* childSec = child.second;
                  PermissionSection = childSec->PermsStr();
                      std::cout << (child.second->IsDir() ? "\033[34m" : "")
                                << child.first << "\033[0m ";
              }
              // adds a new line at the end only if there was something to print
              if(curDir->Children().size() > 0)
                std::cout << std::endl;
            }
            else
            {
              std::cout << "Permission Denied due to not having read permissions" << std::endl;
            }
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
            PermissionSection = curDir->PermsStr();
            //Finds if current User in in same group as object
            for (int i = 0; (unsigned) i < user_list.size(); i++)
            {
              if (user_list.at(i).Username() == curUser->Username())
              {
                parser_flag = true;
                //Print group list for that user
                 InGroup = user_list.at(i).contains(curDir->Group());
              }
            }
            //Checks if root, owner, same group, or public and assigns PermissionSection accordingly
            if(curUser->Username() == "root")
            {
              PermissionSection = "rwx";
            }
            else if(curUser->Username() == curDir->User())
            {
              PermissionSection = PermissionSection.substr(1,3);
            }
            else if(InGroup)
            {
              PermissionSection = PermissionSection.substr(4,3);
            }
            else
            {
              PermissionSection = PermissionSection.substr(7,3);
            }
            if (PermissionSection.find('w') != std::string::npos)
            {
            // iterate over arguments
              for(std::string arg : args)
              {
                // Attempt to add new directory if fails output such a message.
                if(!curDir->AddChild(new Node(arg, true, curDir, curUser->Username(), curUser->Group())))
                {
                  std::cout << "mkdir: cannot create directory '" << arg << "': File exits\n"; 
                }
              }
            }
            else
            {
              std::cout << "Permission Denied due to not having write permissions" << std::endl;
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
            PermissionSection = curDir->PermsStr();
            //Finds if current User in in same group as object
            for (int i = 0; (unsigned) i < user_list.size(); i++)
            {
              if (user_list.at(i).Username() == curUser->Username())
              {
                parser_flag = true;
                InGroup = user_list.at(i).contains(curDir->Group());
              }
            }
            //Checks if root, owner, same group, or public and assigns PermissionSection accordingly
            if(curUser->Username() == "root")
            {
              PermissionSection = "rwx";
            }
            else if(curUser->Username() == curDir->User())
            {
              PermissionSection = PermissionSection.substr(1,3);
            }
            else if(InGroup)
            {
              PermissionSection = PermissionSection.substr(4,3);
            }
            else
            {
              PermissionSection = PermissionSection.substr(7,3);
            }
            if (PermissionSection.find('w') != std::string::npos)
            {
              // iterate over args
              for(std::string arg : args)
              {
                // try to add and if that fails, update the current timestamp
                if(!curDir->AddChild(new Node(arg, false, curDir, curUser->Username(), curUser->Group())))
                {
                    PermissionSection = curDir->children[arg]->PermsStr();
                    //Finds if current User in in same group as object
                    for (int i = 0; (unsigned) i < user_list.size(); i++)
                    {
                      if (user_list.at(i).Username() == curUser->Username())
                      {
                        parser_flag = true;
                        InGroup = user_list.at(i).contains(curDir->children[arg]->Group());
                      }
                    }
                    //Checks if root, owner, same group, or public and assigns PermissionSection accordingly
                    if(curUser->Username() == "root")
                    {
                      PermissionSection = "rwx";
                    }
                    else if(curUser->Username() == curDir->children[arg]->User())
                    {
                      PermissionSection = PermissionSection.substr(1,3);
                    }
                    else if(InGroup)
                    {
                      PermissionSection = PermissionSection.substr(4,3);
                    }
                    else
                    {
                      PermissionSection = PermissionSection.substr(7,3);
                    }
                    if (PermissionSection.find('w') != std::string::npos)
                    {
                        curDir->children[arg]->UpdateTimeStamp();
                    }
                    else
                    {
                      std::cout << "Permission Denied due to not having write permissions" << std::endl;
                    }
                }
              }
            }
            else
            {
              std::cout << "Permission Denied due to not having write permissions" << std::endl;
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
              {
                PermissionSection = file->PermsStr();
                //Finds if current User in in same group as object
                for (int i = 0; (unsigned) i < user_list.size(); i++)
                {
                  if (user_list.at(i).Username() == curUser->Username())
                  {
                    parser_flag = true;
                    InGroup = user_list.at(i).contains(file->Group());
                  }
                }
                //Checks if root, owner, same group, or public and assigns PermissionSection accordingly
                if(curUser->Username() == "root")
                {
                  PermissionSection = "rwx";
                }
                else if(curUser->Username() == file->User())
                {
                  PermissionSection = PermissionSection.substr(1,3);
                }
                else if(InGroup)
                {
                  PermissionSection = PermissionSection.substr(4,3);
                }
                else
                {
                  PermissionSection = PermissionSection.substr(7,3);
                }
                if (PermissionSection.find('x') != std::string::npos || args[0] == ".." || args[0] == "/")
                {
                  // else set curDir to it
                  curDir = file;
                }
                else
                {
                  std::cout << "Permission Denied due to not having execute permissions" << std::endl;
                }
              }
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
            PermissionSection = curDir->PermsStr();
            //Finds if current User in in same group as object
            for (int i = 0; (unsigned) i < user_list.size(); i++)
            {
              if (user_list.at(i).Username() == curUser->Username())
              {
                parser_flag = true;
                InGroup = user_list.at(i).contains(curDir->Group());
              }
            }
            //Checks if root, owner, same group, or public and assigns PermissionSection accordingly
            if(curUser->Username() == "root")
            {
              PermissionSection = "rwx";
            }
            else if(curUser->Username() == curDir->User())
            {
              PermissionSection = PermissionSection.substr(1,3);
            }
            else if(InGroup)
            {
              PermissionSection = PermissionSection.substr(4,3);
            }
            else
            {
              PermissionSection = PermissionSection.substr(7,3);
            }
            if (PermissionSection.find('w') != std::string::npos)
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
            else
            {
              std::cout << "Permission Denied due to not having write permissions" << std::endl;
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
            PermissionSection = curDir->PermsStr();
            //Finds if current User in in same group as object
            for (int i = 0; (unsigned) i < user_list.size(); i++)
            {
              if (user_list.at(i).Username() == curUser->Username())
              {
                parser_flag = true;
                InGroup = user_list.at(i).contains(curDir->Group());
              }
            }
            //Checks if root, owner, same group, or public and assigns PermissionSection accordingly
            if(curUser->Username() == "root")
            {
              PermissionSection = "rwx";
            }
            else if(curUser->Username() == curDir->User())
            {
              PermissionSection = PermissionSection.substr(1,3);
            }
            else if(InGroup)
            {
              PermissionSection = PermissionSection.substr(4,3);
            }
            else
            {
              PermissionSection = PermissionSection.substr(7,3);
            }
            if (PermissionSection.find('w') != std::string::npos)
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
            else
            {
              std::cout << "Permission Denied due to not having write permissions" << std::endl;
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
                    PermissionSection = file->PermsStr();
                    //Finds if current User in in same group as object
                    for (int i = 0; (unsigned) i < user_list.size(); i++)
                    {
                      if (user_list.at(i).Username() == curUser->Username())
                      {
                        parser_flag = true;
                        InGroup = user_list.at(i).contains(file->Group());
                      }
                    }
                    //Checks if root, owner, same group, or public and assigns PermissionSection accordingly
                    if(curUser->Username() == "root")
                    {
                      PermissionSection = "rwx";
                    }
                    else if(curUser->Username() == file->User())
                    {
                      PermissionSection = PermissionSection.substr(1,3);
                    }
                    else if(InGroup)
                    {
                      PermissionSection = PermissionSection.substr(4,3);
                    }
                    else
                    {
                      PermissionSection = PermissionSection.substr(7,3);
                    }
                    if (PermissionSection.find('w') != std::string::npos)
                    {
                      // break up the digit
                      file->perms = 
                      {
                        permInt / 100 % 10, 
                        permInt / 10 % 10, 
                        permInt % 10
                      };
                    }
                    else
                    {
                      std::cout << "Permission Denied due to not having write permissions" << std::endl;
                    }
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
        else if(command == "cat")
        {
        // if no args
          if(args.size() == 0)
          {
            // error
            std::cout << "cat: Invalid use";
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
                  std::cout << "cat: File '" << arg << "' not found\n";
                }
                // if file is a directory
                else if(file->isDir)
                {
                  // error
                  std::cout << "cat: cannot display '" << arg 
                            << "': Is a directory\n";
                }
                // else is valid so display
                else
                {
                  PermissionSection = file->PermsStr();
                  //Finds if current User in in same group as object
                  for (int i = 0; (unsigned) i < user_list.size(); i++)
                  {
                    if (user_list.at(i).Username() == curUser->Username())
                    {
                      parser_flag = true;
                      InGroup = user_list.at(i).contains(file->Group());
                    }
                  }
                  //Checks if root, owner, same group, or public and assigns PermissionSection accordingly
                  if(curUser->Username() == "root")
                  {
                    PermissionSection = "rwx";
                  }
                  else if(curUser->Username() == file->User())
                  {
                    PermissionSection = PermissionSection.substr(1,3);
                  }
                  else if(InGroup)
                  {
                    PermissionSection = PermissionSection.substr(4,3);
                  }
                  else
                  {
                    PermissionSection = PermissionSection.substr(7,3);
                  }
                  if (PermissionSection.find('r') != std::string::npos)
                  {
                    std::cout << "Fake Fake Fake, blah blah blah is in this file. "
                              << "If you are seeing this then you did have permission to cat the file.\n";
                  }
                  else
                  {
                    std::cout << "Permission Denied due to not having read permissions" << std::endl;
                  }
                }
              }
          }
        }
        // Handles useradd command
        else if(command == "useradd")
        {
            // Creates new user and adds them to Users group & Sets that group as their primary
	    if (args.size() == 1)
            {
	      // Checking to make sure that this user does not already exist
              parser_flag = false;
              for (int i = 0; (unsigned)i < user_list.size(); ++i)
              {
                if (user_list.at(i).Username() == args[0])
                {
                  parser_flag = true;
                  break;
                }
              }
              // User already exists, send error message
              if (parser_flag)
              {
                std::cout << "Error: User '" << args[0] << "' already exists. Use 'users' command to see list of current users." << std::endl;
              }
              // User does not already exist: Create and add to nedded lists.
              else
              {
                user_list.push_back(User(args[0], "Users", false));
                curUser = &user_list[0]; //Update curUser pointer after altering vector
              }
	    }
            else if (args.size() == 3 && args[0] == "-G")
            {
              // Form of "useradd -G <group[,group]> <username>"
              // Adds new user to all listed groups if possible (group exists). Still sets Users to be default group.
              // Checking to make sure that this user does not already exist
              parser_flag = false;
              for (int i = 0; (unsigned)i < user_list.size(); ++i)
              {
                if (user_list.at(i).Username() == args[2])
                {
                  parser_flag = true;
                  break;
                }
              }
              // User already exists, send error message
              if (parser_flag)
              {
                std::cout << "Error: User '" << args[2] << "' already exists. Use 'users' command to see list of current users" << std::endl;
              }
              else // User does not already exist so create it and add it to all existing groups listed
              {
                user_list.push_back(User(args[2], "Users", false));
                curUser = &user_list[0]; //Update curUser pointer after altering vector
                // Search through listed groups and add to all existing ones
                temp_string = args[1];
                while (temp_string.find(",") != std::string::npos)
                {
                  parser_flag = false;
		  temp_token = temp_string.substr(0, temp_string.find(","));
                  // Only adds to users groups if the group already exists
                  for (int i = 0; (unsigned) i < group_list.size(); i++)
                  {
                    if (temp_token == group_list.at(i))
                    {
                      parser_flag = true;
                      user_list.back().addGroup(temp_token);
                    }
                  }
                  // For every group that does not exist, inform the user
                  if (!parser_flag)
                  {
                    std::cout << temp_token << ": This group does not exist. Can only add to existing groups." << std::endl;
                  }
                  temp_string.erase(0, temp_string.find(",") + 1);
                }
		parser_flag = false;
                // Only adds to users groups if the group already exists
                for (int i = 0;(unsigned) i < group_list.size(); i++)
                {
                  if (temp_string == group_list.at(i))
                  {
                    parser_flag = true;
                    user_list.back().addGroup(temp_string);
                  }
                }
                // For every group that does not exist, inform the user
                if (!parser_flag)
                {
                  std::cout << temp_string << ": This group does not exist. Can only add to existing groups." << std::endl;
                }
              }
            }
            else
            {
              // Else no arguments provided
	      std::cout << "Invalid use - For help use: help useradd\n";
            }
        }
        // Handles chuser command
        else if(command == "chuser")
        {
             if (args.size() == 1)
            {
              // For of "chuser <username>"
              // Change active user to one indicated if that user exists, otherwise fails
              // Check that requested user exists
              parser_flag = false;
              for (int i = 0; (unsigned) i < user_list.size(); i++)
              {
                if (user_list.at(i).Username() == args[0])
                {
                  // Since user exists, change curr user to that
                  parser_flag = true;
                  curUser = &user_list[i];
                  break;
                }
              }
              if (!parser_flag)
              {
                std::cout << args[0] << " not in user list. Can only change to existing users, see help for details." << std::endl;
              }
            }
            // Else no arguments provided
            else
            {
              std::cout << "Invalid use - For help use: help chuser\n";
            }
        }
        // Handles groupadd command
        else if(command == "groupadd")
        {
            if (args.size() == 1)
            {
              // Form of "groupadd <group>"
              // Creates new group and adds Root user to itself
              // Checks that the group does not already exists before adding it
              parser_flag = false;
              for (int i = 0; (unsigned) i < group_list.size(); i++)
              {
                if (group_list.at(i) == args[0])
                {
                  parser_flag = true;
                }
              }
              if (parser_flag)
              {
                std::cout << args[0] << " already exists" << std::endl;
              }
              else
              {
                // Adding group to group list
                group_list.push_back(args[0]);
		// Adding root to group
                user_list.at(0).addGroup(args[0]);
              }
            }
            else
            {
              // Else no arguments provided
              std::cout << "Invalid use - For help use: help groupadd\n";
            }
        }
        // Handles usermod command
        else if(command == "usermod")
        {
            if (args.size() == 3 && args[0] == "-g")
            {
              // Must have "usermod -g <group> <username>" format.
              // Fail if user or group doesnt exist or the user is not part of the group.
              if (findUser(args[2]) == -1)
              {
                std::cout << args[2] << "is not an existing user" << std::endl;
              }
              else //Both user and group exist
              {
                // Need to check that the user already is part of the group to be able to set it to its primary
		if (user_list.at(findUser(args[2])).contains(args[1]))
                {
                  user_list.at(findUser(args[2])).setPrimaryGroup(args[1]);
                }
                else
                {
                  std::cout << "User is not part of " << args[1] << " group, cannot set to primary" << std::endl;
                }
              }
            }
            else if (args.size() == 4 && args[0] == "-a" && args[1] == "-G")
            {
              // Must have "usermod -a -G <group> <username>" format.
              // Adds indicated user to groups list. Fails if the user or group doesnt already exist.
              if (findGroup(args[2]) == -1)
              {
                std::cout << args[2] << " is not an existing group" << std::endl;
              }
              else if (findUser(args[3]) == -1)
              {
                std::cout << args[3] << " is not an existing user" << std::endl;
              }
              else //Both user and group exist
              {
                user_list.at(findUser(args[3])).addGroup(args[2]); // Add group to users group list
              }
            }
            else
            {
              // Incorrect number of arguments provided
              std::cout << "Invalid use - For help use: help usermod\n";
            }
        }
        else if(command == "chown")
        {
            //Changes the owner of the indicated object to the indicated user. Fails if user or object doesnt exist or user doesnt have write permissions. 
	    if (args.size() == 2)
            {
              Node* file = findFile(args[1]);
              //Check that the user exists
              if (findUser(args[0]) == -1)
              {
                std::cout << args[0] << " is not an existing user" << std::endl;
              }
              //Check that the object exists
              else if (file == nullptr)
              {
                std::cout << args[1] << " is not an existing file or directory" << std::endl;
              }
              // Try and switch owner of object
              else
              {
                PermissionSection = file->PermsStr();
                //Finds if current User in in same group as object
                for (int i = 0; (unsigned) i < user_list.size(); i++)
                {
                  if (user_list.at(i).Username() == curUser->Username())
                  {
                    parser_flag = true;
                    InGroup = user_list.at(i).contains(file->Group());
                  }
                }
                //Checks if root, owner, same group, or public and assigns PermissionSection accordingly
                if(curUser->Username() == "root")
                {
                  PermissionSection = "rwx";
                }
                else if(curUser->Username() == file->User())
                {
                  PermissionSection = PermissionSection.substr(1,3);
                }
                else if(InGroup)
                {
                  PermissionSection = PermissionSection.substr(4,3);
                }
                else
                {
                  PermissionSection = PermissionSection.substr(7,3);
                }
                if (PermissionSection.find('w') != std::string::npos)
                {
                  //Set owner of object to specified user
                  file->setUser(args[0]);
                }
                else
                {
                  std::cout << "Permission Denied due to not having write permissions" << std::endl;
                }
              }
            }
            else
            {
              std::cout << "Invalid use - For help use: help chown\n";
            }
        }
        else if(command == "chgrp")
        {
            Node* file = findFile(args[1]);
            //Change group of the indicated object to indicated group. Fails if object or user doesnt exist as well as user not having permissins
            if (args.size() == 2)
            {
              //Check that the group exists
              if (findGroup(args[0]) == -1)
              {
                std::cout << args[0] << " is not an existing group" << std::endl;
              }
              //Check that the object exists
              else if (file == nullptr)
              {
                std::cout << args[1] << " is not an existing file or directory" << std::endl;
              }
              // Try and switch group of object
              else
              {
                PermissionSection = file->PermsStr();
                //Finds if current User in in same group as object
                for (int i = 0; (unsigned) i < user_list.size(); i++)
                {
                  if (user_list.at(i).Username() == curUser->Username())
                  {
                    parser_flag = true;
                    InGroup = user_list.at(i).contains(file->Group());
                  }
                }
                //Checks if root, owner, same group, or public and assigns PermissionSection accordingly
                if(curUser->Username() == "root")
                {
                  PermissionSection = "rwx";
                }
                else if(curUser->Username() == file->User())
                {
                  PermissionSection = PermissionSection.substr(1,3);
                }
                else if(InGroup)
                {
                  PermissionSection = PermissionSection.substr(4,3);
                }
                else
                {
                  PermissionSection = PermissionSection.substr(7,3);
                }
                if (PermissionSection.find('w') != std::string::npos)
                {
                  //Set owner of object to specified user
                  file->setGroup(args[0]);
                }
                else
                {
                  std::cout << "Permission Denied due to not having write permissions" << std::endl;
                }
              }
            }
            else
            {
              std::cout << "Invalid use - For help use: help chgrp\n";
            }
        }
        else if(command == "userdel")
        {
            //Removes user from the system if it exists and not root. Any owned objects become Root users property.
            if (args.size() == 1)
            {
              if (args[0] == "root")
              {
                std::cout << "Root user cannot be removed" << std::endl;
              }
              else if (findUser(args[0]) == -1)
              {
                std::cout << args[0] << " is not a current existing user" << std::endl;
              }
              else // Not root and an existing user. Valid.
              {
                temp_string = curUser->Username(); // Saves current user
                user_list.erase(user_list.begin() + findUser(args[0]) , user_list.begin() + findUser(args[0]) + 1);
                curUser = &user_list[findUser(temp_string)]; // Fixes the curUser pointer after altering the users list
		//Change any objects that were once owned by this user back to root
                resetUser(args[0], rootFile);
              }
            }
            //Removes listed user from indicated group. Fails if either doen't exist or user is not part of the group, the group is Users, or the user is Root
            else if (args.size() == 3 && args[0] == "-G")
            {
              if (findUser(args[2]) == -1) // Makes sure user exists
              {
                std::cout << args[2] << " is not an existing user" << std::endl;
              }
              else if (findGroup(args[1]) == -1) // Makes sure group exists
              {
                std::cout << args[1] << " is not an existing group" << std::endl;
              }
              else if (!user_list.at(findUser(args[2])).contains(args[1])) // Make sure user is part of group
              {
                std::cout << args[2] << " is not part of that group" << std::endl;
              }
              else if (args[1] == "Users")
              {
                std::cout << "Cannot remove User group from any user" << std::endl;
              }
              else if (args[2] == "root")
              {
                std::cout << "Cannot remove root from any group" << std::endl;
              }
              else // Valid, remove user from group
              {
                user_list.at(findUser(args[2])).removeGroup(args[1]);
              }
            }
            else
            {
              std::cout << "Invalid use - For help use: help userdel\n";
            }
        }
        else if(command == "groupdel")
        {
            //Removes an existing group from the system as long as its not root. All permissions for this group are changed to Users permissions.
            if (args.size() == 1)
            {
              if (args[0] == "root")
              {
                std::cout << "Root group cannot be removed" << std::endl;
              }
              else if (findGroup(args[0]) == -1)
              {
                std::cout << args[0] << " is not a current existing group" << std::endl;
              }
              else // Not root and an existing group. Valid.
              {
                // Erase group from groups list
                group_list.erase(group_list.begin() + findGroup(args[0]), group_list.begin() + findGroup(args[0]) + 1);
		// Erase groups from users group list. If any had this group as primary, set it to Users.
                for (int i = 0; (unsigned) i < user_list.size(); i++)
                {
                  user_list.at(i).removeGroup(args[0]);
                }
                // Switch permissions of anything belonging to this group to Users group
                resetGroup(args[0], rootFile);
              }
            }
            else
            {
              std::cout << "Invalid use - For help use: help groupdel\n";
            }

        }
        else if(command == "groups")
        {
            parser_flag = false;
            // Prints out all the groups associated with this user
            if (args.size() == 1)
            {
              for (int i = 0; (unsigned) i < user_list.size(); i++)
              {
                if (user_list.at(i).Username() == args[0])
                {
                  parser_flag = true;
                  //Print group list for that user
                  user_list.at(i).printGroupList();
                  break;
                }
              }
              if (!parser_flag)
              {
                std::cout << "User does not exist" << std::endl;
              }
            }
            else
            {
              std::cout << "Invalid use - For help use: help groups\n" << std::endl;
            }
        }
        else if(command == "users")
        {
          //Prints all known users
          for (int i = 0; (unsigned)i < user_list.size(); i++)
          {
            std::cout << user_list.at(i).Username() << " ";
          }
          std::cout << std::endl;
        }
        else if(command == "run")
        {
          // if no args
          if(args.size() == 0)
          {
            // error
            std::cout << "run: Invalid use";
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
                  std::cout << "run: File '" << arg << "' not found\n";
                }
                // if file is a directory
                else if(file->isDir)
                {
                  // error
                  std::cout << "run: cannot execute '" << arg 
                            << "': Is a directory\n";
                }
                // else is valid so execute
                else
                {
                  PermissionSection = file->PermsStr();
                  //Finds if current User in in same group as object
                  for (int i = 0; (unsigned) i < user_list.size(); i++)
                  {
                    if (user_list.at(i).Username() == curUser->Username())
                    {
                      parser_flag = true;
                      InGroup = user_list.at(i).contains(file->Group());
                    }
                  }
                  //Checks if root, owner, same group, or public and assigns PermissionSection accordingly
                  if(curUser->Username() == "root")
                  {
                    PermissionSection = "rwx";
                  }
                  else if(curUser->Username() == file->User())
                  {
                    PermissionSection = PermissionSection.substr(1,3);
                  }
                  else if(InGroup)
                  {
                    PermissionSection = PermissionSection.substr(4,3);
                  }
                  else
                  {
                    PermissionSection = PermissionSection.substr(7,3);
                  }
                  if (PermissionSection.find('x') != std::string::npos)
                  {
                    Process p;
                    p.id = file->name;
                    p.startTime = 0; //this will update in the thread as each new process is found by the schedular
                    p.totalTimeNeeded = file->time_run; //this makes it between ten and 50 units long
                    p.user = file->user;
                    if(firstCore) //determines which core the file that needs to be executed will be pushed to
                    {
                      core1.push_back(p);
                      firstCore = false;
                    }
                    else
                    {
                      core2.push_back(p);
                      firstCore = true;
                    }
                  }
                  else
                  {
                    std::cout << "Permission Denied due to not having execute permissions" << std::endl;
                  }
                }
              }
          }
        }
        else if(command == "ps") //calls both cores and says what needs to be run as long as it is not done and is currently running 
        {
            for(unsigned int i = 0; i < core1.size(); i++)
            {
              if(!core1[i].isDone && core1[i].timeScheduled != 1)
              {
                cout << core1[i].id <<  " " << core1[i].user << " " << core1[i].startTime << " " << core1[i].timeScheduled << " "<< core1[i].totalTimeNeeded << endl;
              }
            }
            for(unsigned int i = 0; i < core2.size(); i++)
            {
              if(!core2[i].isDone&& core1[i].timeScheduled != 1)
              {
                cout << core2[i].id << " " << core2[i].user << " " << core2[i].startTime << " " << core2[i].timeScheduled << " "<< core2[i].totalTimeNeeded << endl;
              }
            }
        }
        else if(command == "kill")
        {
          if(args.size() == 0)
          {
            // error
            std::cout << "kill: Invalid use";
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
                  std::cout << "kill: File '" << arg << "' not found\n";
                }
                // if file is a directory
                else if(file->isDir)
                {
                  // error
                  std::cout << "kill: cannot execute '" << arg 
                            << "': Is a directory\n";
                }
                // else is valid so execute
                else
                {
                    for(unsigned int i = 0; i < core1.size(); i++)
                  {
                  if(core1[i].id == file->name)
                    {
                    core1[i].isDone = true;
              }
            }
            for(unsigned int i = 0; i < core2.size(); i++)
            {
              if(core2[i].id == file->name)
              {
                core2[i].isDone = true;
              }
            }
                }
              }
          }
        }
        else if(command == "schedHist") //prints out list of schedHist from point it was called
        {
            for(unsigned int i = 0; i < schedHist.size(); i++)
            {
              cout << schedHist[i] << endl;
            }
        }
        // Commnad to allow the user to switch simulated execution algorithm
        else if(command == "algorithm")
        {
            for(auto arg : args) //schedCh communicates to threads what current algorithm is being used
              {
                if(arg == "RR")
                {
                  schedCh = 1;
                }
                else if(arg == "SPN")
                {
                  schedCh = 2;
                }
                else if(arg == "SRT")
                {
                  schedCh = 3;
                }
                else if(arg == "HRRN")
                {
                  schedCh = 4;
                }
                else if(arg == "FCFS")
                {
                  schedCh = 5;
                }
                else
                {
                  //list of all available options
                  cout << "In correct Option only available algorithms are " << endl;
                  cout << "RR" << endl;
                  cout << "SPN" << endl;
                  cout << "SRT" << endl;
                  cout << "HRRN" << endl;
                  cout << "FCFS" << endl;
                  cout << "All spelled like above." << endl;
                }
              }
        }
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
          else if(args[0] == "useradd")
          {
            std::cout << "Usage: useradd <username> : Creates a new user\n";
            std::cout << "Usage: useradd -G <group[,group]> <username> : Creates a new user\n";
            std::cout << "and adds them to the specified groups\n";
          }
          else if(args[0] == "chuser")
          {
            std::cout << "Usage: chuser <username> : Changes to indicated active user\n";
          }
          else if(args[0] == "groupadd")
          {
            std::cout << "Usage: groupadd <group> : Creates a new group\n";
          }
          else if(args[0] == "usermod")
          {
            std::cout << "Usage: usermod -g <group> <username> : Sets primary group for indicated\n";
            std::cout << "user to be indicated group\n";
            std::cout << "Usage: usermod -a -G <group> <username> : Add indicated user to indicated group\n";
          }
          else if(args[0] == "chown")
          {
            std::cout << "Usage: chown <username> <object> : Change owner of indicated object to indicated user\n";
          }
          else if(args[0] == "chgrp")
          {
            std::cout << "Usage: chgrp <group> <object> : Change objects group to indicated group\n";
          }
          else if(args[0] == "userdel")
          {
            std::cout << "Usage: userdel -G <group> <username> : Remove the indicated user from the indicated\n";
            std::cout << "group\n";
            std::cout << "Usage: userdel <username> : Removes indicated user from system\n";
          }
          else if(args[0] == "groupdel")
          {
            std::cout << "Usage: groupdel <group> : Remove the group from the system\n";
          }
          else if(args[0] == "groups")
          {
            std::cout << "Usage: groups <username> : Lists groups indicated user is part of\n";
          }
          else if(args[0] == "users")
          {
            std::cout << "Usage: users : List the known users to the system\n";
          }
          else if(args[0] == "run")
          {
            std::cout << "Usage: run <file> : Executes the indicated file\n";
          }
          else if(args[0] == "ps")
          {
            std::cout << "Usage: ps : Lists all currently running processes\n";
          }
          else if(args[0] == "kill")
          {
            std::cout << "Usage: kill <file> : Terminates indicated files execution\n";
          }
          else if(args[0] == "schedHist")
          {
            std::cout << "Usage: schedHist : Outputs the scheduling history\n";
          }
          else if(args[0] == "algorithm")
          {
            std::cout << "Usage: algorithms <RR, SPN, SRT, HRRN, FCFS> : Changes the current scheduling algorithm to the indicated algorithm\n";
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
