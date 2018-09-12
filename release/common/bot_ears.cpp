// RACC - AI development project for first-person shooter games derivated from Valve's Half-Life
// (http://www.racc-ai.com/)
//
// The game to engine interfacing code is based on the work done by Jeffrey 'Botman' Broome
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
// bot_ears.cpp
//

#include "racc.h"


extern bot_t bots[MAX_CLIENTS_SUPPORTED_BY_ENGINE];
extern sound_t sounds[MAX_SOUNDS + MAX_LOCAL_SOUNDS];
extern int sound_count;
extern texture_t textures[MAX_TEXTURES];
extern int texture_count;
extern debug_level_t DebugLevel;


void BotHear (bot_t *pBot)
{
   // this is the function that makes the bot hear. For every audible sound (i.e. BOTH in
   // range AND acknowledged by the subconsciousness - not discarded because of too much ambient
   // noise), checks are made to tell whether this sound represents a threat or not, and if so,
   // place the bot's ideal angles in the approximate direction for the bot to get an eye on
   // what's happening. It is based on the following assumptions, 1° that a bot knowing to have
   // teammates around will less care for the threat potential, and 2° that peaks in the
   // derivation of the waveform induce a surprise factor that makes the sound a potential
   // source of danger. Note the waveform is obviously VERY undersampled (the wave is actually
   // the sum of the average loudnesses of the different sounds each frame).

   int index, noise_count = 0;

   if (IsNull (pBot->pEntity))
      return; // reliability check

   // count the number of sounds being heard, and check if a sound has faded out
   for (index = 0; index < BOT_EAR_SENSITIVITY; index++)
      if (pBot->BotEars.noises[index].fade_date > 0)
         if (pBot->BotEars.noises[index].fade_date < CurrentTime ())
            pBot->BotEars.noises[index].fade_date = 0; // that sound has faded now, so delete it
         else
            noise_count++; // here we have a valid sound bot is hearing

   // now go for computing the new average noise...
   pBot->BotEars.average_noise = 0;

   for (index = 0; index < BOT_EAR_SENSITIVITY; index++)
      if (pBot->BotEars.noises[index].fade_date > CurrentTime ())
         pBot->BotEars.average_noise += pBot->BotEars.noises[index].loudness / noise_count;

   // if debug mode is high, print out the average noise just computed
   if (DebugLevel.ears > 1)
      ServerConsole_printf ("BOT %s HEARS AVERAGE NOISE: %.1f\n", NetnameOf (pBot->pEntity), pBot->BotEars.average_noise);

   // TODO: MAKE SOUNDS DIRECTION RELATIVE (LEFT, RIGHT, FRONT, REAR)

   // does the bot hear a new sound ?
   if (pBot->BotEars.new_sound)
   {
      // check it

      pBot->BotEars.new_sound = FALSE; // don't do it twice
   }

   return;
}


void BotFeedEar (bot_t *pBot, sound_t *sound, vector v_origin, float volume)
{
   // this function is in charge of finding, or freeing if necessary, a slot in the bot's ears
   // to put the sound pointed to by sound in. If no free slot is available, the sound that is
   // the most about to finish gets overwritten.

   float next_fade_date = CurrentTime () + 60.0;
   int i, selected_index = 0;

   if (IsNull (pBot->pEntity) || (sound == NULL))
      return; // reliability check

   // find a free slot in bot's ears
   for (i = 0; i < BOT_EAR_SENSITIVITY; i++)
   {
      // does one exists ?
      if (pBot->BotEars.noises[i].fade_date < CurrentTime ())
      {
         // store the sound in that slot of the bot's ear
         strcpy (pBot->BotEars.noises[i].file_path, sound->file_path);
         pBot->BotEars.noises[i].v_origin = v_origin; // remember origin
         pBot->BotEars.noises[i].fade_date = CurrentTime () + sound->duration;
         pBot->BotEars.noises[i].loudness = sound->loudness * volume;

         pBot->BotEars.new_sound = TRUE; // mark new sound flag for the bot to check it
         return;
      }
   }

   // no free slot found, so overwrite one, preferably the one most close to fade
   // FIXME: wrong rule - given several sounds, WHICH ONE are we likely to ignore the most ?
   for (i = 0; i < BOT_EAR_SENSITIVITY; i++)
      if (pBot->BotEars.noises[i].fade_date < next_fade_date)
      {
         next_fade_date = pBot->BotEars.noises[i].fade_date;
         selected_index = i; // select the sound which is the most "finished"
      }

   // store the sound in that slot of the bot's ear
   strcpy (pBot->BotEars.noises[selected_index].file_path, sound->file_path);
   pBot->BotEars.noises[selected_index].v_origin = v_origin;
   pBot->BotEars.noises[selected_index].fade_date = CurrentTime () + sound->duration;
   pBot->BotEars.noises[selected_index].loudness = sound->loudness * volume;

   // if debug mode is enabled, tell the developer what we did
   if (DebugLevel.ears > 0)
      ServerConsole_printf ("BOT %s HEARS SOUND: %s - LOUD %.1f - FADD %.1f\n",
                            NetnameOf (pBot->pEntity),
                            pBot->BotEars.noises[selected_index].file_path,
                            pBot->BotEars.noises[selected_index].loudness,
                            pBot->BotEars.noises[selected_index].fade_date);

   pBot->BotEars.new_sound = TRUE; // mark new sound flag for the bot to check it
   return;
}


