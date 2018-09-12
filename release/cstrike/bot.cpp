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
// bot.cpp
//

#include <sys/types.h>
#include <sys/stat.h>

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "bot_common.h"
#include "bot_specific.h"

extern HINSTANCE h_Library;
extern bool is_dedicated_server;
extern edict_t *listenserver_edict;
extern bot_t bots[32];
extern char bot_names[100][32];
extern char bot_logos[100][32];
extern int bot_nationalities[100];
extern int bot_skills[100];
extern int number_names;
extern int weapon_prices[20][2];
extern bool b_observer_mode;
extern bool b_botdontshoot;
extern bool b_botdontfind;
extern float f_radiotime;
extern bool b_bomb_map;
extern bool b_bomb_planted;
extern float f_bomb_explode_time;


inline edict_t *CREATE_FAKE_CLIENT (const char *netname)
{
   return (*g_engfuncs.pfnCreateFakeClient) (netname);
}

inline char *GET_INFOBUFFER (edict_t *e)
{
   return (*g_engfuncs.pfnGetInfoKeyBuffer) (e);
}

inline char *GET_INFO_KEY_VALUE (char *infobuffer, char *key)
{
   return (g_engfuncs.pfnInfoKeyValue (infobuffer, key));
}

inline void SET_CLIENT_KEY_VALUE (int clientIndex, char *infobuffer, char *key, char *value)
{
   (*g_engfuncs.pfnSetClientKeyValue) (clientIndex, infobuffer, key, value);
}


// this is the LINK_ENTITY_TO_CLASS function that creates a player (bot)
void player (entvars_t *pev)
{
   static LINK_ENTITY_FUNC otherClassName = NULL;
   if (otherClassName == NULL)
      otherClassName = (LINK_ENTITY_FUNC) GetProcAddress (h_Library, "player");
   if (otherClassName != NULL)
      (*otherClassName) (pev);
}


void BotReset (bot_t *pBot)
{
   pBot->msecnum = 0;
   pBot->msecdel = 0.0;
   pBot->msecval = 0.0;

   pBot->bot_health = 0;
   pBot->bot_armor = 0;
   pBot->bot_weapons = 0;
   pBot->blinded_time = 0.0;

   pBot->BotMove.f_max_speed = CVAR_GET_FLOAT ("sv_maxspeed");

   pBot->f_find_item_time = 0.0;

   pBot->ladder_dir = LADDER_UNKNOWN;
   pBot->f_start_use_ladder_time = 0.0;
   pBot->f_end_use_ladder_time = 0.0;

   if (RANDOM_LONG (1, 100) < 33)
      pBot->b_is_fearful = TRUE;
   else
      pBot->b_is_fearful = FALSE;
   pBot->BotMove.b_is_walking = FALSE;
   pBot->BotMove.b_emergency_walkback = FALSE;
   pBot->BotMove.f_forward_time = 0.0;
   pBot->BotMove.f_backwards_time = 0.0;
   pBot->BotMove.f_jump_time = 0.0;
   pBot->BotMove.f_duck_time = 0.0;
   pBot->BotMove.f_strafeleft_time = 0.0;
   pBot->BotMove.f_straferight_time = 0.0;

   pBot->f_exit_water_time = 0.0;

   pBot->pBotEnemy = NULL;
   pBot->v_lastseenenemy_position = Vector (0, 0, 0);
   pBot->f_see_enemy_time = 0.0;
   pBot->f_lost_enemy_time = 0.0;
   pBot->f_reload_time = -1;
   pBot->f_aim_adjust_time = 0.0;
   pBot->pBotUser = NULL;
   pBot->f_bot_use_time = 0.0;
   pBot->f_randomturn_time = 0.0;
   pBot->f_bot_saytext_time = 0.0;
   pBot->f_bot_sayaudio_time = 0.0;
   pBot->BotChat.b_saytext_help = FALSE;
   pBot->b_help_asked = FALSE;
   pBot->BotChat.b_saytext_killed = FALSE;
   pBot->BotChat.b_sayaudio_affirmative = FALSE;
   pBot->BotChat.b_sayaudio_alert = FALSE;
   pBot->BotChat.b_sayaudio_attacking = FALSE;
   pBot->BotChat.b_sayaudio_firstspawn = FALSE;
   pBot->BotChat.b_sayaudio_inposition = FALSE;
   pBot->BotChat.b_sayaudio_negative = FALSE;
   pBot->BotChat.b_sayaudio_report = FALSE;
   pBot->BotChat.b_sayaudio_reporting = FALSE;
   pBot->BotChat.b_sayaudio_seegrenade = FALSE;
   pBot->BotChat.b_sayaudio_takingdamage = FALSE;
   pBot->BotChat.b_sayaudio_throwgrenade = FALSE;
   pBot->BotChat.b_sayaudio_victory = FALSE;

   pBot->f_shoot_time = gpGlobals->time;
   pBot->f_primary_charging = -1.0;
   pBot->charging_weapon_id = 0;

   pBot->buy_state = 0;
   pBot->f_buy_time = gpGlobals->time + RANDOM_FLOAT (2.0, 5.0);
   pBot->f_rush_time = gpGlobals->time + RANDOM_FLOAT (15.0, 45.0);
   pBot->f_pause_time = pBot->f_buy_time;
   pBot->f_sound_update_time = 0;

   pBot->f_find_goal_time = 0;
   pBot->v_goal = Vector (0, 0, 0);
   pBot->v_place_to_keep = Vector (0, 0, 0);
   pBot->f_place_time = 0;
   pBot->f_camp_time = 0;
   pBot->f_reach_time = 0;
   pBot->f_samplefov_time = 0;
   pBot->v_reach_point = Vector (0,0,0);
   pBot->f_turncorner_time = gpGlobals->time + 5.0;

   pBot->bot_order = BOT_ORDER_NOORDER;
   pBot->f_order_time = 0;
   pBot->b_interact = FALSE;
   pBot->f_interact_time = 0;
   pBot->b_lift_moving = FALSE;
   pBot->f_spraying_logo_time = 0;
   pBot->b_logo_sprayed = FALSE;
   pBot->b_is_planting = FALSE;
   pBot->b_is_defusing = FALSE;
   pBot->b_has_defuse_kit = FALSE;

   memset (&(pBot->current_weapon), 0, sizeof (pBot->current_weapon));
   memset (&(pBot->m_rgAmmo), 0, sizeof (pBot->m_rgAmmo));

   // if this bot slot has an associated edict (i.e. this bot "exists" for the engine)
   if (pBot->pEdict != NULL)
   {
      float spawn_angle = RANDOM_FLOAT (-180, 180); // choose a random spawn angle
      BotSetIdealPitch (pBot, 0); // reset pitch to 0 (level horizontally)
      BotSetIdealYaw (pBot, spawn_angle); // set his spawn angle
      BotSetViewAngles (pBot, Vector (0, spawn_angle, 0)); // set his view angles
   }
}


void BotCreate (const char *name, const char *logo, int nationality, int skill, int team, int bot_class)
{
   edict_t *pBotEdict;
   bot_t *pBot;
   char c_name[32];
   char c_logo[32];
   int i_nationality;
   int i_skill;
   int i_team;
   int i_class;
   int index, i;

   // first prevent some CVARs to reach forbidden values
   if (CVAR_GET_FLOAT ("racc_botforceteam") < -1)
      CVAR_SET_FLOAT ("racc_botforceteam", -1); // force racc_botforceteam in bounds
   if (CVAR_GET_FLOAT ("racc_botforceteam") > 3)
      CVAR_SET_FLOAT ("racc_botforceteam", 3); // force racc_botforceteam in bounds
   if (CVAR_GET_FLOAT ("racc_defaultbotnationality") < 0)
      CVAR_SET_FLOAT ("racc_defaultbotnationality", 0); // force racc_defaultbotskill in bounds
   if (CVAR_GET_FLOAT ("racc_defaultbotnationality") > 4)
      CVAR_SET_FLOAT ("racc_defaultbotnationality", 4); // force racc_defaultbotskill in bounds
   if (CVAR_GET_FLOAT ("racc_defaultbotskill") < 1)
      CVAR_SET_FLOAT ("racc_defaultbotskill", 1); // force racc_defaultbotskill in bounds
   if (CVAR_GET_FLOAT ("racc_defaultbotskill") > 5)
      CVAR_SET_FLOAT ("racc_defaultbotskill", 5); // force racc_defaultbotskill in bounds

   // have we been specified parameters ?
   if ((name != NULL) && (*name != 0))
   {
      strcpy (c_name, name); // we have a name, so use it

      if ((logo != NULL) && (*logo != 0))
         strcpy (c_logo, logo); // if we have a logo, use it too
      else
         strcpy (c_logo, "lambda.bmp"); // else use the default logo

      if ((nationality >= 0) && (nationality <= 4))
         i_nationality = nationality; // if we have a nationality, use it too
      else
         i_nationality = CVAR_GET_FLOAT ("racc_defaultbotnationality"); // else use the default nationality

      if ((skill >= 1) && (skill <= 5))
         i_skill = skill; // if we have a skill, use it too
      else
         i_skill = CVAR_GET_FLOAT ("racc_defaultbotskill"); // else use the default skill

      if ((skill >= 1) && (skill <= 5))
         i_team = team; // if we have a team, use it too
      else
         i_team = -1; // else use auto-assign

      if ((skill >= 1) && (skill <= 5))
         i_class = bot_class; // if we have a class, use it too
      else
         i_class = -1; // else use auto-assign
   }

   // else have we a list ?
   else if (number_names > 0)
   {
      // if so, see the names that are used and add one that is not used yet
      int count_used = 0;
      bool bot_names_used[100];

      // reset used names flag array
      for (index = 0; index < number_names; index++)
         bot_names_used[index] = FALSE;

      // cycle all bot slots
      for (i = 0; i < 32; i++)
      {
         // is this bot active ?
         if (bots[i].is_active && (bots[i].pEdict != NULL))
         {
            // cycle all names slots
            for (index = 0; index < number_names; index++)
            {
               // does the bot have the same name that this name slot ?
               if (strcmp (STRING (bots[i].pEdict->v.netname), bot_names[index]) == 0)
               {
                  bot_names_used[index] = TRUE; // this name is used, so flag it
                  count_used++; // increment the used names counter
               }
            }
         }
      }

      // if all the names are used, revert them to non-used
      if (count_used == number_names)
         for (index = 0; index < number_names; index++)
            bot_names_used[index] = FALSE;

      // pick up one that isn't
      do index = RANDOM_LONG (0, number_names - 1);
      while (bot_names_used[index]);

      // copy it to c_name, and the according logo and skill to c_logo and i_skill
      strcpy (c_name, bot_names[index]);
      strcpy (c_logo, bot_logos[index]);
      i_nationality = bot_nationalities[index];
      i_skill = bot_skills[index];

      // use auto-assign for team and class
      i_team = -1;
      i_class = -1;
   }

   // else use defaults
   else
   {
      strcpy (c_name, "Bot");
      strcpy (c_logo, "lambda.bmp");
      i_nationality = CVAR_GET_FLOAT ("racc_defaultbotnationality");
      i_skill = CVAR_GET_FLOAT ("racc_defaultbotskill");
      i_team = -1;
      i_class = -1;
   }

   // okay, now we have a name, a skin and a skill for our new bot

   // if fake client creation is successful...
   if (!FNullEnt (pBotEdict = CREATE_FAKE_CLIENT (c_name)))
   {
      char ptr[128]; // allocate space for message from ClientConnect
      int index = 0;
      int clientIndex;
      char *infobuffer;

      if (pBotEdict->pvPrivateData != NULL)
         FREE_PRIVATE (pBotEdict); // free our predecessor's private data
      pBotEdict->pvPrivateData = NULL; // fools the private data pointer 
      pBotEdict->v.frags = 0; // reset his frag count 

      // create the player entity by calling MOD's player() function
      player (VARS (pBotEdict));

      clientIndex = ENTINDEX (pBotEdict); // get his client index
      infobuffer = GET_INFOBUFFER (pBotEdict); // get his info buffer

      // set him some parameters in the infobuffer
      SET_CLIENT_KEY_VALUE (clientIndex, infobuffer, "model", "helmet");
      SET_CLIENT_KEY_VALUE (clientIndex, infobuffer, "rate", "3500.000000");
      SET_CLIENT_KEY_VALUE (clientIndex, infobuffer, "cl_updaterate", "20");
      SET_CLIENT_KEY_VALUE (clientIndex, infobuffer, "cl_lw", "1");
      SET_CLIENT_KEY_VALUE (clientIndex, infobuffer, "cl_lc", "1");
      SET_CLIENT_KEY_VALUE (clientIndex, infobuffer, "tracker", "0");
      SET_CLIENT_KEY_VALUE (clientIndex, infobuffer, "cl_dlmax", "128");
      SET_CLIENT_KEY_VALUE (clientIndex, infobuffer, "lefthand", "1");
      SET_CLIENT_KEY_VALUE (clientIndex, infobuffer, "friends", "0");
      SET_CLIENT_KEY_VALUE (clientIndex, infobuffer, "dm", "0");
      SET_CLIENT_KEY_VALUE (clientIndex, infobuffer, "ah", "1");

      // let him connect to the server under the name c_name
      ClientConnect (pBotEdict, c_name, "127.0.0.1", ptr);

      // print a notification message on the dedicated server console if in developer mode
      if ((is_dedicated_server) && (CVAR_GET_FLOAT ("developer") > 0))
      {
         if (CVAR_GET_FLOAT ("developer") > 1)
         {
            UTIL_ServerConsole_printf ("Server requiring authentication\n");
            UTIL_ServerConsole_printf ("Client %s connected\n", STRING (pBotEdict->v.netname));
            UTIL_ServerConsole_printf ("Adr: 127.0.0.1:27005\n");
         }
         UTIL_ServerConsole_printf ("Verifying and uploading resources...\n");
         UTIL_ServerConsole_printf ("Custom resources total 0 bytes\n");
         UTIL_ServerConsole_printf ("  Decals:  0 bytes\n");
         UTIL_ServerConsole_printf ("----------------------\n");
         UTIL_ServerConsole_printf ("Resources to request: 0 bytes\n");
      }

      // let him actually join the game
      ClientPutInServer (pBotEdict);

      // find a free slot for this bot
      while ((bots[index].is_active) && (index < 32))
         index++;

      // link his entity to an useful pointer
      pBot = &bots[index];
      pBot->pEdict = pBotEdict;

      // initialize all the variables for this bot...

      BotReset (pBot); // reset our bot for new round

      pBot->pEdict->v.flags |= FL_FAKECLIENT; // set the fake client flag
      pBot->is_active = TRUE; // set his 'is active' flag
      pBot->bot_nationality = i_nationality; // save his nationality
      pBot->bot_skill = i_skill; // save his skill
      pBot->bot_team = i_team; // save his team
      pBot->bot_class = i_class; // save his class
      pBot->bot_money = 0; // reset his money amount
      pBot->menu_notify = MSG_CS_IDLE; // not selecting team yet
      pBot->pEdict->v.pitch_speed = BOT_PITCH_SPEED; // set the vertical speed at which he will turn
      pBot->pEdict->v.yaw_speed = BOT_YAW_SPEED; // set the horizontal speed at which he will turn

      // if internet mode is on...
      if (CVAR_GET_FLOAT ("racc_internetmode") > 0)
      {
         pBot->time_to_live = gpGlobals->time + RANDOM_LONG (300, 3600); // set him a TTL
         pBot->quit_game_time = pBot->time_to_live + RANDOM_FLOAT (3.0, 7.0); // disconnect time
      }
      else
      {
         pBot->time_to_live = -1; // don't set him a TTL (time to live)
         pBot->quit_game_time = -1; // so never quit
      }

      // say hello here
      if (RANDOM_LONG (1, 100) <= (86 - 2 * gpGlobals->maxClients))
      {
         pBot->BotChat.b_saytext_hello = TRUE;
         pBot->f_bot_saytext_time = gpGlobals->time + RANDOM_FLOAT (1.0, 2.0);
      }

      pBot->f_bot_alone_timer = gpGlobals->time + RANDOM_LONG (30, 120); // set an idle delay
      pBot->b_not_started = TRUE; // not started yet
   }
}


