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
// bot.cpp
//

#include "racc.h"


// this is the LINK_ENTITY_TO_CLASS function that creates a player (bot)
extern "C" EXPORT void player (entvars_t *pev);


void BotCreate (void)
{
   // this is where the show begins, i.e. the function that creates a bot. How it works :
   // I check which profiles are not currently in use by other bots. Third step, is to ask
   // the engine to create the fakeclient and give it a player entity pointer. And once
   // ClientPutInServer() has been called, ladies and gentlemen, please welcome our new bot.

   bot_t *pBot;
   edict_t *pBotEdict;
   profile_t *pBotProfile;
   char ip_address[32];
   char reject_reason[128];
   char *infobuffer;
   bool profiles_used[RACC_MAX_PROFILES];
   long index, bot_index, profile_index, used_count;

   // reset used profiles flags array
   memset (profiles_used, 0, sizeof (profiles_used));

   // cycle through all bot slots
   for (bot_index = 0; bot_index < *server.max_clients; bot_index++)
   {
      // is this bot active ?
      if (bots[bot_index].is_active && IsValidPlayer (bots[bot_index].pEdict))
      {
         // cycle through all the bot profiles we know
         for (profile_index = 0; profile_index < profile_count; profile_index++)
         {
            // does the bot have the same profile as this one ?
            if (&profiles[profile_index] == bots[bot_index].pProfile)
            {
               profiles_used[profile_index] = TRUE; // this profile is used, so flag it
               used_count++; // increment the used profiles counter
            }
         }
      }
   }

   // if all the profiles are used, that's there aren't enough living bots to join
   if (used_count == profile_count)
   {
      ServerConsole_printf ("RACC: not enough people in cybernetic population!\n"); // tell why
      server.max_bots = bot_count; // max out the bots to the current number
      return; // ...and cancel bot creation
   }

   // pick up a profile that isn't used
   do
      profile_index = RANDOM_LONG (0, profile_count - 1); // pick up one randomly until not used
   while (profiles_used[profile_index]);

   // okay, now we have a valid profile for our new bot
   pBotProfile = &profiles[profile_index];

   pBotEdict = (*g_engfuncs.pfnCreateFakeClient) (pBotProfile->name); // create the fake client
   if (FNullEnt (pBotEdict))
      return; // cancel if unable to create fake client

   // link his entity to an useful pointer
   pBot = &bots[ENTINDEX (pBotEdict) - 1];
   pBot->pEdict = pBotEdict;
   pBot->pProfile = pBotProfile;

   if (pBot->pEdict->pvPrivateData != NULL)
      FREE_PRIVATE (pBot->pEdict); // free our predecessor's private data
   pBot->pEdict->pvPrivateData = NULL; // fools the private data pointer 
   pBot->pEdict->v.frags = 0; // reset his frag count 

   // initialize his weapons database pointers
   memset (&pBot->bot_weapons, 0, sizeof (pBot->bot_weapons));
   for (index = 0; index < MAX_WEAPONS; index++)
   {
      pBot->bot_weapons[index].hardware = &weapons[index];
      pBot->bot_weapons[index].primary_ammo = &pBot->bot_ammos[weapons[index].primary.type_of_ammo];
      pBot->bot_weapons[index].secondary_ammo = &pBot->bot_ammos[weapons[index].secondary.type_of_ammo];
   }
   pBot->current_weapon = &pBot->bot_weapons[0]; // set current weapon pointer to failsafe value

   // create the player entity by calling MOD's player() function
   player (&pBot->pEdict->v);

   index = ENTINDEX (pBot->pEdict); // get his client index
   infobuffer = (*g_engfuncs.pfnGetInfoKeyBuffer) (pBot->pEdict); // get his info buffer

   // set him some parameters in the infobuffer
   (*g_engfuncs.pfnSetClientKeyValue) (index, infobuffer, "model", "gordon");
   (*g_engfuncs.pfnSetClientKeyValue) (index, infobuffer, "rate", "3500.000000");
   (*g_engfuncs.pfnSetClientKeyValue) (index, infobuffer, "cl_updaterate", "20");
   (*g_engfuncs.pfnSetClientKeyValue) (index, infobuffer, "cl_lw", "1");
   (*g_engfuncs.pfnSetClientKeyValue) (index, infobuffer, "cl_lc", "1");
   (*g_engfuncs.pfnSetClientKeyValue) (index, infobuffer, "tracker", "0");
   (*g_engfuncs.pfnSetClientKeyValue) (index, infobuffer, "cl_dlmax", "128");
   (*g_engfuncs.pfnSetClientKeyValue) (index, infobuffer, "lefthand", "1");
   (*g_engfuncs.pfnSetClientKeyValue) (index, infobuffer, "friends", "0");
   (*g_engfuncs.pfnSetClientKeyValue) (index, infobuffer, "dm", "0");
   (*g_engfuncs.pfnSetClientKeyValue) (index, infobuffer, "ah", "1");

   // let him connect to the server under its own name
   sprintf (ip_address, "127.0.0.%d", ENTINDEX (pBot->pEdict) + 100); // build it an unique address
   ClientConnect (pBot->pEdict, pBot->pProfile->name, ip_address, reject_reason);

   // print a notification message on the dedicated server console if in developer mode
   if (server.is_dedicated && (server.developer_level > 0))
   {
      if (server.developer_level > 1)
      {
         ServerConsole_printf ("Server requiring authentication\n");
         ServerConsole_printf ("Client %s connected\n", STRING (pBot->pEdict->v.netname));
         ServerConsole_printf ("Adr: %s:27005\n", ip_address);
      }
      ServerConsole_printf ("Verifying and uploading resources...\n");
      ServerConsole_printf ("Custom resources total 0 bytes\n");
      ServerConsole_printf ("  Decals:  0 bytes\n");
      ServerConsole_printf ("----------------------\n");
      ServerConsole_printf ("Resources to request: 0 bytes\n");
   }

   // let him actually join the game
   pBot->pEdict->v.flags |= FL_THIRDPARTYBOT; // let ClientPutInServer() know it's a bot connecting
   ClientPutInServer (pBot->pEdict);

   // create his illumination entity (thanks to Tom Simpson from FoxBot for the engine bug fix)
   pBot->pIllumination = pfnCreateNamedEntity (MAKE_STRING ("info_target"));
   Spawn (pBot->pIllumination); // spawn it
   pBot->pIllumination->v.movetype = MOVETYPE_NOCLIP; // set its movement to no clipping
   pBot->pIllumination->v.nextthink = *server.time; // needed to make it think
   pBot->pIllumination->v.classname = MAKE_STRING ("entity_botlightvalue"); // sets its name
   SET_MODEL (pBot->pIllumination, "models/mechgibs.mdl"); // sets it a model

   // initialize all the variables for this bot...

   BotReset (pBot); // reset our bot

   pBot->is_active = TRUE; // set his 'is active' flag
   pBot->bot_money = 0; // reset his money amount
   pBot->BotEyes.BotHUD.menu_state = MENU_CS_IDLE; // not selecting team yet

   // if internet mode is on...
   if (server.is_internetmode)
   {
      pBot->time_to_live = *server.time + RANDOM_LONG (300, 3600); // set him a TTL
      pBot->quit_game_time = pBot->time_to_live + RANDOM_FLOAT (3.0, 7.0); // disconnect time
   }
   else
   {
      pBot->time_to_live = -1; // don't set him a TTL (time to live)
      pBot->quit_game_time = -1; // so never quit
   }

   // say hello here
   if (RANDOM_LONG (1, 100) <= (86 - 2 * player_count))
      pBot->BotChat.bot_saytext = BOT_SAYTEXT_HELLO;

   pBot->f_bot_alone_timer = *server.time + RANDOM_LONG (30, 120); // set an idle delay
   pBot->b_not_started = TRUE; // not started yet
}


void BotReset (bot_t *pBot)
{
   if (!IsValidPlayer (pBot->pEdict))
      return; // reliability check

   v_pathdebug_from = pBot->pEdict->v.origin;

   // reset bot's input channels
   memset (&pBot->BotEyes, 0, sizeof (pBot->BotEyes));
   memset (&pBot->BotEars, 0, sizeof (pBot->BotEars));
   memset (&pBot->BotBody, 0, sizeof (pBot->BotBody));

   BotSetIdealAngles (pBot, Vector (RANDOM_FLOAT (-15, 15), RANDOM_FLOAT (-180, 180), 0));
   pBot->pEdict->v.angles.x = -pBot->pEdict->v.v_angle.x / 3;
   pBot->pEdict->v.angles.y = pBot->pEdict->v.v_angle.y;
   pBot->pEdict->v.angles.z = 0;
   pBot->BotMove.f_max_speed = CVAR_GET_FLOAT ("sv_maxspeed");

   pBot->f_find_item_time = 0.0;

   pBot->pBotLadder = NULL;
   pBot->ladder_direction = LADDER_UNKNOWN;
   pBot->f_start_use_ladder_time = 0.0;
   pBot->f_end_use_ladder_time = 0.0;

   if (RANDOM_LONG (1, 100) < 33)
      pBot->b_is_fearful = TRUE;
   else
      pBot->b_is_fearful = FALSE;
   pBot->BotMove.b_emergency_walkback = FALSE;
   pBot->BotMove.f_walk_time = 0.0;
   pBot->BotMove.f_forward_time = 0.0;
   pBot->BotMove.f_backwards_time = 0.0;
   pBot->BotMove.f_jump_time = 0.0;
   pBot->BotMove.f_duck_time = 0.0;
   pBot->BotMove.f_strafeleft_time = 0.0;
   pBot->BotMove.f_straferight_time = 0.0;

   memset (&pBot->BotEnemy, 0, sizeof (pBot->BotEnemy));
   memset (&pBot->LastSeenEnemy, 0, sizeof (pBot->LastSeenEnemy));
   pBot->f_reload_time = -1;
   pBot->pBotUser = NULL;
   pBot->f_bot_use_time = 0.0;
   pBot->f_randomturn_time = 0.0;
   pBot->BotChat.f_saytext_time = 0.0;
   pBot->BotChat.f_sayaudio_time = 0.0;
   pBot->BotChat.bot_saytext = 0;
   pBot->b_help_asked = FALSE;
   pBot->BotChat.bot_sayaudio = BOT_SAYAUDIO_NOTHING;

   pBot->f_shoot_time = *server.time;

   pBot->buy_state = 0;
   pBot->f_buy_time = *server.time + RANDOM_FLOAT (2.0, 5.0);
   pBot->f_rush_time = *server.time + RANDOM_FLOAT (15.0, 45.0);
   pBot->f_pause_time = pBot->f_buy_time;
   pBot->f_sound_update_time = 0;

   pBot->f_find_goal_time = 0;
   pBot->v_goal = g_vecZero;
   pBot->v_place_to_keep = g_vecZero;
   pBot->f_place_time = 0;
   pBot->f_camp_time = 0;
   pBot->f_reach_time = 0;
   pBot->v_reach_point = g_vecZero;
   pBot->f_turncorner_time = *server.time + 5.0;

   pBot->bot_task = BOT_TASK_IDLE;
   pBot->b_interact = FALSE;
   pBot->f_interact_time = 0;
   pBot->b_lift_moving = FALSE;
   pBot->f_spraying_logo_time = 0;
   pBot->b_logo_sprayed = FALSE;
   pBot->has_defuse_kit = FALSE;

   memset (&pBot->bot_ammos, 0, sizeof (pBot->bot_ammos));
}


