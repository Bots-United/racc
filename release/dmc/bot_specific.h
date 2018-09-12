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
// DMC version
//
// bot_specific.h
//

#ifndef BOT_SPECIFIC_H
#define BOT_SPECIFIC_H


// weapon ID values for Deathmatch Classic
#define DMC_WEAPON_AXE 1
#define DMC_WEAPON_QUAKEGUN 2
#define DMC_WEAPON_SUPERSHOTGUN 4
#define DMC_WEAPON_NAILGUN 8
#define DMC_WEAPON_SUPERNAILGUN 16
#define DMC_WEAPON_ROCKETLAUNCHER 32
#define DMC_WEAPON_GRENADELAUNCHER 64
#define DMC_WEAPON_LIGHTNING 128


// bot_client.cpp function prototypes
void BotClient_Dmc_WeaponList (void *p, int bot_index);
void BotClient_Dmc_CurrentWeapon (void *p, int bot_index);
void BotClient_Dmc_AmmoX (void *p, int bot_index);
void BotClient_Dmc_AmmoPickup (void *p, int bot_index);
void BotClient_Dmc_WeaponPickup (void *p, int bot_index);
void BotClient_Dmc_ItemPickup (void *p, int bot_index);
void BotClient_Dmc_Health (void *p, int bot_index);
void BotClient_Dmc_Battery (void *p, int bot_index);
void BotClient_Dmc_Damage (void *p, int bot_index);
void BotClient_Dmc_SayText (void *p, int bot_index);
void BotClient_Dmc_DeathMsg (void *p, int bot_index);
void BotClient_Dmc_ScreenFade (void *p, int bot_index);


// bot.cpp function prototypes
void BotReset (bot_t *pBot);
void BotCreate (const char *name, const char *skin, const char *logo, int nationality, int skill);
bool BotCheckForGrenades (bot_t *pBot);
void BotCheckForItems (bot_t *pBot);
void BotThink (bot_t *pBot);
void BotAnswerToOrder (bot_t *pBot);
void UpdateBulletSounds (edict_t *pEdict);


// bot_combat.cpp function prototypes
edict_t *BotCheckForEnemies (bot_t *pBot);
void BotSelectItem (bot_t *pBot, int item_id);
void BotSwitchToBestWeapon (bot_t *pBot);
void BotFireWeapon (Vector v_enemy, bot_t *pBot, int weapon_choice);
void BotShootAtEnemy (bot_t *pBot);
void BotShootButton (bot_t *pBot);


#endif // BOT_SPECIFIC_H
