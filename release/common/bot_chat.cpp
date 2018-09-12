// RACC - AI development project for first-person shooter games
// (http://racc.bots-united.com/)
//
// Rational Autonomous Cybernetic Commandos AI
//
// bot_chat.cpp
//

#include "racc.h"


void BotChat (player_t *pPlayer)
{
   // the purpose of this function is to make the bot think what needs to be thought and do
   // what needs to be done (typing on the keyboard and such) for chatting and emitting messages.

   bot_chat_t *pBotChat;

   pBotChat = &pPlayer->Bot.BotChat; // quick access to bot chat

   if (DebugLevel.chat_disabled)
      return; // return if we don't want the AI to chat

   // is bot chat forbidden ?
   if (server.bot_chat_mode == BOT_CHAT_NONE)
      return; // return; bots are not allowed to chat

   // has the bot finished typing some text ?
   if ((pBotChat->saytext_time > 0) && (pBotChat->saytext_time < server.time))
   {
      FakeClientCommand (pPlayer->pEntity, pBotChat->saytext_message); // let bot send the chat string
      pBotChat->bot_saytext = 0; // clear out what the bot wanted to say
      pBotChat->saytext_time = 0; // don't make the bot spam (once said is enough)
      pBotChat->saytext_message[0] = 0; // finished, cleanup message
   }

   // has the bot finished fiddling with its microphone ?
   if ((pBotChat->sayaudio_time > 0) && (pBotChat->sayaudio_time < server.time))
   {
      BotStartTalking (pPlayer); // let bot talk on the radio
      pBotChat->bot_sayaudio = 0; // clear out what the bot wanted to say
      pBotChat->sayaudio_time = 0; // don't keep it talking over and over again
      pBotChat->sayaudio_message[0] = 0; // finished, cleanup message
   }

   // has the bot finished talking into its microphone ?
   if ((pBotChat->speaker_time > 0) && (pBotChat->speaker_time < server.time))
   {
      BotStopTalking (pPlayer); // let bot stop talking on the radio
      pBotChat->speaker_time = 0; // reset the speaking timer
   }

   // in any case, have the bot decide what to say
   BotSayHAL (pPlayer);
   BotSayText (pPlayer);
   BotSayAudio (pPlayer);

   return; // finished chatting
}


void BotSayHAL (player_t *pPlayer)
{
   // the purpose of this function is to make the bot keep an eye on what's happening in the
   // chat room, and in case of new messages, think about a possible reply.

   bot_chat_t *pBotChat;

   pBotChat = &pPlayer->Bot.BotChat; // quick access to bot chat

   // is text chat forbidden ?
   if (server.bot_chat_mode == BOT_CHAT_AUDIOONLY)
      return; // return; bots are not allowed to spam

   if (!pPlayer->Bot.BotEyes.BotHUD.chat.new_message)
      return; // return if nothing new in the chat area of the bot's HUD

   // is this message not the bot's ?
   if (&players[pPlayer->Bot.BotEyes.BotHUD.chat.sender_index] != pPlayer)
   {
      // break the new message into an array of words
      HAL_MakeWords (pPlayer->Bot.BotEyes.BotHUD.chat.text, pPlayer->Bot.BotBrain.input_words);

      // is the sender of the message NOT a bot ?
      if (!players[pPlayer->Bot.BotEyes.BotHUD.chat.sender_index].is_racc_bot)
         HAL_Learn (&pPlayer->Bot.BotBrain.HAL_model, pPlayer->Bot.BotBrain.input_words); // only learn from humans

      // does the bot feel concerned ? (more chances if its name appears)
      if ((RandomLong (1, 100) < 20 - (player_count / 2))
          || (strstr (pPlayer->Bot.BotEyes.BotHUD.chat.text, UpperCase (StripTags (players[ENTINDEX (pPlayer->pEntity) - 1].connection_name))) != NULL))
      {
         // generate and humanize a HAL reply
         sprintf (pBotChat->saytext_message, "say %s\n", HumanizeChat (BotHALGenerateReply (pPlayer)));

         // set the delay necessary for the bot to type in the reply
         pBotChat->saytext_time = server.time + (float) (strlen (pBotChat->saytext_message) * pPlayer->Bot.pProfile->skill) / 10;
      }
   }

   pPlayer->Bot.BotEyes.BotHUD.chat.new_message = FALSE; // OK, we've seen that message
   return;
}


