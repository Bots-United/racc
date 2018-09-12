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
// CSTRIKE version
//
// bot_client.cpp
//

#include "racc.h"


void BotClient_CS_VGUIMenu (void *p, int bot_index)
{
   // this message tells a game client to display a VGUI menu

   // only take the first packet (only want the menu code)
   if (messagestate == 0)
   {
      if ((*(int *) p) == 2) // is it a team select menu ?
         bots[bot_index].BotEyes.BotHUD.menu_state = MENU_CS_TEAMSELECT_MAINMENU;
      else if ((*(int *) p) == 26) // is is the terrorist model select menu ?
         bots[bot_index].BotEyes.BotHUD.menu_state = MENU_CS_TEAMSELECT_TERRMENU;
      else if ((*(int *) p) == 27) // is is the counter-terrorist model select menu ?
         bots[bot_index].BotEyes.BotHUD.menu_state = MENU_CS_TEAMSELECT_COUNTERMENU;
   }
}


void BotClient_CS_ShowMenu (void *p, int bot_index)
{
   // this message tells a game client to display a HUD text menu

   // only take the fifth packet (only want menu string)
   if (messagestate == 4)
   {
      if (strcmp ((char *) p, "#Team_Select") == 0) // is is the team select menu ?
         bots[bot_index].BotEyes.BotHUD.menu_state = MENU_CS_TEAMSELECT_MAINMENU;
      else if (strcmp ((char *) p, "#Terrorist_Select") == 0) // is is the T model select ?
         bots[bot_index].BotEyes.BotHUD.menu_state = MENU_CS_TEAMSELECT_TERRMENU;
      else if (strcmp ((char *) p, "#CT_Select") == 0) // is is the CT model select menu ?
         bots[bot_index].BotEyes.BotHUD.menu_state = MENU_CS_TEAMSELECT_COUNTERMENU;
   }
}


void BotClient_CS_WeaponList (void *p, int bot_index)
{
   // This message is sent when a client joins the game. All the weapons data are sent to him
   // with the weapon ID, information about what type of ammo is used, and the maximal amount
   // of ammo for each rail of the weapon. We hook it to fill in some parts of our weapon
   // database, since this is the most reliable data about weapons we can get.

   static weapon_t weapon, *corresponding_weapon;

   if (messagestate == 0)
      strcpy (weapon.classname, (char *) p);
   else if (messagestate == 1)
      weapon.primary.type_of_ammo = *(int *) p; // primary rail type of ammo (index in ammo array)
   else if (messagestate == 2)
      weapon.primary.max_ammo = *(int *) p; // maximum amount of ammo the primary rail can hold
   else if (messagestate == 3)
      weapon.secondary.type_of_ammo = *(int *) p; // secondary rail type of ammo (index in ammo array)
   else if (messagestate == 4)
      weapon.secondary.max_ammo = *(int *) p; // maximum amount of ammo the secondary rail can hold
   else if (messagestate == 5)
      weapon.slot = *(int *) p; // this weapon's slot in the player's HUD
   else if (messagestate == 6)
      weapon.position = *(int *) p; // this weapon's position in the slot
   else if (messagestate == 7)
      weapon.id = *(int *) p; // the weapon ID itself
   else if (messagestate == 8)
   {
      weapon.flags = *(int *) p; // weapon flags (now what the hell is that ?)

      // finished, so update the weapons database with this weapon and its information
      corresponding_weapon = FindWeaponByName (weapon.classname);

      // has the weapon NOT been found in the database ?
      if (corresponding_weapon->classname[0] == 0)
      {
         // then it's a new weapon, that hasn't been declared in the config file !!!
         ServerConsole_printf ("RACC: ALERT: Engine is telling us about a weapon unknown in database !\n");
         corresponding_weapon = &weapons[weapon_count]; // so prepare a free slot in the database
         weapon_count++; // we know now one weapon more
      }

      // now store this weapon information
      strcpy (corresponding_weapon->classname, weapon.classname);
      corresponding_weapon->id = weapon.id;
      corresponding_weapon->slot = weapon.slot;
      corresponding_weapon->position = weapon.position;
      corresponding_weapon->flags = weapon.flags;
      corresponding_weapon->primary.type_of_ammo = weapon.primary.type_of_ammo;
      corresponding_weapon->primary.max_ammo = weapon.primary.max_ammo;
      corresponding_weapon->secondary.type_of_ammo = weapon.secondary.type_of_ammo;
      corresponding_weapon->secondary.max_ammo = weapon.secondary.max_ammo;
   }
}


