// RACC - AI development project for first-person shooter games
// (http://racc.bots-united.com/)
//
// Rational Autonomous Cybernetic Commandos AI
//
// CSTRIKE version
//
// client.cpp
//

#include "racc.h"


void ExamineNetworkMessage (void)
{
   // this function is called as soon as the network message catcher has trapped a network
   // message that the game DLL has just sent. We then examine the contents of the message and
   // decide of the appropriate actions to take regarding to it.

   // only process bots if we are in multiplayer mode
   if (server.is_multiplayer && !DebugLevel.is_paused)
   {
      if (!message_header.is_broadcasted && (message_header.player_index < 0))
         return; // reliability check: discard message if NOT broadcast AND invalid destination

      // is this message directed towards a player ?
      if (!message_header.is_broadcasted)
      {
         if (message_header.message_type == GetUserMsgId ("VGUIMenu"))
            NetworkMessage_VGUIMenu ();
         else if (message_header.message_type == GetUserMsgId ("ShowMenu"))
            NetworkMessage_ShowMenu ();
         else if (message_header.message_type == GetUserMsgId ("WeaponList"))
            NetworkMessage_WeaponList ();
         else if (message_header.message_type == GetUserMsgId ("CurWeapon"))
            NetworkMessage_CurWeapon ();
         else if (message_header.message_type == GetUserMsgId ("AmmoX"))
            NetworkMessage_AmmoX ();
         else if (message_header.message_type == GetUserMsgId ("WeapPickup"))
            NetworkMessage_WeapPickup ();
         else if (message_header.message_type == GetUserMsgId ("AmmoPickup"))
            NetworkMessage_AmmoPickup ();
         else if (message_header.message_type == GetUserMsgId ("ItemPickup"))
            NetworkMessage_ItemPickup ();
         else if (message_header.message_type == GetUserMsgId ("Health"))
            NetworkMessage_Health ();
         else if (message_header.message_type == GetUserMsgId ("Battery"))
            NetworkMessage_Battery ();
         else if (message_header.message_type == GetUserMsgId ("Damage"))
            NetworkMessage_Damage ();
         else if (message_header.message_type == GetUserMsgId ("Money"))
            NetworkMessage_Money ();
         else if (message_header.message_type == GetUserMsgId ("ScreenFade"))
            NetworkMessage_ScreenFade ();
         else if (message_header.message_type == GetUserMsgId ("ReloadSound"))
            NetworkMessage_ReloadSound ();
         else if (message_header.message_type == GetUserMsgId ("SayText"))
            NetworkMessage_SayText ();
         else if (message_header.message_type == GetUserMsgId ("TextMsg"))
            NetworkMessage_TextMsg ();
         else if (message_header.message_type == GetUserMsgId ("StatusIcon"))
            NetworkMessage_StatusIcon ();
         else if (message_header.message_type == GetUserMsgId ("BarTime"))
            NetworkMessage_BarTime ();
         else if (message_header.message_type == GetUserMsgId ("BombDrop"))
            NetworkMessage_BombDrop ();
         else if (message_header.message_type == GetUserMsgId ("BombPickup"))
            NetworkMessage_BombPickup ();
         else if (message_header.message_type == GetUserMsgId ("RoundTime"))
            NetworkMessage_RoundTime ();
      }

      // else it must be a broadcasted message
      else
      {
         if (message_header.message_type == GetUserMsgId ("DeathMsg"))
            NetworkMessageAll_DeathMsg ();
         else if (message_header.message_type == GetUserMsgId ("TextMsg"))
            NetworkMessageAll_TextMsg ();
      }
   }

   return; // finished parsing this network message
}


void NetworkMessage_VGUIMenu (void)
{
   // this message tells a game client to display a VGUI menu

   player_t *pPlayer = &players[message_header.player_index]; // quick access to player

   if (message[0].iValuePassed == 2) // is it a team select menu ?
      pPlayer->Bot.BotEyes.BotHUD.menu_state = MENU_CSTRIKE_TEAMSELECT_MAINMENU;
   else if (message[0].iValuePassed == 26) // is is the terrorist model select menu ?
      pPlayer->Bot.BotEyes.BotHUD.menu_state = MENU_CSTRIKE_TEAMSELECT_TERRMENU;
   else if (message[0].iValuePassed == 27) // is is the counter-terrorist model select menu ?
      pPlayer->Bot.BotEyes.BotHUD.menu_state = MENU_CSTRIKE_TEAMSELECT_COUNTERMENU;

   return;
}


