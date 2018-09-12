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
// GEARBOX version
//
// bot_start.cpp
//

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "bot_common.h"
#include "bot_specific.h"

extern bool is_opfor_ctf;



void BotStartGame (bot_t *pBot)
{
   char command[32];

   if (pBot->pEdict == NULL)
      return; // reliability check

   // is it Opposing Force Capture The Flag ?
   if (is_opfor_ctf)
   {
      // handle Opposing Force CTF stuff here...
      if (pBot->menu_notify == MSG_OPFOR_TEAMSELECT_TEAMMENU)
      {
         pBot->menu_notify = MSG_OPFOR_IDLE; // switch back to idle

         // select the team the bot wishes to join...
         if ((pBot->bot_team < 1) || (pBot->bot_team > 2))
         {
            // none specified, is bot forced to choose a team ?
            if (CVAR_GET_FLOAT ("racc_botforceteam") > 0)
               pBot->bot_team = CVAR_GET_FLOAT ("racc_botforceteam"); // join the specified team

            if ((pBot->bot_team < 1) || (pBot->bot_team > 2))
               pBot->bot_team = 3; // else use auto team assignment
         }

         sprintf (command, "jointeam %d", pBot->bot_team);
         FakeClientCommand (pBot->pEdict, command);
         return;
      }

      else if (pBot->menu_notify == MSG_OPFOR_TEAMSELECT_CLASSMENU)
      {
         pBot->menu_notify = MSG_OPFOR_IDLE; // switch back to idle

         // select the class the bot wishes to use...
         if ((pBot->bot_class < 0) || (pBot->bot_class > 6))
            pBot->bot_class = 7;

         sprintf (command, "selectchar %d", pBot->bot_class);
         FakeClientCommand (pBot->pEdict, command);
         pBot->b_not_started = FALSE; // bot has now joined the game (doesn't need to be started)
         return;
      }
   }

   // else if not CTF, don't need to do anything to start game...
   else
      pBot->b_not_started = FALSE; // bot has already joined the game
}

