
#include <string>


#ifndef USER_H
#define USER_H

namespace Shell
{
  // basic user class built for future use.
  // Mostly use to store a name and password atm
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
  };

} //

#endif
