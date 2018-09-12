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
// bot.cpp
//

#include "racc.h"


extern bool is_dedicated_server;
extern entity_t *pListenserverEntity;
extern int player_count;
extern bot_t bots[MAX_CLIENTS_SUPPORTED_BY_ENGINE];
extern int bot_count;
extern round_t round;
extern bot_personality_t bot_personalities[MAX_BOT_PERSONALITIES];
extern int personality_count;
extern int msec_method;
extern debug_level_t DebugLevel;



void BotReset (bot_t *pBot)
{
   pBot->msecnum = 0;
   pBot->msecdel = 0.0;
   pBot->msecval = 0.0;

   pBot->BotEyes.f_blinded_time = 0.0;

   memset (&pBot->BotBody, 0, sizeof (pBot->BotBody));

   memset (&pBot->BotLegs, 0, sizeof (pBot->BotLegs));
   pBot->BotLegs.f_max_speed = GetServerVariable ("sv_maxspeed");
   pBot->BotLegs.ideal_angles = ViewAnglesOf (pBot->pEntity);

   pBot->pBotEnemy = NULL;
   pBot->v_lastseenenemy_position = NULLVEC;
   pBot->f_see_enemy_time = 0.0;
   pBot->f_lost_enemy_time = 0.0;

   pBot->BotChat.f_saytext_time = 0.0;
   pBot->BotChat.f_sayaudio_time = 0.0;
   pBot->BotChat.b_saytext_help = FALSE;
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

   pBot->buy_state = 0;
   pBot->f_buy_time = CurrentTime () + RandomFloat (2.0, 5.0);

   pBot->v_place_to_keep = NULLVEC;
   pBot->f_place_time = 0;
   pBot->f_camp_time = 0;
   pBot->f_reach_time = 0;
   pBot->v_reach_point = NULLVEC;
   pBot->f_turncorner_time = CurrentTime () + 5.0;

   pBot->BotEars.bot_order = BOT_ORDER_NOORDER;
   pBot->BotEars.f_order_time = 0;

   memset (&pBot->BotHand, 0, sizeof (pBot->BotHand));
}


void BotCreate (void)
{
   // this is where the show begins, i.e. the function that creates a bot. How it works :
   // First I check if the bot CVARs are in range, and in case not, I set them right. Then,
   // I check which profiles are not currently in use by other bots. Third step, is to ask
   // the engine to create the fakeclient and give it a player entity pointer. And once
   // ConnectABot() has been called, ladies and gentlemen, please welcome our new bot.

   entity_t *pBotEntity;
   int personality_index, bot_index, count_used = 0;
   bool bot_names_used[MAX_BOT_PERSONALITIES];

   // first prevent some CVARs to reach forbidden values
   if (GetServerVariable ("racc_botforceteam") < 0)
      SetServerVariable ("racc_botforceteam", 0); // force racc_botforceteam in bounds
   if (GetServerVariable ("racc_botforceteam") > 2)
      SetServerVariable ("racc_botforceteam", 2); // force racc_botforceteam in bounds

   // see the names that are used and add one that is not used yet

   // reset used names flag array
   for (personality_index = 0; personality_index < personality_count; personality_index++)
      bot_names_used[personality_index] = FALSE;

   // cycle through all bot slots
   for (bot_index = 0; bot_index < MaxClientsOnServer (); bot_index++)
   {
      // is this bot active ?
      if (bots[bot_index].is_active && !IsNull (bots[bot_index].pEntity))
      {
         // cycle through all the bot personalities we know
         for (personality_index = 0; personality_index < personality_count; personality_index++)
         {
            // does the bot have the same name that this name slot ?
            if (strcmp (bot_personalities[personality_index].name, bots[bot_index].pPersonality->name) == 0)
            {
               bot_names_used[personality_index] = TRUE; // this name is used, so flag it
               count_used++; // increment the used names counter
            }
         }
      }
   }

   // if all the names are used, that's there aren't enough living bots to join
   if (count_used == personality_count)
   {
      ServerConsole_printf ("RACC: not enough people in cybernetic population!\n"); // tell why
      SetServerVariable ("racc_maxbots", bot_count); // max out the bots to the current number
      return; // ...and cancel creation
   }

   // pick up one that isn't
   do
      personality_index = RandomInteger (0, personality_count - 1);
   while (bot_names_used[personality_index]);

   // okay, now we have a name, a skin and a skill for our new bot

   // let this bot personality connect the server under its own name
   pBotEntity = ConnectABot (bot_personalities[personality_index].name);
   if (IsNull (pBotEntity))
      return; // reliability check

   bot_index = PlayerIndexOf (pBotEntity); // get the newly created bot's index in the array

   // assign the bot's "brain" to the "body" of its bot structure
   bots[bot_index].pPersonality = &bot_personalities[personality_index];
   bots[bot_index].pEntity = pBotEntity;

   // create its light entity (engine fakeclient illumination bug workaround)
   bots[bot_index].pLightEntity = CreateBotLightEntity ();

   // initialize all the variables for this bot...
   BotReset (&bots[bot_index]); // reset our bot for new round

   bots[bot_index].bot_team = GetServerVariable ("racc_botforceteam"); // set its team in case forced
   bots[bot_index].bot_class = -1; // set its class to auto-assign

   // say hello here
   if (RandomInteger (1, 100) <= (86 - 2 * player_count))
   {
      bots[bot_index].BotChat.b_saytext_hello = TRUE;
      bots[bot_index].BotChat.f_saytext_time = CurrentTime () + RandomFloat (1.0, 2.0);
   }
}


