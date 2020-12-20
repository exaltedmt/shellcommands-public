#include "Shell.h"

using std::cout;
using std::endl;
using std::string;

/**
 * Shell project! Recreate a bash-like shell!
 * \author Dustin Nguyen, Royce Harley, Trevin Metcalf
 */

Shell::Shell() {

  sh.terminal = STDIN_FILENO;
  sh.interactive = isatty(sh.terminal);

  if(sh.interactive) {

    while (tcgetpgrp(sh.terminal) != (sh.pgid = getpgrp())) kill (sh.pgid, SIGTTIN);

    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGCHLD, SIG_IGN);
    
    sh.pgid = getpid();
    if (setpgid(sh.pgid, sh.pgid) < 0) {
      perror("Couldn't put the shell in its own process group");
      exit(EXIT_FAILURE);
    }

    tcsetpgrp(sh.terminal, sh.pgid);
    tcgetattr(sh.terminal, &sh.tmodes);
  }
} // Shell Constructor - Citation: GNU Ch.28

void Shell::print_info() const {
  cout << endl << "Job STDIN  = " << sh.stdin_jobs[sh.stdin-1] << endl
       << "Job STDOUT = " << sh.stdout_jobs[sh.stdout-1] << endl
       << "Job STDERR = " << sh.stderr_jobs[sh.stderr-1] << endl << endl
       << sh.pipes << " pipe(s)" << endl
       << sh.proc  << " process(es)" << endl;
  
  string ** proc_ls = sh.proc_ls;
  
  for (uint count = 0; count < argc;) {
    if (proc_ls[count][0].empty()) {
      count++;
      continue;
    } // if
    cout << endl << "Process " << count << " argv: " << endl;
    for (uint count2 = 0; count2 < argc;) { 
      if (proc_ls[count][count2].empty()) {
	count++;
	count2 = argc;
      } else if (count2 == argc-1) {
	cout << count2 << ": " << proc_ls[count][count2] << endl;
	count++;
	count2++;
      } else {
	cout << count2 << ": " << proc_ls[count][count2] << endl;
	count2++;
      } // else
    } // for
  } // for
} // print_info

int Shell::getinfo(char ** argv) {
  sh.pipes = 0;
  sh.proc = 0;
  sh.stdin = 0;
  sh.stdout = 0;
  sh.stderr = 0;
  
  string val = "", preval = "";
  
  sh.proc_ls = new string * [argc];
  for (uint index = 0; index < argc; index++) {
    sh.proc_ls[index] = new string[argc];
  } // for
  
  for (uint index = 0; index < argc; index++) {
    for (uint index2 = 0; index2 < argc; index2++) {
      sh.proc_ls[index][index2] = "";
    } // for
  } // for
  
  for (uint count = 0; count < argc; count++) {
    val = argv[count];
    if (val == ">" || val == ">>") sh.stdout++;
    else if (val == "<") sh.stdin++;
    else if (val == "e>>" || val == "e>") sh.stderr++;
  } // for
  
  if (sh.stdout == 0) {
    sh.stdout_jobs = new string [1];
    sh.stdout_redir = new bool [1];
    sh.stdout = 1;
    sh.stdout_jobs[0] = strdup("STDOUT_FILENO");
    sh.stdout_redir[0] = false;
  } else {
    sh.stdout_jobs = new string [sh.stdout];
    sh.stdout_redir = new bool [sh.stdout];
  } // else 

  if (sh.stdin == 0) {
    sh.stdin_jobs = new string [1];
    sh.stdin = 1;
    sh.stdin_jobs[0] = strdup("STDIN_FILENO");
  } else sh.stdin_jobs = new string [sh.stdin];
  
  if (sh.stderr == 0) {
    sh.stderr_jobs = new string[1];
    sh.stderr_redir = new bool [1];
    sh.stderr = 1;
    sh.stderr_jobs[0] = strdup("STDERR_FILENO");
    sh.stderr_redir[0] = false;
  } else {
    sh.stderr_jobs = new string [sh.stderr];
    sh.stderr_redir = new bool [sh.stderr];
  } // else 

  for (uint index = 0, args = 0, count = 0, incount = 0, outcount = 0, errcount = 0; count < argc; count++) {
    val = argv[count];
    
    if (val != ">" && val != "<" && val != ">>" && val != "e>>" && val != "e>" && count > 0 && 
  ((preval = argv[count-1]) == ">" || preval == "<" || preval == ">>" || preval == "e>" || preval == "e>>" || (args > 0 && sh.proc_ls[index][args-1] == ""))) {
      sh.proc_ls[index][args] = "";
      args++;
    } // if

    else if (val != ">" && val != "<" && val != "|" && val != ">>" && val != "e>" && val != "e>>") {
      if (val != "" && count == 0) sh.proc++;
      sh.proc_ls[index][args] = argv[count];
      args++;
    } // else if
    
    else if (val == "|" && count != 0) {
      sh.pipes++;
      args = 0;
      index++;
      sh.proc++;
    } // else if
    
    else if (count+1 != argc) {
      if (val == "<" && count != 0) {
	sh.stdin_jobs[incount] = argv[count+1];
	incount++;
      } // if
      
      else if (val == ">" && count != 0) {
	sh.stdout_jobs[outcount] = argv[count+1];
	sh.stdout_redir[errcount] = false;
	outcount++;
      } // else if
      
      else if (val == ">>" && count != 0) {
	sh.stdout_jobs[outcount] = argv[count+1];
	sh.stdout_redir[errcount] = true;
	outcount++;
      } // else if
      
      else if (val == "e>" && count != 0) {
	sh.stderr_jobs[errcount] = argv[count+1];
	sh.stderr_redir[errcount] = false;
	errcount++;
      } // else if
      
      else if (val == "e>>" && count != 0) {
	sh.stderr_jobs[errcount] = argv[count+1];
	sh.stderr_redir[errcount] = true;
	errcount++;
      } // else if      
      
      else return -1;
    } // else if
  } // for
  return 0;    
} // getinfo

