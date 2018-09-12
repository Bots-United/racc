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
// CSTRIKE version
//
// bot_client.cpp
//

#include "racc.h"


extern bot_t bots[MAX_CLIENTS_SUPPORTED_BY_ENGINE];
extern weapon_t weapon_defs[MAX_WEAPONS];
extern sound_t sounds[MAX_SOUNDS + MAX_LOCAL_SOUNDS];
extern int sound_count;
extern int messagestate;



void BotClient_CS_VGUIMenu (void *p, int bot_index)
{
   // this message is sent when the Counter-Strike VGUI menu is displayed.

   // only take the first packet of a VGUI message (only want menu codes)
   if (messagestate == 0)
   {
      // which menu is it ?
      if ((*(int *) p) == 2)
         bots[bot_index].BotEyes.BotHUD.menu_notify = MSG_CS_TEAMSELECT_MAINMENU; // team select menu
      else if ((*(int *) p) == 26)
         bots[bot_index].BotEyes.BotHUD.menu_notify = MSG_CS_TEAMSELECT_TERRMENU; // terrorist menu
      else if ((*(int *) p) == 27)
         bots[bot_index].BotEyes.BotHUD.menu_notify = MSG_CS_TEAMSELECT_COUNTERMENU; // counter-terrorist menu
   }
}


void BotClient_CS_ShowMenu (void *p, int bot_index)
{
   // this message is sent when a menu is being displayed in Counter-Strike

   // only take the fourth packet of a ShowMenu message (only want menu codes)
   if (messagestate == 3)
   {
      // which menu is it ?
      if (strcmp ((char *) p, "#Team_Select") == 0)
         bots[bot_index].BotEyes.BotHUD.menu_notify = MSG_CS_TEAMSELECT_MAINMENU; // team select menu
      else if (strcmp ((char *) p, "#Terrorist_Select") == 0)
         bots[bot_index].BotEyes.BotHUD.menu_notify = MSG_CS_TEAMSELECT_TERRMENU; // terrorist menu
      else if (strcmp ((char *) p, "#CT_Select") == 0)
         bots[bot_index].BotEyes.BotHUD.menu_notify = MSG_CS_TEAMSELECT_COUNTERMENU; // counter-terrorist menu
   }
}


void BotClient_CS_WeaponList (void *p, int bot_index)
{
   // this message is sent when a client joins the game. All of the weapons are sent to him with
   // the weapon ID and information about what ammo is used.

   static weapon_t weapon;

   if (messagestate == 0)
      strcpy (weapon.szClassname, (char *) p); // get the name of that weapon
   else if (messagestate == 1)
      weapon.iAmmo1 = *(int *) p; // get the default primary ammo amount
   else if (messagestate == 2)
      weapon.iAmmo1Max = *(int *) p; // get the maximum primary ammo amount
   else if (messagestate == 3)
      weapon.iAmmo2 = *(int *) p; // get the default secondary ammo amount
   else if (messagestate == 4)
      weapon.iAmmo2Max = *(int *) p; // get the maximum secondary ammo amount
   else if (messagestate == 5)
      weapon.iSlot = *(int *) p; // get the HUD slot for this weapon
   else if (messagestate == 6)
      weapon.iPosition = *(int *) p; // get the position in the HUD slot
   else if (messagestate == 7)
      weapon.iId = *(int *) p; // get the weapon ID number
   else if (messagestate == 8)
   {
      weapon.iFlags = *(int *) p; // get the flags for the weapon (what the hell is that ?)
      weapon_defs[weapon.iId] = weapon; // we can now remember this weapon
   }
}


