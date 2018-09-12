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
// ASHEEP version
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


// This message is sent when a client joins the game.  All of the weapons
// are sent with the weapon ID and information about what ammo is used.
void BotClient_Asheep_WeaponList (void *p, int bot_index)
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
void BotClient_Asheep_CurrentWeapon (void *p, int bot_index)
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


// This message is sent whenever ammo amounts are adjusted (up or down).
void BotClient_Asheep_AmmoX (void *p, int bot_index)
{
   static int state = 0; // current state machine state
   static int index;
   int ammo_index;

   if (state == 0)
   {
      state++;
      index = *(int *) p; // ammo index (for type of ammo)
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


// This message is sent when the bot picks up some ammo (AmmoX messages are
// also sent so this message is probably not really necessary except it
// allows the HUD to draw pictures of ammo that have been picked up.  The
// bots don't really need pictures since they don't have any eyes anyway.
void BotClient_Asheep_AmmoPickup (void *p, int bot_index)
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
void BotClient_Asheep_WeaponPickup (void *p, int bot_index)
{
   // set this weapon bit to indicate that we are carrying this weapon
   bots[bot_index].bot_weapons |= (1 << *(int *) p);

   // don't forget to load the weapon as soon as it is picked up
   bots[bot_index].f_reload_time = gpGlobals->time + RANDOM_LONG (1.0, 2.0);
}


// This message gets sent when the bot picks up an item (like a healthkit)
void BotClient_Asheep_ItemPickup (void *p, int bot_index)
{
}


// This message gets sent when the bots health changes.
void BotClient_Asheep_Health (void *p, int bot_index)
{
   bots[bot_index].bot_health = *(int *)p; // health amount
}


// This message gets sent when the bots are getting damaged.
void BotClient_Asheep_Damage (void *p, int bot_index)
{
   static int state = 0; // current state machine state
   static int damage_taken;
   static int damage_bits; // type of damage being done
   static Vector damage_origin;

   if (state == 0)
      state++;
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

      if (damage_taken > 0)
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
            bots[bot_index].b_use_station = FALSE; // stop using wall-mounted stations
         }
      }
   }
}


// This message gets sent when a chat message is sent
void BotClient_Asheep_SayText (void *p, int bot_index)
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


// This message gets sent when the bots get killed
void BotClient_Asheep_DeathMsg (void *p, int bot_index)
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

      // is this message about a bot being killed?
      if (index != -1)
      {
         if ((killer_index == 0) || (killer_index == victim_index))
            bots[index].pKillerEntity = NULL; // bot killed by world (worldspawn) or by self...
         else
            bots[index].pKillerEntity = INDEXENT (killer_index); // store edict of killer
      }
   }
}


void BotClient_Asheep_ScreenFade (void *p, int bot_index)
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
      bots[bot_index].blinded_time = gpGlobals->time + length - 2.0;
   }
   else
      state++;
}
