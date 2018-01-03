#ifndef NF_TIME_H_INCLUDED
#define NF_TIME_H_INCLUDED
#include <stdint.h>
#include <time.h>

//@ #include "lib/predicates.gh"

/**
   A wrapper around the system time function. Returns the number of
   seconds since the Epoch (1970-01-01 00:00:00 +0000 (UTC)).
   @returns the number of seconds since Epoch.
*/
time_t current_time(void);
//@ requires last_time(?x);
//@ ensures x <= result &*& last_time(result);

#endif//NF_TIME_H_INCLUDED
