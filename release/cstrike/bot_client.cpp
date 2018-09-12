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
// CSTRIKE version
//
// bot_client.cpp
//

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "bot_common.h"
#include "bot_specific.h"

// types of damage to ignore...
#define IGNORE_DAMAGE (DMG_CRUSH | DMG_FREEZE | DMG_FALL | DMG_SHOCK | \
                       DMG_DROWN | DMG_NERVEGAS | DMG_RADIATION | \
                       DMG_DROWNRECOVER | DMG_ACID | DMG_SLOWBURN | \
                       DMG_SLOWFREEZE | 0xFF000000)

extern bot_t bots[32];
extern bot_weapon_t weapon_defs[MAX_WEAPONS];
extern bool b_bomb_planted;
extern float f_bomb_explode_time;
extern bool roundend;



// This message is sent when the Counter-Strike VGUI menu is displayed.
void BotClient_CS_VGUI (void *p, int bot_index)
{
   static int state = 0; // current state machine state

   if (state == 0)	// ignore next 4 fields sent to function (only want menu codes)
   {
      if ((*(int *) p) == 2) // is it a team select menu ?
         bots[bot_index].menu_notify = MSG_CS_TEAMSELECT_MAINMENU;
      else if ((*(int *) p) == 26) // is is the terrorist model select menu ?
         bots[bot_index].menu_notify = MSG_CS_TEAMSELECT_TERRMENU;
      else if ((*(int *) p) == 27) // is is the counter-terrorist model select menu ?
         bots[bot_index].menu_notify = MSG_CS_TEAMSELECT_COUNTERMENU;
   }
   else if (state < 4)
      state++; // ignore next 3 fields
   else
      state = 0; // reset state machine
}


// This message is sent when a menu is being displayed in Counter-Strike.
void BotClient_CS_ShowMenu (void *p, int bot_index)
{
   static int state = 0; // current state machine state

   if (state < 3)
   {
      state++; // ignore first 3 fields of message
      return;
   }
   else
   {
      if (strcmp ((char *) p, "#Team_Select") == 0) // is is the team select menu ?
         bots[bot_index].menu_notify = MSG_CS_TEAMSELECT_MAINMENU;
      else if (strcmp ((char *) p, "#Terrorist_Select") == 0) // is is the T model select ?
         bots[bot_index].menu_notify = MSG_CS_TEAMSELECT_TERRMENU;
      else if (strcmp ((char *) p, "#CT_Select") == 0) // is is the CT model select menu ?
         bots[bot_index].menu_notify = MSG_CS_TEAMSELECT_COUNTERMENU;
      state = 0; // reset state machine
   }
}


// This message is sent when a client joins the game.  All of the weapons
// are sent with the weapon ID and information about what ammo is used.
void BotClient_CS_WeaponList (void *p, int bot_index)
{
   static int state = 0; // current state machine state
   static bot_weapon_t bot_weapon;

   if (state == 0)
   {
      state++;
      strcpy (bot_weapon.szClassname, (char *) p);
   }
   else if (state == 1)
   {
      state++;
      bot_weapon.iAmmo1 = *(int *) p; // ammo index 1
   }
   else if (state == 2)
   {
      state++;
      bot_weapon.iAmmo1Max = *(int *) p; // max ammo1
   }
   else if (state == 3)
   {
      state++;
      bot_weapon.iAmmo2 = *(int *) p; // ammo index 2
   }
   else if (state == 4)
   {
      state++;
      bot_weapon.iAmmo2Max = *(int *) p; // max ammo2
   }
   else if (state == 5)
   {
      state++;
      bot_weapon.iSlot = *(int *) p; // slot for this weapon
   }
   else if (state == 6)
   {
      state++;
      bot_weapon.iPosition = *(int *) p; // position in slot
   }
   else if (state == 7)
   {
      state++;
      bot_weapon.iId = *(int *) p; // weapon ID
   }
   else if (state == 8)
   {
      state = 0;
      bot_weapon.iFlags = *(int *) p; // flags for weapon (WTF???)

      // store away this weapon with it's ammo information...
      weapon_defs[bot_weapon.iId] = bot_weapon;
   }
}


