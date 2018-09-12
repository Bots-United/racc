// RACC - AI development project for first-person shooter games derivated from Valve's Half-Life
// (http://www.racc-ai.com/)
//
// Game config file parser/patcher for the RACC install procedure
// - Retrieves the LANGUAGE of the game to patch game files with the correct language
// - Patches MOD's liblist.gam for the MOD dll field to point to the RACC dll
// - Patches MOD's settings.scr if file install/LANGUAGE/settings.scr.patch exists
// - Patches MOD's commandmenu.txt if file install/LANGUAGE/MOD_commandmenu.txt.patch exists
// - Patches MOD's gfx/shell/kb_act.lst if file install/LANGUAGE/MOD_kb_act.lst.patch exists
//
// This program MUST be launched from the RACC root directory, i.e. C:\Sierra\Half-Life\RACC
// Files processed may not be larger than 16 kilobytes
//
// Usage: PATCH.EXE MOD
//   where MOD is the directory name of the MOD to install RACC for
//
// Extra: PATCH.EXE /DEBUG_CS
//   if this argument is used, patch Counter-Strike's CLIENT.DLL to allow the use of a debugger
//
// patchcfg.cpp
//

#include <stdio.h>
#include <string.h>

// one is sufficient
void main (int argc, char *argv[])
{
   FILE *fp, *fp2;
   char line_buffer[1024], line_buffer2[1024], file_buffer[16384], filename[256], modname[32], language[32];

   if ((argc != 2) || (strcmp ("/?", argv[1]) == 0) || (strcmp ("/help", argv[1]) == 0))
   {
      printf ("PATCH.EXE - Game config file parser/patcher for the RACC install procedure\n");
      printf ("\n");
      printf ("- Retrieves the LANGUAGE of the game to patch files with the correct language\n");
      printf ("- Patches liblist.gam for the MOD dll field to point to the RACC dll\n");
      printf ("- Patches settings.scr if install/LANGUAGE/settings.scr.patch exists\n");
      printf ("- Patches commandmenu.txt if install/LANGUAGE/MOD_commandmenu.txt.patch exists\n");
      printf ("- Patches gfx/shell/kb_act.lst if install/LANGUAGE/MOD_kb_act.lst.patch exists\n");
      printf ("\n");
      printf ("Usage: PATCH.EXE mod_name\n");
      printf ("\n");
      printf ("Extra stuff: PATCH.EXE /DEBUG_CS allows you to run Counter-Strike 1.3 within\n");
      printf ("your favourite debugger (developers only - thanks Count Floyd!)\n");
      printf ("\n");
      return; // if too few/too many arguments, exit
   }

   // if we want to patch Counter-Strike's CLIENT.DLL instead...
   if ((strcmp ("/DEBUG_CS", argv[1]) == 0) || (strcmp ("/debug_cs", argv[1]) == 0))
   {
      fp = fopen ("../cstrike/cl_dlls/client.dll", "rb+"); // opens CLIENT.DLL file readonly
      if (fp != NULL)
      {
         fp2 = fopen ("../cstrike/cl_dlls/client.bak", "wb"); // opens backup file for writing
         if (fp2 != NULL)
         {
            int c, count = 0;

            // get the size of the CLIENT.DLL file
            while ((c = fgetc (fp)) != EOF)
               count++;

            //  if it differs from the CS 1.3 original DLL size
            if (count != 655360)
            {
               printf ("CLIENT.DLL is not the genuine Counter-Strike 1.3 DLL. Patch cancelled.\n");
               return; // don't allow it to be patched
            }

            rewind (fp); // return back to beginning of file
            while ((c = fgetc (fp)) != EOF)
               fputc (c, fp2); // duplicate client.dll to client.bak

            fclose (fp2); // close backup file

            fseek (fp, 191620L, SEEK_SET); // seek at the start of patch offset
            fputc (0x2B, fp); // actually patch client.dll (at offset 191620)
            fclose (fp); // close patched file

            printf ("Counter-Strike's CLIENT.DLL has been successfully patched.\n");
         }
      }
      else
         printf ("CLIENT.DLL not found in ../cstrike/cl_dlls. Patch cancelled.\n");

      return; // end of processing
   }

   strcpy (modname, argv[1]); // store wanted MOD's name

   // get the game language from the sierra.inf file
   fp = fopen ("../sierra.inf", "r"); // opens file readonly

   if (fp != NULL)
   {
      // reads line per line
      while (fgets (line_buffer, 255, fp) != NULL)
      {
         if ((line_buffer[0] == '\n') || ((line_buffer[0] == '/') && (line_buffer[1] == '/')))
            continue; // ignore line if void or commented

         // if that line tells the standard American/English language definition
         if (strncmp ("ShortTitle=HALFLIFE", line_buffer, 19) == 0)
         {
            strcpy (language, "english");
            break;
         }

         // else if that line tells the French language definition
         else if (strncmp ("ShortTitle=HLIFEFR", line_buffer, 18) == 0)
         {
            strcpy (language, "french");
            break;
         }

         // else if that line tells the Deutsch language definition
         else if (strncmp ("ShortTitle=HLIFEDE", line_buffer, 18) == 0)
         {
            strcpy (language, "german");
            break;
         }

         // else if that line tells the Italian language definition
         else if (strncmp ("ShortTitle=HLIFEIT", line_buffer, 18) == 0)
         {
            strcpy (language, "italian");
            break;
         }

         // else if that line tells the Spanish language definition
         else if (strncmp ("ShortTitle=HLIFEES", line_buffer, 18) == 0)
         {
            strcpy (language, "spanish");
            break;
         }
      }

      fclose (fp); // close file
   }
   else
   {
      printf ("ERROR: sierra.inf file not found. Ensure you are running this program\n");
      printf ("from the RACC root directory (for instance, \"C:\\Games\\Half-Life\\RACC\")\n");
      return; // error message if not found
   }

   // process liblist.gam
   file_buffer[0] = 0x00;
   sprintf (filename, "install/%s_liblist.prv", modname);
   fp = fopen (filename, "r"); // opens file readonly
   if (fp != NULL)
   {
      while (fgets (line_buffer, 255, fp) != NULL) // reads line per line
      {
         if (strncmp ("gamedll ", line_buffer, 8) == 0) // if we have reached the gamedll field
            sprintf (file_buffer, "%sgamedll \"../racc/release/%s/release/racc.dll\"\n", file_buffer, modname); // patch original file
         else
            sprintf (file_buffer, "%s%s", file_buffer, line_buffer); // append line to output file
      }
      fclose (fp); // close original file
      sprintf (filename, "../%s/liblist.gam", modname);
      fp = fopen (filename, "w"); // opens file for writing
      if (fp != NULL)
      {
         fputs (file_buffer, fp); // write to file
         fclose (fp); // close file
	   }
   }
   else
   {
      printf ("ERROR: MOD's liblist.gam file not found. Ensure you specify the directory\n");
      printf ("name rather than MOD's full name (for instance, \"PATCHCFG.EXE cstrike\")\n");
      return; // error message if not found
   }

   // process settings.scr
   file_buffer[0] = 0x00;
   sprintf (filename, "install/%s_settings.prv", modname);
   fp = fopen (filename, "r"); // opens file readonly
   if (fp != NULL)
   {
      while (fgets (line_buffer, 255, fp) != NULL) // reads line per line
      {
         if ((line_buffer[0] == '}') && (line_buffer[1] == '\n')) // if we have reached the good place
         {
            sprintf (filename, "install/%s/settings.scr.patch", language);
            fp2 = fopen (filename, "r"); // opens patch file readonly
            if (fp2 != NULL)
            {
               while (fgets (line_buffer2, 255, fp2) != NULL) // reads line per line
                  sprintf (file_buffer, "%s%s", file_buffer, line_buffer2); // patch original file
               fclose (fp2); // close patch file
            }
         }
         sprintf (file_buffer, "%s%s", file_buffer, line_buffer); // append line to output file
      }
      fclose (fp); // close original file
      sprintf (filename, "../%s/settings.scr", modname);
      fp = fopen (filename, "w"); // opens file for writing
      if (fp != NULL)
      {
         fputs (file_buffer, fp); // write to file
         fclose (fp); // close file
	   }
   }

   // process kb_act.lst
   file_buffer[0] = 0x00;
   sprintf (filename, "install/%s_kb_act.prv", modname);
   fp = fopen (filename, "r"); // opens file readonly
   if (fp != NULL)
   {
      sprintf (filename, "install/%s/%s_kb_act.lst.patch", language, modname);
      fp2 = fopen (filename, "r"); // opens patch file readonly
      if (fp2 != NULL)
      {
         while (fgets (line_buffer, 255, fp2) != NULL) // reads line per line
            sprintf (file_buffer, "%s%s", file_buffer, line_buffer); // patch original file
         fclose (fp2); // close patch file
      }
      while (fgets (line_buffer, 255, fp) != NULL) // reads line per line
         sprintf (file_buffer, "%s%s", file_buffer, line_buffer); // append line to output file
      fclose (fp); // close original file
      sprintf (filename, "../%s/gfx/shell/kb_act.lst", modname);
      fp = fopen (filename, "w"); // opens file for writing
      if (fp != NULL)
      {
         fputs (file_buffer, fp); // write to file
         fclose (fp); // close file
	   }
   }

   // process commandmenu.txt
   file_buffer[0] = 0x00;
   sprintf (filename, "install/%s_commandmenu.prv", modname);
   fp = fopen (filename, "r"); // opens file readonly
   if (fp != NULL)
   {
      sprintf (filename, "install/%s/%s_commandmenu.txt.patch", language, modname);
      fp2 = fopen (filename, "r"); // opens patch file readonly
      if (fp2 != NULL)
      {
         while (fgets (line_buffer, 255, fp2) != NULL) // reads line per line
            sprintf (file_buffer, "%s%s", file_buffer, line_buffer); // patch original file
         fclose (fp2); // close patch file
      }
      while (fgets (line_buffer, 255, fp) != NULL) // reads line per line
         sprintf (file_buffer, "%s%s", file_buffer, line_buffer); // append line to output file
      fclose (fp); // close original file
      sprintf (filename, "../%s/commandmenu.txt", modname);
      fp = fopen (filename, "w"); // opens file for writing
      if (fp != NULL)
      {
         fputs (file_buffer, fp); // write to file
         fclose (fp); // close file
	   }
   }

   return; // end of processing
}
