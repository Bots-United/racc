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
extern int team_allies[4];
extern int max_armor[10];
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
   bool  primary_fire_charge; // charge weapon using primary fire?
   float primary_charge_delay; // time to charge weapon
} bot_weapon_select_t;

typedef struct
{
   int iId;
   float primary_base_delay;
   float primary_min_delay[5];
   float primary_max_delay[5];
} bot_fire_delay_t;


// set grenades at the top of the array so they are the first weapon used
bot_weapon_select_t tfc_weapon_select[] = {
   {TF_WEAPON_AC, "tf_weapon_ac", 1, 5.0, 9999.0, TRUE, 1, TRUE, FALSE, 0.0},
   {TF_WEAPON_MEDIKIT, "tf_weapon_medikit", 1, 0.0, 40.0, TRUE, -1, TRUE, FALSE, 0.0},
   {TF_WEAPON_SPANNER, "tf_weapon_spanner", 1, 0.0, 40.0, TRUE, -1, TRUE, FALSE, 0.0},
   {TF_WEAPON_KNIFE, "tf_weapon_knife", 1, 0.0, 40.0, TRUE, -1, TRUE, FALSE, 0.0},
   {TF_WEAPON_SNIPERRIFLE, "tf_weapon_sniperrifle", 1, 5.0, 9999.0, TRUE, 1, FALSE, TRUE, 1.0},
   {TF_WEAPON_FLAMETHROWER, "tf_weapon_flamethrower", 1, 5.0, 700.0, FALSE, 1, TRUE, FALSE, 0.0},
   {TF_WEAPON_AUTORIFLE, "tf_weapon_autorifle", 1, 5.0, 9999.0, TRUE, 1, TRUE, FALSE, 0.0},
   {TF_WEAPON_GL, "tf_weapon_gl", 1, 200.0, 1000.0, TRUE, 1, FALSE, FALSE, 0.0},
   {TF_WEAPON_RPG, "tf_weapon_rpg", 1, 200.0, 9999.0, TRUE, 1, FALSE, FALSE, 0.0},
   {TF_WEAPON_IC, "tf_weapon_ic", 1, 200.0, 1000.0, TRUE, 1, FALSE, FALSE, 0.0},
   {TF_WEAPON_TRANQ, "tf_weapon_tranq", 1, 5.0, 500.0, TRUE, 1, TRUE, FALSE, 0.0},
   {TF_WEAPON_RAILGUN, "tf_weapon_railgun", 1, 5.0, 800.0, TRUE, 1, TRUE, FALSE, 0.0},
   {TF_WEAPON_SUPERNAILGUN, "tf_weapon_superng", 1, 5.0, 9999.0, TRUE, 1, TRUE, FALSE, 0.0},
   {TF_WEAPON_SUPERSHOTGUN, "tf_weapon_supershotgun", 1, 5.0, 9999.0, TRUE, 1, FALSE, FALSE, 0.0},
   {TF_WEAPON_SHOTGUN, "tf_weapon_shotgun", 1, 5.0, 9999.0, TRUE, 1, FALSE, FALSE, 0.0},
   {TF_WEAPON_NAILGUN, "tf_weapon_ng", 1, 5.0, 9999.0, TRUE, 1, TRUE, FALSE, 0.0},
   {TF_WEAPON_AXE, "tf_weapon_axe", 1, 0.0, 40.0, TRUE, -1, TRUE, FALSE, 0.0},
   /* terminator */
   {0, "", 0, 0.0, 0.0, TRUE, 1, FALSE, FALSE, 0.0}
};


