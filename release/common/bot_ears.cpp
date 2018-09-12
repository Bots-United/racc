// RACC - AI development project for first-person shooter games
// (http://racc.bots-united.com/)
//
// Rational Autonomous Cybernetic Commandos AI
//
// bot_ears.cpp
//

#include "racc.h"


void BotHear (player_t *pPlayer)
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

   pBotEar = &pPlayer->Bot.BotEars; // quick access to ear

   if (pBotEar->new_sound_index > BOT_EAR_SENSITIVITY - 1)
      pBotEar->new_sound_index = 0; // reliability check (var is initialized to unknown value)

   if (DebugLevel.ears_disabled)
      return; // return if we don't want the AI to hear

   // count the number of sounds being heard, and check if a sound has faded out
   for (index = 0; index < BOT_EAR_SENSITIVITY; index++)
      if (pBotEar->noises[index].fade_date > 0)
         if (pBotEar->noises[index].fade_date < server.time)
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
      if (pBotEar->noises[index].fade_date > server.time)
         new_average_noise += pBotEar->noises[index].loudness / noise_count;

   // TODO: MAKE SOUNDS DIRECTION RELATIVE (LEFT, RIGHT, FRONT, REAR)

   // does the bot hear a new sound ?
   if (pBotEar->new_sound)
   {
      // is the new average noise significant enough to trigger the bot's attention ?
      if (RandomFloat (0, pBotEar->average_noise) > fabs (new_average_noise - pBotEar->average_noise))
         BotReactToSound (pPlayer, &pBotEar->noises[pBotEar->new_sound_index]); // make the bot react to sound

      pBotEar->new_sound = FALSE; // don't do it twice
   }

   pBotEar->average_noise = new_average_noise; // save new average noise
   return; // finished hearing
}


noise_t *BotDoesHear (player_t *pPlayer, const char *sample)
{
   // this function returns a pointer to a sound sample in the bot's ears if the bot is
   // currently hearing the sound sample whose relative path to the sound file is specified
   // by the "sample" parameter. If the bot does not hear such a sound, we return NULL.

   int index;
   int length;

   length = strlen (sample); // get the length of the sample string we want to search for

   // cycle through all the sounds the bot currently hears
   for (index = 0; index < BOT_EAR_SENSITIVITY; index++)
      if ((pPlayer->Bot.BotEars.noises[index].file_path != NULL)
          && (strncmp (sample, pPlayer->Bot.BotEars.noises[index].file_path, length) == 0))
         return (&pPlayer->Bot.BotEars.noises[index]); // sample was found, bot hears it

   return (NULL); // bot does not hear this sound
}