void DispatchSound (const char *sample, vector v_origin, float volume, float attenuation)
{
   // this function brings the sound to the ears of the bots. Every time a sound is emitted in
   // the game somehow, this function has to be called. It cycles through all the bots that are
   // playing, and does the appropriate checks in order to determine if this bot will hear that
   // sound or not. In case it can, the function places the sound into the bot's ears structure.

   int i, bot_index;
   sound_t *sound = NULL;
   float attenuated_volume = volume;

   if (sample == NULL)
      return; // reliability check

   // find the sound we want in the global list
   for (i = 0; i < sound_count; i++)
      if (strcmp (sounds[i].file_path, sample) == 0)
         sound = &sounds[i]; // link a pointer to this sound's info slot

   // have we found the sound we want to dispatch ?
   if (sound != NULL)
   {
      // if debug mode is enabled, tell the user we are dispatching a sound
      if (DebugLevel.ears > 0)
         printf ("DispatchSound() \"%s\" from (%.1f, %.1f, %.1f): vol %.1f, att %.1f\n", sample, v_origin.x, v_origin.y, v_origin.z, volume, attenuation);

      // cycle through all bot slots
      for (bot_index = 0; bot_index < MaxClientsOnServer (); bot_index++)
      {
         // is this slot used ?
         if (bots[bot_index].is_active && (bots[bot_index].pEntity != NULL))
         {
            // is this sound NOT attenuated by distance ?
            if (attenuation == ATTN_NONE)
               BotFeedEar (&bots[bot_index], sound, v_origin, volume); // if so, this bot will hear it anyway

            // else is that bot within the maximum hearing range of the PAS ?
            // FIXME: ATM, I can't tell the difference between all the different attenuations !
            else if ((v_origin - OriginOf (bots[bot_index].pEntity)).Length () < MAX_HEARING_DISTANCE)
            {
               attenuated_volume = volume * ((MAX_HEARING_DISTANCE - (v_origin - OriginOf (bots[bot_index].pEntity)).Length ()) / MAX_HEARING_DISTANCE);
               BotFeedEar (&bots[bot_index], sound, v_origin, attenuated_volume); // bot hears attenuated sound
            }
         }
      }
   }

   // alert, sound was not found !
   else
   {
      // print a notification for users if in debug mode
      if (DebugLevel.ears > 0)
         printf ("DispatchSound() ALERT: sound \"%s\" not found in database!\n", sample);
   }
}