void BotClient_CS_CurrentWeapon (void *p, int bot_index)
{
   // this message is sent when a weapon is selected (either by the bot chosing a weapon or by
   // the server auto assigning the bot a weapon)

   static int iState;
   static int iId;

   if (messagestate == 0)
      iState = *(int *) p; // get the state of the current weapon
   else if (messagestate == 1)
      iId = *(int *) p; // get the weapon ID of current weapon
   else if (messagestate == 2)
   {
      int iClip = *(int *) p; // get the ammo currently in the clip for this weapon

      // though there are 32 weapons maximum allowed, we're never sure message is not erroneous
      if (iId < 32)
      {
         bots[bot_index].BotHand.weapons |= (1 << iId); // set this weapon bit in the bot's hand

         // TODO: figure out what iState means here
         if (iState == 1)
         {
            bots[bot_index].BotHand.weapon_id = iId;
            bots[bot_index].BotHand.iClip = iClip;

            // update the ammo counts for this weapon...
            bots[bot_index].BotHand.iAmmo1 = bots[bot_index].BotHand.ammo[weapon_defs[iId].iAmmo1];
            bots[bot_index].BotHand.iAmmo2 = bots[bot_index].BotHand.ammo[weapon_defs[iId].iAmmo2];
         }
      }
   }
}


void BotClient_CS_AmmoX (void *p, int bot_index)
{
   // this message is sent whenever ammo ammounts are adjusted (up or down)

   static int index;

   if (messagestate == 0)
      index = *(int *) p; // get the ammo index (for knowing what type of ammo it is)
   else if (messagestate == 1)
   {
      bots[bot_index].BotHand.ammo[index] = *(int *) p; // amount of ammo currently available

      // update the ammo counts for this weapon...
      int weapon_index = bots[bot_index].BotHand.weapon_id; // quick access to weapon index this ammo is for
      bots[bot_index].BotHand.iAmmo1 = bots[bot_index].BotHand.ammo[weapon_defs[weapon_index].iAmmo1];
      bots[bot_index].BotHand.iAmmo2 = bots[bot_index].BotHand.ammo[weapon_defs[weapon_index].iAmmo2];
   }
}


void BotClient_CS_AmmoPickup (void *p, int bot_index)
{
   // this message is sent when the bot picks up some ammo (AmmoX messages are also sent so this
   // message is probably not really necessary except it allows the HUD to draw pictures of ammo
   // that have been picked up. The bots don't really need pictures anyway...)

   static int index;

   if (messagestate == 0)
      index = *(int *) p; // get the ammo index (for knowing what type of ammo it is)
   else if (messagestate == 1)
   {
      bots[bot_index].BotHand.ammo[index] = *(int *) p; // amount of ammo currently available

      // update the ammo counts for this weapon...
      int weapon_index = bots[bot_index].BotHand.weapon_id; // quick access to weapon index this ammo is for
      bots[bot_index].BotHand.iAmmo1 = bots[bot_index].BotHand.ammo[weapon_defs[weapon_index].iAmmo1];
      bots[bot_index].BotHand.iAmmo2 = bots[bot_index].BotHand.ammo[weapon_defs[weapon_index].iAmmo2];
   }
}


void BotClient_CS_WeaponPickup (void *p, int bot_index)
{
   // this message gets sent when the bot picks up a weapon.

   // TODO: decide WHICH one, of the following, or of the weapon update force request with
   // UpdateClientData() in BotPreThink() is redundant. Basically they do the same thing...

   // set this weapon bit to indicate that we are carrying this weapon
   bots[bot_index].BotHand.weapons |= (1 << *(int *) p);

   // don't forget to load the weapon as soon as it is picked up
   bots[bot_index].BotLegs.input_buttons.f_reload_time = CurrentTime () + 0.2;
}


void BotClient_CS_ItemPickup (void *p, int bot_index)
{
   // this message gets sent when the bot picks up an item, like a battery or a healthkit (but
   // since accessing health and armor data through the entity pointer is a lot easier, don't do
   // anything here)
}


void BotClient_CS_Health (void *p, int bot_index)
{
   // this message gets sent when the bots health changes (but since accessing health and armor
   // data through the entity pointer is a lot easier, don't do anything here)
}