// This message is sent when a weapon is selected (either by the bot chosing
// a weapon or by the server auto assigning the bot a weapon).
void BotClient_CS_CurrentWeapon (void *p, int bot_index)
{
   static int state = 0; // current state machine state
   static int iState;
   static int iId;
   static int iClip;

   if (state == 0)
   {
      state++;
      iState = *(int *) p; // state of the current weapon
   }
   else if (state == 1)
   {
      state++;
      iId = *(int *) p; // weapon ID of current weapon
   }
   else if (state == 2)
   {
      state = 0;

      iClip = *(int *) p; // ammo currently in the clip for this weapon

      if (iId <= 31)
      {
         bots[bot_index].bot_weapons |= (1 << iId); // set this weapon bit

         if (iState == 1)
         {
            bots[bot_index].current_weapon.iId = iId;
            bots[bot_index].current_weapon.iClip = iClip;

            // update the ammo counts for this weapon...
            bots[bot_index].current_weapon.iAmmo1 = bots[bot_index].m_rgAmmo[weapon_defs[iId].iAmmo1];
            bots[bot_index].current_weapon.iAmmo2 = bots[bot_index].m_rgAmmo[weapon_defs[iId].iAmmo2];
         }
      }
   }
}


// This message is sent whenever ammo ammounts are adjusted (up or down).
void BotClient_CS_AmmoX (void *p, int bot_index)
{
   static int state = 0; // current state machine state
   static int index;
   int weapon_index;

   if (state == 0)
   {
      state++;
      index = *(int *) p; // ammo index (for type of ammo)
   }
   else if (state == 1)
   {
      state = 0;
      bots[bot_index].m_rgAmmo[index] = *(int *) p; // the amount of ammo currently available

      // update the ammo counts for this weapon...
      weapon_index = bots[bot_index].current_weapon.iId;
      bots[bot_index].current_weapon.iAmmo1 = bots[bot_index].m_rgAmmo[weapon_defs[weapon_index].iAmmo1];
      bots[bot_index].current_weapon.iAmmo2 = bots[bot_index].m_rgAmmo[weapon_defs[weapon_index].iAmmo2];
   }
}


// This message is sent when the bot picks up some ammo (AmmoX messages are
// also sent so this message is probably not really necessary except it
// allows the HUD to draw pictures of ammo that have been picked up. The
// bots don't really need pictures since they don't have any eyes anyway.
void BotClient_CS_AmmoPickup (void *p, int bot_index)
{
   static int state = 0; // current state machine state
   static int index;
   int ammo_index;

   if (state == 0)
   {
      state++;
      index = *(int *) p;
   }
   else if (state == 1)
   {
      state = 0;
      bots[bot_index].m_rgAmmo[index] = *(int *) p; // the ammount of ammo currently available

      // update the ammo counts for this weapon...
      ammo_index = bots[bot_index].current_weapon.iId;
      bots[bot_index].current_weapon.iAmmo1 = bots[bot_index].m_rgAmmo[weapon_defs[ammo_index].iAmmo1];
      bots[bot_index].current_weapon.iAmmo2 = bots[bot_index].m_rgAmmo[weapon_defs[ammo_index].iAmmo2];
   }
}


// This message gets sent when the bot picks up a weapon.
void BotClient_CS_WeaponPickup (void *p, int bot_index)
{
   // set this weapon bit to indicate that we are carrying this weapon
   bots[bot_index].bot_weapons |= (1 << *(int *) p);

   // don't forget to load the weapon as soon as it is picked up
   bots[bot_index].f_reload_time = gpGlobals->time + RANDOM_LONG (1.0, 2.0);
}


// This message gets sent when the bot picks up an item (like a battery or a healthkit)
void BotClient_CS_ItemPickup (void *p, int bot_index)
{
   return;
}


// This message gets sent when the bots health changes.
void BotClient_CS_Health (void *p, int bot_index)
{
   bots[bot_index].bot_health = *(int *) p; // health amount
}


// This message gets sent when the bots armor changes.
void BotClient_CS_Battery (void *p, int bot_index)
{
   bots[bot_index].bot_armor = *(int *) p; // armor amount
}