void BotSayText (player_t *pPlayer)
{
   // the purpose of this function is to make the bot send a chat text that depends of one
   // of the 12 predefined situations : affirmative, negative, greetings, taunting, whining,
   // being idle, about following someone, stopping following someone, staying in position,
   // asking for backup, losing his master and saying goodbye before leaving.

   bot_chat_t *pBotChat;
   bot_language_t *bot_language;
   int bot_index;
   int index, i, recent_count;
   bool is_used;
   char msg[128];

   pBotChat = &pPlayer->Bot.BotChat; // quick access to bot chat

   // is text chat forbidden ?
   if (server.bot_chat_mode == BOT_CHAT_AUDIOONLY)
      return; // return; bots are not allowed to chat

   // is the bot already typing some text ?
   if (pBotChat->saytext_time > server.time)
      return; // return; this bot is already about to say something

   bot_index = ENTINDEX (pPlayer->pEntity) - 1; // get bot index

   // find the bot language
   bot_language = NULL;
   for (index = 0; index < language_count; index++)
      if (strcmp (languages[index].language, pPlayer->Bot.pProfile->nationality) == 0)
         bot_language = &languages[index]; // language name matches with bot nationality

   // have we NOT found one ?
   if (bot_language == NULL)
   {
      // oh well, try again, but with the default language this time
      for (index = 0; index < language_count; index++)
         if (strcmp (languages[index].language, GameConfig.language) == 0)
            bot_language = &languages[index]; // language name matches with default language

      // have we STILL NOT found one ?
      if (bot_language == NULL)
      {
         ServerConsole_printf ("RACC: ALERT: Bot '%s' speaks an unknown language (\"%s\") !\n", pPlayer->connection_name, pPlayer->Bot.pProfile->nationality);
         return; // this bot can't possibly talk
      }
   }

   // does the bot want to acknowledge ?
   if (pBotChat->bot_saytext == BOT_SAYTEXT_AFFIRMATIVE)
   {
      if (FNullEnt (pPlayer->Bot.BotEars.pAskingEntity))
      {
         pBotChat->bot_saytext = 0;
         return; // reliability check
      }

      // audio chat
      pBotChat->bot_sayaudio = BOT_SAYAUDIO_AFFIRMATIVE;

      // text chat
      recent_count = 0;

      while (recent_count < 10)
      {
         index = RandomLong (0, bot_language->text.affirmative_count - 1);
         is_used = FALSE;

         for (i = 0; i < 10; i++)
            if (bot_language->text.recent_affirmative[i] == index)
               is_used = TRUE;

         if (is_used)
            recent_count++;
         else
            break;
      }   

      // cycle down the recently used phrases
      for (i = 9; i > 0; i--)
         bot_language->text.recent_affirmative[i] = bot_language->text.recent_affirmative[i - 1];
      bot_language->text.recent_affirmative[0] = index;

      // is there a "%s" (replacement variable) in this phrase ?
      if (strstr (bot_language->text.affirmative[index], "%s") != NULL)
         sprintf (msg, bot_language->text.affirmative[index], Name (players[ENTINDEX (pPlayer->Bot.BotEars.pAskingEntity) - 1].connection_name));
      else
         strcpy (msg, bot_language->text.affirmative[index]);

      sprintf (pBotChat->saytext_message, "say_team %s\n", HumanizeChat (msg)); // humanize the chat string

      // set the delay necessary for the bot to type in the reply
      pBotChat->saytext_time = server.time + (float) (strlen (pBotChat->saytext_message) * pPlayer->Bot.pProfile->skill) / 10;
      return;
   }

   // else does the bot want to disagree ?
   else if (pBotChat->bot_saytext == BOT_SAYTEXT_NEGATIVE)
   {
      if (FNullEnt (pPlayer->Bot.BotEars.pAskingEntity))
      {
         pBotChat->bot_saytext = 0;
         return; // reliability check
      }

      // audio chat
      pBotChat->bot_sayaudio = BOT_SAYAUDIO_NEGATIVE;

      // text chat
      recent_count = 0;

      while (recent_count < 10)
      {
         index = RandomLong (0, bot_language->text.negative_count - 1);
         is_used = FALSE;

         for (i = 0; i < 10; i++)
            if (bot_language->text.recent_negative[i] == index)
               is_used = TRUE;

         if (is_used)
            recent_count++;
         else
            break;
      }   

      // cycle down the recently used phrases
      for (i = 9; i > 0; i--)
         bot_language->text.recent_negative[i] = bot_language->text.recent_negative[i - 1];
      bot_language->text.recent_negative[0] = index;

      // is there a "%s" (replacement variable) in this phrase ?
      if (strstr (bot_language->text.negative[index], "%s") != NULL)
         sprintf (msg, bot_language->text.negative[index], Name (players[ENTINDEX (pPlayer->Bot.BotEars.pAskingEntity) - 1].connection_name));
      else
         strcpy (msg, bot_language->text.negative[index]);

      sprintf (pBotChat->saytext_message, "say_team %s\n", HumanizeChat (msg)); // humanize the chat string

      // set the delay necessary for the bot to type in the reply
      pBotChat->saytext_time = server.time + (float) (strlen (pBotChat->saytext_message) * pPlayer->Bot.pProfile->skill) / 10;
      return;
   }

   // else does the bot want to say hello ?
   else if (pBotChat->bot_saytext == BOT_SAYTEXT_HELLO)
   {
      recent_count = 0;

      while (recent_count < 10)
      {
         index = RandomLong (0, bot_language->text.hello_count - 1);
         is_used = FALSE;

         for (i = 0; i < 10; i++)
            if (bot_language->text.recent_hello[i] == index)
               is_used = TRUE;

         if (is_used)
            recent_count++;
         else
            break;
      }   

      // cycle down the recently used phrases
      for (i = 9; i > 0; i--)
         bot_language->text.recent_hello[i] = bot_language->text.recent_hello[i - 1];
      bot_language->text.recent_hello[0] = index;

      strcpy (msg, bot_language->text.hello[index]);
      sprintf (pBotChat->saytext_message, "say %s\n", HumanizeChat (msg)); // humanize the chat string

      // set the delay necessary for the bot to type in the reply
      pBotChat->saytext_time = server.time + (float) (strlen (pBotChat->saytext_message) * pPlayer->Bot.pProfile->skill) / 10;
      return;
   }

   // else does the bot want to laugh at a dead enemy ?
   else if (pBotChat->bot_saytext == BOT_SAYTEXT_LAUGH)
   {
      recent_count = 0;

      while (recent_count < 10)
      {
         index = RandomLong (0, bot_language->text.laugh_count - 1);
         is_used = FALSE;

         for (i = 0; i < 10; i++)
            if (bot_language->text.recent_laugh[i] == index)
               is_used = TRUE;

         if (is_used)
            recent_count++;
         else
            break;
      }   

      // cycle down the recently used phrases
      for (i = 9; i > 0; i--)
         bot_language->text.recent_laugh[i] = bot_language->text.recent_laugh[i - 1];
      bot_language->text.recent_laugh[0] = index;

      // is there a "%s" (replacement variable) in this phrase ?
      if (strstr (bot_language->text.laugh[index], "%s") != NULL)
         sprintf (msg, bot_language->text.laugh[index], Name (players[pPlayer->Bot.victim_index].connection_name));
      else
         strcpy (msg, bot_language->text.laugh[index]);

      sprintf (pBotChat->saytext_message, "say %s\n", HumanizeChat (msg)); // humanize the chat string

      // set the delay necessary for the bot to type in the reply
      pBotChat->saytext_time = server.time + (float) (strlen (pBotChat->saytext_message) * pPlayer->Bot.pProfile->skill) / 10;
      return;
   }

   // else does the bot want to complain about being killed ?
   else if (pBotChat->bot_saytext == BOT_SAYTEXT_WHINE)
   {
      recent_count = 0;

      while (recent_count < 10)
      {
         index = RandomLong (0, bot_language->text.whine_count - 1);
         is_used = FALSE;

         for (i = 0; i < 10; i++)
            if (bot_language->text.recent_whine[i] == index)
               is_used = TRUE;

         if (is_used)
            recent_count++;
         else
            break;
      }

      // cycle down the recently used phrases
      for (i = 9; i > 0; i--)
         bot_language->text.recent_whine[i] = bot_language->text.recent_whine[i - 1];
      bot_language->text.recent_whine[0] = index;

      // is there a "%s" (replacement variable) in this phrase ?
      if (strstr (bot_language->text.whine[index], "%s") != NULL)
         sprintf (msg, bot_language->text.whine[index], Name (players[pPlayer->Bot.killer_index].connection_name));
      else
         strcpy (msg, bot_language->text.whine[index]);

      // bot can now forget his killer
      pPlayer->Bot.killer_index = ((unsigned long) pPlayer - (unsigned long) players) / sizeof (player_t);

      sprintf (pBotChat->saytext_message, "say %s\n", HumanizeChat (msg)); // humanize the chat string

      // set the delay necessary for the bot to type in the reply
      pBotChat->saytext_time = server.time + (float) (strlen (pBotChat->saytext_message) * pPlayer->Bot.pProfile->skill) / 10;
      return;
   }

   // else does the bot want to complain about being lonely for a long time ?
   else if (pBotChat->bot_saytext == BOT_SAYTEXT_ALONE)
   {
      // audio chat (ask teammates to check in)
      pBotChat->bot_sayaudio = BOT_SAYAUDIO_REPORT;

      // text chat
      recent_count = 0;

      while (recent_count < 10)
      {
         index = RandomLong (0, bot_language->text.idle_count - 1);
         is_used = FALSE;

         for (i = 0; i < 10; i++)
            if (bot_language->text.recent_idle[i] == index)
               is_used = TRUE;

         if (is_used)
            recent_count++;
         else
            break;
      }

      // cycle down the recently used phrases
      for (i = 9; i > 0; i--)
         bot_language->text.recent_idle[i] = bot_language->text.recent_idle[i - 1];
      bot_language->text.recent_idle[0] = index;

      // is there a "%s" (replacement variable) in this phrase ?
      if (strstr (bot_language->text.idle[index], "%s") != NULL)
         sprintf (msg, bot_language->text.idle[index], Name (RandomPlayerOtherThan (pPlayer, TRUE, TRUE)->connection_name));
      else
         strcpy (msg, bot_language->text.idle[index]);

      sprintf (pBotChat->saytext_message, "say %s\n", HumanizeChat (msg)); // humanize the chat string

      // set the delay necessary for the bot to type in the reply
      pBotChat->saytext_time = server.time + (float) (strlen (pBotChat->saytext_message) * pPlayer->Bot.pProfile->skill) / 10;
      return;
   }

   // else does the bot want to acknowledge to a "follow me" order ?
   else if (pBotChat->bot_saytext == BOT_SAYTEXT_FOLLOWOK)
   {
      if (FNullEnt (pPlayer->Bot.BotEars.pAskingEntity))
      {
         pBotChat->bot_saytext = 0;
         return; // reliability check
      }

      // audio chat
      pBotChat->bot_sayaudio = BOT_SAYAUDIO_AFFIRMATIVE;

      // text chat
      recent_count = 0;

      while (recent_count < 10)
      {
         index = RandomLong (0, bot_language->text.follow_count - 1);
         is_used = FALSE;

         for (i = 0; i < 10; i++)
            if (bot_language->text.recent_follow[i] == index)
               is_used = TRUE;

         if (is_used)
            recent_count++;
         else
            break;
      }

      // cycle down the recently used phrases
      for (i = 9; i > 0; i--)
         bot_language->text.recent_follow[i] = bot_language->text.recent_follow[i - 1];
      bot_language->text.recent_follow[0] = index;

      // is there a "%s" (replacement variable) in this phrase ?
      if (strstr (bot_language->text.follow[index], "%s") != NULL)
         sprintf (msg, bot_language->text.follow[index], Name (players[ENTINDEX (pPlayer->Bot.BotEars.pAskingEntity) - 1].connection_name));
      else
         strcpy (msg, bot_language->text.follow[index]);

      sprintf (pBotChat->saytext_message, "say_team %s\n", HumanizeChat (msg)); // humanize the chat string

      // set the delay necessary for the bot to type in the reply
      pBotChat->saytext_time = server.time + (float) (strlen (pBotChat->saytext_message) * pPlayer->Bot.pProfile->skill) / 10;
      return;
   }

   // else does the bot want to acknowledge to a "stop following me" order ?
   else if (pBotChat->bot_saytext == BOT_SAYTEXT_STOPOK)
   {
      if (FNullEnt (pPlayer->Bot.BotEars.pAskingEntity))
      {
         pBotChat->bot_saytext = 0;
         return; // reliability check
      }

      // audio chat
      pBotChat->bot_sayaudio = BOT_SAYAUDIO_AFFIRMATIVE;

      // text chat
      recent_count = 0;

      while (recent_count < 10)
      {
         index = RandomLong (0, bot_language->text.stop_count - 1);
         is_used = FALSE;

         for (i = 0; i < 10; i++)
            if (bot_language->text.recent_stop[i] == index)
               is_used = TRUE;

         if (is_used)
            recent_count++;
         else
            break;
      }

      // cycle down the recently used phrases
      for (i = 9; i > 0; i--)
         bot_language->text.recent_stop[i] = bot_language->text.recent_stop[i - 1];
      bot_language->text.recent_stop[0] = index;

      // is there a "%s" (replacement variable) in this phrase ?
      if (strstr (bot_language->text.stop[index], "%s") != NULL)
         sprintf (msg, bot_language->text.stop[index], Name (players[ENTINDEX (pPlayer->Bot.BotEars.pAskingEntity) - 1].connection_name));
      else
         strcpy (msg, bot_language->text.stop[index]);

      sprintf (pBotChat->saytext_message, "say_team %s\n", HumanizeChat (msg)); // humanize the chat string

      // set the delay necessary for the bot to type in the reply
      pBotChat->saytext_time = server.time + (float) (strlen (pBotChat->saytext_message) * pPlayer->Bot.pProfile->skill) / 10;
      return;
   }

   // else does the bot want to acknowledge to a "hold the position" order ?
   else if (pBotChat->bot_saytext == BOT_SAYTEXT_HOLDPOSITIONOK)
   {
      if (FNullEnt (pPlayer->Bot.BotEars.pAskingEntity))
      {
         pBotChat->bot_saytext = 0;
         return; // reliability check
      }

      // audio chat
      pBotChat->bot_sayaudio = BOT_SAYAUDIO_INPOSITION;

      // text chat
      recent_count = 0;

      while (recent_count < 10)
      {
         index = RandomLong (0, bot_language->text.stay_count - 1);
         is_used = FALSE;

         for (i = 0; i < 10; i++)
            if (bot_language->text.recent_stay[i] == index)
               is_used = TRUE;

         if (is_used)
            recent_count++;
         else
            break;
      }

      // cycle down the recently used phrases
      for (i = 9; i > 0; i--)
         bot_language->text.recent_stay[i] = bot_language->text.recent_stay[i - 1];
      bot_language->text.recent_stay[0] = index;

      // is there a "%s" (replacement variable) in this phrase ?
      if (strstr (bot_language->text.stay[index], "%s") != NULL)
         sprintf (msg, bot_language->text.stay[index], Name (players[ENTINDEX (pPlayer->Bot.BotEars.pAskingEntity) - 1].connection_name));
      else
         strcpy (msg, bot_language->text.stay[index]);

      sprintf (pBotChat->saytext_message, "say_team %s\n", HumanizeChat (msg)); // humanize the chat string

      // set the delay necessary for the bot to type in the reply
      pBotChat->saytext_time = server.time + (float) (strlen (pBotChat->saytext_message) * pPlayer->Bot.pProfile->skill) / 10;
      return;
   }

   // else does the bot want to ask for backup ?
   else if (pBotChat->bot_saytext == BOT_SAYTEXT_NEEDBACKUP)
   {
      // audio chat
      pBotChat->bot_sayaudio = BOT_SAYAUDIO_TAKINGDAMAGE;

      // text chat
      recent_count = 0;

      while (recent_count < 10)
      {
         index = RandomLong (0, bot_language->text.help_count - 1);
         is_used = FALSE;

         for (i = 0; i < 10; i++)
            if (bot_language->text.recent_help[i] == index)
               is_used = TRUE;

         if (is_used)
            recent_count++;
         else
            break;
      }

      // cycle down the recently used phrases
      for (i = 9; i > 0; i--)
         bot_language->text.recent_help[i] = bot_language->text.recent_help[i - 1];
      bot_language->text.recent_help[0] = index;

      strcpy (msg, bot_language->text.help[index]);
      sprintf (pBotChat->saytext_message, "say_team %s\n", HumanizeChat (msg)); // humanize the chat string

      // set the delay necessary for the bot to type in the reply
      pBotChat->saytext_time = server.time + (float) (strlen (pBotChat->saytext_message) * pPlayer->Bot.pProfile->skill) / 10;
      return;
   }

/*   // else does the bot want to say to its squad superior it can't follow him anymore ?
   else if (pBotChat->bot_saytext == BOT_SAYTEXT_CANTFOLLOW)
   {
      // audio chat
      pBotChat->bot_sayaudio = BOT_SAYAUDIO_NEGATIVE;

      // text chat
      recent_count = 0;

      while (recent_count < 10)
      {
         index = RandomLong (0, cant_count - 1);
         is_used = FALSE;

         for (i = 0; i < 10; i++)
            if (recent_cant[i] == index)
               is_used = TRUE;

         if (is_used)
            recent_count++;
         else
            break;
      }

      // cycle down the recently used phrases
      for (i = 9; i > 0; i--)
         recent_cant[i] = recent_cant[i - 1];
      recent_cant[0] = index;

      // is there a "%s" (replacement variable) in this phrase ?
      if (strstr (bot_cant[index], "%s") != NULL)
         sprintf (msg, bot_cant[index], Name (players[ENTINDEX (pPlayer->Bot.pBotUser) - 1].connection_name));
      else
         strcpy (msg, bot_cant[index]);

      pPlayer->Bot.pBotUser = NULL; // free the bot's user edict
      pPlayer->Bot.v_lastseenuser_position = g_vecZero; // and also the last seen user position

      sprintf (pBotChat->saytext_message, "say_team %s\n", HumanizeChat (msg)); // humanize the chat string

      // set the delay necessary for the bot to type in the reply
      pBotChat->saytext_time = server.time + (float) (strlen (pBotChat->saytext_message) * pPlayer->Bot.pProfile->skill) / 10;
      return;
   }*/

   // else does the bot want to say goodbye ?
   else if (pBotChat->bot_saytext == BOT_SAYTEXT_BYE)
   {
      recent_count = 0;

      while (recent_count < 10)
      {
         index = RandomLong (0, bot_language->text.bye_count - 1);
         is_used = FALSE;

         for (i = 0; i < 10; i++)
            if (bot_language->text.recent_bye[i] == index)
               is_used = TRUE;

         if (is_used)
            recent_count++;
         else
            break;
      }   

      // cycle down the recently used phrases
      for (i = 9; i > 0; i--)
         bot_language->text.recent_bye[i] = bot_language->text.recent_bye[i - 1];
      bot_language->text.recent_bye[0] = index;

      strcpy (msg, bot_language->text.bye[index]);
      sprintf (pBotChat->saytext_message, "say %s\n", HumanizeChat (msg)); // humanize the chat string

      // set the delay necessary for the bot to type in the reply
      pBotChat->saytext_time = server.time + (float) (strlen (pBotChat->saytext_message) * pPlayer->Bot.pProfile->skill) / 10;
      return;
   }

   return; // bot has nothing to say, apparently...
}


