// RACC - AI development project for first-person shooter games derivated from Valve's Half-Life
// (http://www.racc-ai.com/)
//
// The game to engine interfacing code is based on the work done by Jeffrey 'botman' Broome
// (http://planethalflife.com/botman/)
//
// This project is partially based on the work done by Eric Bieschke in his BSDbot
// (http://gamershomepage.com/csbot/)
//
// This project is partially based on the work done by Brendan 'Spyro' McCarthy in his ODD Bot
// (http://oddbot.hlfusion.com/)
//
// This project is partially based on the work done by Alistair 'eLiTe' Stewart in his TEAMbot
// (http://www.planethalflife.com/teambot/)
//
// This project is partially based on the work done by Johannes '@$3.1415rin' Lampel in his JoeBot
// (http://www.joebot.net/)
//
// Rational Autonomous Cybernetic Commandos AI
//
// mfile.cpp
//

#include "racc.h"


// maximum number of files that can be loaded in memory at the same time
#define MAX_MFILES 32


MFILE memory_files[MAX_MFILES];



MFILE *mfopen (const char *file_path, const char *mode)
{
   // this function works exactly the same as fopen(), but whereas fopen() opens a file and have
   // its access to it on disk, mfopen() completely loads the file into memory. The arguments are
   // identical, a pointer to the file path string and the access mode to it. Please not that the
   // access mode parameter has been supplied for keeping the similarity with fopen(), but the
   // file is still obviously open with read authorization.

   FILE *fp;
   int mfile_index;

   if ((file_path == NULL) || (mode == NULL))
      return (NULL); // reliability check

   // find a free slot in the memory-loaded files array
   for (mfile_index = 0; mfile_index < MAX_MFILES; mfile_index++)
      if (memory_files[mfile_index].data == NULL)
         break; // break as soon as a free slot is found

   // if we've reached the maximum number of memory-loaded files
   if (mfile_index == MAX_MFILES)
      TerminateOnError ("mfopen(): Too many memory-loaded files\n"); // bomb out

   fp = fopen (file_path, "rb"); // try to open the file
   if (fp == NULL)
      return (NULL); // if not found, give up

   fseek (fp, 0, SEEK_END); // seek at end of file
   memory_files[mfile_index].file_size = ftell (fp); // get the file size

   // if the file was too large (the file_size integer has overflown)
   if (memory_files[mfile_index].file_size < 0)
   {
      fclose (fp); // close the file first
      TerminateOnError ("mfopen(): File too large to be open with mfopen()\n"); // bomb out
   }

   // allocate space for this file's data in memory
   memory_files[mfile_index].data = (char *) malloc (memory_files[mfile_index].file_size);
   if (memory_files[mfile_index].data == NULL)
   {
      fclose (fp); // if memory allocation was unsuccessful, close the file and bomb an error
      TerminateOnError ("mfopen(): malloc() failure for %s on %d bytes\n", file_path, memory_files[mfile_index].file_size); // bomb out
   }

   fseek (fp, 0, SEEK_SET); // rewind at start of file

   // now dump the file contents to memory
   fread (memory_files[mfile_index].data, memory_files[mfile_index].file_size, 1, fp);
   strcpy (memory_files[mfile_index].path, file_path);
   memory_files[mfile_index].read_pointer_index = 0; // initialize memory file pointer at first position

   fclose (fp); // now close the file on disk
   return (&memory_files[mfile_index]); // and return a pointer to that memory file structure
}


long mftell (MFILE *fp)
{
   // this function works exactly the same as ftell(), it returns the position of the memory-
   // loaded file read pointer

   if (fp == NULL)
      return (-1); // reliability check

   // if file pointer is illegal...
   if (fp->read_pointer_index < 0)
      TerminateOnError ("mftell(): Bad memory-loaded file pointer!\n"); // bomb out

   return (fp->read_pointer_index); // return the position of the read pointer in the memory-loaded file
}


