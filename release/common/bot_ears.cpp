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
// bot_ears.cpp
//

#include "racc.h"


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

   static bot_ears_t *pBotEar;
   char index, noise_count = 0;
   float new_average_noise;

   if (DebugLevel.ears_disabled)
      return; // return if we don't want the AI to hear

   pBotEar = &pBot->BotEars; // quick access to ear

   // count the number of sounds being heard, and check if a sound has faded out
   for (index = 0; index < BOT_EAR_SENSITIVITY; index++)
      if (pBotEar->noises[index].fade_date > 0)
         if (pBotEar->noises[index].fade_date < *server.time)
         {
            // this sound has faded now, so delete it
            memset (&pBotEar->noises[index], 0, sizeof (pBotEar->noises[index]));

            // was the bot about to hear this sound ?
            if (pBotEar->new_sound && (index == pBotEar->new_sound_index))
               pBotEar->new_sound = FALSE; // just forget this sound, bot didn't even notice it
         }
         else
            noise_count++; // here we have a valid sound bot is hearing

   // now go for computing the new average noise...
   new_average_noise = 0;

   for (index = 0; index < BOT_EAR_SENSITIVITY; index++)
      if (pBotEar->noises[index].fade_date > *server.time)
         new_average_noise += pBotEar->noises[index].loudness / noise_count;

   // if debug mode is high, print out the average noise just computed
   if ((DebugLevel.ears > 1) && IsValidPlayer (pListenserverEntity) && ((pBot->pEdict->v.origin - pListenserverEntity->v.origin).Length () <= 100))
      ServerConsole_printf ("BOT %s's AVERAGE NOISE: %.1f\n", STRING (pBot->pEdict->v.netname), new_average_noise);

   // TODO: MAKE SOUNDS DIRECTION RELATIVE (LEFT, RIGHT, FRONT, REAR)

   // does the bot hear a new sound ?
   if (pBotEar->new_sound)
   {
      // is the new average noise significant enough to trigger the bot's attention ?
      if (RANDOM_FLOAT (0, pBotEar->average_noise) > fabs (new_average_noise - pBotEar->average_noise))
         BotReactToSound (pBot, &pBotEar->noises[pBotEar->new_sound_index]); // make the bot react to sound

      pBotEar->new_sound = FALSE; // don't do it twice
   }

   pBotEar->average_noise = new_average_noise; // save new average noise
   return; // finished hearing
}


void BotFeedEar (bot_t *pBot, sound_t *sound, Vector v_origin, float volume)
{
   // this function is in charge of finding, or freeing if necessary, a slot in the bot's ears
   // to put the sound pointed to by sound in. If no free slot is available, the sound that is
   // the most about to finish gets overwritten.

   static bot_ears_t *pBotEar;
   float nearest_fade_date;
   char i, selected_index;

   if (sound == NULL)
      return; // reliability check

   pBotEar = &pBot->BotEars; // quick access to ear

   // find a free slot in bot's ears
   for (selected_index = 0; selected_index < BOT_EAR_SENSITIVITY; selected_index++)
      if (pBotEar->noises[selected_index].fade_date < *server.time)
         break; // break when a free slot is found

   // have we found NO free slot ?
   if (selected_index == BOT_EAR_SENSITIVITY)
   {
      // no free slot found, so overwrite one, preferably the one most close to fade
      // FIXME: wrong rule - given several sounds, WHICH ONE are we likely to ignore the most ?
      nearest_fade_date = *server.time + 60.0;
      selected_index = 0;
      for (i = 0; i < BOT_EAR_SENSITIVITY; i++)
         if (pBotEar->noises[i].fade_date < nearest_fade_date)
         {
            nearest_fade_date = pBotEar->noises[i].fade_date;
            selected_index = i; // select the sound which is the most "finished"
         }
   }

   // store the sound in that slot of the bot's ear
   pBotEar->noises[selected_index].file_path = sound->file_path; // link pointer to sound file path
   pBotEar->noises[selected_index].direction = BotEstimateDirection (pBot, v_origin); // remember origin
   pBotEar->noises[selected_index].fade_date = *server.time + sound->duration; // duration
   pBotEar->noises[selected_index].loudness = sound->loudness * volume; // loudness

   pBotEar->new_sound = TRUE; // notify the bot that it is hearing a new noise
   pBotEar->new_sound_index = selected_index; // mark new sound index for bot to check it

   // if debug mode is high, tell the developer that a new sound is coming to this bot
   if ((DebugLevel.ears > 1) && IsValidPlayer (pListenserverEntity) && ((pBot->pEdict->v.origin - pListenserverEntity->v.origin).Length () <= 100))
      ServerConsole_printf ("NEW SOUND TO BOT %s's EAR: %s - LOUD %.1f - FAD %.1f\n",
                            STRING (pBot->pEdict->v.netname),
                            pBotEar->noises[selected_index].file_path,
                            pBotEar->noises[selected_index].loudness,
                            pBotEar->noises[selected_index].fade_date);
   return;
}


