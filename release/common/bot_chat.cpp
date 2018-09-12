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
// bot_chat.cpp
//

#include "racc.h"

extern char mod_name[256];
extern char map_name[256];
extern bot_t bots[MAX_CLIENTS_SUPPORTED_BY_ENGINE];
extern char *g_argv;
extern int player_count;


void BotChat (bot_t *pBot)
{
   // the purpose of this function is to make the bot keep an eye on what's happening in the
   // chat room, and in case of new messages, think about a possible reply.

   int client_index;

   if (IsNull (pBot->pEntity))
      return; // reliability check

   // is there a new message on the bot's screen ?
   if (pBot->BotEyes.BotHUD.new_chat_message)
   {
      // is bot chat allowed AND this message is not from this bot itself ?
      if ((GetServerVariable ("racc_chatmode") > 0)
          && (strcmp (pBot->BotEyes.BotHUD.chat_line[0].sender, NetnameOf (pBot->pEntity)) != 0))
      {
         // break the new message into an array of words
         HAL_MakeWords (pBot->BotEyes.BotHUD.chat_line[0].text, pBot->pPersonality->input_words);

         // cycle through all players
         for (client_index = 0; client_index < MaxClientsOnServer (); client_index++)
         {
            entity_t *pPlayer = PlayerAtIndex (client_index);

            if (IsNull (pPlayer) || (pPlayer == pBot->pEntity))
               continue; // skip invalid players and skip self (i.e. this bot)

            // is this player the sender of the message AND this player is not a bot ?
            if ((strcmp (pBot->BotEyes.BotHUD.chat_line[0].sender, NetnameOf (pPlayer)) == 0) && !IsABot (pPlayer))
            {
               HAL_Learn (pBot->pPersonality->bot_model, pBot->pPersonality->input_words); // only learn from humans
               break; // stop looping after learning that words into the model
            }
         }

         // does the bot feel concerned ? (more chances of replying if its name appears)
         if ((strstr (pBot->BotEyes.BotHUD.chat_line[0].text, Name (pBot->pPersonality->name)) != NULL)
             || (RandomInteger (1, 100) < 70 - 2 * player_count))
         {
            BotHALGenerateReply (pBot, pBot->BotChat.say_message); // generate the reply
            LowerCase (pBot->BotChat.say_message); // convert the output string to lowercase

            // set the delay necessary for the bot to type in the reply
            pBot->BotChat.b_saytext_halreply = TRUE;
            pBot->BotChat.f_saytext_time = CurrentTime () + (float) (strlen (pBot->BotChat.say_message) * pBot->pPersonality->skill) * 0.06;
         }
      }

      pBot->BotEyes.BotHUD.new_chat_message = FALSE; // OK, we've seen that message
   }

   return;
}