void BotBuyStuff (bot_t *pBot)
{
   // these weapon arrays MUST be sorted by INCREASING PRICE ORDER. Default pistols
   // (GLOCK18 and USP) don't appear because we don't want bots to buy them.
   int secondary_weapon_prices[4][2] =
   {
      {CS_WEAPON_P228,      600},
      {CS_WEAPON_DEAGLE,    650},
      {CS_WEAPON_FIVESEVEN, 750},
      {CS_WEAPON_ELITE,    1000},
   };

   int primary_weapon_prices[16][2] =
   {
      {CS_WEAPON_TMP,      1250},
      {CS_WEAPON_MAC10,    1400},
      {CS_WEAPON_MP5NAVY,  1500},
      {CS_WEAPON_M3,       1700},
      {CS_WEAPON_UMP45,    1700},
      {CS_WEAPON_P90,      2350},
      {CS_WEAPON_AK47,     2500},
      {CS_WEAPON_SCOUT,    2750},
      {CS_WEAPON_XM1014,   3000},
      {CS_WEAPON_M4A1,     3100},
      {CS_WEAPON_AUG,      3500},
      {CS_WEAPON_SG552,    3500},
      {CS_WEAPON_SG550,    4200},
      {CS_WEAPON_AWP,      4750},
      {CS_WEAPON_G3SG1,    5000},
      {CS_WEAPON_M249,     5750}
   };

   if (pBot->pEdict == NULL)
      return; // reliability check

   // check if bot is VIP
   if (strcmp (g_engfuncs.pfnInfoKeyValue ((*g_engfuncs.pfnGetInfoKeyBuffer) (pBot->pEdict), "model"), "vip") == 0)
   {
      pBot->f_buy_time = 0; // don't buy anything (VIPs don't buy stuff)
      return;
   }

   // pause the bot for a while
   pBot->f_pause_time = gpGlobals->time + 1.2;

   // state 0: if armor is damaged and bot has some money, buy some armor
   if (pBot->buy_state == 0)
   {
      if ((pBot->bot_armor < 80) && (pBot->bot_money > 1000))
      {
         FakeClientCommand (pBot->pEdict, "buyequip"); // enter the "buy equipment" menu

         // if bot is rich, buy kevlar + helmet, else buy a single kevlar
         if (pBot->bot_money > 3000)
            FakeClientCommand (pBot->pEdict, BUY_KEVLARHELMET);
         else
            FakeClientCommand (pBot->pEdict, BUY_KEVLAR);

         pBot->f_buy_time = gpGlobals->time + RANDOM_FLOAT (0.3, 0.5); // delay next buy
      }
      pBot->buy_state++;
      return;
   }

   // state 1: if no primary weapon and bot has some money, buy a better primary weapon
   else if (pBot->buy_state == 1)
   {
      if (!BotHasPrimary (pBot) && (pBot->bot_money > 1250))
      {
         char index = 0, weapon_index = 0;

         // set the index at the position of the first gun bot can't buy
         while (primary_weapon_prices[index][1] < pBot->bot_money)
            index++;

         // now pick up one randomly in the affordable zone of the array
         weapon_index = RANDOM_LONG (0, index - 1);

         FakeClientCommand (pBot->pEdict, "buy"); // enter the main buy menu

         // handle each case separately
         switch (primary_weapon_prices[weapon_index][0])
         {
            case CS_WEAPON_M3:
               FakeClientCommand (pBot->pEdict, BUY_SHOTGUNMENU); // buy a SHOTGUN
               FakeClientCommand (pBot->pEdict, BUY_M3); // buy the M3
               break;
            case CS_WEAPON_XM1014:
               FakeClientCommand (pBot->pEdict, BUY_SHOTGUNMENU); // buy a SHOTGUN
               FakeClientCommand (pBot->pEdict, BUY_XM1014); // buy the XM1014
               break;
            case CS_WEAPON_MP5NAVY:
               FakeClientCommand (pBot->pEdict, BUY_SUBMACHINEGUNMENU); // buy a SUB-MACHINE GUN
               FakeClientCommand (pBot->pEdict, BUY_MP5NAVY); // buy the MP5NAVY
               break;
            case CS_WEAPON_TMP:
               FakeClientCommand (pBot->pEdict, BUY_SUBMACHINEGUNMENU); // buy a SUB-MACHINE GUN
               FakeClientCommand (pBot->pEdict, BUY_TMP); // buy the TMP
               break;
            case CS_WEAPON_P90:
               FakeClientCommand (pBot->pEdict, BUY_SUBMACHINEGUNMENU); // buy a SUB-MACHINE GUN
               FakeClientCommand (pBot->pEdict, BUY_P90); // buy the P90
               break;
            case CS_WEAPON_MAC10:
               FakeClientCommand (pBot->pEdict, BUY_SUBMACHINEGUNMENU); // buy a SUB-MACHINE GUN
               FakeClientCommand (pBot->pEdict, BUY_MAC10); // buy the MAC10
               break;
            case CS_WEAPON_UMP45:
               FakeClientCommand (pBot->pEdict, BUY_SUBMACHINEGUNMENU); // buy a SUB-MACHINE GUN
               FakeClientCommand (pBot->pEdict, BUY_UMP45); // buy the UMP45
               break;
            case CS_WEAPON_AK47:
               FakeClientCommand (pBot->pEdict, BUY_RIFLEMENU); // buy a RIFLE
               FakeClientCommand (pBot->pEdict, BUY_AK47); // buy the AK47
               break;
            case CS_WEAPON_SG552:
               FakeClientCommand (pBot->pEdict, BUY_RIFLEMENU); // buy a RIFLE
               FakeClientCommand (pBot->pEdict, BUY_SG552); // buy the SG552
               break;
            case CS_WEAPON_M4A1:
               FakeClientCommand (pBot->pEdict, BUY_RIFLEMENU); // buy a RIFLE
               FakeClientCommand (pBot->pEdict, BUY_M4A1); // buy the M4A1
               break;
            case CS_WEAPON_AUG:
               FakeClientCommand (pBot->pEdict, BUY_RIFLEMENU); // buy a RIFLE
               FakeClientCommand (pBot->pEdict, BUY_AUG); // buy the AUG
               break;
            case CS_WEAPON_SCOUT:
               FakeClientCommand (pBot->pEdict, BUY_RIFLEMENU); // buy a RIFLE
               FakeClientCommand (pBot->pEdict, BUY_SCOUT); // buy the SCOUT
               break;
            case CS_WEAPON_AWP:
               FakeClientCommand (pBot->pEdict, BUY_RIFLEMENU); // buy a RIFLE
               FakeClientCommand (pBot->pEdict, BUY_AWP); // buy the AWP
               break;
            case CS_WEAPON_G3SG1:
               FakeClientCommand (pBot->pEdict, BUY_RIFLEMENU); // buy a RIFLE
               FakeClientCommand (pBot->pEdict, BUY_G3SG1); // buy the G3SG1
               break;
            case CS_WEAPON_SG550:
               FakeClientCommand (pBot->pEdict, BUY_RIFLEMENU); // buy a RIFLE
               FakeClientCommand (pBot->pEdict, BUY_SG550); // buy the AK47
               break;
            case CS_WEAPON_M249:
               FakeClientCommand (pBot->pEdict, BUY_MACHINEGUNMENU); // buy a MACHINEGUN
               FakeClientCommand (pBot->pEdict, BUY_M249); // buy the M249
               break;
            default:
               FakeClientCommand (pBot->pEdict, "close");
               break;
         }

         pBot->f_buy_time = gpGlobals->time + RANDOM_FLOAT (0.3, 0.5); // delay next buy
      }
      pBot->buy_state++;
      return;
   }

   // states 2, 3, 4: if bot has still some money, buy more primary ammo
   else if ((pBot->buy_state == 2) || (pBot->buy_state == 3) || (pBot->buy_state == 4))
   {
      if (pBot->bot_money > 300)
      {
         FakeClientCommand (pBot->pEdict, "buyammo1"); // buy some primary ammo
         pBot->f_buy_time = gpGlobals->time + 0.3; // delay next buy
      }
      pBot->buy_state++;
      return;
   }

   // state 5: if bot has still some money, buy a better secondary weapon
   else if (pBot->buy_state == 5)
   {
      if (pBot->bot_money > 1200)
      {
         char index = 0, weapon_index = 0;

         // set the index at the position of the first gun bot can't buy
         while (secondary_weapon_prices[index][1] < pBot->bot_money)
            index++;

         // now pick up one randomly in the array "buyable" zone
         weapon_index = RANDOM_LONG (0, index - 1);

         FakeClientCommand (pBot->pEdict, "buy"); // enter the main buy menu
         FakeClientCommand (pBot->pEdict, BUY_PISTOLMENU); // enter the "buy pistol" menu

         // handle each case separately
         if (secondary_weapon_prices[weapon_index][0] == CS_WEAPON_DEAGLE)
            FakeClientCommand (pBot->pEdict, BUY_DEAGLE); // buy the DEAGLE
         else if (secondary_weapon_prices[weapon_index][0] == CS_WEAPON_P228)
            FakeClientCommand (pBot->pEdict, BUY_P228); // buy the P228
         else if (secondary_weapon_prices[weapon_index][0] == CS_WEAPON_ELITE)
            FakeClientCommand (pBot->pEdict, BUY_ELITE); // buy the ELITE
         else if (secondary_weapon_prices[weapon_index][0] == CS_WEAPON_FIVESEVEN)
            FakeClientCommand (pBot->pEdict, BUY_FIVESEVEN); // buy the FIVESEVEN
         else
            FakeClientCommand (pBot->pEdict, "close");

         pBot->f_buy_time = gpGlobals->time + RANDOM_FLOAT (0.3, 0.5); // delay next buy
      }
      pBot->buy_state++;
      return;
   }

   // states 6 and 7: if bot has still some money, buy more secondary ammo
   else if ((pBot->buy_state == 6) || (pBot->buy_state == 7))
   {
      if (pBot->bot_money > 300)
      {
         FakeClientCommand (pBot->pEdict, "buyammo2"); // buy some secondary ammo
         pBot->f_buy_time = gpGlobals->time + 0.3; // delay next buy
      }
      pBot->buy_state++;
      return;
   }

   // state 8: if bot has still some money, choose if bot should buy a grenade or not
   else if (pBot->buy_state == 8)
   {
      if ((pBot->bot_money > 600) && (RANDOM_LONG (1, 100) < 66))
      {
         FakeClientCommand (pBot->pEdict, "buyequip"); // enter the "buy equipment" menu

         if (RANDOM_LONG (0, 100) < 66)
            FakeClientCommand (pBot->pEdict, BUY_HEGRENADE); // buy HE grenade
         else if (RANDOM_LONG (0, 100) < 66)
            FakeClientCommand (pBot->pEdict, BUY_FLASHBANG); // buy flashbang
         else
            FakeClientCommand (pBot->pEdict, BUY_SMOKEGRENADE); // buy smokegrenade

         pBot->f_buy_time = gpGlobals->time + RANDOM_FLOAT (0.3, 0.5); // delay next buy
      }
      pBot->buy_state++;
      return;
   }

   // state 9: if bot is counter-terrorist and we're on a bomb map, randomly buy the defuse kit
   else if (pBot->buy_state == 9)
   {
      if ((GetTeam (pBot->pEdict) == CS_COUNTER_TERRORIST) && b_bomb_map && !pBot->b_has_defuse_kit
          && (pBot->bot_money > 200) && RANDOM_LONG (1, 100) < 33)
      {
         FakeClientCommand (pBot->pEdict, "buyequip"); // enter the "buy equipment" menu
         FakeClientCommand (pBot->pEdict, BUY_DEFUSEKIT); // to buy the defuse kit

         pBot->f_buy_time = gpGlobals->time + RANDOM_FLOAT (0.3, 0.5); // delay next buy
      }
      pBot->buy_state++;
      return;
   }

   // state 10: switch to best weapon
   else if (pBot->buy_state == 10)
   {
      BotSwitchToBestWeapon (pBot);
      pBot->f_buy_time = 0; // don't buy anything more after that
      pBot->buy_state = 0; // reset buy state

      if (RANDOM_LONG (1, 100) <= (56 - 2 * gpGlobals->maxClients))
      {
         pBot->BotChat.b_sayaudio_firstspawn = TRUE; // bot says 'go go go' or something like that
         pBot->f_bot_sayaudio_time = gpGlobals->time + RANDOM_FLOAT (0.5, 10.0);
      }
      else if (RANDOM_LONG (1, 100) < 50)
      {
         if (RANDOM_LONG (1, 100) < 34)
            BotTalkOnTheRadio (pBot, RADIOMSG_STICKTOGETHER);
         else if (RANDOM_LONG (1, 100) < 50)
            BotTalkOnTheRadio (pBot, RADIOMSG_GOGOGO);
         else
            BotTalkOnTheRadio (pBot, RADIOMSG_STORMTHEFRONT);
      }

      return;
   }
}


