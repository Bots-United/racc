// RACC - AI development project for first-person shooter games derivated from Valve's Half-Life
// (http://www.racc-ai.com/)
//
// This project is based on the work done by Jeffrey 'Botman' Broome in his HPB bot
// (http://planethalflife.com/botman/)
//
// This project is partially based on the work done by Eric Bieschke in his BSDbot
// (http://gamershomepage.com/csbot/)
//
// This project is partially based on the work done by Brendan "Spyro" McCarthy in his ODD Bot
// (http://oddbot.hlfusion.com/)
//
// This project is partially based on the work done by Alistair 'eLiTe' Stewart in his TEAMbot
// (http://www.planethalflife.com/teambot/)
//
// Rational Autonomous Cybernetic Commandos AI
//
// bot_chat.cpp
//

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "bot_common.h"

extern bot_t bots[32];
char bot_affirmative[5][100][256];
char bot_negative[5][100][256];
char bot_hello[5][100][256];
char bot_laugh[5][100][256];
char bot_whine[5][100][256];
char bot_idle[5][100][256];
char bot_follow[5][100][256];
char bot_stop[5][100][256];
char bot_stay[5][100][256];
char bot_help[5][100][256];
char bot_cant[5][100][256];
char bot_bye[5][100][256];
int recent_bot_affirmative[5][10];
int recent_bot_negative[5][10];
int recent_bot_hello[5][10];
int recent_bot_laugh[5][10];
int recent_bot_whine[5][10];
int recent_bot_idle[5][10];
int recent_bot_follow[5][10];
int recent_bot_stop[5][10];
int recent_bot_stay[5][10];
int recent_bot_help[5][10];
int recent_bot_cant[5][10];
int recent_bot_bye[5][10];
int affirmative_count[5];
int negative_count[5];
int hello_count[5];
int laugh_count[5];
int whine_count[5];
int idle_count[5];
int follow_count[5];
int stop_count[5];
int stay_count[5];
int help_count[5];
int cant_count[5];
int bye_count[5];

extern char *g_argv;