void BotBuyStuff (bot_t *pBot)
{
   // this function is a state machine

   // these weapon arrays MUST be sorted by INCREASING PRICE ORDER. Default pistols
   // (GLOCK18 and USP) don't appear because we don't want bots to buy them.
   int secondary_weapon_prices[][2] =
   {
      {CS_WEAPON_P228,      600},
      {CS_WEAPON_DEAGLE,    650},
      {CS_WEAPON_FIVESEVEN, 750},
      {CS_WEAPON_ELITE,    1000},
      {0,            0x7FFFFFFF}, // terminator
   };

   int primary_weapon_prices[][2] =
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
      {CS_WEAPON_M249,     5750},
      {0,            0x7FFFFFFF}, // terminator
   };

   if (!IsValidPlayer (pBot->pEdict))
      return; // reliability check

   if (PlayerIsVIP (pBot->pEdict))
   {
      pBot->f_buy_time = 0; // don't buy anything if bot is VIP (VIPs don't buy stuff)
      return;
   }

   // pause the bot for a while
   pBot->f_pause_time = *server.time + 1.2;

   // state 0: if armor is damaged and bot has some money, buy some armor
   if (pBot->buy_state == 0)
   {
      if ((pBot->pEdict->v.armorvalue < 80) && (pBot->bot_money > 1000))
      {
         // if bot is rich, buy kevlar + helmet, else buy a single kevlar
         if (pBot->bot_money > 2500)
            FakeClientCommand (pBot->pEdict, BUY_KEVLARHELMET);
         else
            FakeClientCommand (pBot->pEdict, BUY_KEVLAR);

         pBot->f_buy_time = *server.time + RANDOM_FLOAT (0.3, 0.5); // delay next buy
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

         // handle each case separately
         if (primary_weapon_prices[weapon_index][0] == CS_WEAPON_M3)
            FakeClientCommand (pBot->pEdict, BUY_M3); // buy the M3
         else if (primary_weapon_prices[weapon_index][0] == CS_WEAPON_XM1014)
            FakeClientCommand (pBot->pEdict, BUY_XM1014); // buy the XM1014
         else if (primary_weapon_prices[weapon_index][0] == CS_WEAPON_MP5NAVY)
            FakeClientCommand (pBot->pEdict, BUY_MP5NAVY); // buy the MP5NAVY
         else if (primary_weapon_prices[weapon_index][0] == CS_WEAPON_TMP)
            FakeClientCommand (pBot->pEdict, BUY_TMP); // buy the TMP
         else if (primary_weapon_prices[weapon_index][0] == CS_WEAPON_P90)
            FakeClientCommand (pBot->pEdict, BUY_P90); // buy the P90
         else if (primary_weapon_prices[weapon_index][0] == CS_WEAPON_MAC10)
            FakeClientCommand (pBot->pEdict, BUY_MAC10); // buy the MAC10
         else if (primary_weapon_prices[weapon_index][0] == CS_WEAPON_UMP45)
            FakeClientCommand (pBot->pEdict, BUY_UMP45); // buy the UMP45
         else if (primary_weapon_prices[weapon_index][0] == CS_WEAPON_AK47)
            FakeClientCommand (pBot->pEdict, BUY_AK47); // buy the AK47
         else if (primary_weapon_prices[weapon_index][0] == CS_WEAPON_SG552)
            FakeClientCommand (pBot->pEdict, BUY_SG552); // buy the SG552
         else if (primary_weapon_prices[weapon_index][0] == CS_WEAPON_M4A1)
            FakeClientCommand (pBot->pEdict, BUY_M4A1); // buy the M4A1
         else if (primary_weapon_prices[weapon_index][0] == CS_WEAPON_AUG)
            FakeClientCommand (pBot->pEdict, BUY_AUG); // buy the AUG
         else if (primary_weapon_prices[weapon_index][0] == CS_WEAPON_SCOUT)
            FakeClientCommand (pBot->pEdict, BUY_SCOUT); // buy the SCOUT
         else if (primary_weapon_prices[weapon_index][0] == CS_WEAPON_AWP)
            FakeClientCommand (pBot->pEdict, BUY_AWP); // buy the AWP
         else if (primary_weapon_prices[weapon_index][0] == CS_WEAPON_G3SG1)
            FakeClientCommand (pBot->pEdict, BUY_G3SG1); // buy the G3SG1
         else if (primary_weapon_prices[weapon_index][0] == CS_WEAPON_SG550)
            FakeClientCommand (pBot->pEdict, BUY_SG550); // buy the AK47
         else if (primary_weapon_prices[weapon_index][0] == CS_WEAPON_M249)
            FakeClientCommand (pBot->pEdict, BUY_M249); // buy the M249

         pBot->f_buy_time = *server.time + RANDOM_FLOAT (0.3, 0.5); // delay next buy
      }
      pBot->buy_state++;
      return;
   }

   // states 2, 3, 4: if bot has still some money, buy more primary ammo
   else if ((pBot->buy_state == 2) || (pBot->buy_state == 3) || (pBot->buy_state == 4))
   {
      if (pBot->bot_money > 300)
      {
         FakeClientCommand (pBot->pEdict, BUY_PRIMARYAMMO); // buy some primary ammo
         pBot->f_buy_time = *server.time + 0.3; // delay next buy
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

         // handle each case separately
         if (secondary_weapon_prices[weapon_index][0] == CS_WEAPON_DEAGLE)
            FakeClientCommand (pBot->pEdict, BUY_DEAGLE); // buy the DEAGLE
         else if (secondary_weapon_prices[weapon_index][0] == CS_WEAPON_P228)
            FakeClientCommand (pBot->pEdict, BUY_P228); // buy the P228
         else if (secondary_weapon_prices[weapon_index][0] == CS_WEAPON_ELITE)
            FakeClientCommand (pBot->pEdict, BUY_ELITE); // buy the ELITE
         else if (secondary_weapon_prices[weapon_index][0] == CS_WEAPON_FIVESEVEN)
            FakeClientCommand (pBot->pEdict, BUY_FIVESEVEN); // buy the FIVESEVEN

         pBot->f_buy_time = *server.time + RANDOM_FLOAT (0.3, 0.5); // delay next buy
      }
      pBot->buy_state++;
      return;
   }

   // states 6 and 7: if bot has still some money, buy more secondary ammo
   else if ((pBot->buy_state == 6) || (pBot->buy_state == 7))
   {
      if (pBot->bot_money > 300)
      {
         FakeClientCommand (pBot->pEdict, BUY_SECONDARYAMMO); // buy some secondary ammo
         pBot->f_buy_time = *server.time + 0.3; // delay next buy
      }
      pBot->buy_state++;
      return;
   }

   // state 8: if bot has still some money, choose if bot should buy a grenade or not
   else if (pBot->buy_state == 8)
   {
      if ((pBot->bot_money > 600) && (RANDOM_LONG (1, 100) < 66))
      {
         if (RANDOM_LONG (0, 100) < 66)
            FakeClientCommand (pBot->pEdict, BUY_HEGRENADE); // buy HE grenade
         else if (RANDOM_LONG (0, 100) < 66)
            FakeClientCommand (pBot->pEdict, BUY_FLASHBANG); // buy flashbang
         else
            FakeClientCommand (pBot->pEdict, BUY_SMOKEGRENADE); // buy smokegrenade

         pBot->f_buy_time = *server.time + RANDOM_FLOAT (0.3, 0.5); // delay next buy
      }
      pBot->buy_state++;
      return;
   }

   // state 9: if bot is counter-terrorist and we're on a bomb map, randomly buy the defuse kit
   else if (pBot->buy_state == 9)
   {
      if ((GetTeam (pBot->pEdict) == CS_COUNTER_TERRORIST) && round.b_bomb_map && !pBot->has_defuse_kit
          && (pBot->bot_money > 200) && RANDOM_LONG (1, 100) < 33)
      {
         FakeClientCommand (pBot->pEdict, BUY_DEFUSEKIT); // to buy the defuse kit

         pBot->f_buy_time = *server.time + RANDOM_FLOAT (0.3, 0.5); // delay next buy
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

      if (RANDOM_LONG (1, 100) <= (56 - 2 * player_count))
      {
         pBot->BotChat.bot_sayaudio |= BOT_SAYAUDIO_FIRSTSPAWN; // bot says 'go go go' or something like that
         pBot->BotChat.f_sayaudio_time = *server.time + RANDOM_FLOAT (0.5, 10.0);
      }
      else if (RANDOM_LONG (1, 100) < 50)
      {
         if (RANDOM_LONG (1, 100) < 34)
            FakeClientCommand (pBot->pEdict, RADIOMSG_STICKTOGETHER);
         else if (RANDOM_LONG (1, 100) < 50)
            FakeClientCommand (pBot->pEdict, RADIOMSG_GOGOGO);
         else
            FakeClientCommand (pBot->pEdict, RADIOMSG_STORMTHEFRONT);
      }

      return;
   }
}


