// RACC - AI development project for first-person shooter games
// (http://racc.bots-united.com/)
//
// Rational Autonomous Cybernetic Commandos AI
//
// lrand.cpp
//

#include "racc.h"


long lseed; // our random number generator's seed


void lsrand (unsigned long initial_seed)
{
   // this function initializes the random seed based on the initial seed value passed in the
   // initial_seed parameter.

   int i;

   lseed = (long) initial_seed; // fill in the initial seed of the random number generator

   for (i = 0; i < 100; i++)
      lrand (); // call rand a couple times to improve randomness

   return; // that's all folks
}


long lrand (void)
{
   // this function is the equivalent of the rand() standard C library function, except that
   // whereas rand() works only with short integers (i.e. not above 32767), this function is
   // able to generate 32-bit random numbers. Isn't that nice ?

   // credits go to botmeister for showing me this lovely snippet of code !!!

   lseed = 1664525L * lseed + 1013904223L; // do some twisted math (infinite suite)

   return ((lseed >> 1) + 1073741824L); // and return the result. Yeah, simple as that.
}

  
long RandomLong (long from, long to)
{
   // this function returns a random integer number between (and including) the starting and
   // ending values passed by parameters from and to.

   if (to <= from)
      return (from);

   return (from + lrand () / (LONG_MAX / (to - from + 1)));
}


float RandomFloat (float from, float to)
{
   // this function returns a random integer number between (and including) the starting and
   // ending values passed by parameters from and to.

   if (to <= from)
      return (from);

   return (from + (float) lrand () / (LONG_MAX / (to - from)));
}


// "Anyone who consider arithmetic means of producing random number is,
//  of course, in a state of sin"
//                 -- John Von Neumann
