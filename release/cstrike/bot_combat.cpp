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
// bot_combat.cpp
//

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "bot_common.h"
#include "bot_specific.h"

extern bot_weapon_t weapon_defs[MAX_WEAPONS];
extern bool b_observer_mode;
extern float f_team_radiotime[2];
extern float pause_time[5][2];


typedef struct
{
   int   iId; // the weapon ID value
   char  weapon_name[64]; // name of the weapon when selecting it
   int   skill_level; // bot skill must at least equal this value for bot to use this weapon
   float primary_min_distance; // 0 = no minimum
   float primary_max_distance; // 9999 = no maximum
   bool  can_use_underwater; // can use this weapon underwater
   int   min_primary_ammo; // minimum ammout of primary ammo needed to fire
   bool  primary_fire_hold; // hold down primary fire button to use?
   bool  primary_fire_burst; // use burst fire?
   float primary_burst_delay; // time to charge weapon
} bot_weapon_select_t;

typedef struct
{
   int iId;
   float primary_base_delay;
   float primary_min_delay[5];
   float primary_max_delay[5];
} bot_fire_delay_t;


// set grenades at the top of the array so they are the first weapon used
bot_weapon_select_t cs_weapon_select[] = {
   {CS_WEAPON_HEGRENADE, "weapon_hegrenade", 3, 300, 800, FALSE, 1, FALSE, FALSE, 0.0},
   {CS_WEAPON_FLASHBANG, "weapon_flashbang", 3, 200, 850, FALSE, 1, FALSE, FALSE, 0.0},
   {CS_WEAPON_SMOKEGRENADE, "weapon_smokegrenade", 3, 500, 1050, FALSE, 1, FALSE, FALSE, 0.0},
   {CS_WEAPON_M249, "weapon_m249", 1, 5.0, 9999.0, TRUE, 1, TRUE, TRUE, 0.1},
   {CS_WEAPON_MP5NAVY, "weapon_mp5navy", 1, 5.0, 9999.0, TRUE, 1, TRUE, TRUE, 0.1},
   {CS_WEAPON_TMP, "weapon_tmp", 1, 5.0, 9999.0, TRUE, 1, TRUE, TRUE, 0.2},
   {CS_WEAPON_P90, "weapon_p90", 1, 5.0, 9999.0, TRUE, 1, TRUE, TRUE, 0.2},
   {CS_WEAPON_MAC10, "weapon_mac10", 1, 5.0, 9999.0, TRUE, 1, TRUE, TRUE, 0.2},
   {CS_WEAPON_UMP45, "weapon_ump45", 1, 5.0, 9999.0, TRUE, 1, TRUE, TRUE, 0.2},
   {CS_WEAPON_SCOUT, "weapon_scout", 1, 5.0, 9999.0, TRUE, 1, FALSE, FALSE, 0.0},
   {CS_WEAPON_AWP, "weapon_awp", 1, 5.0, 9999.0, TRUE, 1, FALSE, FALSE, 0.0},
   {CS_WEAPON_G3SG1, "weapon_g3sg1", 1, 5.0, 9999.0, TRUE, 1, TRUE, TRUE, 0.1},
   {CS_WEAPON_SG550, "weapon_sg550", 1, 5.0, 9999.0, TRUE, 1, TRUE, TRUE, 0.1},
   {CS_WEAPON_AK47, "weapon_ak47", 1, 5.0, 9999.0, TRUE, 1, TRUE, TRUE, 0.1},
   {CS_WEAPON_SG552, "weapon_sg552", 1, 5.0, 9999.0, TRUE, 1, TRUE, TRUE, 0.2},
   {CS_WEAPON_M4A1, "weapon_m4a1", 1, 5.0, 9999.0, TRUE, 1, TRUE, TRUE, 0.2},
   {CS_WEAPON_AUG, "weapon_aug", 1, 5.0, 9999.0, TRUE, 1, TRUE, TRUE, 0.2},
   {CS_WEAPON_M3, "weapon_m3", 1, 5.0, 9999.0, TRUE, 1, FALSE, FALSE, 0.0},
   {CS_WEAPON_XM1014, "weapon_xm1014", 1, 5.0, 9999.0, TRUE, 1, FALSE, FALSE, 0.0},
   {CS_WEAPON_KNIFE, "weapon_knife", 1, 0.0, 50.0, TRUE, 0, TRUE, FALSE, 0.0},
   {CS_WEAPON_USP, "weapon_usp", 1, 0.0, 9999.0, TRUE, 1, FALSE, FALSE, 0.0},
   {CS_WEAPON_GLOCK18, "weapon_glock18", 1, 0.0, 9999.0, TRUE, 1, FALSE, FALSE, 0.0}, 
   {CS_WEAPON_DEAGLE, "weapon_deagle", 1, 0.0, 9999.0, TRUE, 1, FALSE, FALSE, 0.0},
   {CS_WEAPON_P228, "weapon_p228", 1, 0.0, 9999.0, TRUE, 1, FALSE, FALSE, 0.0},
   {CS_WEAPON_ELITE, "weapon_elite", 1, 0.0, 9999.0, TRUE, 1, FALSE, FALSE, 0.0},
   {CS_WEAPON_FIVESEVEN, "weapon_fiveseven", 1, 0.0, 9999.0, TRUE, 1, FALSE, FALSE, 0.0},
   /* terminator */
   {0, "", 0, 0.0, 0.0, TRUE, 1, FALSE, FALSE, 0.0}
};