// This message gets sent when the bots are getting damaged.
void BotClient_CS_Damage (void *p, int bot_index)
{
   static int state = 0; // current state machine state
   static int damage_armor;
   static int damage_taken;
   static int damage_bits; // type of damage being done
   static Vector damage_origin;

   if (state == 0)
   {
      state++;
      damage_armor = *(int *) p;
   }
   else if (state == 1)
   {
      state++;
      damage_taken = *(int *) p;
   }
   else if (state == 2)
   {
      state++;
      damage_bits = *(int *) p;
   }
   else if (state == 3)
   {
      state++;
      damage_origin.x = *(float *) p;
   }
   else if (state == 4)
   {
      state++;
      damage_origin.y = *(float *) p;
   }
   else if (state == 5)
   {
      state = 0;
      damage_origin.z = *(float *) p;

      if ((damage_armor > 0) || (damage_taken > 0))
      {
         // ignore certain types of damage
         if (damage_bits & IGNORE_DAMAGE)
            return;

         // ignore damage if resulting from a fall (IGNORE_DAMAGE bug workaround)
         if (bots[bot_index].fall_time + 0.25 > gpGlobals->time)
            return;

         // if the bot has no enemy and someone is shooting at him...
         if (bots[bot_index].pBotEnemy == NULL)
         {
            // face the attacker
            Vector bot_angles = UTIL_VecToAngles (damage_origin - bots[bot_index].pEdict->v.origin);
            BotSetIdealYaw (&bots[bot_index], bot_angles.y);
            bots[bot_index].f_reach_time = gpGlobals->time + 0.5; // delay reaching point
         }

         // if no audio chat with this bot for 5 seconds, speak
         if (bots[bot_index].f_bot_sayaudio_time + 5.0 < gpGlobals->time)
         {
            bots[bot_index].BotChat.b_sayaudio_takingdamage = TRUE;
            bots[bot_index].f_bot_sayaudio_time = gpGlobals->time + RANDOM_FLOAT (0.5, 3.0);
         }
      }
   }
}


// This message gets sent when the bots money ammount changes
void BotClient_CS_Money (void *p, int bot_index)
{
   static int state = 0; // current state machine state

   if (state == 0)
   {
      state++;
      bots[bot_index].bot_money = *(int *)p; // amount of money
   }
   else
      state = 0; // ingore this field
}


// This message gets sent when a chat message is sent
void BotClient_CS_SayText (void *p, int bot_index)
{
   // TAKEN FROM ALISTAIR STEWART'S TEAMBOT -- NEED TO BE CLEANED UP -- CURRENTLY DISABLED

/*	static int state = 0; // current state machine state

	if (state < 1)
		state++;
	else
	{
		char message[120] = "", temp_string[120] = "";
		strcpy(message, (char *)p);

		// message[0] will contain a non-ascii character, I think it's some sort of
		// HL chat-text formatting character... so start search at message[1]
		if (message[1] != NULL)
		{
			// Check who sent this message,
			for (int i = 0; i < 32; i++)
			{
				// don't try to access a NULL pointer!!
				if (clients[i] == NULL)
					continue;

				// Add on the ":" to the end of the bot name to take care of the problem
				// of players having similar names (eg. 'eLiTe' and 'eLiTe[nn]')
				strcpy(temp_string, STRING(clients[i]->v.netname));
				strcat(temp_string, " : ");

				// position that we start reading chat from (after team/name)
				int start_pos = strlen(temp_string);

				if (!strncmp(temp_string, &message[1], start_pos))
				{
					// don't reply to our own message
					if (clients[i] == bots[bot_index].pEdict)
						return;

					// skip leading spaces
					while (message[start_pos] == ' ')
					{
						start_pos++;
					}

					if (!strncmp("follow me", &message[start_pos], strlen("follow me")))
					{
						if (FVisible(clients[i]->v.origin, bots[bot_index].pEdict))
						{
							// Give 'affirmative' radio message
							bots[bot_index].radio_time = gpGlobals->time + RANDOM_FLOAT(1, 2);
							bots[bot_index].give_radio_command = TRUE;
							strcpy(bots[bot_index].radio_commands[0], "3");
							strcpy(bots[bot_index].radio_commands[1], "1");

							bots[bot_index].pBotUser = clients[i];
							bots[bot_index].f_bot_use_time = gpGlobals->time; 
						}
						else
						{
							// Give 'negative' radio message
							bots[bot_index].radio_time = gpGlobals->time + RANDOM_FLOAT(1, 2);
							bots[bot_index].give_radio_command = TRUE;
							strcpy(bots[bot_index].radio_commands[0], "3");
							strcpy(bots[bot_index].radio_commands[1], "8");
						}
						return;
					}

					// Keep this for chatting replies
					/*if (bots[bot_index].chat_time < gpGlobals->time)
					{
						strcpy(temp_string, "Hi ");
						strcat(temp_string, STRING(clients[i]->v.netname));
						strcat(temp_string, "!");
						bots[bot_index].team_chat = 0;
						bots[bot_index].chat_time = gpGlobals->time + RANDOM_FLOAT(1, 5);
						strcpy(bots[bot_index].chat_string, temp_string);
					}*//*
				}
			}
		}

		state = 0;
	}*/
}