// weapon firing delay based on skill (min and max delay for each weapon)
// THESE MUST MATCH THE SAME ORDER AS THE WEAPON SELECT ARRAY!!!
bot_fire_delay_t tfc_fire_delay[] = {
   {TF_WEAPON_AC, 0.3, {0.4, 0.3, 0.2, 0.1, 0.0}, {0.5, 0.4, 0.3, 0.2, 0.1}},
   {TF_WEAPON_MEDIKIT, 0.1, {0.4, 0.3, 0.2, 0.1, 0.0}, {0.5, 0.4, 0.3, 0.2, 0.1}},
   {TF_WEAPON_SPANNER, 0.1, {0.4, 0.3, 0.2, 0.1, 0.0}, {0.5, 0.4, 0.3, 0.2, 0.1}},
   {TF_WEAPON_KNIFE, 0.1, {0.4, 0.3, 0.2, 0.1, 0.0}, {0.5, 0.4, 0.3, 0.2, 0.1}},
   {TF_WEAPON_SNIPERRIFLE, 0.9, {0.4, 0.3, 0.2, 0.1, 0.0}, {0.5, 0.4, 0.3, 0.2, 0.1}},
   {TF_WEAPON_FLAMETHROWER, 0.3, {0.4, 0.3, 0.2, 0.1, 0.0}, {0.5, 0.4, 0.3, 0.2, 0.1}},
   {TF_WEAPON_AUTORIFLE, 0.4, {0.4, 0.3, 0.2, 0.1, 0.0}, {0.5, 0.4, 0.3, 0.2, 0.1}},
   {TF_WEAPON_GL, 0.4, {0.4, 0.3, 0.2, 0.1, 0.0}, {0.5, 0.4, 0.3, 0.2, 0.1}},
   {TF_WEAPON_RPG, 0.9, {0.4, 0.3, 0.2, 0.1, 0.0}, {0.5, 0.4, 0.3, 0.2, 0.1}},
   {TF_WEAPON_IC, 0.3, {0.4, 0.3, 0.2, 0.1, 0.0}, {0.5, 0.4, 0.3, 0.2, 0.1}},
   {TF_WEAPON_TRANQ, 0.3, {0.4, 0.3, 0.2, 0.1, 0.0}, {0.5, 0.4, 0.3, 0.2, 0.1}},
   {TF_WEAPON_RAILGUN, 0.3, {0.4, 0.3, 0.2, 0.1, 0.0}, {0.5, 0.4, 0.3, 0.2, 0.1}},
   {TF_WEAPON_SUPERNAILGUN, 0.3, {0.4, 0.3, 0.2, 0.1, 0.0}, {0.5, 0.4, 0.3, 0.2, 0.1}},
   {TF_WEAPON_SUPERSHOTGUN, 0.3, {0.4, 0.3, 0.2, 0.1, 0.0}, {0.5, 0.4, 0.3, 0.2, 0.1}},
   {TF_WEAPON_SHOTGUN, 0.1, {0.4, 0.3, 0.2, 0.1, 0.0}, {0.5, 0.4, 0.3, 0.2, 0.1}},
   {TF_WEAPON_NAILGUN, 0.2, {0.4, 0.3, 0.2, 0.1, 0.0}, {0.5, 0.4, 0.3, 0.2, 0.1}},
   {TF_WEAPON_AXE, 0.1, {0.4, 0.3, 0.2, 0.1, 0.0}, {0.5, 0.4, 0.3, 0.2, 0.1}},
   /* terminator */
   {0, 0.0, {0.0, 0.0, 0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 0.0, 0.0}}
};