void BotFeedEar (player_t *pPlayer, sound_t *sound, Vector v_origin, float volume)
{
   // this function is in charge of finding, or freeing if necessary, a slot in the bot's ears
   // to put the sound pointed to by sound in. If no free slot is available, the sound that is
   // the most about to finish gets overwritten.

   static bot_ears_t *pBotEar;
   float nearest_fade_date;
   char i, selected_index;

   if (sound == NULL)
      return; // reliability check

   pBotEar = &pPlayer->Bot.BotEars; // quick access to ear

   // find a free slot in bot's ears
   for (selected_index = 0; selected_index < BOT_EAR_SENSITIVITY; selected_index++)
      if (pBotEar->noises[selected_index].fade_date < server.time)
         break; // break when a free slot is found

   // have we found NO free slot ?
   if (selected_index == BOT_EAR_SENSITIVITY)
   {
      // no free slot found, so overwrite one, preferably the one most close to fade
      // FIXME: wrong rule - given several sounds, WHICH ONE are we likely to ignore the most ?
      nearest_fade_date = server.time + 60.0;
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
   pBotEar->noises[selected_index].direction = BotEstimateDirection (pPlayer, v_origin); // remember origin
   pBotEar->noises[selected_index].fade_date = server.time + sound->duration; // duration
   pBotEar->noises[selected_index].loudness = sound->loudness * volume; // loudness

   pBotEar->new_sound = TRUE; // notify the bot that it is hearing a new noise
   pBotEar->new_sound_index = selected_index; // mark new sound index for bot to check it

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
   if (DebugLevel.ears > 1)
      printf ("DispatchSound() \"%s\" from (%.1f, %.1f, %.1f): vol %.1f, att %.1f\n", sound->file_path, v_origin.x, v_origin.y, v_origin.z, volume, attenuation);

   // cycle through all bot slots
   for (bot_index = 0; bot_index < server.max_clients; bot_index++)
   {
      if (!IsValidPlayer (&players[bot_index]) || !players[bot_index].is_racc_bot)
         continue; // discard invalid players and real clients, only want active bots

      // is this sound NOT attenuated by distance ?
      if (attenuation == ATTN_NONE)
         BotFeedEar (&players[bot_index], sound, v_origin, volume); // if so, bot will hear it anyway

      // else is that bot within the maximum hearing range of the PAS ?
      // FIXME: ATM, I can't tell the difference between all the different attenuations !
      else if ((v_origin - players[bot_index].v_origin).Length () < GameConfig.max_hearing_distance)
      {
         attenuated_volume = volume * ((GameConfig.max_hearing_distance - (v_origin - players[bot_index].v_origin).Length ()) / GameConfig.max_hearing_distance);
         BotFeedEar (&players[bot_index], sound, v_origin, attenuated_volume); // bot hears attenuated sound
      }
   }

   return; // done, sound dispatched to all bots in range
}


void PlayBulletSoundsForBots (player_t *pPlayer)
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
   int sound_index;
   Vector v_gun_position;

   if (!IsValidPlayer (pPlayer) || !pPlayer->is_alive)
      return; // skip invalid and dead players

   if (DebugLevel.is_observer && !pPlayer->is_racc_bot)
      return; // skip real players if in observer mode

   if (!(pPlayer->input_buttons & (INPUT_KEY_FIRE1 | INPUT_KEY_FIRE2)))
      return; // cancel if player is not firing

   if (STRING (pPlayer->pEntity->v.weaponmodel)[0] == 0)
      return; // cancel if player has no weapon

   pPlayerWeapon = FindWeaponByModel (STRING (pPlayer->pEntity->v.weaponmodel)); // get player's weapon
   if (pPlayerWeapon->id == 0)
      return; // cancel if player has no weapon

   v_gun_position = pPlayer->v_eyeposition; // get this player's gun position

   // now select the sound according to rail (primary or secondary) and mode
   if (pPlayer->input_buttons & INPUT_KEY_FIRE1)
   {
      // primary rail
      if (pPlayer->pEntity->v.weaponanim == 0)
         DispatchSound (pPlayerWeapon->primary.sound1, v_gun_position, 1.0, ATTN_NORM);
      else
         DispatchSound (pPlayerWeapon->primary.sound2, v_gun_position, 1.0, ATTN_NORM);
   }
   else
   {
      // secondary rail
      if (pPlayer->pEntity->v.weaponanim == 0)
         DispatchSound (pPlayerWeapon->secondary.sound1, v_gun_position, 1.0, ATTN_NORM);
      else
         DispatchSound (pPlayerWeapon->secondary.sound2, v_gun_position, 1.0, ATTN_NORM);
   }

   // do we have to worry about ricochet sounds ?
   if (ricochetsound_count > 0)
   {
      // did this player's last traceline hit something AND it is not a player ?
      if ((pPlayer->tr.fraction < 1.0) && !FNullEnt (pPlayer->tr.pHit)
          && !(pPlayer->tr.pHit->v.flags & (FL_MONSTER | FL_CLIENT)))
      {
         // ask the engine for the texture name at the bullet hit point
         texture_name = TRACE_TEXTURE (pPlayer->tr.pHit, v_gun_position, pPlayer->tr.v_endposition);

         // if the engine found the texture, ask the MOD DLL for the texture type
         if (texture_name != NULL)
            texture_type = MDLL_PM_FindTextureType ((char *) texture_name); // ask for texture type

         // loop through all the ricochet sounds the bot knows until we find the right one
         for (sound_index = 0; sound_index < ricochetsound_count; sound_index++)
         {
            // is it this texture type the bullet just hit OR have we reached the default sound ?
            if ((texture_type == ricochetsounds[sound_index].texture_type)
                || (ricochetsounds[sound_index].texture_type == '*'))
               break; // then no need to search further
         }

         // bring this ricochet sound to the bots' ears
         DispatchSound (ricochetsounds[sound_index].file_path, pPlayer->tr.v_endposition, 0.9, ATTN_NORM);
      }
   }

   return;
}