void BotSayText (bot_t *pBot)
{
   int bot_nationality, index, i, recent_count;
   bool used;
   char msg[128];
   char msg_humanized[128];

   if (pBot->pEdict == NULL)
      return; // reliability check

   // is it not time yet to speak ?
   if (pBot->f_bot_saytext_time > gpGlobals->time)
      return; // return; don't speak yet

   // is bot chat forbidden ?
   if (CVAR_GET_FLOAT ("racc_chatmode") == 0)
      return; // return; bots are not allowed to chat

   // check for validity of the racc_defaultbotnationality CVAR
   if ((CVAR_GET_FLOAT ("racc_defaultbotnationality") < 0) || (CVAR_GET_FLOAT ("racc_defaultbotnationality") > 4))
      CVAR_SET_FLOAT ("racc_defaultbotnationality", 0);

   // if foreigner bots are forbidden
   if (CVAR_GET_FLOAT ("racc_internationalmode") == 0)
      bot_nationality = CVAR_GET_FLOAT ("racc_defaultbotnationality");
   else
      bot_nationality = pBot->bot_nationality;

   // is it time to say affirmative ?
   if (pBot->BotChat.b_saytext_affirmative)
   {
      pBot->BotChat.b_saytext_affirmative = FALSE;

      if (pBot->pAskingEntity == NULL)
         return; // reliability check

      // audio chat
      pBot->BotChat.b_sayaudio_affirmative = TRUE;
      pBot->f_bot_sayaudio_time = gpGlobals->time + RANDOM_FLOAT (0.5, 3.0);

      // text chat
      recent_count = 0;

      while (recent_count < 10)
      {
         index = RANDOM_LONG (0, affirmative_count[bot_nationality] - 1);
         used = FALSE;

         for (i = 0; i < 10; i++)
            if (recent_bot_affirmative[bot_nationality][i] == index)
               used = TRUE;

         if (used)
            recent_count++;
         else
            break;
      }   

      for (i = 9; i > 0; i--)
         recent_bot_affirmative[bot_nationality][i] = recent_bot_affirmative[bot_nationality][i - 1];
      recent_bot_affirmative[bot_nationality][0] = index;

      if (strstr (bot_affirmative[bot_nationality][index], "%s") != NULL) // is "%s" in affirmative text ?
         sprintf (msg, bot_affirmative[bot_nationality][index], Name (STRING (pBot->pAskingEntity->v.netname)));
      else
         sprintf (msg, bot_affirmative[bot_nationality][index]);

      strcpy (msg_humanized, HumanizeChat (msg)); // humanize the chat string
      UTIL_HostSay (pBot->pEdict, 1, msg_humanized); // let bot say the chat string
      return;
   }

   // else is it time to say negative ?
   else if (pBot->BotChat.b_saytext_negative)
   {
      pBot->BotChat.b_saytext_negative = FALSE;

      if (pBot->pAskingEntity == NULL)
         return; // reliability check

      // audio chat
      pBot->BotChat.b_sayaudio_negative = TRUE;
      pBot->f_bot_sayaudio_time = gpGlobals->time + RANDOM_FLOAT (0.5, 3.0);

      // text chat
      recent_count = 0;

      while (recent_count < 10)
      {
         index = RANDOM_LONG (0, negative_count[bot_nationality] - 1);
         used = FALSE;

         for (i = 0; i < 10; i++)
            if (recent_bot_negative[bot_nationality][i] == index)
               used = TRUE;

         if (used)
            recent_count++;
         else
            break;
      }   

      for (i = 9; i > 0; i--)
         recent_bot_negative[bot_nationality][i] = recent_bot_negative[bot_nationality][i - 1];
      recent_bot_negative[bot_nationality][0] = index;

      if (strstr (bot_negative[bot_nationality][index], "%s") != NULL) // is "%s" in negative text ?
         sprintf (msg, bot_negative[bot_nationality][index], Name (STRING (pBot->pAskingEntity->v.netname)));
      else
         sprintf (msg, bot_negative[bot_nationality][index]);

      strcpy (msg_humanized, HumanizeChat (msg)); // humanize the chat string
      UTIL_HostSay (pBot->pEdict, 1, msg_humanized); // let bot say the chat string
      return;
   }

   // else is it time to say hello ?
   else if (pBot->BotChat.b_saytext_hello)
   {
      pBot->BotChat.b_saytext_hello = FALSE;
      recent_count = 0;

      while (recent_count < 10)
      {
         index = RANDOM_LONG (0, hello_count[bot_nationality] - 1);
         used = FALSE;

         for (i = 0; i < 10; i++)
            if (recent_bot_hello[bot_nationality][i] == index)
               used = TRUE;

         if (used)
            recent_count++;
         else
            break;
      }   

      for (i = 9; i > 0; i--)
         recent_bot_hello[bot_nationality][i] = recent_bot_hello[bot_nationality][i - 1];
      recent_bot_hello[bot_nationality][0] = index;

      sprintf (msg, bot_hello[bot_nationality][index]);
      strcpy (msg_humanized, HumanizeChat (msg)); // humanize the chat string
      UTIL_HostSay (pBot->pEdict, 0, msg_humanized); // let bot say the chat string
      return;
   }

   // else is it time to laugh ?
   else if (pBot->BotChat.b_saytext_kill)
   {
      pBot->BotChat.b_saytext_kill = FALSE;

      if (pBot->pVictimEntity == NULL)
         return; // reliability check

      recent_count = 0;

      while (recent_count < 10)
      {
         index = RANDOM_LONG (0, laugh_count[bot_nationality] - 1);
         used = FALSE;

         for (i = 0; i < 10; i++)
            if (recent_bot_laugh[bot_nationality][i] == index)
               used = TRUE;

         if (used)
            recent_count++;
         else
            break;
      }   

      for (i = 9; i > 0; i--)
         recent_bot_laugh[bot_nationality][i] = recent_bot_laugh[bot_nationality][i - 1];
      recent_bot_laugh[bot_nationality][0] = index;

      if (strstr (bot_laugh[bot_nationality][index], "%s") != NULL) // is "%s" in laugh text ?
         sprintf (msg, bot_laugh[bot_nationality][index], Name (STRING (pBot->pVictimEntity->v.netname)));
      else
         sprintf (msg, bot_laugh[bot_nationality][index]);

      strcpy (msg_humanized, HumanizeChat (msg)); // humanize the chat string
      UTIL_HostSay (pBot->pEdict, 0, msg_humanized); // let bot say the chat string
      return;
   }

   // else is it time to whine ?
   else if (pBot->BotChat.b_saytext_killed)
   {
      pBot->BotChat.b_saytext_killed = FALSE;

      if (pBot->pKillerEntity == NULL)
         return; // reliability check

      recent_count = 0;

      while (recent_count < 10)
      {
         index = RANDOM_LONG (0, whine_count[bot_nationality] - 1);
         used = FALSE;

         for (i = 0; i < 10; i++)
            if (recent_bot_whine[bot_nationality][i] == index)
               used = TRUE;

         if (used)
            recent_count++;
         else
            break;
      }

      for (i = 9; i > 0; i--)
         recent_bot_whine[bot_nationality][i] = recent_bot_whine[bot_nationality][i - 1];
      recent_bot_whine[bot_nationality][0] = index;

      if (strstr (bot_whine[bot_nationality][index], "%s") != NULL) // is "%s" in whine text ?
         sprintf (msg, bot_whine[bot_nationality][index], Name (STRING (pBot->pKillerEntity->v.netname)));
      else
         sprintf (msg, bot_whine[bot_nationality][index]);

      strcpy (msg_humanized, HumanizeChat (msg)); // humanize the chat string
      UTIL_HostSay (pBot->pEdict, 0, msg_humanized); // let bot say the chat string

      pBot->pKillerEntity = NULL; // bot can now forget his killer now
      return;
   }

   // else is it time to complain about loneliness ?
   else if (pBot->BotChat.b_saytext_idle)
   {
      bool found_one = FALSE;
      float bot_distance[32], farthest_bot_distance = 0.0;
      int farthest_bot_index;

      pBot->BotChat.b_saytext_idle = FALSE;

      if (pBot->pBotUser == NULL)
         return; // don't complain if bot is member of a squad

      // audio chat
      pBot->BotChat.b_sayaudio_report = TRUE;
      pBot->f_bot_sayaudio_time = gpGlobals->time + RANDOM_FLOAT (1.0, 5.0);

      // cycle all bots to find the farthest teammate
      for (i = 0; i < 32; i++)
      {
         // reliability check: is this slot unregistered
         if (bots[i].pEdict == NULL)
            continue;

         // is this one ourself OR dead OR inactive ?
         if ((bots[i].pEdict == pBot->pEdict) || !IsAlive (bots[i].pEdict) || (!bots[i].is_active))
            continue; // if so, skip to the next bot

         // is this one NOT a teammate ?
         if (GetTeam (bots[i].pEdict) != GetTeam (pBot->pEdict))
            continue; // if so, skip to the next bot

         found_one = TRUE;

         // how far away is the bot?
         bot_distance[i] = (pBot->pEdict->v.origin - bots[i].pEdict->v.origin).Length ();

         if (bot_distance[i] > farthest_bot_distance)
         {
            farthest_bot_distance = bot_distance[i];
            farthest_bot_index = i;
         }
      }

      // reliability check
      if (found_one)
      {
         // set the farthest teammate to reporting...
         bots[farthest_bot_index].BotChat.b_sayaudio_reporting = TRUE; // this bot will answer
         bots[farthest_bot_index].f_bot_sayaudio_time = pBot->f_bot_sayaudio_time + RANDOM_FLOAT (2.0, 3.0);
      }

      // text chat
      recent_count = 0;

      while (recent_count < 10)
      {
         index = RANDOM_LONG (0, idle_count[bot_nationality] - 1);
         used = FALSE;

         for (i = 0; i < 10; i++)
            if (recent_bot_idle[bot_nationality][i] == index)
               used = TRUE;

         if (used)
            recent_count++;
         else
            break;
      }

      for (i = 9; i > 0; i--)
         recent_bot_idle[bot_nationality][i] = recent_bot_idle[bot_nationality][i - 1];
      recent_bot_idle[bot_nationality][0] = index;

      if (strstr (bot_idle[bot_nationality][index], "%s") != NULL) // is "%s" in idle text ?
      {
         edict_t *pPlayer = pBot->pEdict; // reliability: fall back on a certified good edict
         i = 0; // incremental counter to avoid a possible endless loop, see below

         // pick up a random player, keep searching until one is found
         while (TRUE)
         {
            pPlayer = INDEXENT (RANDOM_LONG (1, gpGlobals->maxClients)); // pick up a random slot

            if (!pPlayer || !pPlayer->free || (pPlayer == pBot->pEdict))
               continue; // skip invalid players and skip self (i.e. this bot)

            if (GetTeam (pPlayer) == GetTeam (pBot->pEdict))
               continue; // skip teammates
            else
               break; // this one is an enemy, so it's OK to taunt him

            if (i >= 100)
               break; // still no player found ? oh well, won't do this all life long...

            i++; // increment the anti-endless loop conter
         }

         if (i < 100)
            sprintf (msg, bot_idle[bot_nationality][index], Name (STRING (pPlayer->v.netname)));
         else
            sprintf (msg, bot_idle[bot_nationality][index], "PieM"); // aargh, no player found !!
      }
      else
         sprintf (msg, bot_idle[bot_nationality][index]);

      strcpy (msg_humanized, HumanizeChat (msg)); // humanize the chat string
      UTIL_HostSay (pBot->pEdict, 0, msg_humanized); // let bot say the chat string
      return;
   }

   // else is it time to answer to a follow order ?
   else if (pBot->BotChat.b_saytext_follow)
   {
      pBot->BotChat.b_saytext_follow = FALSE;

      if (pBot->pAskingEntity == NULL)
         return; // reliability check

      // audio chat
      pBot->BotChat.b_sayaudio_affirmative = TRUE;
      pBot->f_bot_sayaudio_time = gpGlobals->time + RANDOM_FLOAT (0.0, 1.5);

      // text chat
      recent_count = 0;

      while (recent_count < 10)
      {
         index = RANDOM_LONG (0, follow_count[bot_nationality] - 1);
         used = FALSE;

         for (i = 0; i < 10; i++)
            if (recent_bot_follow[bot_nationality][i] == index)
               used = TRUE;

         if (used)
            recent_count++;
         else
            break;
      }

      for (i = 9; i > 0; i--)
         recent_bot_follow[bot_nationality][i] = recent_bot_follow[bot_nationality][i - 1];
      recent_bot_follow[bot_nationality][0] = index;

      if (strstr (bot_follow[bot_nationality][index], "%s") != NULL) // is "%s" in follow text ?
         sprintf (msg, bot_follow[bot_nationality][index], Name (STRING (pBot->pAskingEntity->v.netname)));
      else
         sprintf (msg, bot_follow[bot_nationality][index]);

      strcpy (msg_humanized, HumanizeChat (msg)); // humanize the chat string
      UTIL_HostSay (pBot->pEdict, 1, msg_humanized); // let bot say the chat string
      return;
   }

   // else is it time to answer to a stop order ?
   else if (pBot->BotChat.b_saytext_stop)
   {
      pBot->BotChat.b_saytext_stop = FALSE;

      if (pBot->pAskingEntity == NULL)
         return; // reliability check

      // audio chat
      pBot->BotChat.b_sayaudio_affirmative = TRUE;
      pBot->f_bot_sayaudio_time = gpGlobals->time + RANDOM_FLOAT (0.0, 1.5);

      // text chat
      recent_count = 0;

      while (recent_count < 10)
      {
         index = RANDOM_LONG (0, stop_count[bot_nationality] - 1);
         used = FALSE;

         for (i = 0; i < 10; i++)
            if (recent_bot_stop[bot_nationality][i] == index)
               used = TRUE;

         if (used)
            recent_count++;
         else
            break;
      }

      for (i = 9; i > 0; i--)
         recent_bot_stop[bot_nationality][i] = recent_bot_stop[bot_nationality][i - 1];
      recent_bot_stop[bot_nationality][0] = index;

      if (strstr (bot_stop[bot_nationality][index], "%s") != NULL) // is "%s" in stop text ?
         sprintf (msg, bot_stop[bot_nationality][index], Name (STRING (pBot->pAskingEntity->v.netname)));
      else
         sprintf (msg, bot_stop[bot_nationality][index]);

      strcpy (msg_humanized, HumanizeChat (msg)); // humanize the chat string
      UTIL_HostSay (pBot->pEdict, 1, msg_humanized); // let bot say the chat string
      return;
   }

   // else is it time to answer to a stay order ?
   else if (pBot->BotChat.b_saytext_stay)
   {
      pBot->BotChat.b_saytext_stay = FALSE;

      if (pBot->pAskingEntity == NULL)
         return; // reliability check

      // audio chat
      pBot->BotChat.b_sayaudio_inposition = TRUE;
      pBot->f_bot_sayaudio_time = gpGlobals->time + RANDOM_FLOAT (0.0, 1.5);

      // text chat
      recent_count = 0;

      while (recent_count < 10)
      {
         index = RANDOM_LONG (0, stay_count[bot_nationality] - 1);
         used = FALSE;

         for (i = 0; i < 10; i++)
            if (recent_bot_stay[bot_nationality][i] == index)
               used = TRUE;

         if (used)
            recent_count++;
         else
            break;
      }

      for (i = 9; i > 0; i--)
         recent_bot_stay[bot_nationality][i] = recent_bot_stay[bot_nationality][i - 1];
      recent_bot_stay[bot_nationality][0] = index;

      if (strstr (bot_stay[bot_nationality][index], "%s") != NULL) // is "%s" in stay text ?
         sprintf (msg, bot_stay[bot_nationality][index], Name (STRING (pBot->pAskingEntity->v.netname)));
      else
         sprintf (msg, bot_stay[bot_nationality][index]);

      strcpy (msg_humanized, HumanizeChat (msg)); // humanize the chat string
      UTIL_HostSay (pBot->pEdict, 1, msg_humanized); // let bot say the chat string
      return;
   }

   // else is it time to ask for backup ?
   else if (!pBot->b_help_asked && pBot->BotChat.b_saytext_help)
   {
      pBot->BotChat.b_saytext_help = FALSE;

      // audio chat
      pBot->BotChat.b_sayaudio_takingdamage = TRUE;
      pBot->f_bot_sayaudio_time = gpGlobals->time + RANDOM_FLOAT (0.5, 2.0);

      // text chat
      recent_count = 0;

      while (recent_count < 10)
      {
         index = RANDOM_LONG (0, help_count[bot_nationality] - 1);
         used = FALSE;

         for (i = 0; i < 10; i++)
            if (recent_bot_help[bot_nationality][i] == index)
               used = TRUE;

         if (used)
            recent_count++;
         else
            break;
      }

      for (i = 9; i > 0; i--)
         recent_bot_help[bot_nationality][i] = recent_bot_help[bot_nationality][i - 1];
      recent_bot_help[bot_nationality][0] = index;

      sprintf (msg, bot_help[bot_nationality][index]);
      strcpy (msg_humanized, HumanizeChat (msg)); // humanize the chat string
      UTIL_HostSay (pBot->pEdict, 1, msg_humanized); // let bot say the chat string
      return;
   }

   // else is it time to say that bot is lost while used ?
   else if (pBot->BotChat.b_saytext_cant)
   {
      pBot->BotChat.b_saytext_cant = FALSE; // don't say it twice

      // audio chat
      pBot->BotChat.b_sayaudio_negative = TRUE;
      pBot->f_bot_sayaudio_time = gpGlobals->time + RANDOM_FLOAT (0.5, 1.5);

      // text chat
      recent_count = 0;

      while (recent_count < 10)
      {
         index = RANDOM_LONG (0, cant_count[bot_nationality] - 1);
         used = FALSE;

         for (i = 0; i < 10; i++)
            if (recent_bot_cant[bot_nationality][i] == index)
               used = TRUE;

         if (used)
            recent_count++;
         else
            break;
      }

      for (i = 9; i > 0; i--)
         recent_bot_cant[bot_nationality][i] = recent_bot_cant[bot_nationality][i - 1];
      recent_bot_cant[bot_nationality][0] = index;

      if (strstr (bot_cant[bot_nationality][index], "%s") != NULL)  // is "%s" in can't text ?
         sprintf (msg, bot_cant[bot_nationality][index], Name (STRING (pBot->pBotUser->v.netname)));
      else
         sprintf (msg, bot_cant[bot_nationality][index]);

      strcpy (msg_humanized, HumanizeChat (msg)); // humanize the chat string
      UTIL_HostSay (pBot->pEdict, 1, msg_humanized); // let bot say the chat string
      pBot->pBotUser = NULL; // free the bot's user edict
      pBot->v_lastseenuser_position = Vector (0, 0, 0); // and also the last seen user position
      return;
   }

   // else is it time to say goodbye ?
   else if (pBot->BotChat.b_saytext_bye)
   {
      pBot->BotChat.b_saytext_bye = FALSE;
      recent_count = 0;

      while (recent_count < 10)
      {
         index = RANDOM_LONG (0, bye_count[bot_nationality] - 1);
         used = FALSE;

         for (i = 0; i < 10; i++)
            if (recent_bot_bye[bot_nationality][i] == index)
               used = TRUE;

         if (used)
            recent_count++;
         else
            break;
      }   

      for (i = 9; i > 0; i--)
         recent_bot_bye[bot_nationality][i] = recent_bot_bye[bot_nationality][i - 1];
      recent_bot_bye[bot_nationality][0] = index;

      sprintf (msg, bot_bye[bot_nationality][index]);
      strcpy (msg_humanized, HumanizeChat (msg)); // humanize the chat string
      UTIL_HostSay (pBot->pEdict, 0, msg_humanized); // let bot say the chat string
      return;
   }

   return; // bot has nothing to say, apparently...
}