bool BotCheckForSpecialZones (bot_t *pBot)
{
   edict_t *pSpecialZone = NULL;

   if (pBot->pEdict == NULL)
      return FALSE; // reliability check

   if (b_botdontfind)
      return FALSE; // don't process if botdontfind is set

   // is there a special zone near here?
   while ((pSpecialZone = UTIL_FindEntityInSphere (pSpecialZone, pBot->pEdict->v.origin, 1000)) != NULL)
   {
      // check for a visible safety zone
      if ((strcmp ("func_vip_safetyzone", STRING (pSpecialZone->v.classname)) == 0)
          && (BotCanSeeThisBModel (pBot, pSpecialZone))
          && (FInViewCone (VecBModelOrigin (pSpecialZone), pBot->pEdict)))
      {
         // is bot a VIP ?
         if (strcmp (g_engfuncs.pfnInfoKeyValue ((*g_engfuncs.pfnGetInfoKeyBuffer) (pBot->pEdict), "model"), "vip") == 0)
         {
            pBot->BotMove.f_forward_time = gpGlobals->time + 60.0; // let our bot go...
            pBot->BotMove.b_is_walking = FALSE; // ...and the sooner he gets there, the better it is !

            // is bot NOT already reaching this safety zone ?
            if (pBot->v_place_to_keep != VecBModelOrigin (pSpecialZone))
            {
               pBot->v_place_to_keep = VecBModelOrigin (pSpecialZone); // reach the safety zone
               pBot->f_place_time = gpGlobals->time; // remember when we last saw the place to keep
               BotTalkOnTheRadio (pBot, RADIOMSG_COVERME); // bot speaks, "cover me!"
            }

            return TRUE; // bot is concerned by this special zone
         }

         // else check if bot is a 'normal' counter-terrorist
         else if (GetTeam (pBot->pEdict) == CS_COUNTER_TERRORIST)
         {
            for (char i = 0; i < 32; i++) // cycle through all bot slots
            {
               // reliability check: is this slot unregistered
               if (bots[i].pEdict == NULL)
                  continue;

               // is this bot dead OR inactive OR self ?
               if (!IsAlive (bots[i].pEdict) || !bots[i].is_active || (bots[i].pEdict == pBot->pEdict))
                  continue;

               // is this one VIP AND visible AND not been used for at least 20 seconds ?
               if ((strcmp (g_engfuncs.pfnInfoKeyValue ((*g_engfuncs.pfnGetInfoKeyBuffer) (bots[i].pEdict), "model"), "vip") == 0)
                   && (bots[i].v_lastseenuser_position == Vector (0, 0, 0))
                   && (bots[i].f_bot_use_time + 20.0 < gpGlobals->time)
                   && (BotGetIdealAimVector (pBot, bots[i].pEdict) != Vector (0, 0, 0)))
               {
                  bots[i].v_place_to_keep = Vector (0, 0, 0); // reset any v_place_to_keep

                  // let's make him head off toward us...
                  BotSetIdealYaw (&bots[i], UTIL_VecToAngles (pBot->pEdict->v.origin - bots[i].pEdict->v.origin).y);
                  bots[i].f_reach_time = gpGlobals->time + 0.5; // make him ignore his reach points for a while

                  // set this bot's use time to now to avoid calling him twice
                  bots[i].f_bot_use_time = gpGlobals->time;

                  BotTalkOnTheRadio (&bots[i], RADIOMSG_AFFIRMATIVE); // make bot agree
                  bots[i].BotChat.b_sayaudio_affirmative = TRUE;
                  bots[i].f_bot_sayaudio_time = gpGlobals->time + RANDOM_FLOAT (0.5, 3.0);
                  return FALSE; // normal CT is NOT directly concerned by this special zone
               }
            }

            return FALSE; // normal CT can't see the VIP and is NOT concerned by this zone
         }
      }

      // check for a visible escape zone
      else if ((strcmp ("func_escapezone", STRING (pSpecialZone->v.classname)) == 0)
               && (BotCanSeeThisBModel (pBot, pSpecialZone))
               && (FInViewCone (VecBModelOrigin (pSpecialZone), pBot->pEdict)))
      {
         // is bot a terrorist?
         if (GetTeam (pBot->pEdict) == CS_TERRORIST)
         {
            pBot->BotMove.f_forward_time = gpGlobals->time + 60.0; // let our bot go...
            pBot->BotMove.b_is_walking = FALSE; // ...and the sooner he gets there, the better it is !

            // is bot NOT already reaching the escape zone ?
            if (pBot->v_place_to_keep != VecBModelOrigin (pSpecialZone))
            {
               pBot->v_place_to_keep = VecBModelOrigin (pSpecialZone); // reach the escape zone
               pBot->f_place_time = gpGlobals->time; // remember when we last saw the place to keep
               BotTalkOnTheRadio (pBot, RADIOMSG_FALLBACK); // bot speaks, "fallback team!"
            }

            // cycle through all bot slots to find teammates
            for (char i = 0; i < 32; i++)
            {
               // reliability check: is this slot unregistered
               if (bots[i].pEdict == NULL)
                  continue;

               // is this bot dead OR inactive OR self ?
               if (!IsAlive (bots[i].pEdict) || !bots[i].is_active || (bots[i].pEdict == pBot->pEdict))
                  continue;

               // is this one terrorist AND visible AND not been used for at least 20s?
               if ((GetTeam (bots[i].pEdict) == CS_TERRORIST)
                   && (bots[i].v_lastseenuser_position == Vector (0, 0, 0))
                   && (bots[i].f_bot_use_time + 20.0 < gpGlobals->time)
                   && (BotGetIdealAimVector (pBot, bots[i].pEdict) != Vector (0, 0, 0)))
               {
                  bots[i].v_place_to_keep = Vector (0, 0, 0); // reset any v_place_to_keep

                  // let's make him head off toward us...
                  BotSetIdealYaw (&bots[i], UTIL_VecToAngles (pBot->pEdict->v.origin - bots[i].pEdict->v.origin).y);
                  bots[i].f_reach_time = gpGlobals->time + 0.5; // make him ignore his reach points for a while

                  // set this bot's use time to now to avoid calling him twice
                  bots[i].f_bot_use_time = gpGlobals->time;

                  BotTalkOnTheRadio (&bots[i], RADIOMSG_AFFIRMATIVE); // bot agrees

                  bots[i].BotChat.b_sayaudio_affirmative = TRUE;
                  bots[i].f_bot_sayaudio_time = gpGlobals->time + RANDOM_FLOAT (0.5, 3.0);
               }
            }

            return TRUE; // bot is concerned by this special zone
         }
      }

      // check for a visible dropped bomb
      else if ((strcmp ("weaponbox", STRING (pSpecialZone->v.classname)) == 0)
               && (BotCanSeeThis (pBot, pSpecialZone->v.origin))
               && (FInViewCone (pSpecialZone->v.origin, pBot->pEdict))
               && (strcmp (STRING (pSpecialZone->v.model), "models/w_backpack.mdl") == 0)
               && ENT_IS_ON_FLOOR (pSpecialZone))
      {
         // both terrorists and counter-terrorists will head to the bomb. Because
         // counter-terrorists won't be able to pick it up, they will so
         // "cruise" around the bomb spot, permanently looking for enemies.

         // is bot a terrorist ?
         if (GetTeam (pBot->pEdict) == CS_TERRORIST)
         {
            BotReachPosition (pBot, pSpecialZone->v.origin); // if bot is a T, go pick it up
            return TRUE; // bot is concerned by this special zone
         }

         // else bot must be a counter-terrorist, can he camp near here ?
         else if ((pBot->f_camp_time + 3.0 < gpGlobals->time) && BotHasPrimary (pBot)
                  && !((pBot->bot_weapons & (1 << CS_WEAPON_M3))
                       || (pBot->bot_weapons & (1 << CS_WEAPON_XM1014))))
         {
            if (BotCanCampNearHere (pBot, pSpecialZone->v.origin)
                && (RANDOM_LONG (1, 100) <= (91 - 2 * gpGlobals->maxClients)))
               BotTalkOnTheRadio (pBot, RADIOMSG_INPOSITION); // let the bot camp here
            return FALSE; // normal CTs are NOT directly concerned by this special zone
         }
      }

      // check for a visible planted bomb
      else if ((strcmp ("grenade", STRING (pSpecialZone->v.classname)) == 0)
               && (BotCanSeeThis (pBot, pSpecialZone->v.origin))
               && (FInViewCone (pSpecialZone->v.origin, pBot->pEdict))
               && (strcmp (STRING (pSpecialZone->v.model), "models/w_c4.mdl") == 0)
               && ENT_IS_ON_FLOOR (pSpecialZone))
      {
         // is bot a counter-terrorist ?
         if (GetTeam (pBot->pEdict) == CS_COUNTER_TERRORIST)
         {
            BotDefuseBomb (pBot, pSpecialZone); // defuse bomb
            return TRUE; // bot is concerned by this special zone
         }

         // else bot must be a terrorist, can he camp near here ?
         else if (pBot->f_camp_time + 10.0 < gpGlobals->time)
         {
            if (BotCanCampNearHere (pBot, pSpecialZone->v.origin)
                && (RANDOM_LONG (1, 100) <= (91 - 2 * gpGlobals->maxClients)))
               BotTalkOnTheRadio (pBot, RADIOMSG_INPOSITION); // let the bot camp here
            return FALSE; // campers are NOT directly concerned by this special zone
         }
      }

      // check for a visible bomb target
      else if ((strcmp ("func_bomb_target", STRING (pSpecialZone->v.classname)) == 0)
               && (BotCanSeeThisBModel (pBot, pSpecialZone))
               && (FInViewCone (VecBModelOrigin (pSpecialZone), pBot->pEdict)))
      {
         // is bot a terrorist ?
         if (GetTeam (pBot->pEdict) == CS_TERRORIST)
         {
            // does the bot have the C4 ?
            if (pBot->bot_weapons & (1 << CS_WEAPON_C4))
            {
               BotPlantBomb (pBot, VecBModelOrigin (pSpecialZone)); // plant bomb
               return TRUE; // bot is concerned by this special zone
            }

            // if not, bot must be a 'normal' terrorist
            else
            {
               for (char i = 0; i < 32; i++) // cycle through all bot slots
               {
                  // reliability check: is this slot unregistered
                  if (bots[i].pEdict == NULL)
                     continue;

                  // is this bot dead OR inactive OR self ?
                  if (!IsAlive (bots[i].pEdict) || !bots[i].is_active || (bots[i].pEdict == pBot->pEdict))
                     continue;

                  // does this one have C4 AND visible AND not been used for at least 20s?
                  if ((bots[i].bot_weapons & (1 << CS_WEAPON_C4))
                      && (bots[i].v_lastseenuser_position == Vector (0, 0, 0))
                      && (bots[i].f_bot_use_time + 20.0 < gpGlobals->time)
                      && (BotGetIdealAimVector (pBot, bots[i].pEdict) != Vector (0, 0, 0)))
                  {
                     bots[i].v_place_to_keep = Vector (0, 0, 0); // reset any v_place_to_keep

                     // let's make him head off toward us...
                     BotSetIdealYaw (&bots[i], UTIL_VecToAngles (pBot->pEdict->v.origin - bots[i].pEdict->v.origin).y);
                     bots[i].f_reach_time = gpGlobals->time + 0.5; // make him ignore his reach points for a while

                     // set this bot's use time to now to avoid calling him twice
                     bots[i].f_bot_use_time = gpGlobals->time;

                     BotTalkOnTheRadio (&bots[i], RADIOMSG_YOUTAKETHEPOINT); // bot speaks, "you take the point!"
                     bots[i].BotChat.b_sayaudio_affirmative = TRUE;
                     bots[i].f_bot_sayaudio_time = gpGlobals->time + RANDOM_FLOAT (0.5, 3.0);
                  }
               }

               return FALSE; // bot is NOT concerned by this special zone
            }
         }

         // else bot must be a counter-terrorist, if bomb not planted yet can the bot camp near here ?
         else if (!b_bomb_planted && (pBot->f_camp_time + 10.0 < gpGlobals->time)
                  && BotHasPrimary (pBot)
                  && !((pBot->bot_weapons & (1 << CS_WEAPON_M3))
                       || (pBot->bot_weapons & (1 << CS_WEAPON_XM1014))))
         {
            if (BotCanCampNearHere (pBot, VecBModelOrigin (pSpecialZone))
                && (RANDOM_LONG (1, 100) <= (91 - 2 * gpGlobals->maxClients)))
               BotTalkOnTheRadio (pBot, RADIOMSG_INPOSITION); // let the bot camp here
            return FALSE; // campers are NOT directly concerned by this special zone
         }
      }

      // check for a visible hostage rescue zone
      else if ((strcmp ("func_hostage_rescue", STRING (pSpecialZone->v.classname)) == 0)
               && (BotCanSeeThisBModel (pBot, pSpecialZone))
               && (FInViewCone (VecBModelOrigin (pSpecialZone), pBot->pEdict)))
      {
         // if bot is a terrorist, has he not camped for at lease 10 seconds ?
         if ((GetTeam (pBot->pEdict) == CS_TERRORIST) && (pBot->f_camp_time + 10.0 < gpGlobals->time))
         {
            if (BotCanCampNearHere (pBot, VecBModelOrigin(pSpecialZone))
                && (RANDOM_LONG (1, 100) <= (91 - 2 * gpGlobals->maxClients)))
               BotTalkOnTheRadio (pBot, RADIOMSG_INPOSITION); // let the bot camp here
            return FALSE; // campers are NOT directly concerned by this special zone
         }
      }

      // check for a visible hostage
      else if ((strcmp ("hostage_entity", STRING (pSpecialZone->v.classname)) == 0)
               && (BotCanSeeThis (pBot, pSpecialZone->v.origin))
               && (FInViewCone (pSpecialZone->v.origin, pBot->pEdict)))
      {
         // is bot a terrorist ?
         if (GetTeam (pBot->pEdict) == CS_TERRORIST)
         {
            // check if the hostage is moving AND bot has no enemy yet
            if ((pSpecialZone->v.velocity.Length2D () > 0) && (pBot->pBotEnemy == NULL))
               pBot->pBotEnemy = pSpecialZone; // alert, this hostage flees away, shoot him down !

            // else has the bot not camped for at least 10 seconds ?
            if (pBot->f_camp_time + 10.0 < gpGlobals->time)
               if (BotCanCampNearHere (pBot, pSpecialZone->v.origin)
                   && (RANDOM_LONG (1, 100) <= (91 - 2 * gpGlobals->maxClients)))
                  BotTalkOnTheRadio (pBot, RADIOMSG_INPOSITION); // let the bot camp here

            return FALSE; // terrorists are NOT directly concerned by this special zone
         }

         // else bot must be a counter-terrorist
         else
         {
            // TODO: implement hostage usage
            return FALSE;
         }
      }
   }

   return FALSE; // bot found nothing interesting
}