// weapon firing delay based on skill (min and max delay for each weapon)
// THESE MUST MATCH THE SAME ORDER AS THE WEAPON SELECT ARRAY!!!
bot_fire_delay_t cs_fire_delay[] = {
   {CS_WEAPON_HEGRENADE, 0.3, {0.4, 0.3, 0.2, 0.1, 0.0}, {0.5, 0.4, 0.3, 0.2, 0.1}},
   {CS_WEAPON_FLASHBANG, 0.3, {0.4, 0.3, 0.2, 0.1, 0.0}, {0.5, 0.4, 0.3, 0.2, 0.1}},
   {CS_WEAPON_SMOKEGRENADE, 0.3, {0.4, 0.3, 0.2, 0.1, 0.0}, {0.5, 0.4, 0.3, 0.2, 0.1}},
   {CS_WEAPON_M249, 0.3, {0.4, 0.3, 0.2, 0.1, 0.0}, {0.5, 0.4, 0.3, 0.2, 0.1}},
   {CS_WEAPON_MP5NAVY, 0.3, {0.4, 0.3, 0.2, 0.1, 0.0}, {0.5, 0.4, 0.3, 0.2, 0.1}},
   {CS_WEAPON_TMP, 0.3, {0.4, 0.3, 0.2, 0.1, 0.0}, {0.5, 0.4, 0.3, 0.2, 0.1}},
   {CS_WEAPON_P90, 0.3, {0.4, 0.3, 0.2, 0.1, 0.0}, {0.5, 0.4, 0.3, 0.2, 0.1}},
   {CS_WEAPON_MAC10, 0.3, {0.4, 0.3, 0.2, 0.1, 0.0}, {0.5, 0.4, 0.3, 0.2, 0.1}},
   {CS_WEAPON_UMP45, 0.3, {0.4, 0.3, 0.2, 0.1, 0.0}, {0.5, 0.4, 0.3, 0.2, 0.1}},
   {CS_WEAPON_SCOUT, 0.5, {0.4, 0.3, 0.2, 0.1, 0.0}, {0.5, 0.4, 0.3, 0.2, 0.1}},
   {CS_WEAPON_AWP, 0.5, {0.4, 0.3, 0.2, 0.1, 0.0}, {0.5, 0.4, 0.3, 0.2, 0.1}},
   {CS_WEAPON_G3SG1, 0.3, {0.4, 0.3, 0.2, 0.1, 0.0}, {0.5, 0.4, 0.3, 0.2, 0.1}},
   {CS_WEAPON_SG550, 0.3, {0.4, 0.3, 0.2, 0.1, 0.0}, {0.5, 0.4, 0.3, 0.2, 0.1}},
   {CS_WEAPON_AK47, 0.3, {0.4, 0.3, 0.2, 0.1, 0.0}, {0.5, 0.4, 0.3, 0.2, 0.1}},
   {CS_WEAPON_SG552, 0.3, {0.4, 0.3, 0.2, 0.1, 0.0}, {0.5, 0.4, 0.3, 0.2, 0.1}},
   {CS_WEAPON_M4A1, 0.3, {0.4, 0.3, 0.2, 0.1, 0.0}, {0.5, 0.4, 0.3, 0.2, 0.1}},
   {CS_WEAPON_AUG, 0.3, {0.4, 0.3, 0.2, 0.1, 0.0}, {0.5, 0.4, 0.3, 0.2, 0.1}},
   {CS_WEAPON_M3, 0.3, {0.4, 0.3, 0.2, 0.1, 0.0}, {0.5, 0.4, 0.3, 0.2, 0.1}},
   {CS_WEAPON_XM1014, 0.3, {0.4, 0.3, 0.2, 0.1, 0.0}, {0.5, 0.4, 0.3, 0.2, 0.1}},
   {CS_WEAPON_KNIFE, 0.1, {0.4, 0.3, 0.2, 0.1, 0.0}, {0.5, 0.4, 0.3, 0.2, 0.1}},
   {CS_WEAPON_USP, 0.1, {0.2, 0.2, 0.1, 0.1, 0.0}, {0.3, 0.2, 0.2, 0.1, 0.1}},
   {CS_WEAPON_GLOCK18, 0.1, {0.2, 0.2, 0.1, 0.1, 0.0}, {0.3, 0.2, 0.2, 0.1, 0.1}},
   {CS_WEAPON_DEAGLE, 0.1, {0.2, 0.2, 0.1, 0.1, 0.0}, {0.3, 0.2, 0.2, 0.1, 0.1}},
   {CS_WEAPON_P228, 0.1, {0.2, 0.2, 0.1, 0.1, 0.0}, {0.3, 0.2, 0.2, 0.1, 0.1}},
   {CS_WEAPON_ELITE, 0.1, {0.2, 0.2, 0.1, 0.1, 0.0}, {0.3, 0.2, 0.2, 0.1, 0.1}},
   {CS_WEAPON_FIVESEVEN, 0.1, {0.2, 0.2, 0.1, 0.1, 0.0}, {0.3, 0.2, 0.2, 0.1, 0.1}},
   /* terminator */
   {0, 0.0, {0.0, 0.0, 0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 0.0, 0.0}}
};