void BotClient_CS_CurWeapon (void *p, int bot_index)
{
   // This message is sent when a game client just selected one weapon. The server updates him
   // with the amount of ammo it knows it has and the state of the weapon, to ensure the client
   // will use the same ammo amount that the server knows he has. It is also sent to clients when
   // the server auto assigns them a weapon. It seems also to be sent when the weapon is fired,
   // causing the amount of ammo currently in clip to decrease, that's why it's the message we
   // hook for dispatching the gunshot sounds to the bots' ears.

   static int weapon_state;
   static int weapon_id;

   if (messagestate == 0)
      weapon_state = *(int *) p; // state of the current weapon
   else if (messagestate == 1)
      weapon_id = *(int *) p; // weapon ID of current weapon
   else if (messagestate == 2)
   {
      // is that new weapon's ID valid and is the weapon in a usable state ?
      if ((weapon_id <= 31) && (weapon_state == 1))
      {
         // update the pointer to the new current weapon in the bot weapons array
         bots[bot_index].current_weapon = &bots[bot_index].bot_weapons[WeaponIndexOf (FindWeaponById (weapon_id))];
         bots[bot_index].current_weapon->clip_ammo = *(int *) p; // update the ammo in clip
         bots[bot_index].current_weapon->primary_charging_time = 0; // reset the charging times
         bots[bot_index].current_weapon->secondary_charging_time = 0; // reset the charging times

         PlayBulletSoundsForBots (bots[bot_index].pEdict); // play bullet sounds on bot's client side
      }
   }
}


void BotClient_CS_AmmoX (void *p, int bot_index)
{
   // this message tells a game client to raise or lower a certain type of ammo by a certain
   // value (for example, a decrease in bullets when the player loads a new clip in his weapon).
   // It is also sent when players spawn, to set all their ammo back to starting values.

   static int ammo_index;

   if (messagestate == 0)
      ammo_index = *(int *) p; // read the ammo index (for the type of ammo)
   else if (messagestate == 1)
      bots[bot_index].bot_ammos[ammo_index] = *(int *) p; // update amount of this type of ammo
}


void BotClient_CS_AmmoPickup (void *p, int bot_index)
{
   // this message tells a game client that his player picked up some ammo. The only difference
   // with the AmmoX message is that this message enables the client to display a nice little
   // icon on the player's HUD to notify him that he just picked up some ammo.

   static int ammo_index;

   if (messagestate == 0)
      ammo_index = *(int *) p;
   else if (messagestate == 1)
      bots[bot_index].bot_ammos[ammo_index] = *(int *) p; // update amount of this type of ammo
}


void BotClient_CS_WeapPickup (void *p, int bot_index)
{
   // this message tells a game client that his player picked up a weapon. Similarly to the
   // AmmoPickup function above, it is used to tell the client to display that little icon on
   // the player's HUD to notify him that he just picked up a weapon.

   // don't forget to load the weapon as soon as it is picked up
   if (messagestate == 0)
      bots[bot_index].f_reload_time = *server.time + RANDOM_LONG (0.5, 1.0);
}


void BotClient_CS_ItemPickup (void *p, int bot_index)
{
   // this message tells a game client that his player picked up an item, like a battery or a
   // healthkit. Similarly to the AmmoPickup function above, it is used to tell the client to
   // display that little icon on the player's HUD to notify him that he picked up something.
}


void BotClient_CS_Health (void *p, int bot_index)
{
   // this message tells a game client that his player's health changed to some new value.
}


void BotClient_CS_Battery (void *p, int bot_index)
{
   // this message tells a game client that his player's armor changed to some new value.
}