bool BotCheckForGrenades (bot_t *pBot)
{
   edict_t *pGrenade = NULL;

   if (pBot->pEdict == NULL)
      return FALSE; // reliability check

   if (b_botdontfind)
      return FALSE; // don't process if botdontfind is set

   // is there an armed grenade near here?
   while ((pGrenade = UTIL_FindEntityInSphere (pGrenade, pBot->pEdict->v.origin, 300)) != NULL)
   {
      // check if entity is an armed grenade
      if ((strcmp ("grenade", STRING (pGrenade->v.classname)) == 0)
          && (BotCanSeeThis (pBot, pGrenade->v.origin))
          && (FInViewCone (pGrenade->v.origin, pBot->pEdict)))
      {
         // check if this grenade is NOT a smoke grenade neither the C4 (not to confuse w/ bomb)
         if ((strcmp (STRING (pGrenade->v.model), "models/w_smokegrenade.mdl") != 0)
             && (strcmp (STRING (pGrenade->v.model), "models/w_c4.mdl") != 0))
         {
            Vector bot_angles = UTIL_VecToAngles (pGrenade->v.origin - pBot->pEdict->v.origin);
            BotSetIdealYaw (pBot, bot_angles.y); // face the grenade...

            // ... and run away!!
            pBot->BotMove.f_backwards_time = gpGlobals->time + 0.5; // until the grenade explodes

            // is it a flashbang ?
            if (strcmp (STRING (pGrenade->v.model), "models/w_flashbang.mdl") == 0)
            {
               // strafe to (hopefully) take cover
               if (RANDOM_LONG (1, 100) < 50)
                  pBot->BotMove.f_strafeleft_time = gpGlobals->time + RANDOM_FLOAT (0.5, 2.0);
               else
                  pBot->BotMove.f_straferight_time = gpGlobals->time + RANDOM_FLOAT (0.5, 2.0);
            }

            // if this grenade is our enemies'
            if (GetTeam (pBot->pEdict) != GetTeam (pGrenade->v.owner))
            {
               pBot->BotChat.b_sayaudio_seegrenade = TRUE; // bot says 'danger'
               pBot->f_bot_sayaudio_time = gpGlobals->time + RANDOM_FLOAT (0.7, 1.5);
            }

            return TRUE; // bot is concerned by this grenade
         }
      }
   }

   return FALSE; // bot found nothing interesting
}


