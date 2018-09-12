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
// bot_start.cpp
//

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "bot_common.h"
#include "bot_specific.h"


void BotStartGame (bot_t *pBot)
{
   char command[32];

   if (pBot->pEdict == NULL)
      return; // reliability check

   // handle Counter-Strike team selection stuff here...
   if (pBot->menu_notify == MSG_CS_TEAMSELECT_MAINMENU)
   {
      pBot->menu_notify = MSG_CS_IDLE; // switch back to idle

      // select the team the bot wishes to join...
      if ((pBot->bot_team < 1) || (pBot->bot_team > 2))
      {
         // none specified, is bot forced to choose a team ?
         if (CVAR_GET_FLOAT ("racc_botforceteam") > 0)
            pBot->bot_team = CVAR_GET_FLOAT ("racc_botforceteam"); // join the specified team

         if ((pBot->bot_team < 1) || (pBot->bot_team > 2))
            pBot->bot_team = 5; // else use auto team assignment
      }

      sprintf (command, "menuselect %d", pBot->bot_team);
      FakeClientCommand (pBot->pEdict, command);
      return;
   }

   else if ((pBot->menu_notify == MSG_CS_TEAMSELECT_COUNTERMENU)
            || (pBot->menu_notify == MSG_CS_TEAMSELECT_TERRMENU))
   {
      pBot->menu_notify = MSG_CS_IDLE; // switch back to idle

      // select the class the bot wishes to use...
      if ((pBot->bot_class < 1) || (pBot->bot_class > 4))
         pBot->bot_class = 5;

      sprintf (command, "menuselect %d", pBot->bot_class);
      FakeClientCommand (pBot->pEdict, command);
      pBot->b_not_started = FALSE; // bot has now joined the game
      return;
   }
}