void DispatchSound (const char *sample, Vector v_origin, float volume, float attenuation)
{
   // this function brings the sound to the ears of the bots. Every time a sound is emitted in
   // the game somehow, this function has to be called. It cycles through all the bots that are
   // playing, and does the appropriate checks in order to determine if this bot will hear that
   // sound or not. In case it can, the function places the sound into the bot's ears structure.

   int bot_index;
   sound_t *sound;
   float attenuated_volume;

   if (sample == NULL)
      return; // reliability check

   sound = FindSoundByFilename (sample); // find the sound we want in the global sound database
   if (sound->duration == 0)
      return; // cancel if sound was not found

   // if debug mode is enabled, tell the user we are dispatching a sound around here
   if ((DebugLevel.ears > 1) || ((DebugLevel.ears > 0) && IsValidPlayer (pListenserverEntity) && ((v_origin - pListenserverEntity->v.origin).Length () <= 1500)))
      printf ("DispatchSound() \"%s\" from (%.1f, %.1f, %.1f): vol %.1f, att %.1f\n", sound->file_path, v_origin.x, v_origin.y, v_origin.z, volume, attenuation);

   // cycle through all bot slots
   for (bot_index = 0; bot_index < RACC_MAX_CLIENTS; bot_index++)
   {
      // is this slot used ?
      if (bots[bot_index].is_active && IsValidPlayer (bots[bot_index].pEdict))
      {
         // is this sound NOT attenuated by distance ?
         if (attenuation == ATTN_NONE)
            BotFeedEar (&bots[bot_index], sound, v_origin, volume); // if so, bot will hear it anyway

         // else is that bot within the maximum hearing range of the PAS ?
         // FIXME: ATM, I can't tell the difference between all the different attenuations !
         else if ((v_origin - bots[bot_index].pEdict->v.origin).Length () < MAX_HEARING_DISTANCE)
         {
            attenuated_volume = volume * ((MAX_HEARING_DISTANCE - (v_origin - bots[bot_index].pEdict->v.origin).Length ()) / MAX_HEARING_DISTANCE);
            BotFeedEar (&bots[bot_index], sound, v_origin, attenuated_volume); // bot hears attenuated sound
         }
      }
   }

   return; // done, sound dispatched to all bots in range
}