int mfseek (MFILE *fp, long offset, int offset_mode)
{
   // this function works exactly the same as fseek(), it sets the position of the memory-
   // loaded file read pointer to the wanted offset, relatively to the specified offset mode
   // which can be SEEK_SET (from start of file), SEEK_END (from end of file) or SEEK_CUR (from
   // current pointer position).

   if (fp == NULL)
      return (-1); // reliability check

   // if file pointer is illegal...
   if (fp->read_pointer_index < 0)
      TerminateOnError ("mfseek(): Bad memory-loaded file pointer!\n"); // bomb out

   // given the offset mode, set the pointer at the right location
   if (offset_mode == SEEK_SET)
   {
      if (offset >= fp->file_size)
         return (-1); // don't allow setting the pointer too far
      fp->read_pointer_index = offset; // seek from start of file
   }
   else if (offset_mode == SEEK_END)
   {
      if (offset >= fp->file_size)
         return (-1); // don't allow setting the pointer too far
      fp->read_pointer_index = fp->file_size - offset; // seek from end of file
   }
   else
   {
      if ((fp->read_pointer_index + offset < 0) || (fp->read_pointer_index + offset >= fp->file_size))
         return (-1); // don't allow setting the pointer too far
      fp->read_pointer_index += offset; // seek from current pointer location
   }

   return (0); // returning 0 means the operation was successful
}


int mfseekAtSection (MFILE *fp, const char *section_name)
{
   // this function sets the file operation pointer inside the file pointed to by fp at the
   // beginning of the section called section_name. If such a section is not found, the file
   // operation pointer is reset to zero. Sections are chunks in binary file that start with
   // the [section] keyword, immediately followed by the section name in all letters, terminated
   // by the standard string terminator (null character), and followed by the section data.
   // Such a structure is typically found in bot navigation brain files. Local variables have
   // been declared static to speedup recurrent calls of this function.

   static char tag[32];
   static int section_name_length;

   if ((fp == NULL) || (section_name == NULL))
      return (-1); // reliability check

   tag[0] = 0; // reset tag string
   section_name_length = strlen (section_name); // get section name's length
   if (section_name_length > 32)
      TerminateOnError ("mfseekAtSection() called with section_name too long (%d characters)\n", section_name_length);

   // while neither the right section has been found nor the end of file has been reached...
   while (TRUE)
   {
      // while we've not found a section tag...
      while (strcmp (tag, "[section]") != 0)
      {
         // read ahead until we find a section tag
         if (mfread (tag, sizeof ("[section]"), 1, fp) != 1)
         {
            fp->read_pointer_index = 0; // end of file reached, rewind
            return (-1); // section not found
         }
         fp->read_pointer_index -= sizeof ("[section]") - 1; // get ahead one character in file
      }
      fp->read_pointer_index += sizeof ("[section]") - 1; // skip the [section] tag again

      // now read the section name
      if (mfread (tag, section_name_length + 1, 1, fp) != 1)
      {
         fp->read_pointer_index = 0; // end of file reached, rewind
         return (-1); // section not found
      }

      // is the section name we just read the wanted one ?
      if (strcmp (tag, section_name) == 0)
      {
         // found wanted section, seek at start of it
         fp->read_pointer_index -= section_name_length + 1 + sizeof ("[section]");
         return (0); // section found, return 0 (means the operation was successful)
      }
   }
}