void BotClient_CS_Damage (void *p, int bot_index)
{
   // this message tells a game client that his player is taking damage by some other entity
   // (likely, an enemy, but it can also be the world itself). It is used to display all the
   // damage-related signaletics on the player's HUD.

   static int damage_armor;
   static int damage_taken;
   static int damage_bits; // type of damage being done
   static Vector damage_origin;

   if (messagestate == 0)
      damage_armor = *(int *) p;
   else if (messagestate == 1)
      damage_taken = *(int *) p;
   else if (messagestate == 2)
      damage_bits = *(int *) p;
   else if (messagestate == 3)
      damage_origin.x = *(float *) p;
   else if (messagestate == 4)
      damage_origin.y = *(float *) p;
   else if (messagestate == 5)
   {
      damage_origin.z = *(float *) p;

      if ((damage_armor > 0) || (damage_taken > 0))
      {
         if (damage_bits & IGNORE_DAMAGE)
            return; // ignore certain types of damage

         // ignore damage if resulting from a fall (IGNORE_DAMAGE bug workaround)
         if (bots[bot_index].BotBody.fall_time + 0.25 > *server.time)
            return;

         // if the bot has no enemy and someone is shooting at him...
         if (FNullEnt (bots[bot_index].BotEnemy.pEdict))
         {
            // face the attacker
            BotSetIdealYaw (&bots[bot_index], UTIL_VecToAngles (damage_origin - bots[bot_index].pEdict->v.origin).y);
            bots[bot_index].f_reach_time = *server.time + 0.5; // delay reaching point
            bots[bot_index].bot_task &= ~BOT_TASK_USINGCHARGER; // stop using wall-mounted chargers
         }

         // if no audio chat with this bot for 5 seconds, speak
         if (bots[bot_index].BotChat.f_sayaudio_time + 5.0 < *server.time)
         {
            bots[bot_index].BotChat.bot_sayaudio |= BOT_SAYAUDIO_TAKINGDAMAGE;
            bots[bot_index].BotChat.f_sayaudio_time = *server.time + RANDOM_FLOAT (0.5, 3.0);
         }
      }
   }
}


void BotClient_CS_Money (void *p, int bot_index)
{
   // this message tells a game client to update his player's amount of money to a certain value

   // only take the first packet (only want the new amount of money)
   if (messagestate == 0)
      bots[bot_index].bot_money = *(int *)p; // update the new amount of money
}


void BotClient_CS_ReloadSound (void *p, int bot_index)
{
   // this message tells a game client that a reload sound is being played in the neighbourhood
   // of its player character. Luckily enough, we built our sounds database in the same order
   // the engine built its own (since we hooked it on PrecacheSound() calls), and so the sound
   // index the engine claims to play the reload sound matches exactly the right slot in our own
   // sounds array. How handy =)

   static int sound_index;

   if (messagestate == 0)
      sound_index = *(int *) p; // get the index of the reload sound
   else if (messagestate == 1)
   {
      float volume;
      edict_t *pSender = players[*(int *) p - 1].pEntity; // get the sender entity

      if (!IsValidPlayer (pSender))
         return; // reliability check, cancel if sender entity is invalid

      // compute the sound attenuation according to the distance (FIXME: this is arbitrary, and
      // probably highly inaccurate)
      volume = 1 - (GetGunPosition (pSender) - (bots[bot_index].pEdict->v.origin + bots[bot_index].pEdict->v.view_ofs)).Length () / 500;
      if (volume < 0.1)
         volume = 0.1; // consider that after 500 units far the bot hears almost nothing

      // dispatch that reload sound to the bot's ears (note that engine sound indexes start at 1)
      BotFeedEar (&bots[bot_index], &sounds[sound_index - 1], GetGunPosition (pSender), volume);
   }
}