void BotSayAudio (bot_t *pBot)
{
   char sound_path[256];
   int bot_nationality;
   char bot_language[32];
   int index;

   if (pBot->pEdict == NULL)
      return; // reliability check

   // is voice chat forbidden ?
   if (CVAR_GET_FLOAT ("racc_voicechatmode") == 0)
      return; // if so, return

   // check for validity of the racc_defaultbotnationality CVAR
   if ((CVAR_GET_FLOAT ("racc_defaultbotnationality") < 0) || (CVAR_GET_FLOAT ("racc_defaultbotnationality") > 4))
      CVAR_SET_FLOAT ("racc_defaultbotnationality", 0);

   // if foreigner bots are forbidden OR we need to play precached sounds (ambient mode)
   if ((CVAR_GET_FLOAT ("racc_internationalmode") == 0) || (CVAR_GET_FLOAT ("racc_voicechatmode") != 2))
      bot_nationality = CVAR_GET_FLOAT ("racc_defaultbotnationality");
   else
      bot_nationality = pBot->bot_nationality;

   // check in which language bot should speech, defaults to english if invalid
   if (bot_nationality == NATIONALITY_FRENCH)
      sprintf (bot_language, "french");
   else if (bot_nationality == NATIONALITY_GERMAN)
      sprintf (bot_language, "german");
   else if (bot_nationality == NATIONALITY_ITALIAN)
      sprintf (bot_language, "italian");
   else if (bot_nationality == NATIONALITY_SPANISH)
      sprintf (bot_language, "spanish");
   else
      sprintf (bot_language, "english");

   // is it time to say affirmative ?
   if (pBot->BotChat.b_sayaudio_affirmative && (pBot->f_bot_sayaudio_time < gpGlobals->time))
   {
      pBot->BotChat.b_sayaudio_affirmative = FALSE; // don't speak twice
      index = RANDOM_LONG (0, 5); // pickup a random chat message
      sprintf (sound_path, "../../racc/talk/%s/affirmative%d.wav", bot_language, index);
      BotTalk (pBot, sound_path); // ambient sound mode
   }

   // else is it time to say alert ?
   else if (pBot->BotChat.b_sayaudio_alert && (pBot->f_bot_sayaudio_time < gpGlobals->time))
   {
      pBot->BotChat.b_sayaudio_alert = FALSE; // don't speak twice
      index = RANDOM_LONG (0, 2); // pickup a random chat message
      sprintf (sound_path, "../../racc/talk/%s/alert%d.wav", bot_language, index);
      BotTalk (pBot, sound_path); // ambient sound mode
   }

   // else is it time to say attacking ?
   else if (pBot->BotChat.b_sayaudio_attacking && (pBot->f_bot_sayaudio_time < gpGlobals->time))
   {
      pBot->BotChat.b_sayaudio_attacking = FALSE; // don't speak twice
      index = RANDOM_LONG (0, 5); // pickup a random chat message
      sprintf (sound_path, "../../racc/talk/%s/attacking%d.wav", bot_language, index);
      BotTalk (pBot, sound_path); // ambient sound mode
   }

   // else is it time to say firstspawn ?
   else if (pBot->BotChat.b_sayaudio_firstspawn && (pBot->f_bot_sayaudio_time < gpGlobals->time))
   {
      pBot->BotChat.b_sayaudio_firstspawn = FALSE; // don't speak twice
      index = RANDOM_LONG (0, 4); // pickup a random chat message
      sprintf (sound_path, "../../racc/talk/%s/firstspawn%d.wav", bot_language, index);
      BotTalk (pBot, sound_path); // ambient sound mode
   }

   // else is it time to say inposition ?
   else if (pBot->BotChat.b_sayaudio_inposition && (pBot->f_bot_sayaudio_time < gpGlobals->time))
   {
      pBot->BotChat.b_sayaudio_inposition = FALSE; // don't speak twice
      index = RANDOM_LONG (0, 3); // pickup a random chat message
      sprintf (sound_path, "../../racc/talk/%s/inposition%d.wav", bot_language, index);
      BotTalk (pBot, sound_path); // ambient sound mode
   }

   // else is it time to say negative ?
   else if (pBot->BotChat.b_sayaudio_negative && (pBot->f_bot_sayaudio_time < gpGlobals->time))
   {
      pBot->BotChat.b_sayaudio_negative = FALSE; // don't speak twice
      index = 0; // pickup a random chat message
      sprintf (sound_path, "../../racc/talk/%s/negative%d.wav", bot_language, index);
      BotTalk (pBot, sound_path); // ambient sound mode
   }

   // else is it time to say report ?
   else if (pBot->BotChat.b_sayaudio_report && (pBot->f_bot_sayaudio_time < gpGlobals->time))
   {
      pBot->BotChat.b_sayaudio_report = FALSE; // don't speak twice
      index = RANDOM_LONG (0, 4); // pickup a random chat message
      sprintf (sound_path, "../../racc/talk/%s/report%d.wav", bot_language, index);
      BotTalk (pBot, sound_path); // ambient sound mode
   }

   // else is it time to say reporting ?
   else if (pBot->BotChat.b_sayaudio_reporting && (pBot->f_bot_sayaudio_time < gpGlobals->time))
   {
      pBot->BotChat.b_sayaudio_reporting = FALSE; // don't speak twice
      index = RANDOM_LONG (0, 17); // pickup a random chat message
      sprintf (sound_path, "../../racc/talk/%s/reporting%d.wav", bot_language, index);
      BotTalk (pBot, sound_path); // ambient sound mode
   }

   // else is it time to say seegrenade ?
   else if (pBot->BotChat.b_sayaudio_seegrenade && (pBot->f_bot_sayaudio_time < gpGlobals->time))
   {
      pBot->BotChat.b_sayaudio_seegrenade = FALSE; // don't speak twice
      index = RANDOM_LONG (0, 3); // pickup a random chat message
      sprintf (sound_path, "../../racc/talk/%s/seegrenade%d.wav", bot_language, index);
      BotTalk (pBot, sound_path); // ambient sound mode
   }

   // else is it time to say takingdamage ?
   else if (pBot->BotChat.b_sayaudio_takingdamage && (pBot->f_bot_sayaudio_time < gpGlobals->time))
   {
      pBot->BotChat.b_sayaudio_takingdamage = FALSE; // don't speak twice
      index = RANDOM_LONG (0, 7); // pickup a random chat message
      sprintf (sound_path, "../../racc/talk/%s/takingdamage%d.wav", bot_language, index);
      BotTalk (pBot, sound_path); // ambient sound mode
   }

   // else is it time to say throwgrenade ?
   else if (pBot->BotChat.b_sayaudio_throwgrenade && (pBot->f_bot_sayaudio_time < gpGlobals->time))
   {
      pBot->BotChat.b_sayaudio_throwgrenade = FALSE; // don't speak twice
      index = RANDOM_LONG (0, 4); // pickup a random chat message
      sprintf (sound_path, "../../racc/talk/%s/throwgrenade%d.wav", bot_language, index);
      BotTalk (pBot, sound_path); // ambient sound mode
   }

   // else is it time to say victory ?
   else if (pBot->BotChat.b_sayaudio_victory && (pBot->f_bot_sayaudio_time < gpGlobals->time))
   {
      pBot->BotChat.b_sayaudio_victory = FALSE; // don't speak twice
      index = RANDOM_LONG (0, 4); // pickup a random chat message
      sprintf (sound_path, "../../racc/talk/%s/victory%d.wav", bot_language, index);
      BotTalk (pBot, sound_path); // ambient sound mode
   }
}


