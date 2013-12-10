#ifndef systemsignal_h
#define systemsignal_h
typedef void Sigfunc(int) ;

void SetupSignalRoutine() ;
Sigfunc* vlSignalInstall(int signo, Sigfunc* func) ;


#endif // systemsignal_h