void BotPreThink (bot_t *pBot)
{
   // this function is the first called of the three Think() routines for bots. Since bots
   // aren't monsters for the Half-Life engine, they don't have a Think() function called
   // for them. That's why we have to force them to do so, by hand, each frame. In the MOD's
   // StartFrame() function (in dll.cpp), we loop through all the bots in the bots array and
   // call successively the three functions that make a monster think by the engine side :
   // PreThink(), then Think(), then PostThink(). This one is the first of the three, and its
   // purpose is to set/reset some states in the bot's structure, such as the weapons, and to
   // make it sense its environment, following the human way of reasoning : sense, think, act.

   if (IsNull (pBot->pEntity))
      return; // reliability check

   pBot->BotLegs.movement_speed = vector (0, 0, 0); // initially reset movement speed
   pBot->b_is_stuck = FALSE; // reset stuck state
   pBot->BotHand.weapons = WeaponsOf (pBot->pEntity); // update bot's weapons list

   BotSee (pBot); // make bot see
   BotHear (pBot); // make bot hear
   BotTouch (pBot); // make bot touch
}


void BotThink (bot_t *pBot)
{
   // here is the Think() function of the bots. It is the second of the cycle of the three
   // (PreThink, Think, PostThink) functions that make any AI-animated entity behave in
   // Half-Life. Here it's time for the bot to decide what to do this frame. Either it has
   // not joined the game yet, if so we make it select its team and class, either it is dead,
   // and if so we make it wait patiently for the next round, or it's just playing, and it's
   // worth the minding about what to do, if so ! In all cases, keep an eye on the chat room.

   if (IsNull (pBot->pEntity))
      return; // reliability check

   // if the bot sees a menu for selecting team and class, do select a team and a class
   if (pBot->BotEyes.BotHUD.menu_notify != MSG_CS_IDLE)
   {
      BotSelectTeamAndClass (pBot);
      return;
   }

   // keep an eye on the chat room
   BotChat (pBot);

   // if the bot is dead, wait for next round...
   if (!IsAlive (pBot->pEntity))
      return;

   // else this bot is actually really playing, so let's decide what to do right now...
   else
      BotDecideWhatToDoNow (pBot);

   return;
}


void BotPostThink (bot_t *pBot)
{
   // here's the final episode of the Think() trilogy. What do we do after thinking ? Action.
   // Chatting, speaking, pointing the gun where it wants to look, pressing the buttons for
   // moving, is what we make the bot do here. Optionally (but not that optionally after all),
   // we record some useful data about the current state of the bot, like its current position
   // and view angles, for simulating reaction time. And finally, we ask the engine to perform
   // the movement of the bot : that's the place where we call the famous RunPlayerMove().

   if (IsNull (pBot->pEntity))
      return; // reliability check

   BotSayText (pBot); // make bot type text if needed
   BotSayAudio (pBot); // make bot open its mouth if needed

   BotPointGun (pBot); // make bot aim at where it should look

   // if freeze time is elapsed...
   if (CurrentTime () > round.f_start_time + GetServerVariable ("mp_freezetime"))
      BotMove (pBot); // make bot press the keys for movement

   // save some data about the current state of the bot
   pBot->v_prev_position = OriginOf (pBot->pEntity); // save position (for checking if stuck)

   // issue the bot movement
   ComputeBotMsecVal (pBot); // calculate the msec delay for bot movement
   pBot->f_prev_playermove_time = CurrentTime (); // save date at which the last RunPlayerMove call happened

   PerformPlayerMovement (pBot->pEntity, pBot->pLightEntity, pBot->BotLegs.movement_speed, pBot->BotLegs.input_buttons, pBot->msecval);
}