void BotTalk (bot_t *pBot, char *sound_path)
{
   if (pBot->pEdict == NULL)
      return; // reliability check

   // do we want ambient speech audio chat ?
   if (CVAR_GET_FLOAT ("racc_voicechatmode") == 1)
   {
      EMIT_SOUND_DYN2 (pBot->pEdict, CHAN_VOICE, sound_path, VOL_NORM, ATTN_NORM, 0, RANDOM_FLOAT (95, 110));
      return;
   }

   // else do we want HLVoice-style audio chat ?
   else if (CVAR_GET_FLOAT ("racc_voicechatmode") == 2)
   {
      // cycle through all clients
      for (int clientindex = 1; clientindex <= gpGlobals->maxClients; clientindex++)
      {
         edict_t *pPlayer = INDEXENT (clientindex); // get entity of client index

         // skip invalid players and skip self (i.e. this bot)
         if (pPlayer && !pPlayer->free && IsAlive (pPlayer) && (pPlayer != pBot->pEdict))
         {
            // skip this player if not a real client
            if (!(pPlayer->v.flags & FL_CLIENT) || (pPlayer->v.flags & FL_FAKECLIENT))
               continue; // only talk to real clients

            // check if it is a teammate; if so, talk "in his head"
            if (GetTeam (pPlayer) == GetTeam (pBot->pEdict))
            {
               UTIL_DestroySpeakerIcon (pBot, pPlayer); // reset any previously displayed speaker on this bot
               sprintf (g_argv, "play %s\n", sound_path);
               CLIENT_COMMAND (pPlayer, g_argv); // play bot's talk on client side
               UTIL_DisplaySpeakerIcon (pBot, pPlayer, RANDOM_LONG (8, 22)); // display speaker icon
            }
         }
      }

      return;
   }

   // else audio chat is mute
   else
      return;
}