edict_t *BotCheckForEnemies (bot_t *pBot)
{
   Vector vecEnd;
   edict_t *pNewEnemy = NULL;
   float nearestdistance = 2500;
   int i;

   if (pBot->pEdict == NULL)
      return NULL; // reliability check

   if (pBot->pBotEnemy != NULL)  // does the bot already have an enemy?
   {
      vecEnd = pBot->pBotEnemy->v.origin + pBot->pBotEnemy->v.view_ofs;

      // is the enemy dead ?
      if (!IsAlive (pBot->pBotEnemy))
      {
         if (RANDOM_LONG(1,100) <= (56 - 2 * gpGlobals->maxClients))
         {
            pBot->BotChat.b_saytext_kill = TRUE; // bot laughs (text)
            pBot->f_bot_saytext_time = gpGlobals->time + RANDOM_FLOAT (3.0, 5.0);
         }

         pBot->BotChat.b_sayaudio_attacking = FALSE; // don't complain about this enemy anymore
         pBot->BotChat.b_sayaudio_takingdamage = FALSE; // idem
         pBot->BotChat.b_sayaudio_victory = TRUE; // bot laughs (audio)
         pBot->f_bot_sayaudio_time = gpGlobals->time + RANDOM_FLOAT (0.5, 3.0);

         // once out of two times send a radio message
         if (RANDOM_LONG (1, 100) < 50)
            BotTalkOnTheRadio (pBot, RADIOMSG_ENEMYDOWN);

         // if this enemy was close to us, spray a logo
         if (((pBot->pBotEnemy->v.origin - pBot->pEdict->v.origin).Length () < 200) && (RANDOM_LONG (1, 100) < 33))
            pBot->f_spraying_logo_time = gpGlobals->time + RANDOM_FLOAT (1.0, 2.0);

         pBot->b_enemy_hidden = FALSE; // have no enemy anymore
         pBot->pVictimEntity = pBot->pBotEnemy; // bot remembers his victim
         pBot->pBotEnemy = NULL; // and nulls out enemy pointer
      }

      // else if enemy is still visible and in field of view, look for a greater threat, else keep it
      else if (FInViewCone (pBot->pBotEnemy->v.origin, pBot->pEdict)
               && (BotGetIdealAimVector (pBot, pBot->pBotEnemy) != Vector (0, 0, 0)))
      {
         // don't look for closer enemies if bot's current enemy is the VIP
         if (strcmp (g_engfuncs.pfnInfoKeyValue ((*g_engfuncs.pfnGetInfoKeyBuffer) (pBot->pBotEnemy), "model"), "vip") != 0)
         {
            // search the world for players...
            for (i = 1; i <= gpGlobals->maxClients; i++)
            {
               edict_t *pPlayer = INDEXENT (i);

               if (!pPlayer || pPlayer->free || (pPlayer == pBot->pEdict))
                  continue; // skip invalid players and skip self (i.e. this bot)

               if (!IsAlive (pPlayer))
                  continue; // skip this player if not alive (i.e. dead or dying)

               if (b_observer_mode && !(pPlayer->v.flags & FL_FAKECLIENT))
                  continue; // skip this player if real client and we are in observer mode

               if (GetTeam (pBot->pEdict) == GetTeam (pPlayer))
                  continue; // skip this player if it is a teammate

               // see if bot can see the player...
               if (FInViewCone (pPlayer->v.origin, pBot->pEdict) && (BotGetIdealAimVector (pBot, pPlayer) != Vector (0, 0, 0)))
               {
                  float distance = (pPlayer->v.origin - pBot->pEdict->v.origin).Length ();

                  if (distance < nearestdistance)
                  {
                     nearestdistance = distance; // update nearest distance
                     pBot->pBotEnemy = pPlayer; // bot found a greater threat !
                  }
               }
            }
         }

         // face the enemy
         Vector bot_angles = UTIL_VecToAngles (pBot->pBotEnemy->v.origin - pBot->pEdict->v.origin);
         BotSetIdealYaw (pBot, bot_angles.y);

         if (pBot->f_bot_sayaudio_time < gpGlobals->time)
         {
            pBot->BotChat.b_sayaudio_attacking = TRUE; // bot yells attack (audio)
            pBot->f_bot_sayaudio_time = gpGlobals->time + RANDOM_FLOAT (1.0, 4.5);
         }

         // set next time to reload
         pBot->f_reload_time = gpGlobals->time + RANDOM_LONG (3.0, 5.0);

         return (pBot->pBotEnemy);
      }

      // else bot lost his enemy
      else
      {
         pBot->f_lost_enemy_time = gpGlobals->time; // save lost enemy time
         pBot->BotMove.f_strafeleft_time = 0; // stop strafing
         pBot->BotMove.f_straferight_time = 0;
         if (RANDOM_LONG (1, 100) < 33)
            pBot->f_pause_time = gpGlobals->time + RANDOM_FLOAT (2.0, 7.0); // pause for a while
      }
   }

   // no enemy yet, let's search the world for players...
   for (i = 1; i <= gpGlobals->maxClients; i++)
   {
      edict_t *pPlayer = INDEXENT (i);

      if (!pPlayer || pPlayer->free || (pPlayer == pBot->pEdict))
         continue; // skip invalid players and skip self (i.e. this bot)

      if (!IsAlive (pPlayer))
         continue; // skip this player if not alive (i.e. dead or dying)

      if (b_observer_mode && !(pPlayer->v.flags & FL_FAKECLIENT))
         continue; // skip real players in observater mode

      // if it is a teammate, check if this teammate is firing in some direction
      if (GetTeam (pBot->pEdict) == GetTeam (pPlayer))
      {
         // check if this teammate is visible and relatively close
         if ((BotGetIdealAimVector (pBot, pPlayer) != Vector (0, 0, 0))
             && ((pPlayer->v.origin - pBot->pEdict->v.origin).Length () < 750))
         {
            // if this teammate is attacking something
            if ((pPlayer->v.button & IN_ATTACK) == IN_ATTACK)
            {
               // if this teammate is far, come to him
               if ((pPlayer->v.origin - pBot->pEdict->v.origin).Length () > 200)
                  pBot->v_reach_point = pPlayer->v.origin; // "i'm coming, mate !!"

               // else if we are close to him...
               else
               {
                  BotSetIdealYaw (pBot, pPlayer->v.v_angle.y); // look where he is looking
                  pBot->f_reach_time = gpGlobals->time + 0.5; // let the bot turn
               }
            }
         }

         continue; // don't target your teammates...
      }

      vecEnd = pPlayer->v.origin + pPlayer->v.view_ofs;

      // see if bot can see the player...
      if (FInViewCone (vecEnd, pBot->pEdict) && (BotGetIdealAimVector (pBot, pPlayer) != Vector (0, 0, 0)))
      {
         float distance = (pPlayer->v.origin - pBot->pEdict->v.origin).Length ();

         // target the nearest enemy if no VIP found
         if ((distance < nearestdistance)
             || (strcmp (g_engfuncs.pfnInfoKeyValue ((*g_engfuncs.pfnGetInfoKeyBuffer) (pPlayer), "model"), "vip") == 0))
         {
            if (strcmp (g_engfuncs.pfnInfoKeyValue ((*g_engfuncs.pfnGetInfoKeyBuffer) (pPlayer), "model"), "vip") == 0)
               nearestdistance = 0; // if the VIP has been found, keep it
            else
               nearestdistance = distance; // else normal proximity selection rules apply

            pNewEnemy = pPlayer; // bot found an enemy !

            pBot->f_see_enemy_time = gpGlobals->time; // save when we first saw this enemy
            pBot->f_aim_adjust_time = gpGlobals->time; // start adjusting aim now

            // if no enemy alert for about 30 seconds in team, send a radio message
            if (f_team_radiotime[GetTeam (pBot->pEdict)] + 30 < gpGlobals->time)
            {
               BotTalkOnTheRadio (pBot, RADIOMSG_ENEMYSPOTTED); // bot says 'enemy spotted'
               f_team_radiotime[GetTeam (pBot->pEdict)] = gpGlobals->time + RANDOM_FLOAT (5.0, 15.0);
               pBot->BotChat.b_sayaudio_alert = TRUE; // bot says 'alert'
               pBot->f_bot_sayaudio_time = gpGlobals->time + RANDOM_FLOAT (0.5, 3.0);
            }
         }
      }
   }

   if (pNewEnemy)
   {
      // face the enemy
      Vector bot_angles = UTIL_VecToAngles (pNewEnemy->v.origin - pBot->pEdict->v.origin);
      BotSetIdealYaw (pBot, bot_angles.y);

      // set next time to reload
      pBot->f_reload_time = gpGlobals->time + RANDOM_LONG (3.0, 5.0);
   }

   // is it time to reload ?
   if ((pBot->f_reload_time > 0) && (pBot->f_reload_time <= gpGlobals->time))
   {
      BotSwitchToBestWeapon (pBot); // switch to best gun;
      pBot->f_reload_time = -1; // so we won't keep reloading
      pBot->pEdict->v.button |= IN_RELOAD; // press reload button
   }

   return (pNewEnemy);
}