void BotClient_CS_SayText (void *p, int bot_index)
{
   // this message updates a game client with a chat message, to tell him to display this
   // message on his player's HUD.

   // only take the second packet of a SayText message (only want the actual text string)
   if (messagestate == 1)
   {
      char message[120], sender_id_string[33];
      int index, start_pos, sender_id_length;
      edict_t *pPlayer;
      bool teamonly = FALSE;

      strcpy (message, (char *) p); // get the message
      start_pos = 1; // first character is the color set id, so skip it

      // does it end with a carriage return ?
      if (message[strlen (message) - 1] == '\n')
         message[strlen (message) - 1] = 0; // strip the carriage return

      // is it a dead player speaking ?
      if (strncmp (&message[start_pos], "*DEAD* ", 6) == 0)
         start_pos += 6; // skip the death notification text

      // is it a team only message ?
      if (strncmp (&message[start_pos], "(Terrorist) ", 12) == 0)
      {
         start_pos += 12; // skip the team notification text
         teamonly = TRUE; // remember this is a team only message
      }
      else if (strncmp (&message[start_pos], "(Counter-Terrorist) ", 20) == 0)
      {
         start_pos += 20; // skip the team notification text
         teamonly = TRUE; // remember this is a team only message
      }

      // search the world for players...
      for (index = 0; index < *server.max_clients; index++)
      {
         pPlayer = players[index].pEntity; // quick access to player

         if (!IsValidPlayer (pPlayer))
            continue; // skip invalid players

         // what would this player name look like in a chat string ?
         if (teamonly)
            sprintf (sender_id_string, "%s :   ", STRING (pPlayer->v.netname)); // THREE spaces for team messages
         else
            sprintf (sender_id_string, "%s :    ", STRING (pPlayer->v.netname)); // FOUR spaces for other messages

         sender_id_length = strlen (sender_id_string); // get the length of that string

         // so, is it that guy whose name is written right here ?
         if (strncmp (&message[start_pos], sender_id_string, sender_id_length) == 0)
         {
            // we've found the sender of the message
            start_pos += sender_id_length; // skip that part

            // so copy the sender name and the message to the bot's HUD for the bot to see them
            bots[bot_index].BotEyes.BotHUD.chat.pSender = pPlayer;
            strcpy (bots[bot_index].BotEyes.BotHUD.chat.text, UpperCase (&message[start_pos]));

            bots[bot_index].BotEyes.BotHUD.chat.new_message = TRUE; // tells the bot someone just chatted
         }
      }
   }
}