bool BotCheckForSpecialZones (bot_t *pBot)
{
   edict_t *pSpecialZone = NULL;
   Vector v_zone_origin;

   if (!IsValidPlayer (pBot->pEdict))
      return (FALSE); // reliability check

   if (DebugLevel.is_dontfindmode)
      return (FALSE); // don't process if botdontfind is set

   // is there a special zone near here?
   while (!FNullEnt (pSpecialZone = UTIL_FindEntityInSphere (pSpecialZone, pBot->pEdict->v.origin, 1000)))
   {
      // check for a visible safety zone
      if ((strcmp ("func_vip_safetyzone", STRING (pSpecialZone->v.classname)) == 0)
          && (BotCanSeeOfEntity (pBot, pSpecialZone) != g_vecZero))
      {
         v_zone_origin = VecBModelOrigin (pSpecialZone);

         // is bot a VIP ?
         if (PlayerIsVIP (pBot->pEdict))
         {
            pBot->BotMove.f_forward_time = *server.time + 60.0; // let our bot go...
            pBot->f_camp_time = *server.time; // ...stop camping...
            pBot->BotMove.f_walk_time = 0.0; // ...and the sooner he gets there, the better it is !

            // is bot NOT already reaching this safety zone ?
            if (pBot->v_reach_point != v_zone_origin)
            {
               pBot->v_reach_point = v_zone_origin; // reach the safety zone
               FakeClientCommand (pBot->pEdict, RADIOMSG_COVERME); // bot speaks, "cover me!"
            }

            return (TRUE); // bot is concerned by this special zone
         }

         // else don't mind it if bot is camping
         else if (pBot->f_camp_time > *server.time)
            return (FALSE); // bot is not concerned by this special zone

         // else check if bot is a 'normal' counter-terrorist
         else if (GetTeam (pBot->pEdict) == CS_COUNTER_TERRORIST)
         {
            // cycle through all bot slots
            for (char i = 0; i < RACC_MAX_CLIENTS; i++)
            {
               // reliability check: is this slot unregistered
               if (bots[i].pEdict == NULL)
                  continue;

               // is this bot dead OR inactive OR self ?
               if (!players[i].is_alive || !bots[i].is_active || (bots[i].pEdict == pBot->pEdict))
                  continue;

               // is this one VIP AND visible AND not seeing safety zone
               // AND not been used for at least 20 seconds ?
               if (PlayerIsVIP (bots[i].pEdict)
                   && (bots[i].v_lastseenuser_position == g_vecZero)
                   && (bots[i].f_bot_use_time + 20.0 < *server.time)
                   && !BotCanSeeThis (&bots[i], v_zone_origin)
                   && (BotCanSeeOfEntity (pBot, bots[i].pEdict) != g_vecZero))
               {
                  bots[i].v_place_to_keep = g_vecZero; // reset any v_place_to_keep

                  // let's make him head off toward us...
                  BotSetIdealYaw (&bots[i], UTIL_VecToAngles (pBot->pEdict->v.origin - bots[i].pEdict->v.origin).y);
                  bots[i].f_reach_time = *server.time + 0.5; // make him ignore his reach points for a while

                  // set this bot's use time to now to avoid calling him twice
                  bots[i].f_bot_use_time = *server.time;

                  FakeClientCommand (bots[i].pEdict, RADIOMSG_AFFIRMATIVE); // make bot agree
                  bots[i].BotChat.bot_sayaudio |= BOT_SAYAUDIO_AFFIRMATIVE;
                  bots[i].BotChat.f_sayaudio_time = *server.time + RANDOM_FLOAT (0.5, 3.0);
                  return (FALSE); // normal CT is NOT directly concerned by this special zone
               }
            }

            return (FALSE); // normal CT can't see the VIP and is NOT concerned by this zone
         }
      }

      // check for a visible escape zone
      else if ((strcmp ("func_escapezone", STRING (pSpecialZone->v.classname)) == 0)
               && (BotCanSeeOfEntity (pBot, pSpecialZone) != g_vecZero))
      {
         v_zone_origin = VecBModelOrigin (pSpecialZone);

         // is bot a terrorist?
         if (GetTeam (pBot->pEdict) == CS_TERRORIST)
         {
            pBot->BotMove.f_forward_time = *server.time + 60.0; // let our bot go...
            pBot->f_camp_time = *server.time; // ...stop camping...
            pBot->BotMove.f_walk_time = 0.0; // ...and the sooner he gets there, the better it is !

            // is bot NOT already reaching the escape zone ?
            if (pBot->v_reach_point != v_zone_origin)
            {
               pBot->v_reach_point = v_zone_origin; // reach the escape zone
               FakeClientCommand (pBot->pEdict, RADIOMSG_FALLBACK); // bot speaks, "fallback team!"
            }

            // cycle through all bot slots to find teammates
            for (char i = 0; i < RACC_MAX_CLIENTS; i++)
            {
               // reliability check: is this slot unregistered
               if (bots[i].pEdict == NULL)
                  continue;

               // is this bot dead OR inactive OR self ?
               if (!players[i].is_alive || !bots[i].is_active || (bots[i].pEdict == pBot->pEdict))
                  continue;

               // is this one terrorist AND visible AND not seeing escape zone
               // AND not been used for at least 20 seconds ?
               if ((GetTeam (bots[i].pEdict) == CS_TERRORIST)
                   && (bots[i].v_lastseenuser_position == g_vecZero)
                   && (bots[i].f_bot_use_time + 20.0 < *server.time)
                   && !BotCanSeeThis (&bots[i], v_zone_origin)
                   && (BotCanSeeOfEntity (pBot, bots[i].pEdict) != g_vecZero))
               {
                  bots[i].v_place_to_keep = g_vecZero; // reset any v_place_to_keep

                  // let's make him head off toward us...
                  BotSetIdealYaw (&bots[i], UTIL_VecToAngles (pBot->pEdict->v.origin - bots[i].pEdict->v.origin).y);
                  bots[i].f_reach_time = *server.time + 0.5; // make him ignore his reach points for a while

                  // set this bot's use time to now to avoid calling him twice
                  bots[i].f_bot_use_time = *server.time;

                  FakeClientCommand (bots[i].pEdict, RADIOMSG_AFFIRMATIVE); // bot agrees

                  bots[i].BotChat.bot_sayaudio |= BOT_SAYAUDIO_AFFIRMATIVE;
                  bots[i].BotChat.f_sayaudio_time = *server.time + RANDOM_FLOAT (0.5, 3.0);
               }
            }

            return (TRUE); // bot is concerned by this special zone
         }
      }

      // check for a visible dropped bomb
      else if ((strcmp ("weaponbox", STRING (pSpecialZone->v.classname)) == 0)
               && (BotCanSeeThis (pBot, pSpecialZone->v.origin))
               && (IsInPlayerFOV (pBot->pEdict, pSpecialZone->v.origin))
               && (strcmp (STRING (pSpecialZone->v.model), "models/w_backpack.mdl") == 0)
               && ENT_IS_ON_FLOOR (pSpecialZone))
      {
         // both terrorists and counter-terrorists will head to the bomb. Because
         // counter-terrorists won't be able to pick it up, they will so
         // "cruise" around the bomb spot, permanently looking for enemies.

         v_zone_origin = pSpecialZone->v.origin;

         // is bot a terrorist ?
         if (GetTeam (pBot->pEdict) == CS_TERRORIST)
            return (BotReachPosition (pBot, v_zone_origin)); // if bot is a T, go pick it up

         // else don't mind it if bot is camping
         else if (pBot->f_camp_time > *server.time)
            return (FALSE); // bot is not concerned by this special zone

         // else bot must be a counter-terrorist, can he camp near here ?
         else if ((pBot->f_camp_time + 3.0 < *server.time) && BotHasPrimary (pBot)
                  && !((pBot->pEdict->v.weapons & (1 << CS_WEAPON_M3))
                       || (pBot->pEdict->v.weapons & (1 << CS_WEAPON_XM1014))))
         {
            if (BotCanCampNearHere (pBot, v_zone_origin)
                && (RANDOM_LONG (1, 100) <= (91 - 2 * player_count)))
               FakeClientCommand (pBot->pEdict, RADIOMSG_INPOSITION); // let the bot camp here
            return (FALSE); // normal CTs are NOT directly concerned by this special zone
         }
      }

      // check for a visible planted bomb
      else if ((strcmp ("grenade", STRING (pSpecialZone->v.classname)) == 0)
               && (BotCanSeeThis (pBot, pSpecialZone->v.origin))
               && (IsInPlayerFOV (pBot->pEdict, pSpecialZone->v.origin))
               && (strcmp (STRING (pSpecialZone->v.model), "models/w_c4.mdl") == 0)
               && ENT_IS_ON_FLOOR (pSpecialZone))
      {
         v_zone_origin = pSpecialZone->v.origin;

         // is bot a counter-terrorist ?
         if (GetTeam (pBot->pEdict) == CS_COUNTER_TERRORIST)
         {
            pBot->f_camp_time = *server.time; // stop camping, OMG, a bomb!!!
            BotDefuseBomb (pBot, pSpecialZone); // defuse bomb
            return (TRUE); // bot is concerned by this special zone
         }

         // else don't mind it if bot is camping
         else if (pBot->f_camp_time > *server.time)
            return (FALSE); // bot is not concerned by this special zone

         // else bot must be a terrorist, can he camp near here ?
         else if (pBot->f_camp_time + 10.0 < *server.time)
         {
            if (BotCanCampNearHere (pBot, v_zone_origin)
                && (RANDOM_LONG (1, 100) <= (91 - 2 * player_count)))
               FakeClientCommand (pBot->pEdict, RADIOMSG_INPOSITION); // let the bot camp here
            return (FALSE); // campers are NOT directly concerned by this special zone
         }
      }

      // check for a visible bomb target
      else if (((strcmp ("func_bomb_target", STRING (pSpecialZone->v.classname)) == 0)
                && (BotCanSeeOfEntity (pBot, pSpecialZone) != g_vecZero))
               || ((strcmp ("info_bomb_target", STRING (pSpecialZone->v.classname)) == 0)
                   && (BotCanSeeThis (pBot, pSpecialZone->v.origin))
                   && (IsInPlayerFOV (pBot->pEdict, pSpecialZone->v.origin))))
      {
         if (strncmp ("func_", STRING (pSpecialZone->v.classname), 5) == 0)
            v_zone_origin = VecBModelOrigin (pSpecialZone);
         else
            v_zone_origin = pSpecialZone->v.origin;

         // is bot a terrorist ?
         if (GetTeam (pBot->pEdict) == CS_TERRORIST)
         {
            // does the bot have the C4 ?
            if (pBot->pEdict->v.weapons & (1 << CS_WEAPON_C4))
            {
               BotPlantBomb (pBot, v_zone_origin); // plant bomb
               return (TRUE); // bot is concerned by this special zone
            }

            // if not, bot must be a 'normal' terrorist
            else
            {
               // cycle through all bot slots
               for (char i = 0; i < RACC_MAX_CLIENTS; i++)
               {
                  // reliability check: is this slot unregistered
                  if (bots[i].pEdict == NULL)
                     continue;

                  // is this bot dead OR inactive OR self ?
                  if (!players[i].is_alive || !bots[i].is_active || (bots[i].pEdict == pBot->pEdict))
                     continue;

                  // does this one have C4 AND visible AND not seeing bomb site
                  // AND not been used for at least 20 seconds ?
                  if ((bots[i].pEdict->v.weapons & (1 << CS_WEAPON_C4))
                      && (bots[i].v_lastseenuser_position == g_vecZero)
                      && (bots[i].f_bot_use_time + 20.0 < *server.time)
                      && !BotCanSeeThis (&bots[i], v_zone_origin)
                      && (BotCanSeeOfEntity (pBot, bots[i].pEdict) != g_vecZero))
                  {
                     bots[i].v_place_to_keep = g_vecZero; // reset any v_place_to_keep

                     // let's make him head off toward us...
                     BotSetIdealYaw (&bots[i], UTIL_VecToAngles (pBot->pEdict->v.origin - bots[i].pEdict->v.origin).y);
                     bots[i].f_reach_time = *server.time + 0.5; // make him ignore his reach points for a while

                     // set this bot's use time to now to avoid calling him twice
                     bots[i].f_bot_use_time = *server.time;

                     FakeClientCommand (bots[i].pEdict, RADIOMSG_YOUTAKETHEPOINT); // bot speaks, "you take the point!"
                     bots[i].BotChat.bot_sayaudio |= BOT_SAYAUDIO_AFFIRMATIVE;
                     bots[i].BotChat.f_sayaudio_time = *server.time + RANDOM_FLOAT (0.5, 3.0);
                  }
               }

               return (FALSE); // bot is NOT concerned by this special zone
            }
         }

         // else don't mind it if bot is camping
         else if (pBot->f_camp_time > *server.time)
            return (FALSE); // bot is not concerned by this special zone

         // else bot must be a counter-terrorist, if bomb not planted yet can the bot camp near here ?
         else if (!round.b_bomb_planted && (pBot->f_camp_time + 10.0 < *server.time)
                  && BotHasPrimary (pBot)
                  && !((pBot->pEdict->v.weapons & (1 << CS_WEAPON_M3))
                       || (pBot->pEdict->v.weapons & (1 << CS_WEAPON_XM1014))))
         {
            if (BotCanCampNearHere (pBot, v_zone_origin)
                && (RANDOM_LONG (1, 100) <= (91 - 2 * player_count)))
               FakeClientCommand (pBot->pEdict, RADIOMSG_INPOSITION); // let the bot camp here
            return (FALSE); // campers are NOT directly concerned by this special zone
         }
      }

      // check for a visible hostage rescue zone
      else if ((strcmp ("func_hostage_rescue", STRING (pSpecialZone->v.classname)) == 0)
               && (BotCanSeeOfEntity (pBot, pSpecialZone) != g_vecZero))
      {
         v_zone_origin = VecBModelOrigin (pSpecialZone);

         // if bot is a terrorist, has he not camped for at lease 10 seconds ?
         if ((GetTeam (pBot->pEdict) == CS_TERRORIST) && (pBot->f_camp_time + 10.0 < *server.time))
         {
            if (BotCanCampNearHere (pBot, v_zone_origin)
                && (RANDOM_LONG (1, 100) <= (91 - 2 * player_count)))
               FakeClientCommand (pBot->pEdict, RADIOMSG_INPOSITION); // let the bot camp here
            return (FALSE); // campers are NOT directly concerned by this special zone
         }
      }

      // check for a visible hostage
      else if ((strcmp ("hostage_entity", STRING (pSpecialZone->v.classname)) == 0)
               && (BotCanSeeThis (pBot, pSpecialZone->v.origin))
               && (IsInPlayerFOV (pBot->pEdict, pSpecialZone->v.origin)))
      {
         v_zone_origin = pSpecialZone->v.origin;

         // is bot a terrorist ?
         if (GetTeam (pBot->pEdict) == CS_TERRORIST)
         {
            // check if the hostage is moving AND bot has no enemy yet
            if ((pSpecialZone->v.velocity.Length2D () > 0) && FNullEnt (pBot->BotEnemy.pEdict))
            {
               pBot->BotEnemy.pEdict = pSpecialZone; // alert, this hostage flees away, shoot him down !
               pBot->BotEnemy.appearance_time = *server.time;
            }

            // else has the bot not camped for at least 10 seconds ?
            if (pBot->f_camp_time + 10.0 < *server.time)
               if (BotCanCampNearHere (pBot, v_zone_origin)
                   && (RANDOM_LONG (1, 100) <= (91 - 2 * player_count)))
                  FakeClientCommand (pBot->pEdict, RADIOMSG_INPOSITION); // let the bot camp here

            return (FALSE); // terrorists are NOT directly concerned by this special zone
         }

         // else bot must be a counter-terrorist
         else
         {
            // TODO: implement hostage usage
            return (FALSE);
         }
      }
   }

   return (FALSE); // bot found nothing interesting
}