void PrecacheSoundForBots (char *sound_path, int sound_index)
{
   // this function fills out an information structure about a WAV file pointed to by pMemFile.
   // WAV files are divided into chunks, respectively the RIFF, WAVE, FORMAT and DATA chunk.
   // Since there are often extra data added to the end of chunks, or even extra chunks inserted
   // between the usual order, this function parses through the file intelligently looking for
   // each chunk and filling the header structure accordingly. Once the header is known, this
   // function computes the average loudness of the WAV file, which computation is done according
   // to the rules defined by the wave header. The purpose of this computation is to provide the
   // bots with an immediate idea of the loudness of a sound being played, since listening to
   // samples in real-time for them would require quite a lot processing power. Actually the bots
   // don't hear each bit of a sound sample, they hear the global loudness it gives instead, but,
   // like us humans, they hear it as long it lasts. One final note, using LoadFileForMe() from
   // the engine instead of fopen() to open files makes us able to read through the engine's
   // virtual filesystem: a merge of the "valve" folder with the MOD folder including all the
   // PAK subsystem contents, both in the valve directory and in the MOD one.

   FILE fp;
   char file_path[256];
   wav_header_t wav_header;
   int index;
   signed int iSample;

   // look if that sound already exists in our array
   for (index = 0; index < sound_count; index++)
      if (strcmp (sound_path, sounds[index].file_path) == 0)
         return; // if that sound is already precached, return

   // have we reached the max precachable number of sounds ?
   if (sound_count == MAX_SOUNDS + MAX_LOCAL_SOUNDS)
   {
      ServerConsole_printf ("RACC: PrecacheSoundForBots(): max sounds reached!\n", sound_path);
      return; // return, no room for that one...
   }

   // ask the engine to locate and open that sound file
   sprintf (file_path, "sound/%s", sound_path); // build the sound path relative to game directory

   memset (&fp, 0, sizeof (fp)); // reset the memory-loaded file structure first
   fp._ptr = (char *) pfnLoadFileForMe (file_path, &fp._bufsiz); // load wave file
   if ((fp._ptr == NULL) || (fp._bufsiz == 0))
   {
      ServerConsole_printf ("RACC: PrecacheSoundForBots(): wave file %s not found!\n", sound_path);
      return; // if not found, then give up
   }

   // if developer mode is high, notify about this sound precaching
   if (DeveloperMode () == DEVELOPER_VERBOSE)
      ServerConsole_printf ("RACC: precaching sound %s for bots (engine index %d)\n", sound_path, sound_index);

   // first reset the wav header structure
   memset (&wav_header, 0, sizeof (wav_header));

   // read the RIFF chunk
   while (strncmp (wav_header.riff_chunk_id, "RIFF", 4) != 0)
   {
      if (fp._cnt + 4 > fp._bufsiz)
      {
         ServerConsole_printf ("RACC: PrecacheSoundForBots(): wave file %s: RIFF chunk ID not found!\n", sound_path);
         mfclose (&fp); // don't forget to free the wave file
         return; // avoid buffer overflows
      }
      memcpy (wav_header.riff_chunk_id, fp._ptr + fp._cnt, 4); // read riff id (4 bytes)
      fp._cnt++; // and get ahead one character
   }
   fp._cnt--; // step back again to get back to the start of the chunk
   if (fp._cnt + 7 < fp._bufsiz)
      memcpy (wav_header.riff_chunk_id, fp._ptr + fp._cnt, 8); // read riff chunk (8 bytes)
   else
   {
      ServerConsole_printf ("RACC: PrecacheSoundForBots(): wave file %s: RIFF chunk truncated!\n", sound_path);
      mfclose (&fp); // don't forget to free the wave file
      return; // avoid buffer overflows
   }
   fp._cnt += 8; // get out of riff chunk

   // read the WAVE chunk
   while (strncmp (wav_header.wave_chunk_id, "WAVE", 4) != 0)
   {
      if (fp._cnt + 4 > fp._bufsiz)
      {
         ServerConsole_printf ("RACC: PrecacheSoundForBots(): wave file %s: WAVE chunk ID not found!\n", sound_path);
         mfclose (&fp); // don't forget to free the wave file
         return; // avoid buffer overflows
      }
      memcpy (wav_header.wave_chunk_id, fp._ptr + fp._cnt, 4); // read wave id (4 bytes)
      fp._cnt++; // and get ahead one character
   }
   fp._cnt--; // step back again to get back to the start of the chunk
   if (fp._cnt + 3 < fp._bufsiz)
      memcpy (wav_header.wave_chunk_id, fp._ptr + fp._cnt, 4); // read wave chunk (4 bytes)
   else
   {
      ServerConsole_printf ("RACC: PrecacheSoundForBots(): wave file %s: WAVE chunk truncated!\n", sound_path);
      mfclose (&fp); // don't forget to free the wave file
      return; // avoid buffer overflows
   }
   fp._cnt += 4; // get out of wave chunk

   // read the FORMAT chunk
   while (strncmp (wav_header.fmt_chunk_id, "fmt ", 4) != 0)
   {
      if (fp._cnt + 4 > fp._bufsiz)
      {
         ServerConsole_printf ("RACC: PrecacheSoundForBots(): wave file %s: fmt_ chunk ID not found!\n", sound_path);
         mfclose (&fp); // don't forget to free the wave file
         return; // avoid buffer overflows
      }
      memcpy (wav_header.fmt_chunk_id, fp._ptr + fp._cnt, 4); // read format id (4 bytes)
      fp._cnt++; // and get ahead one character
   }
   fp._cnt--; // step back again to get back to the start of the chunk
   if (fp._cnt + 23 < fp._bufsiz)
      memcpy (wav_header.fmt_chunk_id, fp._ptr + fp._cnt, 24); // read format chunk (24 bytes)
   else
   {
      ServerConsole_printf ("RACC: PrecacheSoundForBots(): wave file %s: fmt_ chunk truncated!\n", sound_path);
      mfclose (&fp); // don't forget to free the wave file
      return; // avoid buffer overflows
   }
   fp._cnt += 24; // get out of format chunk

   // read the DATA chunk
   while (strncmp (wav_header.data_chunk_id, "data", 4) != 0)
   {
      if (fp._cnt + 4 > fp._bufsiz)
      {
         ServerConsole_printf ("RACC: PrecacheSoundForBots(): wave file %s: data chunk ID not found!\n", sound_path);
         mfclose (&fp); // don't forget to free the wave file
         return; // avoid buffer overflows
      }
      memcpy (wav_header.data_chunk_id, fp._ptr + fp._cnt, 4); // read data id (4 bytes)
      fp._cnt++; // and get ahead one character
   }
   fp._cnt--; // step back again to get back to the start of the chunk
   if (fp._cnt + 7 < fp._bufsiz)
      memcpy (wav_header.data_chunk_id, fp._ptr + fp._cnt, 8); // read START of data chunk (8 bytes)
   else
   {
      ServerConsole_printf ("RACC: PrecacheSoundForBots(): wave file %s: data chunk truncated!\n", sound_path);
      mfclose (&fp); // don't forget to free the wave file
      return; // avoid buffer overflows
   }

   // is that file of an unknown format ?
   if ((wav_header.bytes_per_sample != 1) && (wav_header.bytes_per_sample != 2))
   {
      ServerConsole_printf ("RACC: PrecacheSoundForBots(): wave file %s: invalid bytes per sample value (%d)!\n", sound_path, wav_header.bytes_per_sample);
      mfclose (&fp); // don't forget to free the wave file
      return; // avoid buffer overflows
   }

   fp._cnt += 8; // reach start of data

   // file ok, we can compute its loudness and fill one slot more in the global sounds array

   index = fp._cnt; // index will point at start of data now

   // read each sample, don't exceed end of file anyway...
   while (((unsigned int) index < fp._cnt + wav_header.data_chunk_length) && (index < fp._bufsiz))
   {
      iSample = 0x0000; // reset the container variable
      memcpy (&iSample, fp._ptr + index, wav_header.bytes_per_sample); // read N-byte sample
      sounds[sound_count].loudness += (float) abs (iSample - (wav_header.bytes_per_sample == 1 ? 128 : 16384)) * (wav_header.bytes_per_sample == 1 ? 256 : 1) / (float) wav_header.data_chunk_length; // sum it
      index += wav_header.bytes_per_sample; // get to the next sample, N bytes further
   }

   // now store the duration and the sound path (relative to the MOD's sound folder)
   sounds[sound_count].duration = (float) wav_header.data_chunk_length / (float) wav_header.bytes_per_second;
   strcpy (sounds[sound_count].file_path, sound_path); // store the sound path in the array

   if (DeveloperMode () == DEVELOPER_VERBOSE)
      ServerConsole_printf ("sound #%d (%s) - length %f - noise %f\n", sound_count, sounds[sound_count].file_path, sounds[sound_count].duration, sounds[sound_count].loudness);

   sound_count++; // increment the number of sounds we know (we know one more now)
   mfclose (&fp); // don't forget to free the wave file
   return; // finished
}