void PlayBulletSoundsForBots (edict_t *pPlayer)
{
   // this function is in charge of emulating the gunshot sounds for the bots. Since these sounds
   // are only predicted by the client, and bots have no client DLL, obviously we have to do the
   // work for them. We consider a client is told gunshot sound occurs when he receives the
   // msg_CurWeapon network message, which gets sent whenever a player is lowering his ammo.
   // That's why we hook those messages in MessageBegin(), and send the entity responsible of
   // it have a walk around here. Given the weapon this player is holding in his hand then, the
   // appropriate gunshot sound is played, amongst all the sounds that are listed in the
   // weaponsounds.cfg file. Then DispatchSound() is called to bring the selected sound to the
   // bot's ears.

   weapon_t *pPlayerWeapon;
   const char *texture_name;
   char texture_type;
   int player_index, sound_index;
   Vector v_gun_position;

   if (!IsValidPlayer (pPlayer) || !IsAlive (pPlayer))
      return; // skip invalid and dead players

   if (DebugLevel.is_observer && !(pPlayer->v.flags & FL_THIRDPARTYBOT))
      return; // skip real players if in observer mode

   if (!(pPlayer->v.button & (IN_ATTACK | IN_ATTACK2)))
      return; // cancel if player is not firing

   if (STRING (pPlayer->v.weaponmodel)[0] == 0)
      return; // cancel if player has no weapon

   pPlayerWeapon = FindWeaponByModel (STRING (pPlayer->v.weaponmodel)); // get player's weapon
   if (pPlayerWeapon->id == 0)
      return; // cancel if player has no weapon

   v_gun_position = GetGunPosition (pPlayer); // get this player's gun position

   // now select the sound according to rail (primary or secondary) and mode
   if (pPlayer->v.button & IN_ATTACK)
   {
      // primary rail
      if (pPlayer->v.weaponanim == 0)
         DispatchSound (pPlayerWeapon->primary.sound1, v_gun_position, 1.0, ATTN_NORM);
      else
         DispatchSound (pPlayerWeapon->primary.sound2, v_gun_position, 1.0, ATTN_NORM);
   }
   else
   {
      // secondary rail
      if (pPlayer->v.weaponanim == 0)
         DispatchSound (pPlayerWeapon->secondary.sound1, v_gun_position, 1.0, ATTN_NORM);
      else
         DispatchSound (pPlayerWeapon->secondary.sound2, v_gun_position, 1.0, ATTN_NORM);
   }

   // do we have to worry about ricochet sounds ?
   if (ricochetsound_count > 0)
   {
      player_index = ENTINDEX (pPlayer) - 1; // get the player index

      // did this player's last traceline hit something AND it is not a player ?
      if ((players[player_index].tr.flFraction < 1.0) && !FNullEnt (players[player_index].tr.pHit)
          && !(players[player_index].tr.pHit->v.flags & (FL_MONSTER | FL_CLIENT)))
      {
         // ask the engine for the texture name at the bullet hit point
         texture_name = TRACE_TEXTURE (players[player_index].tr.pHit, v_gun_position, players[player_index].tr.vecEndPos);

         // if the engine found the texture, ask the MOD DLL for the texture type
         if (texture_name != NULL)
            texture_type = PM_FindTextureType ((char *) texture_name); // ask for texture type

         // loop through all the ricochet sounds the bot knows until we find the right one
         for (sound_index = 0; sound_index < ricochetsound_count; sound_index++)
         {
            // is it this texture type the bullet just hit OR have we reached the default sound ?
            if ((texture_type == ricochetsounds[sound_index].texture_type)
                || (ricochetsounds[sound_index].texture_type == '*'))
               break; // then no need to search further
         }

         // bring this ricochet sound to the bots' ears
         DispatchSound (ricochetsounds[sound_index].file_path, players[player_index].tr.vecEndPos, 0.9, ATTN_NORM);
      }
   }

   return;
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

   MFILE fp;
   char file_path[256];
   wav_header_t wav_header;
   int index;
   signed long iSample;

   // look if that sound already exists in our array
   for (index = 0; index < sound_count; index++)
      if (strcmp (sound_path, sounds[index].file_path) == 0)
         return; // if that sound is already precached, return

   // have we reached the max precachable number of sounds ?
   if (sound_count == RACC_MAX_SOUNDS)
   {
      ServerConsole_printf ("RACC: PrecacheSoundForBots(): max sounds reached!\n", sound_path);
      return; // return, no room for that one...
   }

   // ask the engine to locate and open that sound file
   sprintf (file_path, "sound/%s", sound_path); // build the sound path relative to game directory

   memset (&fp, 0, sizeof (fp)); // reset the memory-loaded file structure first
   fp.data = (char *) LOAD_FILE_FOR_ME (file_path, (int *) &fp.file_size); // load wave file
   if ((fp.data == NULL) || (fp.file_size == 0))
   {
      ServerConsole_printf ("RACC: PrecacheSoundForBots(): wave file %s not found!\n", sound_path);
      return; // if not found, then give up
   }

   // if debug level is activated, notify about this sound precaching
   if (DebugLevel.ears > 0)
      ServerConsole_printf ("RACC: precaching sound %s for bots (engine index %d)\n", sound_path, sound_index);

   // first reset the wav header structure
   memset (&wav_header, 0, sizeof (wav_header));

   // read the RIFF chunk
   while (strncmp (wav_header.riff_chunk_id, "RIFF", 4) != 0)
   {
      if (fp.read_pointer_index + 4 > fp.file_size)
      {
         ServerConsole_printf ("RACC: PrecacheSoundForBots(): wave file %s: RIFF chunk ID not found!\n", sound_path);
         FREE_FILE (fp.data); // don't forget to free the wave file
         return; // avoid buffer overflows
      }
      memcpy (wav_header.riff_chunk_id, fp.data + fp.read_pointer_index, 4); // read riff id (4 bytes)
      fp.read_pointer_index++; // and get ahead one character
   }
   fp.read_pointer_index--; // step back again to get back to the start of the chunk
   if (fp.read_pointer_index + 7 < fp.file_size)
      memcpy (wav_header.riff_chunk_id, fp.data + fp.read_pointer_index, 8); // read riff chunk (8 bytes)
   else
   {
      ServerConsole_printf ("RACC: PrecacheSoundForBots(): wave file %s: RIFF chunk truncated!\n", sound_path);
      FREE_FILE (fp.data); // don't forget to free the wave file
      return; // avoid buffer overflows
   }
   fp.read_pointer_index += 8; // get out of riff chunk

   // read the WAVE chunk
   while (strncmp (wav_header.wave_chunk_id, "WAVE", 4) != 0)
   {
      if (fp.read_pointer_index + 4 > fp.file_size)
      {
         ServerConsole_printf ("RACC: PrecacheSoundForBots(): wave file %s: WAVE chunk ID not found!\n", sound_path);
         FREE_FILE (fp.data); // don't forget to free the wave file
         return; // avoid buffer overflows
      }
      memcpy (wav_header.wave_chunk_id, fp.data + fp.read_pointer_index, 4); // read wave id (4 bytes)
      fp.read_pointer_index++; // and get ahead one character
   }
   fp.read_pointer_index--; // step back again to get back to the start of the chunk
   if (fp.read_pointer_index + 3 < fp.file_size)
      memcpy (wav_header.wave_chunk_id, fp.data + fp.read_pointer_index, 4); // read wave chunk (4 bytes)
   else
   {
      ServerConsole_printf ("RACC: PrecacheSoundForBots(): wave file %s: WAVE chunk truncated!\n", sound_path);
      FREE_FILE (fp.data); // don't forget to free the wave file
      return; // avoid buffer overflows
   }
   fp.read_pointer_index += 4; // get out of wave chunk

   // read the FORMAT chunk
   while (strncmp (wav_header.fmt_chunk_id, "fmt ", 4) != 0)
   {
      if (fp.read_pointer_index + 4 > fp.file_size)
      {
         ServerConsole_printf ("RACC: PrecacheSoundForBots(): wave file %s: fmt_ chunk ID not found!\n", sound_path);
         FREE_FILE (fp.data); // don't forget to free the wave file
         return; // avoid buffer overflows
      }
      memcpy (wav_header.fmt_chunk_id, fp.data + fp.read_pointer_index, 4); // read format id (4 bytes)
      fp.read_pointer_index++; // and get ahead one character
   }
   fp.read_pointer_index--; // step back again to get back to the start of the chunk
   if (fp.read_pointer_index + 23 < fp.file_size)
      memcpy (wav_header.fmt_chunk_id, fp.data + fp.read_pointer_index, 24); // read format chunk (24 bytes)
   else
   {
      ServerConsole_printf ("RACC: PrecacheSoundForBots(): wave file %s: fmt_ chunk truncated!\n", sound_path);
      FREE_FILE (fp.data); // don't forget to free the wave file
      return; // avoid buffer overflows
   }
   fp.read_pointer_index += 24; // get out of format chunk

   // read the DATA chunk
   while (strncmp (wav_header.data_chunk_id, "data", 4) != 0)
   {
      if (fp.read_pointer_index + 4 > fp.file_size)
      {
         ServerConsole_printf ("RACC: PrecacheSoundForBots(): wave file %s: data chunk ID not found!\n", sound_path);
         FREE_FILE (fp.data); // don't forget to free the wave file
         return; // avoid buffer overflows
      }
      memcpy (wav_header.data_chunk_id, fp.data + fp.read_pointer_index, 4); // read data id (4 bytes)
      fp.read_pointer_index++; // and get ahead one character
   }
   fp.read_pointer_index--; // step back again to get back to the start of the chunk
   if (fp.read_pointer_index + 7 < fp.file_size)
      memcpy (wav_header.data_chunk_id, fp.data + fp.read_pointer_index, 8); // read START of data chunk (8 bytes)
   else
   {
      ServerConsole_printf ("RACC: PrecacheSoundForBots(): wave file %s: data chunk truncated!\n", sound_path);
      FREE_FILE (fp.data); // don't forget to free the wave file
      return; // avoid buffer overflows
   }

   // is that file of an unknown format ?
   if ((wav_header.bytes_per_sample != 1) && (wav_header.bytes_per_sample != 2))
   {
      ServerConsole_printf ("RACC: PrecacheSoundForBots(): wave file %s: invalid bytes per sample value (%d)!\n", sound_path, wav_header.bytes_per_sample);
      FREE_FILE (fp.data); // don't forget to free the wave file
      return; // avoid buffer overflows
   }

   fp.read_pointer_index += 8; // reach start of data

   // file ok, we can compute its loudness and fill one slot more in the global sounds array

   index = fp.read_pointer_index; // index will point at start of data now

   // read each sample, don't exceed end of file anyway...
   while (((unsigned long) index < fp.read_pointer_index + wav_header.data_chunk_length) && (index < fp.file_size))
   {
      iSample = 0x0000; // reset the container variable
      memcpy (&iSample, fp.data + index, wav_header.bytes_per_sample); // read N-byte sample
      sounds[sound_count].loudness += (float) fabs (iSample - (wav_header.bytes_per_sample == 1 ? 128 : 16384)) * (wav_header.bytes_per_sample == 1 ? 256 : 1) / (float) wav_header.data_chunk_length; // sum it
      index += wav_header.bytes_per_sample; // get to the next sample, N bytes further
   }

   // now store the duration and the sound path (relative to the MOD's sound folder)
   sounds[sound_count].duration = (float) wav_header.data_chunk_length / (float) wav_header.bytes_per_second;
   strcpy (sounds[sound_count].file_path, sound_path); // store the sound path in the array

   // if hearing debug level is high, print info about this sound
   if (DebugLevel.ears > 1)
      ServerConsole_printf ("sound #%d (%s) - length %f - noise %f\n", sound_count, sounds[sound_count].file_path, sounds[sound_count].duration, sounds[sound_count].loudness);

   FREE_FILE (fp.data); // don't forget to free the wave file
   sound_count++; // increment the number of sounds we know (we know one more now)
   return; // finished
}