void BotClient_CS_TextMsg (void *p, int bot_index)
{
   // this message tells a client that his player received a radio message, to enable him to
   // play the appropriate radio sound sample file.

   static bool is_radio_message = FALSE;
   static char sound_file[256];
   static sound_t *radio_sound;
   static int i;

   if (messagestate == 0)
   {
      if (strcmp ((char *) p, "#Game_radio") == 0)
         is_radio_message = TRUE; // remember when it's a radio message
      else
         is_radio_message = FALSE; // reset the flag otherwise since it is static
   }
   else if (messagestate == 2)
   {
      // only check the rest of the message if we are certain it's a radio message
      if (is_radio_message)
      {
         // identify the radio sample and choose the sound file that will be played at the bot's
         // first radio menu
         if (strcmp ((char *) p, "#Cover_me") == 0)
            strcpy (sound_file, "radio/ct_coverme.wav"); // 'Cover me' radio message
         else if (strcmp ((char *) p, "#You_take_the_point") == 0)
            strcpy (sound_file, "radio/takepoint.wav"); // 'You take the point' radio message
         else if (strcmp ((char *) p, "#Hold_this_position") == 0)
            strcpy (sound_file, "radio/position.wav"); // 'Hold this position' radio message
         else if (strcmp ((char *) p, "#Regroup_team") == 0)
            strcpy (sound_file, "radio/regroup.wav"); // 'Regroup team' radio message
         else if (strcmp ((char *) p, "#Follow_me") == 0)
            strcpy (sound_file, "radio/followme.wav"); // 'Follow me' radio message
         else if (strcmp ((char *) p, "#Taking_fire") == 0)
            strcpy (sound_file, "radio/fireassis.wav"); // 'Taking fire, need backup' radio message

         // second radio menu
         else if (strcmp ((char *) p, "#Go_go_go") == 0)
            strcpy (sound_file, "radio/com_go.wav"); // 'Go Go Go' radio message
         else if (strcmp ((char *) p, "#Team_fall_back") == 0)
            strcpy (sound_file, "radio/fallback.wav"); // 'Team fall back' radio message
         else if (strcmp ((char *) p, "#Stick_together_team") == 0)
            strcpy (sound_file, "radio/sticktog.wav"); // 'Stick together team' radio message
         else if (strcmp ((char *) p, "#Get_in_position_and_wait") == 0)
            strcpy (sound_file, "radio/com_getinpos.wav"); // 'Stay in position and wait for my go' radio message
         else if (strcmp ((char *) p, "#Storm_the_front") == 0)
            strcpy (sound_file, "radio/stormfront.wav"); // 'Storm The Front' radio message
         else if (strcmp ((char *) p, "#Report_in_team") == 0)
            strcpy (sound_file, "radio/com_reportin.wav"); // 'Report In' radio message

         // third radio menu
         else if (strcmp ((char *) p, "#Affirmative") == 0)
            strcpy (sound_file, "radio/ct_affirm.wav"); // 'Affirmative' radio message
         else if (strcmp ((char *) p, "#Enemy_spotted") == 0)
            strcpy (sound_file, "radio/ct_enemys.wav"); // 'Enemy spotted' radio message
         else if (strcmp ((char *) p, "#Need_backup") == 0)
            strcpy (sound_file, "radio/ct_backup.wav"); // 'Need backup' radio message
         else if (strcmp ((char *) p, "#Sector_clear") == 0)
            strcpy (sound_file, "radio/clear.wav"); // 'Sector clear' radio message
         else if (strcmp ((char *) p, "#In_position") == 0)
            strcpy (sound_file, "radio/ct_inpos.wav"); // 'I'm in position' radio message
         else if (strcmp ((char *) p, "#Reporting_in") == 0)
            strcpy (sound_file, "radio/ct_reportingin.wav"); // 'Reporting in' radio message
         else if (strcmp ((char *) p, "#Get_out_of_there") == 0)
            strcpy (sound_file, "radio/blow.wav"); // 'Get outta here' radio message
         else if (strcmp ((char *) p, "#Negative") == 0)
            strcpy (sound_file, "radio/negative.wav"); // 'Negative' radio message
         else if (strcmp ((char *) p, "#Enemy_down") == 0)
            strcpy (sound_file, "radio/enemydown.wav"); // 'Negative' radio message

         // client-side radio messages
         // I miss "A hostage has been rescued", dunno how to catch it (if you do, lemme know)
         else if (strcmp ((char *) p, "#Bomb_Planted") == 0)
            strcpy (sound_file, "radio/bombpl.wav"); // 'Bomb planted' radio message
         else if (strcmp ((char *) p, "#Bomb_Defused") == 0)
            strcpy (sound_file, "radio/bombdef.wav"); // 'Bomb defused' radio message
         else if (strcmp ((char *) p, "#Round_Draw") == 0)
            strcpy (sound_file, "radio/rounddraw.wav"); // 'Round draw' radio message
         else if (strcmp ((char *) p, "#Terrorists_Win") == 0)
            strcpy (sound_file, "radio/terwin.wav"); // 'Terrorists win' radio message
         else if (strcmp ((char *) p, "#CTs_Win") == 0)
            strcpy (sound_file, "radio/ctwin.wav"); // 'Counter-terrorists win' radio message

         // find the sound we want in the global list
         for (i = 0; i < sound_count; i++)
            if (strcmp (sounds[i].file_path, sound_file) == 0)
               radio_sound = &sounds[i]; // link a pointer to this sound's info slot

         // have we found the sound we want to play ?
         if (radio_sound != NULL)
            BotFeedEar (&bots[bot_index], radio_sound, bots[bot_index].pEdict->v.origin + bots[bot_index].pEdict->v.view_ofs, 1.0); // bot will hear this radio sound
      }
   }
}


void BotClient_CS_ScreenFade (void *p, int bot_index)
{
   // this message tells a game client to fade in/out the screen of his player to a certain
   // color, and for a certain duration. It can happen, for example, when players are affected
   // by a flash grenade.

   static int duration;

   if (messagestate == 0)
      duration = *(int *) p; // read the duration of the effect
   else if (messagestate == 1)
      bots[bot_index].BotEyes.blinded_time = *server.time + ((float) (duration + *(int *) p) / 4096) - bots[bot_index].pProfile->skill;
}


void BotClient_CS_StatusIcon (void *p, int bot_index)
{
   // this message tells a game client to display (or stop displaying) a certain status icon on
   // his player's HUD.

   static int icon_status;

   if (messagestate == 0)
      icon_status = *(int *) p;
   else if (messagestate == 1)
   {
      // is it the C4 icon ?
      if (strcmp ((char *) p, "c4") == 0)
      {
         if (icon_status == 2)
            bots[bot_index].b_can_plant = TRUE; // bomb icon is blinking, bot can plant the bomb
         else
            bots[bot_index].b_can_plant = FALSE; // either bot has no bomb or it is not blinking yet
      }

      // else is it the defuse kit icon ?
      else if (strcmp ((char *) p, "defuser") == 0)
      {
         if (icon_status == 1)
            bots[bot_index].has_defuse_kit = TRUE; // defuser icon is present
         else
            bots[bot_index].has_defuse_kit = FALSE; // defuser icon is (no more) present
      }
   }
}