void NetworkMessage_ShowMenu (void)
{
   // this message tells a game client to display a HUD text menu

   player_t *pPlayer = &players[message_header.player_index]; // quick access to player

   if (strcmp ("#Team_Select", message[3].szValuePassed) == 0) // is is the team select menu ?
      pPlayer->Bot.BotEyes.BotHUD.menu_state = MENU_CSTRIKE_TEAMSELECT_MAINMENU;
   else if (strcmp ("#Terrorist_Select", message[3].szValuePassed) == 0) // is is the T model select ?
      pPlayer->Bot.BotEyes.BotHUD.menu_state = MENU_CSTRIKE_TEAMSELECT_TERRMENU;
   else if (strcmp ("#CT_Select", message[3].szValuePassed) == 0) // is is the CT model select menu ?
      pPlayer->Bot.BotEyes.BotHUD.menu_state = MENU_CSTRIKE_TEAMSELECT_COUNTERMENU;

   return;
}


void NetworkMessage_WeaponList (void)
{
   // This message is sent when a client joins the game. All the weapons data are sent to him
   // with the weapon ID, information about what type of ammo is used, and the maximal amount
   // of ammo for each rail of the weapon. We hook it to fill in some parts of our weapon
   // database, since this is the most reliable data about weapons we can get.

   player_t *pPlayer;
   weapon_t weapon, *corresponding_weapon;

   pPlayer = &players[message_header.player_index]; // quick access to player

   // build the weapon structure corresponding to the weapon this message is talking about
   strcpy (weapon.classname, message[0].szValuePassed);
   weapon.primary.type_of_ammo = message[1].iValuePassed; // primary rail type of ammo (index in ammo array)
   weapon.primary.max_ammo = message[2].iValuePassed; // maximum amount of ammo the primary rail can hold
   weapon.secondary.type_of_ammo = message[3].iValuePassed; // secondary rail type of ammo (index in ammo array)
   weapon.secondary.max_ammo = message[4].iValuePassed; // maximum amount of ammo the secondary rail can hold
   weapon.id = message[7].iValuePassed; // the weapon ID itself

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

   // now store this weapon information, overwriting the existing data (read from file) if needed
   strcpy (corresponding_weapon->classname, weapon.classname);
   corresponding_weapon->id = weapon.id;
   corresponding_weapon->primary.type_of_ammo = weapon.primary.type_of_ammo;
   corresponding_weapon->primary.max_ammo = weapon.primary.max_ammo;
   corresponding_weapon->secondary.type_of_ammo = weapon.secondary.type_of_ammo;
   corresponding_weapon->secondary.max_ammo = weapon.secondary.max_ammo;

   return;
}


void NetworkMessage_CurWeapon (void)
{
   // This message is sent when a game client just selected one weapon. The server updates him
   // with the amount of ammo it knows it has and the state of the weapon, to ensure the client
   // will use the same ammo amount that the server knows he has. It is also sent to clients when
   // the server auto assigns them a weapon. It seems also to be sent when the weapon is fired,
   // causing the amount of ammo currently in clip to decrease, that's why it's the message we
   // hook for dispatching the gunshot sounds to the bots' ears.

   player_t *pPlayer;
   int weapon_id;

   pPlayer = &players[message_header.player_index]; // quick access to player

   if (!pPlayer->is_alive)
      return; // discard CurWeapon messages sent to players spectating in first-person mode

   weapon_id = message[1].iValuePassed; // weapon ID of current weapon

   // is that new weapon's ID valid and is the weapon in a usable state (FIXME: do we need this?)
   if ((weapon_id <= 31) && (message[0].iValuePassed == 1))
   {
      // update the pointer to the new current weapon in the bot weapons array
      pPlayer->Bot.current_weapon = &pPlayer->Bot.bot_weapons[WeaponIndexOf (FindWeaponById (weapon_id))];
      pPlayer->Bot.current_weapon->clip_ammo = message[2].iValuePassed; // update the ammo in clip
      pPlayer->Bot.current_weapon->primary_charging_time = 0; // reset the charging times
      pPlayer->Bot.current_weapon->secondary_charging_time = 0; // reset the charging times

      PlayBulletSoundsForBots (pPlayer); // play bullet sounds on bot's client side
   }

   return;
}


