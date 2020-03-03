// reset ; gcc -DDEBUG_EXAMPLE -Wall -g -o debug_example debug.c
// reset ; valgrind -q --leak-check=full --show-reachable=yes --track-origins=yes --track-fds=yes ./debug_example

#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "debug.h"



static void timespec_diff(struct timespec *end, struct timespec *start, struct timespec *result)
{
if(end->tv_nsec < start->tv_nsec)
  {
  result->tv_sec  = end->tv_sec - start->tv_sec - 1;
  result->tv_nsec = 1000000000 + end->tv_nsec - start->tv_nsec;
  }
  else
  {
  result->tv_sec  = end->tv_sec  - start->tv_sec;
  result->tv_nsec = end->tv_nsec - start->tv_nsec;
  }
}



// https://stackoverflow.com/questions/8304259/formatting-struct-timespec/14746954

static char *timespec2str(struct timespec *TS)
{
static char   ret[] = "2012-12-31 12:59:59.123456789";   // Maximum size
struct tm     t;

tzset();
if(localtime_r(&(TS->tv_sec), &t) == NULL)
  die("localtime_r");

if(strftime(ret, strlen(ret), "%F %T", &t) == 0)
  die("strftime");

sprintf(&ret[strlen(ret)], ".%09ld", TS->tv_nsec);

return ret;
}


void debug_close_logfile(void)
{
if(debug_options.logfile_fd != -1)
  close(debug_options.logfile_fd);
}


void DEBUG_INTERNAL(struct debug_options *O, const char *File, const char *Function, uint32_t Line, uint32_t Errno, char *Msg, ...)
{
#define DEBUG_MSG_SIZE   4069

char              msg[DEBUG_MSG_SIZE];
uint32_t          r = 0;
struct timespec   tmp;
struct timespec   now;

if(!O->to_stdout && !O->to_stderr && !O->to_file)
  return;

if(O->print_timestamp)
  {
  switch(O->print_timestamp)
    {
    case DEBUG_TIMESTAMP_WALLTIME:    if(clock_gettime(CLOCK_REALTIME, &now) == -1)
                                        die("CLOCK_REALTIME");
                                      r = r + snprintf(&msg[r], DEBUG_MSG_SIZE - r, "[%s]:", timespec2str(&now));
                                      break;

    case DEBUG_TIMESTAMP_CPUTIME:     if(clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &now) == -1)
                                        die("CLOCK_PROCESS_CPUTIME_ID");
                                      break;

    case DEBUG_TIMESTAMP_DIFF:        if(O->timestamp.tv_sec == 0 && O->timestamp.tv_nsec == 0)
                                        if(clock_gettime(CLOCK_REALTIME, &O->timestamp) == -1)                       // Record the wall clock time
                                          die("CLOCK_REALTIME");
                                      if(clock_gettime(CLOCK_REALTIME, &tmp) == -1)
                                        die("CLOCK_REALTIME");
                                      timespec_diff(&tmp, &O->timestamp, &now);
                                      memcpy(&O->timestamp, &tmp, sizeof(struct timespec));
                                      break;

    case DEBUG_TIMESTAMP_ELAPSED:     if(O->timestamp.tv_sec == 0 && O->timestamp.tv_nsec == 0)
                                        if(clock_gettime(CLOCK_REALTIME, &O->timestamp) == -1)                       // Record the wall clock time
                                          die("CLOCK_REALTIME");
                                      if(clock_gettime(CLOCK_REALTIME, &tmp) == -1)
                                        die("CLOCK_REALTIME");
                                      timespec_diff(&tmp, &O->timestamp, &now);
                                      break;
    default:                          die("switch");
    }
  if(O->print_timestamp != DEBUG_TIMESTAMP_WALLTIME)
    r = r + snprintf(&msg[r], DEBUG_MSG_SIZE - r, "[%lu.%09lu]:", now.tv_sec, now.tv_nsec);
  }

if(O->print_ppid)
  r = r + snprintf(&msg[r], DEBUG_MSG_SIZE - r, "%u:", getppid());

if(O->print_pid)
  r = r + snprintf(&msg[r], DEBUG_MSG_SIZE - r, "%u:", getpid());

if(O->print_file)
  r = r + snprintf(&msg[r], DEBUG_MSG_SIZE - r, "%s:", File);

if(O->print_function)
  r = r + snprintf(&msg[r], DEBUG_MSG_SIZE - r, "%s:", Function);

if(O->print_line)
  r = r + snprintf(&msg[r], DEBUG_MSG_SIZE - r, "%u:", Line);

va_list ap;
va_start(ap, Msg);
r = r + vsnprintf(&msg[r], DEBUG_MSG_SIZE - r, Msg, ap);
va_end(ap);

if(O->print_strerror && errno != 0)
  r = r + snprintf(&msg[r], DEBUG_MSG_SIZE - r, ":%s", strerror(errno));

r = r + snprintf(&msg[r], DEBUG_MSG_SIZE - r, "\n");

if(O->to_stdout)
  {
  // fprintf(stdout, "stdout:");
  fprintf(stdout, "%.*s", r, msg);
  fflush(stdout);
  }

if(O->to_stderr)
  {
  // fprintf(stderr, "stderr:");
  fprintf(stderr, "%.*s", r, msg);
  fflush(stderr);
  }

if(O->to_file)
  {
  if(O->logfile_fd == -1)
    {
    if( (O->logfile_fd = open(O->to_file, O_WRONLY | O_APPEND | O_CREAT, 0644)) == -1)    // O_SYNC is possible, but might hit performance
      {
      O->to_file = NULL;
      die("Opening logfile failed");
      }
    atexit(debug_close_logfile);
    }

  write(O->logfile_fd, msg, r);
  }

} // DEBUG_INTERNAL()



void DIE_INTERNAL(struct debug_options *O, const char *File, const char *Function, uint32_t Line, uint32_t Errno, char *Msg, ...)
{
if(!O->to_stdout)
  O->to_stderr = 1;                      // If we're dying, make sure we output!

va_list ap;
va_start(ap, Msg);
DEBUG_INTERNAL(O, File, Function, Line, Errno, Msg, ap);
va_end(ap);
exit(1);
}



#ifdef DEBUG_EXAMPLE
  int main(void)
  {
  debug_options.to_stdout = 1;
  //debug_options.to_file = "debug.log";
  debug_options.print_ppid = 1;
  debug_options.print_pid = 1;
  debug_options.print_timestamp = DEBUG_TIMESTAMP_ELAPSED;

  debug("Hello world");
  sleep(3);
  debug("Hello %s", "you");
  sleep(1);
  debug("blaahyou");

  return 0;
  }
#endif

// EOF