void BotClient_CS_Battery (void *p, int bot_index)
{
   // this message gets sent when the bots armor changes (but since accessing health and armor
   // data through the entity pointer is a lot easier, don't do anything here)
}


void BotClient_CS_Damage (void *p, int bot_index)
{
   // this message gets sent when the bots are getting damaged

   static int damage_armor;
   static int damage_taken;
   static int damage_bits; // type of damage being done
   static vector damage_origin, bot_angles;

   if (messagestate == 0)
      damage_armor = *(int *) p; // get the amount of damage inflicted to armor
   else if (messagestate == 1)
      damage_taken = *(int *) p; // get the amount of damage inflicted to health
   else if (messagestate == 2)
      damage_bits = *(int *) p; // get the type of damage it is (bullet, fall, heat, etc...)
   else if (messagestate == 3)
      damage_origin.x = *(float *) p; // get the x coordinate of the damage origin vector
   else if (messagestate == 4)
      damage_origin.y = *(float *) p; // get the y coordinate of the damage origin vector
   else if (messagestate == 5)
   {
      damage_origin.z = *(float *) p; // get the z coordinate of the damage origin vector

      // if we've actually taken some damage...
      if ((damage_armor > 0) || (damage_taken > 0))
      {
         // TODO: remove this so that ANY type of damage affects the bots (at least emotically)
         if (damage_bits & (DMG_CRUSH | DMG_FREEZE | DMG_FALL | DMG_SHOCK | DMG_DROWN
                            | DMG_NERVEGAS | DMG_RADIATION | DMG_DROWNRECOVER | DMG_ACID
                            | DMG_SLOWBURN | DMG_SLOWFREEZE | 0xFF000000))
            return; // ignore certain types of damage
         else if ((CurrentDamageOf (bots[bot_index].pEntity) > 0)
                  && (strcmp ("worldspawn", ClassnameOf (DamageInflictorOf (bots[bot_index].pEntity))) == 0))
            return; // ignore damage if resulting from a fall (ignore damage bug workaround)

         // if the bot has no enemy and someone is shooting at him...
         if (IsNull (bots[bot_index].pBotEnemy))
         {
            // face the attacker
            bot_angles = AnglesOfVector (damage_origin - OriginOf (bots[bot_index].pEntity));

            BotSetIdealAngles (&bots[bot_index], bot_angles);
            bots[bot_index].f_reach_time = CurrentTime () + 0.5; // delay reaching point
         }

         // if no audio chat with this bot for 5 seconds, speak
         if (bots[bot_index].BotChat.f_sayaudio_time + 5.0 < CurrentTime ())
         {
            bots[bot_index].BotChat.b_sayaudio_takingdamage = TRUE;
            bots[bot_index].BotChat.f_sayaudio_time = CurrentTime () + RandomFloat (0.5, 3.0);
         }
      }
   }
}


void BotClient_CS_Money (void *p, int bot_index)
{
   // this message gets sent when the bots money amount changes

   // only take the first packet of a Money message (only want the new amount of money)
   if (messagestate == 0)
      bots[bot_index].bot_money = *(int *)p; // get the new amount of money
}


void BotClient_CS_ReloadSound (void *p, int bot_index)
{
   // this message gets sent when a reload sound is being heard

   static int sound_index;

   if (messagestate == 0)
      sound_index = *(int *) p; // get the index of the reload sound
   else if (messagestate == 1)
   {
      float volume;
      entity_t *pSender = EntityAtIndex (*(int *) p); // get the sender entity

      // compute the sound attenuation according to the distance (FIXME: this is arbitrary, and
      // probably highly inaccurate)
      volume = 1 - (GunOriginOf (pSender) - EyeOriginOf (bots[bot_index].pEntity)).Length () / 500;
      if (volume < 0.1)
         volume = 0.1; // consider that after 500 units far the bot hears almost nothing

      // dispatch that reload sound to the bot's ears (note that engine sound indexes start at 1)
      BotFeedEar (&bots[bot_index], &sounds[sound_index - 1], GunOriginOf (pSender), volume);
   }
}


