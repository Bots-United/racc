// RACC - AI development project for first-person shooter games derived from Valve's Half-Life
// (http://www.racc-ai.com/)
//
// The game to engine interfacing code is based on the work done by Jeffrey 'botman' Broome
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

#include "racc.h"


void BotChat (bot_t *pBot)
{
   // the purpose of this function is to make the bot think what needs to be thought and do
   // what needs to be done (typing on the keyboard and such) for chatting and emitting messages.

   if (DebugLevel.chat_disabled)
      return; // return if we don't want the AI to chat

   // is bot chat forbidden ?
   if (server.bot_chat_mode == BOT_CHAT_NONE)
      return; // return; bots are not allowed to chat

   BotSayHAL (pBot);
   BotSayText (pBot);
   BotSayAudio (pBot);

   // has the bot finished typing its text ?
   if ((pBot->BotChat.f_saytext_time > 0) && (pBot->BotChat.f_saytext_time < *server.time))
   {
      FakeClientCommand (pBot->pEdict, pBot->BotChat.saytext_message); // let bot send the chat string
      pBot->BotChat.f_saytext_time = 0; // don't make the bot spam (once said is enough)
   }

   // has the bot finished browsing its audio menu ?
   if ((pBot->BotChat.f_sayaudio_time > 0) && (pBot->BotChat.f_sayaudio_time < *server.time))
   {
      BotTalk (pBot, pBot->BotChat.sayaudio_message); // let bot talk on the radio
      pBot->BotChat.f_sayaudio_time = 0; // don't keep it talking over and over again
   }

   return; // finished chatting
}


void BotSayHAL (bot_t *pBot)
{
   // the purpose of this function is to make the bot keep an eye on what's happening in the
   // chat room, and in case of new messages, think about a possible reply.

   // is text chat forbidden ?
   if (server.bot_chat_mode == BOT_CHAT_AUDIOONLY)
      return; // return; bots are not allowed to spam

   if (!pBot->BotEyes.BotHUD.chat.new_message)
      return; // return if nothing new in the chat area of the bot's HUD

   // is this message not the bot's ?
   if (pBot->BotEyes.BotHUD.chat.pSender != pBot->pEdict)
   {
      // break the new message into an array of words
      HAL_MakeWords (pBot->BotEyes.BotHUD.chat.text, pBot->BotBrain.input_words);

      // is the sender of the message NOT a bot ?
      if (!(pBot->BotEyes.BotHUD.chat.pSender->v.flags & FL_THIRDPARTYBOT))
         HAL_Learn (&pBot->BotBrain.HAL_model, pBot->BotBrain.input_words); // only learn from humans

      // does the bot feel concerned ? (more chances if its name appears)
      if ((RANDOM_LONG (1, 100) < 20 - (player_count / 2))
          || (strstr (pBot->BotEyes.BotHUD.chat.text, UpperCase (StripTags (STRING (pBot->pEdict->v.netname)))) != NULL))
      {
         // generate and humanize a HAL reply
         sprintf (pBot->BotChat.saytext_message, "say %s\n", HumanizeChat (BotHALGenerateReply (pBot)));

         // set the delay necessary for the bot to type in the reply
         pBot->BotChat.f_saytext_time = *server.time + (float) (strlen (pBot->BotChat.saytext_message) * pBot->pProfile->skill) / 10;
      }
   }

   pBot->BotEyes.BotHUD.chat.new_message = FALSE; // OK, we've seen that message
   return;
}