void InitSounds (void)
{
   // this function is in charge of opening the sounds.cfg file in the knowledge/MOD directory,
   // and filling the game sounds database accordingly. Such a task only needs to be performed
   // once and only once, preferably at GameDLLInit(), since sounds aren't likely to change much
   // between each map, and our game sounds database is superior in size to the engine one.

   FILE *fp;
   char filename[256];
   char line_buffer[256];
   int length;

   // had we already allocated memory space for bot sounds ?
   if (sounds != NULL)
      free (sounds); // if so, free it
   sounds = NULL;
   sound_count = 0; // and reset the sounds count to zero

   // mallocate the minimal space for sounds
   sounds = (sound_t *) malloc (sizeof (sound_t));
   if (sounds == NULL)
      TerminateOnError ("InitSounds(): malloc() failure for bot sounds on %d bytes (out of memory ?)\n", sizeof (sound_t));

   // open the "sounds.cfg" file in the knowledge directory, in the MOD's folder
   sprintf (filename, "%s/knowledge/%s/sounds.cfg", GameConfig.racc_basedir, GameConfig.mod_name);
   fp = fopen (filename, "r");
   if (fp == NULL)
   {
      ServerConsole_printf ("RACC: Unable to precache game sounds (sounds.cfg file not found)\n");
      return; // if the file doesn't exist, then the bot won't know any gunshot sound at all
   }

   // read line per line...
   while (fgets (line_buffer, 256, fp) != NULL)
   {
      length = strlen (line_buffer); // get length of line
      if ((length > 0) && (line_buffer[length - 1] == '\n'))
         length--; // remove any final '\n'
      line_buffer[length] = 0; // terminate the string

      if ((line_buffer[0] == 0) || (line_buffer[0] == '\n')
          || (line_buffer[0] == ';') || (line_buffer[0] == '#'))
         continue; // ignore line if void or commented

      // we have another sound line, must allocate some space more
      sounds = (sound_t *) realloc (sounds, (sound_count + 1) * sizeof (sound_t));
      if (sounds == NULL)
         TerminateOnError ("InitSounds(): realloc() failure for bot sounds on %d bytes (out of memory ?)\n", (sound_count + 1) * sizeof (sound_t));

      // this line looks like a valid data line, prepare a slot for the new sound
      sprintf (sounds[sound_count].file_path, GetField (line_buffer, 0));
      sounds[sound_count].duration = atof (GetField (line_buffer, 2));
      sounds[sound_count].loudness = atof (GetField (line_buffer, 1));

      sound_count++; // the bots now is able to distinguish one sound more
   }

   fclose (fp); // finished parsing the file, close it

   // print out how many sounds and textures we have precached
   ServerConsole_printf ("RACC: Bot sound system identifies %d game sounds\n", sound_count);
   return;
}


