
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
      std::string group;
      bool isAdmin;
      std::string pword;
    public:
      // Constructor
      User(std::string name, std::string g, bool a, std::string pass) 
      {
        uname = name;
        group = g;
        isAdmin = a;
        pword = pass;
      }
      // getters
      std::string Username() const { return uname; }
      std::string Group() const { return group; }
      bool IsAdmin() const { return isAdmin; }
      // verify password bc not gonna expose password
      bool VerifyPassword(std::string pass) const { return pword == pass; }
  
  };
} // 

#endif