void BotDecideWhatToDoNow (bot_t *pBot)
{
/*   entity_t *pEntity;
   int entity_index;*/

   if (IsNull (pBot->pEntity))
      return; // reliability check

   // the commented code below was added as an example on how to get a bot running through
   // the level. It won't work unless you have a BotReachPosition() function correctly written.

/*   // is the listen server player alive ?
   if (!IsNull (pListenserverEntity) && IsAlive (pListenserverEntity))
   {
      // check if the bot sees the listen server player
      entity_index = 0;
      while (!IsNull (pEntity = pBot->BotEyes.pEntitiesInSight[entity_index++]))
         if (pEntity == pListenserverEntity)
            break;

      if (!IsNull (pEntity))
         BotReachPosition (pBot, OriginOf (pEntity));
   }*/

   return;
}


bool BotItemIsInteresting (bot_t *pBot, entity_t *pItem)
{
   if (IsNull (pBot->pEntity) || IsNull (pItem))
      return FALSE; // reliability check

   // if bot has no primary weapon or little ammo or no ammo left and this is a primary weapon...
   if ((!BotHasPrimary (pBot)
        || (BotHoldsPrimary (pBot) && (pBot->BotHand.iAmmo1 < 8))
        || (BotHasPrimary (pBot) && BotHoldsSecondary (pBot)))
       && ItemIsPrimary (pItem))
      return TRUE; // this item is really interesting

   // if bot has no secondary weapon or little ammo and this is a secondary weapon...
   if ((!BotHasSecondary (pBot)
        || (BotHoldsSecondary (pBot) && (pBot->BotHand.iAmmo1 < 8)))
       && ItemIsSecondary (pItem))
      return TRUE; // this item is really interesting

   return FALSE; // all other stuff may not be interesting
}


void BotDiscardStuffInOrderToGetItem (bot_t *pBot, entity_t *pItem)
{
   if (IsNull (pBot->pEntity) || IsNull (pItem))
      return; // reliability check

   // if bot is wanting to pick up a primary weapon and needs to discard one to do so...
   if (ItemIsPrimary (pItem) && BotHasPrimary (pBot))
   {
      // if the bot is not currently holding his primary weapon, select it
      if (!BotHoldsPrimary (pBot))
      {
         if (pBot->BotHand.weapons & (1 << CS_WEAPON_AK47))
            FakeClientCommand (pBot->pEntity, "weapon_ak47");
         else if (pBot->BotHand.weapons & (1 << CS_WEAPON_AUG))
            FakeClientCommand (pBot->pEntity, "weapon_aug");
         else if (pBot->BotHand.weapons & (1 << CS_WEAPON_AWP))
            FakeClientCommand (pBot->pEntity, "weapon_awp");
         else if (pBot->BotHand.weapons & (1 << CS_WEAPON_G3SG1))
            FakeClientCommand (pBot->pEntity, "weapon_g3sg1");
         else if (pBot->BotHand.weapons & (1 << CS_WEAPON_M249))
            FakeClientCommand (pBot->pEntity, "weapon_m249");
         else if (pBot->BotHand.weapons & (1 << CS_WEAPON_M3))
            FakeClientCommand (pBot->pEntity, "weapon_m3");
         else if (pBot->BotHand.weapons & (1 << CS_WEAPON_M4A1))
            FakeClientCommand (pBot->pEntity, "weapon_m4a1");
         else if (pBot->BotHand.weapons & (1 << CS_WEAPON_MAC10))
            FakeClientCommand (pBot->pEntity, "weapon_mac10");
         else if (pBot->BotHand.weapons & (1 << CS_WEAPON_MP5NAVY))
            FakeClientCommand (pBot->pEntity, "weapon_mp5navy");
         else if (pBot->BotHand.weapons & (1 << CS_WEAPON_P228))
            FakeClientCommand (pBot->pEntity, "weapon_p228");
         else if (pBot->BotHand.weapons & (1 << CS_WEAPON_P90))
            FakeClientCommand (pBot->pEntity, "weapon_p90");
         else if (pBot->BotHand.weapons & (1 << CS_WEAPON_SCOUT))
            FakeClientCommand (pBot->pEntity, "weapon_scout");
         else if (pBot->BotHand.weapons & (1 << CS_WEAPON_SG550))
            FakeClientCommand (pBot->pEntity, "weapon_sg550");
         else if (pBot->BotHand.weapons & (1 << CS_WEAPON_SG552))
            FakeClientCommand (pBot->pEntity, "weapon_sg552");
         else if (pBot->BotHand.weapons & (1 << CS_WEAPON_TMP))
            FakeClientCommand (pBot->pEntity, "weapon_tmp");
         else if (pBot->BotHand.weapons & (1 << CS_WEAPON_UMP45))
            FakeClientCommand (pBot->pEntity, "weapon_ump45");
         else if (pBot->BotHand.weapons & (1 << CS_WEAPON_XM1014))
            FakeClientCommand (pBot->pEntity, "weapon_xm1014");
      }
      else
         FakeClientCommand (pBot->pEntity, "drop"); // discard primary weapon
   }

   // else if the bot wants to pick up a secondary weapon...
   else if (ItemIsSecondary (pItem))
   {
      // if the bot is not currently holding his secondary weapon, select it
      if (!BotHoldsSecondary (pBot))
      {
         if (pBot->BotHand.weapons & (1 << CS_WEAPON_DEAGLE))
            FakeClientCommand (pBot->pEntity, "weapon_deagle");
         else if (pBot->BotHand.weapons & (1 << CS_WEAPON_ELITE))
            FakeClientCommand (pBot->pEntity, "weapon_elite");
         else if (pBot->BotHand.weapons & (1 << CS_WEAPON_FIVESEVEN))
            FakeClientCommand (pBot->pEntity, "weapon_fiveseven");
         else if (pBot->BotHand.weapons & (1 << CS_WEAPON_GLOCK18))
            FakeClientCommand (pBot->pEntity, "weapon_glock18");
         else if (pBot->BotHand.weapons & (1 << CS_WEAPON_P228))
            FakeClientCommand (pBot->pEntity, "weapon_p228");
         else if (pBot->BotHand.weapons & (1 << CS_WEAPON_USP))
            FakeClientCommand (pBot->pEntity, "weapon_usp");
      }
      else
         FakeClientCommand (pBot->pEntity, "drop"); // discard secondary weapon
   }
}