void InitRicochetSounds (void)
{
   // this function is in charge of opening the ricochetsounds.cfg file in the knowledge/MOD
   // directory, and filling the ricochet sounds database accordingly. Each type of texture gets
   // attributed a different ricochet sound. Such a task should be performed once and only once,
   // preferably at GameDLLInit(), since ricochet sounds aren't likely to change between each map.

   FILE *fp;
   char filename[256];
   char line_buffer[256];
   int length;

   // first reset the ricochet sounds count
   ricochetsound_count = 0;

   // open the "ricochetsounds.cfg" file in the knowledge directory, in the MOD's folder
   sprintf (filename, "racc/knowledge/%s/ricochetsounds.cfg", server.mod_name);
   fp = fopen (filename, "r");
   if (fp == NULL)
   {
      ServerConsole_printf ("RACC: Unable to precache ricochet sounds (ricochetsounds.cfg file not found)\n");
      return; // if the file doesn't exist, then the bot won't know any gunshot sound at all
   }

   // for each line in the file...
   while ((fgets (line_buffer, 256, fp) != NULL) && (ricochetsound_count < RACC_MAX_RICOCHETSOUNDS))
   {
      length = strlen (line_buffer); // get length of line
      if ((length > 0) && (line_buffer[length - 1] == '\n'))
         length--; // remove any final '\n'
      line_buffer[length] = 0; // terminate the string

      if ((line_buffer[0] == 0) || (line_buffer[0] == '\n') || (line_buffer[0] == '#'))
         continue; // ignore line if void or commented

      // this line looks like a valid data line, prepare a slot for the ricochet sound
      ricochetsounds[ricochetsound_count].texture_type = GetField (line_buffer, 0)[0];
      sprintf (ricochetsounds[ricochetsound_count].file_path, GetField (line_buffer, 1));

      ricochetsound_count++; // the bots now distinguish one ricochet sound more
   }

   fclose (fp); // finished parsing the file, close it

   // print out how many sounds and textures we have precached
   ServerConsole_printf ("RACC: Bot sound system recognizes %d ricochet sounds\n", ricochetsound_count);
   return;
}