const char *Name (const char *string)
{
   int index, length;
   static char buffer[32];

   // drop the tag marks when speaking to someone 75 percent of time
   if (RANDOM_LONG (1, 100) < 75)
      strcpy (buffer, StripTags (string));
   else
      strcpy (buffer, StripBlanks (string));

   length = strlen (buffer); // get name buffer's length

   // half the time switch the name to lower characters
   if (RANDOM_LONG (1, 100) < 50)
      for (index = 0; index < length; index++)
         buffer[index] = tolower (buffer[index]); // switch name buffer to lowercase

   buffer[length] = 0; // terminate the string

   return &buffer[0]; // returns the name now in a humanized form
}


const char *HumanizeChat (const char *string)
{
   int index, length, pos;
   static char buffer[128];

   strcpy (buffer, StripBlanks (string)); // copy original string to buffer for processing
   length = strlen (buffer); // get length of string

   // 33 percent of time switch text to lowercase
   if (RANDOM_LONG (1, 100) < 33)
   {
      for (index = 0; index < length; index++)
         buffer[index] = tolower (buffer[index]); // switch buffer to lowercase
   }

   // if length is sufficient to assume the text had to be typed in a hurry
   if (length > 15)
   {
      // "length" percent of time drop a character
      if (RANDOM_LONG (1, 100) < length)
      {
         pos = RANDOM_LONG ((int) (length / 8), (int) (3 * length / 8)); // choose a random position in string

         for (index = pos; index < length - 1; index++)
            buffer[index] = buffer[index + 1]; // overwrite the buffer with the stripped string
         buffer[index] = 0; // terminate the string
         length--; // update new string length
      }

      // "length" / 2 percent of time swap a character
      if (RANDOM_LONG (1, 100) < length / 2)
      {
         char tempchar;
         pos = RANDOM_LONG ((int) (length / 8), (int) (3 * length / 8)); // choose a random position in string

         tempchar = buffer[pos]; // swap characters
         buffer[pos] = buffer[pos + 1];
         buffer[pos + 1] = tempchar;
      }
   }

   buffer[length] = 0; // terminate the string

   return &buffer[0]; // returns text now in a humanized form
}


