#ifndef INCLUDE_DEBUG_H_ONCE
#define INCLUDE_DEBUG_H_ONCE

#include <errno.h>
#include <inttypes.h>

#define DEBUG_LEVEL_DEBUG           1
#define DEBUG_LEVEL_INFO            2
#define DEBUG_LEVEL_WARNING         3
#define DEBUG_LEVEL_ERROR           4

#define DEBUG_TIMESTAMP_WALLTIME    1       // Prints current time (wallclock)
#define DEBUG_TIMESTAMP_ELAPSED     2       // Prints total time used (wallclock)
#define DEBUG_TIMESTAMP_DIFF        3       // Prints time between calls (wallclock)
#define DEBUG_TIMESTAMP_CPUTIME     4       // Prints CPU time used

struct debug_options
  {
  uint32_t          level;
  uint32_t          threshold;

  uint32_t          to_stdout;
  uint32_t          to_stderr;
  char             *to_file;                      // at_exit() close file
  int               logfile_fd;

  uint32_t          print_ppid;
  uint32_t          print_pid;
  uint32_t          print_file;
  uint32_t          print_function;
  uint32_t          print_line;
  uint32_t          print_timestamp;
  uint32_t          print_strerror;

  struct timespec   timestamp;
  };


// Set debug options per object file, but make sure the compiler doesn't complain if you don't use it
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
  static struct debug_options debug_options = { .print_file = 1, .print_function = 1, .print_line = 1, .print_strerror = 1, .logfile_fd = -1 };
#pragma GCC diagnostic pop

//void DEBUG_CONTROL_INTERNAL(struct debug_opt *O);
void DEBUG_INTERNAL(struct debug_options *O, const char *File, const char *Function, uint32_t Line, uint32_t Errno, char *Msg, ...);
void DIE_INTERNAL(struct debug_options *O, const char *File, const char *Function, uint32_t Line, uint32_t Errno, char *Msg, ...);

//#define debug_control(...) do{DEBUG_CONTROL_INTERNAL(&debug_options, (struct debug_opt){ ## __VA_ARGS__ });}while(0)
#define debug(...) do{DEBUG_INTERNAL(&debug_options, __FILE__, __PRETTY_FUNCTION__, __LINE__, errno, __VA_ARGS__ );}while(0)
#define die(...) do{DIE_INTERNAL(&debug_options, __FILE__, __PRETTY_FUNCTION__, __LINE__, errno, __VA_ARGS__ );}while(0)

#endif

// EOF