bool BotCheckForGrenades (bot_t *pBot)
{
   edict_t *pGrenade = NULL;

   if (!IsValidPlayer (pBot->pEdict))
      return (FALSE); // reliability check

   if (DebugLevel.is_dontfindmode)
      return (FALSE); // don't process if botdontfind is set

   // is there an armed grenade near here?
   while ((pGrenade = UTIL_FindEntityInSphere (pGrenade, pBot->pEdict->v.origin, 300)) != NULL)
   {
      // check if entity is an armed grenade
      if ((strcmp ("grenade", STRING (pGrenade->v.classname)) == 0)
          && (BotCanSeeThis (pBot, pGrenade->v.origin))
          && (IsInPlayerFOV (pBot->pEdict, pGrenade->v.origin)))
      {
         // check if this grenade is NOT a smoke grenade neither the C4 (not to confuse w/ bomb)
         if ((strcmp (STRING (pGrenade->v.model), "models/w_smokegrenade.mdl") != 0)
             && (strcmp (STRING (pGrenade->v.model), "models/w_c4.mdl") != 0))
         {
            float grenade_angle = UTIL_VecToAngles (pGrenade->v.origin - pBot->pEdict->v.origin).y;
            BotSetIdealYaw (pBot, grenade_angle); // face the grenade...

            // ... and run away !!
            pBot->BotMove.f_backwards_time = *server.time + 0.5; // until the grenade explodes

            // is it a flashbang ?
            if (strcmp (STRING (pGrenade->v.model), "models/w_flashbang.mdl") == 0)
            {
               // strafe to (hopefully) take cover
               if (RANDOM_LONG (1, 100) < 50)
                  pBot->BotMove.f_strafeleft_time = *server.time + RANDOM_FLOAT (0.5, 2.0);
               else
                  pBot->BotMove.f_straferight_time = *server.time + RANDOM_FLOAT (0.5, 2.0);
            }

            // if this grenade is our enemies'
            if (GetTeam (pBot->pEdict) != GetTeam (pGrenade->v.owner))
            {
               pBot->BotChat.bot_sayaudio |= BOT_SAYAUDIO_SEEGRENADE; // bot says 'danger'
               pBot->BotChat.f_sayaudio_time = *server.time + RANDOM_FLOAT (0.7, 1.5);
            }

            return (TRUE); // bot is concerned by this grenade
         }
      }
   }

   return (FALSE); // bot found nothing interesting
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

   if (!IsValidPlayer (pBot->pEdict))
      return; // reliability check

   if (DebugLevel.is_dontfindmode)
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

         // check if entity is outside field of view
         if (!IsInPlayerFOV (pBot->pEdict, entity_origin))
            continue;  // skip this item if bot can't "see" it

         // check if entity is a ladder (ladders are a special case)
         if (strcmp ("func_ladder", STRING (pent->v.classname)) == 0)
         {
            // force ladder origin to same z coordinate as bot since
            // the VecBModelOrigin is the center of the ladder.  For
            // LONG ladders, the center MAY be hundreds of units above
            // the bot.  Fake an origin at the same level as the bot...

            entity_origin.z = pBot->pEdict->v.origin.z;

            // trace a line from bot's eyes to func_ladder entity...
            UTIL_TraceLine (GetGunPosition (pBot->pEdict), entity_origin, ignore_monsters, pBot->pEdict, &tr);

            // check if traced all the way up to the entity (didn't hit wall)
            if (tr.flFraction >= 1.0)
            {
               // always use the ladder if haven't used a ladder in at least 5 seconds...
               if (pBot->f_end_use_ladder_time + 5.0 < *server.time)
                  can_pickup = TRUE;
            }
         }
         else
         {
            // trace a line from bot's eyes to entity
            UTIL_TraceLine (GetGunPosition (pBot->pEdict), entity_origin, ignore_monsters, pBot->pEdict, &tr);
            
            // if traced all the way up to the entity (didn't hit wall)
            if (strcmp (STRING (pent->v.classname), STRING (tr.pHit->v.classname)) == 0)
            {
               // find distance to item for later use...
               float distance = (entity_origin - pBot->pEdict->v.origin).Length ();

               // check if entity is a breakable...
               if ((strcmp ("func_breakable", STRING (pent->v.classname)) == 0)
                   && (pent->v.takedamage != DAMAGE_NO) && (pent->v.health > 0)
                   && !(pent->v.flags & FL_WORLDBRUSH)
                   && (fabs (entity_origin.z - pBot->pEdict->v.origin.z) < 60))
               {
                  // check if close enough...
                  if (distance < 50)
                  {
                     if (pBot->current_weapon->hardware->id != CS_WEAPON_KNIFE)
                        FakeClientCommand (pBot->pEdict, "weapon_knife"); // select a proximity weapon
                     else
                     {
                        // point the weapon at the breakable and strike it
                        BotSetIdealAngles (pBot, UTIL_VecToAngles (entity_origin - GetGunPosition (pBot->pEdict)));
                        pBot->pEdict->v.button |= IN_ATTACK; // strike the breakable
                        pBot->f_reload_time = *server.time + RANDOM_LONG (1.5, 3.0); // set next time to reload
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

         // check if entity is outside field of view
         if (!IsInPlayerFOV (pBot->pEdict, entity_origin))
            continue;  // skip this item if bot can't "see" it

         // check if line of sight to object is not blocked (i.e. visible)
         if (BotCanSeeThis (pBot, entity_origin))
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

               if (!pBot->has_defuse_kit && (GetTeam (pBot->pEdict) == CS_COUNTER_TERRORIST))
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

   if (!FNullEnt (pPickupEntity))
   {
      pBot->b_is_picking_item = TRUE; // set bot picking item flag
      pBot->v_reach_point = pickup_origin; // save the location of item bot is trying to get
   }
   else
      pBot->b_is_picking_item = FALSE; // reset picking item flag
}


void BotPreThink (bot_t *pBot)
{
   // this is the first step of the bot Think() trilogy. In nature, every behaviour that can be
   // associated to intelligence is resulting of three invariable steps :
   // 1 - sensing the environment and the character's state
   // 2 - working out the changes to commit onto the environment and the character's state
   // 3 - performing these changes, and experiencing this action, looping back to step 1.
   // Here we deal with the first step, sensing, which is a correlation of three "input vectors"
   // (actually there are more of them but these three together are sufficient to be said
   // symptomatic of the human behaviour), respectively in order of importance the sight, the
   // hearing, and the touch feeling. Since FPS players experience this last one mainly by proxy,
   // the game simulating it by visual and auditive means, the touch feeling of the bot will be
   // a little more efficient than those of the players, giving them a slight advantage in this
   // particular domain. But since we're very far from emulating the two other vectors (vision
   // and hearing) as accurately as their human equivalents, it's just fair that way :)

   pBot->pEdict->v.flags |= FL_THIRDPARTYBOT; // set the third party bot flag

   // compute the bot's eye position and its aim vector
   pBot->BotAim.v_eyeposition = pBot->pEdict->v.origin + pBot->pEdict->v.view_ofs;
   pfnAngleVectors (pBot->pEdict->v.v_angle, (float *) &pBot->BotAim.v_forward, (float *) &pBot->BotAim.v_right, (float *) &pBot->BotAim.v_up);

   BotSee (pBot); // make bot see
   BotHear (pBot); // make bot hear
   BotTouch (pBot); // make bot touch

   pBot->pEdict->v.button = 0; // reset buttons pressed
   pBot->pEdict->v.impulse = 0; // reset impulse buttons pressed
   pBot->BotMove.f_move_speed = 0; // reset move_speed
   pBot->BotMove.f_strafe_speed = 0; // reset strafe_speed
   pBot->BotMove.b_emergency_walkback = FALSE;

   return;
}


void BotThink (bot_t *pBot)
{
   // this is the second step of the bot Think() trilogy. In nature, every behaviour that can be
   // associated to intelligence is resulting of three invariable steps :
   // 1 - sensing the environment and the character's state
   // 2 - working out the changes to commit onto the environment and the character's state
   // 3 - performing these changes, and experiencing this action, looping back to step 1.
   // Here we deal with the second step, thinking, where we involve all the cognitive stuff and
   // case-based reasoning that conditions the AI's behaviour.
   // Note: so far, this function is just a large bunch of if/else statments.

   // if the bot hasn't selected stuff to start the game yet, go do that...
   if ((pBot->b_not_started) || (pBot->BotEyes.BotHUD.menu_state != MENU_CS_IDLE))
   {
      BotStartGame (pBot);
      return;
   }

   // is the bot controlled by the player ?
   if (pBot->is_controlled)
   {
      BotIsBewitched (pBot); // OMG, this is witchery !
      return; // let the player steer this bot
   }

   // is it time for the bot to leave the game ? (depending on his time to live)
   if ((pBot->time_to_live > 0) && (pBot->time_to_live <= *server.time))
   {
      pBot->time_to_live = *server.time + 6.0; // don't say it twice (bad hack)
      pBot->BotMove.f_forward_time = 0.0; // stop the bot while he is leaving
      BotSetIdealAngles (pBot, pBot->pEdict->v.v_angle); // don't make it move its crosshair
      if (RANDOM_LONG (1, 100) <= (66 - 2 * player_count))
         pBot->BotChat.bot_saytext = BOT_SAYTEXT_BYE; // say goodbye
      return;
   }

   // if the bot is dead, wait for respawn...
   if (!IsAlive (pBot->pEdict))
   {
      // if not reset yet...
      if (pBot->BotEyes.sample_time > 0)
         BotReset (pBot); // reset our bot for next round

      // was the bot killed by another player AND has it not complained yet (on random) ?
      if (IsValidPlayer (pBot->pKillerEntity) && (pBot->BotChat.f_saytext_time < *server.time)
          && (RANDOM_LONG (1, 100) <= (56 - 2 * player_count)))
         pBot->BotChat.bot_saytext = BOT_SAYTEXT_WHINE;

      return;
   }

   // if the bot is stuck, try to unstuck it
   if (pBot->b_is_stuck && !IsFlying (pBot->pEdict))
      BotUnstuck (pBot); // try to unstuck our poor bot...

   // think (or continue thinking) of a path if necessary
   BotRunPathMachine (pBot);

   // should the bot complain of being alone for a long time ?
   if ((pBot->f_bot_alone_timer > 0) && (pBot->f_bot_alone_timer < *server.time))
   {
      pBot->f_bot_alone_timer = *server.time + RANDOM_LONG (30, 120); // sets new delay

      if (RANDOM_LONG (1, 100) <= (66 - 2 * player_count))
      {
         pBot->BotChat.bot_saytext = BOT_SAYTEXT_ALONE; // complain

         // once out of three times send a radio message
         if (RANDOM_LONG (1, 100) < 34)
            if (RANDOM_LONG (1, 100) < 50)
               FakeClientCommand (pBot->pEdict, RADIOMSG_SECTORCLEAR);
            else
               FakeClientCommand (pBot->pEdict, RADIOMSG_REPORTINGIN);
      }
   }

   // should the bot yell for backup ?
   if (!pBot->b_help_asked && !FNullEnt (pBot->BotEnemy.pEdict) && (pBot->pEdict->v.health <= 20))
      if (RANDOM_LONG (1, 100) <= (91 - 2 * player_count))
      {
         pBot->BotChat.bot_saytext = BOT_SAYTEXT_NEEDBACKUP; // yell
         FakeClientCommand (pBot->pEdict, RADIOMSG_NEEDBACKUP); // send a radio message
         pBot->b_help_asked = TRUE; // don't do it twice
      }

   // is the bot planting the bomb ?
   if (pBot->bot_task & BOT_TASK_PLANTING)
   {
      if (!pBot->b_can_plant)
         pBot->bot_task &= ~BOT_TASK_PLANTING; // stop planting if bomb icon doesn't blink anymore
      if (!FNullEnt (pBot->BotEnemy.pEdict = BotCheckForEnemies (pBot)))
         pBot->bot_task &= ~BOT_TASK_PLANTING; // stop planting if enemy found
      if (round.b_bomb_planted)
      {
         pBot->bot_task &= ~BOT_TASK_PLANTING; // finished planting when "bomb planted" message received
         if (BotHasPrimary (pBot)
             && !((pBot->pEdict->v.weapons & (1 << CS_WEAPON_M3))
                  || (pBot->pEdict->v.weapons & (1 << CS_WEAPON_XM1014))))
            if (BotCanCampNearHere (pBot, pBot->pEdict->v.origin)
                && (RANDOM_LONG (1, 100) <= (91 - 2 * player_count)))
               FakeClientCommand (pBot->pEdict, RADIOMSG_INPOSITION); // let the bot camp here
      }
      else if (pBot->current_weapon->hardware->id != CS_WEAPON_C4)
         FakeClientCommand (pBot->pEdict, "weapon_c4"); // take the C4
      pBot->f_pause_time = *server.time + 0.5; // pause the bot
      BotSetIdealAngles (pBot, WrapAngles (Vector (-45 / pBot->pProfile->skill, pBot->pEdict->v.v_angle.y, 0))); // look down at 45 degree angle
      if (pBot->pProfile->skill > 2)
         pBot->BotMove.f_duck_time = *server.time + 0.5; // if bot is skilled enough, duck
      pBot->pEdict->v.button |= IN_ATTACK; // plant the bomb
      return;
   }

   // else is the bot defusing bomb ?
   else if (pBot->bot_task & BOT_TASK_DEFUSING)
   {
      if (!round.b_bomb_planted)
         pBot->bot_task &= ~BOT_TASK_DEFUSING; // finished defusing when "bomb defused" message received
      Vector bot_angles = UTIL_VecToAngles (pBot->v_goal - GetGunPosition (pBot->pEdict));
      BotSetIdealAngles (pBot, Vector (bot_angles.x / 2 + bot_angles.x / (2 * pBot->pProfile->skill), bot_angles.y, bot_angles.z)); // look at bomb
      pBot->f_pause_time = *server.time + 0.5; // pause the bot
      if (pBot->v_goal.z < pBot->pEdict->v.origin.z)
         pBot->BotMove.f_duck_time = *server.time + 0.5; // if bomb is under the bot, let the bot duck
      if (pBot->BotEyes.BotHUD.has_progress_bar || (RANDOM_LONG (1, 100) < 95))
         pBot->pEdict->v.button |= IN_USE; // keep pressing the button once the progress bar appeared
      return;
   }

   // has the bot been ordered something ?
   if ((pBot->BotEars.bot_order != BOT_ORDER_NOORDER) && (pBot->BotEars.f_order_time + 1.0 < *server.time))
      BotAnswerToOrder (pBot); // answer to this order

   // is the bot alive and should the bot buy stuff now ?
   if ((pBot->pEdict->v.health > 0) && (pBot->pEdict->v.deadflag == DEAD_NO)
       && (pBot->f_buy_time > 0) && (pBot->f_buy_time < *server.time))
   {
      BotBuyStuff (pBot); // buy stuff
      return;
   }

   // is the bot blinded (e.g. affected by a flashbang) ?
   if (pBot->BotEyes.blinded_time > *server.time)
   {
      pBot->BotMove.f_duck_time = *server.time + RANDOM_FLOAT (0.5, 1.0); // duck when blinded

      // pick up a random strafe direction
      if (RANDOM_LONG (1, 100) < 50)
         pBot->BotMove.f_strafeleft_time = *server.time + 0.1;
      else
         pBot->BotMove.f_straferight_time = *server.time + 0.1;

      if (RANDOM_LONG (0, 100) < 50)
         pBot->BotMove.b_emergency_walkback = TRUE;

      return;
   }

   // see how much we have turned
   if (fabs (pBot->BotAim.v_turn_speed.y) < 2.0)
      pBot->b_is_walking_straight = TRUE;
   else
      pBot->b_is_walking_straight = FALSE;

   if ((fabs (pBot->BotAim.v_turn_speed.x) > 10) || (fabs (pBot->BotAim.v_turn_speed.y) > 10))
      pBot->BotMove.f_walk_time = *server.time + 0.2; // slow down if turning a lot

   // let's look for enemies...
   pBot->BotEnemy.pEdict = BotCheckForEnemies (pBot);

   // avoid walls, corners and teammates
   if (pBot->f_avoid_time < *server.time)
      BotAvoidObstacles (pBot);

   // are there armed grenades near us ?
   if (BotCheckForGrenades (pBot))
      pBot->BotMove.b_emergency_walkback = TRUE;

   // does an enemy exist ?
   if (!FNullEnt (pBot->BotEnemy.pEdict))
   {
      pBot->LastSeenEnemy = pBot->BotEnemy; // remember the last seen enemy
      pBot->f_pause_time = 0; // dont't pause
      BotShootAtEnemy (pBot); // shoot at the enemy

      return; // the bot has something to do
   }

   // else has the enemy suddently disappeared ?
   else if (!FNullEnt (pBot->LastSeenEnemy.pEdict))
   {
      // did the enemy just went out of FOV ?
      if (!IsInPlayerFOV (pBot->pEdict, pBot->LastSeenEnemy.pEdict->v.origin)
          && BotCanSeeThis (pBot, pBot->LastSeenEnemy.pEdict->v.origin))
      {
         // OMG, this enemy is circle-strafing us out !!!
         BotSetIdealAngles (pBot, UTIL_VecToAngles (pBot->LastSeenEnemy.pEdict->v.origin - GetGunPosition (pBot->pEdict)));
         pBot->BotMove.b_emergency_walkback = TRUE; // walk back to get the enemy back in field
      }

      // else has the enemy just gone hiding ?
      else if (IsInPlayerFOV (pBot->pEdict, pBot->LastSeenEnemy.pEdict->v.origin)
               && !BotCanSeeThis (pBot, pBot->LastSeenEnemy.pEdict->v.origin))
      {
         pBot->LastSeenEnemy.is_hiding = TRUE; // bot remembers this enemy is hiding

         //BotShootAtHiddenEnemy (pBot); // shoot at the hidden enemy

         // if bot is waiting for enemy to strike back, don't move
         if (pBot->f_pause_time > *server.time)
         {
            pBot->BotMove.f_forward_time = 0; // don't move while pausing
            pBot->LastSeenEnemy.disappearance_time = *server.time; // set lost enemy time to now
         }

         // else rush after that coward one
         else if ((pBot->LastSeenEnemy.v_targetpoint - pBot->pEdict->v.origin).Length () > 50)
         {
            // if bot is unable to chase it, then just wander around
            if (!BotReachPosition (pBot, pBot->LastSeenEnemy.v_targetpoint))
               memset (&pBot->LastSeenEnemy, 0, sizeof (pBot->LastSeenEnemy)); // here we are, seems that bot really lost enemy
         }

         else
            memset (&pBot->LastSeenEnemy, 0, sizeof (pBot->LastSeenEnemy)); // here we are, seems that bot really lost enemy
      }

      return; // the bot has something to do
   }

   // else look for special zones
   else if (BotCheckForSpecialZones (pBot))
   {
      // is bot about to hit something it can jump up ?
      if ((pBot->BotBody.hit_state & OBSTACLE_FRONT_LOWWALL) && (pBot->BotMove.f_jump_time + 2.0 < *server.time))
         pBot->BotMove.f_jump_time = *server.time; // jump up and move forward

      // else is it about to hit something it can duck under ?
      else if (pBot->BotBody.hit_state & OBSTACLE_FRONT_LOWCEILING)
         pBot->BotMove.f_duck_time = *server.time + RANDOM_FLOAT (0.5, 1.5); // duck & go

      // if bot is about to fall...
      if (pBot->f_fallcheck_time < *server.time)
      {
         if (pBot->BotBody.hit_state & OBSTACLE_FRONT_FALL)
            BotTurnAtFall (pBot); // try to avoid falling
      }

      return;
   }

   // is bot keeping a place ?
   if (pBot->v_place_to_keep != g_vecZero)
      BotStayInPosition (pBot);

   // else is bot being "used" ?
   else if (pBot->pBotUser != NULL)
      BotFollowUser (pBot);

   // else may the bot spray a logo (don't spray if bot has an enemy) ?
   else if ((pBot->f_spraying_logo_time > *server.time) && FNullEnt (pBot->BotEnemy.pEdict))
   {
      pBot->BotMove.f_forward_time = 0; // don't move
      BotSetIdealPitch (pBot, -50); // look down at 45 degree angle
      pBot->f_reach_time = *server.time + 0.5; // don't reach point for half a second

      // is the bot finally looking down enough to spray its logo ?
      if (!pBot->b_logo_sprayed && (pBot->pEdict->v.v_angle.x > 45))
      {
         pBot->pEdict->v.impulse = 201; // spray logo when finished looking down
         pBot->BotMove.f_backwards_time = *server.time + RANDOM_FLOAT (0.5, 1.0); // move back
         pBot->b_logo_sprayed = TRUE; // remember this is done
         pBot->BotEyes.sample_time = *server.time; // open eyes again

         return;
      }
   }

   // else if nothing special to do...
   else
   {
      if (pBot->f_find_item_time < *server.time)
         BotCheckForItems (pBot); // if time to, see if there are any visible items
      else
         pBot->b_is_picking_item = FALSE;

      if (pBot->f_find_goal_time < *server.time)
         BotFindGoal (pBot); // if time to, find a new goal

      BotWander (pBot); // then just wander around
   }

   if (pBot->f_pause_time > *server.time) // is the bot "paused"?
      pBot->BotMove.f_forward_time = 0; // don't move while pausing

   return;
}


void BotPostThink (bot_t *pBot)
{
   // this is the last step of the bot Think() trilogy. In nature, every behaviour that can be
   // associated to intelligence is resulting of three invariable steps :
   // 1 - sensing the environment and the character's state
   // 2 - working out the changes to commit onto the environment and the character's state
   // 3 - performing these changes, and experiencing this action, looping back to step 1.
   // Here we deal with the third step, action. This is what I call the 'motile' part of the AI
   // (by opposition to the 'sensitive' part which is the first step of the trilogy). Motile
   // intelligence is the result of three "output vectors" - as formerly, more than three in fact,
   // but these three can be said to be representative of the human behaviour. These are: the
   // ability to communicate (chat), the ability to walk standing (move) and the ability to use
   // the hands as tools (point gun). We do all these actions here, provided the AI character
   // is "alive" in the game ; if not, it has, like a player, only the ability to chat. Once all
   // these three sub-steps of the motile part of the thinking cycle have been made, all we have
   // to do is to ask the engine to move the bot's player entity until the next frame, according
   // to our computations. This is done by calling the engine function RunPlayerMove().

   // handle bot speaking stuff
   BotChat (pBot);

   // is the bot alive in the game ?
   if (IsAlive (pBot->pEdict))
   {
      // handle bot moving stuff
      if (round.f_start_time < *server.time)
         BotMove (pBot); // don't allow bots to move when freeze time is not elapsed yet
      BotPointGun (pBot);

      if (pBot->pEdict->v.velocity.z > MAX_SAFEFALL_SPEED)
         pBot->BotBody.fall_time = *server.time; // save bot fall time
      pBot->v_prev_position = pBot->pEdict->v.origin; // save previous position (for checking if stuck)
   }

   // ask the engine to do the fakeclient movement on server
   pfnRunPlayerMove (pBot->pEdict, WrapAngles (pBot->pEdict->v.v_angle), pBot->BotMove.f_move_speed, pBot->BotMove.f_strafe_speed, 0, pBot->pEdict->v.button, pBot->pEdict->v.impulse, server.msecval);

   // if this fakeclient has an illumination entity (RedFox's engine bug fix, thanks m8t :))
   if (!FNullEnt (pBot->pIllumination))
   {
      SET_ORIGIN (pBot->pIllumination, pBot->pEdict->v.origin); // make his light entity follow him
      if (pBot->pIllumination->v.nextthink + 0.1 < *server.time)
         pBot->pIllumination->v.nextthink = *server.time + 0.2; // make it think at 3 Hertz
   }

   return; // finished bot's motile part of the thinking cycle
}


bool BotItemIsInteresting (bot_t *pBot, edict_t *pItem)
{
   if (!IsValidPlayer (pBot->pEdict) || (pItem == NULL))
      return (FALSE); // reliability check

   // if bot has no primary weapon or little ammo or no ammo left and this is a primary weapon...
   if ((!BotHasPrimary (pBot)
        || (BotHoldsPrimary (pBot) && (*pBot->current_weapon->primary_ammo < 8))
        || (BotHasPrimary (pBot) && BotHoldsSecondary (pBot)))
       && ItemIsPrimary (pItem))
      return (TRUE); // this item is really interesting

   // if bot has no secondary weapon or little ammo and this is a secondary weapon...
   if ((!BotHasSecondary (pBot)
        || (BotHoldsSecondary (pBot) && (*pBot->current_weapon->primary_ammo < 8)))
       && ItemIsSecondary (pItem))
      return (TRUE); // this item is really interesting

   return (FALSE); // all other stuff may not be interesting
}


void BotDiscardItem (bot_t *pBot, edict_t *pItem)
{
   if (!IsValidPlayer (pBot->pEdict) || (pItem == NULL))
      return; // reliability check

   // if bot is wanting to pick up a primary weapon and needs to discard one to do so...
   if (ItemIsPrimary (pItem) && BotHasPrimary (pBot))
   {
      // if the bot is not currently holding his primary weapon, select it
      if (!BotHoldsPrimary (pBot))
      {
         if (pBot->pEdict->v.weapons & (1 << CS_WEAPON_AK47))
            FakeClientCommand (pBot->pEdict, "weapon_ak47");
         else if (pBot->pEdict->v.weapons & (1 << CS_WEAPON_AUG))
            FakeClientCommand (pBot->pEdict, "weapon_aug");
         else if (pBot->pEdict->v.weapons & (1 << CS_WEAPON_AWP))
            FakeClientCommand (pBot->pEdict, "weapon_awp");
         else if (pBot->pEdict->v.weapons & (1 << CS_WEAPON_G3SG1))
            FakeClientCommand (pBot->pEdict, "weapon_g3sg1");
         else if (pBot->pEdict->v.weapons & (1 << CS_WEAPON_M249))
            FakeClientCommand (pBot->pEdict, "weapon_m249");
         else if (pBot->pEdict->v.weapons & (1 << CS_WEAPON_M3))
            FakeClientCommand (pBot->pEdict, "weapon_m3");
         else if (pBot->pEdict->v.weapons & (1 << CS_WEAPON_M4A1))
            FakeClientCommand (pBot->pEdict, "weapon_m4a1");
         else if (pBot->pEdict->v.weapons & (1 << CS_WEAPON_MAC10))
            FakeClientCommand (pBot->pEdict, "weapon_mac10");
         else if (pBot->pEdict->v.weapons & (1 << CS_WEAPON_MP5NAVY))
            FakeClientCommand (pBot->pEdict, "weapon_mp5navy");
         else if (pBot->pEdict->v.weapons & (1 << CS_WEAPON_P228))
            FakeClientCommand (pBot->pEdict, "weapon_p228");
         else if (pBot->pEdict->v.weapons & (1 << CS_WEAPON_P90))
            FakeClientCommand (pBot->pEdict, "weapon_p90");
         else if (pBot->pEdict->v.weapons & (1 << CS_WEAPON_SCOUT))
            FakeClientCommand (pBot->pEdict, "weapon_scout");
         else if (pBot->pEdict->v.weapons & (1 << CS_WEAPON_SG550))
            FakeClientCommand (pBot->pEdict, "weapon_sg550");
         else if (pBot->pEdict->v.weapons & (1 << CS_WEAPON_SG552))
            FakeClientCommand (pBot->pEdict, "weapon_sg552");
         else if (pBot->pEdict->v.weapons & (1 << CS_WEAPON_TMP))
            FakeClientCommand (pBot->pEdict, "weapon_tmp");
         else if (pBot->pEdict->v.weapons & (1 << CS_WEAPON_UMP45))
            FakeClientCommand (pBot->pEdict, "weapon_ump45");
         else if (pBot->pEdict->v.weapons & (1 << CS_WEAPON_XM1014))
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
         if (pBot->pEdict->v.weapons & (1 << CS_WEAPON_DEAGLE))
            FakeClientCommand (pBot->pEdict, "weapon_deagle");
         else if (pBot->pEdict->v.weapons & (1 << CS_WEAPON_ELITE))
            FakeClientCommand (pBot->pEdict, "weapon_elite");
         else if (pBot->pEdict->v.weapons & (1 << CS_WEAPON_FIVESEVEN))
            FakeClientCommand (pBot->pEdict, "weapon_fiveseven");
         else if (pBot->pEdict->v.weapons & (1 << CS_WEAPON_GLOCK18))
            FakeClientCommand (pBot->pEdict, "weapon_glock18");
         else if (pBot->pEdict->v.weapons & (1 << CS_WEAPON_P228))
            FakeClientCommand (pBot->pEdict, "weapon_p228");
         else if (pBot->pEdict->v.weapons & (1 << CS_WEAPON_USP))
            FakeClientCommand (pBot->pEdict, "weapon_usp");
      }
      else
         FakeClientCommand (pBot->pEdict, "drop"); // discard secondary weapon
   }

   pBot->f_find_item_time = *server.time + 3.0; // delay looking for items
}


void FakeClientCommand (bot_t *pBot, char *radiocmd, char *radiomsg)
{
   if (!IsValidPlayer (pBot->pEdict))
      return; // reliability check

      // browse down Counter-Strike specific radio menu selections
   FakeClientCommand (pBot->pEdict, radiocmd);
   FakeClientCommand (pBot->pEdict, radiomsg);
}


void BotFindGoal (bot_t *pBot)
{
   edict_t *pEntity = NULL;
   float distance, min_distance;
   bool b_found_goal;

   if (!IsValidPlayer (pBot->pEdict))
      return; // reliability check

   if (DebugLevel.is_dontfindmode)
      return; // don't process if botdontfind is set

   if ((pBot->v_goal != g_vecZero) && ((pBot->v_goal - pBot->pEdict->v.origin).Length () > 300))
   {
      pBot->f_find_goal_time = *server.time + RANDOM_FLOAT (1.0, 5.0); // set next lookout time
      return; // bot already has a goal to reach
   }

   // check if bot is VIP
   if (PlayerIsVIP (pBot->pEdict))
   {
      min_distance = 9999.0; // reset min_distance
      b_found_goal = FALSE; // reset found goal flag

      // loop through all VIP safety zones
      while ((pEntity = UTIL_FindEntityByString (pEntity, "classname", "func_vip_safetyzone")) != NULL)
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
      while ((pEntity = UTIL_FindEntityByString (pEntity, "classname", "func_escapezone")) != NULL)
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

      // loop through all bomb zones
      while ((pEntity = UTIL_FindEntityByString (pEntity, "classname", "func_bomb_target")) != NULL)
      {
         distance = (VecBModelOrigin (pEntity) - pBot->pEdict->v.origin).Length ();

         // randomly choose the nearest goal if bomb not planted yet
         if ((distance < min_distance) && ((RANDOM_LONG (1, 100) < 66) || !b_found_goal) && !round.b_bomb_planted)
         {
            b_found_goal = TRUE; // bot found a goal
            min_distance = distance; // update the minimum distance
            pBot->v_goal = VecBModelOrigin (pEntity); // remember this entity
         }
      }

      // loop through all bomb spots
      while ((pEntity = UTIL_FindEntityByString (pEntity, "classname", "info_bomb_target")) != NULL)
      {
         distance = (pEntity->v.origin - pBot->pEdict->v.origin).Length ();

         // randomly choose the nearest goal if bomb not planted yet
         if ((distance < min_distance) && ((RANDOM_LONG (1, 100) < 66) || !b_found_goal) && !round.b_bomb_planted)
         {
            b_found_goal = TRUE; // bot found a goal
            min_distance = distance; // update the minimum distance
            pBot->v_goal = pEntity->v.origin; // remember this entity
         }
      }
   }

   // else check if bot is counter-terrorist
   else if (GetTeam (pBot->pEdict) == CS_COUNTER_TERRORIST)
   {
      min_distance = 9999.0; // reset min_distance
      b_found_goal = FALSE; // reset found goal flag

      // loop through all escape zones
      while ((pEntity = UTIL_FindEntityByString (pEntity, "classname", "func_escapezone")) != NULL)
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
      while ((pEntity = UTIL_FindEntityByString (pEntity, "classname", "hostage_entity")) != NULL)
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
      while ((pEntity = UTIL_FindEntityByString (pEntity, "classname", "func_hostage_rescue")) != NULL)
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

      // loop through all bomb zones
      while ((pEntity = UTIL_FindEntityByString (pEntity, "classname", "func_bomb_target")) != NULL)
      {
         distance = (VecBModelOrigin (pEntity) - pBot->pEdict->v.origin).Length ();

         // randomly choose the nearest goal if bomb is already planted
         if ((distance < min_distance) && ((RANDOM_LONG (1, 100) < 66) || !b_found_goal) && round.b_bomb_planted)
         {
            b_found_goal = TRUE; // bot found a goal
            min_distance = distance; // update the minimum distance
            pBot->v_goal = VecBModelOrigin (pEntity); // remember this entity
         }
      }

      // loop through all bomb spots
      while ((pEntity = UTIL_FindEntityByString (pEntity, "classname", "info_bomb_target")) != NULL)
      {
         distance = (pEntity->v.origin - pBot->pEdict->v.origin).Length ();

         // randomly choose the nearest goal if bomb not planted yet
         if ((distance < min_distance) && ((RANDOM_LONG (1, 100) < 66) || !b_found_goal) && !round.b_bomb_planted)
         {
            b_found_goal = TRUE; // bot found a goal
            min_distance = distance; // update the minimum distance
            pBot->v_goal = pEntity->v.origin; // remember this entity
         }
      }
   }

   pBot->f_find_goal_time = *server.time + 10.0; // next goal lookout in 10 seconds
   return;
}


void BotAnswerToOrder (bot_t *pBot)
{
   if (!IsValidPlayer (pBot->pEdict) || FNullEnt (pBot->BotEars.pAskingEntity))
      return; // reliability check

   // has the bot been asked to follow someone ?
   if (pBot->BotEars.bot_order == BOT_ORDER_FOLLOW)
   {
      // does the bot want to follow the caller ?
      if (FNullEnt (pBot->BotEnemy.pEdict)
          && ((RANDOM_LONG (1, 100) < 80) && pBot->b_is_fearful)
              || ((RANDOM_LONG (1, 100) < 40) && !pBot->b_is_fearful))
      {
         pBot->v_place_to_keep = g_vecZero; // don't stay in position anymore
         pBot->pBotUser = pBot->BotEars.pAskingEntity; // mark this client as using the bot
         pBot->v_lastseenuser_position = pBot->BotEars.pAskingEntity->v.origin; // remember last seen user position
         pBot->BotChat.bot_saytext = BOT_SAYTEXT_FOLLOWOK; // bot acknowledges
         FakeClientCommand (pBot->pEdict, RADIOMSG_AFFIRMATIVE); // send a radio message
      }
      else
      {
         pBot->BotChat.bot_saytext = BOT_SAYTEXT_NEGATIVE; // bot refuses
         FakeClientCommand (pBot->pEdict, RADIOMSG_NEGATIVE); // send a radio message
      }
   }

   // else has the bot been asked to check in ?
   else if (pBot->BotEars.bot_order == BOT_ORDER_REPORT)
   {
      // does the bot want to answer the caller ?
      if (RANDOM_LONG (1, 100) < 66)
      {
         // does the bot have no enemy ?
         if (FNullEnt (pBot->BotEnemy.pEdict))
         {
            pBot->BotChat.bot_sayaudio |= BOT_SAYAUDIO_REPORTING; // set him for reporting
            pBot->BotChat.f_sayaudio_time = *server.time + RANDOM_FLOAT (1.0, 2.0);
            if (RANDOM_LONG (1, 100) < 50)
               FakeClientCommand (pBot->pEdict, RADIOMSG_SECTORCLEAR); // send "sector clear" radio message
            else
               FakeClientCommand (pBot->pEdict, RADIOMSG_REPORTINGIN); // send "checking in" radio message
         }
         else
         {
            pBot->BotChat.bot_sayaudio |= BOT_SAYAUDIO_ATTACKING; // bot yells attack (audio)
            pBot->BotChat.f_sayaudio_time = *server.time + RANDOM_FLOAT (1.0, 2.0);
            FakeClientCommand (pBot->pEdict, RADIOMSG_UNDERFIRE); // send "under fire" radio message
         }
      }
   }

   // else has the bot been asked to keep a position ?
   else if (pBot->BotEars.bot_order == BOT_ORDER_STAY)
   {
      // does the bot wants to obey the caller ?
      if (FNullEnt (pBot->BotEnemy.pEdict)
          && ((RANDOM_LONG (1, 100) < 80) && pBot->b_is_fearful)
              || ((RANDOM_LONG (1, 100) < 40) && !pBot->b_is_fearful))
      {
         pBot->v_place_to_keep = pBot->pEdict->v.origin; // position to stay in
         pBot->f_place_time = *server.time; // remember when we last saw the place to keep
         pBot->pBotUser = NULL; // free the user client slot
         pBot->v_lastseenuser_position = g_vecZero; // forget last seen user position
         pBot->BotChat.bot_saytext = BOT_SAYTEXT_HOLDPOSITIONOK; // bot acknowledges
         FakeClientCommand (pBot->pEdict, RADIOMSG_INPOSITION); // send a radio message
      }
      else
      {
         pBot->BotChat.bot_saytext = BOT_SAYTEXT_NEGATIVE; // bot refuses
         FakeClientCommand (pBot->pEdict, RADIOMSG_NEGATIVE); // send a radio message
      }
   }

   // else has the bot been asked to rush on his own ?
   else if (pBot->BotEars.bot_order == BOT_ORDER_GO)
   {
      pBot->v_place_to_keep = g_vecZero; // don't stay in position anymore
      pBot->pBotUser = NULL; // free the user client slot
      pBot->v_lastseenuser_position = g_vecZero; // forget last seen user position
      if (!pBot->b_is_fearful)
         pBot->f_rush_time = *server.time + RANDOM_FLOAT (15.0, 45.0); // rush if not fearful

      // does the bot want to answer the caller ?
      if (RANDOM_LONG (1, 100) < 50)
         if (RANDOM_LONG (1, 100) < 66)
            pBot->BotChat.bot_saytext = BOT_SAYTEXT_AFFIRMATIVE; // bot acknowledges
         else
            FakeClientCommand (pBot->pEdict, RADIOMSG_AFFIRMATIVE); // send a radio message
   }

   pBot->BotEars.bot_order = BOT_ORDER_NOORDER; // reset bot order field
   return;
}


void BotReactToSound (bot_t *pBot, noise_t *sound)
{
   if (!IsValidPlayer (pBot->pEdict))
      return; // reliability check

   // is it a C4 close to explode sound ?
   if (strcmp ("weapons/c4_beep4", sound->file_path) == 0)
   {
      // was the bot camping ?
      if (pBot->f_camp_time > *server.time)
      {
         pBot->v_place_to_keep = g_vecZero; // forget our camp spot...
         pBot->f_camp_time = *server.time; // ...get up...
         pBot->f_rush_time = *server.time + 60.0; // ...and run away !!
         if (RANDOM_LONG (1, 100) <= (91 - 2 * player_count))
            FakeClientCommand (pBot->pEdict, RADIOMSG_FALLBACK); // bot says, "fallback team !"
      }
   }

   // else is it a player movement sound ?
   else if (strncmp ("player/pl_", sound->file_path, 10) == 0)
   {
      // are there teammates around ?
      //if (bot is in squad OR teammates around)
         // don't process this sound

      // given the direction the bot thinks the sound is, let the bot have a look there
      if (sound->direction & DIRECTION_LEFT)
         BotAddIdealYaw (pBot, RANDOM_FLOAT (45, 135));
      else if (sound->direction & DIRECTION_RIGHT)
         BotAddIdealYaw (pBot, -RANDOM_FLOAT (45, 135));
      else if (sound->direction & DIRECTION_BACK)
         BotAddIdealYaw (pBot, RANDOM_FLOAT (135, 225));
   }

         /*if (strcmp ((char *) p, "#Follow_me") == 0) // 'Follow Me' radio command
         {
            // check if bot can see the caller
            if (BotGetIdealAimVector (&bots[bot_index], players[sender_index - 1].pEntity)) != g_vecZero)
            {
               bots[bot_index].BotEars.bot_order = BOT_ORDER_FOLLOW; // let the bot know he has been ordered something
               bots[bot_index].BotEars.pAskingEntity = players[sender_index - 1].pEntity; // remember asker
               bots[bot_index].BotEars.f_order_time = *server.time; // remember when the order came
            }
         }
         else if (strcmp ((char *) p, "#Hold_this_position") == 0) // 'Hold This Position' radio command
         {
            // check if bot can see the caller
            if (BotGetIdealAimVector (&bots[bot_index], players[sender_index - 1].pEntity) != g_vecZero)
            {
               bots[bot_index].BotEars.bot_order = BOT_ORDER_STAY; // let the bot know he has been ordered something
               bots[bot_index].BotEars.pAskingEntity = players[sender_index - 1].pEntity; // remember asker
               bots[bot_index].BotEars.f_order_time = *server.time; // remember when the order came
            }
         }
         else if ((strcmp ((char *) p, "#Go_go_go") == 0) // 'Go Go Go' radio command
                  || (strcmp ((char *) p, "#Storm_the_front") == 0)) // 'Storm The Front' radio command
         {
            bots[bot_index].BotEars.bot_order = BOT_ORDER_GO; // let the bot know he has been ordered something
            bots[bot_index].BotEars.pAskingEntity = players[sender_index - 1].pEntity; // remember asker
            bots[bot_index].BotEars.f_order_time = *server.time; // remember when the order came
         }
         else if (strcmp ((char *) p, "#Report_in_team") == 0) // 'Report In' radio command
         {
            bots[bot_index].BotEars.bot_order = BOT_ORDER_REPORT; // let the bot know he has been ordered something
            bots[bot_index].BotEars.pAskingEntity = players[sender_index - 1].pEntity; // remember asker
            bots[bot_index].BotEars.f_order_time = *server.time; // remember when the order came
         }*/

   return; // finished
}


bool BotHasPrimary (bot_t *pBot)
{
   return ((pBot->pEdict->v.weapons & ((1 << CS_WEAPON_AK47)
                                       | (1 << CS_WEAPON_AUG)
                                       | (1 << CS_WEAPON_AWP)
                                       | (1 << CS_WEAPON_G3SG1)
                                       | (1 << CS_WEAPON_M249)
                                       | (1 << CS_WEAPON_M3)
                                       | (1 << CS_WEAPON_M4A1)
                                       | (1 << CS_WEAPON_MAC10)
                                       | (1 << CS_WEAPON_MP5NAVY)
                                       | (1 << CS_WEAPON_P228)
                                       | (1 << CS_WEAPON_P90)
                                       | (1 << CS_WEAPON_SCOUT)
                                       | (1 << CS_WEAPON_SG550)
                                       | (1 << CS_WEAPON_SG552)
                                       | (1 << CS_WEAPON_TMP)
                                       | (1 << CS_WEAPON_UMP45)
                                       | (1 << CS_WEAPON_XM1014))) != 0);
}


bool BotHasSecondary (bot_t *pBot)
{
   return ((pBot->pEdict->v.weapons & ((1 << CS_WEAPON_DEAGLE)
                                       | (1 << CS_WEAPON_ELITE)
                                       | (1 << CS_WEAPON_FIVESEVEN)
                                       | (1 << CS_WEAPON_GLOCK18)
                                       | (1 << CS_WEAPON_P228)
                                       | (1 << CS_WEAPON_USP))) != 0);
}


bool BotHoldsPrimary (bot_t *pBot)
{
   return ((pBot->current_weapon->hardware->id == CS_WEAPON_AK47)
           || (pBot->current_weapon->hardware->id == CS_WEAPON_AUG)
           || (pBot->current_weapon->hardware->id == CS_WEAPON_AWP)
           || (pBot->current_weapon->hardware->id == CS_WEAPON_G3SG1)
           || (pBot->current_weapon->hardware->id == CS_WEAPON_M249)
           || (pBot->current_weapon->hardware->id == CS_WEAPON_M3)
           || (pBot->current_weapon->hardware->id == CS_WEAPON_M4A1)
           || (pBot->current_weapon->hardware->id == CS_WEAPON_MAC10)
           || (pBot->current_weapon->hardware->id == CS_WEAPON_MP5NAVY)
           || (pBot->current_weapon->hardware->id == CS_WEAPON_P228)
           || (pBot->current_weapon->hardware->id == CS_WEAPON_P90)
           || (pBot->current_weapon->hardware->id == CS_WEAPON_SCOUT)
           || (pBot->current_weapon->hardware->id == CS_WEAPON_SG550)
           || (pBot->current_weapon->hardware->id == CS_WEAPON_SG552)
           || (pBot->current_weapon->hardware->id == CS_WEAPON_TMP)
           || (pBot->current_weapon->hardware->id == CS_WEAPON_UMP45)
           || (pBot->current_weapon->hardware->id == CS_WEAPON_XM1014));
}


bool BotHoldsSecondary (bot_t *pBot)
{
   return ((pBot->current_weapon->hardware->id == CS_WEAPON_DEAGLE)
           || (pBot->current_weapon->hardware->id == CS_WEAPON_ELITE)
           || (pBot->current_weapon->hardware->id == CS_WEAPON_FIVESEVEN)
           || (pBot->current_weapon->hardware->id == CS_WEAPON_GLOCK18)
           || (pBot->current_weapon->hardware->id == CS_WEAPON_P228)
           || (pBot->current_weapon->hardware->id == CS_WEAPON_USP));
}


bool ItemIsPrimary (edict_t *pItem)
{
   const char *item_model;

   if (FNullEnt (pItem))
      return (FALSE); // reliability check

   // speedup: get the string in the string table and skip the 'models/w_' prefix...
   item_model = STRING (pItem->v.model) + 9;

   return ((strcmp (item_model, "ak47.mdl") == 0)
           || (strcmp (item_model, "aug.mdl") == 0)
           || (strcmp (item_model, "awp.mdl") == 0)
           || (strcmp (item_model, "g3sg1.mdl") == 0)
           || (strcmp (item_model, "m249.mdl") == 0)
           || (strcmp (item_model, "m3.mdl") == 0)
           || (strcmp (item_model, "m4a1.mdl") == 0)
           || (strcmp (item_model, "mac10.mdl") == 0)
           || (strcmp (item_model, "mp5.mdl") == 0)
           || (strcmp (item_model, "p228.mdl") == 0)
           || (strcmp (item_model, "p90.mdl") == 0)
           || (strcmp (item_model, "scout.mdl") == 0)
           || (strcmp (item_model, "sg550.mdl") == 0)
           || (strcmp (item_model, "sg552.mdl") == 0)
           || (strcmp (item_model, "tmp.mdl") == 0)
           || (strcmp (item_model, "ump45.mdl") == 0)
           || (strcmp (item_model, "xm1014.mdl") == 0));
}


bool ItemIsSecondary (edict_t *pItem)
{
   const char *item_model;

   if (FNullEnt (pItem))
      return (FALSE); // reliability check

   // speedup: get the string in the string table and skip the 'models/w_' prefix...
   item_model = STRING (pItem->v.model) + 9;

   return ((strcmp (item_model, "deagle.mdl") == 0)
           || (strcmp (item_model, "elite.mdl") == 0)
           || (strcmp (item_model, "fiveseven.mdl") == 0)
           || (strcmp (item_model, "glock18.mdl") == 0)
           || (strcmp (item_model, "p228.mdl") == 0)
           || (strcmp (item_model, "usp.mdl") == 0));
}


bool PlayerIsVIP (edict_t *pPlayer)
{
   // this function returns TRUE if the player whose entity is pointed to by pPlayer is the VIP

   if (!IsValidPlayer (pPlayer))
      return (FALSE); // reliability check

   return (strcmp (g_engfuncs.pfnInfoKeyValue ((*g_engfuncs.pfnGetInfoKeyBuffer) (pPlayer), "model"), "vip") == 0);
}


void PlayClientSoundsForBots (edict_t *pPlayer)
{
   // this function determines if the player pPlayer is walking or running, or climbing a ladder,
   // or landing on the ground, and so if he's likely to emit some client sound or not. Since
   // these types of sounds are predicted on the client side only, and bots have no client DLL,
   // we have to simulate their emitting in order for the bots to hear them. So in case a player
   // is moving, we bring his footstep sounds to the ears of the bots around. This sound is based
   // on the texture the player is walking on. Using TraceTexture(), we ask the engine for that
   // texture, then look up in the step sounds database in order to determine which footstep
   // sound is related to that texture. The ladder check then assumes that a player moving
   // vertically, not on the ground, having a ladder in his immediate surroundings is climbing
   // it, and the ladder sound is emitted periodically the same way footstep sounds are emitted.
   // Then, the landing check looks for non-null value of the player's punch angles (screen
   // tilting) while this player's damage inflictor be either null, or the world. If the test
   // success, a landing sound is emitted as well.
   // thanks to Tom Simpson from FoxBot for the water sounds handling

   edict_t *pGroundEntity = NULL;
   const char *texture_name, *player_weapon;
   char texture_type;
   char sound_path[256];
   int player_index;
   float player_velocity, volume;

   if (DebugLevel.is_observer && !(pPlayer->v.flags & FL_THIRDPARTYBOT))
      return; // skip real players if in observer mode

   player_index = ENTINDEX (pPlayer) - 1; // get the player index
   player_velocity = pPlayer->v.velocity.Length (); // get the player velocity
   player_weapon = STRING (pPlayer->v.weaponmodel) + 9; // get player's weapon, skip 'models/p_'

   // does the server allow footstep sounds AND this player is actually moving
   // AND is player on the ground AND is it time for him to make a footstep sound
   // OR has that player just landed on the ground after a jump ?
   if ((server.does_footsteps && IsOnFloor (pPlayer) && (player_velocity > 0)
        && (players[player_index].step_sound_time < *server.time))
       || ((FNullEnt (pPlayer->v.dmg_inflictor) || (pPlayer->v.dmg_inflictor == pWorldEntity))
           && (pPlayer->v.punchangle != g_vecZero)
           && !(players[player_index].prev_v.flags & (FL_ONGROUND | FL_PARTIALGROUND))))
   {
      // is this player sloshing in water ?
      if (pPlayer->v.waterlevel > 0)
      {
         sprintf (sound_path, "player/pl_slosh%d.wav", RANDOM_LONG (1, 4)); // build a slosh sound path

         // bring slosh sound from this player to the bots' ears
         DispatchSound (sound_path, pPlayer->v.origin + Vector (0, 0, -18), 0.9, ATTN_NORM);
         players[player_index].step_sound_time = *server.time + 0.300; // next slosh in 300 milliseconds
      }

      // else this player is definitely not in water, does he move fast enough to make sounds ?
      else if (player_velocity > MAX_WALK_SPEED)
      {
         // get the entity under the player's feet
         if (!FNullEnt (pPlayer->v.groundentity))
            pGroundEntity = pPlayer->v.groundentity; // this player is standing over something
         else
            pGroundEntity = pWorldEntity; // this player is standing over the world itself

         // ask the engine for the texture name on pGroundEntity under the player's feet
         texture_name = TRACE_TEXTURE (pGroundEntity, pPlayer->v.origin, Vector (0, 0, -9999));

         // if the engine found the texture, ask the game DLL for the texture type
         if (texture_name != NULL)
            texture_type = PM_FindTextureType ((char *) texture_name); // ask for texture type

         // given the type of texture under player's feet, prepare a sound file for being played
         switch (texture_type)
         {
            default:
            case CHAR_TEX_CONCRETE:
               sprintf (sound_path, "player/pl_step%d.wav", RANDOM_LONG (1, 4)); // 4 step sounds
               volume = 0.9;
               break;
            case CHAR_TEX_METAL:
               sprintf (sound_path, "player/pl_metal%d.wav", RANDOM_LONG (1, 4)); // 4 metal sounds
               volume = 0.9;
               break;
            case CHAR_TEX_DIRT:
               sprintf (sound_path, "player/pl_dirt%d.wav", RANDOM_LONG (1, 4)); // 4 dirt sounds
               volume = 0.9;
               break;
            case CHAR_TEX_VENT:
               sprintf (sound_path, "player/pl_duct%d.wav", RANDOM_LONG (1, 4)); // 4 duct sounds
               volume = 0.5;
               break;
            case CHAR_TEX_GRATE:
               sprintf (sound_path, "player/pl_grate%d.wav", RANDOM_LONG (1, 4)); // 4 grate sounds
               volume = 0.9;
               break;
            case CHAR_TEX_TILE:
               sprintf (sound_path, "player/pl_tile%d.wav", RANDOM_LONG (1, 5)); // 5 tile sounds
               volume = 0.8;
               break;
            case CHAR_TEX_SLOSH:
               sprintf (sound_path, "player/pl_slosh%d.wav", RANDOM_LONG (1, 4)); // 4 slosh sounds
               volume = 0.9;
               break;
            case CHAR_TEX_WOOD:
               sprintf (sound_path, "debris/wood%d.wav", RANDOM_LONG (1, 3)); // 3 wood sounds
               volume = 0.9;
               break;
            case CHAR_TEX_GLASS:
            case CHAR_TEX_COMPUTER:
               sprintf (sound_path, "debris/glass%d.wav", RANDOM_LONG (1, 4)); // 4 glass sounds
               volume = 0.8;
               break;
            case 'N':
               sprintf (sound_path, "player/pl_snow%d.wav", RANDOM_LONG (1, 6)); // 6 snow sounds
               volume = 0.8;
               break;
         }

         // did we hit a breakable ?
         if (!FNullEnt (pPlayer->v.groundentity)
             && (strcmp ("func_breakable", STRING (pPlayer->v.groundentity->v.classname)) == 0))
            volume /= 1.5; // drop volume, the object will already play a damaged sound

         // bring footstep sound from this player's feet to the bots' ears
         DispatchSound (sound_path, pPlayer->v.origin + Vector (0, 0, -18), volume, ATTN_NORM);
         players[player_index].step_sound_time = *server.time + 0.3; // next step in 300 milliseconds
      }
   }

   // is this player completely in water AND it's time to play a wade sound
   // AND this player is pressing the jump key for swimming up ?
   if ((players[player_index].step_sound_time < *server.time)
       && (pPlayer->v.waterlevel == 2) && (pPlayer->v.button & IN_JUMP))
   {
      sprintf (sound_path, "player/pl_wade%d.wav", RANDOM_LONG (1, 4)); // build a wade sound path

      // bring wade sound from this player to the bots' ears
      DispatchSound (sound_path, pPlayer->v.origin + Vector (0, 0, -18), 0.9, ATTN_NORM);
      players[player_index].step_sound_time = *server.time + 0.5; // next wade in 500 milliseconds
   }

   // now let's see if this player is on a ladder, for that we consider that he's not on the
   // ground, he's actually got a velocity (especially vertical), and that he's got a
   // func_ladder entity right in front of him. Is that player moving anormally NOT on ground ?
   if ((pPlayer->v.velocity.z != 0) && IsFlying (pPlayer) && !IsOnFloor (pPlayer))
   {
      pGroundEntity = NULL; // first ensure the pointer at which to start the search is NULL

      // cycle through all ladders...
      while ((pGroundEntity = UTIL_FindEntityByString (pGroundEntity, "classname", "func_ladder")) != NULL)
      {
         // is this ladder at the same height as the player AND the player is next to it (in
         // which case, assume he's climbing it), AND it's time for him to emit ladder sound ?
         if ((pGroundEntity->v.absmin.z < pPlayer->v.origin.z) && (pGroundEntity->v.absmax.z > pPlayer->v.origin.z)
             && (((pGroundEntity->v.absmin + pGroundEntity->v.absmax) / 2 - pPlayer->v.origin).Length2D () < 40)
             && (players[player_index].step_sound_time < *server.time))
         {
            volume = 0.8; // default volume for ladder sounds (empirical)

            // now build a random sound path amongst the 4 different ladder sounds
            sprintf (sound_path, "player/pl_ladder%d.wav", RANDOM_LONG (1, 4));

            // is the player ducking ?
            if (pPlayer->v.button & IN_DUCK)
               volume /= 1.5; // drop volume, the player is trying to climb silently

            // bring ladder sound from this player's feet to the bots' ears
            DispatchSound (sound_path, pPlayer->v.origin + Vector (0, 0, -18), volume, ATTN_NORM);
            players[player_index].step_sound_time = *server.time + 0.500; // next in 500 milliseconds
         }
      }
   }

   // and now let's see if this player is pulling the pin of a grenade...
   if ((pPlayer->v.button & IN_ATTACK) && !(pPlayer->v.oldbuttons & IN_ATTACK)
       && ((strcmp (player_weapon, "flashbang.mdl") == 0)
           || (strcmp (player_weapon, "hegrenade.mdl") == 0)
           || (strcmp (player_weapon, "smokegrenade.mdl") == 0)))
      DispatchSound ("weapons/pinpull.wav", GetGunPosition (pPlayer), 1.0, ATTN_NORM);

   return;
}