void BotCheckForItems (bot_t *pBot)
{
   edict_t *pent = NULL;
   edict_t *pPickupEntity = NULL;
   Vector pickup_origin;
   Vector entity_origin;
   float min_distance = 501;
   bool can_pickup;
   TraceResult tr;
   Vector vecStart;
   Vector vecEnd;
   int angle_to_entity;

   if (pBot->pEdict == NULL)
      return; // reliability check

   if (b_botdontfind)
      return; // don't process if botdontfind is set

   pBot->b_is_picking_item = FALSE;

   while ((pent = UTIL_FindEntityInSphere (pent, pBot->pEdict->v.origin, 500)) != NULL)
   {
      can_pickup = FALSE; // assume can't use it until known otherwise

      // see if this is a "func_" type of entity (func_button, etc.)...
      if (strncmp ("func_", STRING (pent->v.classname), 5) == 0)
      {
         // BModels have 0,0,0 for origin so must use VecBModelOrigin...
         entity_origin = VecBModelOrigin (pent);

         vecStart = pBot->pEdict->v.origin + pBot->pEdict->v.view_ofs;
         vecEnd = entity_origin;

         angle_to_entity = BotAngleToLocation (pBot, vecEnd - vecStart);

         // check if entity is outside field of view (+/- 60 degrees)
         if (angle_to_entity > 60)
            continue;  // skip this item if bot can't "see" it

         // check if entity is a ladder (ladders are a special case)
         if (strcmp ("func_ladder", STRING (pent->v.classname)) == 0)
         {
            // force ladder origin to same z coordinate as bot since
            // the VecBModelOrigin is the center of the ladder.  For
            // LONG ladders, the center MAY be hundreds of units above
            // the bot.  Fake an origin at the same level as the bot...

            entity_origin.z = pBot->pEdict->v.origin.z;
            vecEnd = entity_origin;

            // trace a line from bot's eyes to func_ladder entity...
            UTIL_TraceLine (vecStart, vecEnd, ignore_monsters, pBot->pEdict->v.pContainingEntity, &tr);

            // check if traced all the way up to the entity (didn't hit wall)
            if (tr.flFraction >= 1.0)
            {
               // always use the ladder if haven't used a ladder in at least 5 seconds...
               if (pBot->f_end_use_ladder_time + 5.0 < gpGlobals->time)
                  can_pickup = TRUE;
            }
         }
         else
         {
            // trace a line from bot's eyes to entity
            UTIL_TraceLine (vecStart, vecEnd, ignore_monsters, pBot->pEdict->v.pContainingEntity, &tr);
            
            // if traced all the way up to the entity (didn't hit wall)
            if (strcmp (STRING (pent->v.classname), STRING (tr.pHit->v.classname)) == 0)
            {
               // find distance to item for later use...
               float distance = (vecEnd - vecStart).Length ();

               // check if entity is a breakable...
               if ((strcmp ("func_breakable", STRING (pent->v.classname)) == 0)
                        && (pent->v.takedamage != DAMAGE_NO) && (pent->v.health > 0)
                        && !(pent->v.flags & FL_WORLDBRUSH) && (abs (vecEnd.z - vecStart.z) < 60))
               {
                  // check if close enough...
                  if (distance < 50)
                  {
                     if (pBot->current_weapon.iId != CS_WEAPON_KNIFE)
                        BotSelectItem (pBot, "weapon_knife"); // select a proximity weapon
                     else
                     {
                        // point the weapon at the breakable and strike it
                        BotPointGun (pBot, UTIL_VecToAngles (vecEnd - GetGunPosition (pBot->pEdict)));
                        pBot->pEdict->v.button |= IN_ATTACK; // strike the breakable
                        pBot->f_reload_time = gpGlobals->time + RANDOM_LONG (1.5, 3.0); // set next time to reload
                     }
                  }

                  can_pickup = TRUE;
               }
            }
         }
      }
      else // everything else...
      {
         entity_origin = pent->v.origin;

         vecStart = pBot->pEdict->v.origin + pBot->pEdict->v.view_ofs;
         vecEnd = pent->v.origin;

         // find angles from bot origin to entity...
         angle_to_entity = BotAngleToLocation (pBot, vecEnd - vecStart);

         // check if entity is outside field of view (+/- 60 degrees)
         if (angle_to_entity > 60)
            continue;  // skip this item if bot can't "see" it

         // check if line of sight to object is not blocked (i.e. visible)
         if (BotCanSeeThis (pBot, vecEnd))
         {
            // check if entity is some interesting weapon...
            if (strcmp ("weaponbox", STRING (pent->v.classname)) == 0)
            {
               if (pent->v.effects & EF_NODRAW)
                  continue; // the item is not spawned (not visible)

               // check if the item is really interesting for our bot
               if (BotItemIsInteresting (pBot, pent))
               {
                  if ((entity_origin - pBot->pEdict->v.origin).Length () < 60)
                     BotDiscardItem (pBot, pent); // discard our current stuff

                  can_pickup = TRUE;
               }
               else
                  can_pickup = FALSE;
            }

            // else check if entity is an abandoned defuse kit...
            if (strcmp ("item_thighpack", STRING (pent->v.classname)) == 0)
            {
               if (pent->v.effects & EF_NODRAW)
                  continue; // the item is not spawned (not visible)

               if (!pBot->b_has_defuse_kit && (GetTeam (pBot->pEdict) == CS_COUNTER_TERRORIST))
                  can_pickup = TRUE; // if bot has no defuse kit AND is a CT, go pick that one up
               else
                  can_pickup = FALSE; // this item is not interesting
            }
         } // end if object is visible
      } // end else not "func_" entity

      if (can_pickup) // if the bot found something it can pickup...
      {
         float distance = (entity_origin - pBot->pEdict->v.origin).Length ();

         // see if it's the closest item so far...
         if (distance < min_distance)
         {
            min_distance = distance; // update the minimum distance
            pPickupEntity = pent; // remember this entity
            pickup_origin = entity_origin; // remember location of entity
         }
      }
   } // end while loop

   if (pPickupEntity != NULL)
   {
      pBot->b_is_picking_item = TRUE; // set bot picking item flag
      pBot->v_reach_point = pickup_origin; // save the location of item bot is trying to get
   }
   else
      pBot->b_is_picking_item = FALSE; // reset picking item flag
}