void BotTalkOnTheRadio (bot_t *pBot, char *radiomsg)
{
   // this function makes the bot use Counter-Strike's radio messages UI ; since it's about
   // pre-recorded stuff, that's not quite chat, that's why definitely I couldn't put it
   // in bot_chat.cpp. It's more about making an action than chatting, actually...

   if (IsNull (pBot->pEntity))
      return; // reliability check

   // browse down Counter-Strike radio menu selections
   FakeClientCommand (pBot->pEntity, radiomsg);
}


bool BotHasPrimary (bot_t *pBot)
{
   return ((pBot->BotHand.weapons & (1 << CS_WEAPON_AK47))
           || (pBot->BotHand.weapons & (1 << CS_WEAPON_AUG))
           || (pBot->BotHand.weapons & (1 << CS_WEAPON_AWP))
           || (pBot->BotHand.weapons & (1 << CS_WEAPON_G3SG1))
           || (pBot->BotHand.weapons & (1 << CS_WEAPON_M249))
           || (pBot->BotHand.weapons & (1 << CS_WEAPON_M3))
           || (pBot->BotHand.weapons & (1 << CS_WEAPON_M4A1))
           || (pBot->BotHand.weapons & (1 << CS_WEAPON_MAC10))
           || (pBot->BotHand.weapons & (1 << CS_WEAPON_MP5NAVY))
           || (pBot->BotHand.weapons & (1 << CS_WEAPON_P228))
           || (pBot->BotHand.weapons & (1 << CS_WEAPON_P90))
           || (pBot->BotHand.weapons & (1 << CS_WEAPON_SCOUT))
           || (pBot->BotHand.weapons & (1 << CS_WEAPON_SG550))
           || (pBot->BotHand.weapons & (1 << CS_WEAPON_SG552))
           || (pBot->BotHand.weapons & (1 << CS_WEAPON_TMP))
           || (pBot->BotHand.weapons & (1 << CS_WEAPON_UMP45))
           || (pBot->BotHand.weapons & (1 << CS_WEAPON_XM1014)));
}


