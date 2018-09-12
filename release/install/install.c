// RACC - AI development project for first-person shooter games derivated from Valve's Half-Life
// (http://www.racc-ai.com/)
//
// Game config file parser/patcher for the RACC install procedure
// Patches MOD's liblist.gam for the MOD dll field to point to the RACC dll
//
// Files processed may not be larger than 16 kilobytes
//
// Usage: install MOD
//   where MOD is the directory name of the MOD to install RACC for
//
// install.c
//

#include <stdio.h>
#include <string.h>


void main (int argc, char *argv[])
{
   FILE *fp;
   char filename[256], line_buffer[256], file_buffer[16384];

   if ((argc != 2) || (strcmp ("/?", argv[1]) == 0) || (strcmp ("/help", argv[1]) == 0))
   {
      printf ("\n");
      printf ("Game config file parser/patcher for the RACC install procedure\n");
      printf ("Patches liblist.gam for the MOD DLL field to point to the RACC DLL\n");
      printf ("\n");
      printf ("Usage: install mod_directory_name\n");
      printf ("\n");
      return; // if too few/too many arguments, exit
   }

   // process liblist.gam
   file_buffer[0] = 0; // first empty the file buffer
   sprintf (filename, "../%s/liblist.gam", argv[1]); // build the liblist.gam file path
   fp = fopen (filename, "r+"); // open the file in ASCII read/write mode
   if (fp == NULL)
   {
      printf ("ERROR: MOD's liblist.gam file not found. Ensure you specify the directory\n");
      printf ("name rather than MOD's full name (for example, \"install cstrike\")\n");
      return; // error message if not found
   }

   while (fgets (line_buffer, 255, fp) != NULL) // reads line per line
   {
      // if this line is the gamedll field...
      if (strncmp ("gamedll ", line_buffer, 8) == 0) // if we have reached the gamedll field
         sprintf (file_buffer, "%sgamedll \"../racc/release/%s/release/racc.dll\"\n", file_buffer, argv[1]); // patch original file
      else
         sprintf (file_buffer, "%s%s", file_buffer, line_buffer); // append line to output file

      // if this line is the gamedll_linux field
      if (strncmp ("gamedll_linux ", line_buffer, 14) == 0) // if we have reached the gamedll field
         sprintf (file_buffer, "%sgamedll \"../racc/release/%s/release/racc_i386.so\"\n", file_buffer, argv[1]); // patch original file
      else
         sprintf (file_buffer, "%s%s", file_buffer, line_buffer); // append line to output file
   }

   rewind (fp); // get back to start of file
   fprintf (fp, file_buffer); // write buffer back to file
   fclose (fp); // close file

   return; // finished
}