int mfseekAfterSection (MFILE *fp, const char *section_name)
{
   // this function sets the file operation pointer inside the file pointed to by fp after the
   // section called section_name. If such a section is not found, the file operation pointer is
   // set to zero. Sections are chunks in binary file that start with the [section] keyword,
   // immediately followed by the section name in all letters, terminated by the standard string
   // terminator (null character), and followed by the section data. Such a structure is
   // typically found in bot navigation brain files. Local variables have been declared static
   // to speedup recurrent calls of this function.

   static char tag[32];
   static int section_name_length;

   if ((fp == NULL) || (section_name == NULL))
      return (-1); // reliability check

   tag[0] = 0; // reset tag string
   section_name_length = strlen (section_name); // get section name's length
   if (section_name_length > 32)
      TerminateOnError ("mfseekAfterSection() called with section_name too long (%d characters)\n", section_name_length);

   // while neither the right section has been found nor the end of file has been reached...
   while (TRUE)
   {
      // while we've not found a section tag...
      while (strcmp (tag, "[section]") != 0)
      {
         // read ahead until we find a section tag
         if (mfread (tag, sizeof ("[section]"), 1, fp) != 1)
         {
            fp->read_pointer_index = 0; // end of file reached, rewind
            return (-1); // section not found
         }
         fp->read_pointer_index -= sizeof ("[section]") - 1; // get ahead one character in file
      }
      fp->read_pointer_index += sizeof ("[section]") - 1; // skip the [section] tag again

      // now read the section name
      if (mfread (tag, section_name_length + 1, 1, fp) != 1)
      {
         fp->read_pointer_index = 0; // end of file reached, rewind
         return (-1); // section not found
      }
      tag[section_name_length] = 0; // terminate the section name string

      // is the section name we just read the wanted one ?
      if (strcmp (tag, section_name) == 0)
         break; // found wanted section, stop searching
   }

   // wanted section has been found, now all we have to do is to seek after it

   // while we've not found the next section tag...
   while (strcmp (tag, "[section]") != 0)
   {
      // read ahead until we find a section tag
      if (mfread (tag, sizeof ("[section]"), 1, fp) != 1)
      {
         fp->read_pointer_index = fp->file_size; // end of file reached, seek there then
         return (0); // operation successful, we're at the end of the file though
      }
      fp->read_pointer_index -= (sizeof ("[section]") - 1); // get ahead one character in file
   }
   fp->read_pointer_index--; // found next section tag, step back at start of it
   return (0); // next section found, return 0 (means the operation was successful)
}


int mfeof (MFILE *fp)
{
   // this function works exactly the same as feof(), it returns a non-zero value if the
   // position of the memory-loaded file read pointer has reached the end of the file.

   if (fp == NULL)
      return (-1); // reliability check

   // if file pointer is illegal...
   if (fp->read_pointer_index < 0)
      TerminateOnError ("mfeof(): Bad memory-loaded file pointer!\n"); // bomb out

   // if end of file is reached...
   if (fp->read_pointer_index >= fp->file_size)
      return (-1); // return a non-zero value when end of file is reached

   return (0); // returning zero means the end of file is not reached yet
}


size_t mfread (void *destination, size_t block_size, size_t num_blocks, MFILE *fp)
{
   // this function works exactly the same as fread(), but whereas fread() reads data from a
   // file on disk, mfread() reads data from a text file that has been loaded into memory
   // using mfopen(). The arguments are identical, the destination buffer to be filled, the size
   // of the data to read and the number of blocks to read, and the file pointer. If the total
   // amount of data requested for reading exceeds the length of the file, only the allowed data
   // is read, and the function returns the number of blocks it read successfully.

   int blocks_read;

   if ((destination == NULL) || (block_size == 0) || (num_blocks == 0) || (fp == NULL))
      return (0); // reliability check

   // if file pointer is illegal...
   if (fp->read_pointer_index < 0)
      TerminateOnError ("mfread(): Bad memory-loaded file pointer!\n"); // bomb out

   // compute the maximum amount of data we're allowed to read
   blocks_read = min ((fp->file_size - fp->read_pointer_index) / block_size, num_blocks);

   // read the specified amount of data in the memory-loaded file
   memcpy (destination, &fp->data[fp->read_pointer_index], blocks_read * block_size);
   fp->read_pointer_index += blocks_read * block_size; // update file pointer position

   return (blocks_read); // return the number of blocks we read
}