void Shell::execute(const proc_t & proc_no) {
  char ** proc = new char * [argc+1];
  for (uint index = 0; index < argc+1; index++) { 
    if (index == argc || sh.proc_ls[proc_no-1][index].empty()) proc[index] = nullptr;
    else {
      proc[index] = strdup(sh.proc_ls[proc_no-1][index].c_str());
    } //getting just individual processes
  } // for
  
  execvp(proc[0],proc);
  perror("1730sh: exec"); 
  for (uint i = 0; i < argc+1; i++) delete [] proc[i];
  delete [] proc;
  exit(EXIT_FAILURE);
} // execute

void Shell::redirect_out() {
  char buf[1];
  int id;

  if (sh.stdout_jobs[0] != "STDOUT_FILENO") {
    for (uint index = 0; index < sh.stdout; index++) { 
      if (sh.stdout_redir[index]) id = open(sh.stdout_jobs[index].c_str(), O_RDWR | O_CREAT | O_TRUNC, 0644);
      else id = open(sh.stdout_jobs[index].c_str(), O_RDWR | O_CREAT | O_APPEND, 0644);
      if (index != sh.stdout-1) close(id);
    } // for
    while ((read(STDIN_FILENO,buf,1) > 0)) write(id, buf, 1);
    close(id);
  } // if

  if (sh.stderr_jobs[0] != "STDERR_FILENO") {
    for (uint index = 0; index < sh.stderr; index++) {
      if (sh.stdout_redir[index]) id = open(sh.stdout_jobs[index].c_str(), O_RDWR | O_TRUNC | O_CREAT, 0644);
      else id = open(sh.stdout_jobs[index].c_str(), O_RDWR | O_APPEND | O_CREAT, 0644);
      if (index != sh.stderr-1) close(id);
    } // for
    while ((read(STDERR_FILENO, buf, 1) > 0)) write(id, buf, 1);
    close(id);
  } // if

} // redirect_out

void Shell::redirect_in() {
  struct stat stats;
  char * buf;
  int id;

  if (sh.stdin_jobs[0] != "STDIN_FILENO") {
    if ((id = open(sh.stdin_jobs[0].c_str(), O_RDONLY)) == -1) perror("open");    
    else {
      stat(sh.stdin_jobs[0].c_str(), &stats);
      buf = new char [stats.st_size];
      read(id, buf, stats.st_size);
      write(STDOUT_FILENO, buf, stats.st_size);
      delete [] buf;
    } // else
  } // if
} // redirect_in

void Shell::setargc(const uint & argc) {
  this->argc = argc;
} // setargc
  
uint & Shell::getargc() {
  return argc;
} // getargc

Shell::shinfo & Shell::getshinfo() {
  return sh;
} // getshinfo()

void Shell::store_j(int pos, job j){
  sh.job_list[pos] = j;
}

void Shell::store_js(int pos, int status){
  sh.job_list[pos].j_status = status;
}




