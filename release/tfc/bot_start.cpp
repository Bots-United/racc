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
// TFC version
//
// bot_start.cpp
//

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "bot_common.h"
#include "bot_specific.h"

extern int team_class_limits[4];


void BotStartGame (bot_t *pBot)
{
   char command[32];
   int team;
   int class_not_allowed;

   if (pBot->pEdict == NULL)
      return; // reliability check

   // handle Team Fortress Classic stuff here...
   if (pBot->menu_notify == MSG_TFC_TEAMSELECT_TEAMMENU)
   {
      pBot->menu_notify = MSG_TFC_IDLE; // switch back to idle

      // select the team the bot wishes to join...
      if ((pBot->bot_team < 1) || (pBot->bot_team > 4))
      {
         // none specified, is bot forced to choose a team ?
         if (CVAR_GET_FLOAT ("racc_botforceteam") > 0)
            pBot->bot_team = CVAR_GET_FLOAT ("racc_botforceteam"); // join the specified team

         if ((pBot->bot_team < 1) || (pBot->bot_team > 4))
            pBot->bot_team = 5; // else use auto team assignment
      }

      sprintf (command, "jointeam %d", pBot->bot_team);
      FakeClientCommand (pBot->pEdict, command);
      return;
   }

   else if (pBot->menu_notify == MSG_TFC_TEAMSELECT_CLASSMENU)
   {
      pBot->menu_notify = MSG_TFC_IDLE; // switch back to idle

      team = GetTeam (pBot->pEdict);

      if (team_class_limits[team] == -1) // civilian only?
         pBot->bot_class = 0; // civilian
      else
      {
         if (pBot->bot_class == 10)
            class_not_allowed = team_class_limits[team] & (1 << 7);
         else if ((pBot->bot_class > 0) && (pBot->bot_class <= 7))
            class_not_allowed = team_class_limits[team] & (1 << (pBot->bot_class - 1));
         else if ((pBot->bot_class > 7) && (pBot->bot_class <= 9))
            class_not_allowed = team_class_limits[team] & (1 << (pBot->bot_class));

         if (class_not_allowed)
            pBot->bot_class = 10; // use auto class assignment if specified class is invalid
      }

      // select the class the bot wishes to use...
      if (pBot->bot_class == 0)
         strcpy (command, "civilian");
      else if (pBot->bot_class == 1)
         strcpy (command, "scout");
      else if (pBot->bot_class == 2)
         strcpy (command, "sniper");
      else if (pBot->bot_class == 3)
         strcpy (command, "soldier");
      else if (pBot->bot_class == 4)
         strcpy (command, "demoman");
      else if (pBot->bot_class == 5)
         strcpy (command, "medic");
      else if (pBot->bot_class == 6)
         strcpy (command, "hwguy");
      else if (pBot->bot_class == 7)
         strcpy (command, "pyro");
      else if (pBot->bot_class == 8)
         strcpy (command, "spy");
      else if (pBot->bot_class == 9)
         strcpy (command, "engineer");
      else
         strcpy (command, "randompc"); // use auto class assignment if nothing specified

      FakeClientCommand (pBot->pEdict, command);
      pBot->b_not_started = FALSE; // bot has now joined the game (doesn't need to be started)
      return;
   }
}