void BotSayAudio (player_t *pPlayer)
{
   // the purpose of this function is the same as BotSayText(), making the bot speak (by
   // emitting sound samples) according to the 12 predefined in-game audio chat situations :
   // affirmative, alert, when attacking, at spawn time, getting in position, asking team
   // to report, reporting to teammates, upon the sight of an enemy grenade, when taking some
   // damage, when throwing a grenade and upon victory in a battle.

   bot_chat_t *pBotChat;
   bot_language_t *bot_language;
   int index;

   pBotChat = &pPlayer->Bot.BotChat; // quick access to bot chat

   // is voice chat forbidden ?
   if ((server.bot_chat_mode == BOT_CHAT_NONE) || (server.bot_chat_mode == BOT_CHAT_TEXTONLY))
      return; // if so, return

   // does the bot already knows what to say ?
   if (pBotChat->sayaudio_time > server.time)
      return; // return; this bot is already about to say something

   if (!players[ENTINDEX (pPlayer->pEntity) - 1].is_alive)
      return; // if bot is dead, return

   // find the bot language
   bot_language = NULL;
   for (index = 0; index < language_count; index++)
      if (strcmp (languages[index].language, pPlayer->Bot.pProfile->nationality) == 0)
         bot_language = &languages[index]; // language name matches with bot nationality

   // have we NOT found one ?
   if (bot_language == NULL)
   {
      // oh well, try again, but with the default language this time
      for (index = 0; index < language_count; index++)
         if (strcmp (languages[index].language, GameConfig.language) == 0)
            bot_language = &languages[index]; // language name matches with default language

      // have we STILL NOT found one ?
      if (bot_language == NULL)
      {
         ServerConsole_printf ("RACC: ALERT: Bot '%s' speaks an unknown language (\"%s\") !\n", pPlayer->connection_name, pPlayer->Bot.pProfile->nationality);
         return; // this bot can't possibly talk
      }
   }

   // is it time to say affirmative ?
   if (pBotChat->bot_sayaudio == BOT_SAYAUDIO_AFFIRMATIVE)
   {
      // pickup a random chat message, store it and set the date at which the bot will say it
      index = RandomLong (0, bot_language->audio.affirmative_count - 1);
      sprintf (pBotChat->sayaudio_message, "racc/%s/affirmative%d.wav", pPlayer->Bot.pProfile->nationality, index);
      pBotChat->sayaudio_time = server.time + RandomFloat (1.0, 2.0);
   }

   // else is it time to say alert ?
   else if (pBotChat->bot_sayaudio == BOT_SAYAUDIO_ALERT)
   {
      // pickup a random chat message, store it and set the date at which the bot will say it
      index = RandomLong (0, bot_language->audio.alert_count - 1); // pickup a random chat message
      sprintf (pBotChat->sayaudio_message, "racc/%s/alert%d.wav", pPlayer->Bot.pProfile->nationality, index);
      pBotChat->sayaudio_time = server.time + RandomFloat (0.5, 2.5);
   }

   // else is it time to say attacking ?
   else if (pBotChat->bot_sayaudio == BOT_SAYAUDIO_ATTACKING)
   {
      // pickup a random chat message, store it and set the date at which the bot will say it
      index = RandomLong (0, bot_language->audio.attacking_count - 1); // pickup a random chat message
      sprintf (pBotChat->sayaudio_message, "racc/%s/attacking%d.wav", pPlayer->Bot.pProfile->nationality, index);
      pBotChat->sayaudio_time = server.time + RandomFloat (1.5, 4.5);
   }

   // else is it time to say firstspawn ?
   else if (pBotChat->bot_sayaudio == BOT_SAYAUDIO_FIRSTSPAWN)
   {
      // pickup a random chat message, store it and set the date at which the bot will say it
      index = RandomLong (0, bot_language->audio.firstspawn_count - 1); // pickup a random chat message
      sprintf (pBotChat->sayaudio_message, "racc/%s/firstspawn%d.wav", pPlayer->Bot.pProfile->nationality, index);
      pBotChat->sayaudio_time = server.time + RandomFloat (2.0, 10.0);
   }

   // else is it time to say inposition ?
   else if (pBotChat->bot_sayaudio == BOT_SAYAUDIO_INPOSITION)
   {
      // pickup a random chat message, store it and set the date at which the bot will say it
      index = RandomLong (0, bot_language->audio.inposition_count - 1); // pickup a random chat message
      sprintf (pBotChat->sayaudio_message, "racc/%s/inposition%d.wav", pPlayer->Bot.pProfile->nationality, index);
      pBotChat->sayaudio_time = server.time + RandomFloat (0.5, 1.5);
   }

   // else is it time to say negative ?
   else if (pBotChat->bot_sayaudio == BOT_SAYAUDIO_NEGATIVE)
   {
      // pickup a random chat message, store it and set the date at which the bot will say it
      index = RandomLong (0, bot_language->audio.negative_count - 1); // pickup a random chat message
      sprintf (pBotChat->sayaudio_message, "racc/%s/negative%d.wav", pPlayer->Bot.pProfile->nationality, index);
      pBotChat->sayaudio_time = server.time + RandomFloat (1.0, 2.0);
   }

   // else is it time to say report ?
   else if (pBotChat->bot_sayaudio == BOT_SAYAUDIO_REPORT)
   {
      // pickup a random chat message, store it and set the date at which the bot will say it
      index = RandomLong (0, bot_language->audio.report_count - 1); // pickup a random chat message
      sprintf (pBotChat->sayaudio_message, "racc/%s/report%d.wav", pPlayer->Bot.pProfile->nationality, index);
      pBotChat->sayaudio_time = server.time + RandomFloat (1.0, 2.0);
   }

   // else is it time to say reportingin ?
   else if (pBotChat->bot_sayaudio == BOT_SAYAUDIO_REPORTINGIN)
   {
      // pickup a random chat message, store it and set the date at which the bot will say it
      index = RandomLong (0, bot_language->audio.reporting_count - 1); // pickup a random chat message
      sprintf (pBotChat->sayaudio_message, "racc/%s/reporting%d.wav", pPlayer->Bot.pProfile->nationality, index);
      pBotChat->sayaudio_time = server.time + RandomFloat (2.0, 5.0);
   }

   // else is it time to say seegrenade ?
   else if (pBotChat->bot_sayaudio == BOT_SAYAUDIO_SEEGRENADE)
   {
      // pickup a random chat message, store it and set the date at which the bot will say it
      index = RandomLong (0, bot_language->audio.seegrenade_count - 1); // pickup a random chat message
      sprintf (pBotChat->sayaudio_message, "racc/%s/seegrenade%d.wav", pPlayer->Bot.pProfile->nationality, index);
      pBotChat->sayaudio_time = server.time + RandomFloat (0.3, 0.8);
   }

   // else is it time to say takingdamage ?
   else if (pBotChat->bot_sayaudio == BOT_SAYAUDIO_TAKINGDAMAGE)
   {
      // pickup a random chat message, store it and set the date at which the bot will say it
      index = RandomLong (0, bot_language->audio.takingdamage_count - 1); // pickup a random chat message
      sprintf (pBotChat->sayaudio_message, "racc/%s/takingdamage%d.wav", pPlayer->Bot.pProfile->nationality, index);
      pBotChat->sayaudio_time = server.time + RandomFloat (0.5, 1.5);
   }

   // else is it time to say throwgrenade ?
   else if (pBotChat->bot_sayaudio == BOT_SAYAUDIO_THROWGRENADE)
   {
      // pickup a random chat message, store it and set the date at which the bot will say it
      index = RandomLong (0, bot_language->audio.throwgrenade_count - 1); // pickup a random chat message
      sprintf (pBotChat->sayaudio_message, "racc/%s/throwgrenade%d.wav", pPlayer->Bot.pProfile->nationality, index);
      pBotChat->sayaudio_time = server.time + RandomFloat (0.5, 1.0);
   }

   // else is it time to say victory ?
   else if (pBotChat->bot_sayaudio == BOT_SAYAUDIO_VICTORY)
   {
      // pickup a random chat message, store it and set the date at which the bot will say it
      index = RandomLong (0, bot_language->audio.victory_count - 1); // pickup a random chat message
      sprintf (pBotChat->sayaudio_message, "racc/%s/victory%d.wav", pPlayer->Bot.pProfile->nationality, index);
      pBotChat->sayaudio_time = server.time + RandomFloat (2.0, 4.0);
   }

   return; // finished
}


