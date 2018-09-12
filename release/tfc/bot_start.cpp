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
// TFC version
//
// bot_start.cpp
//

#include "racc.h"


void BotStartGame (bot_t *pBot)
{
   char command[32];
   int team;
   int class_not_allowed;

   if (!IsValidPlayer (pBot->pEdict))
      return; // reliability check

   // handle Team Fortress Classic stuff here...
   if (pBot->BotEyes.BotHUD.menu_state == MENU_TFC_TEAMSELECT_TEAMMENU)
   {
      pBot->BotEyes.BotHUD.menu_state = MENU_TFC_IDLE; // switch back to idle

      // select the team the bot wishes to join...
      if ((pBot->pProfile->team < 1) || (pBot->pProfile->team > 4))
      {
         // none specified, is bot forced to choose a team ?
         if ((server.bot_forced_team[0] == 'B') || (server.bot_forced_team[0] == 'b'))
            pBot->pProfile->team = 1; // join the blue team
         else if ((server.bot_forced_team[0] == 'R') || (server.bot_forced_team[0] == 'r'))
            pBot->pProfile->team = 2; // join the red team
         else if ((server.bot_forced_team[0] == 'Y') || (server.bot_forced_team[0] == 'y'))
            pBot->pProfile->team = 3; // join the yellow team
         else if ((server.bot_forced_team[0] == 'G') || (server.bot_forced_team[0] == 'g'))
            pBot->pProfile->team = 4; // join the green team

         if ((pBot->pProfile->team < 1) || (pBot->pProfile->team > 4))
            pBot->pProfile->team = 5; // else use auto team assignment
      }

      sprintf (command, "jointeam %d", pBot->pProfile->team);
      FakeClientCommand (pBot->pEdict, command);
      return;
   }

   else if (pBot->BotEyes.BotHUD.menu_state == MENU_TFC_TEAMSELECT_CLASSMENU)
   {
      pBot->BotEyes.BotHUD.menu_state = MENU_TFC_IDLE; // switch back to idle

      team = GetTeam (pBot->pEdict);

      if (team_class_limits[team] == -1) // civilian only?
         pBot->pProfile->subclass = 0; // civilian
      else
      {
         if (pBot->pProfile->subclass == 10)
            class_not_allowed = team_class_limits[team] & (1 << 7);
         else if ((pBot->pProfile->subclass > 0) && (pBot->pProfile->subclass <= 7))
            class_not_allowed = team_class_limits[team] & (1 << (pBot->pProfile->subclass - 1));
         else if ((pBot->pProfile->subclass > 7) && (pBot->pProfile->subclass <= 9))
            class_not_allowed = team_class_limits[team] & (1 << (pBot->pProfile->subclass));

         if (class_not_allowed)
            pBot->pProfile->subclass = 10; // use auto class assignment if specified class is invalid
      }

      // select the class the bot wishes to use...
      if (pBot->pProfile->subclass == 0)
         strcpy (command, "civilian");
      else if (pBot->pProfile->subclass == 1)
         strcpy (command, "scout");
      else if (pBot->pProfile->subclass == 2)
         strcpy (command, "sniper");
      else if (pBot->pProfile->subclass == 3)
         strcpy (command, "soldier");
      else if (pBot->pProfile->subclass == 4)
         strcpy (command, "demoman");
      else if (pBot->pProfile->subclass == 5)
         strcpy (command, "medic");
      else if (pBot->pProfile->subclass == 6)
         strcpy (command, "hwguy");
      else if (pBot->pProfile->subclass == 7)
         strcpy (command, "pyro");
      else if (pBot->pProfile->subclass == 8)
         strcpy (command, "spy");
      else if (pBot->pProfile->subclass == 9)
         strcpy (command, "engineer");
      else
         strcpy (command, "randompc"); // use auto class assignment if nothing specified

      FakeClientCommand (pBot->pEdict, command);
      pBot->b_not_started = FALSE; // bot has now joined the game (doesn't need to be started)
      return;
   }
}