void BotThink (bot_t *pBot)
{
   float pitch_degrees;
   float yaw_degrees;

   if (pBot->pEdict == NULL)
      return; // reliability check

   pBot->pEdict->v.flags |= FL_FAKECLIENT;

   if (pBot->msecdel <= gpGlobals->time)
   {
      pBot->msecdel = gpGlobals->time + 0.5;
      if (pBot->msecnum > 0)
         pBot->msecval = 450.0 / pBot->msecnum;
      pBot->msecnum = 0;
   }
   else
      pBot->msecnum++;

   if (pBot->msecval < 1) // don't allow msec to be less than 1...
      pBot->msecval = 1;

   if (pBot->msecval > 100) // ...or greater than 100
      pBot->msecval = 100;

   pBot->pEdict->v.button = 0; // reset buttons pressed
   pBot->BotMove.f_move_speed = 0; // reset move_speed
   pBot->BotMove.f_strafe_speed = 0; // reset strafe_speed

   // handle bot speaking stuff
   BotSayText (pBot);
   BotSayAudio (pBot);

   // handle bot moving stuff
   BotMove (pBot);

   // if the bot hasn't selected stuff to start the game yet, go do that...
   if ((pBot->b_not_started) || (pBot->menu_notify != MSG_CS_IDLE))
   {
      BotStartGame (pBot);
      g_engfuncs.pfnRunPlayerMove (pBot->pEdict, UTIL_WrapAngles (pBot->pEdict->v.v_angle), 0, 0, 0, 0, 0, pBot->msecval);
      return;
   }

   // is it time for the bot to leave the game? (depending on his time to live)
   if ((pBot->time_to_live > 0) && (pBot->time_to_live <= gpGlobals->time))
   {
      pBot->time_to_live = gpGlobals->time + 6.0; // don't say it twice (bad hack)
      pBot->f_pause_time = gpGlobals->time + 10.0; // stop the bot while he is leaving
      if (RANDOM_LONG (1, 100) <= (66 - 2 * gpGlobals->maxClients))
         pBot->BotChat.b_saytext_bye = TRUE; // say goodbye
   }

   // if the bot is dead, wait for respawn...
   if (!IsAlive (pBot->pEdict))
   {
      BotReset (pBot); // reset our bot for next round

      // was the bot killed by another player ?
      if (pBot->pKillerEntity != NULL)
      {
         if (RANDOM_LONG (1, 100) <= (56 - 2 * gpGlobals->maxClients))
         {
            pBot->BotChat.b_saytext_killed = TRUE;
            pBot->f_bot_saytext_time = gpGlobals->time + RANDOM_FLOAT (5.0, 10.0);
         }
      }

      g_engfuncs.pfnRunPlayerMove (pBot->pEdict, Vector (0, 0, 0), 0, 0, 0, 0, 0, pBot->msecval);
      return;
   }

   // should the bot complain of being alone for a long time ?
   if ((pBot->f_bot_alone_timer > 0) && (pBot->f_bot_alone_timer <= gpGlobals->time))
   {
      pBot->f_bot_alone_timer = gpGlobals->time + RANDOM_LONG (30, 120); // sets new delay

      if (RANDOM_LONG (1, 100) <= (66 - 2 * gpGlobals->maxClients))
      {
         pBot->BotChat.b_saytext_idle = TRUE; // complain
         pBot->f_bot_saytext_time = gpGlobals->time;

         // once out of three times send a radio message
         if (RANDOM_LONG (1, 100) < 34)
            if (RANDOM_LONG (1, 100) < 50)
               BotTalkOnTheRadio (pBot, RADIOMSG_SECTORCLEAR);
            else
               BotTalkOnTheRadio (pBot, RADIOMSG_REPORTINGIN);
      }
   }

   // should the bot yell for backup ?
   if (!pBot->b_help_asked && (pBot->pBotEnemy != NULL) && (pBot->bot_health <= 20))
      if (RANDOM_LONG (1, 100) <= (91 - 2 * gpGlobals->maxClients))
      {
         pBot->BotChat.b_saytext_help = TRUE; // yell
         pBot->f_bot_saytext_time = gpGlobals->time;
         BotTalkOnTheRadio (pBot, RADIOMSG_NEEDBACKUP); // send a radio message
         pBot->b_help_asked = TRUE; // don't do it twice
      }

   // is the bot planting the bomb ?
   if (pBot->b_is_planting)
   {
      if (BotCheckForEnemies (pBot) != 0)
         pBot->b_is_planting = FALSE; // stop planting if enemy found
      if (b_bomb_planted)
      {
         pBot->b_is_planting = FALSE; // finished planting when "bomb planted" message received
         if (BotHasPrimary (pBot)
             && !((pBot->bot_weapons & (1 << CS_WEAPON_M3))
                  || (pBot->bot_weapons & (1 << CS_WEAPON_XM1014))))
            if (BotCanCampNearHere (pBot, pBot->pEdict->v.origin)
                && (RANDOM_LONG (1, 100) <= (91 - 2 * gpGlobals->maxClients)))
               BotTalkOnTheRadio (pBot, RADIOMSG_INPOSITION); // let the bot camp here
      }
      else if (pBot->current_weapon.iId != CS_WEAPON_C4)
         FakeClientCommand (pBot->pEdict, "weapon_c4"); // take the C4
      pBot->BotMove.f_forward_time = 0; // don't move
      BotPointGun (pBot, Vector (-45, pBot->pEdict->v.v_angle.y, pBot->pEdict->v.v_angle.z)); // look down at 45 degree angle
      pBot->pEdict->v.button = IN_DUCK | IN_ATTACK; // plant the bomb
      g_engfuncs.pfnRunPlayerMove (pBot->pEdict, UTIL_WrapAngles (pBot->pEdict->v.v_angle), 0, 0, 0, pBot->pEdict->v.button, 0, pBot->msecval);
      return;
   }

   // else is the bot defusing bomb ?
   else if (pBot->b_is_defusing)
   {
      if (!b_bomb_planted)
         pBot->b_is_defusing = FALSE; // finished defusing when "bomb defused" message received
      BotPointGun (pBot, UTIL_VecToAngles (pBot->v_goal - GetGunPosition (pBot->pEdict))); // look at bomb
      pBot->BotMove.f_forward_time = 0; // don't move
      pBot->pEdict->v.button = IN_DUCK | IN_USE; // defuse the bomb
      g_engfuncs.pfnRunPlayerMove (pBot->pEdict, UTIL_WrapAngles (pBot->pEdict->v.v_angle), 0, 0, 0, pBot->pEdict->v.button, 0, pBot->msecval);
      return;
   }

   // is the bot camping AND is the bomb about to explode (flee time based on skill) ?
   if ((pBot->f_camp_time > gpGlobals->time) && (f_bomb_explode_time > 0)
       && (f_bomb_explode_time - (10 - pBot->bot_skill) < gpGlobals->time))
   {
      pBot->v_place_to_keep = Vector (0, 0, 0); // forget our camp spot...
      pBot->f_camp_time = gpGlobals->time; // ...get up...
      pBot->f_rush_time = gpGlobals->time + 60.0; // ...and run away !!
      if (RANDOM_LONG (1, 100) <= (91 - 2 * gpGlobals->maxClients))
         BotTalkOnTheRadio (pBot, RADIOMSG_FALLBACK); // bot says, "fallback team !"
   }

   // has the bot been ordered something ?
   if ((pBot->bot_order != BOT_ORDER_NOORDER) && (pBot->f_order_time + 1.0 < gpGlobals->time))
      BotAnswerToOrder (pBot); // answer to this order

   // is the bot alive and should the bot buy stuff now ?
   if ((pBot->pEdict->v.health > 0) && (pBot->pEdict->v.deadflag == DEAD_NO)
       && (pBot->f_buy_time > 0) && (pBot->f_buy_time < gpGlobals->time))
   {
      BotBuyStuff (pBot); // buy stuff
      g_engfuncs.pfnRunPlayerMove (pBot->pEdict, UTIL_WrapAngles (pBot->pEdict->v.v_angle), pBot->BotMove.f_move_speed, pBot->BotMove.f_strafe_speed, 0, pBot->pEdict->v.button, 0, pBot->msecval);
      return;
   }

   // is the bot blinded (e.g. affected by a flashbang) ?
   if (pBot->blinded_time > gpGlobals->time)
   {
      if (pBot->idle_angle_time <= gpGlobals->time)
      {
         pBot->idle_angle_time = gpGlobals->time + RANDOM_FLOAT (0.5, 2.0);
         BotSetIdealYaw (pBot, pBot->idle_angle + RANDOM_FLOAT (-20, 20));
      }

      BotChangeYaw (pBot, pBot->pEdict->v.yaw_speed / 10); // turn slower towards ideal yaw
      pBot->BotMove.f_duck_time = gpGlobals->time + RANDOM_FLOAT (0.5, 1.0); // duck when blinded

      // pick up a random strafe direction
      if (RANDOM_LONG (1, 100) < 50)
         pBot->BotMove.f_strafeleft_time = gpGlobals->time + 0.1;
      else
         pBot->BotMove.f_straferight_time = gpGlobals->time + 0.1;

      if (RANDOM_LONG (0, 100) < 50)
         pBot->BotMove.b_emergency_walkback = TRUE;

      g_engfuncs.pfnRunPlayerMove (pBot->pEdict, UTIL_WrapAngles (pBot->pEdict->v.v_angle), pBot->BotMove.f_move_speed, pBot->BotMove.f_strafe_speed, 0, pBot->pEdict->v.button, 0, pBot->msecval);
      return;
   }
   else
      pBot->idle_angle = UTIL_WrapAngle (pBot->pEdict->v.v_angle.y); // the bot is not idle

   // check if time to check for player sounds (if don't already have enemy)
   if ((pBot->f_sound_update_time < gpGlobals->time) && (pBot->pBotEnemy == NULL) && !b_botdontshoot)
   {
      int ind;
      edict_t *pPlayer;

      pBot->f_sound_update_time = gpGlobals->time + 1.0;

      for (ind = 1; ind <= gpGlobals->maxClients; ind++)
      {
         pPlayer = INDEXENT (ind);

         if (!pPlayer || pPlayer->free || (pPlayer == pBot->pEdict))
            continue; // skip invalid players and skip self (i.e. this bot)

         if (b_observer_mode && (pPlayer->v.flags & FL_CLIENT))
            continue; // if observer mode enabled, don't listen to real players...

         // if this player is alive AND not teammate AND human-like
         if (IsAlive (pPlayer) && (GetTeam (pPlayer) != GetTeam (pBot->pEdict))
             && ((pPlayer->v.flags & FL_CLIENT) || (pPlayer->v.flags & FL_FAKECLIENT)))
         {
            // check for sounds being made by other players...
            if (BotCheckForSounds (pBot, pPlayer))
               pBot->f_sound_update_time = gpGlobals->time + 10.0; // next check in 10 seconds
         }
      }
   }

   // turn towards ideal pitch and ideal yaw by pitch_speed and yaw_speed degrees
   pitch_degrees = BotChangePitch (pBot, pBot->pEdict->v.pitch_speed);
   yaw_degrees = BotChangeYaw (pBot, pBot->pEdict->v.yaw_speed);

   if (yaw_degrees <= 1.0)
      pBot->b_is_walking_straight = TRUE;
   else
      pBot->b_is_walking_straight = FALSE;

   if ((pitch_degrees >= 10) || (yaw_degrees >= 10))
      pBot->BotMove.b_is_walking = TRUE; // slow down if turning a lot

   // let's look for enemies...
   if (b_botdontshoot)
      pBot->pBotEnemy = NULL; // botdontshoot is set, bot won't look for enemies
   else
      pBot->pBotEnemy = BotCheckForEnemies (pBot); // see if there are visible enemies

   // avoid walls, corners and teammates
   if (pBot->f_avoid_time < gpGlobals->time)
      BotAvoidObstacles (pBot);

   // are there armed grenades near us ?
   if (BotCheckForGrenades (pBot))
      pBot->BotMove.b_emergency_walkback = TRUE;
   else
      pBot->BotMove.b_emergency_walkback = FALSE;

   // does an enemy exist?
   if (pBot->pBotEnemy != NULL)
   {
      BotShootAtEnemy (pBot); // shoot at the enemy
      pBot->f_pause_time = 0; // dont't pause if enemy exists
   }

   // else did the bot just lost his enemy ?
   else if (pBot->f_lost_enemy_time + 5.0 > gpGlobals->time)
   {
      // if bot is waiting for enemy to strike back, don't move
      if (pBot->f_pause_time > gpGlobals->time)
         pBot->f_lost_enemy_time = gpGlobals->time; // set lost enemy time to now

      // else rush after that coward one
      else if ((pBot->v_lastseenenemy_position - pBot->pEdict->v.origin).Length () > 50)
         BotReachPosition (pBot, pBot->v_lastseenenemy_position); // chase

      else
         pBot->f_lost_enemy_time = 0.0; // here we are, seems that bot really lost enemy
   }

   // else look for special zones
   else if (BotCheckForSpecialZones (pBot))
   {
      pBot->pEdict->v.angles.y = UTIL_WrapAngle (pBot->pEdict->v.v_angle.y); // body angles = view angles

      // check if the bot is stuck, not paused and NOT on a ladder since handled elsewhere
      if (BotIsStuck (pBot) && (pBot->pEdict->v.movetype != MOVETYPE_FLY) && (pBot->f_pause_time < gpGlobals->time))
         BotUnstuck (pBot); // try to unstuck our poor bot

      if (pBot->pEdict->v.flFallVelocity > pBot->BotMove.f_max_speed)
         pBot->fall_time = gpGlobals->time; // save bot fall time

      pBot->v_prev_position = pBot->pEdict->v.origin; // save previous position (for checking if stuck)
      g_engfuncs.pfnRunPlayerMove (pBot->pEdict, UTIL_WrapAngles (pBot->pEdict->v.v_angle), pBot->BotMove.f_move_speed, pBot->BotMove.f_strafe_speed, 0, pBot->pEdict->v.button, 0, pBot->msecval);
      return;
   }

   // is bot keeping a place ?
   if (pBot->v_place_to_keep != Vector (0, 0, 0))
      BotStayInPosition (pBot);

   // else is bot being "used" ?
   else if (pBot->pBotUser != NULL)
      BotFollowUser (pBot);

   // else may the bot spray a logo (don't spray if bot has an enemy) ?
   else if ((pBot->f_spraying_logo_time > gpGlobals->time) && (pBot->pBotEnemy == NULL))
   {
      pBot->BotMove.f_forward_time = 0; // don't move
      BotPointGun (pBot, Vector (-45, pBot->pEdict->v.v_angle.y, pBot->pEdict->v.v_angle.z)); // look down at 45 degree angle

      if ((pBot->f_spraying_logo_time - 0.75 < gpGlobals->time) && (!pBot->b_logo_sprayed))
      {
         // spray logo when finished looking down
         pBot->b_logo_sprayed = TRUE; // remember this is done
         pBot->BotMove.f_backwards_time = gpGlobals->time + RANDOM_FLOAT (0.5, 1.0); // move back

         // the "impulse 201" command is actually passed in pfnRunPlayerMove for fake clients...
         g_engfuncs.pfnRunPlayerMove (pBot->pEdict, UTIL_WrapAngles (pBot->pEdict->v.v_angle), 0, 0, 0, 0, 201, pBot->msecval);
         return;
      }

      pBot->f_reach_time = gpGlobals->time + 0.5; // don't reach point for half a second
   }

   // else if no enemy & nothing special to do...
   else if (pBot->pBotEnemy == NULL)
   {
      if (pBot->f_find_item_time < gpGlobals->time)
         BotCheckForItems (pBot); // if time to, see if there are any visible items
      else
         pBot->b_is_picking_item = FALSE;

      if (pBot->f_find_goal_time < gpGlobals->time)
         BotFindGoal (pBot); // if time to, find a new goal

      BotWander (pBot); // then just wander around
   }

   if (pBot->f_pause_time > gpGlobals->time) // is the bot "paused"?
      pBot->BotMove.f_forward_time = 0; // don't move while pausing

   // make the body face the same way the bot is looking
   pBot->pEdict->v.angles.y = UTIL_WrapAngle (pBot->pEdict->v.v_angle.y);

   if (pBot->pEdict->v.flFallVelocity > pBot->BotMove.f_max_speed)
      pBot->fall_time = gpGlobals->time; // save bot fall time

   pBot->v_prev_position = pBot->pEdict->v.origin; // save previous position (for checking if stuck)
   g_engfuncs.pfnRunPlayerMove (pBot->pEdict, UTIL_WrapAngles (pBot->pEdict->v.v_angle), pBot->BotMove.f_move_speed, pBot->BotMove.f_strafe_speed, 0, pBot->pEdict->v.button, 0, pBot->msecval);
   return;
}


bool BotItemIsInteresting (bot_t *pBot, edict_t *pItem)
{
   if ((pBot->pEdict == NULL) || (pItem == NULL))
      return FALSE; // reliability check

   // if bot has no primary weapon or little ammo or no ammo left and this is a primary weapon...
   if ((!BotHasPrimary (pBot)
        || (BotHoldsPrimary (pBot) && (pBot->current_weapon.iAmmo1 < 8))
        || (BotHasPrimary (pBot) && BotHoldsSecondary (pBot)))
       && ItemIsPrimary (pItem))
      return TRUE; // this item is really interesting

   // if bot has no secondary weapon or little ammo and this is a secondary weapon...
   if ((!BotHasSecondary (pBot)
        || (BotHoldsSecondary (pBot) && (pBot->current_weapon.iAmmo1 < 8)))
       && ItemIsSecondary (pItem))
      return TRUE; // this item is really interesting

   return FALSE; // all other stuff may not be interesting
}