void BotStartTalking (player_t *pPlayer)
{
   // this is the function that fakes the HLVoice-style chatting for bots. Here each client is
   // ordered to play this sound locally, this resulting in non attenuated sound, simulating
   // a HLVoice conversation. A HLVoice-style icon is also displayed on top of the bot's head
   // for the duration of the sound, adding a little to credibility.

   int index;
   player_t *pTeammate;

   // is audio chat forbidden ?
   if ((server.bot_chat_mode == BOT_CHAT_NONE) || (server.bot_chat_mode == BOT_CHAT_TEXTONLY))
      return; // return; bots are not allowed to talk

   // cycle through all clients
   for (index = 0; index < server.max_clients; index++)
   {
      pTeammate = &players[index]; // quick access to player

      if (!IsValidPlayer (pTeammate) || (pTeammate == pPlayer)
          || !pTeammate->is_alive || pTeammate->is_racc_bot)
         continue; // skip invalid players and skip self (i.e. this bot)

      // check if it is a teammate; if so, talk "in his head"
      if (GetTeam (pTeammate) == GetTeam (pPlayer))
      {
         sprintf (g_argv, "play %s\n", pPlayer->Bot.BotChat.sayaudio_message);
         CLIENT_COMMAND (pTeammate->pEntity, g_argv); // play bot's talk on client side
         DisplaySpeakerIcon (pPlayer, pTeammate); // display speaker icon
      }
   }

   // set the speaker icon disappearance time
   pPlayer->Bot.BotChat.speaker_time = server.time + RandomFloat (1.5, 2.5);
   return;
}


