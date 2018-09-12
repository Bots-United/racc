#include <stdio.h>
#include <string.h>


void main (void)
{
   char stringname[50];
   int length;
   int i, j;

   // initialize the string
   strcpy (stringname, "The book keeper reads books");

   // first off, remove spaces

   length = strlen (stringname); // get string length

   // parse string, start at pos #0 and stop at the first space encountered
   for (i = 0; i < length; i++)
      if (stringname[i] == ' ')
      {
         // we've found a space at i, so shuffle the rest of the string one character back to get rid of it
         for (j = i; j < length - 1; j++)
            stringname[j] = stringname[j + 1]; // shuffle string
         stringname[j] = 0; // at the end of the string, drop a termination character (for safety)
         length--; // once string is shuffled, it's one character shorter

         // space removed, advance by one character in the string
      }

   // now get rid of the double letters

   length = strlen (stringname); // get length again

   // parse string, and stop at first double encountered
   // start at 1 because we want to compare it with 0 (heh)
   for (i = 1; i < length; i++)
      if (stringname[i] == stringname[i - 1])
      {
         // we've found a double at i, so shuffle the rest of the string one character back to get rid of it
         for (j = i; j < length - 1; j++)
            stringname[j] = stringname[j + 1]; // shuffle string
         stringname[j] = 0; // at the end of the string, drop a termination character (for safety)
         length--; // once string is shuffled, it's one character shorter

         // now the space has disappeared, we must redo this from the start, so reset i to 1
         i = 1;
      }

   // here it is, string should now be:
   // "thebokepereadsboks"
   printf ("%s\n", stringname);
   getchar ();
}