// This message gets sent when a client receives a radio message
void BotClient_CS_TextMsg (void *p, int bot_index)
{
   static int state = 0; // current state machine state
   static int sender_index = -1;

   if (strcmp ((char *) p, "#Game_radio") == 0)
      state++; // if this message is a radio message, let's check what sort of message it is

   else if (state == 1)
   {
      sender_index = -1; // don't forget to reset sender_index as it is static

      // cycle through all players
      for (int client_index = 1; client_index <= gpGlobals->maxClients; client_index++)
      {
         edict_t *pPlayer = INDEXENT (client_index);

         // skip invalid players and skip self (i.e. this bot)
         if ((pPlayer) && (!pPlayer->free) && (pPlayer != bots[bot_index].pEdict))
         {
            // is this player the sender of the message ?
            if (strcmp ((char *) p, STRING (pPlayer->v.netname)) == 0)
            {
               sender_index = client_index; // save the sender index
               break;
            }
         }
      }

      state++;
   }
   else if (state == 2)
   {
      // if the sender is a valid client, let's listen to him
      if (sender_index != -1)
      {
         if (strcmp ((char *) p, "#Follow_me") == 0) // 'Follow Me' radio command
         {
            // check if bot can see the caller
            if (BotGetIdealAimVector (&bots[bot_index], INDEXENT (sender_index)) != Vector (0, 0, 0))
            {
               bots[bot_index].bot_order = BOT_ORDER_FOLLOW; // let the bot know he has been ordered something
               bots[bot_index].pAskingEntity = INDEXENT (sender_index); // remember asker
               bots[bot_index].f_order_time = gpGlobals->time; // remember when the order came
            }
         }
         else if (strcmp ((char *) p, "#Hold_this_position") == 0) // 'Hold This Position' radio command
         {
            // check if bot can see the caller
            if (BotGetIdealAimVector (&bots[bot_index], INDEXENT (sender_index)) != Vector (0, 0, 0))
            {
               bots[bot_index].bot_order = BOT_ORDER_STAY; // let the bot know he has been ordered something
               bots[bot_index].pAskingEntity = INDEXENT (sender_index); // remember asker
               bots[bot_index].f_order_time = gpGlobals->time; // remember when the order came
            }
         }
         else if ((strcmp ((char *) p, "#Go_go_go") == 0) // 'Go Go Go' radio command
                  || (strcmp ((char *) p, "#Storm_the_front") == 0)) // 'Storm The Front' radio command
         {
            bots[bot_index].bot_order = BOT_ORDER_GO; // let the bot know he has been ordered something
            bots[bot_index].pAskingEntity = INDEXENT (sender_index); // remember asker
            bots[bot_index].f_order_time = gpGlobals->time; // remember when the order came
         }
         else if (strcmp ((char *) p, "#Report_in_team") == 0) // 'Report In' radio command
         {
            bots[bot_index].bot_order = BOT_ORDER_REPORT; // let the bot know he has been ordered something
            bots[bot_index].pAskingEntity = INDEXENT (sender_index); // remember asker
            bots[bot_index].f_order_time = gpGlobals->time; // remember when the order came
         }
      }

      state = 0; // reset state machine state
   }
}