void NetworkMessage_AmmoX (void)
{
   // this message tells a game client to raise or lower a certain type of ammo by a certain
   // value (for example, a decrease in bullets when the player loads a new clip in his weapon).
   // It is also sent when players spawn, to set all their ammo back to starting values.

   player_t *pPlayer = &players[message_header.player_index]; // quick access to player

   // update the new amount of this type of ammo (ammo type is packet #0)
   pPlayer->Bot.bot_ammos[message[0].iValuePassed] = message[1].iValuePassed; // store it away

   return;
}


void NetworkMessage_AmmoPickup (void)
{
   // this message tells a game client that his player picked up some ammo. The only difference
   // with the AmmoX message is that this message enables the client to display a nice little
   // icon on the player's HUD to notify him that he just picked up some ammo.

   player_t *pPlayer = &players[message_header.player_index]; // quick access to player

   // update the new amount of this type of ammo (ammo type is packet #0)
   pPlayer->Bot.bot_ammos[message[0].iValuePassed] = message[1].iValuePassed; // store it away

   return;
}


void NetworkMessage_WeapPickup (void)
{
   // this message tells a game client that his player picked up a weapon. Similarly to the
   // AmmoPickup function above, it is used to tell the client to display that little icon on
   // the player's HUD to notify him that he just picked up a weapon.

   player_t *pPlayer = &players[message_header.player_index]; // quick access to player

   // don't forget to load the weapon as soon as it is picked up
   pPlayer->Bot.reload_time = server.time + RandomFloat (0.5, 1.0);

   return;
}


void NetworkMessage_ItemPickup (void)
{
   // this message tells a game client that his player picked up an item, like a battery or a
   // healthkit. Similarly to the AmmoPickup function above, it is used to tell the client to
   // display that little icon on the player's HUD to notify him that he picked up something.

   // TODO: improve self-confidence of the bot here

   return;
}


void NetworkMessage_Health (void)
{
   // this message tells a game client that his player's health changed to some new value.

   // TODO: improve self-confidence of the bot here

   return;
}


void NetworkMessage_Battery (void)
{
   // this message tells a game client that his player's armor changed to some new value.

   // TODO: improve self-confidence of the bot here

   return;
}


void NetworkMessage_Damage (void)
{
   // this message tells a game client that his player is taking damage by some other entity
   // (likely, an enemy, but it can also be the world itself). It is used to display all the
   // damage-related signaletics on the player's HUD.

   player_t *pPlayer;
   int damage_health;
   int damage_armor;
   Vector damage_origin;

   pPlayer = &players[message_header.player_index]; // quick access to player

   damage_armor = message[0].iValuePassed;
   damage_health = message[1].iValuePassed;
   damage_origin = Vector (message[3].flValuePassed, message[4].flValuePassed, message[5].flValuePassed);

   // did we, indeed, take some visible damage ?
   if ((damage_health > 0) || (damage_armor > 0))
   {
      // ignore damage if resulting from a fall
      if (pPlayer->Bot.BotBody.fall_time + 0.25 > server.time)
         return;

      // if the bot has no enemy and someone is shooting at him...
      if (FNullEnt (pPlayer->Bot.BotEnemy.pEdict))
      {
         // face the attacker
         BotSetIdealYaw (pPlayer, VecToAngles (damage_origin - pPlayer->v_origin).y);
         pPlayer->Bot.reach_time = server.time + 0.5; // delay reaching point
         pPlayer->Bot.BotBrain.bot_task = BOT_TASK_IDLE; // stop any current task
      }

      pPlayer->Bot.BotChat.bot_sayaudio = BOT_SAYAUDIO_TAKINGDAMAGE;

      // TODO: decrease self-confidence of the bot here
   }

   return;
}


void NetworkMessage_Money (void)
{
   // this message tells a game client to update his player's amount of money to a certain value

   player_t *pPlayer = &players[message_header.player_index]; // quick access to player

   // only take the first packet (only want the new amount of money)
   pPlayer->money = message[0].iValuePassed; // update the new amount of money

   return;
}