void BotSayText (bot_t *pBot)
{
   // the purpose of this function is to make the bot send a chat text that depends of one
   // of the 12 predefined situations : affirmative, negative, greetings, taunting, whining,
   // being idle, about following someone, stopping following someone, staying in position,
   // asking for backup, losing his master and saying goodbye before leaving.

   int index, i, recent_count;
   bool used;
   char msg[128];
   char bot_nationality;

   // is text chat forbidden ?
   if (server.bot_chat_mode == BOT_CHAT_AUDIOONLY)
      return; // return; bots are not allowed to chat

   bot_nationality = pBot->pProfile->nationality; // quick access to bot nationality

   // does the bot want to acknowledge ?
   if (pBot->BotChat.bot_saytext == BOT_SAYTEXT_AFFIRMATIVE)
   {
      pBot->BotChat.bot_saytext = 0;

      if (FNullEnt (pBot->BotEars.pAskingEntity))
         return; // reliability check

      // audio chat
      pBot->BotChat.bot_sayaudio |= BOT_SAYAUDIO_AFFIRMATIVE;
      pBot->BotChat.f_sayaudio_time = *server.time + RANDOM_FLOAT (0.5, 3.0);

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
         sprintf (msg, bot_affirmative[bot_nationality][index], Name (STRING (pBot->BotEars.pAskingEntity->v.netname)));
      else
         sprintf (msg, bot_affirmative[bot_nationality][index]);

      sprintf (pBot->BotChat.saytext_message, "say_team %s\n", HumanizeChat (msg)); // humanize the chat string

      // set the delay necessary for the bot to type in the reply
      pBot->BotChat.f_saytext_time = *server.time + (float) (strlen (msg) * pBot->pProfile->skill) / 10;
      return;
   }

   // else does the bot want to disagree ?
   else if (pBot->BotChat.bot_saytext == BOT_SAYTEXT_NEGATIVE)
   {
      pBot->BotChat.bot_saytext = 0;

      if (FNullEnt (pBot->BotEars.pAskingEntity))
         return; // reliability check

      // audio chat
      pBot->BotChat.bot_sayaudio |= BOT_SAYAUDIO_NEGATIVE;
      pBot->BotChat.f_sayaudio_time = *server.time + RANDOM_FLOAT (0.5, 3.0);

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
         sprintf (msg, bot_negative[bot_nationality][index], Name (STRING (pBot->BotEars.pAskingEntity->v.netname)));
      else
         sprintf (msg, bot_negative[bot_nationality][index]);

      sprintf (pBot->BotChat.saytext_message, "say_team %s\n", HumanizeChat (msg)); // humanize the chat string

      // set the delay necessary for the bot to type in the reply
      pBot->BotChat.f_saytext_time = *server.time + (float) (strlen (msg) * pBot->pProfile->skill) / 10;
      return;
   }

   // else does the bot want to say hello ?
   else if (pBot->BotChat.bot_saytext == BOT_SAYTEXT_HELLO)
   {
      pBot->BotChat.bot_saytext = 0;
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
      sprintf (pBot->BotChat.saytext_message, "say %s\n", HumanizeChat (msg)); // humanize the chat string

      // set the delay necessary for the bot to type in the reply
      pBot->BotChat.f_saytext_time = *server.time + (float) (strlen (msg) * pBot->pProfile->skill) / 10;
      return;
   }

   // else does the bot want to laugh at a dead enemy ?
   else if (pBot->BotChat.bot_saytext == BOT_SAYTEXT_LAUGH)
   {
      pBot->BotChat.bot_saytext = 0;

      if (FNullEnt (pBot->pVictimEntity))
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

      sprintf (pBot->BotChat.saytext_message, "say %s\n", HumanizeChat (msg)); // humanize the chat string

      // set the delay necessary for the bot to type in the reply
      pBot->BotChat.f_saytext_time = *server.time + (float) (strlen (msg) * pBot->pProfile->skill) / 10;
      return;
   }

   // else does the bot want to complain about being killed ?
   else if (pBot->BotChat.bot_saytext == BOT_SAYTEXT_WHINE)
   {
      pBot->BotChat.bot_saytext = 0;

      if (FNullEnt (pBot->pKillerEntity))
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

      pBot->pKillerEntity = NULL; // bot can now forget its murderer

      sprintf (pBot->BotChat.saytext_message, "say %s\n", HumanizeChat (msg)); // humanize the chat string

      // set the delay necessary for the bot to type in the reply
      pBot->BotChat.f_saytext_time = *server.time + (float) (strlen (msg) * pBot->pProfile->skill) / 10;
      return;
   }

   // else does the bot want to complain about being lonely for a long time ?
   else if (pBot->BotChat.bot_saytext == BOT_SAYTEXT_ALONE)
   {
      bool found_one = FALSE;
      float bot_distance[RACC_MAX_CLIENTS], farthest_bot_distance = 0.0;
      int farthest_bot_index;

      pBot->BotChat.bot_saytext = 0;

      if (FNullEnt (pBot->pBotUser))
         return; // don't complain if bot is member of a squad

      // audio chat (ask teammates to check in)
      pBot->BotChat.bot_sayaudio |= BOT_SAYAUDIO_REPORT;
      pBot->BotChat.f_sayaudio_time = *server.time + RANDOM_FLOAT (1.0, 3.0);

      // cycle all bots to find the farthest teammate
      for (i = 0; i < RACC_MAX_CLIENTS; i++)
      {
         // reliability check: is this slot unregistered
         if (FNullEnt (bots[i].pEdict))
            continue;

         // is this one ourself OR dead OR inactive ?
         if ((bots[i].pEdict == pBot->pEdict) || !players[i].is_alive || !bots[i].is_active)
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
         bots[farthest_bot_index].BotChat.bot_sayaudio |= BOT_SAYAUDIO_REPORTING; // this bot will answer
         bots[farthest_bot_index].BotChat.f_sayaudio_time = pBot->BotChat.f_sayaudio_time + RANDOM_FLOAT (3.5, 5.0);
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
         edict_t *pPlayer = NULL;
         i = 0; // incremental counter to avoid a possible endless loop, see below

         // pick up a random player, keep searching until one is found
         while (TRUE)
         {
            pPlayer = players[RANDOM_LONG (0, *server.max_clients - 1)].pEntity; // pick up a random slot

            if (FNullEnt (pPlayer) || pPlayer->free || (pPlayer == pBot->pEdict))
               continue; // skip invalid players and skip self (i.e. this bot)

            if (GetTeam (pPlayer) == GetTeam (pBot->pEdict))
               continue; // skip teammates
            else
               break; // this one is an enemy, so it's OK to taunt him

            if (i >= 32)
               break; // still no player found ? oh well, won't do this all life long...

            i++; // increment the anti-endless loop conter
         }

         if (FNullEnt (pPlayer))
            return; // reliability check : return if bot can't figure out who to talk to

         sprintf (msg, bot_idle[bot_nationality][index], Name (STRING (pPlayer->v.netname)));
      }
      else
         sprintf (msg, bot_idle[bot_nationality][index]);

      sprintf (pBot->BotChat.saytext_message, "say %s\n", HumanizeChat (msg)); // humanize the chat string

      // set the delay necessary for the bot to type in the reply
      pBot->BotChat.f_saytext_time = *server.time + (float) (strlen (msg) * pBot->pProfile->skill) / 10;
      return;
   }

   // else does the bot want to acknowledge to a "follow me" order ?
   else if (pBot->BotChat.bot_saytext == BOT_SAYTEXT_FOLLOWOK)
   {
      pBot->BotChat.bot_saytext = 0;

      if (FNullEnt (pBot->BotEars.pAskingEntity))
         return; // reliability check

      // audio chat
      pBot->BotChat.bot_sayaudio |= BOT_SAYAUDIO_AFFIRMATIVE;
      pBot->BotChat.f_sayaudio_time = *server.time + RANDOM_FLOAT (0.0, 1.5);

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
         sprintf (msg, bot_follow[bot_nationality][index], Name (STRING (pBot->BotEars.pAskingEntity->v.netname)));
      else
         sprintf (msg, bot_follow[bot_nationality][index]);

      sprintf (pBot->BotChat.saytext_message, "say_team %s\n", HumanizeChat (msg)); // humanize the chat string

      // set the delay necessary for the bot to type in the reply
      pBot->BotChat.f_saytext_time = *server.time + (float) (strlen (msg) * pBot->pProfile->skill) / 10;
      return;
   }

   // else does the bot want to acknowledge to a "stop following me" order ?
   else if (pBot->BotChat.bot_saytext == BOT_SAYTEXT_STOPOK)
   {
      pBot->BotChat.bot_saytext = 0;

      if (FNullEnt (pBot->BotEars.pAskingEntity))
         return; // reliability check

      // audio chat
      pBot->BotChat.bot_sayaudio |= BOT_SAYAUDIO_AFFIRMATIVE;
      pBot->BotChat.f_sayaudio_time = *server.time + RANDOM_FLOAT (0.0, 1.5);

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
         sprintf (msg, bot_stop[bot_nationality][index], Name (STRING (pBot->BotEars.pAskingEntity->v.netname)));
      else
         sprintf (msg, bot_stop[bot_nationality][index]);

      sprintf (pBot->BotChat.saytext_message, "say_team %s\n", HumanizeChat (msg)); // humanize the chat string

      // set the delay necessary for the bot to type in the reply
      pBot->BotChat.f_saytext_time = *server.time + (float) (strlen (msg) * pBot->pProfile->skill) / 10;
      return;
   }

   // else does the bot want to acknowledge to a "hold the position" order ?
   else if (pBot->BotChat.bot_saytext == BOT_SAYTEXT_HOLDPOSITIONOK)
   {
      pBot->BotChat.bot_saytext = 0;

      if (FNullEnt (pBot->BotEars.pAskingEntity))
         return; // reliability check

      // audio chat
      pBot->BotChat.bot_sayaudio |= BOT_SAYAUDIO_INPOSITION;
      pBot->BotChat.f_sayaudio_time = *server.time + RANDOM_FLOAT (0.0, 1.5);

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
         sprintf (msg, bot_stay[bot_nationality][index], Name (STRING (pBot->BotEars.pAskingEntity->v.netname)));
      else
         sprintf (msg, bot_stay[bot_nationality][index]);

      sprintf (pBot->BotChat.saytext_message, "say_team %s\n", HumanizeChat (msg)); // humanize the chat string

      // set the delay necessary for the bot to type in the reply
      pBot->BotChat.f_saytext_time = *server.time + (float) (strlen (msg) * pBot->pProfile->skill) / 10;
      return;
   }

   // else does the bot want to ask for backup ?
   else if (pBot->BotChat.bot_saytext == BOT_SAYTEXT_NEEDBACKUP)
   {
      pBot->BotChat.bot_saytext = 0;

      // audio chat
      pBot->BotChat.bot_sayaudio |= BOT_SAYAUDIO_TAKINGDAMAGE;
      pBot->BotChat.f_sayaudio_time = *server.time + RANDOM_FLOAT (0.5, 2.0);

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
      sprintf (pBot->BotChat.saytext_message, "say_team %s\n", HumanizeChat (msg)); // humanize the chat string

      // set the delay necessary for the bot to type in the reply
      pBot->BotChat.f_saytext_time = *server.time + (float) (strlen (msg) * pBot->pProfile->skill) / 10;
      return;
   }

   // else does the bot want to say to its squad superior it can't follow him anymore ?
   else if (pBot->BotChat.bot_saytext == BOT_SAYTEXT_CANTFOLLOW)
   {
      pBot->BotChat.bot_saytext = 0;

      // audio chat
      pBot->BotChat.bot_sayaudio |= BOT_SAYAUDIO_NEGATIVE;
      pBot->BotChat.f_sayaudio_time = *server.time + RANDOM_FLOAT (0.5, 1.5);

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

      pBot->pBotUser = NULL; // free the bot's user edict
      pBot->v_lastseenuser_position = g_vecZero; // and also the last seen user position

      sprintf (pBot->BotChat.saytext_message, "say_team %s\n", HumanizeChat (msg)); // humanize the chat string

      // set the delay necessary for the bot to type in the reply
      pBot->BotChat.f_saytext_time = *server.time + (float) (strlen (msg) * pBot->pProfile->skill) / 10;
      return;
   }

   // else does the bot want to say goodbye ?
   else if (pBot->BotChat.bot_saytext == BOT_SAYTEXT_BYE)
   {
      pBot->BotChat.bot_saytext = 0;
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
      sprintf (pBot->BotChat.saytext_message, "say %s\n", HumanizeChat (msg)); // humanize the chat string

      // set the delay necessary for the bot to type in the reply
      pBot->BotChat.f_saytext_time = *server.time + (float) (strlen (msg) * pBot->pProfile->skill) / 10;
      return;
   }

   return; // bot has nothing to say, apparently...
}