// This message gets sent when a text message is displayed on the HUD of all players
void BotClient_CS_TextMsgAll (void *p, int bot_index)
{
   // catch the "bomb planted" message
   if (strcmp ((char *) p, "#Bomb_Planted") == 0)
   {
      b_bomb_planted = TRUE;
      f_bomb_explode_time = gpGlobals->time + CVAR_GET_FLOAT ("mp_c4timer");

      // cycle through all bot slots
      for (int index = 0; index < 32; index++)
      {
         // is this slot used ?
         if (bots[bot_index].is_active && (bots[bot_index].pEdict != NULL))
         {
            // is this bot a Counter-Terrorist and is he camping right now ?
            if ((GetTeam (bots[bot_index].pEdict) == CS_COUNTER_TERRORIST) && (bots[bot_index].f_camp_time > gpGlobals->time))
            {
               bots[bot_index].v_place_to_keep = Vector (0, 0, 0); // forget this place
               bots[bot_index].f_camp_time = gpGlobals->time; // don't camp anymore
               bots[bot_index].f_rush_time = f_bomb_explode_time; // rush until the bomb explodes
            }
         }
      }
   }

   // catch the "bomb defused" message
   else if (strcmp ((char *) p, "#Bomb_Defused") == 0)
   {
      b_bomb_planted = FALSE;
      f_bomb_explode_time = 0;
   }

   // catch the "round draw" message
   else if (strcmp ((char *) p, "#Round_Draw") == 0)
   {
      b_bomb_planted = FALSE;
      f_bomb_explode_time = 0;
   }

   // catch the "terrorists win" end of round message
   else if ((strcmp ((char *) p, "#Terrorists_Win") == 0)
            || (strcmp ((char *) p, "#Target_Bombed") == 0)
            || (strcmp ((char *) p, "#Terrorists_Escaped") == 0)
            || (strcmp ((char *) p, "#VIP_Assassinated") == 0)
            || (strcmp ((char *) p, "#VIP_Not_Escaped") == 0)
            || (strcmp ((char *) p, "#Hostages_Not_Rescued") == 0))
   {
      b_bomb_planted = FALSE;
      f_bomb_explode_time = 0;
   }

   // catch the "counter-terrorists win" end of round message
   else if ((strcmp ((char *) p, "#CTs_Win") == 0)
            || (strcmp ((char *) p, "#Target_Saved") == 0)
            || (strcmp ((char *) p, "#Terrorists_Not_Escaped") == 0)
            || (strcmp ((char *) p, "#CTs_PreventEscape") == 0)
            || (strcmp ((char *) p, "#VIP_Escaped") == 0)
            || (strcmp ((char *) p, "#All_Hostages_Rescued") == 0))
   {
      b_bomb_planted = FALSE;
      f_bomb_explode_time = 0;
   }

   return;
}


// This message gets sent when the bots get killed
void BotClient_CS_DeathMsg (void *p, int bot_index)
{
   static int state = 0; // current state machine state
   static int killer_index;
   static int victim_index;
   static int index;

   if (state == 0)
   {
      state++;
      killer_index = *(int *) p; // ENTINDEX() of killer
   }
   else if (state == 1)
   {
      state++;
      victim_index = *(int *) p; // ENTINDEX() of victim
   }
   else if (state == 2)
   {
      state = 0;
      index = UTIL_GetBotIndex (INDEXENT (victim_index));

      // is this message about a bot being killed ?
      if (index != -1)
      {
         if ((killer_index == 0) || (killer_index == victim_index))
            bots[index].pKillerEntity = NULL; // bot killed by world (worldspawn) or by self...
         else
            bots[index].pKillerEntity = INDEXENT (killer_index); // store edict of killer
      }
   }
}


// This message gets sent when the bot is affected by a flashbang
void BotClient_CS_ScreenFade (void *p, int bot_index)
{
   static int state = 0; // current state machine state
   static int duration;
   static int hold_time;
   static int fade_flags;
   int length;

   if (state == 0)
   {
      state++;
      duration = *(int *) p;
   }
   else if (state == 1)
   {
      state++;
      hold_time = *(int *) p;
   }
   else if (state == 2)
   {
      state++;
      fade_flags = *(int *) p;
   }
   else if (state == 6)
   {
      state = 0;
      length = (duration + hold_time) / 4096;
      bots[bot_index].blinded_time = gpGlobals->time + length - RANDOM_FLOAT (2.0, 5.0);
   }
   else
      state++;
}


// This message gets sent when an icon appears on the bot's HUD
void BotClient_CS_StatusIcon (void *p, int bot_index)
{
   static int state = 0; // current state machine state
   static int icon_status;

   if (state == 0)
   {
      icon_status = *(int *) p;
      state++;
   }
   else if (state == 1)
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
            bots[bot_index].b_has_defuse_kit = TRUE; // defuser icon is present
         else
            bots[bot_index].b_has_defuse_kit = FALSE; // defuser icon is (no more) present
      }

      if (icon_status == 0)
         state = 0;
      else
         state++;
   }
   else if (state == 4)
      state = 0;
   else
      state++;
}