void BotStopTalking (player_t *pPlayer)
{
   // this is the pending function of BotStartTalking(), which fakes the HLVoice-style chatting
   // for bots. Here the function destroys each HLVoice speaker icon above the head of the bot
   // meaning that the bot finished talking.

   int index;
   player_t *pTeammate;

   // is audio chat forbidden ?
   if ((server.bot_chat_mode == BOT_CHAT_NONE) || (server.bot_chat_mode == BOT_CHAT_TEXTONLY))
      return; // return; bots are not allowed to talk

   // cycle through all clients
   for (index = 0; index < server.max_clients; index++)
   {
      pTeammate = &players[index]; // quick access to player

      if (!IsValidPlayer (pTeammate) || (pTeammate == pPlayer)
          || !pTeammate->is_alive || pTeammate->is_racc_bot)
         continue; // skip invalid players and skip self (i.e. this bot)

      // check if it is a teammate; if so, talk "in his head"
      if (GetTeam (pTeammate) == GetTeam (pPlayer))
         DestroySpeakerIcon (pPlayer, pTeammate); // destroy speaker icon
   }

   return;
}


void DisplaySpeakerIcon (player_t *pPlayer, player_t *pViewerClient)
{
   // this function is supposed to display that tiny speaker icon above the head of the player
   // whose entity is pointed to by pPlayer, so that pViewerClient sees it, during duration * 10
   // seconds long.

   // l33t CS 1.6 specific bot audio (thanks Whistler)
   MESSAGE_BEGIN (MSG_ONE, GetUserMsgId ("BotVoice"), NULL, pViewerClient->pEntity);
   WRITE_BYTE (1); // turns on speaker - thanks to Whistler for the trick !
   WRITE_BYTE (ENTINDEX (pPlayer->pEntity)); // byte (entity index of pEdict)
   MESSAGE_END ();
}


void DestroySpeakerIcon (player_t *pPlayer, player_t *pViewerClient)
{
   // this function stops displaying any speaker icon above the head of the player whose entity
   // is pointed to by pPlayer, so that pViewerClient doesn't see them anymore. Actually it also
   // stops displaying any player attachment temporary entity.

   // l33t CS 1.6 specific bot audio (thanks Whistler)
   MESSAGE_BEGIN (MSG_ONE, GetUserMsgId ("BotVoice"), NULL, pViewerClient->pEntity);
   WRITE_BYTE (0); // turns off speaker
   WRITE_BYTE (ENTINDEX (pPlayer->pEntity)); // byte (entity index of pEdict)
   MESSAGE_END ();
}