void BotSayAudio (bot_t *pBot)
{
   // the purpose of this function is the same as BotSayText(), making the bot speak (by
   // emitting sound samples) according to the 12 predefined in-game audio chat situations :
   // affirmative, alert, when attacking, at spawn time, getting in position, asking team
   // to report, reporting to teammates, upon the sight of an enemy grenade, when taking some
   // damage, when throwing a grenade and upon victory in a battle.

   char bot_nationality, bot_language[32];
   int index;

   // is voice chat forbidden ?
   if ((server.bot_chat_mode == BOT_CHAT_NONE) || (server.bot_chat_mode == BOT_CHAT_TEXTONLY))
      return; // if so, return

   if (!IsAlive (pBot->pEdict))
      return; // if bot is dead, return

   bot_nationality = pBot->pProfile->nationality; // quick access to bot nationality

   // check in which language bot should speech, defaults to english if invalid
   if (bot_nationality == NATIONALITY_FRENCH)
      strcpy (bot_language, "french");
   else if (bot_nationality == NATIONALITY_GERMAN)
      strcpy (bot_language, "german");
   else if (bot_nationality == NATIONALITY_ITALIAN)
      strcpy (bot_language, "italian");
   else if (bot_nationality == NATIONALITY_SPANISH)
      strcpy (bot_language, "spanish");
   else
      strcpy (bot_language, "english");

   // is it time to say affirmative ?
   if (pBot->BotChat.bot_sayaudio & BOT_SAYAUDIO_AFFIRMATIVE)
   {
      pBot->BotChat.bot_sayaudio &= ~BOT_SAYAUDIO_AFFIRMATIVE; // don't speak twice
      index = RANDOM_LONG (0, audio_affirmative_count[bot_nationality] - 1); // pickup a random chat message
      sprintf (pBot->BotChat.sayaudio_message, "racc/%s/affirmative%d.wav", bot_language, index);
   }

   // else is it time to say alert ?
   else if (pBot->BotChat.bot_sayaudio & BOT_SAYAUDIO_ALERT)
   {
      pBot->BotChat.bot_sayaudio &= ~BOT_SAYAUDIO_ALERT; // don't speak twice
      index = RANDOM_LONG (0, audio_alert_count[bot_nationality] - 1); // pickup a random chat message
      sprintf (pBot->BotChat.sayaudio_message, "racc/%s/alert%d.wav", bot_language, index);
   }

   // else is it time to say attacking ?
   else if (pBot->BotChat.bot_sayaudio & BOT_SAYAUDIO_ATTACKING)
   {
      pBot->BotChat.bot_sayaudio &= ~BOT_SAYAUDIO_ATTACKING; // don't speak twice
      index = RANDOM_LONG (0, audio_attacking_count[bot_nationality] - 1); // pickup a random chat message
      sprintf (pBot->BotChat.sayaudio_message, "racc/%s/attacking%d.wav", bot_language, index);
   }

   // else is it time to say firstspawn ?
   else if (pBot->BotChat.bot_sayaudio & BOT_SAYAUDIO_FIRSTSPAWN)
   {
      pBot->BotChat.bot_sayaudio &= ~BOT_SAYAUDIO_FIRSTSPAWN; // don't speak twice
      index = RANDOM_LONG (0, audio_firstspawn_count[bot_nationality] - 1); // pickup a random chat message
      sprintf (pBot->BotChat.sayaudio_message, "racc/%s/firstspawn%d.wav", bot_language, index);
   }

   // else is it time to say inposition ?
   else if (pBot->BotChat.bot_sayaudio & BOT_SAYAUDIO_INPOSITION)
   {
      pBot->BotChat.bot_sayaudio &= ~BOT_SAYAUDIO_INPOSITION; // don't speak twice
      index = RANDOM_LONG (0, audio_inposition_count[bot_nationality] - 1); // pickup a random chat message
      sprintf (pBot->BotChat.sayaudio_message, "racc/%s/inposition%d.wav", bot_language, index);
   }

   // else is it time to say negative ?
   else if (pBot->BotChat.bot_sayaudio & BOT_SAYAUDIO_NEGATIVE)
   {
      pBot->BotChat.bot_sayaudio &= ~BOT_SAYAUDIO_NEGATIVE; // don't speak twice
      index = RANDOM_LONG (0, audio_negative_count[bot_nationality] - 1); // pickup a random chat message
      sprintf (pBot->BotChat.sayaudio_message, "racc/%s/negative%d.wav", bot_language, index);
   }

   // else is it time to say report ?
   else if (pBot->BotChat.bot_sayaudio & BOT_SAYAUDIO_REPORT)
   {
      pBot->BotChat.bot_sayaudio &= ~BOT_SAYAUDIO_REPORT; // don't speak twice
      index = RANDOM_LONG (0, audio_report_count[bot_nationality] - 1); // pickup a random chat message
      sprintf (pBot->BotChat.sayaudio_message, "racc/%s/report%d.wav", bot_language, index);
   }

   // else is it time to say reporting ?
   else if (pBot->BotChat.bot_sayaudio & BOT_SAYAUDIO_REPORTING)
   {
      pBot->BotChat.bot_sayaudio &= ~BOT_SAYAUDIO_REPORTING; // don't speak twice
      index = RANDOM_LONG (0, audio_reporting_count[bot_nationality] - 1); // pickup a random chat message
      sprintf (pBot->BotChat.sayaudio_message, "racc/%s/reporting%d.wav", bot_language, index);
   }

   // else is it time to say seegrenade ?
   else if (pBot->BotChat.bot_sayaudio & BOT_SAYAUDIO_SEEGRENADE)
   {
      pBot->BotChat.bot_sayaudio &= ~BOT_SAYAUDIO_SEEGRENADE; // don't speak twice
      index = RANDOM_LONG (0, audio_seegrenade_count[bot_nationality] - 1); // pickup a random chat message
      sprintf (pBot->BotChat.sayaudio_message, "racc/%s/seegrenade%d.wav", bot_language, index);
   }

   // else is it time to say takingdamage ?
   else if (pBot->BotChat.bot_sayaudio & BOT_SAYAUDIO_TAKINGDAMAGE)
   {
      pBot->BotChat.bot_sayaudio &= ~BOT_SAYAUDIO_TAKINGDAMAGE; // don't speak twice
      index = RANDOM_LONG (0, audio_takingdamage_count[bot_nationality] - 1); // pickup a random chat message
      sprintf (pBot->BotChat.sayaudio_message, "racc/%s/takingdamage%d.wav", bot_language, index);
   }

   // else is it time to say throwgrenade ?
   else if (pBot->BotChat.bot_sayaudio & BOT_SAYAUDIO_THROWGRENADE)
   {
      pBot->BotChat.bot_sayaudio &= ~BOT_SAYAUDIO_THROWGRENADE; // don't speak twice
      index = RANDOM_LONG (0, audio_throwgrenade_count[bot_nationality] - 1); // pickup a random chat message
      sprintf (pBot->BotChat.sayaudio_message, "racc/%s/throwgrenade%d.wav", bot_language, index);
   }

   // else is it time to say victory ?
   else if (pBot->BotChat.bot_sayaudio & BOT_SAYAUDIO_VICTORY)
   {
      pBot->BotChat.bot_sayaudio &= ~BOT_SAYAUDIO_VICTORY; // don't speak twice
      index = RANDOM_LONG (0, audio_victory_count[bot_nationality] - 1); // pickup a random chat message
      sprintf (pBot->BotChat.sayaudio_message, "racc/%s/victory%d.wav", bot_language, index);
   }

   return; // finished
}


void BotTalk (bot_t *pBot, char *sound_path)
{
   // this is the function that fakes the HLVoice-style chatting for bots. Here each client is
   // ordered to play this sound locally, this resulting in non attenuated sound, simulating
   // a HLVoice conversation. A HLVoice-style icon is also displayed on top of the bot's head
   // for the duration of the sound, adding a little to credibility.

   int index;
   edict_t *pPlayer;

   // is audio chat forbidden ?
   if ((server.bot_chat_mode == BOT_CHAT_NONE) || (server.bot_chat_mode == BOT_CHAT_TEXTONLY))
      return; // return; bots are not allowed to talk

   // cycle through all clients
   for (index = 0; index < *server.max_clients; index++)
   {
      pPlayer = players[index].pEntity; // quick access to player

      // skip invalid players and skip self (i.e. this bot)
      if (IsValidPlayer (pPlayer) && IsAlive (pPlayer) && (pPlayer != pBot->pEdict))
      {
         // skip this player if not a real client
         if (pPlayer->v.flags & (FL_THIRDPARTYBOT | FL_FAKECLIENT))
            continue; // only talk to real clients

         // check if it is a teammate; if so, talk "in his head"
         if (GetTeam (pPlayer) == GetTeam (pBot->pEdict))
         {
            DestroySpeakerIcon (pBot, pPlayer); // reset any previously displayed speaker on this bot
            sprintf (g_argv, "play %s\n", sound_path);
            CLIENT_COMMAND (pPlayer, g_argv); // play bot's talk on client side
            DisplaySpeakerIcon (pBot, pPlayer, RANDOM_LONG (8, 22)); // display speaker icon
         }
      }
   }

   return;
}


void DisplaySpeakerIcon (bot_t *pBot, edict_t *pViewerClient, int duration)
{
   // this function is supposed to display that tiny speaker icon above the head of the player
   // whose entity is pointed to by pPlayer, so that pViewerClient sees it, during duration * 10
   // seconds long. That's not exactly what the engine does when a client uses the voice system,
   // but that's all I've found so far to simulate it.

   MESSAGE_BEGIN (MSG_ONE, GetUserMsgId ("TempEntity"), NULL, pViewerClient);
   WRITE_BYTE (TE_PLAYERATTACHMENT); // thanks to Count Floyd for the trick !
   WRITE_BYTE (ENTINDEX (pBot->pEdict)); // byte (entity index of pEdict)
   WRITE_COORD (voiceicon_height + pBot->pEdict->v.view_ofs.z + 34 * (pBot->BotMove.f_duck_time > *server.time)); // coord (vertical offset)
   WRITE_SHORT (speaker_model); // short (model index of tempent)
   WRITE_SHORT (duration); // short (life * 10) e.g. 40 = 4 seconds
   MESSAGE_END (); 
}


void DestroySpeakerIcon (bot_t *pBot, edict_t *pViewerClient)
{
   // this function stops displaying any speaker icon above the head of the player whose entity
   // is pointed to by pPlayer, so that pViewerClient doesn't see them anymore. Actually it also
   // stops displaying any player attachment temporary entity. One day I swear I'll get rid of
   // that crap. Soon.

   MESSAGE_BEGIN (MSG_ONE, GetUserMsgId ("TempEntity"), NULL, pViewerClient);
   WRITE_BYTE (TE_KILLPLAYERATTACHMENTS); // destroy all temporary entities attached to pBot
   WRITE_BYTE (ENTINDEX (pBot->pEdict)); // byte (entity index of pEdict)
   MESSAGE_END (); 
}


const char *Name (const char *string)
{
   // assuming string is a player name, this function returns a random equivalent in a
   // "humanized" form, i.e. without the tag marks, without trailing spaces, and optionally
   // turned into lower case. The return string is held into a static buffer, so be sure not
   // to use it directly.

   int index, length;
   static char buffer[32];

   // drop the tag marks when speaking to someone 85 percent of time
   if (RANDOM_LONG (1, 100) < 85)
      strcpy (buffer, StripTags (string));
   else
      strcpy (buffer, StripBlanks (string));

   length = strlen (buffer); // get name buffer's length

   // half the time switch the name to lower characters
   if (RANDOM_LONG (1, 100) < 50)
      for (index = 0; index < length; index++)
         buffer[index] = tolower (buffer[index]); // switch name buffer to lowercase

   buffer[length] = 0; // terminate the string

   return (&buffer[0]); // returns the name now in a humanized form
}


const char *HumanizeChat (const char *string)
{
   // this function randomizes a character string by dropping a character, swapping characters,
   // eventually turning it all to lowercase, and stripping leading and trailing blanks, in
   // order for the resulting string to look like if it had beed typed by a player in a hurry.
   // The return string is held into a static buffer, so be sure not to use it directly.
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
         pos = RANDOM_LONG (length / 8, length); // choose a random position in string

         for (index = pos; index < length - 1; index++)
            buffer[index] = buffer[index + 1]; // overwrite the buffer with the stripped string
         buffer[index] = 0; // terminate the string
         length--; // update new string length
      }

      // "length" / 2 percent of time swap a character
      if (RANDOM_LONG (1, 100) < length / 2)
      {
         char tempchar;
         pos = RANDOM_LONG (length / 8, length); // choose a random position in string

         tempchar = buffer[pos]; // swap characters
         buffer[pos] = buffer[pos + 1];
         buffer[pos + 1] = tempchar;
      }
   }

   buffer[length] = 0; // terminate the string

   return (&buffer[0]); // returns text now in a humanized form
}