void BotClient_CS_SayText (void *p, int bot_index)
{
   // this message gets sent when a chat message is sent

   // only take the second packet of a SayText message (only want the actual text string)
   if (messagestate == 1)
   {
      char message[120], sender_id_string[33];
      int player_index, start_pos, sender_id_length = 0;
      bool teamonly = FALSE;

      strcpy (message, (char *) p); // get the message
      start_pos = 1; // first character is the color set id, so skip it

      // does it end with a carriage return ?
      if (message[strlen (message) - 1] == '\n')
         message[strlen (message) - 1] = 0; // strip the carriage return

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
      for (player_index = 0; player_index < MaxClientsOnServer (); player_index++)
      {
         entity_t *pPlayer = PlayerAtIndex (player_index);

         if (IsNull (pPlayer))
            continue; // skip invalid players

         // what would this player name look like in a chat string ?
         strcpy (sender_id_string, NetnameOf (pPlayer));

         // curiously, team messages are not formatted the exact same way as the others...
         if (teamonly)
            strcat (sender_id_string, " :   "); // THREE spaces for team messages
         else
            strcat (sender_id_string, " :    "); // FOUR spaces for other messages

         // so, is it that guy whose name is written right here ?
         if (strncmp (&message[start_pos], sender_id_string, strlen (sender_id_string)) == 0)
         {
            // we've found the sender of the message
            start_pos += strlen (sender_id_string); // skip the name

            // remember this message, shuffle down the previous ones on the bot's 'screen'
            for (int i = MAX_CHAT_MESSAGES - 1; i > 0; i--)
            {
               strcpy (bots[bot_index].BotEyes.BotHUD.chat_line[i].sender, bots[bot_index].BotEyes.BotHUD.chat_line[i - 1].sender);
               strcpy (bots[bot_index].BotEyes.BotHUD.chat_line[i].text, bots[bot_index].BotEyes.BotHUD.chat_line[i - 1].text);
            }
            strcpy (bots[bot_index].BotEyes.BotHUD.chat_line[0].sender, NetnameOf (pPlayer));
            strcpy (bots[bot_index].BotEyes.BotHUD.chat_line[0].text, &message[start_pos]);
            UpperCase (bots[bot_index].BotEyes.BotHUD.chat_line[0].text); // convert the input string to uppercase

            bots[bot_index].BotEyes.BotHUD.new_chat_message = TRUE; // tells the bot someone just chatted
         }
      }
   }
}