player_t *RandomPlayerOtherThan (player_t *pOtherPlayer, bool want_enemy, bool want_alive)
{
   // this function is a helper that returns a pointer to a random player other than pOtherPlayer.
   // If there are not enough players in game to return another player name, the function returns
   // NULL. If want_alive is specified to TRUE, this function returns the name of a living player
   // only. Identically, if want_enemy is specified to TRUE, this function returns the name of an
   // enemy of pOtherPlayer.

   player_t *pPlayer;
   int index;
   bool all_dead;
   bool all_friends;

   if (player_count == 1)
      return (NULL); // if only 1 player, can't possibly get another player name

   // do we want a living player only OR an enemy only ?
   if (want_alive || want_enemy)
   {
      // cycle through all players but self and check if one is alive or enemy
      all_dead = TRUE; // start by assuming that all are dead
      all_friends = TRUE; // start by assuming that all are friends
      for (index = 0; index < server.max_clients; index++)
      {
         pPlayer = &players[index]; // quick access to player

         if (IsValidPlayer (pPlayer) && (pPlayer != pOtherPlayer))
         {
            if (want_alive && pPlayer->is_alive)
               all_dead = FALSE; // phew, at least one of them is alive. kewl.
            if (want_enemy && (GetTeam (pPlayer) != GetTeam (pOtherPlayer)))
               all_friends = FALSE; // phew, at least one of them is enemy. kewl.
         }
      }
   }

   if (want_alive && all_dead)
      return (NULL); // if none alive, can't possibly get another player name

   if (want_enemy && all_friends)
      return (NULL); // if none enemy, can't possibly get another player name

   // pick up a random player, keep searching until one is found
   do
   {
      pPlayer = &players[RandomLong (0, server.max_clients - 1)]; // quick access to random player

      if (want_alive && !pPlayer->is_alive)
         continue; // skip dead players if we only want living ones

      if (want_enemy && (GetTeam (pPlayer) == GetTeam (pOtherPlayer)))
         continue; // skip dead players if we only want living ones
   }
   while (!IsValidPlayer (pPlayer) || (pPlayer == pOtherPlayer));

   // found one, return a pointer to his structure
   return (pPlayer);
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
   if (RandomLong (1, 100) < 85)
      strcpy (buffer, StripTags (string));
   else
      strcpy (buffer, StripBlanks (string));

   length = strlen (buffer); // get name buffer's length

   // half the time switch the name to lower characters
   if (RandomLong (1, 100) < 50)
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
   if (RandomLong (1, 100) < 33)
   {
      for (index = 0; index < length; index++)
         buffer[index] = tolower (buffer[index]); // switch buffer to lowercase
   }

   // if length is sufficient to assume the text had to be typed in a hurry
   if (length > 15)
   {
      // "length" percent of time drop a character
      if (RandomLong (1, 100) < length)
      {
         pos = RandomLong (length / 8, length); // choose a random position in string

         for (index = pos; index < length - 1; index++)
            buffer[index] = buffer[index + 1]; // overwrite the buffer with the stripped string
         buffer[index] = 0; // terminate the string
         length--; // update new string length
      }

      // "length" / 2 percent of time swap a character
      if (RandomLong (1, 100) < length / 2)
      {
         char tempchar;
         pos = RandomLong (length / 8, length); // choose a random position in string

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
        buffer[index] = ')';
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


const char *BotHALGenerateReply (player_t *pPlayer)
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

   keywords = BotHALMakeKeywords (pPlayer, pPlayer->Bot.BotBrain.input_words);
   replywords = BotHALBuildReplyDictionary (pPlayer, keywords);

   last_word = pPlayer->Bot.BotBrain.input_words->size - 1;
   last_character = pPlayer->Bot.BotBrain.input_words->entry[last_word].length - 1;

   // was it a question (i.e. was the last word in the general chat record a question mark ?)
   if (pPlayer->Bot.BotBrain.input_words->entry[last_word].word[last_character] == '?')
   {
      // try ten times to answer something relevant
      for (tries_count = 0; tries_count < 10; tries_count++)
      {
         if (HAL_DictionariesDiffer (pPlayer->Bot.BotBrain.input_words, replywords))
            break; // stop as soon as we've got something to say
         else
            replywords = BotHALBuildReplyDictionary (pPlayer, keywords); // else think again
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
   else if (HAL_DictionariesDiffer (pPlayer->Bot.BotBrain.input_words, replywords))
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
      TerminateOnError ("HAL_AddWord() unable to reallocate the dictionary index\n");

   // allocate one more entry for the word array
   if (dictionary->entry == NULL)
      dictionary->entry = (HAL_STRING *) malloc (sizeof (HAL_STRING) * dictionary->size);
   else
      dictionary->entry = (HAL_STRING *) realloc ((HAL_STRING *) dictionary->entry, sizeof (HAL_STRING) * dictionary->size);

   if (dictionary->entry == NULL)
      TerminateOnError ("HAL_AddWord() unable to reallocate the dictionary to %d elements\n", dictionary->size);

   // copy the new word into the word array
   dictionary->entry[dictionary->size - 1].length = word.length;
   dictionary->entry[dictionary->size - 1].word = (char *) malloc (sizeof (char) * word.length);
   if (dictionary->entry[dictionary->size - 1].word == NULL)
      TerminateOnError ("HAL_AddWord() unable to allocate the word\n");

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
      TerminateOnError ("HAL_NewDictionary() unable to allocate dictionary\n");

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
         TerminateOnError ("HAL_LoadDictionary() unable to allocate word\n");

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
      TerminateOnError ("HAL_NewNode() unable to allocate node\n");

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
      TerminateOnError ("HAL_AddNode() unable to reallocate subtree\n");

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
      TerminateOnError ("HAL_LoadTree() unable to allocate subtree\n");

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
            TerminateOnError ("HAL_MakeWords() unable to reallocate dictionary\n");

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


HAL_DICTIONARY *BotHALMakeKeywords (player_t *pPlayer, HAL_DICTIONARY *words)
{
   // this function puts all the interesting words from the user's input into a keywords
   // dictionary, which will be used when generating a reply

   register int i;
   register int j;
   int c;

   if (pPlayer->Bot.BotBrain.keys->entry != NULL)
      for (i = 0; i < (int) pPlayer->Bot.BotBrain.keys->size; ++i)
         if (pPlayer->Bot.BotBrain.keys->entry[i].word != NULL)
            free (pPlayer->Bot.BotBrain.keys->entry[i].word);

   HAL_EmptyDictionary (pPlayer->Bot.BotBrain.keys);

   for (i = 0; i < (int) words->size; ++i)
   {
      // find the symbol ID of the word. If it doesn't exist in the model, or if it begins
      // with a non-alphanumeric character, or if it is in the exclusion array, then skip it

      c = 0;

      for (j = 0; j < pPlayer->Bot.BotBrain.swappable_keywords->size; ++j)
         if (HAL_CompareWords (pPlayer->Bot.BotBrain.swappable_keywords->from[j], words->entry[i]) == 0)
         {
            BotHALAddKeyword (pPlayer, pPlayer->Bot.BotBrain.keys, pPlayer->Bot.BotBrain.swappable_keywords->to[j]);
            c++;
         }

      if (c == 0)
         BotHALAddKeyword (pPlayer, pPlayer->Bot.BotBrain.keys, words->entry[i]);
   }

   if (pPlayer->Bot.BotBrain.keys->size > 0)
      for (i = 0; i < (int) words->size; ++i)
      {
         c = 0;

         for (j = 0; j < pPlayer->Bot.BotBrain.swappable_keywords->size; ++j)
            if (HAL_CompareWords (pPlayer->Bot.BotBrain.swappable_keywords->from[j], words->entry[i]) == 0)
            {
               BotHALAddAuxiliaryKeyword (pPlayer, pPlayer->Bot.BotBrain.keys, pPlayer->Bot.BotBrain.swappable_keywords->to[j]);
               c++;
            }

         if (c == 0)
            BotHALAddAuxiliaryKeyword (pPlayer, pPlayer->Bot.BotBrain.keys, words->entry[i]);
      }

   return (pPlayer->Bot.BotBrain.keys);
}


void BotHALAddKeyword (player_t *pPlayer, HAL_DICTIONARY *keys, HAL_STRING word)
{
   // this function adds a word to the keyword dictionary

   int symbol;
   
   if (!IsValidPlayer (pPlayer))
      return; // reliability check

   symbol = HAL_FindWord (pPlayer->Bot.BotBrain.HAL_model.dictionary, word);

   if (symbol == 0)
      return; // if void, return

   if (isalnum (word.word[0]) == 0)
      return; // if not alphanumeric, return

   symbol = HAL_FindWord (pPlayer->Bot.BotBrain.banned_keywords, word); // is this word in the banned dictionary ?
   if (symbol != 0)
      return; // if so, return

   symbol = HAL_FindWord (pPlayer->Bot.BotBrain.auxiliary_keywords, word); // is this word in the auxiliary dictionary ?
   if (symbol != 0)
      return; // if so, return

   HAL_AddWord (keys, word); // once we are sure this word isn't known yet, we can add it
}


void BotHALAddAuxiliaryKeyword (player_t *pPlayer, HAL_DICTIONARY *keys, HAL_STRING word)
{
   // this function adds an auxilliary keyword to the keyword dictionary

   int symbol;

   if (!IsValidPlayer (pPlayer))
      return; // reliability check

   symbol = HAL_FindWord (pPlayer->Bot.BotBrain.HAL_model.dictionary, word);

   if (symbol == 0)
      return; // if void, return

   if (isalnum (word.word[0]) == 0)
      return; // if not alphanumeric, return

   symbol = HAL_FindWord (pPlayer->Bot.BotBrain.auxiliary_keywords, word); // is it already in the dictionary ?
   if (symbol == 0)
      return; // if already in dictionary, return

   HAL_AddWord (keys, word); // add this word to the keywords dictionary
}


HAL_DICTIONARY *BotHALBuildReplyDictionary (player_t *pPlayer, HAL_DICTIONARY *keys)
{
   // this function generates a dictionary of reply words relevant to the dictionary of keywords

   register int i;
   int symbol;
   bool start = TRUE;

   HAL_EmptyDictionary (pPlayer->Bot.BotBrain.replies);

   // start off by making sure that the model's context is empty
   HAL_InitializeContext (&pPlayer->Bot.BotBrain.HAL_model);
   pPlayer->Bot.BotBrain.HAL_model.context[0] = pPlayer->Bot.BotBrain.HAL_model.forward;
   pPlayer->Bot.BotBrain.keyword_is_used = FALSE;

   // generate the reply in the forward direction
   while (TRUE)
   {
      // get a random symbol from the current context
      if (start == TRUE)
         symbol = BotHALSeedReply (pPlayer, keys);
      else
         symbol = BotHALBabble (pPlayer, keys, pPlayer->Bot.BotBrain.replies);

      if ((symbol == 0) || (symbol == 1))
         break;

      start = FALSE;

      // append the symbol to the reply dictionary
      if (pPlayer->Bot.BotBrain.replies->entry == NULL)
         pPlayer->Bot.BotBrain.replies->entry = (HAL_STRING *) malloc ((pPlayer->Bot.BotBrain.replies->size + 1) * sizeof (HAL_STRING));
      else
         pPlayer->Bot.BotBrain.replies->entry = (HAL_STRING *) realloc (pPlayer->Bot.BotBrain.replies->entry, (pPlayer->Bot.BotBrain.replies->size + 1) * sizeof (HAL_STRING));

      if (pPlayer->Bot.BotBrain.replies->entry == NULL)
         TerminateOnError ("BotHALBuildReplyDictionary() unable to reallocate dictionary\n");

      pPlayer->Bot.BotBrain.replies->entry[pPlayer->Bot.BotBrain.replies->size].length = pPlayer->Bot.BotBrain.HAL_model.dictionary->entry[symbol].length;
      pPlayer->Bot.BotBrain.replies->entry[pPlayer->Bot.BotBrain.replies->size].word = pPlayer->Bot.BotBrain.HAL_model.dictionary->entry[symbol].word;
      pPlayer->Bot.BotBrain.replies->size++;

      // extend the current context of the model with the current symbol
      HAL_UpdateContext (&pPlayer->Bot.BotBrain.HAL_model, symbol);
   }

   // start off by making sure that the model's context is empty
   HAL_InitializeContext (&pPlayer->Bot.BotBrain.HAL_model);
   pPlayer->Bot.BotBrain.HAL_model.context[0] = pPlayer->Bot.BotBrain.HAL_model.backward;

   // re-create the context of the model from the current reply dictionary so that we can
   // generate backwards to reach the beginning of the string.
   if (pPlayer->Bot.BotBrain.replies->size > 0)
      for (i = min (pPlayer->Bot.BotBrain.replies->size - 1, pPlayer->Bot.BotBrain.HAL_model.order); i >= 0; --i)
      {
         symbol = HAL_FindWord (pPlayer->Bot.BotBrain.HAL_model.dictionary, pPlayer->Bot.BotBrain.replies->entry[i]);
         HAL_UpdateContext (&pPlayer->Bot.BotBrain.HAL_model, symbol);
      }

   // generate the reply in the backward direction
   while (TRUE)
   {
      // get a random symbol from the current context
      symbol = BotHALBabble (pPlayer, keys, pPlayer->Bot.BotBrain.replies);
      if ((symbol == 0) || (symbol == 1))
         break;

      // prepend the symbol to the reply dictionary
      if (pPlayer->Bot.BotBrain.replies->entry == NULL)
         pPlayer->Bot.BotBrain.replies->entry = (HAL_STRING *) malloc ((pPlayer->Bot.BotBrain.replies->size + 1) * sizeof (HAL_STRING));
      else
         pPlayer->Bot.BotBrain.replies->entry = (HAL_STRING *) realloc (pPlayer->Bot.BotBrain.replies->entry, (pPlayer->Bot.BotBrain.replies->size + 1) * sizeof (HAL_STRING));

      if (pPlayer->Bot.BotBrain.replies->entry == NULL)
         TerminateOnError ("BotHALBuildReplyDictionary() unable to reallocate dictionary\n");

      // shuffle everything up for the prepend
      for (i = pPlayer->Bot.BotBrain.replies->size; i > 0; --i)
      {
         pPlayer->Bot.BotBrain.replies->entry[i].length = pPlayer->Bot.BotBrain.replies->entry[i - 1].length;
         pPlayer->Bot.BotBrain.replies->entry[i].word = pPlayer->Bot.BotBrain.replies->entry[i - 1].word;
      }

      pPlayer->Bot.BotBrain.replies->entry[0].length = pPlayer->Bot.BotBrain.HAL_model.dictionary->entry[symbol].length;
      pPlayer->Bot.BotBrain.replies->entry[0].word = pPlayer->Bot.BotBrain.HAL_model.dictionary->entry[symbol].word;
      pPlayer->Bot.BotBrain.replies->size++;

      // extend the current context of the model with the current symbol
      HAL_UpdateContext (&pPlayer->Bot.BotBrain.HAL_model, symbol);
   }

   return (pPlayer->Bot.BotBrain.replies);
}


int BotHALBabble (player_t *pPlayer, HAL_DICTIONARY *keys, HAL_DICTIONARY *words)
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
   for (i = 0; i <= pPlayer->Bot.BotBrain.HAL_model.order; ++i)
      if (pPlayer->Bot.BotBrain.HAL_model.context[i] != NULL)
         node = pPlayer->Bot.BotBrain.HAL_model.context[i];

   if (node->branch == 0)
      return (0);

   // choose a symbol at random from this context
   i = RandomLong (0, node->branch - 1);
   count = RandomLong (0, node->usage - 1);

   while (count >= 0)
   {
      // if the symbol occurs as a keyword, then use it. Only use an auxilliary keyword if
      // a normal keyword has already been used.
      symbol = node->tree[i]->symbol;

      if ((HAL_FindWord (keys, pPlayer->Bot.BotBrain.HAL_model.dictionary->entry[symbol]) != 0)
          && (pPlayer->Bot.BotBrain.keyword_is_used || (HAL_FindWord (pPlayer->Bot.BotBrain.auxiliary_keywords, pPlayer->Bot.BotBrain.HAL_model.dictionary->entry[symbol]) == 0))
          && !HAL_WordExists (words, pPlayer->Bot.BotBrain.HAL_model.dictionary->entry[symbol]))
      {
         pPlayer->Bot.BotBrain.keyword_is_used = TRUE;
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


int BotHALSeedReply (player_t *pPlayer, HAL_DICTIONARY *keys)
{
   // this function seeds the reply by guaranteeing that it contains a keyword, if one exists

   register int i;
   int symbol;
   int stop;

   // be aware of the special case where the tree is empty
   if (pPlayer->Bot.BotBrain.HAL_model.context[0]->branch == 0)
      symbol = 0;
   else
      symbol = pPlayer->Bot.BotBrain.HAL_model.context[0]->tree[RandomLong (0, pPlayer->Bot.BotBrain.HAL_model.context[0]->branch - 1)]->symbol;

   if (keys->size > 0)
   {
      i = RandomLong (0, keys->size - 1);
      stop = i;

      while (TRUE)
      {
         if ((HAL_FindWord (pPlayer->Bot.BotBrain.HAL_model.dictionary, keys->entry[i]) != 0) && (HAL_FindWord (pPlayer->Bot.BotBrain.auxiliary_keywords, keys->entry[i]) == 0))
         {
            symbol = HAL_FindWord (pPlayer->Bot.BotBrain.HAL_model.dictionary, keys->entry[i]);
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
      TerminateOnError ("HAL_NewSwap() unable to allocate swap\n");

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
         TerminateOnError ("HAL_AddSwap() unable to allocate list->from\n");
   }

   if (list->to == NULL)
   {
      list->to = (HAL_STRING *) malloc (sizeof (HAL_STRING));
      if (list->to == NULL)
         TerminateOnError ("HAL_AddSwap() unable to allocate list->to\n");
   }

   list->from = (HAL_STRING *) realloc (list->from, sizeof (HAL_STRING) * (list->size + 1));
   if (list->from == NULL)
      TerminateOnError ("HAL_AddSwap() unable to reallocate from\n");

   list->to = (HAL_STRING *) realloc (list->to, sizeof (HAL_STRING) * (list->size + 1));
   if (list->to == NULL)
      TerminateOnError ("HAL_AddSwap() unable to reallocate to\n");

   list->from[list->size].length = (unsigned char) strlen (s);
   list->from[list->size].word = strdup (s);
   list->to[list->size].length = (unsigned char) strlen (d);
   list->to[list->size].word = strdup (d);
   list->size++;
}


HAL_SWAP *HAL_InitializeSwap (char *filename)
{
   // this function reads a HAL swap structure from a file. Filename indicates a relative path
   // starting from the RACC base directory.

   HAL_SWAP *list;
   MFILE *fp;
   char file_path[256];
   char line_buffer[1024];
   char *from;
   char *to;

   list = HAL_NewSwap ();

   if (filename == NULL)
      return (list);

   // build the file path
   sprintf (file_path, "%s/%s", GameConfig.racc_basedir, filename);

   fp = mfopen (file_path, "r");
   if (fp == NULL)
      return (list);

   while (!mfeof (fp))
   {
      if (mfgets (line_buffer, 1024, fp) == NULL)
         break;

      if ((line_buffer[0] == 0) || (line_buffer[0] == '\n')
          || (line_buffer[0] == ';') || (line_buffer[0] == '#'))
         continue; // ignore line if void or commented

      from = strtok (line_buffer, "\t"); // split the line at next separator
      to = strtok (NULL, "\t\n#;"); // split the line at end of field

      HAL_AddSwap (list, from, to);
   }

   mfclose (fp);
   return (list);
}


HAL_DICTIONARY *HAL_InitializeList (char *filename)
{
   // this function reads a HAL dictionary from a file. Filename indicates a relative path
   // starting from the RACC base directory.

   HAL_DICTIONARY *list;
   MFILE *fp;
   char file_path[256];
   HAL_STRING word;
   char *string;
   char line_buffer[1024];

   list = HAL_NewDictionary ();

   if (filename == NULL)
      return (list);

   // build the file path
   sprintf (file_path, "%s/%s", GameConfig.racc_basedir, filename);

   fp = mfopen (file_path, "r");
   if (fp == NULL)
      return (list);

   while (!mfeof (fp))
   {
      if (mfgets (line_buffer, 1024, fp) == NULL)
         break;

      if ((line_buffer[0] == 0) || (line_buffer[0] == '\n')
          || (line_buffer[0] == ';') || (line_buffer[0] == '#'))
         continue; // ignore line if void or commented

      string = strtok (line_buffer, "\t\n#;"); // split the string at the next separator

      if ((string != NULL) && (strlen (string) > 0))
      {
         word.length = (unsigned char) strlen (string);
         word.word = strdup (line_buffer);
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


void BotHALLoadBrain (player_t *pPlayer)
{
   // this function prepares and load a HAL brain for the specified bot

   MFILE *fp;
   FILE *fp2;
   char ban_filename[256];
   char aux_filename[256];
   char swp_filename[256];
   char brn_filename[256];
   bot_brain_t *brain;
   char cookie[32];
   bool valid_brain = FALSE;

   brain = &pPlayer->Bot.BotBrain; // quick access to bot brain

   // build the file names
   sprintf (ban_filename, "%s/knowledge/%s/keywords.ban", GameConfig.racc_basedir, GameConfig.mod_name);
   sprintf (aux_filename, "%s/knowledge/%s/keywords.aux", GameConfig.racc_basedir, GameConfig.mod_name);
   sprintf (swp_filename, "%s/knowledge/%s/keywords.swp", GameConfig.racc_basedir, GameConfig.mod_name);
   sprintf (brn_filename, "%s/knowledge/%s/%s.brn", GameConfig.racc_basedir, GameConfig.mod_name, NormalizeChars (pPlayer->connection_name));

   // first make sure the brain space is empty
   HAL_FreeDictionary (brain->banned_keywords);
   HAL_FreeDictionary (brain->auxiliary_keywords);
   HAL_FreeSwap (brain->swappable_keywords);
   HAL_EmptyModel (&brain->HAL_model);
   HAL_FreeDictionary (brain->input_words);
   HAL_FreeDictionary (brain->bot_words);
   HAL_FreeDictionary (brain->keys);
   HAL_FreeDictionary (brain->replies);
   brain->keyword_is_used = FALSE;

   // initialize a new model
   brain->HAL_model.order = (unsigned char) BOT_HAL_MODEL_ORDER; // create a n-gram language model
   brain->HAL_model.forward = HAL_NewNode ();
   brain->HAL_model.backward = HAL_NewNode ();
   brain->HAL_model.context = (HAL_TREE **) malloc ((BOT_HAL_MODEL_ORDER + 2) * sizeof (HAL_TREE *));
   if (brain->HAL_model.context == NULL)
      TerminateOnError ("BotHALLoadBrain() unable to allocate context array\n");

   // initialize the new model's context and dictionary
   HAL_InitializeContext (&brain->HAL_model);
   brain->HAL_model.dictionary = HAL_NewDictionary ();
   HAL_InitializeDictionary (brain->HAL_model.dictionary);

   // read dictionaries containing banned keywords, auxiliary keywords and swap keywords
   brain->banned_keywords = HAL_InitializeList (ban_filename);
   brain->auxiliary_keywords = HAL_InitializeList (aux_filename);
   brain->swappable_keywords = HAL_InitializeSwap (swp_filename);

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
      ServerConsole_printf ("RACC: bot %s's HAL brain damaged!\n", pPlayer->connection_name);

      if (server.developer_level > 1)
         ServerConsole_printf ("RACC: inferring a new HAL brain to %s\n", pPlayer->connection_name);

      // create the new brain (i.e, save a void one in the brain file)
      fp2 = fopen (brn_filename, "wb");
      if (fp2 == NULL)
         TerminateOnError ("BotLoadHALBrain(): %s's HAL brain refuses surgery!\n", pPlayer->connection_name);

      fwrite ("RACCHAL", sizeof ("RACCHAL"), 1, fp2);
      fwrite (&brain->HAL_model.order, sizeof (unsigned char), 1, fp2);
      HAL_SaveTree (fp2, brain->HAL_model.forward);
      HAL_SaveTree (fp2, brain->HAL_model.backward);
      HAL_SaveDictionary (fp2, brain->HAL_model.dictionary);
      fclose (fp2); // everything is saved, close the file
   }

   if (server.developer_level > 1)
      ServerConsole_printf ("RACC: restoring HAL brain to %s\n", pPlayer->connection_name);

   // now that we ensured about its validity, we can safely load the brain
   fp = mfopen (brn_filename, "rb"); // open the brain file again
   mfseek (fp, 0, SEEK_SET); // seek at start of file
   mfread (cookie, sizeof ("RACCHAL"), 1, fp); // read the brain signature
   mfread (&brain->HAL_model.order, 1, 1, fp); // load the model order
   HAL_LoadTree (fp, brain->HAL_model.forward); // load the forward tree
   HAL_LoadTree (fp, brain->HAL_model.backward); // load the backwards tree
   HAL_LoadDictionary (fp, brain->HAL_model.dictionary); // load the dictionary
   mfclose (fp);

   brain->input_words = HAL_NewDictionary (); // create the global chat dictionary
   brain->keys = HAL_NewDictionary (); // create the temporary keywords dictionary
   brain->replies = HAL_NewDictionary (); // create the temporary replies dictionary

   return; // finished
}


void BotHALSaveBrain (player_t *pPlayer)
{
   // this function saves the current state to a HAL brain file

   FILE *fp;
   bot_brain_t *brain;
   char filename[256];

   brain = &pPlayer->Bot.BotBrain; // quick access to bot brain

   // build the file names
   sprintf (filename, "%s/knowledge/%s/%s.brn", GameConfig.racc_basedir, GameConfig.mod_name, NormalizeChars (pPlayer->connection_name));

   fp = fopen (filename, "wb");
   if (fp == NULL)
   {
      ServerConsole_printf ("RACC: Unable to save %s's HAL brain to %s\n", pPlayer->connection_name, filename);
      return;
   }

   // dump the HAL brain to disk
   fwrite ("RACCHAL", sizeof ("RACCHAL"), 1, fp);
   fwrite (&brain->HAL_model.order, sizeof (unsigned char), 1, fp);
   HAL_SaveTree (fp, brain->HAL_model.forward);
   HAL_SaveTree (fp, brain->HAL_model.backward);
   HAL_SaveDictionary (fp, brain->HAL_model.dictionary);

   fclose (fp); // finished, close the file
   return; // and return
}