void InitFootstepSounds (void)
{
   // this function is in charge of opening the footstepsounds.cfg file in the knowledge/MOD
   // directory, and filling the footsteps sounds database accordingly. Each type of texture gets
   // attributed a different footstep sound. Such a task should be performed once and only once,
   // preferably at GameDLLInit(), since footstep sounds aren't likely to change between each map.

   FILE *fp;
   char filename[256];
   char line_buffer[256];
   int length;

   // had we already allocated memory space for footstep sounds ?
   if (footstepsounds != NULL)
      free (footstepsounds); // if so, free it
   footstepsounds = NULL;
   footstepsound_count = 0; // and reset the sounds count to zero

   // mallocate the minimal space for footstep sounds
   footstepsounds = (footstepsound_t *) malloc (sizeof (footstepsound_t));
   if (footstepsounds == NULL)
      TerminateOnError ("InitFootstepSounds(): malloc() failure for footstep sounds on %d bytes (out of memory ?)\n", sizeof (footstepsound_t));

   // open the "footstepsounds.cfg" file in the knowledge directory, in the MOD's folder
   sprintf (filename, "%s/knowledge/%s/footstepsounds.cfg", GameConfig.racc_basedir, GameConfig.mod_name);
   fp = fopen (filename, "r");
   if (fp == NULL)
   {
      ServerConsole_printf ("RACC: Unable to precache footstep sounds (footstepsounds.cfg file not found)\n");
      return; // if the file doesn't exist, then the bot won't know any gunshot sound at all
   }

   // read line per line...
   while (fgets (line_buffer, 256, fp) != NULL)
   {
      length = strlen (line_buffer); // get length of line
      if ((length > 0) && (line_buffer[length - 1] == '\n'))
         length--; // remove any final '\n'
      line_buffer[length] = 0; // terminate the string

      if ((line_buffer[0] == 0) || (line_buffer[0] == '\n')
          || (line_buffer[0] == ';') || (line_buffer[0] == '#'))
         continue; // ignore line if void or commented

      // we have another footstep sound line, must allocate some space more
      footstepsounds = (footstepsound_t *) realloc (footstepsounds, (footstepsound_count + 1) * sizeof (footstepsound_t));
      if (footstepsounds == NULL)
         TerminateOnError ("InitFootstepSounds(): realloc() failure for footstep sounds on %d bytes (out of memory ?)\n", (footstepsound_count + 1) * sizeof (footstepsound_t));

      // this line looks like a valid data line, prepare a slot for the footstep sound
      footstepsounds[footstepsound_count].texture_type = GetField (line_buffer, 0)[0];
      footstepsounds[footstepsound_count].volume = atof (GetField (line_buffer, 1));
      sprintf (footstepsounds[footstepsound_count].file_path, GetField (line_buffer, 2));

      footstepsound_count++; // the bots now distinguish one footstep sound more
   }

   fclose (fp); // finished parsing the file, close it

   // print out how many sounds and textures we have precached
   ServerConsole_printf ("RACC: Bot sound system recognizes %d footstep sounds\n", footstepsound_count);
   return;
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

   // had we already allocated memory space for ricochet sounds ?
   if (ricochetsounds != NULL)
      free (ricochetsounds); // if so, free it
   ricochetsounds = NULL;
   ricochetsound_count = 0; // and reset the ricochet sounds count to zero

   // mallocate the minimal space for ricochet sounds
   ricochetsounds = (ricochetsound_t *) malloc (sizeof (ricochetsound_t));
   if (ricochetsounds == NULL)
      TerminateOnError ("InitRicochetSounds(): malloc() failure for ricochet sounds on %d bytes (out of memory ?)\n", sizeof (ricochetsound_t));

   // open the "ricochetsounds.cfg" file in the knowledge directory, in the MOD's folder
   sprintf (filename, "%s/knowledge/%s/ricochetsounds.cfg", GameConfig.racc_basedir, GameConfig.mod_name);
   fp = fopen (filename, "r");
   if (fp == NULL)
   {
      ServerConsole_printf ("RACC: Unable to precache ricochet sounds (ricochetsounds.cfg file not found)\n");
      return; // if the file doesn't exist, then the bot won't know any gunshot sound at all
   }

   // read line per line...
   while (fgets (line_buffer, 256, fp) != NULL)
   {
      length = strlen (line_buffer); // get length of line
      if ((length > 0) && (line_buffer[length - 1] == '\n'))
         length--; // remove any final '\n'
      line_buffer[length] = 0; // terminate the string

      if ((line_buffer[0] == 0) || (line_buffer[0] == '\n')
          || (line_buffer[0] == ';') || (line_buffer[0] == '#'))
         continue; // ignore line if void or commented

      // we have another ricochet sound line, must allocate some space more
      ricochetsounds = (ricochetsound_t *) realloc (ricochetsounds, (ricochetsound_count + 1) * sizeof (ricochetsound_t));
      if (ricochetsounds == NULL)
         TerminateOnError ("InitRicochetSounds(): realloc() failure for ricochet sounds on %d bytes (out of memory ?)\n", (ricochetsound_count + 1) * sizeof (ricochetsound_t));

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


void EvaluateSoundForBots (const char *sound_path)
{
   // this function displays the information structure used by bots about the specified WAV file.
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
   float duration;
   float loudness;
   signed long iSample;

   // look if that sound already exists in our array
   for (index = 0; index < sound_count; index++)
      if (strcmp (sound_path, sounds[index].file_path) == 0)
      {
         // if so, print out the previous evaluation from the sounds database
         ServerConsole_printf ("RACC: previous evaluation of \"%s\":\n", sound_path);
         ServerConsole_printf ("\"%s\"\t\t\t\t%f\t\t%f\n",
                               sounds[index].file_path,
                               sounds[index].duration,
                               sounds[index].loudness);
      }

   // ask the engine to locate and open that sound file
   sprintf (file_path, "sound/%s", sound_path); // build the sound path relative to game directory

   memset (&fp, 0, sizeof (fp)); // reset the memory-loaded file structure first
   fp.data = (char *) LOAD_FILE_FOR_ME (file_path, (int *) &fp.file_size); // load wave file
   if ((fp.data == NULL) || (fp.file_size == 0))
   {
      ServerConsole_printf ("RACC: EvaluateSoundForBots(): wave file %s not found!\n", sound_path);
      return; // if not found, then give up
   }

   // notify people about the new evaluation
   ServerConsole_printf ("RACC: evaluating sound \"%s\":\n", sound_path);

   // first reset the wav header structure
   memset (&wav_header, 0, sizeof (wav_header));

   // read the RIFF chunk
   while (strncmp (wav_header.riff_chunk_id, "RIFF", 4) != 0)
   {
      if (fp.read_pointer_index + 4 > fp.file_size)
      {
         ServerConsole_printf ("RACC: EvaluateSoundForBots(): wave file %s: RIFF chunk ID not found!\n", sound_path);
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
      ServerConsole_printf ("RACC: EvaluateSoundForBots(): wave file %s: RIFF chunk truncated!\n", sound_path);
      FREE_FILE (fp.data); // don't forget to free the wave file
      return; // avoid buffer overflows
   }
   fp.read_pointer_index += 8; // get out of riff chunk

   // read the WAVE chunk
   while (strncmp (wav_header.wave_chunk_id, "WAVE", 4) != 0)
   {
      if (fp.read_pointer_index + 4 > fp.file_size)
      {
         ServerConsole_printf ("RACC: EvaluateSoundForBots(): wave file %s: WAVE chunk ID not found!\n", sound_path);
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
      ServerConsole_printf ("RACC: EvaluateSoundForBots(): wave file %s: WAVE chunk truncated!\n", sound_path);
      FREE_FILE (fp.data); // don't forget to free the wave file
      return; // avoid buffer overflows
   }
   fp.read_pointer_index += 4; // get out of wave chunk

   // read the FORMAT chunk
   while (strncmp (wav_header.fmt_chunk_id, "fmt ", 4) != 0)
   {
      if (fp.read_pointer_index + 4 > fp.file_size)
      {
         ServerConsole_printf ("RACC: EvaluateSoundForBots(): wave file %s: fmt_ chunk ID not found!\n", sound_path);
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
      ServerConsole_printf ("RACC: EvaluateSoundForBots(): wave file %s: fmt_ chunk truncated!\n", sound_path);
      FREE_FILE (fp.data); // don't forget to free the wave file
      return; // avoid buffer overflows
   }
   fp.read_pointer_index += 24; // get out of format chunk

   // read the DATA chunk
   while (strncmp (wav_header.data_chunk_id, "data", 4) != 0)
   {
      if (fp.read_pointer_index + 4 > fp.file_size)
      {
         ServerConsole_printf ("RACC: EvaluateSoundForBots(): wave file %s: data chunk ID not found!\n", sound_path);
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
      ServerConsole_printf ("RACC: EvaluateSoundForBots(): wave file %s: data chunk truncated!\n", sound_path);
      FREE_FILE (fp.data); // don't forget to free the wave file
      return; // avoid buffer overflows
   }

   // is that file of an unknown format ?
   if ((wav_header.bytes_per_sample != 1) && (wav_header.bytes_per_sample != 2))
   {
      ServerConsole_printf ("RACC: EvaluateSoundForBots(): wave file %s: invalid bytes per sample value (%d)!\n", sound_path, wav_header.bytes_per_sample);
      FREE_FILE (fp.data); // don't forget to free the wave file
      return; // avoid buffer overflows
   }

   fp.read_pointer_index += 8; // reach start of data

   // file ok, we can compute its loudness and fill one slot more in the global sounds array

   index = fp.read_pointer_index; // index will point at start of data now

   // read each sample, don't exceed end of file anyway...
   loudness = 0;
   while (((unsigned long) index < fp.read_pointer_index + wav_header.data_chunk_length) && (index < fp.file_size))
   {
      iSample = 0x0000; // reset the container variable
      memcpy (&iSample, fp.data + index, wav_header.bytes_per_sample); // read N-byte sample
      loudness += (float) abs (iSample - (wav_header.bytes_per_sample == 1 ? 128 : 16384)) * (wav_header.bytes_per_sample == 1 ? 256 : 1) / (float) wav_header.data_chunk_length; // sum it
      index += wav_header.bytes_per_sample; // get to the next sample, N bytes further
   }

   // now store the duration and the sound path (relative to the MOD's sound folder)
   duration = (float) wav_header.data_chunk_length / (float) wav_header.bytes_per_second;

   // and print the results
   ServerConsole_printf ("\"%s\"\t\t\t\t%f\t\t\t\t%f\n", sound_path, duration, loudness);
   ServerConsole_printf ("Add this line to your sounds.cfg file.\n");

   FREE_FILE (fp.data); // don't forget to free the wave file
   sound_count++; // increment the number of sounds we know (we know one more now)
   return; // finished
}


sound_t *FindSoundByFilename (const char *sound_filename)
{
   // given a sound filename, this function finds the actual sound entry in the global sounds
   // database, so that we get access to its parameters (volume, duration, etc.)
   // If no sound with that filename is found in the database, return an empty static structure.

   // TODO: pre-sort sounds and perform faster binary search here.

   register int index;
   static sound_t unknown_sound = { "", 0.0, 0.0 };

   if (sound_filename == NULL)
      return (&unknown_sound); // reliability check

   // for each sound in the global array, check if the filename matchs
   for (index = 0; index < sound_count; index++)
      if (strcmp (sounds[index].file_path, sound_filename) == 0)
         return (&sounds[index]); // when found, return a pointer to the sound entry

   // damnit, sound not found !
   if (DebugLevel.ears > 0)
      ServerConsole_printf ("RACC: FindSoundByFilename(): sound \"%s\" not found in database\n", sound_filename);

   return (&unknown_sound); // return empty weapon
}