void BotSelectItem (bot_t *pBot, char *item_name)
{
   if (pBot->pEdict == NULL)
      return; // reliability check

   // if bot is selecting a sniper gun, switch to deadly sniper mode
   if ((strcmp (item_name, "weapon_scout") == 0)
       || (strcmp (item_name, "weapon_awp") == 0)
       || (strcmp (item_name, "weapon_sg550") == 0)
       || (strcmp (item_name, "weapon_sg552") == 0)
       || (strcmp (item_name, "weapon_g3sg1") == 0)
       || (strcmp (item_name, "weapon_aug") == 0))
   {
      pBot->f_togglesniper_time = gpGlobals->time + 0.45; // toggle sniper mode in 0.45 second
      pBot->f_shoot_time = gpGlobals->time + 0.50; // don't shoot in the meantime
   }

   FakeClientCommand (pBot->pEdict, item_name); // issue the select item command
}


void BotSwitchToBestWeapon (bot_t *pBot)
{
   bot_weapon_select_t *pSelect = &cs_weapon_select[0];
   bool ammo_left;
   int select_index = 0;

   if (pBot->pEdict == NULL)
      return; // reliability check

   // select the best weapon to use
   while (pSelect[select_index].iId)
   {
      // reset weapon usable state
      ammo_left = FALSE;

      // is the bot NOT carrying this weapon ?
      if (!(pBot->bot_weapons & (1 << pSelect[select_index].iId)))
      {
         select_index++; // skip to next weapon
         continue;
      }   

      // is the bot NOT skilled enough to use this weapon ?
      if (pBot->bot_skill < pSelect[select_index].skill_level)
      {
         select_index++; // skip to next weapon
         continue;
      }

      // is the bot underwater AND does this weapon NOT work under water ?
      if ((pBot->pEdict->v.waterlevel == 3) && !(pSelect[select_index].can_use_underwater))
      {
         select_index++; // skip to next weapon
         continue;
      }

      // is the bot camping AND does this weapon NOT fit for the job ?
      if ((pBot->f_camp_time > gpGlobals->time)
          && ((pSelect[select_index].iId == CS_WEAPON_FLASHBANG)
              || (pSelect[select_index].iId == CS_WEAPON_HEGRENADE)
              || (pSelect[select_index].iId == CS_WEAPON_SMOKEGRENADE)
              || (pSelect[select_index].iId == CS_WEAPON_M3)
              || (pSelect[select_index].iId == CS_WEAPON_XM1014)
              || (pSelect[select_index].iId == CS_WEAPON_KNIFE)))
      {
         select_index++; // skip to next weapon
         continue;
      }

      // is the bot already holding this weapon and there is still ammo in clip ?
      if ((pSelect[select_index].iId == pBot->current_weapon.iId)
          && ((pSelect[select_index].min_primary_ammo <= 0) || (pBot->current_weapon.iClip < 0)
              || (pBot->current_weapon.iClip >= pSelect[select_index].min_primary_ammo)))
         ammo_left = TRUE;

      // does this weapon have clips left ?
      if (((weapon_defs[pSelect[select_index].iId].iAmmo1 == -1)
           || (pBot->m_rgAmmo[weapon_defs[pSelect[select_index].iId].iAmmo1]
               >= pSelect[select_index].min_primary_ammo)))
         ammo_left = TRUE;

      // see if there is enough ammo OR no ammo required
      if (ammo_left)
         break; // at last, bot found the right weapon to kick your ass with
      else
      {
         select_index++; // skip to next weapon
         continue;
      }
   }

   // is this one NOT bot's current weapon ?
   if ((pSelect[select_index].iId > 0) && (pBot->current_weapon.iId != pSelect[select_index].iId))
      BotSelectItem (pBot, pSelect[select_index].weapon_name); // select this weapon
}