bool BotHasSecondary (bot_t *pBot)
{
   return ((pBot->BotHand.weapons & (1 << CS_WEAPON_DEAGLE))
           || (pBot->BotHand.weapons & (1 << CS_WEAPON_ELITE))
           || (pBot->BotHand.weapons & (1 << CS_WEAPON_FIVESEVEN))
           || (pBot->BotHand.weapons & (1 << CS_WEAPON_GLOCK18))
           || (pBot->BotHand.weapons & (1 << CS_WEAPON_P228))
           || (pBot->BotHand.weapons & (1 << CS_WEAPON_USP)));
}


bool BotHoldsPrimary (bot_t *pBot)
{
   return ((pBot->BotHand.weapon_id == CS_WEAPON_AK47)
           || (pBot->BotHand.weapon_id == CS_WEAPON_AUG)
           || (pBot->BotHand.weapon_id == CS_WEAPON_AWP)
           || (pBot->BotHand.weapon_id == CS_WEAPON_G3SG1)
           || (pBot->BotHand.weapon_id == CS_WEAPON_M249)
           || (pBot->BotHand.weapon_id == CS_WEAPON_M3)
           || (pBot->BotHand.weapon_id == CS_WEAPON_M4A1)
           || (pBot->BotHand.weapon_id == CS_WEAPON_MAC10)
           || (pBot->BotHand.weapon_id == CS_WEAPON_MP5NAVY)
           || (pBot->BotHand.weapon_id == CS_WEAPON_P228)
           || (pBot->BotHand.weapon_id == CS_WEAPON_P90)
           || (pBot->BotHand.weapon_id == CS_WEAPON_SCOUT)
           || (pBot->BotHand.weapon_id == CS_WEAPON_SG550)
           || (pBot->BotHand.weapon_id == CS_WEAPON_SG552)
           || (pBot->BotHand.weapon_id == CS_WEAPON_TMP)
           || (pBot->BotHand.weapon_id == CS_WEAPON_UMP45)
           || (pBot->BotHand.weapon_id == CS_WEAPON_XM1014));
}


bool BotHoldsSecondary (bot_t *pBot)
{
   return ((pBot->BotHand.weapon_id == CS_WEAPON_DEAGLE)
           || (pBot->BotHand.weapon_id == CS_WEAPON_ELITE)
           || (pBot->BotHand.weapon_id == CS_WEAPON_FIVESEVEN)
           || (pBot->BotHand.weapon_id == CS_WEAPON_GLOCK18)
           || (pBot->BotHand.weapon_id == CS_WEAPON_P228)
           || (pBot->BotHand.weapon_id == CS_WEAPON_USP));
}


void BotSelectTeamAndClass (bot_t *pBot)
{
   // the purpose of this function is to make the bot browse down through the team and class
   // selection menus of Counter-Strike, making it select the appropriate class according to
   // its profile loaded from the "profiles.cfg" file.

   if (IsNull (pBot->pEntity))
      return; // reliability check

   // is the team selection menu currently displayed on the bot's HUD ?
   if (pBot->BotEyes.BotHUD.menu_notify == MSG_CS_TEAMSELECT_MAINMENU)
   {
      pBot->BotEyes.BotHUD.menu_notify = MSG_CS_IDLE; // switch back to idle

      // select the team the bot wishes to join...
      if ((pBot->bot_team < 1) || (pBot->bot_team > 2))
      {
         // none specified, is bot forced to choose a team ?
         if (GetServerVariable ("racc_botforceteam") > 0)
            pBot->bot_team = GetServerVariable ("racc_botforceteam"); // join the specified team

         if ((pBot->bot_team < 1) || (pBot->bot_team > 2))
            pBot->bot_team = 5; // else use auto team assignment
      }

      // issue the command for choosing the team
      FakeClientCommand (pBot->pEntity, "menuselect %d\n", pBot->bot_team);
      pBot->bot_team = 5; // next try, if that didn't work, use auto-select
   }

   // else is the class selection menu currently displayed on the bot's HUD ?
   else if ((pBot->BotEyes.BotHUD.menu_notify == MSG_CS_TEAMSELECT_COUNTERMENU)
            || (pBot->BotEyes.BotHUD.menu_notify == MSG_CS_TEAMSELECT_TERRMENU))
   {
      pBot->BotEyes.BotHUD.menu_notify = MSG_CS_IDLE; // switch back to idle

      // select the class the bot wishes to use, if invalid use auto-select
      if ((pBot->bot_class < 1) || (pBot->bot_class > 4))
         pBot->bot_class = 5;

      // issue the command for choosing the class
      FakeClientCommand (pBot->pEntity, "menuselect %d\n", pBot->bot_class);
   }

   return;
}