const char *StripBlanks (const char *string)
{
   int index, length, fieldstart, fieldstop, i;
   static char buffer[128];

   strcpy (buffer, string); // copy original string to buffer for processing
   length = strlen (buffer); // get length of string

   // does the string has content ?
   if (length != 0)
   {
      index = 0; // let's now strip leading blanks
      while ((index < length) && ((buffer[index] == ' ') || (buffer[index] == '\t')))
         index++; // ignore any tabs or spaces, going forward from the start
      fieldstart = index; // save field start position

      index = length - 1; // let's now strip trailing blanks
      while ((index >= 0) && ((buffer[index] == ' ') || (buffer[index] == '\t')))
         index--; // ignore any tabs or spaces, going backwards from the end
      fieldstop = index; // save field stop position (before the '"')

      for (i = fieldstart; i <= fieldstop; i++)
         buffer[i - fieldstart] = buffer[i]; // now overwrite the buffer with the field value
      buffer[i - fieldstart] = 0; // terminate the string
   }

   return &buffer[0]; // and return the stripped out string
}


const char *StripTags (const char *string)
{
   char *tagstart[22] = {"-=", "-[", "-]", "-}", "-{", "<[", "<]", "[-", "]-", "{-", "}-", "[[", "[", "{", "]", "}", "<", ">", "-", "|", "=", "+"};
   char *tagstop[22] = {"=-", "]-", "[-", "{-", "}-", "]>", "[>", "-]", "-[", "-}", "-{", "]]", "]", "}", "[", "{", ">", "<", "-", "|", "=", "+"};
   int index, fieldstart, fieldstop, i, length;
   static char buffer[32];

   strcpy (buffer, StripBlanks (string)); // copy original string to buffer for processing
   length = strlen (buffer); // get length of string

   // for each known tag...
   for (index = 0; index < 22; index++)
   {
      fieldstart = &strstr (buffer, tagstart[index])[0] - &buffer[0]; // look for a tag start

      // have we found a tag start ?
      if ((fieldstart >= 0) && (fieldstart < 32))
      {
         fieldstop = &strstr (buffer, tagstop[index])[0] - &buffer[0]; // look for a tag stop

         // have we found a tag stop ?
         if ((fieldstop > fieldstart) && (fieldstop < 32))
         {
            for (i = fieldstart; i < length - (fieldstop + (int) strlen (tagstop[index]) - fieldstart); i++)
               buffer[i] = buffer[i + (fieldstop + (int) strlen (tagstop[index]) - fieldstart)]; // overwrite the buffer with the stripped string
            buffer[i] = 0; // terminate the string
         }
      }
   }

   // have we stripped too much (all the stuff) ?
   if (strlen (buffer) == 0)
   {
      // if so, string is just a tag
      strcpy (buffer, StripBlanks (string)); // copy original string to buffer for processing

      // strip just the tag part...
      for (index = 0; index < 22; index++)
      {
         fieldstart = &strstr (buffer, tagstart[index])[0] - &buffer[0]; // look for a tag start

         // have we found a tag start ?
         if ((fieldstart >= 0) && (fieldstart < 32))
         {
            fieldstop = fieldstart + strlen (tagstart[index]); // set the tag stop

            for (i = fieldstart; i < length - (int) strlen (tagstart[index]); i++)
               buffer[i] = buffer[i + (int) strlen (tagstart[index])]; // overwrite the buffer with the stripped string
            buffer[i] = 0; // terminate the string

            fieldstart = &strstr (buffer, tagstop[index])[0] - &buffer[0]; // look for a tag stop

            // have we found a tag stop ?
            if ((fieldstart >= 0) && (fieldstart < 32))
            {
               fieldstop = fieldstart + strlen (tagstop[index]); // set the tag stop

               for (i = fieldstart; i < length - (int) strlen (tagstop[index]); i++)
                  buffer[i] = buffer[i + (int) strlen (tagstop[index])]; // overwrite the buffer with the stripped string
               buffer[i] = 0; // terminate the string
            }
         }
      }
   }

   return &StripBlanks (buffer)[0]; // to finish, strip eventual blanks after and before the tag marks
}