// specifing a weapon_choice allows you to choose the weapon the bot will
// use (assuming enough ammo exists for that weapon)

void BotFireWeapon (Vector v_enemy, bot_t *pBot, int weapon_choice)
{
   bot_weapon_select_t *pSelect = &cs_weapon_select[0];
   bot_fire_delay_t *pDelay = &cs_fire_delay[0];
   bool ammo_left;
   int select_index = 0;
   float distance = v_enemy.Length (); // distance to enemy

   if (pBot->pEdict == NULL)
      return; // reliability check

   // are we burst-firing ?
   if (pBot->f_primary_charging > 0)
   {
      // are we firing with a sniper rifle ?
      if ((pBot->charging_weapon_id == CS_WEAPON_SCOUT)
          || (pBot->charging_weapon_id == CS_WEAPON_AWP)
          || (pBot->charging_weapon_id == CS_WEAPON_G3SG1)
          || (pBot->charging_weapon_id == CS_WEAPON_SG550))
         pBot->BotMove.f_forward_time = 0; // don't move while using sniper rifle

      // is it time to stop the burst ?
      if (pBot->f_primary_charging <= gpGlobals->time)
      {
         pBot->f_primary_charging = -1; // stop firing it (-1 means not firing)

         // identify the weapon we were firing
         while ((pSelect[select_index].iId) && (pSelect[select_index].iId != pBot->charging_weapon_id))
            select_index++;

         // set next time to shoot, as next frame will automatically release fire button
         float base_delay = pDelay[select_index].primary_base_delay;
         float min_delay = pDelay[select_index].primary_min_delay[pBot->bot_skill - 1];
         float max_delay = pDelay[select_index].primary_max_delay[pBot->bot_skill - 1];

         pBot->f_shoot_time = gpGlobals->time + base_delay + RANDOM_FLOAT (min_delay, max_delay);
         return;
      }
      else
      {
         pBot->pEdict->v.button |= IN_ATTACK; // press the FIRE button
         pBot->f_shoot_time = gpGlobals->time; // set next frame to keep pressing the button
         return;
      }
   }

   // we are not already charging a weapon, so select the best one to use
   while (pSelect[select_index].iId)
   {
      // reset weapon usable state
      ammo_left = FALSE;

      // was a weapon choice specified AND if so do they NOT match ?
      if ((weapon_choice != 0) && (weapon_choice != pSelect[select_index].iId))
      {
         select_index++; // skip to next weapon
         continue;
      }

      // is the bot NOT carrying this weapon ?
      if (!(pBot->bot_weapons & (1 << pSelect[select_index].iId)))
      {
         select_index++; // skip to next weapon
         continue;
      }   

      // is the bot NOT skilled enough to use this weapon ?
      if (pBot->bot_skill < pSelect[select_index].skill_level)
      {
         select_index++; // skip to next weapon
         continue;
      }

      // is the bot underwater AND does this weapon NOT work under water ?
      if ((pBot->pEdict->v.waterlevel == 3) && !(pSelect[select_index].can_use_underwater))
      {
         select_index++; // skip to next weapon
         continue;
      }

      // is the bot already holding this weapon and there is still ammo in clip ?
      if ((pSelect[select_index].iId == pBot->current_weapon.iId)
          && ((pSelect[select_index].min_primary_ammo <= 0) || (pBot->current_weapon.iClip < 0)
              || (pBot->current_weapon.iClip >= pSelect[select_index].min_primary_ammo)))
         ammo_left = TRUE;

      // does this weapon have clips left ?
      if (((weapon_defs[pSelect[select_index].iId].iAmmo1 == -1)
           || (pBot->m_rgAmmo[weapon_defs[pSelect[select_index].iId].iAmmo1]
               >= pSelect[select_index].min_primary_ammo)))
         ammo_left = TRUE;

      // see if there is enough ammo
      // AND the bot is far enough away to fire
      // AND the bot is close enough to the enemy to fire
      if (ammo_left
          && (distance >= pSelect[select_index].primary_min_distance)
          && (distance <= pSelect[select_index].primary_max_distance))
         break; // at last, bot found the right weapon to kick your ass with
      else
      {
         select_index++; // skip to next weapon
         continue;
      }
   }

   // if bot can't decide which weapon to choose, cycle again but don't check for distances
   if (pSelect[select_index].iId == 0)
   {
      select_index = 0; // reset select_index to the start of the array

      while (pSelect[select_index].iId)
      {
         // reset weapon usable state
         ammo_left = FALSE;

         // is the bot NOT carrying this weapon ?
         if (!(pBot->bot_weapons & (1 << pSelect[select_index].iId)))
         {
            select_index++; // skip to next weapon
            continue;
         }   

         // is the bot underwater AND does this weapon NOT work under water ?
         if ((pBot->pEdict->v.waterlevel == 3) && !(pSelect[select_index].can_use_underwater))
         {
            select_index++; // skip to next weapon
            continue;
         }

         // is the bot already holding this weapon and there is still ammo in clip ?
         if ((pSelect[select_index].iId == pBot->current_weapon.iId)
             && ((pSelect[select_index].min_primary_ammo <= 0) || (pBot->current_weapon.iClip < 0)
                 || (pBot->current_weapon.iClip >= pSelect[select_index].min_primary_ammo)))
            ammo_left = TRUE;

         // does this weapon have clips left ?
         if (((weapon_defs[pSelect[select_index].iId].iAmmo1 == -1)
              || (pBot->m_rgAmmo[weapon_defs[pSelect[select_index].iId].iAmmo1]
                  >= pSelect[select_index].min_primary_ammo)))
            ammo_left = TRUE;

         // see if there is enough ammo
         if (ammo_left)
            break; // bot finally found a weapon to use
         else
         {
            select_index++; // skip to next weapon
            continue;
         }
      }
   }

   // is this one NOT bot's current weapon ?
   if ((pSelect[select_index].iId > 0) && (pBot->current_weapon.iId != pSelect[select_index].iId))
      BotSelectItem (pBot, pSelect[select_index].weapon_name); // select this weapon

   // is bot just 'standing' on enemy ?
   if ((pBot->pEdict->v.origin.z - pBot->pBotEnemy->v.origin.z > 30)
       && ((pBot->pBotEnemy->v.origin - pBot->pEdict->v.origin).Length () < 90))
      pBot->BotMove.f_duck_time = gpGlobals->time + 0.1; // duck to hit him

   // is this weapon a sniper rifle ?
   if ((pSelect[select_index].iId == CS_WEAPON_SCOUT) || (pSelect[select_index].iId == CS_WEAPON_AWP)
       || (pSelect[select_index].iId == CS_WEAPON_G3SG1) || (pSelect[select_index].iId == CS_WEAPON_SG550))
   {
      pBot->BotMove.f_forward_time = 0; // don't move while using sniper rifle

      // are we moving too fast to use it ?
      if (pBot->pEdict->v.velocity.Length () > 50)
         return; // don't press attack key until velocity is < 50

      UTIL_MakeVectors (pBot->pEdict->v.v_angle); // build base vectors in aim direction

      // have we aimed with insufficient accuracy to fire ?
      if (UTIL_AngleOfVectors (pBot->pBotEnemy->v.origin - pBot->pEdict->v.origin, gpGlobals->v_forward)
          > (300 / (pBot->pBotEnemy->v.origin - pBot->pEdict->v.origin).Length ()))
         return; // don't shoot yet, it would be a monumental error...
   }

   // is this weapon a grenade ?
   if ((pSelect[select_index].iId == CS_WEAPON_HEGRENADE)
       || (pSelect[select_index].iId == CS_WEAPON_FLASHBANG)
       || (pSelect[select_index].iId == CS_WEAPON_SMOKEGRENADE))
   {
      pBot->BotChat.b_sayaudio_throwgrenade = TRUE; // bot says 'throwgrenade'
      pBot->f_bot_sayaudio_time = gpGlobals->time; + RANDOM_FLOAT (0.2, 0.8);
   }

   // if NOT in the case where it is a proximity weapon and bot is far from his enemy...
   if (!((pSelect[select_index].iId == CS_WEAPON_KNIFE)
         && ((pBot->pBotEnemy->v.origin - pBot->pEdict->v.origin).Length () > 50))
        || (RANDOM_LONG (1, 100) < 2))
      pBot->pEdict->v.button |= IN_ATTACK; // press the FIRE button

   // should we use burst-fire ?
   if ((pBot->bot_skill > 2) && (pSelect[select_index].primary_fire_burst)
       && ((pBot->pBotEnemy->v.origin - pBot->pEdict->v.origin).Length () < 200 * (6 - pBot->bot_skill)))
   {
      float burst_delay = pSelect[select_index].primary_burst_delay + ((6 - pBot->bot_skill) / 10);

      pBot->f_primary_charging = gpGlobals->time + burst_delay; // set the delay of burst
      pBot->charging_weapon_id = pSelect[select_index].iId; // save charging weapon id
      pBot->f_shoot_time = gpGlobals->time; // set next frame to keep pressing the button
   }

   // else this is a normal shoot
   else
   {
      // should we hold button down to fire ?
      if (pSelect[select_index].primary_fire_hold)
      {
         // if no need for ammo OR no need for clip OR still enough ammo in clip...
         if ((pSelect[select_index].min_primary_ammo <= 0) || (pBot->current_weapon.iClip < 0)
             || (pBot->current_weapon.iClip >= pSelect[select_index].min_primary_ammo))
            pBot->f_shoot_time = gpGlobals->time; // set next frame to keep pressing the button
         else
         {
            pBot->f_shoot_time = gpGlobals->time + 0.25; // delay a while and reload
            pBot->pEdict->v.button = IN_RELOAD; // press the RELOAD button
         }
      }

      // else set next time to shoot
      else
      {
         float base_delay = pDelay[select_index].primary_base_delay;
         float min_delay = pDelay[select_index].primary_min_delay[pBot->bot_skill - 1];
         float max_delay = pDelay[select_index].primary_max_delay[pBot->bot_skill - 1];

         pBot->f_shoot_time = gpGlobals->time + base_delay + RANDOM_FLOAT (min_delay, max_delay);
      }
   }
}