void BotBuyStuff (bot_t *pBot)
{
   // this function makes the bot buy some stuff before starting the game. It is highly
   // perfectible, and, that's obvious, not designed the right way. I just wanted to provide
   // a simple routine for the bots to frag each other with other weapons than the knife and
   // pistol ; else it's just plain boring. Weapon buying should be done according to the bot's
   // desire and duty, the map layout, and a real tactical analysis of the money business.
   // Feel free to rewrite it the way it should be ! :-)

   // these weapon arrays MUST be sorted by INCREASING PRICE ORDER.
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

   if (IsNull (pBot->pEntity))
      return; // reliability check

   // check if bot is VIP
   if (strcmp (ModelOf (pBot->pEntity), "vip") == 0)
   {
      pBot->f_buy_time = 0; // don't buy anything (VIPs don't buy stuff)
      return;
   }

   // state 0: if armor is damaged and bot has some money, buy some armor
   if (pBot->buy_state == 0)
   {
      if ((ArmorPercentageOf (pBot->pEntity) < 80) && (pBot->bot_money > 1000))
      {
         // if bot is rich, buy kevlar + helmet, else buy a single kevlar
         if (pBot->bot_money > 3000)
            FakeClientCommand (pBot->pEntity, BUY_EQUIPMENT_KEVLARHELMET);
         else
            FakeClientCommand (pBot->pEntity, BUY_EQUIPMENT_KEVLAR);

         pBot->f_buy_time = CurrentTime () + RandomFloat (0.3, 0.5); // delay next buy
      }

      pBot->buy_state++; // increment state machine state
      return;
   }

   // state 1: if no primary weapon and bot has some money, buy a better primary weapon
   else if (pBot->buy_state == 1)
   {
      if (!BotHasPrimary (pBot) && (pBot->bot_money > 1250))
      {
         char index = 0, weapon_index = 0;

         // set the index at the position of the first gun bot can't buy
         while ((primary_weapon_prices[index][1] < pBot->bot_money) && (index < 15))
            index++;

         // now pick up one randomly in the affordable zone of the array
         weapon_index = RandomInteger (0, index - 1);

         // handle each case separately
         if (primary_weapon_prices[weapon_index][0] == CS_WEAPON_M3)
            FakeClientCommand (pBot->pEntity, BUY_SHOTGUN_M3); // buy the M3
         else if (primary_weapon_prices[weapon_index][0] == CS_WEAPON_XM1014)
            FakeClientCommand (pBot->pEntity, BUY_SHOTGUN_XM1014); // buy the XM1014
         else if (primary_weapon_prices[weapon_index][0] == CS_WEAPON_MP5NAVY)
            FakeClientCommand (pBot->pEntity, BUY_SUBMACHINEGUN_MP5NAVY); // buy the MP5NAVY
         else if (primary_weapon_prices[weapon_index][0] == CS_WEAPON_TMP)
            FakeClientCommand (pBot->pEntity, BUY_SUBMACHINEGUN_TMP); // buy the TMP
         else if (primary_weapon_prices[weapon_index][0] == CS_WEAPON_P90)
            FakeClientCommand (pBot->pEntity, BUY_SUBMACHINEGUN_P90); // buy the P90
         else if (primary_weapon_prices[weapon_index][0] == CS_WEAPON_MAC10)
            FakeClientCommand (pBot->pEntity, BUY_SUBMACHINEGUN_MAC10); // buy the MAC10
         else if (primary_weapon_prices[weapon_index][0] == CS_WEAPON_UMP45)
            FakeClientCommand (pBot->pEntity, BUY_SUBMACHINEGUN_UMP45); // buy the UMP45
         else if (primary_weapon_prices[weapon_index][0] == CS_WEAPON_AK47)
            FakeClientCommand (pBot->pEntity, BUY_RIFLE_AK47); // buy the AK47
         else if (primary_weapon_prices[weapon_index][0] == CS_WEAPON_SG552)
            FakeClientCommand (pBot->pEntity, BUY_RIFLE_SG552); // buy the SG552
         else if (primary_weapon_prices[weapon_index][0] == CS_WEAPON_M4A1)
            FakeClientCommand (pBot->pEntity, BUY_RIFLE_M4A1); // buy the M4A1
         else if (primary_weapon_prices[weapon_index][0] == CS_WEAPON_AUG)
            FakeClientCommand (pBot->pEntity, BUY_RIFLE_AUG); // buy the AUG
         else if (primary_weapon_prices[weapon_index][0] == CS_WEAPON_SCOUT)
            FakeClientCommand (pBot->pEntity, BUY_RIFLE_SCOUT); // buy the SCOUT
         else if (primary_weapon_prices[weapon_index][0] == CS_WEAPON_AWP)
            FakeClientCommand (pBot->pEntity, BUY_RIFLE_AWP); // buy the AWP
         else if (primary_weapon_prices[weapon_index][0] == CS_WEAPON_G3SG1)
            FakeClientCommand (pBot->pEntity, BUY_RIFLE_G3SG1); // buy the G3SG1
         else if (primary_weapon_prices[weapon_index][0] == CS_WEAPON_SG550)
            FakeClientCommand (pBot->pEntity, BUY_RIFLE_SG550); // buy the AK47
         else if (primary_weapon_prices[weapon_index][0] == CS_WEAPON_M249)
            FakeClientCommand (pBot->pEntity, BUY_MACHINEGUN_M249); // buy the M249

         pBot->f_buy_time = CurrentTime () + RandomFloat (0.3, 0.5); // delay next buy
      }

      pBot->buy_state++; // increment state machine state
      return;
   }

   // states 2, 3, 4: if bot has still some money, buy more primary ammo
   else if ((pBot->buy_state == 2) || (pBot->buy_state == 3) || (pBot->buy_state == 4))
   {
      if (pBot->bot_money > 300)
      {
         FakeClientCommand (pBot->pEntity, "buyammo1"); // buy some primary ammo
         pBot->f_buy_time = CurrentTime () + 0.3; // delay next buy
      }

      pBot->buy_state++; // increment state machine state
      return;
   }

   // state 5: if bot has still some money, buy a better secondary weapon
   else if (pBot->buy_state == 5)
   {
      if (pBot->bot_money > 1200)
      {
         char index = 0, weapon_index = 0;

         // set the index at the position of the first gun bot can't buy
         while ((secondary_weapon_prices[index][1] < pBot->bot_money) && (index < 3))
            index++;

         // now pick up one randomly in the array "buyable" zone
         weapon_index = RandomInteger (0, index - 1);

         // handle each case separately
         if (secondary_weapon_prices[weapon_index][0] == CS_WEAPON_DEAGLE)
            FakeClientCommand (pBot->pEntity, BUY_PISTOL_DEAGLE); // buy the DEAGLE
         else if (secondary_weapon_prices[weapon_index][0] == CS_WEAPON_P228)
            FakeClientCommand (pBot->pEntity, BUY_PISTOL_P228); // buy the P228
         else if (secondary_weapon_prices[weapon_index][0] == CS_WEAPON_ELITE)
            FakeClientCommand (pBot->pEntity, BUY_PISTOL_ELITE); // buy the ELITE
         else if (secondary_weapon_prices[weapon_index][0] == CS_WEAPON_FIVESEVEN)
            FakeClientCommand (pBot->pEntity, BUY_PISTOL_FIVESEVEN); // buy the FIVESEVEN

         pBot->f_buy_time = CurrentTime () + RandomFloat (0.3, 0.5); // delay next buy
      }

      pBot->buy_state++; // increment state machine state
      return;
   }

   // states 6 and 7: if bot has still some money, buy more secondary ammo
   else if ((pBot->buy_state == 6) || (pBot->buy_state == 7))
   {
      if (pBot->bot_money > 300)
      {
         FakeClientCommand (pBot->pEntity, "buyammo2"); // buy some secondary ammo
         pBot->f_buy_time = CurrentTime () + 0.3; // delay next buy
      }

      pBot->buy_state++; // increment state machine state
      return;
   }

   // state 8: if bot has still some money, choose if bot should buy a grenade or not
   else if (pBot->buy_state == 8)
   {
      if ((pBot->bot_money > 600) && (RandomInteger (1, 100) < 66))
      {
         if (RandomInteger (0, 100) < 66)
            FakeClientCommand (pBot->pEntity, BUY_EQUIPMENT_HEGRENADE); // buy HE grenade
         else if (RandomInteger (0, 100) < 66)
            FakeClientCommand (pBot->pEntity, BUY_EQUIPMENT_FLASHBANG); // buy flashbang
         else
            FakeClientCommand (pBot->pEntity, BUY_EQUIPMENT_SMOKEGRENADE); // buy smokegrenade

         pBot->f_buy_time = CurrentTime () + RandomFloat (0.3, 0.5); // delay next buy
      }

      pBot->buy_state++; // increment state machine state
      return;
   }

   // state 9: if bot is counter-terrorist and we're on a bomb map, randomly buy the defuse kit
   else if (pBot->buy_state == 9)
   {
      if ((GetTeam (pBot->pEntity) == CS_COUNTER_TERRORIST)
          && (pBot->BotEyes.BotHUD.defuser_icon_state == HUD_ICON_OFF)
          && (pBot->bot_money > 200) && RandomInteger (1, 100) < 33)
      {
         FakeClientCommand (pBot->pEntity, BUY_EQUIPMENT_DEFUSEKIT); // buy the defuse kit
         pBot->f_buy_time = CurrentTime () + RandomFloat (0.3, 0.5); // delay next buy
      }

      pBot->buy_state++; // increment state machine state
      return;
   }
}