void BotSayText (bot_t *pBot)
{
   // the purpose of this function is to make the bot send a chat text that depends of one
   // of the 13 predefined situations : affirmative, negative, greetings, goodbye, taunt,
   // whine, idle, about following someone, stopping following someone, staying in position,
   // asking for backup, losing his master and replying to some idle chat on his own (HAL).

   int index, i;
   char msg[128];
   char msg_humanized[128];

   if (IsNull (pBot->pEntity))
      return; // reliability check

   // is it not time yet to speak ?
   if (pBot->BotChat.f_saytext_time > CurrentTime ())
      return; // return; don't speak yet

   // is bot chat forbidden ?
   if (GetServerVariable ("racc_chatmode") == 0)
      return; // return; bots are not allowed to chat

   // is it time to say affirmative ?
   if (pBot->BotChat.b_saytext_affirmative)
   {
      pBot->BotChat.b_saytext_affirmative = FALSE; // don't say it twice

      if (IsNull (pBot->BotEars.pAskingEntity))
         return; // reliability check

      // audio chat
      pBot->BotChat.b_sayaudio_affirmative = TRUE;
      pBot->BotChat.f_sayaudio_time = CurrentTime () + RandomFloat (0.5, 3.0);

      // text chat
      index = RandomInteger (0, pBot->pPersonality->text_affirmative_count - 1);

      if (strstr (pBot->pPersonality->text_affirmative[index], "%s") != NULL) // is "%s" in affirmative text ?
         sprintf (msg, pBot->pPersonality->text_affirmative[index], Name (NetnameOf (pBot->BotEars.pAskingEntity)));
      else
         sprintf (msg, pBot->pPersonality->text_affirmative[index]);

      strcpy (msg_humanized, HumanizeChat (msg)); // humanize the chat string
      FakeClientCommand (pBot->pEntity, "say_team %s\n", msg_humanized); // let bot say the chat string
      return;
   }

   // else is it time to say negative ?
   else if (pBot->BotChat.b_saytext_negative)
   {
      pBot->BotChat.b_saytext_negative = FALSE; // don't say it twice

      if (IsNull (pBot->BotEars.pAskingEntity))
         return; // reliability check

      // audio chat
      pBot->BotChat.b_sayaudio_negative = TRUE;
      pBot->BotChat.f_sayaudio_time = CurrentTime () + RandomFloat (0.5, 3.0);

      // text chat
      index = RandomInteger (0, pBot->pPersonality->text_negative_count - 1);

      if (strstr (pBot->pPersonality->text_negative[index], "%s") != NULL) // is "%s" in negative text ?
         sprintf (msg, pBot->pPersonality->text_negative[index], Name (NetnameOf (pBot->BotEars.pAskingEntity)));
      else
         sprintf (msg, pBot->pPersonality->text_negative[index]);

      strcpy (msg_humanized, HumanizeChat (msg)); // humanize the chat string
      FakeClientCommand (pBot->pEntity, "say_team %s\n", msg_humanized); // let bot say the chat string
      return;
   }

   // else is it time to say hello ?
   else if (pBot->BotChat.b_saytext_hello)
   {
      pBot->BotChat.b_saytext_hello = FALSE; // don't say it twice

      // text chat
      strcpy (msg_humanized, HumanizeChat (pBot->pPersonality->text_hello[RandomInteger (0, pBot->pPersonality->text_hello_count - 1)])); // humanize the chat string
      FakeClientCommand (pBot->pEntity, "say %s\n", msg_humanized); // let bot say the chat string
      return;
   }

   // else is it time to say goodbye ?
   else if (pBot->BotChat.b_saytext_bye)
   {
      pBot->BotChat.b_saytext_bye = FALSE; // don't say it twice

      // text chat
      strcpy (msg_humanized, HumanizeChat (pBot->pPersonality->text_bye[RandomInteger (0, pBot->pPersonality->text_bye_count - 1)])); // humanize the chat string
      FakeClientCommand (pBot->pEntity, "say %s\n", msg_humanized); // let bot say the chat string
      return;
   }

   // else is it time to laugh ?
   else if (pBot->BotChat.b_saytext_kill)
   {
      pBot->BotChat.b_saytext_kill = FALSE;

      if (IsNull (pBot->pVictimEntity))
         return; // reliability check

      // text chat
      index = RandomInteger (0, pBot->pPersonality->text_laugh_count - 1);

      if (strstr (pBot->pPersonality->text_laugh[index], "%s") != NULL) // is "%s" in laugh text ?
         sprintf (msg, pBot->pPersonality->text_laugh[index], Name (NetnameOf (pBot->pVictimEntity)));
      else
         sprintf (msg, pBot->pPersonality->text_laugh[index]);

      strcpy (msg_humanized, HumanizeChat (msg)); // humanize the chat string
      FakeClientCommand (pBot->pEntity, "say %s\n", msg_humanized); // let bot say the chat string
      return;
   }

   // else is it time to whine ?
   else if (pBot->BotChat.b_saytext_killed)
   {
      pBot->BotChat.b_saytext_killed = FALSE;

      if (IsNull (pBot->pKillerEntity))
         return; // reliability check

      // text chat
      index = RandomInteger (0, pBot->pPersonality->text_whine_count - 1);

      if (strstr (pBot->pPersonality->text_whine[index], "%s") != NULL) // is "%s" in whine text ?
         sprintf (msg, pBot->pPersonality->text_whine[index], Name (NetnameOf (pBot->pKillerEntity)));
      else
         sprintf (msg, pBot->pPersonality->text_whine[index]);

      strcpy (msg_humanized, HumanizeChat (msg)); // humanize the chat string
      FakeClientCommand (pBot->pEntity, "say %s\n", msg_humanized); // let bot say the chat string

      pBot->pKillerEntity = NULL; // bot can now forget his killer now
      return;
   }

   // else is it time to complain about loneliness ?
   else if (pBot->BotChat.b_saytext_idle)
   {
      bool found_one = FALSE;
      float bot_distance[MAX_CLIENTS_SUPPORTED_BY_ENGINE], farthest_bot_distance = 0.0;
      int farthest_bot_index;

      pBot->BotChat.b_saytext_idle = FALSE;

      // audio chat
      pBot->BotChat.b_sayaudio_report = TRUE;
      pBot->BotChat.f_sayaudio_time = CurrentTime () + RandomFloat (1.0, 5.0);

      // cycle all bots to find the farthest teammate
      for (i = 0; i < MaxClientsOnServer (); i++)
      {
         if (IsNull (bots[i].pEntity))
            continue; // reliability check

         // is this one ourself OR dead OR inactive ?
         if ((bots[i].pEntity == pBot->pEntity) || !IsAlive (bots[i].pEntity) || (!bots[i].is_active))
            continue; // if so, skip to the next bot

         // is this one NOT a teammate ?
         if (GetTeam (bots[i].pEntity) != GetTeam (pBot->pEntity))
            continue; // if so, skip to the next bot

         found_one = TRUE;

         // how far away is the bot?
         bot_distance[i] = (OriginOf (pBot->pEntity) - OriginOf (bots[i].pEntity)).Length ();

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
         bots[farthest_bot_index].BotChat.f_sayaudio_time = pBot->BotChat.f_sayaudio_time + RandomFloat (2.0, 3.0);
      }

      // text chat
      index = RandomInteger (0, pBot->pPersonality->text_idle_count - 1);

      if (strstr (pBot->pPersonality->text_idle[index], "%s") != NULL) // is "%s" in idle text ?
      {
         entity_t *pPlayer = pBot->pEntity; // reliability: fall back on a certified good edict
         i = 0; // incremental counter to avoid a possible endless loop, see below

         // pick up a random player, keep searching 100 times until one is found
         while (i++ < 100)
         {
            pPlayer = PlayerAtIndex (RandomInteger (0, MaxClientsOnServer () - 1)); // pick up a random slot

            if (IsNull (pPlayer) || (pPlayer == pBot->pEntity))
               continue; // skip invalid players and skip self (i.e. this bot)

            if (GetTeam (pPlayer) == GetTeam (pBot->pEntity))
               continue; // skip teammates
            else
               break; // this one is an enemy, so it's OK to taunt him
         }

         if (i < 100)
            sprintf (msg, pBot->pPersonality->text_idle[index], Name (NetnameOf (pPlayer)));
         else
            sprintf (msg, pBot->pPersonality->text_idle[index], ""); // aargh, no player found !!
      }
      else
         sprintf (msg, pBot->pPersonality->text_idle[index]);

      strcpy (msg_humanized, HumanizeChat (msg)); // humanize the chat string
      FakeClientCommand (pBot->pEntity, "say %s\n", msg_humanized); // let bot say the chat string
      return;
   }

   // else is it time to answer to a follow order ?
   else if (pBot->BotChat.b_saytext_follow)
   {
      pBot->BotChat.b_saytext_follow = FALSE;

      if (IsNull (pBot->BotEars.pAskingEntity))
         return; // reliability check

      // audio chat
      pBot->BotChat.b_sayaudio_affirmative = TRUE;
      pBot->BotChat.f_sayaudio_time = CurrentTime () + RandomFloat (0.0, 1.5);

      // text chat
      index = RandomInteger (0, pBot->pPersonality->text_follow_count - 1);

      if (strstr (pBot->pPersonality->text_follow[index], "%s") != NULL) // is "%s" in follow text ?
         sprintf (msg, pBot->pPersonality->text_follow[index], Name (NetnameOf (pBot->BotEars.pAskingEntity)));
      else
         sprintf (msg, pBot->pPersonality->text_follow[index]);

      strcpy (msg_humanized, HumanizeChat (msg)); // humanize the chat string
      FakeClientCommand (pBot->pEntity, "say_team %s\n", msg_humanized); // let bot say the chat string
      return;
   }

   // else is it time to answer to a stop order ?
   else if (pBot->BotChat.b_saytext_stop)
   {
      pBot->BotChat.b_saytext_stop = FALSE;

      if (IsNull (pBot->BotEars.pAskingEntity))
         return; // reliability check

      // audio chat
      pBot->BotChat.b_sayaudio_affirmative = TRUE;
      pBot->BotChat.f_sayaudio_time = CurrentTime () + RandomFloat (0.0, 1.5);

      // text chat
      index = RandomInteger (0, pBot->pPersonality->text_stop_count - 1);

      if (strstr (pBot->pPersonality->text_stop[index], "%s") != NULL) // is "%s" in stop text ?
         sprintf (msg, pBot->pPersonality->text_stop[index], Name (NetnameOf (pBot->BotEars.pAskingEntity)));
      else
         sprintf (msg, pBot->pPersonality->text_stop[index]);

      strcpy (msg_humanized, HumanizeChat (msg)); // humanize the chat string
      FakeClientCommand (pBot->pEntity, "say_team %s\n", msg_humanized); // let bot say the chat string
      return;
   }

   // else is it time to answer to a stay order ?
   else if (pBot->BotChat.b_saytext_stay)
   {
      pBot->BotChat.b_saytext_stay = FALSE;

      if (IsNull (pBot->BotEars.pAskingEntity))
         return; // reliability check

      // audio chat
      pBot->BotChat.b_sayaudio_inposition = TRUE;
      pBot->BotChat.f_sayaudio_time = CurrentTime () + RandomFloat (0.0, 1.5);

      // text chat
      index = RandomInteger (0, pBot->pPersonality->text_stay_count - 1);

      if (strstr (pBot->pPersonality->text_stay[index], "%s") != NULL) // is "%s" in stay text ?
         sprintf (msg, pBot->pPersonality->text_stay[index], Name (NetnameOf (pBot->BotEars.pAskingEntity)));
      else
         sprintf (msg, pBot->pPersonality->text_stay[index]);

      strcpy (msg_humanized, HumanizeChat (msg)); // humanize the chat string
      FakeClientCommand (pBot->pEntity, "say_team %s\n", msg_humanized); // let bot say the chat string
      return;
   }

   // else is it time to ask for backup ?
   else if (pBot->BotChat.b_saytext_help)
   {
      pBot->BotChat.b_saytext_help = FALSE; // don't say it twice

      // audio chat
      pBot->BotChat.b_sayaudio_takingdamage = TRUE;
      pBot->BotChat.f_sayaudio_time = CurrentTime () + RandomFloat (0.5, 2.0);

      // text chat
      strcpy (msg_humanized, HumanizeChat (pBot->pPersonality->text_help[RandomInteger (0, pBot->pPersonality->text_help_count - 1)])); // humanize the chat string
      FakeClientCommand (pBot->pEntity, "say_team %s\n", msg_humanized); // let bot say the chat string
      return;
   }

   // else is it time to say that bot is lost while used ?
   else if (pBot->BotChat.b_saytext_cant)
   {
      pBot->BotChat.b_saytext_cant = FALSE; // don't say it twice

      // audio chat
      pBot->BotChat.b_sayaudio_negative = TRUE;
      pBot->BotChat.f_sayaudio_time = CurrentTime () + RandomFloat (0.5, 1.5);

      // text chat
      index = RandomInteger (0, pBot->pPersonality->text_cant_count - 1);

      if (strstr (pBot->pPersonality->text_cant[index], "%s") != NULL)  // is "%s" in can't text ?
         sprintf (msg, pBot->pPersonality->text_cant[index], Name (NetnameOf (pBot->BotEars.pAskingEntity)));
      else
         sprintf (msg, pBot->pPersonality->text_cant[index]);

      strcpy (msg_humanized, HumanizeChat (msg)); // humanize the chat string
      FakeClientCommand (pBot->pEntity, "say_team %s\n", msg_humanized); // let bot say the chat string
      return;
   }

   // else is it time to answer something to idle chat ?
   else if (pBot->BotChat.b_saytext_halreply)
   {
      pBot->BotChat.b_saytext_halreply = FALSE; // don't say it twice

      strcpy (msg_humanized, HumanizeChat (pBot->BotChat.say_message)); // humanize the chat string
      FakeClientCommand (pBot->pEntity, "say %s\n", msg_humanized); // let bot say the chat string
      pBot->BotChat.say_message[0] = 0; // after the bot talked, reset the chat string
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

   char sound_path[256];
   int index;

   if (IsNull (pBot->pEntity))
      return; // reliability check

   // is voice chat forbidden ?
   if (GetServerVariable ("racc_voicechatmode") == 0)
      return; // if so, return

   // is bot dead ?
   if (!IsAlive (pBot->pEntity))
      return; // don't speak when dead

   // is it time to say affirmative ?
   if (pBot->BotChat.b_sayaudio_affirmative && (pBot->BotChat.f_sayaudio_time < CurrentTime ()))
   {
      pBot->BotChat.b_sayaudio_affirmative = FALSE; // don't speak twice
      if (pBot->pPersonality->audio_affirmative_count != -1)
      {
         index = RandomInteger (0, pBot->pPersonality->audio_affirmative_count); // pickup a random chat message
         sprintf (sound_path, "../../racc/profiles/%s/affirmative%d.wav", pBot->pPersonality->name, index);
         BotTalk (pBot, sound_path); // let the bot talk
      }
   }

   // else is it time to say alert ?
   else if (pBot->BotChat.b_sayaudio_alert && (pBot->BotChat.f_sayaudio_time < CurrentTime ()))
   {
      pBot->BotChat.b_sayaudio_alert = FALSE; // don't speak twice
      if (pBot->pPersonality->audio_alert_count != -1)
      {
         index = RandomInteger (0, pBot->pPersonality->audio_alert_count); // pickup a random chat message
         sprintf (sound_path, "../../racc/profiles/%s/alert%d.wav", pBot->pPersonality->name, index);
         BotTalk (pBot, sound_path); // let the bot talk
      }
   }

   // else is it time to say attacking ?
   else if (pBot->BotChat.b_sayaudio_attacking && (pBot->BotChat.f_sayaudio_time < CurrentTime ()))
   {
      pBot->BotChat.b_sayaudio_attacking = FALSE; // don't speak twice
      if (pBot->pPersonality->audio_attacking_count != -1)
      {
         index = RandomInteger (0, pBot->pPersonality->audio_attacking_count); // pickup a random chat message
         sprintf (sound_path, "../../racc/profiles/%s/attacking%d.wav", pBot->pPersonality->name, index);
         BotTalk (pBot, sound_path); // let the bot talk
      }
   }

   // else is it time to say firstspawn ?
   else if (pBot->BotChat.b_sayaudio_firstspawn && (pBot->BotChat.f_sayaudio_time < CurrentTime ()))
   {
      pBot->BotChat.b_sayaudio_firstspawn = FALSE; // don't speak twice
      if (pBot->pPersonality->audio_firstspawn_count != -1)
      {
         index = RandomInteger (0, pBot->pPersonality->audio_firstspawn_count); // pickup a random chat message
         sprintf (sound_path, "../../racc/profiles/%s/firstspawn%d.wav", pBot->pPersonality->name, index);
         BotTalk (pBot, sound_path); // let the bot talk
      }
   }

   // else is it time to say inposition ?
   else if (pBot->BotChat.b_sayaudio_inposition && (pBot->BotChat.f_sayaudio_time < CurrentTime ()))
   {
      pBot->BotChat.b_sayaudio_inposition = FALSE; // don't speak twice
      if (pBot->pPersonality->audio_inposition_count != -1)
      {
         index = RandomInteger (0, pBot->pPersonality->audio_inposition_count); // pickup a random chat message
         sprintf (sound_path, "../../racc/profiles/%s/inposition%d.wav", pBot->pPersonality->name, index);
         BotTalk (pBot, sound_path); // let the bot talk
      }
   }

   // else is it time to say negative ?
   else if (pBot->BotChat.b_sayaudio_negative && (pBot->BotChat.f_sayaudio_time < CurrentTime ()))
   {
      pBot->BotChat.b_sayaudio_negative = FALSE; // don't speak twice
      if (pBot->pPersonality->audio_negative_count != -1)
      {
         index = RandomInteger (0, pBot->pPersonality->audio_negative_count); // pickup a random chat message
         sprintf (sound_path, "../../racc/profiles/%s/negative%d.wav", pBot->pPersonality->name, index);
         BotTalk (pBot, sound_path); // let the bot talk
      }
   }

   // else is it time to say report ?
   else if (pBot->BotChat.b_sayaudio_report && (pBot->BotChat.f_sayaudio_time < CurrentTime ()))
   {
      pBot->BotChat.b_sayaudio_report = FALSE; // don't speak twice
      if (pBot->pPersonality->audio_report_count != -1)
      {
         index = RandomInteger (0, pBot->pPersonality->audio_report_count); // pickup a random chat message
         sprintf (sound_path, "../../racc/profiles/%s/report%d.wav", pBot->pPersonality->name, index);
         BotTalk (pBot, sound_path); // let the bot talk
      }
   }

   // else is it time to say reporting ?
   else if (pBot->BotChat.b_sayaudio_reporting && (pBot->BotChat.f_sayaudio_time < CurrentTime ()))
   {
      pBot->BotChat.b_sayaudio_reporting = FALSE; // don't speak twice
      if (pBot->pPersonality->audio_reporting_count != -1)
      {
         index = RandomInteger (0, pBot->pPersonality->audio_reporting_count); // pickup a random chat message
         sprintf (sound_path, "../../racc/profiles/%s/reporting%d.wav", pBot->pPersonality->name, index);
         BotTalk (pBot, sound_path); // let the bot talk
      }
   }

   // else is it time to say seegrenade ?
   else if (pBot->BotChat.b_sayaudio_seegrenade && (pBot->BotChat.f_sayaudio_time < CurrentTime ()))
   {
      pBot->BotChat.b_sayaudio_seegrenade = FALSE; // don't speak twice
      if (pBot->pPersonality->audio_seegrenade_count != -1)
      {
         index = RandomInteger (0, pBot->pPersonality->audio_seegrenade_count); // pickup a random chat message
         sprintf (sound_path, "../../racc/profiles/%s/seegrenade%d.wav", pBot->pPersonality->name, index);
         BotTalk (pBot, sound_path); // let the bot talk
      }
   }

   // else is it time to say takingdamage ?
   else if (pBot->BotChat.b_sayaudio_takingdamage && (pBot->BotChat.f_sayaudio_time < CurrentTime ()))
   {
      pBot->BotChat.b_sayaudio_takingdamage = FALSE; // don't speak twice
      if (pBot->pPersonality->audio_takingdamage_count != -1)
      {
         index = RandomInteger (0, pBot->pPersonality->audio_takingdamage_count); // pickup a random chat message
         sprintf (sound_path, "../../racc/profiles/%s/takingdamage%d.wav", pBot->pPersonality->name, index);
         BotTalk (pBot, sound_path); // let the bot talk
      }
   }

   // else is it time to say throwgrenade ?
   else if (pBot->BotChat.b_sayaudio_throwgrenade && (pBot->BotChat.f_sayaudio_time < CurrentTime ()))
   {
      pBot->BotChat.b_sayaudio_throwgrenade = FALSE; // don't speak twice
      if (pBot->pPersonality->audio_throwgrenade_count != -1)
      {
         index = RandomInteger (0, pBot->pPersonality->audio_throwgrenade_count); // pickup a random chat message
         sprintf (sound_path, "../../racc/profiles/%s/throwgrenade%d.wav", pBot->pPersonality->name, index);
         BotTalk (pBot, sound_path); // let the bot talk
      }
   }

   // else is it time to say victory ?
   else if (pBot->BotChat.b_sayaudio_victory && (pBot->BotChat.f_sayaudio_time < CurrentTime ()))
   {
      pBot->BotChat.b_sayaudio_victory = FALSE; // don't speak twice
      if (pBot->pPersonality->audio_victory_count != -1)
      {
         index = RandomInteger (0, pBot->pPersonality->audio_victory_count); // pickup a random chat message
         sprintf (sound_path, "../../racc/profiles/%s/victory%d.wav", pBot->pPersonality->name, index);
         BotTalk (pBot, sound_path); // let the bot talk
      }
   }
}


void BotTalk (bot_t *pBot, char *sound_path)
{
   int client_index;

   if (IsNull (pBot->pEntity))
      return; // reliability check

   if (GetServerVariable ("racc_voicechatmode") == 0)
      return; // cancel if voice chat is disabled

   // cycle through all clients
   for (client_index = 0; client_index < MaxClientsOnServer (); client_index++)
   {
      entity_t *pPlayer = PlayerAtIndex (client_index); // get entity of client index

      if (IsNull (pPlayer) || (pPlayer == pBot->pEntity))
         continue; // skip invalid players and skip self (i.e. this bot)

      if (!IsAlive (pPlayer) || !IsAPlayer (pPlayer))
         continue; // skip this player if dead or dying or not a real client

      // check if it is a teammate; if so, talk "in his head"
      if (GetTeam (pPlayer) == GetTeam (pBot->pEntity))
      {
         DestroySpeakerIcon (pBot->pEntity, pPlayer); // reset any previously displayed speaker on this bot
         sprintf (g_argv, "play %s\n", sound_path);
         pfnClientCommand (pPlayer, g_argv); // play bot's talk on client side
         DisplaySpeakerIcon (pBot->pEntity, pPlayer, RandomInteger (8, 22)); // display speaker icon
      }
   }

   return;
}


const char *Name (const char *string)
{
   // assuming string is a player name, this function returns a random equivalent in a
   // "humanized" form, i.e. without the tag marks, without trailing spaces, and optionally
   // turned into lower case.

   int index, length;
   static char buffer[32];

   // drop the tag marks when speaking to someone 75 percent of time
   if (RandomInteger (1, 100) < 75)
      strcpy (buffer, StripTags (string));
   else
      strcpy (buffer, StripBlanks (string));

   length = strlen (buffer); // get name buffer's length

   // half the time switch the name to lower characters
   if (RandomInteger (1, 100) < 50)
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
   if (RandomInteger (1, 100) < 33)
   {
      for (index = 0; index < length; index++)
         buffer[index] = tolower (buffer[index]); // switch buffer to lowercase
   }

   // if length is sufficient to assume the text had to be typed in a hurry
   if (length > 15)
   {
      // "length" percent of time drop a character
      if (RandomInteger (1, 100) < length)
      {
         pos = RandomInteger ((int) (length / 8), length - (int) (length / 8)); // choose a random position in string

         for (index = pos; index < length - 1; index++)
            buffer[index] = buffer[index + 1]; // overwrite the buffer with the stripped string
         buffer[index] = 0; // terminate the string
         length--; // update new string length
      }

      // "length" / 2 percent of time swap a character
      if (RandomInteger (1, 100) < length / 2)
      {
         char tempchar;
         pos = RandomInteger ((int) (length / 8), (int) (3 * length / 8)); // choose a random position in string

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


void UpperCase (char *string)
{
   // this function converts a string to its uppercase representation

   register int i;

   for (i = 0; i < (int) strlen (string); i++)
      string[i] = (char) toupper ((int) string[i]);
}


void LowerCase (char *string)
{
   // this function converts a string to its lowercase representation

   register int i;

   for (i = 0; i < (int) strlen (string); i++)
      string[i] = (char) tolower ((int) string[i]);
}


void BotHALGenerateReply (bot_t *pBot, char *output)
{
   // this function takes a string of user input and return a string of output which may
   // vaguely be construed as containing a reply to whatever is in the input string.
   // Create an array of keywords from the words in the user's input...

   HAL_DICTIONARY *keywords, *replywords;
   static char *output_template = NULL;
   int tries_count, last_entry, last_character, length = 1;
   register int i, j;


   if (IsNull (pBot->pEntity))
      return; // reliability check

   if (output_template == NULL)
   {
      output_template = (char *) malloc (sizeof (char));
      if (output_template == NULL)
         TerminateOnError ("HAL: BotHALGenerateReply() unable to allocate output\n");
   }

   output_template[0] = 0; // first reset the reply string

   keywords = BotHALMakeKeywords (pBot, pBot->pPersonality->input_words);
   replywords = BotHALBuildReplyDictionary (pBot, keywords);

   last_entry = pBot->pPersonality->input_words->size - 1;
   last_character = pBot->pPersonality->input_words->entry[last_entry].length - 1;

   // was it a question (i.e. was the last word in the general chat record a question mark ?)
   if (pBot->pPersonality->input_words->entry[last_entry].word[last_character] == '?')
   {
      // try ten times to answer something relevant
      for (tries_count = 0; tries_count < 10; tries_count++)
      {
         if (HAL_DictionariesDiffer (pBot->pPersonality->input_words, replywords))
            break; // stop as soon as we've got something to say
         else
            replywords = BotHALBuildReplyDictionary (pBot, keywords); // else think again
      }

      // if we've finally found something to say, generate the reply
      if (tries_count < 10)
      {
         // if no words in the reply dictionary...
         if (replywords->size == 0)
         {
            sprintf (output, "mommy mommy!"); // then copy a "newborn" answer
            return; // the HAL is too young to talk yet
         }

         for (i = 0; i < (int) replywords->size; ++i)
            length += replywords->entry[i].length;

         output_template = (char *) realloc (output_template, sizeof (char) * length);
         if (output_template == NULL)
            TerminateOnError ("HAL: HAL_MakeOutput() unable to reallocate output\n");

         length = 0;

         for (i = 0; i < (int) replywords->size; ++i)
            for (j = 0; j < replywords->entry[i].length; ++j)
               output_template[length++] = replywords->entry[i].word[j];

         if (length > 127)
            output_template[127] = 0; // disallow strings to be longer than 128 chars
         else
            output_template[length] = 0; // terminate the string

         sprintf (output, output_template); // then copy the answer
         return;
      }
   }

   // else if we are not paraphrazing, generate a string from the dictionary of reply words
   else if (HAL_DictionariesDiffer (pBot->pPersonality->input_words, replywords))
   {
      // if no words in the reply dictionary...
      if (replywords->size == 0)
      {
         sprintf (output, "mommy mommy!"); // then copy a "newborn" answer
         return; // the HAL is too young to talk yet
      }

      for (i = 0; i < (int) replywords->size; ++i)
         length += replywords->entry[i].length;

      output_template = (char *) realloc (output_template, sizeof (char) * length);
      if (output_template == NULL)
         TerminateOnError ("HAL: HAL_MakeOutput() unable to reallocate output\n");

      length = 0;

      for (i = 0; i < (int) replywords->size; ++i)
         for (j = 0; j < replywords->entry[i].length; ++j)
            output_template[length++] = replywords->entry[i].word[j];

      if (length > 127)
         output_template[127] = 0; // disallow strings to be longer than 128 chars
      else
         output_template[length] = 0; // terminate the string

      sprintf (output, output_template); // then copy the answer
      return;
   }
}


unsigned short HAL_AddWord (HAL_DICTIONARY *dictionary, HAL_STRING word)
{
   // this function adds a word to a dictionary, and return the identifier assigned to the
   // word. If the word already exists in the dictionary, then return its current identifier
   // without adding it again.

   register int i;
   int position;
   bool found;

   // if the word's already in the dictionary, there is no need to add it
   position = HAL_SearchDictionary (dictionary, word, &found);
   if (found)
      return (dictionary->index[position]);

   // increase the number of words in the dictionary
   dictionary->size++;

   // allocate one more entry for the word index
   if (dictionary->index == NULL)
      dictionary->index = (unsigned short *) malloc (sizeof (unsigned short) * dictionary->size);
   else
      dictionary->index = (unsigned short *) realloc ((unsigned short *) dictionary->index, sizeof (unsigned short) * dictionary->size);

   if (dictionary->index == NULL)
      TerminateOnError ("HAL: HAL_AddWord() unable to reallocate the dictionary index\n");

   // allocate one more entry for the word array
   if (dictionary->entry == NULL)
      dictionary->entry = (HAL_STRING *) malloc (sizeof (HAL_STRING) * dictionary->size);
   else
      dictionary->entry = (HAL_STRING *) realloc ((HAL_STRING *) dictionary->entry, sizeof (HAL_STRING) * dictionary->size);

   if (dictionary->entry == NULL)
      TerminateOnError ("HAL: HAL_AddWord() unable to reallocate the dictionary to %d elements\n", dictionary->size);

   // copy the new word into the word array
   dictionary->entry[dictionary->size - 1].length = word.length;
   dictionary->entry[dictionary->size - 1].word = (char *) malloc (sizeof (char) * word.length);
   if (dictionary->entry[dictionary->size - 1].word == NULL)
      TerminateOnError ("HAL: HAL_AddWord() unable to allocate the word\n");

   for (i = 0; i < word.length; ++i)
      dictionary->entry[dictionary->size - 1].word[i] = word.word[i];

   // shuffle the word index to keep it sorted alphabetically
   for (i = dictionary->size - 1; i > position; --i)
      dictionary->index[i] = dictionary->index[i - 1];

   // copy the new symbol identifier into the word index
   dictionary->index[position] = (unsigned short) dictionary->size - 1;

   return (dictionary->index[position]);
}


int HAL_SearchDictionary (HAL_DICTIONARY *dictionary, HAL_STRING word, bool *find)
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
      *find = FALSE;
      return (0);
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
         *find = TRUE;
         return (middle);
      }
      else if (compar > 0)
      {
         if (max == middle)
         {
            *find = FALSE;
            return (middle + 1);
         }

         min = middle + 1;
      }
      else
      {
         if (min == middle)
         {
            *find = FALSE;
            return (middle);
         }

         max = middle - 1;
      }
   }
}


unsigned short HAL_FindWord (HAL_DICTIONARY *dictionary, HAL_STRING word)
{
   // this function returns the symbol corresponding to the word specified. We assume that
   // the word with index zero is equal to a NULL word, indicating an error condition.

   bool found;
   int position = HAL_SearchDictionary (dictionary, word, &found);

   if (found == TRUE)
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

   HAL_STRING word = { 7, " < ERROR > " };
   HAL_STRING end = { 5, " < FIN > " };

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


void HAL_LoadDictionary (FILE *file, HAL_DICTIONARY *dictionary)
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
         TerminateOnError ("HAL: HAL_LoadDictionary() unable to allocate word\n");

      for (j = 0; j < word.length; ++j)
         mfread (&word.word[j], sizeof (char), 1, file);

      HAL_AddWord (dictionary, word);
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


HAL_MODEL *HAL_NewModel (int order)
{
   // this function creates and initializes a new ngram model

   HAL_MODEL *model = NULL;

   model = (HAL_MODEL *) malloc (sizeof (HAL_MODEL));
   if (model == NULL)
      TerminateOnError ("HAL: HAL_NewModel() unable to allocate model\n");

   model->order = (unsigned char) order;
   model->forward = HAL_NewNode ();
   model->backward = HAL_NewNode ();
   model->context = (HAL_TREE **) malloc (sizeof (HAL_TREE *) * (order + 2));
   if (model->context == NULL)
      TerminateOnError ("HAL: HAL_NewModel() unable to allocate context array\n");

   HAL_InitializeContext (model);
   model->dictionary = HAL_NewDictionary ();
   HAL_InitializeDictionary (model->dictionary);

   return (model);
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
   bool found_symbol = FALSE;

   // perform a binary search for the symbol
   i = HAL_SearchNode (node, symbol, &found_symbol);
   if (found_symbol)
      found = node->tree[i];

   return (found);
}


HAL_TREE *HAL_FindSymbolAdd (HAL_TREE *node, int symbol)
{
   // this function is conceptually similar to HAL_FindSymbol, apart from the fact that if the
   // symbol is not found, a new node is automatically allocated and added to the tree

   register int i;
   HAL_TREE *found = NULL;
   bool found_symbol = FALSE;

   // perform a binary search for the symbol. If the symbol isn't found, attach a new sub-node
   // to the tree node so that it remains sorted.
   i = HAL_SearchNode (node, symbol, &found_symbol);

   if (found_symbol)
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


int HAL_SearchNode (HAL_TREE *node, int symbol, bool *found_symbol)
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
      *found_symbol = FALSE;
      return (0);
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
         *found_symbol = TRUE;
         return (middle);
      }
      else if (compar > 0)
      {
         if (max == middle)
         {
            *found_symbol = FALSE;
            return (middle + 1);
         }

         min = middle + 1;
      }
      else
      {
         if (min == middle)
         {
            *found_symbol = FALSE;
            return (middle);
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


void HAL_LoadTree (FILE *file, HAL_TREE *node)
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
            TerminateOnError ("HAL: HAL_MakeWords() unable to reallocate dictionary\n");

         words->entry[words->size].length = (unsigned char) offset;
         words->entry[words->size].word = input;
         words->size += 1;

         if (offset == (int) strlen (input))
            break;

         input += offset;
         offset = 0;
      }
      else
         ++offset;
   }

   // if the last word isn't punctuation, then replace it with a full-stop character
   if (isalnum (words->entry[words->size - 1].word[0]))
   {
      if (words->entry == NULL)
         words->entry = (HAL_STRING *) malloc ((words->size + 1) * sizeof (HAL_STRING));
      else
         words->entry = (HAL_STRING *) realloc (words->entry, (words->size + 1) * sizeof (HAL_STRING));

      if (words->entry == NULL)
         TerminateOnError ("HAL: HAL_MakeWords() unable to reallocate dictionary\n");

      words->entry[words->size].length = 1;
      words->entry[words->size].word = ".";
      ++words->size;
   }

   else if (strchr ("!.?", words->entry[words->size - 1].word[words->entry[words->size - 1].length - 1]) == NULL)
   {
      words->entry[words->size - 1].length = 1;
      words->entry[words->size - 1].word = ".";
   }

   return;
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

   static HAL_DICTIONARY *keys = NULL;
   register int i;
   register int j;
   int c;

   if (IsNull (pBot->pEntity))
      return (NULL); // reliability check

   if (keys == NULL)
      keys = HAL_NewDictionary ();

   for (i = 0; i < (int) keys->size; ++i)
      free (keys->entry[i].word);

   HAL_EmptyDictionary (keys);

   for (i = 0; i < (int) words->size; ++i)
   {
      // find the symbol ID of the word. If it doesn't exist in the model, or if it begins
      // with a non-alphanumeric character, or if it is in the exclusion array, then skip it

      c = 0;

      for (j = 0; j < pBot->pPersonality->swappable_keywords->size; ++j)
         if (HAL_CompareWords (pBot->pPersonality->swappable_keywords->from[j], words->entry[i]) == 0)
         {
            BotHALAddKeyword (pBot, keys, pBot->pPersonality->swappable_keywords->to[j]);
            ++c;
         }

      if (c == 0)
         BotHALAddKeyword (pBot, keys, words->entry[i]);
   }

   if (keys->size > 0)
      for (i = 0; i < (int) words->size; ++i)
      {
         c = 0;

         for (j = 0; j < pBot->pPersonality->swappable_keywords->size; ++j)
            if (HAL_CompareWords (pBot->pPersonality->swappable_keywords->from[j], words->entry[i]) == 0)
            {
               BotHALAddAuxiliaryKeyword (pBot, keys, pBot->pPersonality->swappable_keywords->to[j]);
               ++c;
            }

         if (c == 0)
            BotHALAddAuxiliaryKeyword (pBot, keys, words->entry[i]);
      }

   return (keys);
}


void BotHALAddKeyword (bot_t *pBot, HAL_DICTIONARY *keys, HAL_STRING word)
{
   // this function adds a word to the keyword dictionary

   int symbol;
   
   if (IsNull (pBot->pEntity))
      return; // reliability check

   symbol = HAL_FindWord (pBot->pPersonality->bot_model->dictionary, word);

   if (symbol == 0)
      return; // if void, return

   if (isalnum (word.word[0]) == 0)
      return; // if not alphanumeric, return

   symbol = HAL_FindWord (pBot->pPersonality->banned_keywords, word); // is this word in the banned dictionary ?
   if (symbol != 0)
      return; // if so, return

   symbol = HAL_FindWord (pBot->pPersonality->auxiliary_keywords, word); // is this word in the auxiliary dictionary ?
   if (symbol != 0)
      return; // if so, return

   HAL_AddWord (keys, word); // once we are sure this word isn't known yet, we can add it
}


void BotHALAddAuxiliaryKeyword (bot_t *pBot, HAL_DICTIONARY *keys, HAL_STRING word)
{
   // this function adds an auxilliary keyword to the keyword dictionary

   int symbol;

   if (IsNull (pBot->pEntity))
      return; // reliability check

   symbol = HAL_FindWord (pBot->pPersonality->bot_model->dictionary, word);

   if (symbol == 0)
      return; // if void, return

   if (isalnum (word.word[0]) == 0)
      return; // if not alphanumeric, return

   symbol = HAL_FindWord (pBot->pPersonality->auxiliary_keywords, word); // is it already in the dictionary ?
   if (symbol == 0)
      return; // if already in dictionary, return

   HAL_AddWord (keys, word); // add this word to the keywords dictionary
}


HAL_DICTIONARY *BotHALBuildReplyDictionary (bot_t *pBot, HAL_DICTIONARY *keys)
{
   // this function generates a dictionary of reply words relevant to the dictionary of keywords

   static HAL_DICTIONARY *replies = NULL;
   register int i;
   int symbol;
   bool start = TRUE;

   if (IsNull (pBot->pEntity))
      return (NULL); // reliability check

   if (replies == NULL)
      replies = HAL_NewDictionary ();

   HAL_EmptyDictionary (replies);

   // start off by making sure that the model's context is empty
   HAL_InitializeContext (pBot->pPersonality->bot_model);
   pBot->pPersonality->bot_model->context[0] = pBot->pPersonality->bot_model->forward;
   pBot->pPersonality->keyword_is_used = FALSE;

   // generate the reply in the forward direction
   while (TRUE)
   {
      // get a random symbol from the current context
      if (start == TRUE)
         symbol = BotHALSeedReply (pBot, keys);
      else
         symbol = BotHALBabble (pBot, keys, replies);

      if ((symbol == 0) || (symbol == 1))
         break;

      start = FALSE;

      // append the symbol to the reply dictionary
      if (replies->entry == NULL)
         replies->entry = (HAL_STRING *) malloc ((replies->size + 1) * sizeof (HAL_STRING));
      else
         replies->entry = (HAL_STRING *) realloc (replies->entry, (replies->size + 1) * sizeof (HAL_STRING));

      if (replies->entry == NULL)
         TerminateOnError ("HAL: BotHALBuildReplyDictionary() unable to reallocate dictionary\n");

      replies->entry[replies->size].length = pBot->pPersonality->bot_model->dictionary->entry[symbol].length;
      replies->entry[replies->size].word = pBot->pPersonality->bot_model->dictionary->entry[symbol].word;
      replies->size++;

      // extend the current context of the model with the current symbol
      HAL_UpdateContext (pBot->pPersonality->bot_model, symbol);
   }

   // start off by making sure that the model's context is empty
   HAL_InitializeContext (pBot->pPersonality->bot_model);
   pBot->pPersonality->bot_model->context[0] = pBot->pPersonality->bot_model->backward;

   // re-create the context of the model from the current reply dictionary so that we can
   // generate backwards to reach the beginning of the string.
   if (replies->size > 0)
      for (i = min (replies->size - 1, pBot->pPersonality->bot_model->order); i >= 0; --i)
      {
         symbol = HAL_FindWord (pBot->pPersonality->bot_model->dictionary, replies->entry[i]);
         HAL_UpdateContext (pBot->pPersonality->bot_model, symbol);
      }

   // generate the reply in the backward direction
   while (TRUE)
   {
      // get a random symbol from the current context
      symbol = BotHALBabble (pBot, keys, replies);
      if ((symbol == 0) || (symbol == 1))
         break;

      // prepend the symbol to the reply dictionary
      if (replies->entry == NULL)
         replies->entry = (HAL_STRING *) malloc ((replies->size + 1) * sizeof (HAL_STRING));
      else
         replies->entry = (HAL_STRING *) realloc (replies->entry, (replies->size + 1) * sizeof (HAL_STRING));

      if (replies->entry == NULL)
         TerminateOnError ("HAL: BotHALBuildReplyDictionary() unable to reallocate dictionary\n");

      // shuffle everything up for the prepend
      for (i = replies->size; i > 0; --i)
      {
         replies->entry[i].length = replies->entry[i - 1].length;
         replies->entry[i].word = replies->entry[i - 1].word;
      }

      replies->entry[0].length = pBot->pPersonality->bot_model->dictionary->entry[symbol].length;
      replies->entry[0].word = pBot->pPersonality->bot_model->dictionary->entry[symbol].word;
      replies->size++;

      // extend the current context of the model with the current symbol
      HAL_UpdateContext (pBot->pPersonality->bot_model, symbol);
   }

   return (replies);
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

   if (IsNull (pBot->pEntity))
      return (0); // reliability check

   // select the longest available context
   for (i = 0; i <= pBot->pPersonality->bot_model->order; ++i)
      if (pBot->pPersonality->bot_model->context[i] != NULL)
         node = pBot->pPersonality->bot_model->context[i];

   if (node->branch == 0)
      return (0);

   // choose a symbol at random from this context
   i = RandomInteger (0, node->branch - 1);
   count = RandomInteger (0, node->usage - 1);

   while (count >= 0)
   {
      // if the symbol occurs as a keyword, then use it. Only use an auxilliary keyword if
      // a normal keyword has already been used.
      symbol = node->tree[i]->symbol;

      if ((HAL_FindWord (keys, pBot->pPersonality->bot_model->dictionary->entry[symbol]) != 0)
          && (pBot->pPersonality->keyword_is_used || (HAL_FindWord (pBot->pPersonality->auxiliary_keywords, pBot->pPersonality->bot_model->dictionary->entry[symbol]) == 0))
          && !HAL_WordExists (words, pBot->pPersonality->bot_model->dictionary->entry[symbol]))
      {
         pBot->pPersonality->keyword_is_used = TRUE;
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

   if (IsNull (pBot->pEntity))
      return (0); // reliability check

   // be aware of the special case where the tree is empty
   if (pBot->pPersonality->bot_model->context[0]->branch == 0)
      symbol = 0;
   else
      symbol = pBot->pPersonality->bot_model->context[0]->tree[RandomInteger (0, pBot->pPersonality->bot_model->context[0]->branch - 1)]->symbol;

   if (keys->size > 0)
   {
      i = RandomInteger (0, keys->size - 1);
      stop = i;

      while (TRUE)
      {
         if ((HAL_FindWord (pBot->pPersonality->bot_model->dictionary, keys->entry[i]) != 0) && (HAL_FindWord (pBot->pPersonality->auxiliary_keywords, keys->entry[i]) == 0))
         {
            symbol = HAL_FindWord (pBot->pPersonality->bot_model->dictionary, keys->entry[i]);
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
   FILE *fp;
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
         continue;

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
   FILE *fp;
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
   if (model->dictionary != NULL)
   {
      HAL_EmptyDictionary (model->dictionary);
      free (model->dictionary);
   }

   free (model);
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

   free (tree);
}


void HAL_FreeSwap (HAL_SWAP *swap)
{
   // this function frees the memory space used in a swap structure

   register int i;

   if (swap == NULL)
      return; // if already freed, return

   // for each element of the swap structure...
   for (i = 0; i < swap->size; ++i)
   {
      free (swap->from[i].word); // free the "from" word
      free (swap->to[i].word); // free the "to" word
   }

   free (swap->from); // free the "from" array
   free (swap->to); // free the "to" array
   free (swap); // free the swap structure itself
}


void PrepareHALBrainForPersonality (bot_personality_t *personality)
{
   // this function prepares a HAL personality

   FILE *fp;
   char ban_filename[256];
   char aux_filename[256];
   char swp_filename[256];
   char brn_filename[256];
   char cookie[32];
   bool error = FALSE;

   if (personality == NULL)
      return; // reliability check

   personality->bot_model = HAL_NewModel (BOT_HAL_MODEL_ORDER); // create a language model of a certain order

   // build the file names
   sprintf (ban_filename, "racc/profiles/%s/%s/%s/racc-hal.ban", personality->name, mod_name, map_name);
   sprintf (aux_filename, "racc/profiles/%s/%s/%s/racc-hal.aux", personality->name, mod_name, map_name);
   sprintf (swp_filename, "racc/profiles/%s/%s/%s/racc-hal.swp", personality->name, mod_name, map_name);
   sprintf (brn_filename, "racc/profiles/%s/%s/%s/racc-hal.brn", personality->name, mod_name, map_name);

   // read dictionaries containing banned keywords, auxiliary keywords and swap keywords
   personality->banned_keywords = HAL_InitializeList (ban_filename);
   personality->auxiliary_keywords = HAL_InitializeList (aux_filename);
   personality->swappable_keywords = HAL_InitializeSwap (swp_filename);

   // check if the brain exists, try to open it
   fp = mfopen (brn_filename, "rb");
   if (fp != NULL)
   {
      mfseek (fp, 0, SEEK_SET); // seek at start of file
      mfread (cookie, sizeof ("RACCHAL"), 1, fp); // read the brain signature
      mfclose (fp); // close the brain (we just wanted the signature)

      // check for brain file validity
      if (strcmp (cookie, "RACCHAL") == 0)
         return; // ok, brain is valid
   }

   // there is a problem with the brain, infer a brand new one
   ServerConsole_printf ("RACC: %s's HAL brain damaged!\n", personality->name);

   if (DeveloperMode () >= DEVELOPER_VERBOSE)
      ServerConsole_printf ("RACC: inferring a new HAL brain to %s\n", personality->name);

   // create the new brain (i.e, save a void one in the brain file)
   fp = fopen (brn_filename, "wb");
   if (fp == NULL)
      TerminateOnError ("PrepareHALBrainForPersonality(): %s's HAL brain refuses surgery!\n", personality->name);

   fwrite ("RACCHAL", sizeof ("RACCHAL"), 1, fp);
   fwrite (&personality->bot_model->order, sizeof (unsigned char), 1, fp);
   HAL_SaveTree (fp, personality->bot_model->forward);
   HAL_SaveTree (fp, personality->bot_model->backward);
   HAL_SaveDictionary (fp, personality->bot_model->dictionary);
   fclose (fp); // everything is saved, close the file

   return; // ok, now it is guarantee that this personality has an associated brain
}


bool LoadHALBrainForPersonality (bot_personality_t *personality)
{
   // this function loads a HAL brain

   FILE *fp;
   char filename[256];
   char cookie[8];

   // build the file name
   sprintf (filename, "racc/profiles/%s/%s/%s/racc-hal.brn", personality->name, mod_name, map_name);

   // check if the brain exists, try to open it
   fp = mfopen (filename, "rb");
   if (fp == NULL)
   {
      LogToFile ("LoadHALBrainForPersonality(): %s's HAL brain refuses to wake up!\n", personality->name);
      return (TRUE); // there was an error, return TRUE
   }

   // check for brain file validity
   mfread (cookie, sizeof ("RACCHAL"), 1, fp); // read the brain signature
   if (strcmp (cookie, "RACCHAL") != 0)
   {
      LogToFile ("LoadHALBrainForPersonality(): %s's HAL brain damaged!\n", personality->name); // bad brain
      mfclose (fp); // close file
      return (TRUE); // there was an error, return TRUE
   }

   if (DeveloperMode () == DEVELOPER_VERBOSE)
      ServerConsole_printf ("RACC HAL: restoring brain to %s\n", personality->name);

   mfread (&personality->bot_model->order, 1, 1, fp);
   HAL_LoadTree (fp, personality->bot_model->forward);
   HAL_LoadTree (fp, personality->bot_model->backward);
   HAL_LoadDictionary (fp, personality->bot_model->dictionary);
   mfclose (fp);

   personality->input_words = HAL_NewDictionary (); // create the global chat dictionary
   return (FALSE); // no error, return FALSE
}


void SaveHALBrainForPersonality (bot_personality_t *personality)
{
   // this function saves the current state to a HAL brain file

   FILE *fp;
   char filename[256];

   // build the file names
   sprintf (filename, "racc/profiles/%s/%s/%s/racc-hal.brn", personality->name, mod_name, map_name);

   fp = fopen (filename, "wb");
   if (fp == NULL)
   {
      ServerConsole_printf ("RACC: Unable to save %s's HAL brain to %s\n", personality->name, filename);
      return;
   }

   // dump the HAL brain to disk
   fwrite ("RACCHAL", sizeof ("RACCHAL"), 1, fp);
   fwrite (&personality->bot_model->order, sizeof (unsigned char), 1, fp);
   HAL_SaveTree (fp, personality->bot_model->forward);
   HAL_SaveTree (fp, personality->bot_model->backward);
   HAL_SaveDictionary (fp, personality->bot_model->dictionary);

   fclose (fp);
}