void NetworkMessage_ReloadSound (void)
{
   // this message tells a game client that a reload sound is being played in the neighbourhood
   // of its player character.

   player_t *pPlayer;
   player_t *pSender;
   float volume;

   // FIXME: normally, the engine sends the sound index to be played in packet #0, but we can't
   // use it since our own sounds database is not necessarily in the same order as the engine's.
   // Also, whenever this gets fixed, REMEMBER that engine sound indices start at 1.
   static sound_t *reload_sound = NULL;
   if (reload_sound == NULL)
      reload_sound = FindSoundByFilename ("weapons/generic_reload.wav");

   pPlayer = &players[message_header.player_index]; // quick access to player

   // is the player receiving this sound a bot ?
   if (pPlayer->is_racc_bot)
   {
      pSender = &players[message[1].iValuePassed - 1]; // quick access to sender entity

      if (!IsValidPlayer (pSender))
         return; // reliability check, cancel if sender entity is invalid

      // compute the sound attenuation according to the distance (FIXME: this is arbitrary, and
      // probably highly inaccurate)
      volume = 1 - (pSender->v_eyeposition - (pPlayer->v_eyeposition)).Length () / 500;
      if (volume < 0.1)
         volume = 0.1; // consider that after 500 units far the bot hears almost nothing

      // dispatch that reload sound to the bot's ears
      BotFeedEar (pPlayer, reload_sound, pSender->v_eyeposition, volume);
   }

   return;
}


void NetworkMessage_SayText (void)
{
   // this message updates a game client with a chat message, to tell him to display this
   // message on his player's HUD.

   char chat_message[128];
   int chat_message_length;
   player_t *pPlayer;

   pPlayer = &players[message_header.player_index]; // quick access to player

   // is the player receiving this message a bot ?
   if (pPlayer->is_racc_bot)
   {
      strcpy (chat_message, message[3].szValuePassed); // get the message
      chat_message_length = strlen (chat_message); // get its length

      // does it end with a carriage return ?
      if (chat_message[chat_message_length - 1] == '\n')
         chat_message[chat_message_length - 1] = 0; // strip the carriage return

      // copy the sender index and the message to the bot's HUD for the bot to see them
      pPlayer->Bot.BotEyes.BotHUD.chat.sender_index = message[0].iValuePassed;
      strcpy (pPlayer->Bot.BotEyes.BotHUD.chat.text, UpperCase (&chat_message[1]));

      pPlayer->Bot.BotEyes.BotHUD.chat.new_message = TRUE; // tells the bot someone just chatted
   }

   return;
}