const char *StripBlanks (const char *string)
{
   // this function returns a string which is a copy of the input character string, stripped of
   // its leading and trailing whitespaces. The return string is held into a static buffer, so
   // be sure not to use it directly.

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

   return (&buffer[0]); // and return the stripped out string
}


const char *StripTags (const char *string)
{
   // the purpose of this function is to strip the obnoxious "clan tags" that often appear in
   // player names, in order to have only the player name in the return string, which is held
   // into a static buffer, so be sure not to use it directly.

   char *tagstart[23] = {"-=", "-[", "-]", "-}", "-{", "<[", "<]", "[-", "]-", "{-", "}-", "[[", ".:", "[", "{", "]", "}", "<", ">", "-", "|", "=", "+"};
   char *tagstop[23] = {"=-", "]-", "[-", "{-", "}-", "]>", "[>", "-]", "-[", "-}", "-{", "]]", ":.", "]", "}", "[", "{", ">", "<", "-", "|", "=", "+"};
   int index, fieldstart, fieldstop, i, length;
   static char buffer[32];

   strcpy (buffer, StripBlanks (string)); // copy original string to buffer and strip blanks
   length = strlen (buffer); // get length of string

   // for each known tag...
   for (index = 0; index < 22; index++)
   {
      fieldstart = &strstr (buffer, tagstart[index])[0] - &buffer[0]; // look for a tag start

      // have we found a tag start in the string ?
      if ((fieldstart >= 0) && (fieldstart < 32))
      {
         fieldstop = &strstr (buffer, tagstop[index])[0] - &buffer[0]; // look for a tag stop

         // have we found a tag stop in the string ?
         if ((fieldstop > fieldstart) && (fieldstop < 32))
         {
            // overwrite the buffer with the stripped string
            for (i = fieldstart; i < length - (fieldstop + (int) strlen (tagstop[index]) - fieldstart); i++)
               buffer[i] = buffer[i + (fieldstop + (int) strlen (tagstop[index]) - fieldstart)];
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

            // overwrite the buffer with the stripped string
            for (i = fieldstart; i < length - (int) strlen (tagstart[index]); i++)
               buffer[i] = buffer[i + (int) strlen (tagstart[index])];

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

   return (&StripBlanks (buffer)[0]); // to finish, strip eventual blanks after and before the tag marks
}


const char *NormalizeChars (const char *string)
{
   // this function converts a string to a representation of it that does not contain invalid
   // characters for the filesystem (i.e, slashes, backslashes, colons, asterisks, etc.)

   int index, length;
   static char buffer[128];

   strcpy (buffer, string); // copy original string to buffer for processing
   length = strlen (buffer); // get length of string

   // let's now normalize the special characters
   for (index = 0; index < length; index++)
   {
      if ((buffer[index] == '\\') || (buffer[index] == '/'))
        buffer[index] = '-';
      else if (buffer[index] == ':')
        buffer[index] = '=';
      else if (buffer[index] == '*')
        buffer[index] = 'x';
      else if (buffer[index] == '?')
        buffer[index] = '!';
      else if (buffer[index] == '"')
        buffer[index] = '\'';
      else if (buffer[index] == '<')
        buffer[index] = '(';
      else if (buffer[index] == '>')
        buffer[index] = '>';
      else if (buffer[index] == '|')
        buffer[index] = 'I';
   }

   return (&buffer[0]); // and return the normalized string
}


char *UpperCase (const char *input_string)
{
   // this function converts a string to its uppercase representation

   static char output_string[128];
   register int i, length;

   length = (int) strlen (input_string); // get the string size
   if (length > 127)
      length = 127; // don't exceed 128 chars (including the null terminator)

   // convert each character in the string
   for (i = 0; i < length; i++)
      output_string[i] = (char) toupper ((int) input_string[i]);
   output_string[i] = 0; // terminate the string

   return (&output_string[0]); // and return a pointer to a converted string, in uppercase
}


char *LowerCase (const char *input_string)
{
   // this function converts a string to its lowercase representation

   static char output_string[128];
   register int i, length;

   length = (int) strlen (input_string); // get the string size
   if (length > 127)
      length = 127; // don't exceed 128 chars (including the null terminator)

   // convert each character in the string
   for (i = 0; i < length; i++)
      output_string[i] = (char) tolower ((int) input_string[i]);
   output_string[i] = 0; // terminate the string

   return (&output_string[0]); // and return a pointer to a converted string, in lowercase
}


const char *BotHALGenerateReply (bot_t *pBot)
{
   // this function takes a string of user input and return a string of output which may
   // vaguely be construed as containing a reply to whatever is in the input string.
   // Create an array of keywords from the words in the user's input...

   HAL_DICTIONARY *keywords, *replywords;
   static char output_template[128];
   int tries_count, last_word, last_character, length;
   register int i, j;

   output_template[0] = 0; // first reset the reply string
   length = 0;

   keywords = BotHALMakeKeywords (pBot, pBot->BotBrain.input_words);
   replywords = BotHALBuildReplyDictionary (pBot, keywords);

   last_word = pBot->BotBrain.input_words->size - 1;
   last_character = pBot->BotBrain.input_words->entry[last_word].length - 1;

   // was it a question (i.e. was the last word in the general chat record a question mark ?)
   if (pBot->BotBrain.input_words->entry[last_word].word[last_character] == '?')
   {
      // try ten times to answer something relevant
      for (tries_count = 0; tries_count < 10; tries_count++)
      {
         if (HAL_DictionariesDiffer (pBot->BotBrain.input_words, replywords))
            break; // stop as soon as we've got something to say
         else
            replywords = BotHALBuildReplyDictionary (pBot, keywords); // else think again
      }

      // if we've finally found something to say, generate the reply
      if (tries_count < 10)
      {
         // if no words in the reply dictionary...
         if (replywords->size == 0)
            return (&"mommy mommy!"); // then return a "newborn" answer

         for (i = 0; i < (int) replywords->size; ++i)
            for (j = 0; j < replywords->entry[i].length; ++j)
            {
               output_template[length++] = replywords->entry[i].word[j];
               if (length == 127)
                  break; // break when the bot client's keyboard input buffer is full
            }
      }
   }

   // else if we are not paraphrazing, generate a string from the dictionary of reply words
   else if (HAL_DictionariesDiffer (pBot->BotBrain.input_words, replywords))
   {
      // if no words in the reply dictionary...
      if (replywords->size == 0)
         return (&"mommy mommy!"); // then copy a "newborn" answer

      for (i = 0; i < (int) replywords->size; ++i)
         for (j = 0; j < replywords->entry[i].length; ++j)
         {
            output_template[length++] = replywords->entry[i].word[j];
            if (length == 127)
               break; // break when the bot client's keyboard input buffer is full
         }
   }

   output_template[length] = 0; // terminate the string
   return (&LowerCase (output_template)[0]); // and copy the answer, in lowercase
}


unsigned short HAL_AddWord (HAL_DICTIONARY *dictionary, HAL_STRING word)
{
   // this function adds a word to a dictionary, and return the identifier assigned to the
   // word. If the word already exists in the dictionary, then return its current identifier
   // without adding it again.

   register int i;
   int position;

   // if the word's already in the dictionary, there is no need to add it
   if (HAL_SearchDictionary (dictionary, word, &position))
      return (dictionary->index[position]);

   // increase the number of words in the dictionary
   dictionary->size++;

   // allocate one more entry for the word index
   if (dictionary->index == NULL)
      dictionary->index = (unsigned short *) malloc (sizeof (unsigned short) * dictionary->size);
   else
      dictionary->index = (unsigned short *) realloc ((unsigned short *) dictionary->index, sizeof (unsigned short) * dictionary->size);

   if (dictionary->index == NULL)
      TerminateOnError ("RACC: HAL_AddWord() unable to reallocate the dictionary index\n");

   // allocate one more entry for the word array
   if (dictionary->entry == NULL)
      dictionary->entry = (HAL_STRING *) malloc (sizeof (HAL_STRING) * dictionary->size);
   else
      dictionary->entry = (HAL_STRING *) realloc ((HAL_STRING *) dictionary->entry, sizeof (HAL_STRING) * dictionary->size);

   if (dictionary->entry == NULL)
      TerminateOnError ("RACC: HAL_AddWord() unable to reallocate the dictionary to %d elements\n", dictionary->size);

   // copy the new word into the word array
   dictionary->entry[dictionary->size - 1].length = word.length;
   dictionary->entry[dictionary->size - 1].word = (char *) malloc (sizeof (char) * word.length);
   if (dictionary->entry[dictionary->size - 1].word == NULL)
      TerminateOnError ("RACC: HAL_AddWord() unable to allocate the word\n");

   for (i = 0; i < word.length; ++i)
      dictionary->entry[dictionary->size - 1].word[i] = word.word[i];

   // shuffle the word index to keep it sorted alphabetically
   for (i = dictionary->size - 1; i > position; --i)
      dictionary->index[i] = dictionary->index[i - 1];

   // copy the new symbol identifier into the word index
   dictionary->index[position] = (unsigned short) dictionary->size - 1;

   return (dictionary->index[position]);
}


bool HAL_SearchDictionary (HAL_DICTIONARY *dictionary, HAL_STRING word, int *position)
{
   // search the dictionary for the specified word, returning its position in the index if
   // found, or the position where it should be inserted otherwise

   int min;
   int max;
   int middle;
   int compar;

   // if the dictionary is empty, then obviously the word won't be found
   if (dictionary->size == 0)
   {
      *position = 0;
      return (FALSE);
   }

   // initialize the lower and upper bounds of the search
   min = 0;
   max = dictionary->size - 1;

   // search repeatedly, halving the search space each time, until either the entry is found,
   // or the search space becomes empty
   while (TRUE)
   {
      // see whether the middle element of the search space is greater than, equal to, or
      // less than the element being searched for.
      middle = (min + max) / 2;
      compar = HAL_CompareWords (word, dictionary->entry[dictionary->index[middle]]);

      // if equal then we have found the element. Otherwise halve the search space accordingly
      if (compar == 0)
      {
         *position = middle;
         return (TRUE);
      }
      else if (compar > 0)
      {
         if (max == middle)
         {
            *position = middle + 1;
            return (FALSE);
         }

         min = middle + 1;
      }
      else
      {
         if (min == middle)
         {
            *position = middle;
            return (FALSE);
         }

         max = middle - 1;
      }
   }
}


unsigned short HAL_FindWord (HAL_DICTIONARY *dictionary, HAL_STRING word)
{
   // this function returns the symbol corresponding to the word specified. We assume that
   // the word with index zero is equal to a NULL word, indicating an error condition.

   int position;
   
   if (HAL_SearchDictionary (dictionary, word, &position))
      return (dictionary->index[position]);
   else
      return (0);
}


int HAL_CompareWords (HAL_STRING word1, HAL_STRING word2)
{
   // this function compares two words, and return an integer indicating whether the first
   // word is less than, equal to or greater than the second word

   register int i;
   int bound;

   bound = min (word1.length, word2.length);

   for (i = 0; i < bound; ++i)
      if (toupper (word1.word[i]) != toupper (word2.word[i]))
         return ((int) (toupper (word1.word[i]) - toupper (word2.word[i])));

   if (word1.length < word2.length)
      return (-1);
   else if (word1.length > word2.length)
      return (1);
   else
      return (0);
}


void HAL_InitializeDictionary (HAL_DICTIONARY *dictionary)
{
   // this function adds dummy words to the dictionary

   HAL_STRING word = { 7, "<ERROR>" };
   HAL_STRING end = { 5, "<FIN>" };

   (void) HAL_AddWord (dictionary, word);
   (void) HAL_AddWord (dictionary, end);
}


HAL_DICTIONARY *HAL_NewDictionary (void)
{
   // this function allocates room for a new dictionary

   HAL_DICTIONARY *dictionary = NULL;

   dictionary = (HAL_DICTIONARY *) malloc (sizeof (HAL_DICTIONARY));
   if (dictionary == NULL)
      TerminateOnError ("HAL: HAL_NewDictionary() unable to allocate dictionary\n");

   dictionary->size = 0;
   dictionary->index = NULL;
   dictionary->entry = NULL;

   return (dictionary);
}


void HAL_SaveDictionary (FILE *file, HAL_DICTIONARY *dictionary)
{
   // this function saves a dictionary to the specified file

   register int i, j;

   fwrite (&dictionary->size, sizeof (unsigned long), 1, file);

   // save each word to the file
   for (i = 0; i < (int) dictionary->size; ++i)
   {
      fwrite (&dictionary->entry[i].length, sizeof (unsigned char), 1, file);
      for (j = 0; j < dictionary->entry[i].length; ++j)
         fwrite (&dictionary->entry[i].word[j], sizeof (char), 1, file);
   }
}


void HAL_LoadDictionary (MFILE *file, HAL_DICTIONARY *dictionary)
{
   // this function loads a dictionary from the specified file

   register int i, j;
   int size;
   HAL_STRING word;

   mfread (&size, sizeof (unsigned long), 1, file);

   // load each dictionary word from the file
   for (i = 0; i < size; ++i)
   {
      mfread (&word.length, sizeof (unsigned char), 1, file);

      word.word = (char *) malloc (sizeof (char) * word.length);
      if (word.word == NULL)
         TerminateOnError ("RACC: HAL_LoadDictionary() unable to allocate word\n");

      for (j = 0; j < word.length; ++j)
         mfread (&word.word[j], sizeof (char), 1, file);

      HAL_AddWord (dictionary, word);

      if (word.word != NULL)
         free (word.word);
   }
}


HAL_TREE *HAL_NewNode (void)
{
   // allocate a new node for the n-gram tree, and initialise its contents to sensible values

   HAL_TREE *node = NULL;

   // allocate memory for the new node
   node = (HAL_TREE *) malloc (sizeof (HAL_TREE));
   if (node == NULL)
      TerminateOnError ("HAL: HAL_NewNode() unable to allocate node\n");

   // initialise the contents of the node
   node->symbol = 0;
   node->usage = 0;
   node->count = 0;
   node->branch = 0;
   node->tree = NULL;

   return (node);
}


void HAL_UpdateModel (HAL_MODEL *model, int symbol)
{
   // this function uppdates the model with the specified symbol

   register int i;

   // update all of the models in the current context with the specified symbol
   for (i = model->order + 1; i > 0; --i)
      if (model->context[i - 1] != NULL)
         model->context[i] = HAL_AddSymbol (model->context[i - 1], (unsigned short) symbol);

   return;
}


void HAL_UpdateContext (HAL_MODEL *model, int symbol)
{
   // this function updates the context of the model without adding the symbol

   register int i;

   for (i = model->order + 1; i > 0; --i)
      if (model->context[i - 1] != NULL)
         model->context[i] = HAL_FindSymbol (model->context[i - 1], symbol);
}


HAL_TREE *HAL_AddSymbol (HAL_TREE *tree, unsigned short symbol)
{
   // this function updates the statistics of the specified tree with the specified symbol,
   // which may mean growing the tree if the symbol hasn't been seen in this context before

   HAL_TREE *node = NULL;

   // search for the symbol in the subtree of the tree node
   node = HAL_FindSymbolAdd (tree, symbol);

   // increment the symbol counts
   if (node->count < 65535)
   {
      node->count++;
      tree->usage++;
   }

   return (node);
}


HAL_TREE *HAL_FindSymbol (HAL_TREE *node, int symbol)
{
   // this function returns a pointer to the child node, if one exists, which contains symbol

   register int i;
   HAL_TREE *found = NULL;

   // perform a binary search for the symbol
   if (HAL_SearchNode (node, symbol, &i))
      found = node->tree[i];

   return (found);
}


HAL_TREE *HAL_FindSymbolAdd (HAL_TREE *node, int symbol)
{
   // this function is conceptually similar to HAL_FindSymbol, apart from the fact that if the
   // symbol is not found, a new node is automatically allocated and added to the tree

   register int i;
   HAL_TREE *found = NULL;

   // perform a binary search for the symbol. If the symbol isn't found, attach a new sub-node
   // to the tree node so that it remains sorted.

   if (HAL_SearchNode (node, symbol, &i))
      found = node->tree[i];
   else
   {
      found = HAL_NewNode ();
      found->symbol = (unsigned short) symbol;
      HAL_AddNode (node, found, i);
   }

   return (found);
}


void HAL_AddNode (HAL_TREE *tree, HAL_TREE *node, int position)
{
   // this function attachs a new child node to the sub-tree of the tree specified

   register int i;

   // allocate room for one more child node, which may mean allocating the sub-tree from scratch
   if (tree->tree == NULL)
      tree->tree = (HAL_TREE **) malloc (sizeof (HAL_TREE *) * (tree->branch + 1));
   else
      tree->tree = (HAL_TREE **) realloc ((HAL_TREE **) tree->tree, sizeof (HAL_TREE *) * (tree->branch + 1));

   if (tree->tree == NULL)
      TerminateOnError ("HAL: HAL_AddNode() unable to reallocate subtree\n");

   // shuffle nodes down so that we can insert new node at subtree index given by position
   for (i = tree->branch; i > position; --i)
      tree->tree[i] = tree->tree[i - 1];

   // add the new node to the sub-tree
   tree->tree[position] = node;
   tree->branch++;
}


bool HAL_SearchNode (HAL_TREE *node, int symbol, int *position)
{
   // this function performs a binary search for the specified symbol on the subtree of the
   // given node. Return the position of the child node in the subtree if the symbol was found,
   // or the position where it should be inserted to keep the subtree sorted if it wasn't

   int min;
   int max;
   int middle;
   int compar;

   // handle the special case where the subtree is empty
   if (node->branch == 0)
   {
      *position = 0;
      return (FALSE);
   }

   // perform a binary search on the subtree
   min = 0;
   max = node->branch - 1;

   while (TRUE)
   {
      middle = (min + max) / 2;
      compar = symbol-node->tree[middle]->symbol;

      if (compar == 0)
      {
         *position = middle;
         return (TRUE);
      }
      else if (compar > 0)
      {
         if (max == middle)
         {
            *position = middle + 1;
            return (FALSE);
         }

         min = middle + 1;
      }
      else
      {
         if (min == middle)
         {
            *position = middle;
            return (FALSE);
         }

         max = middle - 1;
      }
   }
}


void HAL_InitializeContext (HAL_MODEL *model)
{
   // this function sets the context of the model to a default value

   register int i;

   for (i = 0; i <= model->order; ++i)
      model->context[i] = NULL; // reset all the context elements
}


void HAL_Learn (HAL_MODEL *model, HAL_DICTIONARY *words)
{
   // this function learns from the user's input

   register int i;
   unsigned short symbol;

   if (words->size <= model->order)
      return; // only learn from inputs which are long enough

   // train the model in the forward direction. Start by initializing the context of the model
   HAL_InitializeContext (model);
   model->context[0] = model->forward;

   for (i = 0; i < (int) words->size; ++i)
   {
      // add the symbol to the model's dictionary if necessary, and update the model accordingly
      symbol = HAL_AddWord (model->dictionary, words->entry[i]);
      HAL_UpdateModel (model, symbol);
   }

   // add the sentence-terminating symbol
   HAL_UpdateModel (model, 1);

   // train the model in the backwards direction. Start by initializing the context of the model
   HAL_InitializeContext (model);
   model->context[0] = model->backward;

   for (i = words->size - 1; i >= 0; --i)
   {
      // find the symbol in the model's dictionary, and update the backward model accordingly
      symbol = HAL_FindWord (model->dictionary, words->entry[i]);
      HAL_UpdateModel (model, symbol);
   }

   // add the sentence-terminating symbol
   HAL_UpdateModel (model, 1);

   return;
}


void HAL_SaveTree (FILE *file, HAL_TREE *node)
{
   // this function saves a tree structure to the specified file

   register int i;

   fwrite (&node->symbol, sizeof (unsigned short), 1, file);
   fwrite (&node->usage, sizeof (unsigned long), 1, file);
   fwrite (&node->count, sizeof (unsigned short), 1, file);
   fwrite (&node->branch, sizeof (unsigned short), 1, file);

   for (i = 0; i < node->branch; ++i)
      HAL_SaveTree (file, node->tree[i]);
}


void HAL_LoadTree (MFILE *file, HAL_TREE *node)
{
   // this function loads a tree structure from the specified file

   register int i;

   mfread (&node->symbol, sizeof (unsigned short), 1, file);
   mfread (&node->usage, sizeof (unsigned long), 1, file);
   mfread (&node->count, sizeof (unsigned short), 1, file);
   mfread (&node->branch, sizeof (unsigned short), 1, file);

   if (node->branch == 0)
      return; // reliability check

   node->tree = (HAL_TREE **) malloc (sizeof (HAL_TREE *) * node->branch);
   if (node->tree == NULL)
      TerminateOnError ("HAL: HAL_LoadTree() unable to allocate subtree\n");

   for (i = 0; i < node->branch; ++i)
   {
      node->tree[i] = HAL_NewNode ();
      HAL_LoadTree (file, node->tree[i]);
   }
}


void HAL_MakeWords (char *input, HAL_DICTIONARY *words)
{
   // this function breaks a string into an array of words

   int offset = 0;

   // clear the entries in the dictionary
   HAL_EmptyDictionary (words);

   if (strlen (input) == 0)
      return; // if void, return

   // loop forever
   while (TRUE)
   {
      // if the current character is of the same type as the previous character, then include
      // it in the word. Otherwise, terminate the current word.
      if (HAL_BoundaryExists (input, offset))
      {
         // add the word to the dictionary
         if (words->entry == NULL)
            words->entry = (HAL_STRING *) malloc ((words->size + 1) * sizeof (HAL_STRING));
         else
            words->entry = (HAL_STRING *) realloc (words->entry,  (words->size + 1) * sizeof (HAL_STRING));

         if (words->entry == NULL)
            TerminateOnError ("RACC: HAL_MakeWords() unable to reallocate dictionary\n");

         words->entry[words->size].length = (unsigned char) offset;
         words->entry[words->size].word = input;
         words->size += 1;

         if (offset == (int) strlen (input))
            break;

         input += offset;
         offset = 0;
      }
      else
         offset++;
   }

   return; // finished, no need to add punctuation (it's an ACTION game, woohoo!)
}
 
 
bool HAL_BoundaryExists (char *string, int position)
{
   // this function returns whether or not a word boundary exists in a string at the
   // specified location

   if (position == 0)
      return (FALSE);

   if (position == (int) strlen (string))
      return (TRUE);

   if ((string[position] == '\'') && (isalpha (string[position - 1]) != 0)
       && (isalpha (string[position + 1]) != 0))
      return (FALSE);

   if ((position > 1) && (string[position - 1] == '\'')
       && (isalpha (string[position - 2]) != 0) && (isalpha (string[position]) != 0))
      return (FALSE);

   if ((isalpha (string[position]) != 0) && (isalpha (string[position-1]) == 0))
      return (TRUE);
   
   if ((isalpha (string[position]) == 0) && (isalpha (string[position - 1]) != 0))
      return (TRUE);
   
   if (isdigit (string[position]) != isdigit (string[position - 1]))
      return (TRUE);

   return (FALSE);
}
 
 
bool HAL_DictionariesDiffer (HAL_DICTIONARY *words1, HAL_DICTIONARY *words2)
{
   // this function returns TRUE if the dictionaries are NOT the same or FALSE if not

   register int i;

   if (words1->size != words2->size)
      return (TRUE); // if they haven't the same size, obviously they aren't the same

   // for each word of the first dictionary...
   for (i = 0; i < (int) words1->size; ++i)
      if (HAL_CompareWords (words1->entry[i], words2->entry[i]) != 0)
         return (TRUE); // compare it with the second and break at the first difference

   return (FALSE); // looks like those dictionaries are identical
}


HAL_DICTIONARY *BotHALMakeKeywords (bot_t *pBot, HAL_DICTIONARY *words)
{
   // this function puts all the interesting words from the user's input into a keywords
   // dictionary, which will be used when generating a reply

   register int i;
   register int j;
   int c;

   if (pBot->BotBrain.keys->entry != NULL)
      for (i = 0; i < (int) pBot->BotBrain.keys->size; ++i)
         if (pBot->BotBrain.keys->entry[i].word != NULL)
            free (pBot->BotBrain.keys->entry[i].word);

   HAL_EmptyDictionary (pBot->BotBrain.keys);

   for (i = 0; i < (int) words->size; ++i)
   {
      // find the symbol ID of the word. If it doesn't exist in the model, or if it begins
      // with a non-alphanumeric character, or if it is in the exclusion array, then skip it

      c = 0;

      for (j = 0; j < pBot->BotBrain.swappable_keywords->size; ++j)
         if (HAL_CompareWords (pBot->BotBrain.swappable_keywords->from[j], words->entry[i]) == 0)
         {
            BotHALAddKeyword (pBot, pBot->BotBrain.keys, pBot->BotBrain.swappable_keywords->to[j]);
            c++;
         }

      if (c == 0)
         BotHALAddKeyword (pBot, pBot->BotBrain.keys, words->entry[i]);
   }

   if (pBot->BotBrain.keys->size > 0)
      for (i = 0; i < (int) words->size; ++i)
      {
         c = 0;

         for (j = 0; j < pBot->BotBrain.swappable_keywords->size; ++j)
            if (HAL_CompareWords (pBot->BotBrain.swappable_keywords->from[j], words->entry[i]) == 0)
            {
               BotHALAddAuxiliaryKeyword (pBot, pBot->BotBrain.keys, pBot->BotBrain.swappable_keywords->to[j]);
               c++;
            }

         if (c == 0)
            BotHALAddAuxiliaryKeyword (pBot, pBot->BotBrain.keys, words->entry[i]);
      }

   return (pBot->BotBrain.keys);
}


void BotHALAddKeyword (bot_t *pBot, HAL_DICTIONARY *keys, HAL_STRING word)
{
   // this function adds a word to the keyword dictionary

   int symbol;
   
   if (!IsValidPlayer (pBot->pEdict))
      return; // reliability check

   symbol = HAL_FindWord (pBot->BotBrain.HAL_model.dictionary, word);

   if (symbol == 0)
      return; // if void, return

   if (isalnum (word.word[0]) == 0)
      return; // if not alphanumeric, return

   symbol = HAL_FindWord (pBot->BotBrain.banned_keywords, word); // is this word in the banned dictionary ?
   if (symbol != 0)
      return; // if so, return

   symbol = HAL_FindWord (pBot->BotBrain.auxiliary_keywords, word); // is this word in the auxiliary dictionary ?
   if (symbol != 0)
      return; // if so, return

   HAL_AddWord (keys, word); // once we are sure this word isn't known yet, we can add it
}


void BotHALAddAuxiliaryKeyword (bot_t *pBot, HAL_DICTIONARY *keys, HAL_STRING word)
{
   // this function adds an auxilliary keyword to the keyword dictionary

   int symbol;

   if (!IsValidPlayer (pBot->pEdict))
      return; // reliability check

   symbol = HAL_FindWord (pBot->BotBrain.HAL_model.dictionary, word);

   if (symbol == 0)
      return; // if void, return

   if (isalnum (word.word[0]) == 0)
      return; // if not alphanumeric, return

   symbol = HAL_FindWord (pBot->BotBrain.auxiliary_keywords, word); // is it already in the dictionary ?
   if (symbol == 0)
      return; // if already in dictionary, return

   HAL_AddWord (keys, word); // add this word to the keywords dictionary
}


HAL_DICTIONARY *BotHALBuildReplyDictionary (bot_t *pBot, HAL_DICTIONARY *keys)
{
   // this function generates a dictionary of reply words relevant to the dictionary of keywords

   register int i;
   int symbol;
   bool start = TRUE;

   HAL_EmptyDictionary (pBot->BotBrain.replies);

   // start off by making sure that the model's context is empty
   HAL_InitializeContext (&pBot->BotBrain.HAL_model);
   pBot->BotBrain.HAL_model.context[0] = pBot->BotBrain.HAL_model.forward;
   pBot->BotBrain.keyword_is_used = FALSE;

   // generate the reply in the forward direction
   while (TRUE)
   {
      // get a random symbol from the current context
      if (start == TRUE)
         symbol = BotHALSeedReply (pBot, keys);
      else
         symbol = BotHALBabble (pBot, keys, pBot->BotBrain.replies);

      if ((symbol == 0) || (symbol == 1))
         break;

      start = FALSE;

      // append the symbol to the reply dictionary
      if (pBot->BotBrain.replies->entry == NULL)
         pBot->BotBrain.replies->entry = (HAL_STRING *) malloc ((pBot->BotBrain.replies->size + 1) * sizeof (HAL_STRING));
      else
         pBot->BotBrain.replies->entry = (HAL_STRING *) realloc (pBot->BotBrain.replies->entry, (pBot->BotBrain.replies->size + 1) * sizeof (HAL_STRING));

      if (pBot->BotBrain.replies->entry == NULL)
         TerminateOnError ("HAL: BotHALBuildReplyDictionary() unable to reallocate dictionary\n");

      pBot->BotBrain.replies->entry[pBot->BotBrain.replies->size].length = pBot->BotBrain.HAL_model.dictionary->entry[symbol].length;
      pBot->BotBrain.replies->entry[pBot->BotBrain.replies->size].word = pBot->BotBrain.HAL_model.dictionary->entry[symbol].word;
      pBot->BotBrain.replies->size++;

      // extend the current context of the model with the current symbol
      HAL_UpdateContext (&pBot->BotBrain.HAL_model, symbol);
   }

   // start off by making sure that the model's context is empty
   HAL_InitializeContext (&pBot->BotBrain.HAL_model);
   pBot->BotBrain.HAL_model.context[0] = pBot->BotBrain.HAL_model.backward;

   // re-create the context of the model from the current reply dictionary so that we can
   // generate backwards to reach the beginning of the string.
   if (pBot->BotBrain.replies->size > 0)
      for (i = min (pBot->BotBrain.replies->size - 1, pBot->BotBrain.HAL_model.order); i >= 0; --i)
      {
         symbol = HAL_FindWord (pBot->BotBrain.HAL_model.dictionary, pBot->BotBrain.replies->entry[i]);
         HAL_UpdateContext (&pBot->BotBrain.HAL_model, symbol);
      }

   // generate the reply in the backward direction
   while (TRUE)
   {
      // get a random symbol from the current context
      symbol = BotHALBabble (pBot, keys, pBot->BotBrain.replies);
      if ((symbol == 0) || (symbol == 1))
         break;

      // prepend the symbol to the reply dictionary
      if (pBot->BotBrain.replies->entry == NULL)
         pBot->BotBrain.replies->entry = (HAL_STRING *) malloc ((pBot->BotBrain.replies->size + 1) * sizeof (HAL_STRING));
      else
         pBot->BotBrain.replies->entry = (HAL_STRING *) realloc (pBot->BotBrain.replies->entry, (pBot->BotBrain.replies->size + 1) * sizeof (HAL_STRING));

      if (pBot->BotBrain.replies->entry == NULL)
         TerminateOnError ("HAL: BotHALBuildReplyDictionary() unable to reallocate dictionary\n");

      // shuffle everything up for the prepend
      for (i = pBot->BotBrain.replies->size; i > 0; --i)
      {
         pBot->BotBrain.replies->entry[i].length = pBot->BotBrain.replies->entry[i - 1].length;
         pBot->BotBrain.replies->entry[i].word = pBot->BotBrain.replies->entry[i - 1].word;
      }

      pBot->BotBrain.replies->entry[0].length = pBot->BotBrain.HAL_model.dictionary->entry[symbol].length;
      pBot->BotBrain.replies->entry[0].word = pBot->BotBrain.HAL_model.dictionary->entry[symbol].word;
      pBot->BotBrain.replies->size++;

      // extend the current context of the model with the current symbol
      HAL_UpdateContext (&pBot->BotBrain.HAL_model, symbol);
   }

   return (pBot->BotBrain.replies);
}


int BotHALBabble (bot_t *pBot, HAL_DICTIONARY *keys, HAL_DICTIONARY *words)
{
   // this function returns a random symbol from the current context, or a zero symbol
   // identifier if we've reached either the start or end of the sentence. Selection of the
   // symbol is based on probabilities, favouring keywords. In all cases, use the longest
   // available context to choose the symbol

   HAL_TREE *node = NULL;
   register int i;
   int count;
   int symbol = 0;

   // select the longest available context
   for (i = 0; i <= pBot->BotBrain.HAL_model.order; ++i)
      if (pBot->BotBrain.HAL_model.context[i] != NULL)
         node = pBot->BotBrain.HAL_model.context[i];

   if (node->branch == 0)
      return (0);

   // choose a symbol at random from this context
   i = RANDOM_LONG (0, node->branch - 1);
   count = RANDOM_LONG (0, node->usage - 1);

   while (count >= 0)
   {
      // if the symbol occurs as a keyword, then use it. Only use an auxilliary keyword if
      // a normal keyword has already been used.
      symbol = node->tree[i]->symbol;

      if ((HAL_FindWord (keys, pBot->BotBrain.HAL_model.dictionary->entry[symbol]) != 0)
          && (pBot->BotBrain.keyword_is_used || (HAL_FindWord (pBot->BotBrain.auxiliary_keywords, pBot->BotBrain.HAL_model.dictionary->entry[symbol]) == 0))
          && !HAL_WordExists (words, pBot->BotBrain.HAL_model.dictionary->entry[symbol]))
      {
         pBot->BotBrain.keyword_is_used = TRUE;
         break;
      }

      count -= node->tree[i]->count;

      i = (i >= node->branch - 1)?0:i + 1;
   }

   return (symbol);
}


bool HAL_WordExists (HAL_DICTIONARY *dictionary, HAL_STRING word)
{
   // here's a silly brute-force searcher for the reply string
   
   register int i;

   // for each element of the dictionary, compare word with it...
   for (i = 0; i < (int) dictionary->size; ++i)
      if (HAL_CompareWords (dictionary->entry[i], word) == 0)
         return (TRUE); // word was found

   return (FALSE); // word was not found
}


int BotHALSeedReply (bot_t *pBot, HAL_DICTIONARY *keys)
{
   // this function seeds the reply by guaranteeing that it contains a keyword, if one exists

   register int i;
   int symbol;
   int stop;

   // be aware of the special case where the tree is empty
   if (pBot->BotBrain.HAL_model.context[0]->branch == 0)
      symbol = 0;
   else
      symbol = pBot->BotBrain.HAL_model.context[0]->tree[RANDOM_LONG (0, pBot->BotBrain.HAL_model.context[0]->branch - 1)]->symbol;

   if (keys->size > 0)
   {
      i = RANDOM_LONG (0, keys->size - 1);
      stop = i;

      while (TRUE)
      {
         if ((HAL_FindWord (pBot->BotBrain.HAL_model.dictionary, keys->entry[i]) != 0) && (HAL_FindWord (pBot->BotBrain.auxiliary_keywords, keys->entry[i]) == 0))
         {
            symbol = HAL_FindWord (pBot->BotBrain.HAL_model.dictionary, keys->entry[i]);
            return (symbol);
         }

         i++;

         if (i == (int) keys->size)
            i = 0;

         if (i == stop)
            return (symbol);
      }
   }

   return (symbol);
}


HAL_SWAP *HAL_NewSwap (void)
{
   // allocate a new swap structure.

   HAL_SWAP *list = (HAL_SWAP *) malloc (sizeof (HAL_SWAP));
   if (list == NULL)
      TerminateOnError ("HAL: HAL_NewSwap() unable to allocate swap\n");

   list->size = 0; // initialize to defaults
   list->from = NULL;
   list->to = NULL;

   return (list); // return the fresh new swap
}


void HAL_AddSwap (HAL_SWAP *list, char *s, char *d)
{
   // this function adds a new entry to the swap structure.

   if (list->from == NULL)
   {
      list->from = (HAL_STRING *) malloc (sizeof (HAL_STRING));
      if (list->from == NULL)
         TerminateOnError ("HAL: HAL_AddSwap() unable to allocate list->from\n");
   }

   if (list->to == NULL)
   {
      list->to = (HAL_STRING *) malloc (sizeof (HAL_STRING));
      if (list->to == NULL)
         TerminateOnError ("HAL: HAL_AddSwap() unable to allocate list->to\n");
   }

   list->from = (HAL_STRING *) realloc (list->from, sizeof (HAL_STRING) * (list->size + 1));
   if (list->from == NULL)
      TerminateOnError ("HAL: HAL_AddSwap() unable to reallocate from\n");

   list->to = (HAL_STRING *) realloc (list->to, sizeof (HAL_STRING) * (list->size + 1));
   if (list->to == NULL)
      TerminateOnError ("HAL: HAL_AddSwap() unable to reallocate to\n");

   list->from[list->size].length = (unsigned char) strlen (s);
   list->from[list->size].word = strdup (s);
   list->to[list->size].length = (unsigned char) strlen (d);
   list->to[list->size].word = strdup (d);
   list->size++;
}


HAL_SWAP *HAL_InitializeSwap (char *filename)
{
   // this function reads a swap structure from a file.

   HAL_SWAP *list;
   MFILE *fp;
   char buffer[1024];
   char *from;
   char *to;

   list = HAL_NewSwap ();

   if (filename == NULL)
      return (list);

   fp = mfopen (filename, "r");
   if (fp == NULL)
      return (list);

   while (!mfeof (fp))
   {
      if (mfgets (buffer, 1024, fp) == NULL)
         break;

      if ((buffer[0] == '#') || (buffer[0] == '\n'))
         continue; // skip void or sharp-prepended lines

      from = strtok (buffer, "\t");
      to = strtok (NULL, "\t\n#");

      HAL_AddSwap (list, from, to);
   }

   mfclose (fp);
   return (list);
}


HAL_DICTIONARY *HAL_InitializeList (char *filename)
{
   // this function reads a dictionary from a file

   HAL_DICTIONARY *list;
   MFILE *fp;
   HAL_STRING word;
   char *string;
   char buffer[1024];

   list = HAL_NewDictionary ();

   if (filename == NULL)
      return (list);

   fp = mfopen (filename, "r");
   if (fp == NULL)
      return (list);

   while (!mfeof (fp))
   {
      if (mfgets (buffer, 1024, fp) == NULL)
         break;

      if ((buffer[0] == '#') || (buffer[0] == '\n'))
         continue;

      string = strtok (buffer, "\t\n#");

      if ((string != NULL) && (strlen (string) > 0))
      {
         word.length = (unsigned char) strlen (string);
         word.word = strdup (buffer);
         HAL_AddWord (list, word);
      }
   }

   mfclose (fp);
   return (list);
}


void HAL_EmptyDictionary (HAL_DICTIONARY *dictionary)
{
   // this function empties the memory space used by a dictionary, cutting it down to zero.
   // NOTE THAT IT DOES NOT FREE THE DICTIONARY MEMORY SPACE

   if (dictionary == NULL)
      return; // reliability check

   if (dictionary->entry != NULL)
   {
      free (dictionary->entry);
      dictionary->entry = NULL;
   }

   if (dictionary->index != NULL)
   {
      free (dictionary->index);
      dictionary->index = NULL;
   }

   dictionary->size = 0;
}


void HAL_FreeDictionary (HAL_DICTIONARY *dictionary)
{
   // this function frees the memory space used by a dictionary

   if (dictionary == NULL)
      return; // reliability check

   HAL_EmptyDictionary (dictionary);

   if (dictionary != NULL)
      free (dictionary);
   dictionary = NULL;

   return; // finished
}


void HAL_EmptyModel (HAL_MODEL *model)
{
   // this function empties the memory space used by a model
   // NOTE THAT IT DOES NOT FREE THE MODEL MEMORY SPACE

   if (model == NULL)
      return; // reliability check

   if (model->forward != NULL)
      HAL_FreeTree (model->forward);
   if (model->backward != NULL)
      HAL_FreeTree (model->backward);
   if (model->context != NULL)
      free (model->context);
   HAL_FreeDictionary (model->dictionary);

   return; // finished
}


void HAL_FreeModel (HAL_MODEL *model)
{
   // this function frees the memory space used by a model

   if (model == NULL)
      return; // reliability check

   if (model->forward != NULL)
      HAL_FreeTree (model->forward);
   if (model->backward != NULL)
      HAL_FreeTree (model->backward);
   if (model->context != NULL)
      free (model->context);
   HAL_FreeDictionary (model->dictionary);

   if (model != NULL)
      free (model);

   return; // finished
}


void HAL_FreeTree (HAL_TREE *tree)
{
   // this function frees the memory space used by a model tree

   register int i;

   if (tree == NULL)
      return; // reliability check

   if (tree->tree != NULL)
   {
      for (i = 0; i < tree->branch; ++i)
         HAL_FreeTree (tree->tree[i]);
      free (tree->tree);
   }

   if (tree != NULL)
      free (tree);

   return; // finished
}


void HAL_FreeSwap (HAL_SWAP *swap)
{
   // this function frees the memory space used in a swap structure

   register int i;

   if (swap == NULL)
      return; // reliability check

   // for each element of the swap structure...
   if (swap->from != NULL)
   {
      for (i = 0; i < swap->size; ++i)
         if (swap->from[i].word != NULL)
            free (swap->from[i].word); // free the "from" word
      free (swap->from); // free the "from" array
   }

   if (swap->to != NULL)
   {
      for (i = 0; i < swap->size; ++i)
         if (swap->to[i].word != NULL)
            free (swap->to[i].word); // free the "to" word
      free (swap->to); // free the "to" array
   }

   if (swap != NULL)
      free (swap); // free the swap structure itself
   swap = NULL;

   return; // finished
}


void BotHALLoadBrain (bot_t *pBot)
{
   // this function prepares and load a HAL brain for the specified bot

   MFILE *fp;
   FILE *fp2;
   char ban_filename[256];
   char aux_filename[256];
   char swp_filename[256];
   char brn_filename[256];
   char cookie[32];
   bool valid_brain = FALSE;

   if (FNullEnt (pBot->pEdict))
      return; // reliability check

   // build the file names
   sprintf (ban_filename, "racc/knowledge/%s/keywords.ban", server.mod_name);
   sprintf (aux_filename, "racc/knowledge/%s/keywords.aux", server.mod_name);
   sprintf (swp_filename, "racc/knowledge/%s/keywords.swp", server.mod_name);
   sprintf (brn_filename, "racc/knowledge/%s/%s.brn", server.mod_name, NormalizeChars (STRING (pBot->pEdict->v.netname)));

   // first make sure the brain space is empty
   HAL_FreeDictionary (pBot->BotBrain.banned_keywords);
   HAL_FreeDictionary (pBot->BotBrain.auxiliary_keywords);
   HAL_FreeSwap (pBot->BotBrain.swappable_keywords);
   HAL_EmptyModel (&pBot->BotBrain.HAL_model);
   HAL_FreeDictionary (pBot->BotBrain.input_words);
   HAL_FreeDictionary (pBot->BotBrain.bot_words);
   HAL_FreeDictionary (pBot->BotBrain.keys);
   HAL_FreeDictionary (pBot->BotBrain.replies);
   pBot->BotBrain.keyword_is_used = FALSE;

   // initialize a new model
   pBot->BotBrain.HAL_model.order = (unsigned char) BOT_HAL_MODEL_ORDER; // create a n-gram language model
   pBot->BotBrain.HAL_model.forward = HAL_NewNode ();
   pBot->BotBrain.HAL_model.backward = HAL_NewNode ();
   pBot->BotBrain.HAL_model.context = (HAL_TREE **) malloc ((BOT_HAL_MODEL_ORDER + 2) * sizeof (HAL_TREE *));
   if (pBot->BotBrain.HAL_model.context == NULL)
      TerminateOnError ("RACC: BotHALLoadBrain() unable to allocate context array\n");

   // initialize the new model's context and dictionary
   HAL_InitializeContext (&pBot->BotBrain.HAL_model);
   pBot->BotBrain.HAL_model.dictionary = HAL_NewDictionary ();
   HAL_InitializeDictionary (pBot->BotBrain.HAL_model.dictionary);

   // read dictionaries containing banned keywords, auxiliary keywords and swap keywords
   pBot->BotBrain.banned_keywords = HAL_InitializeList (ban_filename);
   pBot->BotBrain.auxiliary_keywords = HAL_InitializeList (aux_filename);
   pBot->BotBrain.swappable_keywords = HAL_InitializeSwap (swp_filename);

   // first prepare the brain ; i.e. check if the file exists, try to open it
   fp = mfopen (brn_filename, "rb");
   if (fp != NULL)
   {
      mfseek (fp, 0, SEEK_SET); // seek at start of file
      mfread (cookie, sizeof ("RACCHAL"), 1, fp); // read the brain signature
      mfclose (fp); // close the brain (we just wanted the signature)

      // check whether the brain file signature is valid
      if (strcmp (cookie, "RACCHAL") == 0)
         valid_brain = TRUE; // this brain file has the right signature
   }

   // if that brain was NOT explicitly reported as valid, we must fix it
   if (!valid_brain)
   {
      // there is a problem with the brain, infer a brand new one
      ServerConsole_printf ("RACC: bot %s's HAL brain damaged!\n", STRING (pBot->pEdict->v.netname));

      if (server.developer_level > 1)
         ServerConsole_printf ("RACC: inferring a new HAL brain to %s\n", STRING (pBot->pEdict->v.netname));

      // create the new brain (i.e, save a void one in the brain file)
      fp2 = fopen (brn_filename, "wb");
      if (fp2 == NULL)
         TerminateOnError ("BotLoadHALBrain(): %s's HAL brain refuses surgery!\n", STRING (pBot->pEdict->v.netname));

      fwrite ("RACCHAL", sizeof ("RACCHAL"), 1, fp2);
      fwrite (&pBot->BotBrain.HAL_model.order, sizeof (unsigned char), 1, fp2);
      HAL_SaveTree (fp2, pBot->BotBrain.HAL_model.forward);
      HAL_SaveTree (fp2, pBot->BotBrain.HAL_model.backward);
      HAL_SaveDictionary (fp2, pBot->BotBrain.HAL_model.dictionary);
      fclose (fp2); // everything is saved, close the file
   }

   if (server.developer_level > 1)
      ServerConsole_printf ("RACC: restoring HAL brain to %s\n", STRING (pBot->pEdict->v.netname));

   // now that we ensured about its validity, we can safely load the brain
   fp = mfopen (brn_filename, "rb"); // open the brain file again
   mfseek (fp, 0, SEEK_SET); // seek at start of file
   mfread (cookie, sizeof ("RACCHAL"), 1, fp); // read the brain signature
   mfread (&pBot->BotBrain.HAL_model.order, 1, 1, fp); // load the model order
   HAL_LoadTree (fp, pBot->BotBrain.HAL_model.forward); // load the forward tree
   HAL_LoadTree (fp, pBot->BotBrain.HAL_model.backward); // load the backwards tree
   HAL_LoadDictionary (fp, pBot->BotBrain.HAL_model.dictionary); // load the dictionary
   mfclose (fp);

   pBot->BotBrain.input_words = HAL_NewDictionary (); // create the global chat dictionary
   pBot->BotBrain.keys = HAL_NewDictionary (); // create the temporary keywords dictionary
   pBot->BotBrain.replies = HAL_NewDictionary (); // create the temporary replies dictionary

   return; // finished
}


void BotHALSaveBrain (bot_t *pBot)
{
   // this function saves the current state to a HAL brain file

   FILE *fp;
   char filename[256];

   if (FNullEnt (pBot->pEdict))
      return; // reliability check

   // build the file names
   sprintf (filename, "racc/knowledge/%s/%s.brn", server.mod_name, NormalizeChars (STRING (pBot->pEdict->v.netname)));

   fp = fopen (filename, "wb");
   if (fp == NULL)
   {
      ServerConsole_printf ("RACC: Unable to save %s's HAL brain to %s\n", STRING (pBot->pEdict->v.netname), filename);
      return;
   }

   // dump the HAL brain to disk
   fwrite ("RACCHAL", sizeof ("RACCHAL"), 1, fp);
   fwrite (&pBot->BotBrain.HAL_model.order, sizeof (unsigned char), 1, fp);
   HAL_SaveTree (fp, pBot->BotBrain.HAL_model.forward);
   HAL_SaveTree (fp, pBot->BotBrain.HAL_model.backward);
   HAL_SaveDictionary (fp, pBot->BotBrain.HAL_model.dictionary);

   fclose (fp); // finished, close the file
   return; // and return
}
