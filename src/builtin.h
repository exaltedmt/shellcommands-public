#ifndef BUILTIN_H
#define BUILTIN_H

#include "Shell.h"

using std::cout;
using std::string;
using std::endl;

void cd(const char * path);
void kp(int signo, pid_t pid);
int fg(pid_t pgid, Shell & sh);
void jl(Shell & sh, int j_ctr);
void exp(const char * word);
void bg(pid_t pgid, Shell & sh);
void shell_exit(int ex);
void help();

#endif