void BotClient_CS_TextMsg (void *p, int bot_index)
{
   // this message gets sent when a client receives a radio message

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
            sprintf (sound_file, "radio/ct_coverme.wav"); // 'Cover me' radio message
         else if (strcmp ((char *) p, "#You_take_the_point") == 0)
            sprintf (sound_file, "radio/takepoint.wav"); // 'You take the point' radio message
         else if (strcmp ((char *) p, "#Hold_this_position") == 0)
            sprintf (sound_file, "radio/position.wav"); // 'Hold this position' radio message
         else if (strcmp ((char *) p, "#Regroup_team") == 0)
            sprintf (sound_file, "radio/regroup.wav"); // 'Regroup team' radio message
         else if (strcmp ((char *) p, "#Follow_me") == 0)
            sprintf (sound_file, "radio/followme.wav"); // 'Follow me' radio message
         else if (strcmp ((char *) p, "#Taking_fire") == 0)
            sprintf (sound_file, "radio/fireassis.wav"); // 'Taking fire, need backup' radio message

         // second radio menu
         else if (strcmp ((char *) p, "#Go_go_go") == 0)
            sprintf (sound_file, "radio/com_go.wav"); // 'Go Go Go' radio message
         else if (strcmp ((char *) p, "#Team_fall_back") == 0)
            sprintf (sound_file, "radio/fallback.wav"); // 'Team fall back' radio message
         else if (strcmp ((char *) p, "#Stick_together_team") == 0)
            sprintf (sound_file, "radio/sticktog.wav"); // 'Stick together team' radio message
         else if (strcmp ((char *) p, "#Get_in_position_and_wait") == 0)
            sprintf (sound_file, "radio/com_getinpos.wav"); // 'Stay in position and wait for my go' radio message
         else if (strcmp ((char *) p, "#Storm_the_front") == 0)
            sprintf (sound_file, "radio/stormfront.wav"); // 'Storm The Front' radio message
         else if (strcmp ((char *) p, "#Report_in_team") == 0)
            sprintf (sound_file, "radio/com_reportin.wav"); // 'Report In' radio message

         // third radio menu
         else if (strcmp ((char *) p, "#Affirmative") == 0)
            sprintf (sound_file, "radio/ct_affirm.wav"); // 'Affirmative' radio message
         else if (strcmp ((char *) p, "#Enemy_spotted") == 0)
            sprintf (sound_file, "radio/ct_enemys.wav"); // 'Enemy spotted' radio message
         else if (strcmp ((char *) p, "#Need_backup") == 0)
            sprintf (sound_file, "radio/ct_backup.wav"); // 'Need backup' radio message
         else if (strcmp ((char *) p, "#Sector_clear") == 0)
            sprintf (sound_file, "radio/clear.wav"); // 'Sector clear' radio message
         else if (strcmp ((char *) p, "#In_position") == 0)
            sprintf (sound_file, "radio/ct_inpos.wav"); // 'I'm in position' radio message
         else if (strcmp ((char *) p, "#Reporting_in") == 0)
            sprintf (sound_file, "radio/ct_reportingin.wav"); // 'Reporting in' radio message
         else if (strcmp ((char *) p, "#Get_out_of_there") == 0)
            sprintf (sound_file, "radio/blow.wav"); // 'Get outta here' radio message
         else if (strcmp ((char *) p, "#Negative") == 0)
            sprintf (sound_file, "radio/negative.wav"); // 'Negative' radio message
         else if (strcmp ((char *) p, "#Enemy_down") == 0)
            sprintf (sound_file, "radio/enemydown.wav"); // 'Negative' radio message

         // client-side radio messages (I miss "A hostage has been rescued", don't know how to catch it)
         else if (strcmp ((char *) p, "#Bomb_Planted") == 0)
            sprintf (sound_file, "radio/bombpl.wav"); // 'Bomb planted' radio message
         else if (strcmp ((char *) p, "#Bomb_Defused") == 0)
            sprintf (sound_file, "radio/bombdef.wav"); // 'Bomb defused' radio message
         else if (strcmp ((char *) p, "#Round_Draw") == 0)
            sprintf (sound_file, "radio/rounddraw.wav"); // 'Round draw' radio message
         else if (strcmp ((char *) p, "#Terrorists_Win") == 0)
            sprintf (sound_file, "radio/terwin.wav"); // 'Terrorists win' radio message
         else if (strcmp ((char *) p, "#CTs_Win") == 0)
            sprintf (sound_file, "radio/ctwin.wav"); // 'Counter-terrorists win' radio message

         // find the sound we want in the global list
         for (i = 0; i < sound_count; i++)
            if (strcmp (sounds[i].file_path, sound_file) == 0)
               radio_sound = &sounds[i]; // link a pointer to this sound's info slot

         // have we found the sound we want to dispatch ?
         if (radio_sound != NULL)
            BotFeedEar (&bots[bot_index], radio_sound, OriginOf (bots[bot_index].pEntity), 1.0); // bot will hear this radio sound
      }
   }
}


