#include "history.hpp"
#include <string>
#include <fstream>
#include <vector>
#include <numeric>
#include <iostream>
#include <stack>
#include <cstdio>
#include <stdlib.h>

static const std::string refresh = "refresh";
static const std::string exit_msg = "exit";
static const std::string undo = "undo";
static const char pre_entry = 'p';
static const char main_entry = 'm';

static const std::string main_start = "\nint main() {\n";
static const std::string main_end = "\n}\n";

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
static const std::string slash = "\\";
#else
static const std::string slash = "/";
#endif

static const std::string exec_command = "g++ shell_out.cpp -o shell_out.out && ." + slash + "shell_out.out";

inline std::string History::join_main()
{
  return std::accumulate(main_func.begin(), main_func.end(), std::string(""));
}

inline std::string History::join_pre()
{
  return std::accumulate(pre_lines.begin(), pre_lines.end(), std::string(""));
}

inline std::string History::semi_check(std::string str)
{
  return str[str.size() - 1] == ';' ? str : str + ';';
}

void History::execute()
{
  std::ofstream out("shell_out.cpp");
  out << join_pre() << main_start << join_main() << main_end;
  out.close();
  system(exec_command.c_str());
}

int History::check_command(std::string str)
{
  if (str == refresh)
  {
    main_func.clear();
    pre_lines.clear();
    execute();
    return 1;
  }
  else if (str == exit_msg)
  {
    std::remove("shell_out.cpp");
    std::remove("shell_out.out");
    return 2;
  }
  else if (str == undo)
  {
    if (instructions.empty())
    {
      std::cout << "Nothing to undo" << std::endl;
      return 1;
    }

    char instr = instructions.top();
    instructions.pop();
    if (instr == pre_entry)
    {
      pre_lines.pop_back();
      execute();
    }
    else if (instr == main_entry)
    {
      main_func.pop_back();
      execute();
    }

    return 1;
  }
  return 0;
}

bool History::add(std::string str)
{
  if (str.empty())
    return true;

  int command = check_command(str);
  if (command == 1)
    return true;
  if (command == 2)
    return false;

  if (str[0] == '#')
  {
    pre_lines.push_back(str + '\n');
    instructions.push(pre_entry);
    execute();
    return true;
  }

  std::string pre_processed = semi_check(str) + '\n';

  if (str.size() > 4 && str.substr(0, 5) == "using")
  {
    pre_lines.push_back(pre_processed);
    instructions.push(pre_entry);
    execute();
    return true;
  }

  main_func.push_back(pre_processed);
  instructions.push(main_entry);
  execute();
  return true;
}