void NetworkMessage_TextMsg (void)
{
   // this message tells a client that his player received a radio message, to enable him to
   // play the appropriate radio sound sample file.

   player_t *pPlayer;
   char sound_file[256];
   sound_t *radio_sound;
   int i;

   if (!((message[0].packet_type == PACKET_STRING)
         && (strcmp (message[0].szValuePassed, "#Game_radio") == 0)))
      return; // cancel if message is NOT a radio message

   pPlayer = &players[message_header.player_index]; // quick access to player

   // is the player receiving this message a bot AND is it a radio message ?
   if (pPlayer->is_racc_bot)
   {
      // identify the radio sample and choose the sound file that will be played at the bot's
      // first radio menu
      if (strcmp ("#Cover_me", message[2].szValuePassed) == 0)
         strcpy (sound_file, "radio/ct_coverme.wav"); // 'Cover me' radio message
      else if (strcmp ("#You_take_the_point", message[2].szValuePassed) == 0)
         strcpy (sound_file, "radio/takepoint.wav"); // 'You take the point' radio message
      else if (strcmp ("#Hold_this_position", message[2].szValuePassed) == 0)
         strcpy (sound_file, "radio/position.wav"); // 'Hold this position' radio message
      else if (strcmp ("#Regroup_team", message[2].szValuePassed) == 0)
         strcpy (sound_file, "radio/regroup.wav"); // 'Regroup team' radio message
      else if (strcmp ("#Follow_me", message[2].szValuePassed) == 0)
         strcpy (sound_file, "radio/followme.wav"); // 'Follow me' radio message
      else if (strcmp ("#Taking_fire", message[2].szValuePassed) == 0)
         strcpy (sound_file, "radio/fireassis.wav"); // 'Taking fire, need backup' radio message

      // second radio menu
      else if (strcmp ("#Go_go_go", message[2].szValuePassed) == 0)
         strcpy (sound_file, "radio/com_go.wav"); // 'Go Go Go' radio message
      else if (strcmp ("#Team_fall_back", message[2].szValuePassed) == 0)
         strcpy (sound_file, "radio/fallback.wav"); // 'Team fall back' radio message
      else if (strcmp ("#Stick_together_team", message[2].szValuePassed) == 0)
         strcpy (sound_file, "radio/sticktog.wav"); // 'Stick together team' radio message
      else if (strcmp ("#Get_in_position_and_wait", message[2].szValuePassed) == 0)
         strcpy (sound_file, "radio/com_getinpos.wav"); // 'Stay in position and wait for my go' radio message
      else if (strcmp ("#Storm_the_front", message[2].szValuePassed) == 0)
         strcpy (sound_file, "radio/stormfront.wav"); // 'Storm The Front' radio message
      else if (strcmp ("#Report_in_team", message[2].szValuePassed) == 0)
         strcpy (sound_file, "radio/com_reportin.wav"); // 'Report In' radio message

      // third radio menu
      else if (strcmp ("#Affirmative", message[2].szValuePassed) == 0)
         strcpy (sound_file, "radio/ct_affirm.wav"); // 'Affirmative' radio message
      else if (strcmp ("#Enemy_spotted", message[2].szValuePassed) == 0)
         strcpy (sound_file, "radio/ct_enemys.wav"); // 'Enemy spotted' radio message
      else if (strcmp ("#Need_backup", message[2].szValuePassed) == 0)
         strcpy (sound_file, "radio/ct_backup.wav"); // 'Need backup' radio message
      else if (strcmp ("#Sector_clear", message[2].szValuePassed) == 0)
         strcpy (sound_file, "radio/clear.wav"); // 'Sector clear' radio message
      else if (strcmp ("#In_position", message[2].szValuePassed) == 0)
         strcpy (sound_file, "radio/ct_inpos.wav"); // 'I'm in position' radio message
      else if (strcmp ("#Reporting_in", message[2].szValuePassed) == 0)
         strcpy (sound_file, "radio/ct_reportingin.wav"); // 'Reporting in' radio message
      else if (strcmp ("#Get_out_of_there", message[2].szValuePassed) == 0)
         strcpy (sound_file, "radio/blow.wav"); // 'Get outta here' radio message
      else if (strcmp ("#Negative", message[2].szValuePassed) == 0)
         strcpy (sound_file, "radio/negative.wav"); // 'Negative' radio message
      else if (strcmp ("#Enemy_down", message[2].szValuePassed) == 0)
         strcpy (sound_file, "radio/enemydown.wav"); // 'Negative' radio message

      // client-side radio messages
      // I miss "A hostage has been rescued", dunno how to catch it (if you do, lemme know)
      else if (strcmp ("#Bomb_Planted", message[2].szValuePassed) == 0)
         strcpy (sound_file, "radio/bombpl.wav"); // 'Bomb planted' radio message
      else if (strcmp ("#Bomb_Defused", message[2].szValuePassed) == 0)
         strcpy (sound_file, "radio/bombdef.wav"); // 'Bomb defused' radio message
      else if (strcmp ("#Round_Draw", message[2].szValuePassed) == 0)
         strcpy (sound_file, "radio/rounddraw.wav"); // 'Round draw' radio message
      else if (strcmp ("#Terrorists_Win", message[2].szValuePassed) == 0)
         strcpy (sound_file, "radio/terwin.wav"); // 'Terrorists win' radio message
      else if (strcmp ("#CTs_Win", message[2].szValuePassed) == 0)
         strcpy (sound_file, "radio/ctwin.wav"); // 'Counter-terrorists win' radio message

      // find the sound we want in the global list
      for (i = 0; i < sound_count; i++)
         if (strcmp (sounds[i].file_path, sound_file) == 0)
            radio_sound = &sounds[i]; // link a pointer to this sound's info slot

      // have we found the sound we want to play ?
      if (radio_sound != NULL)
         BotFeedEar (pPlayer, radio_sound, pPlayer->v_eyeposition, 1.0); // bot will hear this radio sound
   }

   return;
}