edict_t *BotCheckForEnemies (bot_t *pBot)
{
   Vector vecEnd;
   edict_t *pent = NULL;
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

         // if this enemy was close to us, spray a logo
         if (((pBot->pBotEnemy->v.origin - pBot->pEdict->v.origin).Length () < 200) && (RANDOM_LONG (1, 100) < 33))
            pBot->f_spraying_logo_time = gpGlobals->time + RANDOM_FLOAT (1.0, 2.0);

         pBot->b_enemy_hidden = FALSE; // have no enemy anymore
         pBot->pVictimEntity = pBot->pBotEnemy; // bot remembers his victim
         pBot->pBotEnemy = NULL; // and nulls out enemy pointer
      }

      // else if the enemy is still visible and in field of view, look for a greater threat, else keep it
      else if (FInViewCone (vecEnd, pBot->pEdict) && (BotGetIdealAimVector (pBot, pBot->pBotEnemy) != Vector (0, 0, 0)))
      {
         // if bot is medic, enemy can be a teammate bot is healing
         if (pBot->pEdict->v.playerclass == TFC_CLASS_MEDIC)
         {
            if (pBot->pBotEnemy->v.health >= pBot->pBotEnemy->v.max_health)
               pBot->pBotEnemy = NULL; // player is healed, null out pointer
         }

         // else if bot is engineer, enemy can be a teammate or a sentry gun bot is repairing
         else if (pBot->pEdict->v.playerclass == TFC_CLASS_ENGINEER)
         {
            if ((pBot->pBotEnemy->v.armorvalue >= max_armor[pBot->pBotEnemy->v.playerclass])
                || (pBot->pBotEnemy->v.health / pBot->pBotEnemy->v.max_health * 100 > 80))
               pBot->pBotEnemy = NULL; // player/sentry gun is repaired, null out pointer
         }

         // this one really is a big fat evil enemy, or a teammate still needing to be healed
         else
         {
            // don't look for closer enemies if bot's current enemy is not "human-like"
            if ((pBot->pBotEnemy->v.flags & FL_CLIENT) || (pBot->pBotEnemy->v.flags & FL_FAKECLIENT))
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

                  int player_team = GetTeam (pPlayer);
                  int bot_team = GetTeam (pBot->pEdict);

                  if ((bot_team == player_team) || (team_allies[bot_team] & (1 << player_team)))
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

            // face him
            Vector bot_angles = UTIL_VecToAngles (pBot->pBotEnemy->v.origin - pBot->pEdict->v.origin);
            BotSetIdealYaw (pBot, bot_angles.y);

            int player_team = GetTeam (pBot->pBotEnemy);
            int bot_team = GetTeam (pBot->pEdict);

            // if certain to deal with an enemy, talk
            if ((pBot->f_bot_sayaudio_time < gpGlobals->time)
                && !((bot_team == player_team) || (team_allies[bot_team] & (1 << player_team))))
            {
               pBot->BotChat.b_sayaudio_attacking = TRUE; // bot yells attack (audio)
               pBot->f_bot_sayaudio_time = gpGlobals->time + RANDOM_FLOAT (1.0, 4.5);
            }

            // set next time to reload
            pBot->f_reload_time = gpGlobals->time + RANDOM_LONG (3.0, 5.0);
            pBot->v_lastseenenemy_position = pBot->pBotEnemy->v.origin; // save enemy position

            return (pBot->pBotEnemy);
         }
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

   // if bot is a medic, check for players needing to be healed
   if (pBot->pEdict->v.playerclass == TFC_CLASS_MEDIC)
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
            continue; // skip real players in observater mode

         int player_team = GetTeam (pPlayer);
         int bot_team = GetTeam (pBot->pEdict);

         // don't target your enemies...
         if ((bot_team != player_team) && !(team_allies[bot_team] & (1 << player_team)))
            continue;

         // check if player needs to be healed...
         if ((pPlayer->v.health / pPlayer->v.max_health) > 0.50)
            continue; // health greater than 50% so ignore

         vecEnd = pPlayer->v.origin + pPlayer->v.view_ofs;

         // see if bot can see the player...
         if (FInViewCone (vecEnd, pBot->pEdict) && (BotGetIdealAimVector (pBot, pPlayer) != Vector (0, 0, 0)))
         {
            float distance = (pPlayer->v.origin - pBot->pEdict->v.origin).Length ();

            if (distance < nearestdistance)
            {
               nearestdistance = distance; // update nearest distance
               pNewEnemy = pPlayer; // bot found a teammate needing to be healed !

               pBot->f_see_enemy_time = gpGlobals->time; // remember when we first saw this mate
               pBot->f_aim_adjust_time = gpGlobals->time; // start adjusting aim now
            }
         }
      }
   }

   // if bot is an engineer, check for players needing to be repaired
   else if (pBot->pEdict->v.playerclass == TFC_CLASS_ENGINEER)
   {
      // search the world for players...
      for (i = 1; i <= gpGlobals->maxClients; i++)
      {
         edict_t *pPlayer = INDEXENT(i);

         if (!pPlayer || pPlayer->free || (pPlayer == pBot->pEdict))
            continue; // skip invalid players and skip self (i.e. this bot)

         if (!IsAlive (pPlayer))
            continue; // skip this player if not alive (i.e. dead or dying)

         if (b_observer_mode && !(pPlayer->v.flags & FL_FAKECLIENT))
            continue; // skip real players in observater mode

         int player_team = GetTeam (pPlayer);
         int bot_team = GetTeam (pBot->pEdict);

         // don't target your enemies...
         if ((bot_team != player_team) && !(team_allies[bot_team] & (1 << player_team)))
            continue;

         // check if player needs to be repaired...
         if ((pPlayer->v.armorvalue / max_armor[pPlayer->v.playerclass]) > 0.50)
            continue; // armor greater than 50% so ignore

         vecEnd = pPlayer->v.origin + pPlayer->v.view_ofs;

         // see if bot can see the player...
         if (FInViewCone (vecEnd, pBot->pEdict) && (BotGetIdealAimVector (pBot, pPlayer) != Vector (0, 0, 0)))
         {
            float distance = (pPlayer->v.origin - pBot->pEdict->v.origin).Length ();

            if (distance < nearestdistance)
            {
               nearestdistance = distance; // update nearest distance
               pNewEnemy = pPlayer; // bot found a teammate needing to be repaired !

               pBot->f_see_enemy_time = gpGlobals->time; // remember when we first saw this mate
               pBot->f_aim_adjust_time = gpGlobals->time; // start adjusting aim now
            }
         }
      }
   }

   // ok, let's look for enemies...
   if (pNewEnemy == NULL)
   {
      // look for sentry guns first
      while ((pent = UTIL_FindEntityByClassname (pent, "building_sentrygun")) != NULL)
      {
         int sentry_team = -1;
         int bot_team = GetTeam (pBot->pEdict);

         if (pent->v.colormap == 0xA096)
            sentry_team = 0; // blue team's sentry
         else if (pent->v.colormap == 0x04FA)
            sentry_team = 1; // red team's sentry
         else if (pent->v.colormap == 0x372D)
            sentry_team = 2; // yellow team's sentry
         else if (pent->v.colormap == 0x6E64)
            sentry_team = 3; // green team's sentry

         vecEnd = pent->v.origin + pent->v.view_ofs;

         // is this sentry gun visible?
         if (FInViewCone (vecEnd, pBot->pEdict) && BotCanSeeThis (pBot, vecEnd))
         {
            float distance = (pent->v.origin - pBot->pEdict->v.origin).Length ();

            // is this the closest sentry gun?
            if (distance < nearestdistance)
            {
               // if this sentry gun is your own team's or allie's...
               if ((bot_team == sentry_team) || (team_allies[bot_team] & (1 << sentry_team)))
               {
                  // if bot is an engineer AND has enough metal AND sentry gun can be upgraded
                  if ((pBot->pEdict->v.playerclass == TFC_CLASS_ENGINEER)
                      && (pBot->m_rgAmmo[weapon_defs[TF_WEAPON_SPANNER].iAmmo1] >= 130)
                      && (pent->v.health / pent->v.max_health * 100 <= 80))
                  {
                     nearestdistance = distance; // update nearest distance
                     pNewEnemy = pent; // bot found an upgradable sentry gun !
                     pBot->f_see_enemy_time = gpGlobals->time; // save when we first saw this sentry
                     pBot->f_aim_adjust_time = gpGlobals->time; // start adjusting aim now
                     break; // don't look for anything else
                  }
                  else
                     continue; // don't target our own sentry guns...
               }

               nearestdistance = distance; // update nearest distance
               pNewEnemy = pent; // bot found an enemy's sentry gun !

               pBot->f_see_enemy_time = gpGlobals->time; // remember when we first saw this sentry
               pBot->f_aim_adjust_time = gpGlobals->time; // start adjusting aim now

               // let's spend 30 seconds for normal audio chat
               if (pBot->f_bot_sayaudio_time + 30.0 < gpGlobals->time)
               {
                  pBot->BotChat.b_sayaudio_alert = TRUE; // bot says 'alert'
                  pBot->f_bot_sayaudio_time = gpGlobals->time + RANDOM_FLOAT (0.5, 3.0);
               }
            }
         }
      }
   }

   if (pNewEnemy == NULL)
   {
      nearestdistance = 2500;

      // still no enemy, let's search the world for real players now...
      for (i = 1; i <= gpGlobals->maxClients; i++)
      {
         edict_t *pPlayer = INDEXENT (i);

         if (!pPlayer || pPlayer->free || (pPlayer == pBot->pEdict))
            continue; // skip invalid players and skip self (i.e. this bot)

         if (!IsAlive (pPlayer))
            continue; // skip this player if not alive (i.e. dead or dying)

         if (b_observer_mode && !(pPlayer->v.flags & FL_FAKECLIENT))
            continue; // skip real players in observater mode

         int player_team = GetTeam (pPlayer);
         int bot_team = GetTeam (pBot->pEdict);

         // if it is a teammate OR allie, check if this one is firing in some direction
         if ((bot_team == player_team) || (team_allies[bot_team] & (1 << player_team)))
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

         // 99 percent time bot can't notice enemy if enemy is a disguised spy
         if ((pPlayer->v.playerclass == TFC_CLASS_SPY) && (RANDOM_FLOAT (1, 100) < 99))
         {
            char color[32];
            int bot_color, player_color;

            strcpy (color, (g_engfuncs.pfnInfoKeyValue ((*g_engfuncs.pfnGetInfoKeyBuffer) (pBot->pEdict), "topcolor")));
            sscanf (color, "%d", &bot_color);

            strcpy (color, (g_engfuncs.pfnInfoKeyValue ((*g_engfuncs.pfnGetInfoKeyBuffer) (pPlayer), "topcolor")));
            sscanf (color, "%d", &player_color);

            if (((bot_color == 140) || (bot_color == 148) || (bot_color == 150) || (bot_color == 153))
                && ((player_color == 140) || (player_color == 148) || (player_color == 150) || (player_color == 153)))
               continue;

            if (((bot_color == 5) || (bot_color == 250) || (bot_color == 255))
                && ((player_color == 5) || (player_color == 250) || (player_color == 255)))
               continue;

            if ((bot_color == 45) && (player_color == 45))
               continue;

            if (((bot_color == 80) || (bot_color == 100)) && ((player_color == 80) || (player_color == 100)))
               continue;
         }

         vecEnd = pPlayer->v.origin + pPlayer->v.view_ofs;

         // see if bot can see the player...
         if (FInViewCone (vecEnd, pBot->pEdict) && (BotGetIdealAimVector (pBot, pPlayer) != Vector (0, 0, 0)))
         {
            float distance = (pPlayer->v.origin - pBot->pEdict->v.origin).Length ();

            if (distance < nearestdistance)
            {
               nearestdistance = distance; // update nearest distance
               pNewEnemy = pPlayer; // bot found an enemy !

               pBot->f_see_enemy_time = gpGlobals->time; // remember when we first saw this enemy
               pBot->f_aim_adjust_time = gpGlobals->time; // start adjusting aim now

               pBot->BotChat.b_sayaudio_alert = TRUE; // alert teammates by audio chat
               pBot->f_bot_sayaudio_time = gpGlobals->time + RANDOM_FLOAT (0.5, 2.0);
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

   // if bot is selecting the sniper rifle, switch to deadly sniper mode
   if (strcmp (item_name, "tf_weapon_sniperrifle") == 0)
   {
      pBot->f_togglesniper_time = gpGlobals->time + 0.45; // toggle sniper mode in 0.45 second
      pBot->f_shoot_time = gpGlobals->time + 0.50; // don't shoot in the meantime
   }

   FakeClientCommand (pBot->pEdict, item_name); // issue the select item command
}


void BotSwitchToBestWeapon (bot_t *pBot)
{
   bot_weapon_select_t *pSelect = &tfc_weapon_select[0];
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
          && ((pSelect[select_index].iId == TF_WEAPON_MEDIKIT)
              || (pSelect[select_index].iId == TF_WEAPON_SPANNER)
              || (pSelect[select_index].iId == TF_WEAPON_AXE)
              || (pSelect[select_index].iId == TF_WEAPON_GL)
              || (pSelect[select_index].iId == TF_WEAPON_FLAMETHROWER)
              || (pSelect[select_index].iId == TF_WEAPON_IC)
              || (pSelect[select_index].iId == TF_WEAPON_TRANQ)
              || (pSelect[select_index].iId == TF_WEAPON_PL)
              || (pSelect[select_index].iId == TF_WEAPON_KNIFE)
              || (pSelect[select_index].iId == TF_WEAPON_GRENADE)))
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
   bot_weapon_select_t *pSelect = &tfc_weapon_select[0];
   bot_fire_delay_t *pDelay = &tfc_fire_delay[0];
   bool ammo_left;
   int select_index = 0;
   float distance = v_enemy.Length (); // distance to enemy

   if (pBot->pEdict == NULL)
      return; // reliability check

   // are we charging the primary fire ?
   if (pBot->f_primary_charging > 0)
   {
      // are we charging with the sniper rifle ?
      if (pBot->charging_weapon_id == TF_WEAPON_SNIPERRIFLE)
         pBot->BotMove.f_forward_time = 0; // don't move while using sniper rifle

      // is it time to fire the charged weapon ?
      if (pBot->f_primary_charging <= gpGlobals->time)
      {
         pBot->f_primary_charging = -1; // stop charging it (-1 means not charging)

         // identify the weapon we were charging
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

      // is it the grenade launcher AND is enemy too high for us ?
      if ((pSelect[select_index].iId == TF_WEAPON_GL) && (v_enemy.z > 300))
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

         // is it the grenade launcher AND is enemy too high for us ?
         if ((pSelect[select_index].iId == TF_WEAPON_GL) && (v_enemy.z > 300))
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

   // is it the sniper rifle ?
   if (pSelect[select_index].iId == TF_WEAPON_SNIPERRIFLE)
   {
      pBot->BotMove.f_forward_time = 0; // don't move while sniping

      // are we moving too fast to use it ?
      if (pBot->pEdict->v.velocity.Length () > 50)
         return; // don't press attack key until velocity is < 50

      UTIL_MakeVectors (pBot->pEdict->v.v_angle); // build base vectors in aim direction

      // have we aimed with insufficient accuracy to fire ?
      if (UTIL_AngleOfVectors (pBot->pBotEnemy->v.origin - pBot->pEdict->v.origin, gpGlobals->v_forward)
          > (300 / (pBot->pBotEnemy->v.origin - pBot->pEdict->v.origin).Length ()))
         return; // don't shoot yet, it would be a monumental error...
   }

   // is it the RPG ?
   if (pSelect[select_index].iId == TF_WEAPON_RPG)
   {
      // is it unsafe to fire a rocket here ?
      TraceResult tr;

      UTIL_MakeVectors (pBot->pEdict->v.v_angle); // build base vectors in bot's view angle

      // check at head level to take rocket initial uplift deviation in account
      UTIL_TraceHull (pBot->pEdict->v.origin + pBot->pEdict->v.view_ofs,
                      pBot->pEdict->v.origin + pBot->pEdict->v.view_ofs + gpGlobals->v_forward * 150,
                      ignore_monsters, head_hull, pBot->pEdict->v.pContainingEntity, &tr);
      if (tr.flFraction < 1.0)
         return; // if hit something, then it is unsafe to fire here

      // check at gun level to take rocket initial right deviation in account
      UTIL_TraceLine (GetGunPosition (pBot->pEdict),
                      GetGunPosition (pBot->pEdict) + gpGlobals->v_forward * 75 + gpGlobals->v_right * 32,
                      ignore_monsters, pBot->pEdict->v.pContainingEntity, &tr);
      if (tr.flFraction < 1.0)
         return; // if hit something, then it is unsafe to fire here
   }

   // is bot a medic ?
   if (pBot->pEdict->v.playerclass == TFC_CLASS_MEDIC)
   {
      int player_team = GetTeam (pBot->pBotEnemy);
      int bot_team = GetTeam (pBot->pEdict);

      // is the "enemy" a teammate or an allie AND weapon is NOT the medikit ?
      if (((bot_team == player_team) || (team_allies[bot_team] & (1 << player_team)))
          && (pSelect[select_index].iId != TF_WEAPON_MEDIKIT))
         return; // don't press fire unless weapon is medikit
   }

   // is bot an engineer ?
   if (pBot->pEdict->v.playerclass == TFC_CLASS_ENGINEER)
   {
      int player_team = GetTeam (pBot->pBotEnemy);
      int bot_team = GetTeam (pBot->pEdict);

      // is the "enemy" a teammate or an allie AND weapon is NOT the spanner ?
      if (((bot_team == player_team) || (team_allies[bot_team] & (1 << player_team)))
          && (pSelect[select_index].iId != TF_WEAPON_SPANNER))
         return; // don't press fire unless weapon is spanner
   }

   // is it time to fire a grenade AND bot is skilled enough AND enemy distance is in range ?
   if ((pBot->f_throwgrenade_time < gpGlobals->time) && (pBot->bot_skill >= 3) && (distance > 300) && (distance < 1200))
      BotFireGrenade (pBot, 0); // fire a grenade if any available

   // if NOT in the case where it is a proximity weapon and bot is far from his enemy...
   if (!(((pSelect[select_index].iId == TF_WEAPON_MEDIKIT)
          || (pSelect[select_index].iId == TF_WEAPON_SPANNER)
          || (pSelect[select_index].iId == TF_WEAPON_AXE)
          || (pSelect[select_index].iId == TF_WEAPON_KNIFE))
         && ((pBot->pBotEnemy->v.origin - pBot->pEdict->v.origin).Length () > 50))
        || (RANDOM_LONG (1, 100) < 2))
      pBot->pEdict->v.button |= IN_ATTACK; // press the FIRE button

   // should we charge the primary fire ?
   if (pSelect[select_index].primary_fire_charge)
   {
      pBot->charging_weapon_id = pSelect[select_index].iId; // save charging weapon id
      pBot->f_primary_charging = gpGlobals->time + pSelect[select_index].primary_charge_delay;
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


void BotFireGrenade (bot_t *pBot, int grenade_type)
{
   if (pBot->pEdict == NULL)
      return; // reliability check

   pBot->f_throwgrenade_time = gpGlobals->time + RANDOM_FLOAT (3.0, 15.0); // sets new delay

   // if no grenades available OR bot is under water, give up
   if (((pBot->bot_grenades_1 == 0) && (pBot->bot_grenades_2 == 0))
       || (pBot->pEdict->v.waterlevel == 3))
      return; // can't fire a grenade

   // if no grenade type specified, choose one
   if (grenade_type == 0)
   {
      // check if bot has both types of grenades...
      if ((pBot->bot_grenades_1 > 0) && (pBot->bot_grenades_2 > 0))
      {
         // randomly choose one grenade type...
         if (RANDOM_LONG (1, 100) <= 66)
            grenade_type = 1;
         else
            grenade_type = 2;
      }

      // else select primary grenades (if any)
      else if (pBot->bot_grenades_1 > 0)
         grenade_type = 1;

      // else select secondary grenades (if any)
      else if (pBot->bot_grenades_2 > 0)
         grenade_type = 2;
   }

   if (grenade_type != 0)
   {
      pBot->BotChat.b_sayaudio_throwgrenade = TRUE; // bot says 'throwgrenade'
      pBot->f_bot_sayaudio_time = gpGlobals->time; + RANDOM_FLOAT (0.2, 0.8);

      if (grenade_type == 1)
      {
         FakeClientCommand (pBot->pEdict, "+gren1");
         FakeClientCommand (pBot->pEdict, "-gren1");
      }

      else
      {
         FakeClientCommand (pBot->pEdict, "+gren2");
         FakeClientCommand (pBot->pEdict, "-gren2");
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
      int player_team = GetTeam (pBot->pBotEnemy);
      int bot_team = GetTeam (pBot->pEdict);

      // if certain to deal with an enemy...
      if (!((bot_team == player_team) || (team_allies[bot_team] & (1 << player_team))))
      {
         // if not fearful, no sniper gun and won't fall OR proximity weapon, run to enemy
         if ((!pBot->b_is_fearful && (pBot->current_weapon.iId != TF_WEAPON_SNIPERRIFLE) && !BotCanFallForward (pBot, &tr))
            || (pBot->current_weapon.iId == TF_WEAPON_AXE)
            || (pBot->current_weapon.iId == TF_WEAPON_KNIFE)
            || (pBot->current_weapon.iId == TF_WEAPON_MEDIKIT)
            || (pBot->current_weapon.iId == TF_WEAPON_SPANNER))
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
                  && (pBot->current_weapon.iId != TF_WEAPON_SNIPERRIFLE) && (pBot->bot_skill > 2))
            pBot->BotMove.f_jump_time = gpGlobals->time + RANDOM_FLOAT (0.1, (6.0 - pBot->bot_skill) / 4); //jump

         // else if fearful and not currently strafing, duck
         else if (pBot->b_is_fearful && (pBot->BotMove.f_strafeleft_time + 0.6 < gpGlobals->time)
                  && (pBot->BotMove.f_straferight_time + 0.6 < gpGlobals->time))
         {
            pBot->BotMove.f_duck_time = gpGlobals->time + 0.8; // duck and fire
            if ((pBot->f_see_enemy_time + 5.0 < gpGlobals->time)
                || ((pBot->f_see_enemy_time + 0.5 < gpGlobals->time)
                    && ((pBot->current_weapon.iId == TF_WEAPON_AXE)
                        || (pBot->current_weapon.iId == TF_WEAPON_KNIFE)
                        || (pBot->current_weapon.iId == TF_WEAPON_MEDIKIT)
                        || (pBot->current_weapon.iId == TF_WEAPON_SPANNER))))
            pBot->b_is_fearful = FALSE; // prevent bot from never attacking
         }
      }

      // else this enemy is a teammate or a friendly sentry gun
      else
      {
         pBot->BotMove.f_forward_time = gpGlobals->time + 60.0; // run to teammate
         if (f_distance < 100)
            pBot->BotMove.b_is_walking = TRUE; // walk if getting closer
         else
            pBot->BotMove.b_is_walking = FALSE;
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
          && (pBot->pBotEnemy->v.flDuckTime <= 0))
      {
         pBot->BotMove.f_duck_time = gpGlobals->time + 0.8; // duck and fire
         if ((pBot->f_see_enemy_time + 5.0 < gpGlobals->time)
                || ((pBot->f_see_enemy_time + 0.5 < gpGlobals->time)
                    && ((pBot->current_weapon.iId == TF_WEAPON_AXE)
                        || (pBot->current_weapon.iId == TF_WEAPON_KNIFE)
                        || (pBot->current_weapon.iId == TF_WEAPON_MEDIKIT)
                        || (pBot->current_weapon.iId == TF_WEAPON_SPANNER))))
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