int mfgetc (MFILE *fp)
{
   // this function works exactly the same as fgetc(), but whereas fgetc() reads a character from
   // a file on disk, mfgetc() reads it from a file that has been loaded into memory using
   // mfopen(). The function returns the character read as an integer, else it returns -1 on
   // error or when the end of file has been reached.

   static int char_read;

   if ((fp == NULL) || (fp->read_pointer_index >= fp->file_size))
      return (-1); // reliability check

   // if file pointer is illegal...
   if (fp->read_pointer_index < 0)
      TerminateOnError ("mfgetc(): Bad memory-loaded file pointer!\n"); // bomb out

   char_read = fp->data[fp->read_pointer_index]; // read one character from the memory-loaded file
   fp->read_pointer_index++; // increment the file pointer

   return (char_read); // and return the character we read
}


const char *mfgets (char *line_buffer, int buffer_size, MFILE *fp)
{
   // this function works exactly the same as fgets(), but whereas fgets() reads a line from a
   // file on disk, mfgets() reads a line from a text file that has been loaded into memory
   // using mfopen(). The three arguments are identical, the line buffer to be filled, the size
   // of the data to read at each iteration of mfgets(), and the file pointer.

   int start_of_line_offset, end_of_line_offset, index;

   if (fp == NULL)
      return (NULL); // reliability check

   if (fp->read_pointer_index >= fp->file_size)
      return (NULL); // return if we've already reached the end of file

   // if file pointer is illegal...
   if (fp->read_pointer_index < 0)
      TerminateOnError ("mfgets(): Bad memory-loaded file pointer!\n"); // bomb out

   start_of_line_offset = fp->read_pointer_index; // remember current offset as the start of the line

   // set the position of the last byte to read not to exceed the buffer size
   if (fp->file_size - fp->read_pointer_index > buffer_size - 1)
      end_of_line_offset = fp->read_pointer_index + buffer_size - 1;
   else
      end_of_line_offset = fp->file_size - 1;

   // look for the position of the first newline we find, stop anyway if buffer is full
   while (fp->read_pointer_index < end_of_line_offset)
   {
      // do we have a line feed ?
      if (fp->data[fp->read_pointer_index] == 0x0A)
         end_of_line_offset = fp->read_pointer_index; // save end of line offset

      fp->read_pointer_index++; // get one character further
   }

   // have we actually read something ?
   if (fp->read_pointer_index == start_of_line_offset)
      return (NULL); // return if nothing was read

   // now we know where we must start the reading and where we must stop it

   // copy the data to the buffer, reading from file_pos to index
   for (index = start_of_line_offset; index <= end_of_line_offset; index++)
      line_buffer[index - start_of_line_offset] = fp->data[index];

   // is there a CR+LF sequence at the end of that line ?
   if (line_buffer[index - start_of_line_offset - 2] == 0x0D)
   {
      line_buffer[index - start_of_line_offset - 2] = '\n'; // turn it to a standard end of line
      index--; // get back one position
   }

   // is there a carriage return or a line feed at the end of that line ?
   if ((line_buffer[index - start_of_line_offset - 1] == 0x0D) || (line_buffer[index - start_of_line_offset - 1] == 0x0A))
      line_buffer[index - start_of_line_offset - 1] = '\n'; // turn it to a standard end of line

   line_buffer[index - start_of_line_offset] = 0; // terminate string

   return (line_buffer); // and return the buffer filled out with what we've read
}


void mfclose (MFILE *fp)
{
   // this function frees the memory space in which has been stored the contents of a file opened
   // by mfopen().

   if (fp == NULL)
      return; // reliability check

   if (fp->data)
      free (fp->data); // free the memory-loaded file data space if needed
   fp->data = NULL; // nulls out its pointer
   fp = NULL; // and nulls out the file pointer so that we know the slot is free

   return; // finished
}
