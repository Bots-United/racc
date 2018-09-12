// RACC - AI development project for first-person shooter games derivated from Valve's Half-Life
// (http://www.racc-ai.com/)
//
// The game to engine interfacing code is based on the work done by Jeffrey 'Botman' Broome
// (http://planethalflife.com/botman/)
//
// This project is partially based on the work done by Eric Bieschke in his BSDbot
// (http://gamershomepage.com/csbot/)
//
// This project is partially based on the work done by Brendan 'Spyro' McCarthy in his ODD Bot
// (http://oddbot.hlfusion.com/)
//
// This project is partially based on the work done by Alistair 'eLiTe' Stewart in his TEAMbot
// (http://www.planethalflife.com/teambot/)
//
// This project is partially based on the work done by Johannes '@$3.1415rin' Lampel in his JoeBot
// (http://www.joebot.net/)
//
// Rational Autonomous Cybernetic Commandos AI
//
// CSTRIKE version
//
// mod_specific.cpp
//


#include "racc.h"



int GetTeam (entity_t *pPlayer)
{
   // I definitely found the game DLL interface lacked some function like this. Doh, I reminded
   // later that Half-Life was not originally meant to be a team-based game. Okay whatever, this
   // is obviously game-specific stuff, hence why we stick it into the multi-mod interface. Oh,
   // not that we forgot: the purpose of this function is to return an integer describing the
   // team the player whose entity is pointed to by pPlayer belongs to. Satisfied ?

   static char model_name[256];

   if (IsNull (pPlayer))
      return (-1); // reliability check

   strcpy (model_name, ModelOf (pPlayer)); // get player's model name

   if ((strcmp (model_name, "terror") == 0) // Phoenix Connektion
       || (strcmp (model_name, "arab") == 0) // old L337 Krew
       || (strcmp (model_name, "leet") == 0) // L337 Krew
       || (strcmp (model_name, "artic") == 0) // Artic Avenger
       || (strcmp (model_name, "guerilla") == 0)) // Gorilla Warfare
      return (CS_TERRORIST);
   else if ((strcmp (model_name, "urban") == 0) // Seal Team 6
            || (strcmp (model_name, "gsg9") == 0) // German GSG-9
            || (strcmp (model_name, "sas") == 0) // UK SAS
            || (strcmp (model_name, "gign") == 0) // French GIGN
            || (strcmp (model_name, "vip") == 0)) // VIP
      return (CS_COUNTER_TERRORIST);

   return (-1); // team not found !
}


bool ItemIsPrimary (entity_t *pItem)
{
   // this function returns TRUE if the entity pointed to by pItem is a primary weapon, FALSE
   // otherwise.

   if (IsNull (pItem))
      return FALSE; // reliability check

   return ((strcmp (ModelOf (pItem), "models/w_ak47.mdl") == 0)
           || (strcmp (ModelOf (pItem), "models/w_aug.mdl") == 0)
           || (strcmp (ModelOf (pItem), "models/w_awp.mdl") == 0)
           || (strcmp (ModelOf (pItem), "models/w_g3sg1.mdl") == 0)
           || (strcmp (ModelOf (pItem), "models/w_m249.mdl") == 0)
           || (strcmp (ModelOf (pItem), "models/w_m3.mdl") == 0)
           || (strcmp (ModelOf (pItem), "models/w_m4a1.mdl") == 0)
           || (strcmp (ModelOf (pItem), "models/w_mac10.mdl") == 0)
           || (strcmp (ModelOf (pItem), "models/w_mp5.mdl") == 0)
           || (strcmp (ModelOf (pItem), "models/w_p228.mdl") == 0)
           || (strcmp (ModelOf (pItem), "models/w_p90.mdl") == 0)
           || (strcmp (ModelOf (pItem), "models/w_scout.mdl") == 0)
           || (strcmp (ModelOf (pItem), "models/w_sg550.mdl") == 0)
           || (strcmp (ModelOf (pItem), "models/w_sg552.mdl") == 0)
           || (strcmp (ModelOf (pItem), "models/w_tmp.mdl") == 0)
           || (strcmp (ModelOf (pItem), "models/w_ump45.mdl") == 0)
           || (strcmp (ModelOf (pItem), "models/w_xm1014.mdl") == 0));
}


bool ItemIsSecondary (entity_t *pItem)
{
   // this function returns TRUE if the entity pointed to by pItem is a secondary weapon, FALSE
   // otherwise.

   if (IsNull (pItem))
      return FALSE; // reliability check

   return ((strcmp (ModelOf (pItem), "models/w_deagle.mdl") == 0)
           || (strcmp (ModelOf (pItem), "models/w_elite.mdl") == 0)
           || (strcmp (ModelOf (pItem), "models/w_fiveseven.mdl") == 0)
           || (strcmp (ModelOf (pItem), "models/w_glock18.mdl") == 0)
           || (strcmp (ModelOf (pItem), "models/w_p228.mdl") == 0)
           || (strcmp (ModelOf (pItem), "models/w_usp.mdl") == 0));
}