void BotClient_CS_BarTime (void *p, int bot_index)
{
   // this message tells a game client to display a progress bar on his player's HUD for a
   // certain amount of time. If this value is zero, the client should make the progress bar
   // disappear from the HUD.

   if (messagestate == 0)
   {
      if (*(int *) p > 0)
         bots[bot_index].BotEyes.BotHUD.has_progress_bar = TRUE; // progress bar present on HUD
      else if (*(int *) p == 0)
         bots[bot_index].BotEyes.BotHUD.has_progress_bar = FALSE; // progress bar disappeared
   }
}


void BotClient_CS_TextMsg_All (void *p, int bot_index)
{
   // This message gets sent when a text message is displayed on the HUD of all players

   // catch the "bomb planted" message
   if (strcmp ((char *) p, "#Bomb_Planted") == 0)
   {
      round.b_bomb_planted = TRUE;

      // cycle through all bot slots
      for (int index = 0; index < RACC_MAX_CLIENTS; index++)
      {
         if (!bots[index].is_active || !IsValidPlayer (bots[index].pEdict))
            continue; // skip inactive bots and invalid player slots

         // is this bot a Counter-Terrorist and is he camping right now ?
         if ((GetTeam (bots[index].pEdict) == CS_COUNTER_TERRORIST) && (bots[index].f_camp_time > *server.time))
         {
            bots[index].v_place_to_keep = g_vecZero; // forget this place
            bots[index].f_camp_time = *server.time; // don't camp anymore
            bots[index].f_rush_time = *server.time + 60; // rush until the bomb explodes
         }
      }
   }

   // catch the "bomb defused" message
   else if (strcmp ((char *) p, "#Bomb_Defused") == 0)
   {
      round.b_bomb_planted = FALSE;
   }

   // catch the "round draw" message
   else if (strcmp ((char *) p, "#Round_Draw") == 0)
   {
      round.b_bomb_planted = FALSE;
   }

   // catch the "terrorists win" end of round message
   else if ((strcmp ((char *) p, "#Terrorists_Win") == 0)
            || (strcmp ((char *) p, "#Target_Bombed") == 0)
            || (strcmp ((char *) p, "#Terrorists_Escaped") == 0)
            || (strcmp ((char *) p, "#VIP_Assassinated") == 0)
            || (strcmp ((char *) p, "#VIP_Not_Escaped") == 0)
            || (strcmp ((char *) p, "#Hostages_Not_Rescued") == 0))
   {
      round.b_bomb_planted = FALSE;
   }

   // catch the "counter-terrorists win" end of round message
   else if ((strcmp ((char *) p, "#CTs_Win") == 0)
            || (strcmp ((char *) p, "#Target_Saved") == 0)
            || (strcmp ((char *) p, "#Terrorists_Not_Escaped") == 0)
            || (strcmp ((char *) p, "#CTs_PreventEscape") == 0)
            || (strcmp ((char *) p, "#VIP_Escaped") == 0)
            || (strcmp ((char *) p, "#All_Hostages_Rescued") == 0))
   {
      round.b_bomb_planted = FALSE;
   }

   return;
}


void BotClient_CS_DeathMsg_All (void *p, int bot_index)
{
   // this message is a broadcast message that tells the game clients that a certain player got
   // killed by a certain entity, in a certain manner. When the client whose player got killed
   // receives the message, it tells him to take the appropriate actions to let his player know
   // that he is now dead (scratch him on the ground and disable him to move, for example).

   static int killer_index;
   static int victim_index;

   if (messagestate == 0)
      killer_index = *(int *) p; // ENTINDEX() of killer
   else if (messagestate == 1)
      victim_index = *(int *) p; // ENTINDEX() of victim
   else if (messagestate == 2)
   {
      // is this message about a bot being killed ?
      if (bots[victim_index - 1].is_active)
      {
         if ((killer_index == 0) || (killer_index == victim_index))
            bots[victim_index - 1].pKillerEntity = NULL; // bot killed by world or by self...
         else
            bots[victim_index - 1].pKillerEntity = players[killer_index - 1].pEntity; // store edict of killer
      }
   }
}