void NetworkMessage_ScreenFade (void)
{
   // this message tells a game client to fade in/out the screen of his player to a certain
   // color, and for a certain duration. It can happen, for example, when players are affected
   // by a flash grenade.

   player_t *pPlayer = &players[message_header.player_index]; // quick access to player

   // is the player whose screen is fading a bot ?
   if (pPlayer->is_racc_bot)
      pPlayer->Bot.BotEyes.blinded_time = server.time
                                          + ((float) (message[0].iValuePassed + message[1].iValuePassed) / 4096)
                                          - pPlayer->Bot.pProfile->skill; // bot is blind

   return;
}


void NetworkMessage_StatusIcon (void)
{
   // this message tells a game client to display (or stop displaying) a certain status icon on
   // his player's HUD.

   int client_index;
   player_t *pPlayer;
   player_t *pOtherPlayer;

   pPlayer = &players[message_header.player_index]; // quick access to player

   // is it the C4 icon ?
   if (strcmp ("c4", message[1].szValuePassed) == 0)
   {
      // is icon off ?
      if (message[0].iValuePassed == 0)
         pPlayer->Bot.BotEyes.BotHUD.icons_state[HUD_ICON_BOMB] = HUD_ICON_OFF; // icon off

      // else is icon lit ?
      else if (message[0].iValuePassed == 1)
      {
         // was the icon off before ? (BombPickup network message bug workaround)
         if (pPlayer->Bot.BotEyes.BotHUD.icons_state[HUD_ICON_BOMB] == HUD_ICON_OFF)
         {
            mission.bomb = BOMB_CARRIED; // remember that this bomb is carried

            // cycle through all teammates bots to notify them that the bomb has been picked up
            for (client_index = 0; client_index < server.max_clients; client_index++)
            {
               pOtherPlayer = &players[client_index]; // quick access to player

               if (IsValidPlayer (pOtherPlayer) && pOtherPlayer->is_racc_bot
                   && (GetTeam (pOtherPlayer) == GetTeam (pPlayer)))
                  pOtherPlayer->Bot.BotBrain.bot_goal = BOT_GOAL_NONE; // have this bot decide if it should change goal
            }
         }

         pPlayer->Bot.BotEyes.BotHUD.icons_state[HUD_ICON_BOMB] = HUD_ICON_LIT; // icon lit
      }

      // else is icon blinking ?
      else if (message[0].iValuePassed == 2)
         pPlayer->Bot.BotEyes.BotHUD.icons_state[HUD_ICON_BOMB] = HUD_ICON_BLINKING; // icon blinking
   }

   // else is it the defuse kit icon ?
   else if (strcmp ("defuser", message[1].szValuePassed) == 0)
   {
      // is icon off ?
      if (message[0].iValuePassed == 0)
         pPlayer->Bot.BotEyes.BotHUD.icons_state[HUD_ICON_DEFUSER] = HUD_ICON_OFF; // icon off

      // else is icon lit ?
      else if (message[0].iValuePassed == 1)
         pPlayer->Bot.BotEyes.BotHUD.icons_state[HUD_ICON_DEFUSER] = HUD_ICON_LIT; // icon lit

      // else is icon blinking ?
      else if (message[0].iValuePassed == 2)
         pPlayer->Bot.BotEyes.BotHUD.icons_state[HUD_ICON_DEFUSER] = HUD_ICON_BLINKING; // icon blinking
   }

   return;
}


void NetworkMessage_BarTime (void)
{
   // this message tells a game client to display a progress bar on his player's HUD for a
   // certain amount of time. If this value is zero, the client should make the progress bar
   // disappear from the HUD.

   player_t *pPlayer = &players[message_header.player_index]; // quick access to player

   // progress bar activated ? (the bartime duration in seconds is passed as packet #0)
   if (message[0].iValuePassed > 0)
      pPlayer->Bot.BotEyes.BotHUD.has_progress_bar = TRUE; // progress bar present on HUD

   // else has the progress bar just stopped ?
   else if (message[0].iValuePassed == 0)
      pPlayer->Bot.BotEyes.BotHUD.has_progress_bar = FALSE; // progress bar disappeared

   return;
}


void NetworkMessage_BombDrop (void)
{
   // This message gets sent when the bot receives a notification that someone on his team has
   // dropped the bomb on the ground

   player_t *pPlayer = &players[message_header.player_index]; // quick access to player

   mission.bomb = BOMB_DROPPED; // remember that this bomb is dropped

   // is the player receiving this message a bot ?
   if (pPlayer->is_racc_bot)
      pPlayer->Bot.BotBrain.bot_goal = BOT_GOAL_NONE; // have this bot decide if it should change goal

   return;
}


