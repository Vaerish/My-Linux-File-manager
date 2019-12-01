
#include <string>


#ifndef USER_H
#define USER_H

namespace Shell
{
  // Basic user class built for future use.
  class User
  {
    private:
      std::string uname;
      std::string primary_group;
      std::vector<std::string> group_list;
      bool isAdmin;
    public:
      // Constructor
      User(std::string name, std::string g, bool a)
      {
        uname = name;
        primary_group = g;
        isAdmin = a;
        group_list.push_back(g);
      }
      // getters
      std::string Username() const { return uname; }
      std::string Group() const { return primary_group; }
      bool IsAdmin() const { return isAdmin; }
      // Functions
      // Adds a group to the group list.
      void addGroup(const std::string group_name)
      {
        group_list.push_back(group_name);
      }
      // Removes a group from the group list. If the name doesnt exist it, nothing will happen.
      void removeGroup(const std::string group_name)
      {
        for(int i = 0; (unsigned) i < group_list.size(); i++)
        {
          if (group_list.at(i) == group_name)
          {
            group_list.erase(group_list.begin() + i);
            // If the primary group was this then it will set it to be Users by default
            if (primary_group == group_name)
            {
              primary_group = "Users";
            }
            break;
          }
        }
      }
      //Prints out group members
      void printGroupList()
      {
        for (int i = 0; (unsigned) i < group_list.size(); i++)
        {
          std::cout << group_list.at(i) << " ";
        }
        std::cout << std::endl;
      }
      //Returns true if submitted group is part of the group list
      bool contains(const std::string gname)
      {
        for (int i = 0; (unsigned) i < group_list.size(); i++)
        {
          if (group_list.at(i) == gname)
          {
            return true;
          }
        }
        return false;
      }
      // Sets primary group
      void setPrimaryGroup (const std::string gname)
      {
        primary_group = gname;
      }
  };

} //

#endif