void BotDiscardItem (bot_t *pBot, edict_t *pItem)
{
   if ((pBot->pEdict == NULL) || (pItem == NULL))
      return; // reliability check

   // if bot is wanting to pick up a primary weapon and needs to discard one to do so...
   if (ItemIsPrimary (pItem) && BotHasPrimary (pBot))
   {
      // if the bot is not currently holding his primary weapon, select it
      if (!BotHoldsPrimary (pBot))
      {
         if (pBot->bot_weapons & (1 << CS_WEAPON_AK47))
            FakeClientCommand (pBot->pEdict, "weapon_ak47");
         else if (pBot->bot_weapons & (1 << CS_WEAPON_AUG))
            FakeClientCommand (pBot->pEdict, "weapon_aug");
         else if (pBot->bot_weapons & (1 << CS_WEAPON_AWP))
            FakeClientCommand (pBot->pEdict, "weapon_awp");
         else if (pBot->bot_weapons & (1 << CS_WEAPON_G3SG1))
            FakeClientCommand (pBot->pEdict, "weapon_g3sg1");
         else if (pBot->bot_weapons & (1 << CS_WEAPON_M249))
            FakeClientCommand (pBot->pEdict, "weapon_m249");
         else if (pBot->bot_weapons & (1 << CS_WEAPON_M3))
            FakeClientCommand (pBot->pEdict, "weapon_m3");
         else if (pBot->bot_weapons & (1 << CS_WEAPON_M4A1))
            FakeClientCommand (pBot->pEdict, "weapon_m4a1");
         else if (pBot->bot_weapons & (1 << CS_WEAPON_MAC10))
            FakeClientCommand (pBot->pEdict, "weapon_mac10");
         else if (pBot->bot_weapons & (1 << CS_WEAPON_MP5NAVY))
            FakeClientCommand (pBot->pEdict, "weapon_mp5navy");
         else if (pBot->bot_weapons & (1 << CS_WEAPON_P228))
            FakeClientCommand (pBot->pEdict, "weapon_p228");
         else if (pBot->bot_weapons & (1 << CS_WEAPON_P90))
            FakeClientCommand (pBot->pEdict, "weapon_p90");
         else if (pBot->bot_weapons & (1 << CS_WEAPON_SCOUT))
            FakeClientCommand (pBot->pEdict, "weapon_scout");
         else if (pBot->bot_weapons & (1 << CS_WEAPON_SG550))
            FakeClientCommand (pBot->pEdict, "weapon_sg550");
         else if (pBot->bot_weapons & (1 << CS_WEAPON_SG552))
            FakeClientCommand (pBot->pEdict, "weapon_sg552");
         else if (pBot->bot_weapons & (1 << CS_WEAPON_TMP))
            FakeClientCommand (pBot->pEdict, "weapon_tmp");
         else if (pBot->bot_weapons & (1 << CS_WEAPON_UMP45))
            FakeClientCommand (pBot->pEdict, "weapon_ump45");
         else if (pBot->bot_weapons & (1 << CS_WEAPON_XM1014))
            FakeClientCommand (pBot->pEdict, "weapon_xm1014");
      }
      else
         FakeClientCommand (pBot->pEdict, "drop"); // discard primary weapon
   }

   // else if the bot wants to pick up a secondary weapon...
   else if (ItemIsSecondary (pItem))
   {
      // if the bot is not currently holding his secondary weapon, select it
      if (!BotHoldsSecondary (pBot))
      {
         if (pBot->bot_weapons & (1 << CS_WEAPON_DEAGLE))
            FakeClientCommand (pBot->pEdict, "weapon_deagle");
         else if (pBot->bot_weapons & (1 << CS_WEAPON_ELITE))
            FakeClientCommand (pBot->pEdict, "weapon_elite");
         else if (pBot->bot_weapons & (1 << CS_WEAPON_FIVESEVEN))
            FakeClientCommand (pBot->pEdict, "weapon_fiveseven");
         else if (pBot->bot_weapons & (1 << CS_WEAPON_GLOCK18))
            FakeClientCommand (pBot->pEdict, "weapon_glock18");
         else if (pBot->bot_weapons & (1 << CS_WEAPON_P228))
            FakeClientCommand (pBot->pEdict, "weapon_p228");
         else if (pBot->bot_weapons & (1 << CS_WEAPON_USP))
            FakeClientCommand (pBot->pEdict, "weapon_usp");
      }
      else
         FakeClientCommand (pBot->pEdict, "drop"); // discard secondary weapon
   }

   pBot->f_find_item_time = gpGlobals->time + 3.0; // delay looking for items
}


void BotTalkOnTheRadio (bot_t *pBot, char *radiocmd, char *radiomsg)
{
   if (pBot->pEdict == NULL)
      return; // reliability check

   // if no radio message was sent for the last 1.5 seconds (this is cleaner...)
   if (f_radiotime + 1.5 < gpGlobals->time)
   {
      // browse down Counter-Strike specific radio menu selections
	   FakeClientCommand (pBot->pEdict, radiocmd);
      FakeClientCommand (pBot->pEdict, radiomsg);

      // save the global radio talk time
      f_radiotime = gpGlobals->time;
   }
}


void BotFindGoal (bot_t *pBot)
{
   edict_t *pEntity = NULL;
   float distance, min_distance;
   bool b_found_goal;

   if (pBot->pEdict == NULL)
      return; // reliability check

   if (b_botdontfind)
      return; // don't process if botdontfind is set

   if ((pBot->v_goal != Vector (0, 0, 0)) && ((pBot->v_goal - pBot->pEdict->v.origin).Length () > 300))
   {
      pBot->f_find_goal_time = gpGlobals->time + RANDOM_FLOAT (1.0, 5.0); // set next lookout time
      return; // bot already has a goal to reach
   }

   // check if bot is VIP
   if (strcmp (g_engfuncs.pfnInfoKeyValue ((*g_engfuncs.pfnGetInfoKeyBuffer) (pBot->pEdict), "model"), "vip") == 0)
   {
      min_distance = 9999.0; // reset min_distance
      b_found_goal = FALSE; // reset found goal flag

      // loop through all VIP safety zones
      while ((pEntity = UTIL_FindEntityByClassname (pEntity, "func_vip_safetyzone")) != NULL)
      {
         distance = (VecBModelOrigin (pEntity) - pBot->pEdict->v.origin).Length ();

         // randomly choose the nearest goal
         if ((distance < min_distance) && ((RANDOM_LONG (1, 100) < 66) || !b_found_goal))
         {
            b_found_goal = TRUE; // bot found a goal
            min_distance = distance; // update the minimum distance
            pBot->v_goal = VecBModelOrigin (pEntity); // remember this entity
         }
      }
   }

   // else check if bot is terrorist
   else if (GetTeam (pBot->pEdict) == CS_TERRORIST)
   {
      min_distance = 9999.0; // reset min_distance
      b_found_goal = FALSE; // reset found goal flag

      // loop through all escape zones
      while ((pEntity = UTIL_FindEntityByClassname (pEntity, "func_escapezone")) != NULL)
      {
         distance = (VecBModelOrigin (pEntity) - pBot->pEdict->v.origin).Length ();

         // randomly choose the nearest goal
         if ((distance < min_distance) && ((RANDOM_LONG (1, 100) < 66) || !b_found_goal))
         {
            b_found_goal = TRUE; // bot found a goal
            min_distance = distance; // update the minimum distance
            pBot->v_goal = VecBModelOrigin (pEntity); // remember this entity
         }
      }

      min_distance = 9999.0; // reset min_distance
      b_found_goal = FALSE; // reset found goal flag

      // loop through all bomb spots
      while ((pEntity = UTIL_FindEntityByClassname (pEntity, "func_bomb_target")) != NULL)
      {
         distance = (VecBModelOrigin (pEntity) - pBot->pEdict->v.origin).Length ();

         // randomly choose the nearest goal if bomb not planted yet
         if ((distance < min_distance) && ((RANDOM_LONG (1, 100) < 66) || !b_found_goal) && !b_bomb_planted)
         {
            b_found_goal = TRUE; // bot found a goal
            min_distance = distance; // update the minimum distance
            pBot->v_goal = VecBModelOrigin (pEntity); // remember this entity
         }
      }
   }

   // else check if bot is counter-terrorist
   else if (GetTeam (pBot->pEdict) == CS_COUNTER_TERRORIST)
   {
      min_distance = 9999.0; // reset min_distance
      b_found_goal = FALSE; // reset found goal flag

      // loop through all escape zones
      while ((pEntity = UTIL_FindEntityByClassname (pEntity, "func_escapezone")) != NULL)
      {
         distance = (VecBModelOrigin (pEntity) - pBot->pEdict->v.origin).Length ();

         // randomly choose the nearest goal
         if ((distance < min_distance) && ((RANDOM_LONG (1, 100) < 66) || !b_found_goal))
         {
            b_found_goal = TRUE; // bot found a goal
            min_distance = distance; // update the minimum distance
            pBot->v_goal = VecBModelOrigin (pEntity); // remember this entity
         }
      }

      min_distance = 9999.0; // reset min_distance
      b_found_goal = FALSE; // reset found goal flag

      // loop through all hostages
      while ((pEntity = UTIL_FindEntityByClassname (pEntity, "hostage_entity")) != NULL)
      {
         distance = (pEntity->v.origin - pBot->pEdict->v.origin).Length ();

         // randomly choose the nearest goal
         if ((distance < min_distance) && ((RANDOM_LONG (1, 100) < 66) || !b_found_goal))
         {
            b_found_goal = TRUE; // bot found a goal
            min_distance = distance; // update the minimum distance
            pBot->v_goal = pEntity->v.origin; // remember this entity
         }
      }

      min_distance = 9999.0; // reset min_distance
      b_found_goal = FALSE; // reset found goal flag

      // loop through all hostage rescue zones
      while ((pEntity = UTIL_FindEntityByClassname (pEntity, "func_hostage_rescue")) != NULL)
      {
         distance = (VecBModelOrigin (pEntity) - pBot->pEdict->v.origin).Length ();

         // randomly choose the nearest goal if bot has taken one or more hostage
         if ((distance < min_distance) && ((RANDOM_LONG (1, 100) < 66) || !b_found_goal) && pBot->b_has_valuable)
         {
            b_found_goal = TRUE; // bot found a goal
            min_distance = distance; // update the minimum distance
            pBot->v_goal = VecBModelOrigin (pEntity); // remember this entity
         }
      }

      min_distance = 9999.0; // reset min_distance
      b_found_goal = FALSE; // reset found goal flag

      // loop through all bomb spots
      while ((pEntity = UTIL_FindEntityByClassname (pEntity, "func_bomb_target")) != NULL)
      {
         distance = (VecBModelOrigin (pEntity) - pBot->pEdict->v.origin).Length ();

         // randomly choose the nearest goal if bomb is already planted
         if ((distance < min_distance) && ((RANDOM_LONG (1, 100) < 66) || !b_found_goal) && b_bomb_planted)
         {
            b_found_goal = TRUE; // bot found a goal
            min_distance = distance; // update the minimum distance
            pBot->v_goal = VecBModelOrigin (pEntity); // remember this entity
         }
      }
   }

   pBot->f_find_goal_time = gpGlobals->time + 10.0; // next goal lookout in 10 seconds
   return;
}


