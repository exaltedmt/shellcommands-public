#include "builtin.h"

using std::string;
using std::cout;
using std::endl;
using std::cin;
using std::getline;
using std::getenv;

void print_error(const string error = "") {
  cout << error << endl;
} // print_error

/**
 * Parsing the path, replacing the home path w/ a tilde
 */

char * calc_path(char * path) {
  string p = path;
  string h = getenv("HOME");
  
  for (uint index = 0; index < p.length(); index++) {
    if (p.substr(0,index+1) == h) {
      string temp = "~";
      temp += p.substr(index+1);
      p = temp;
    } // if
  } // for

  return strdup(p.c_str());
} // calc_path

/**
 * Calculate the arguments in a given input. Args are to be seperated by spaces.
 */

char ** calc_args(const string input, Shell & shell) {
  uint argc = 0; int q_check = -1;
  char ** argv;
  
  // Calculate the amount of arguments via analyzing spaces
  for (uint index = 0; index < input.length(); index++) {
    if (input[index] == '\"' && q_check == -1) {
      argc++;
      q_check = index;
    } // if
    else if (input[index] == '\"' && q_check != -1 && input[index-1] == '\\') continue;
    else if (input[index] == '\"' && q_check != -1 && input[index-1] != '\\') q_check = -1;
    else if (input[index] != ' ' && index == 0) argc++;
    else if (input[index] != ' ' && input[index-1] == ' ' && q_check == -1) argc++;
    else if (input[index] == ' ') continue;
  } // for

  shell.setargc(argc); // set the amount of args in the shell
  argv = new char * [argc]; // Array of strings. Each string in the array represents a provided argument.
  int start = -1, end = -1; // Used to parse the input into substrings

  for (uint argvIndex = 0, charCount = 0; charCount < input.length() && argvIndex < argc; charCount++) {
    
    // argvIndex keeps track of the index argv array. charCount keeps track of the character in the input string.
    // We are basically seperating string input into an array (elements seperated by spaces)

    if (charCount+1 == input.length() && input[charCount] != ' ' && charCount == 0) {
      string temp = input.substr(0);
      argv[argvIndex] = strdup(temp.c_str());
      argvIndex++;
    } // if
    
    else if (charCount+1 == input.length() && input[charCount] != ' ' && start != -1) {
      if (input[start] == '\"' && input[charCount] == '\"') {
  string temp = input.substr(start+1,charCount-start-1);
  argv[argvIndex] = strdup(temp.c_str());
  argvIndex++;
  start = -1;
  end = -1;
      } else if (input[start] == '\"' && input[charCount] != '\"') {
  string temp = "";
  argv[argvIndex] = strdup(temp.c_str());
  argvIndex++;
  start = -1;
  end = -1;
      } else {
  string temp = input.substr(start);
  argv[argvIndex] = strdup(temp.c_str());
  argvIndex++;
  start = -1;
  end = -1;  
      } // else
    } // else if

    else if (charCount+1 == input.length() && input[charCount] != ' ') {
      string temp = input.substr(charCount);
      argv[argvIndex] = strdup(temp.c_str());
      argvIndex++;
      start = -1;
      end = -1;
    } // else if

    else if (input[charCount] != ' ' && start == -1) {
      start = charCount;
    } // if    
    
    else if (start != -1 && input[start] == '\"' && charCount+1 == input.length()) {
      start = -1;
      end = -1;
      string temp = "";
      argv[argvIndex] = strdup(temp.c_str());
      argvIndex++;
    } // else if 

    else if ((input[charCount] == ' ') && start != -1) {
      if (input[start] == '\"' && input[charCount-1] == '\"' && input[charCount-2] == '\\') continue;
      else if (input[start] == '\"' && (input[charCount-1] != '\"' || charCount-1 == (uint)start)) continue;
      else if (input[start] == '\"' && input[charCount-1] == '\"') {
  end = charCount-2;
  string temp = input.substr(start+1,end-start);
  argv[argvIndex] = strdup(temp.c_str());
  argvIndex++;
  start = -1;
  end = -1;
      } else {
  end = charCount;
  string temp = input.substr(start,end-start);
  argv[argvIndex] = strdup(temp.c_str());
  argvIndex++;
  start = -1;
  end = -1;
      } // else 
    } // else if    
  } // for
  
  return argv;
} // calc_args


