#include "builtin.h"

/**
 * Change directory.
 */

void cd(const char * path) {
  DIR * dir = opendir(path);
  if (dir == NULL) perror ("1730sh: cd");
  else {

    int err_catch = 0;

    if((err_catch = chdir(path) == -1)){
    perror("cd");
  }

    closedir(dir);
  } // else
} // cd

void kp(int signo, pid_t pid){
  kill(pid,signo);
}

void jl(Shell & sh, int j_ctr){

  cout << "JID" << "   " << "STATUS" << "        " << "COMMAND" << endl;
  for(int i = 0; i < j_ctr; ++i){
    cout << sh.getshinfo().job_list[i].pgid << "  " << sh.getshinfo().job_list[i].j_status
    << "  " << sh.getshinfo().job_list[i].args;
  }
}

void exp(const char * word){
  char buff[500];
  strcpy(buff,word);
  putenv(buff);
}

void bg(pid_t pgid, Shell & sh){

   for(int pos = 0; pos < 100; ++pos){
    if(sh.getshinfo().job_list[pos].pgid == pgid){
      kill(pgid, SIGCONT);
      break;
    } // finding pgid
  } // iterate
} // Citation: GNU 28.4

int fg(pid_t pgid, Shell & sh){

  int status = 0;
  int spot = 0;

   for(int pos = 0; pos < 100; ++pos){
    if(sh.getshinfo().job_list[pos].pgid == pgid){
      tcsetpgrp (sh.getshinfo().terminal, sh.getshinfo().job_list[pos].pgid);
      tcsetattr (sh.getshinfo().terminal, TCSADRAIN, &sh.getshinfo().job_list[pos].j_tmodes);
      kill(pgid, SIGCONT);
      spot = pos;
      break;
    } // finding pgid
  } // iterate

  waitpid(WAIT_ANY, &status, WUNTRACED);

   /* Put the shell back in the foreground.  */
  tcsetpgrp (sh.getshinfo().terminal, sh.getshinfo().pgid);

  /* Restore the shell’s terminal modes.  */
  tcgetattr (sh.getshinfo().terminal, &sh.getshinfo().job_list[spot].j_tmodes);
  tcsetattr (sh.getshinfo().terminal, TCSADRAIN, &sh.getshinfo().tmodes);

  return status;
} // Citation : GNU Ch. 28.4

void shell_exit(int ex) {
  
  exit(ex);
} // shell_exit

void help() {

  cout << "– bg JID – Resume the stopped job JID in the background, as if it had been started with &.\n";
cout << "– cd [PATH] – Change the current directory to PATH. The environmental variable HOME is the default PATH.\n";
cout << "– exit [N] – Cause the shell to exit with a status of N. If N is omitted, the exit status is that of the last job executed.\n";
cout << "– export NAME[=WORD] – NAME is automatically included in the environment of subsequently executed jobs.\n";
cout << "– fg JID – Resume job JID in the foreground, and make it the current job.\n";
cout << "– help – Display helpful information about builtin commands.\n";
cout << "– jobs – List current jobs.\n";
cout << "- kill [-s SIGNAL] PID – The kill utility sends the specified signal to the specified process or process group PID\n";
cout << "(see kill(2) ). If no signal is specified, the SIGTERM signal is sent." << endl; 

} // help