void BotAnswerToOrder (bot_t *pBot)
{
   if ((pBot->pEdict == NULL) || (pBot->pAskingEntity == NULL))
      return; // reliability check

   // has the bot been asked to follow someone ?
   if (pBot->bot_order == BOT_ORDER_FOLLOW)
   {
      // does the bot want to follow the caller ?
      if ((pBot->pBotEnemy == NULL)
          && ((RANDOM_LONG (1, 100) < 80) && pBot->b_is_fearful)
              || ((RANDOM_LONG (1, 100) < 40) && !pBot->b_is_fearful))
      {
         pBot->v_place_to_keep = Vector (0, 0, 0); // don't stay in position anymore
         pBot->pBotUser = pBot->pAskingEntity; // mark this client as using the bot
         pBot->v_lastseenuser_position = pBot->pAskingEntity->v.origin; // remember last seen user position
         pBot->BotChat.b_saytext_follow = TRUE; // bot acknowledges
         pBot->f_bot_saytext_time = gpGlobals->time + RANDOM_FLOAT (0.5, 1.5);
         BotTalkOnTheRadio (pBot, RADIOMSG_AFFIRMATIVE); // send a radio message
      }
      else
      {
         pBot->BotChat.b_saytext_negative = TRUE; // bot refuses
         pBot->f_bot_saytext_time = gpGlobals->time + RANDOM_FLOAT (0.5, 1.5);
         BotTalkOnTheRadio (pBot, RADIOMSG_NEGATIVE); // send a radio message
      }
   }

   // else has the bot been asked to check in ?
   else if (pBot->bot_order == BOT_ORDER_REPORT)
   {
      // does the bot want to answer the caller ?
      if (RANDOM_LONG (1, 100) < 66)
      {
         // does the bot have no enemy ?
         if (pBot->pBotEnemy == NULL)
         {
            pBot->BotChat.b_sayaudio_reporting = TRUE; // set him for reporting
            pBot->f_bot_sayaudio_time = gpGlobals->time + RANDOM_FLOAT (1.0, 2.0);
            if (RANDOM_LONG (1, 100) < 50)
               BotTalkOnTheRadio (pBot, RADIOMSG_SECTORCLEAR); // send "sector clear" radio message
            else
               BotTalkOnTheRadio (pBot, RADIOMSG_REPORTINGIN); // send "checking in" radio message
         }
         else
         {
            pBot->BotChat.b_sayaudio_attacking = TRUE; // bot yells attack (audio)
            pBot->f_bot_sayaudio_time = gpGlobals->time + RANDOM_FLOAT (1.0, 2.0);
            BotTalkOnTheRadio (pBot, RADIOMSG_UNDERFIRE); // send "under fire" radio message
         }
      }
   }

   // else has the bot been asked to keep a position ?
   else if (pBot->bot_order == BOT_ORDER_STAY)
   {
      // does the bot wants to obey the caller ?
      if ((pBot->pBotEnemy == NULL)
          && ((RANDOM_LONG (1, 100) < 80) && pBot->b_is_fearful)
              || ((RANDOM_LONG (1, 100) < 40) && !pBot->b_is_fearful))
      {
         pBot->v_place_to_keep = pBot->pEdict->v.origin; // position to stay in
         pBot->f_place_time = gpGlobals->time; // remember when we last saw the place to keep
         pBot->pBotUser = NULL; // free the user client slot
         pBot->v_lastseenuser_position = Vector (0, 0, 0); // forget last seen user position
         pBot->BotChat.b_saytext_stay = TRUE; // bot acknowledges
         pBot->f_bot_saytext_time = gpGlobals->time + RANDOM_FLOAT (0.5, 1.5);
         BotTalkOnTheRadio (pBot, RADIOMSG_INPOSITION); // send a radio message
      }
      else
      {
         pBot->BotChat.b_saytext_negative = TRUE; // bot refuses
         pBot->f_bot_saytext_time = gpGlobals->time + RANDOM_FLOAT (0.5, 1.5);
         BotTalkOnTheRadio (pBot, RADIOMSG_NEGATIVE); // send a radio message
      }
   }

   // else has the bot been asked to rush on his own ?
   else if (pBot->bot_order == BOT_ORDER_GO)
   {
      pBot->v_place_to_keep = Vector (0, 0, 0); // don't stay in position anymore
      pBot->pBotUser = NULL; // free the user client slot
      pBot->v_lastseenuser_position = Vector (0, 0, 0); // forget last seen user position
      if (!pBot->b_is_fearful)
         pBot->f_rush_time = gpGlobals->time + RANDOM_FLOAT (15.0, 45.0); // rush if not fearful

      // does the bot want to answer the caller ?
      if (RANDOM_LONG (1, 100) < 50)
         if (RANDOM_LONG (1, 100) < 66)
         {
            pBot->BotChat.b_saytext_affirmative = TRUE; // bot acknowledges
            pBot->f_bot_saytext_time = gpGlobals->time + RANDOM_FLOAT (0.5, 3.5);
         }
         else
            BotTalkOnTheRadio (pBot, RADIOMSG_AFFIRMATIVE); // send a radio message
   }

   pBot->bot_order = BOT_ORDER_NOORDER; // reset bot order field
   return;
}


bool BotHasPrimary (bot_t *pBot)
{
   return ((pBot->bot_weapons & (1 << CS_WEAPON_AK47))
           || (pBot->bot_weapons & (1 << CS_WEAPON_AUG))
           || (pBot->bot_weapons & (1 << CS_WEAPON_AWP))
           || (pBot->bot_weapons & (1 << CS_WEAPON_G3SG1))
           || (pBot->bot_weapons & (1 << CS_WEAPON_M249))
           || (pBot->bot_weapons & (1 << CS_WEAPON_M3))
           || (pBot->bot_weapons & (1 << CS_WEAPON_M4A1))
           || (pBot->bot_weapons & (1 << CS_WEAPON_MAC10))
           || (pBot->bot_weapons & (1 << CS_WEAPON_MP5NAVY))
           || (pBot->bot_weapons & (1 << CS_WEAPON_P228))
           || (pBot->bot_weapons & (1 << CS_WEAPON_P90))
           || (pBot->bot_weapons & (1 << CS_WEAPON_SCOUT))
           || (pBot->bot_weapons & (1 << CS_WEAPON_SG550))
           || (pBot->bot_weapons & (1 << CS_WEAPON_SG552))
           || (pBot->bot_weapons & (1 << CS_WEAPON_TMP))
           || (pBot->bot_weapons & (1 << CS_WEAPON_UMP45))
           || (pBot->bot_weapons & (1 << CS_WEAPON_XM1014)));
}


bool BotHasSecondary (bot_t *pBot)
{
   return ((pBot->bot_weapons & (1 << CS_WEAPON_DEAGLE))
           || (pBot->bot_weapons & (1 << CS_WEAPON_ELITE))
           || (pBot->bot_weapons & (1 << CS_WEAPON_FIVESEVEN))
           || (pBot->bot_weapons & (1 << CS_WEAPON_GLOCK18))
           || (pBot->bot_weapons & (1 << CS_WEAPON_P228))
           || (pBot->bot_weapons & (1 << CS_WEAPON_USP)));
}


bool BotHoldsPrimary (bot_t *pBot)
{
   return ((pBot->current_weapon.iId == CS_WEAPON_AK47)
           || (pBot->current_weapon.iId == CS_WEAPON_AUG)
           || (pBot->current_weapon.iId == CS_WEAPON_AWP)
           || (pBot->current_weapon.iId == CS_WEAPON_G3SG1)
           || (pBot->current_weapon.iId == CS_WEAPON_M249)
           || (pBot->current_weapon.iId == CS_WEAPON_M3)
           || (pBot->current_weapon.iId == CS_WEAPON_M4A1)
           || (pBot->current_weapon.iId == CS_WEAPON_MAC10)
           || (pBot->current_weapon.iId == CS_WEAPON_MP5NAVY)
           || (pBot->current_weapon.iId == CS_WEAPON_P228)
           || (pBot->current_weapon.iId == CS_WEAPON_P90)
           || (pBot->current_weapon.iId == CS_WEAPON_SCOUT)
           || (pBot->current_weapon.iId == CS_WEAPON_SG550)
           || (pBot->current_weapon.iId == CS_WEAPON_SG552)
           || (pBot->current_weapon.iId == CS_WEAPON_TMP)
           || (pBot->current_weapon.iId == CS_WEAPON_UMP45)
           || (pBot->current_weapon.iId == CS_WEAPON_XM1014));
}


bool BotHoldsSecondary (bot_t *pBot)
{
   return ((pBot->current_weapon.iId == CS_WEAPON_DEAGLE)
           || (pBot->current_weapon.iId == CS_WEAPON_ELITE)
           || (pBot->current_weapon.iId == CS_WEAPON_FIVESEVEN)
           || (pBot->current_weapon.iId == CS_WEAPON_GLOCK18)
           || (pBot->current_weapon.iId == CS_WEAPON_P228)
           || (pBot->current_weapon.iId == CS_WEAPON_USP));
}


bool ItemIsPrimary (edict_t *pItem)
{
   if (pItem == NULL)
      return FALSE; // reliability check

   return ((strcmp (STRING (pItem->v.model), "models/w_ak47.mdl") == 0)
           || (strcmp (STRING (pItem->v.model), "models/w_aug.mdl") == 0)
           || (strcmp (STRING (pItem->v.model), "models/w_awp.mdl") == 0)
           || (strcmp (STRING (pItem->v.model), "models/w_g3sg1.mdl") == 0)
           || (strcmp (STRING (pItem->v.model), "models/w_m249.mdl") == 0)
           || (strcmp (STRING (pItem->v.model), "models/w_m3.mdl") == 0)
           || (strcmp (STRING (pItem->v.model), "models/w_m4a1.mdl") == 0)
           || (strcmp (STRING (pItem->v.model), "models/w_mac10.mdl") == 0)
           || (strcmp (STRING (pItem->v.model), "models/w_mp5.mdl") == 0)
           || (strcmp (STRING (pItem->v.model), "models/w_p228.mdl") == 0)
           || (strcmp (STRING (pItem->v.model), "models/w_p90.mdl") == 0)
           || (strcmp (STRING (pItem->v.model), "models/w_scout.mdl") == 0)
           || (strcmp (STRING (pItem->v.model), "models/w_sg550.mdl") == 0)
           || (strcmp (STRING (pItem->v.model), "models/w_sg552.mdl") == 0)
           || (strcmp (STRING (pItem->v.model), "models/w_tmp.mdl") == 0)
           || (strcmp (STRING (pItem->v.model), "models/w_ump45.mdl") == 0)
           || (strcmp (STRING (pItem->v.model), "models/w_xm1014.mdl") == 0));
}


bool ItemIsSecondary (edict_t *pItem)
{
   if (pItem == NULL)
      return FALSE; // reliability check

   return ((strcmp (STRING (pItem->v.model), "models/w_deagle.mdl") == 0)
           || (strcmp (STRING (pItem->v.model), "models/w_elite.mdl") == 0)
           || (strcmp (STRING (pItem->v.model), "models/w_fiveseven.mdl") == 0)
           || (strcmp (STRING (pItem->v.model), "models/w_glock18.mdl") == 0)
           || (strcmp (STRING (pItem->v.model), "models/w_p228.mdl") == 0)
           || (strcmp (STRING (pItem->v.model), "models/w_usp.mdl") == 0));
}


void UpdateBulletSounds (edict_t *pEdict)
{
   if (pEdict == NULL)
      return; // reliability check

   // check if this player is actually attacking something
   if (((pEdict->v.button & IN_ATTACK) == 0)
       || ((pEdict->v.button & IN_ATTACK2) == 0))
      return; // give up, this player is not firing

   // message indicates that the player is decreasing his ammo, was this ammo a bullet ?
   if ((strcmp (STRING (pEdict->v.weaponmodel), "models/p_c4") == 0)
       || (strcmp (STRING (pEdict->v.weaponmodel), "models/p_knife") == 0)
       || (strcmp (STRING (pEdict->v.weaponmodel), "models/p_flashbang") == 0)
       || (strcmp (STRING (pEdict->v.weaponmodel), "models/p_hegrenade") == 0)
       || (strcmp (STRING (pEdict->v.weaponmodel), "models/p_smokegrenade") == 0))
      return; // give up, player's weapon does not fire bullets

   // determine the impact point
   TraceResult tr;

   UTIL_MakeVectors (pEdict->v.v_angle); // build base vectors
   UTIL_TraceLine (GetGunPosition (pEdict), gpGlobals->v_forward * 9999, dont_ignore_monsters, ignore_glass, pEdict, &tr);

   // cycle through all bots and alert those who saw the bullet ricochet
   for (int index = 0; index < 32; index++)
   {
      // does this bot have no enemy AND ricochet was close to bot AND bot can see ricochet ?
      if ((bots[index].pEdict != pEdict) && (bots[index].pBotEnemy == NULL) && ((tr.vecEndPos - bots[index].pEdict->v.origin).Length () < 300)
          && BotCanSeeThis (&bots[index], tr.vecEndPos) && FInViewCone (tr.vecEndPos, bots[index].pEdict))
      {
         BotSetIdealYaw (&bots[index], UTIL_VecToAngles (bots[index].pEdict->v.origin - tr.vecEndPos).y); // face where it came from
         bots[index].f_reach_time = gpGlobals->time + 0.5; // don't reach point for half a second
      }
   }

   return;
}