void BotShootAtEnemy (bot_t *pBot)
{
   float f_distance;
   Vector v_enemy, v_target_angle;
   TraceResult tr;

   if ((pBot->pEdict == NULL) || (pBot->pBotEnemy == NULL))
      return; // reliability check

   // target some part of our enemy's body
   v_enemy = BotGetIdealAimVector (pBot, pBot->pBotEnemy) - GetGunPosition (pBot->pEdict);

   // add imprecision offset resulting from bot & player movement (based on bot's skill)
   v_enemy = v_enemy + (((pBot->pBotEnemy->v.velocity - pBot->pEdict->v.velocity) / (pBot->bot_skill * 2)) * RANDOM_FLOAT (0, 1));

   // move the aim cursor
   BotPointGun (pBot, UTIL_VecToAngles (v_enemy));

   f_distance = v_enemy.Length (); // how far away is the enemy scum ?

   // decide which behaviour the bot will have while firing
   if (f_distance > 200)
   {
      bool has_sniper_rifle = ((pBot->current_weapon.iId == CS_WEAPON_SCOUT)
                               || (pBot->current_weapon.iId == CS_WEAPON_AWP)
                               || (pBot->current_weapon.iId == CS_WEAPON_G3SG1)
                               || (pBot->current_weapon.iId == CS_WEAPON_SG550));

      // if not fearful, no sniper gun and won't fall OR proximity weapon, run to enemy
      if ((!pBot->b_is_fearful && !has_sniper_rifle && !BotCanFallForward (pBot, &tr))
          || (pBot->current_weapon.iId == CS_WEAPON_KNIFE))
      {
         pBot->BotMove.f_forward_time = gpGlobals->time + 60.0; // if not fearful and no sniper...
         pBot->BotMove.b_is_walking = FALSE; // ...run if enemy is far
      }

      // else if not fearful OR quite far from enemy, keep the distance
      else if ((!pBot->b_is_fearful) || (f_distance > 500))
         pBot->BotMove.f_forward_time = 0; // try to stay at a distant range from our enemy

      // else bot may be fearful, or enemy too close, so step back
      else
         pBot->BotMove.f_backwards_time = gpGlobals->time + 0.5; // make some steps back

      // if not strafing for [max pause time] seconds, pick up a strafe direction
      if ((pBot->BotMove.f_strafeleft_time + pause_time[pBot->bot_skill - 1][1] < gpGlobals->time)
          && (pBot->BotMove.f_straferight_time + pause_time[pBot->bot_skill - 1][1] < gpGlobals->time)
          && (pBot->bot_skill > 2))
      {
         if (RANDOM_LONG (1, 100) < 50)
            pBot->BotMove.f_strafeleft_time = gpGlobals->time + RANDOM_FLOAT (0.5, 2.0);
         else
            pBot->BotMove.f_straferight_time = gpGlobals->time + RANDOM_FLOAT (0.5, 2.0);
      }

      // else if bot is already strafing AND bot has not jumped for 0.7 s
      // AND bot hasn't some sort of sniper gun AND bot is skilled enough, randomly jump
      else if ((pBot->BotMove.f_strafe_speed != 0) && (pBot->BotMove.f_jump_time + 0.7 < gpGlobals->time)
               && (!has_sniper_rifle) && (pBot->bot_skill > 2))
         pBot->BotMove.f_jump_time = gpGlobals->time + RANDOM_FLOAT (0.1, (6.0 - pBot->bot_skill) / 4); //jump

      // else if bot skilled enough and not currently strafing, duck
      else if ((pBot->bot_skill > 2) && (pBot->BotMove.f_strafeleft_time + 0.6 < gpGlobals->time)
               && (pBot->BotMove.f_straferight_time + 0.6 < gpGlobals->time))
      {
         pBot->BotMove.f_duck_time = gpGlobals->time + 0.8; // duck and fire
         if ((pBot->f_see_enemy_time + 5.0 < gpGlobals->time)
             || ((pBot->f_see_enemy_time + 0.5 < gpGlobals->time)
                 && (pBot->current_weapon.iId == CS_WEAPON_KNIFE)))
            pBot->b_is_fearful = FALSE; // prevent bot from never attacking
      }

      // if bot should move and is not actually moving, try to unstuck it
      if (((pBot->BotMove.f_move_speed != 0) || (pBot->BotMove.f_strafe_speed != 0)) && BotIsStuck (pBot))
         BotUnstuck (pBot);
   }

   else if (f_distance > 20)
   {
      pBot->BotMove.f_forward_time = gpGlobals->time + 60.0; // walk if distance is closer
      pBot->BotMove.b_is_walking = TRUE;

      // if bot is fearful and not currently strafing AND enemy is not ducking yet, duck
      if (pBot->b_is_fearful && (pBot->BotMove.f_strafeleft_time + 0.6 < gpGlobals->time)
          && (pBot->BotMove.f_straferight_time + 0.6 < gpGlobals->time)
          && ((pBot->pBotEnemy->v.button & IN_DUCK) != IN_DUCK))
      {
         pBot->BotMove.f_duck_time = gpGlobals->time + 0.8; // duck and fire
         if ((pBot->f_see_enemy_time + 5.0 < gpGlobals->time)
             || ((pBot->f_see_enemy_time + 0.5 < gpGlobals->time)
                 && (pBot->current_weapon.iId == CS_WEAPON_KNIFE)))
            pBot->b_is_fearful = FALSE; // prevent bot from never attacking
      }
   }

   else
      pBot->BotMove.f_forward_time = 0; // don't move if close enough

   // is it time to shoot yet ?
   if (pBot->f_shoot_time <= gpGlobals->time)
      BotFireWeapon (v_enemy, pBot, 0); // select the best weapon at this distance and fire

   // else is it time to toggle sniper mode ?
   else if ((pBot->f_togglesniper_time > 0) && (pBot->f_togglesniper_time < gpGlobals->time))
   {
      pBot->pEdict->v.button |= IN_ATTACK2; // press the secondary fire button to toggle mode
      pBot->f_togglesniper_time = 0; // reset toggle sniper mode time
   }
}