void ComputeBotMsecVal (bot_t *pBot)
{
   // the purpose of this function is to compute, according to the specified computation
   // method, the msec value which will be passed as an argument of pfnRunPlayerMove. This
   // function is called every frame for every bot, since the RunPlayerMove is the function
   // that tells the engine to put the bot character model in movement. This msec value
   // tells the engine how long should the movement of the model extend inside the current
   // frame. It is very important for it to be exact, else one can experience bizarre
   // problems, such as bots getting stuck into each others. That's because the model's
   // bounding boxes, which are the boxes the engine uses to compute and detect all the
   // collisions of the model, only exist, and are only valid, while in the duration of the
   // movement. That's why if you get a pfnRunPlayerMove for one bot that lasts a little too
   // short in comparison with the frame's duration, the remaining time until the frame
   // elapses, that bot will behave like a ghost : no movement, but bullets and players can
   // pass through it. Then, when the next frame will begin, the stucking problem will arise !

   if (IsNull (pBot->pEntity))
      return; // reliability check

   switch (msec_method)
   {
      default:
         msec_method = METHOD_LEON; // use Leon Hartwig's (from Valve Software) method by default

      case (METHOD_PM):
         // this is Pierre-Marie 'PieM' Baty's method for calculating the msec value.

         pBot->msecval = (int) (FrameDuration () * 1000);
         if (pBot->msecval > 255)
            pBot->msecval = 255;

         break;

      case (METHOD_RICH):
         // this is Rich 'TheFatal' Whitehouse's method for calculating the msec value,

         if (pBot->msecdel <= CurrentTime ())
         {
            if (pBot->msecnum > 0)
               pBot->msecval = 450.0 / pBot->msecnum;

            pBot->msecdel = CurrentTime () + 0.5; // next check in half a second
            pBot->msecnum = 0;
         }
         else
            pBot->msecnum++;

         if (pBot->msecval < 1)
            pBot->msecval = 1; // don't allow the msec delay to be null
         else if (pBot->msecval > 100)
            pBot->msecval = 100; // don't allow it to last longer than 100 milliseconds either

         break;

      case (METHOD_LEON):
         // this is Leon 'Jehannum' Hartwig's method for calculating the msec value,

         pBot->msecval = (int) ((CurrentTime () - pBot->f_prev_playermove_time) * 1000);
         if (pBot->msecval > 255)
            pBot->msecval = 255;

         break;

      case (METHOD_TOBIAS):
         // this is Tobias 'Killaruna' Heimann's method for calculating the msec value,

         if ((pBot->msecdel + pBot->msecnum / 1000) < CurrentTime () - 0.5)
         {
            pBot->msecdel = CurrentTime () - 0.05; // after pause
            pBot->msecnum = 0;
         }

         if (pBot->msecdel > CurrentTime ())
         {
            pBot->msecdel = CurrentTime () - 0.05; // after map changes
            pBot->msecnum = 0;
         }

         pBot->msecval = (CurrentTime () - pBot->msecdel) * 1000 - pBot->msecnum; // optimal msec value since start of 1 sec period
         pBot->msecnum = (CurrentTime () - pBot->msecdel) * 1000; // value ve have to add to reach optimum

         // do we have to start a new 1 sec period ?
         if (pBot->msecnum > 1000)
         {
            pBot->msecdel += pBot->msecnum / 1000;
            pBot->msecnum = 0;
         }

         if (pBot->msecval < 5)
            pBot->msecval = 5; // don't allow the msec delay to be too low
         else if (pBot->msecval > 255)
            pBot->msecval = 255; // don't allow it to last longer than 255 milliseconds either

         break;
   }

   return;
}