/**
 * Recursively fork and execute depending on amount of pipes.
 */

void exec_loop(Shell & shell, const pipe_t & pipes) {
  pid_t pid;
  int pipefd[2];
  
  if (pipes < shell.getshinfo().pipes) {
    pipe(pipefd);
    if ((pid = fork()) == -1) perror("fork");  
    else if (pid == 0) {
      close(pipefd[0]);
      dup2(pipefd[1], STDOUT_FILENO);
      close(pipefd[1]);
      exec_loop(shell, pipes+1);
    } else {
      close(pipefd[1]);
      dup2(pipefd[0], STDIN_FILENO);
      close(pipefd[0]);
      waitpid(pid,nullptr,0);

      /** REDIRECTION **/

      if (pipes == 0 && (shell.getshinfo().stdout_jobs[0] != "STDOUT_FILENO" || shell.getshinfo().stderr_jobs[0] != "STDERR_FILENO")) {
  int pipe_redir[2];
  int pipe_err[2];
  pipe(pipe_redir);
  pipe(pipe_err);
  if ((pid = fork()) == -1) perror("fork");
  else if (pid == 0) {
    close(pipe_redir[0]);
    close(pipe_err[0]);
    dup2(pipe_redir[1], STDOUT_FILENO);
    dup2(pipe_err[1], STDERR_FILENO);
    close(pipe_redir[1]);
    close(pipe_err[1]);
    shell.execute(shell.getshinfo().pipes-pipes+1);
  } else {
    close(pipe_redir[1]);
    close(pipe_err[1]);
    dup2(pipe_redir[0], STDIN_FILENO);
    dup2(pipe_err[0], STDERR_FILENO);
    close(pipe_redir[0]);
    close(pipe_err[0]);
    waitpid(pid,nullptr,0);
    shell.redirect_out();
    kill(getpid(), SIGKILL);
  } // else
      } // if

      else shell.execute(shell.getshinfo().pipes-pipes+1);
    } // else
  }  // if

  else { 
    if (shell.getshinfo().pipes == 0) {
      if (shell.getshinfo().stdout_jobs[0] != "STDOUT_FILENO" || shell.getshinfo().stderr_jobs[0] != "STDERR_FILENO" || shell.getshinfo().stdin_jobs[0] != "STDIN_FILENO") {
  int pipe_redir[2];
  int pipe_err[2];
  pipe(pipe_redir);
  pipe(pipe_err);

  if ((pid = fork()) == -1) perror("fork");
  else if (pid == 0) {
    close(pipe_redir[0]);
    close(pipe_err[0]);
    dup2(pipe_redir[1], STDOUT_FILENO);
    dup2(pipe_err[1], STDERR_FILENO);
    close(pipe_redir[1]);
    close(pipe_err[1]);
    
    if (shell.getshinfo().stdin_jobs[0] != "STDIN_FILENO") {
      int pipe_in[2];
      pipe(pipe_in);
      if ((pid = fork()) == -1) perror("fork");
      else if (pid == 0) {
        close(pipe_in[0]);
        close(pipe_redir[0]);
        close(pipe_redir[1]);
        dup2(pipe_in[1], STDOUT_FILENO);
        close(pipe_in[1]);
        shell.redirect_in();
        kill(getpid(), SIGKILL);    
      } else {
        close(pipe_in[1]);
        dup2(pipe_in[0], STDIN_FILENO);
        close(pipe_in[0]);
        waitpid(pid,nullptr,0);
      } // else 
    } // if

    shell.execute(1);
  } else {
    close(pipe_redir[1]);
    close(pipe_err[1]);
    dup2(pipe_redir[0], STDIN_FILENO);
    dup2(pipe_err[0], STDERR_FILENO);
    close(pipe_redir[0]);
    close(pipe_err[0]);
    waitpid(pid,nullptr,0);
    shell.redirect_out();
    kill(getpid(), SIGKILL);
  } // else
      } else shell.execute(1);
    } // if
 
    else {
      if (shell.getshinfo().stdin_jobs[0] != "STDIN_FILENO") {
  int pipe_in[2];
  pipe(pipe_in);
  
  if ((pid = fork()) == -1) perror("fork");
  else if (pid == 0) {
    close(pipe_in[0]);
    dup2(pipe_in[1], STDOUT_FILENO);
    close(pipe_in[1]);
    shell.redirect_in();
    kill(getpid(), SIGKILL);    
  } else {
    close(pipe_in[1]);
    dup2(pipe_in[0], STDIN_FILENO);
    close(pipe_in[0]);
    waitpid(pid,nullptr,0);
    shell.execute(1);
  } // else
      } else shell.execute(1);
    } // if
  } // else
  
} // exec_loop