void BotPlantBomb (bot_t *pBot, Vector v_target)
{
   if (pBot->pEdict == NULL)
      return; // reliability check

   // if bomb icon is blinking OR bot is close to target, plant the bomb...
   if (pBot->b_can_plant || ((v_target - pBot->pEdict->v.origin).Length () < 70))
   {
      pBot->b_is_planting = TRUE; // make the bot remember he is planting a bomb
      BotTalkOnTheRadio (pBot, RADIOMSG_GONNABLOW); // bot speaks, "get outta here!"
      pBot->f_pause_time = gpGlobals->time + 4.5; // time for planting the bomb
      pBot->BotMove.f_duck_time = gpGlobals->time + 4.5; // duck while planting
   }

   // else let's run there...
   else
      BotReachPosition (pBot, v_target);
}


void BotDefuseBomb (bot_t *pBot, edict_t *pBomb)
{
   if ((pBot->pEdict == NULL) || (pBomb == NULL))
      return; // reliability check

   // if close to bomb, defuse it...
   if ((pBomb->v.origin - pBot->pEdict->v.origin).Length2D () < 40)
   {
      pBot->v_goal = pBomb->v.origin; // make the bot remember the bomb location
      pBot->b_is_defusing = TRUE; // make the bot remember he is defusing a bomb
      BotTalkOnTheRadio (pBot, RADIOMSG_NEEDBACKUP); // bot speaks, "i need backup!"
      pBot->f_pause_time = gpGlobals->time + 12.5; // time for defusing the bomb
      pBot->BotMove.f_duck_time = gpGlobals->time + 12.5; // duck while defusing
   }

   // else if getting close, slow down
   else if ((pBomb->v.origin - pBot->pEdict->v.origin).Length2D () < 60)
   {
      pBot->BotMove.f_forward_time = gpGlobals->time + 60.0; // go ahead...
      pBot->BotMove.b_is_walking = TRUE; // ...but walk
      BotPointGun (pBot, UTIL_VecToAngles (pBomb->v.origin - GetGunPosition (pBot->pEdict))); // look at bomb
   }

   // else let's run there...
   else
      BotReachPosition (pBot, pBomb->v.origin);
}