void BotClient_CS_ScreenFade (void *p, int bot_index)
{
   // this message gets sent when the bot is affected by a flashbang

   static int duration;

   if (messagestate == 0)
      duration = *(int *) p; // get the duration of the fading effect
   else if (messagestate == 1)
   {
      // compute the date under which the bot will be blinded (also skill dependent)
      bots[bot_index].BotEyes.f_blinded_time = CurrentTime () + (duration + *(int *) p) / 4096
                                               - bots[bot_index].pPersonality->skill;
   }
}


void BotClient_CS_StatusIcon (void *p, int bot_index)
{
   // this message gets sent when an icon appears on the bot's HUD

   static int icon_state;

   if (messagestate == 0)
      icon_state = *(int *) p; // get the messagestate of the icon (either off, lit, or blinking)
   else if (messagestate == 1)
   {
      // is it the C4 icon ?
      if (strcmp ((char *) p, "c4") == 0)
         bots[bot_index].BotEyes.BotHUD.bomb_icon_state = icon_state; // remember bomb icon messagestate

      // else is it the defuse kit icon ?
      else if (strcmp ((char *) p, "defuser") == 0)
         bots[bot_index].BotEyes.BotHUD.defuser_icon_state = icon_state; // remember defuse kit bomb icon messagestate
   }
}


void BotClient_CS_DeathMsgAll (void *p, int dont_use_me)
{
   // this message gets sent when a player gets killed (for displaying the A killed B icon).
   // Note this message is a broadcast message.

   static int killer_index;

   if (messagestate == 0)
      killer_index = *(int *) p; // get entity index of killer player
   else if (messagestate == 1)
   {
      // is this message about a bot being killed (p holds the victim entity index) ?
      if (IsABot (EntityAtIndex (*(int *) p)))
      {
         if ((killer_index == 0) || (killer_index == *(int *) p))
            bots[*(int *) p - 1].pKillerEntity = NULL; // bot killed by world (worldspawn) or by self...
         else
            bots[*(int *) p - 1].pKillerEntity = EntityAtIndex (killer_index); // store edict of killer
      }
   }
}


void BotClient_CS_TextMsgAll (void *p, int dont_use_me)
{
   // this message gets sent when a text message is displayed on the HUD of all players. It
   // triggers generally the playing of a radio sound, so let's feed the bots ears with it...

   static char sound_file[256];
   static sound_t *radio_sound;
   static int i;

   if (messagestate == 1)
   {
      // identify the radio sample and choose the sound file that will be played at the bot's
      // client-side radio messages (we may miss some here actually)
      if (strcmp ((char *) p, "#Bomb_Planted") == 0)
         sprintf (sound_file, "radio/bombpl.wav"); // 'Bomb planted' radio message
      else if (strcmp ((char *) p, "#Bomb_Defused") == 0)
         sprintf (sound_file, "radio/bombdef.wav"); // 'Bomb defused' radio message
      else if (strcmp ((char *) p, "#Round_Draw") == 0)
         sprintf (sound_file, "radio/rounddraw.wav"); // 'Round draw' radio message
      else if (strcmp ((char *) p, "#Terrorists_Win") == 0)
         sprintf (sound_file, "radio/terwin.wav"); // 'Terrorists win' radio message
      else if (strcmp ((char *) p, "#CTs_Win") == 0)
         sprintf (sound_file, "radio/ctwin.wav"); // 'Counter-terrorists win' radio message

      // find the sound we want in the global list
      for (i = 0; i < sound_count; i++)
         if (strcmp (sounds[i].file_path, sound_file) == 0)
            radio_sound = &sounds[i]; // link a pointer to this sound's info slot

      // have we found the sound we want to dispatch ?
      if (radio_sound != NULL)
      {
         // we set the sender of this radio sound as the bot himself, since it's a client sound
         for (i = 0; i < MaxClientsOnServer (); i++)
            BotFeedEar (&bots[i], radio_sound, OriginOf (bots[i].pEntity), 1.0); // bot will hear this radio sound
      }
   }
}