void sig_handler(int signo) {
  cout << " " << strsignal(signo) << "    "; 
} // sig_handler

// Why not just use the signal function? (see signal(2))
void set_sig_handler(int signo, sighandler_t handler) {
  struct sigaction sa;     // sigaction struct object
  sa.sa_handler = handler; // set disposition
  if (sigaction(signo, &sa, nullptr) == -1) perror("sigaction");
} // setup_sig_handler

int main(const int argc, const char ** argv) {
  
  Shell shell; // Contains methods for the shell, as well as the data structure containing shell info.
  char ** argval; 
  char *shellPath;
  char * currPath;
  char buff[PATH_MAX];
  string input;
  bool backg = false;

  cout.setf(std::ios::unitbuf);

  shellPath = getenv("HOME");
  chdir(shellPath);
  shellPath = calc_path(shellPath);

  job first_j;
  job other_js;
  int j_itr = 0;
  
  const std::map<string, int> signal_map { 
    #ifdef SIGINT
      {"SIGINT", SIGINT},
    #endif
    #ifdef SIGQUIT
      {"SIGQUIT", SIGQUIT},
    #endif
    #ifdef SIGTSTP
      {"SIGTSTP", SIGTSTP},
    #endif
    #ifdef SIGTTIN
      {"SIGTTIN", SIGTTIN},
    #endif
    #ifdef SIGTTOU
      {"SIGTTOU", SIGTTOU},
    #endif
    #ifdef SIGCONT
      {"SIGCONT", SIGCONT},
    #endif
  };
  
  while (true) {

    set_sig_handler(SIGCONT, sig_handler); // set SIGCONT handler
          set_sig_handler(SIGTSTP, sig_handler); // set SIGTSTP handler
          set_sig_handler(SIGINT, sig_handler); // set  SIGINT handler
          set_sig_handler(SIGTERM, sig_handler); // set  SIGINT handler
    
    cout << "1730sh:" << shellPath << "$ ";
    getline(cin, input);    
    string copy = input;
    size_t pos;
    pos = input.find('&');
    if (pos != std::string::npos) input.erase(pos,1);
    argval = calc_args(input, shell);
    
    if (shell.getinfo(argval) == -1) print_error();
    
    for (uint i = 0; i < shell.getargc(); i++) delete [] argval[i]; 
    delete [] argval;
    
    if(shell.getargc() > 0){
      if (shell.getshinfo().proc_ls[0][0] == "exit"){
        if(shell.getargc() == 1)
          shell_exit(EXIT_SUCCESS);
        else {
          long ret_val = std::stol(shell.getshinfo().proc_ls[0][1]);
          shell_exit(ret_val);
        }
      } 
      else if (shell.getshinfo().proc_ls[0][0] == "cd"){
        cd(shell.getshinfo().proc_ls[0][1].c_str());
        currPath = getcwd(buff, PATH_MAX);
        shellPath = strcpy(shellPath, calc_path(currPath));
      } 

      else if (shell.getshinfo().proc_ls[0][0] == "kill") {

        int c = 0;
        bool kilt = false;

         while ((c = getopt (argc, (char**)argv, "s:")) != -1)
          switch (c) {
            case 's':
              kilt = true;
              break;
            default: 
              kilt = false; 
              break;
          } // getopt

        if(kilt){
          if(shell.getargc() == 4){
            long pid_holder = std::stol(shell.getshinfo().proc_ls[0][2]);
            int signo = 0;

             try {
              signo = signal_map.at(shell.getshinfo().proc_ls[0][3].c_str());
            } catch (std::exception & e) {
              std::cerr << "invalid signal" << endl;
            } // try

            kp(signo, pid_holder); 
          } // amount of args

          else perror("1730sh: kill");
        }  //kill with sig

        else {
          if(shell.getargc() == 2){
            long pid_holder = std::stol(shell.getshinfo().proc_ls[0][1]);
            kp(SIGTERM, pid_holder);
          } // amount of args

          else perror("1730sh: kill");
        } // else sigterm
      } // else if kill

      else if (shell.getshinfo().proc_ls[0][0] == "bg"){
        if(shell.getargc() == 2){
          long pid_holder = std::stol(shell.getshinfo().proc_ls[0][1]);
          bg(pid_holder, shell);

        } // amount of args

        else perror("1730sh: bg");
      } // else if bg

      else if (shell.getshinfo().proc_ls[0][0] == "fg"){
        if(shell.getargc() == 2){
          long pid_holder = std::stol(shell.getshinfo().proc_ls[0][1]);
          fg(pid_holder, shell);
        } // amount of args

        else perror("1730sh: fg");
      } // else if fg

      else if (shell.getshinfo().proc_ls[0][0] == "export"){
        if(shell.getargc() == 2){
          exp(shell.getshinfo().proc_ls[0][1].c_str());
        } // amount of args

        else perror("1730sh: export");
      } // else if fg

      else if (shell.getshinfo().proc_ls[0][0] == "jobs"){
        if(shell.getargc() == 1){
          jl(shell, j_itr);
        } // amount of args

        else perror("1730sh: jobs");
      } // else if fg

      else if (shell.getshinfo().proc_ls[0][0] == "help") help();

      else {
        pid_t pid;
        job temp_j;
        int pstatus;

        if ((pid = fork()) == -1) perror("fork");
        else if (pid == 0){

          signal (SIGINT, SIG_DFL); //reset signals so we can kill
          signal (SIGQUIT, SIG_DFL);
          signal (SIGTSTP, SIG_DFL);
          signal (SIGTTIN, SIG_DFL);
          signal (SIGTTOU, SIG_DFL);
          signal (SIGCHLD, SIG_DFL);

          if(backg){
              kill(SIGTSTP, pid);

              if(j_itr == 0)
                cout << getpid() << "   " << "STOPPED" << first_j.args << endl;

              else if(j_itr > 0)
                cout << getpid() << "   " << "STOPPED" << other_js.args << endl;
          } // if & then stop

          exec_loop(shell,0);
        } // if child set pgid to parent id
        else {

           if(j_itr == 0){
            first_j.args = copy;
            first_j.pgid = pid;
            shell.store_j(j_itr, first_j);
            if(j_itr+1 < 100)
              ++j_itr;
          } // on first job

          else if(j_itr > 0){
            other_js.pgid = pid;
            other_js.args = copy;
            shell.store_j(j_itr, other_js); // store in the array here
            if(j_itr+1 < 100)
              ++j_itr;
          } // other jobs

         

          if(backg){
            waitpid(pid, &pstatus, WNOHANG);
          }

          else{
            waitpid(pid, &pstatus, 0);
          }

          if(j_itr == 1){
            shell.store_js(j_itr-1, pstatus);
          } // on first job

          else if(j_itr > 1){
            shell.store_js(j_itr-1, pstatus);
          } // other jobs

        } // else 
  
        for (uint i = 0; i < shell.getargc(); i++) delete [] shell.getshinfo().proc_ls[i];
        delete [] shell.getshinfo().proc_ls;
  
      } // else exec
    } // argc > 0
  } // while
} // main