void InitTextureSoundsForBots (void)
{
   // this function is in charge of opening the materials.txt file, and filling the texture
   // sounds database accordingly. Each texture in the game produces a different sound if the
   // player gets walking on it. This is determined by associating a texture type to a texture
   // name, and this is done in the file "sounds/materials.txt". Such a task should be performed
   // once and only once, preferably at GameDLLInit(), since textures aren't reloaded between
   // each map. Note the use of LoadFileForMe(): this function is a heaven's gift for programmers
   // since it handles perfectly the virtual filesystem of the engine, which rules are as follow:
   // 1°) if such a file exists in the MOD's directory, open it. Else 2°) if such a file exists
   // in the PAK file structure of the MOD under the same path, open it. Else 3°) if such a file
   // exists in the "valve" directory under the same path, open it. Else finally 4°) if such a
   // file exists in Valve's PAK file structure under the same path, then open it. Obviously this
   // handy function saves us a lot of PITA, looking for the right path to open the file in.
   // Thank you guys at Valve Software ! Moreover, instead of just setting a pointer to the file
   // data somewhere on the hard disk, the file gets loaded *completely* into memory, thus
   // speeding up the seek & read routines, hence the use of mfgets() instead of fgets(), hence
   // too the use of free() instead of fclose(). Yay!

   FILE fp;
   char line_buffer[256];
   int index, fieldstart, length;

   // first prepare the default texture
   textures[0].name[0] = 0; // this texture has default name (nothing)
   textures[0].type = CHAR_TEX_CONCRETE; // this texture makes default sound (concrete)
   texture_count = 1; // we know at least one texture now

   // ask the engine to find and open the file "materials.txt"
   memset (&fp, 0, sizeof (fp));
   fp._ptr = (char *) pfnLoadFileForMe ("sound/materials.txt", &fp._bufsiz);
   if (fp._ptr == NULL)
      return;

   // for each line in the file...
   while ((mfgets (line_buffer, 256, &fp) != NULL) && (texture_count < MAX_TEXTURES))
   {
      length = strlen (line_buffer); // get length of line
      if ((length > 0) && (line_buffer[length - 1] == '\n'))
         length--; // remove any final '\n'
      line_buffer[length] = 0; // terminate the string

      index = 0; // let's now parse the line to get the field value

      if ((line_buffer[0] == 0) || ((length > 1) && (line_buffer[0] == '/') && (line_buffer[1] == '/')))
         continue; // ignore line if void or commented

      while ((index < length) && ((line_buffer[index] == ' ') || (line_buffer[index] == '\t')))
         index++; // ignore any tabs or spaces

      // is this line a valid data line ?
      if (isalpha (line_buffer[index]))
      {
         textures[texture_count].type = toupper (line_buffer[index]); // save texture type
         index++; // get out of field

         while ((index < length) && ((line_buffer[index] == ' ') || (line_buffer[index] == '\t')))
            index++; // ignore any tabs or spaces

         fieldstart = index; // remember field start position

         // now save texture name (only get the CBTEXTURENAMEMAX first caracters)
         for (index = fieldstart; index < (fieldstart + CBTEXTURENAMEMAX < length ? (fieldstart + CBTEXTURENAMEMAX) : length); index++)
            textures[texture_count].name[index - fieldstart] = line_buffer[index];

         // look if that texture already existed in our array
         for (index = 0; index < texture_count; index++)
            if (strcmp (textures[texture_count].name, textures[index].name) == 0)
               continue; // if that texture was already precached, don't count it

         texture_count++; // else increment the total number of textures we know
      }
   }

   free (fp._ptr); // don't forget to free the file from memory

   // if debug mode is enabled, print out how many textures we have precached
   if (DebugLevel.ears > 0)
      ServerConsole_printf ("RACC: Bot sound system precached %d textures\n", texture_count);

   return;
}