void NetworkMessage_BombPickup (void)
{
   // This message gets sent when the bot receives a notification that someone on his team has
   // picked up a dropped bomb

   player_t *pPlayer = &players[message_header.player_index]; // quick access to player

   mission.bomb = BOMB_CARRIED; // remember that this bomb is carried

   // is the player receiving this message a bot ?
   if (pPlayer->is_racc_bot)
      pPlayer->Bot.BotBrain.bot_goal = BOT_GOAL_NONE; // have this bot decide if it should change goal

   return;
}


void NetworkMessage_RoundTime (void)
{
   // This message gets sent when the round timer changes (for example, when the round has just
   // restarted)

   // if we are passing the duration of the freeze time, that's we are restarting a new round
   if (message[0].iValuePassed == atoi (CVAR_GET_STRING ("mp_freezetime")))
      mission.finished = TRUE; // remember that the round has restarted

   return;
}


void NetworkMessageAll_TextMsg (void)
{
   // This message gets sent when a text message is displayed on the HUD of all players

   int index;
   player_t *pPlayer;

   pPlayer = &players[message_header.player_index]; // quick access to player

   // any text made available to ALL players tend to indicate that there is an important change
   // in the mission so as to make the bots change their goals, so notify them.
   for (index = 0; index < server.max_clients; index++)
      players[index].Bot.BotBrain.bot_goal = BOT_GOAL_NONE; // reset all bots' goals

   // catch the "bomb planted" message
   if (strcmp ("#Bomb_Planted", message[1].szValuePassed) == 0)
      mission.bomb = BOMB_PLANTED; // make everybody know the bomb has been planted

   // catch the "bomb defused" message
   else if (strcmp ("#Bomb_Defused", message[1].szValuePassed) == 0)
      mission.bomb = BOMB_NONE; // no more bomb on this map

   // catch the "round draw" message
   else if (strcmp ("#Round_Draw", message[1].szValuePassed) == 0)
   {
      ;
   }

   // catch the "terrorists win" end of round message
   else if ((strcmp ("#Terrorists_Win", message[1].szValuePassed) == 0)
            || (strcmp ("#Target_Bombed", message[1].szValuePassed) == 0)
            || (strcmp ("#Terrorists_Escaped", message[1].szValuePassed) == 0)
            || (strcmp ("#VIP_Assassinated", message[1].szValuePassed) == 0)
            || (strcmp ("#VIP_Not_Escaped", message[1].szValuePassed) == 0)
            || (strcmp ("#Hostages_Not_Rescued", message[1].szValuePassed) == 0))
   {
      ;
   }

   // catch the "counter-terrorists win" end of round message
   else if ((strcmp ("#CTs_Win", message[1].szValuePassed) == 0)
            || (strcmp ("#Target_Saved", message[1].szValuePassed) == 0)
            || (strcmp ("#Terrorists_Not_Escaped", message[1].szValuePassed) == 0)
            || (strcmp ("#CTs_PreventEscape", message[1].szValuePassed) == 0)
            || (strcmp ("#VIP_Escaped", message[1].szValuePassed) == 0)
            || (strcmp ("#All_Hostages_Rescued", message[1].szValuePassed) == 0))
   {
      ;
   }

   return;
}


void NetworkMessageAll_DeathMsg (void)
{
   // this message is a broadcast message that tells the game clients that a certain player got
   // killed by a certain entity, in a certain manner. When the client whose player got killed
   // receives the message, it tells him to take the appropriate actions to let his player know
   // that he is now dead (scratch him on the ground and disable him to move, for example).

   player_t *pPlayer;
   player_t *pVictim;

   pPlayer = &players[message_header.player_index]; // quick access to player

   pVictim = &players[message[1].iValuePassed - 1]; // quick access to victim

   if (IsValidPlayer (pVictim))
      return; // reliability check

   // is the killer index pointing to a real player ?
   if (message[0].iValuePassed > 0)
      pVictim->Bot.killer_index = message[0].iValuePassed - 1; // store index of killer
   else
      pVictim->Bot.killer_index = message[1].iValuePassed - 1; // player killed by world

   return;
}
