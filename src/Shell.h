#ifndef SHELL_H
#define SHELL_H

#include <unistd.h>
#include <cstdlib>
#include <iostream>
#include <string>
#include <cstring>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <climits>
#include <signal.h>
#include <termios.h>
#include <dirent.h>
#include <ctype.h>
#include <stdio.h>
#include <map>
#include <exception>

typedef unsigned int uint;
typedef uint pipe_t;
typedef uint proc_t;
typedef uint redir_t;

/* A job is a pipeline of processes.  */
typedef struct job
{
  pid_t pid[100];             // hold process pids
  int completed[100];         // hold completed/stopped value of processes
  int status[100];            // hold statuses
  int j_status;
  pid_t pgid;                 
  std::string args;        // for jobs list 
  struct termios j_tmodes;      
} job;

class Shell {

 private:
  
  uint argc = 0;
  struct shinfo {
    std::string * stdin_jobs;
    std::string * stdout_jobs;
    std::string * stderr_jobs;
    std::string ** proc_ls;
    pipe_t pipes = 0;
    proc_t proc = 0;
    redir_t stdin = 0;
    redir_t stdout = 0;
    redir_t stderr = 0;
    bool * stdout_redir;
    bool * stderr_redir;

    job job_list[100];
    int bg[100];
    pid_t pgid;
    struct termios tmodes;
    int terminal;
    int interactive;
  } sh; // shinfo
 
 public:

  Shell(); // Constructor, sets signal disposition

  /**
   * Print the info within the shinfo struct. 
   * @note must run getinfo() before printing!
   */

  void print_info() const;

  /**
   * Parses the provided array and fills the shinfo struct.
   */

  int getinfo(char ** argv);

  /**
   * Executes the command at the proc_no index of the proc_ls array.
   */

  void execute(const proc_t & proc_no);

  /**
   * Handles the redirection of standard error and standard output.
   */

  void redirect_out();
  
  /**
   * Handles the redirection of standard input.
   */

  void redirect_in();

  /**
   * Set amount of args provided in the command line.
   */

  void setargc(const uint & argc);

  /**
   * Retrieve amount of args detected in the command line.
   */

  uint & getargc();

  /**
   * Retrieve the shinfo struct.
   */

  struct shinfo & getshinfo(); 

  /*  Store job*/

  void store_j(int pos, job j);

  /*  Store j_status */

  void store_js(int pos, int status);

}; // Shell
  
#endif // SHELL_H
  
