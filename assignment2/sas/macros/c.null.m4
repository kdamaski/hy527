divert(-1)

dnl --------------------------------------------------------------------
dnl Null macros. The SPLASH/SPLASH2 applications, when used with this
dnl file should work without any problems
dnl --------------------------------------------------------------------

define(ALLOCATE_PAGE, `{;}')
define(ALLOCATE_RANGE, `{;}')
define(RESET_STATISTICS, `{;}')
define(ADEC, `int ($1);')
define(AINIT, `{;}')
define(ALOCK, `{;}')
define(ALOCKDEC, `int ($1);')
define(ALOCKINIT, `{;}')
define(AUG_DELAY, `{;}')
define(AUG_OFF, `')
define(AUG_ON, `')
define(AUG_SET_HILIMIT, `{;}')
define(AUG_SET_LOLIMIT, `{;}')
define(AULOCK, `{;}')
define(AUNLOCK, `{;}')
define(ACQUIRE, `{;}')
define(RELEASE, `{;}')
define(AACQUIRE, `{;}')
define(ARELEASE, `{;}')
define(BARDEC, `int ($1);')
define(BARINIT, `{;}')
define(BARRIER, `{;}')
define(CLEARPAUSE, `{;}')
define(CLOCK, `{  unsigned long now;
                  gettimeofday(&the_time, NULL);
                  now = ((the_time.tv_sec - 879217568) * 1000) + (the_time.tv_usec / 1000);
                  //if ((now < 10000) || (now > (unsigned int)2000000000)) printf("The time now is %d %d\n", now, the_time.tv_sec);
                  $1 = (unsigned int)now;
               }')
define(CONTINUE, `{;}')

define(CREATE, `{
	fprintf (stderr, "No more processors -- this is a uniprocessor version!\n");
	exit (-1);
}')

define(CREATE_LITE, `{
	fprintf (stderr, "No more processors -- this is a uniprocessor version!\n");
	exit (-1);
}')
dnl define(CREATE, `{;}');

define(DELAY, `{;}')
define(DYN_REF_TRACE_OFF, `;')
define(DYN_REF_TRACE_ON, `;')
define(DYN_SCHED_OFF, `;')
define(DYN_SCHED_ON, `;')
define(DYN_SIM_OFF, `;')
define(DYN_SIM_ON, `;')
define(DYN_TRACE_OFF, `;')
define(DYN_TRACE_ON, `;')

define(ENV_G, `
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/time.h>

void
  exit (),
  perror (),
  Sc_Reschedule (),
  SC_Reschedule ();
')

define(ENV, `
ENV_G
struct timeval the_time;
')

define(EVENT, `{;}')
define(EXTERN_ENV, `
ENV_G
extern struct timeval the_time;
')
define(G_FREE, `;')
define(G_MALLOC, `G_MALLOC_F($1);')
define(G_MALLOC_F, `malloc ($1)')
define(MALLOC, `malloc ($1)')
define(FREE, `;')
define(GET_HOME, `{($1) = 0;}')
define(GET_PID_F, `0')
define(GET_PID, `{($1) = GET_PID_F;}')

define(GETSUB,

  if (($1) <= ($3))
    ($2) = ($1)++;
  else

    ($2) = -1;
    ($1) = 0;
  }
}')

define(GSDEC, `int ($1);')
define(GSINIT, `{ ($1) = 0; }')
define(LOCK, `{;}')
define(LOCKDEC, `int ($1);')
define(LOCKINIT, `{;}')

define(MAIN_END, `')
define(MAIN_END_LITE, `')

define(MAIN_ENV, `ENV ;')
define(MAIN_INITENV, `{;}')
define(MAIN_INITENV_LITE, `{;}')
define(MENTER, `{;}')
define(MEXIT, `{;}')
define(MONINIT, `{;}')
define(NEWPROC, `{;}')
define(NLOCK, `{;}')
define(NLOCKDEC, `int ($1);')
define(NLOCKINIT, `{;}')
define(NU_FREE, `;')
define(NU_GETSUB, `GETSUB($1,$2,$3,$4)')
define(NU_GSDEC, `int ($1);')
define(NU_GSINIT, `{ ($1) = 0; }')
define(NU_MALLOC, `NU_MALLOC_F($1);')
define(NU_MALLOC_F, `malloc ($1)')
define(NUNLOCK, `{;}')
define(PAUSE, `{;}')
define(PAUSEDEC, `{;}')
define(PAUSEINIT, `{;}')
define(PROBEND, `{;}')
define(REF_TRACE_OFF, `{;}')
define(REF_TRACE_ON, `{;}')
define(SEMDEC, `int ($1);')
define(SEMINIT, `{($1)=($2);}')
define(SEMP, `{if((--($1))<0){
  fprintf (stderr, "Cannot block -- this is a uniprocessor version!\n");
  exit (-1);}}')
define(SEMV, `{($1)++;}')
define(SET_HOME, `{;}')
define(SETPAUSE, `{;}')
define(ST_LOG, `{;}')
define(TRACE_OFF, `{;}')
define(TRACE_ON, `{;}')
define(UNLOCK, `{;}')
define(WAIT_FOR_END, `{;}')
define(WAITPAUSE, `{;}')

define(WEAKGETSUB,

  if (($1) <= ($3))
    ($2) = ($1)++;
  else

    ($2) = -1;
  }
}')

divert(0)
#ifndef SYS_THREAD
extern struct sys_thread_t {
  int user_pid;
} *SYS_THREAD_PTR;
#define SYS_THREAD
#endif /* !SYS_THREAD */





