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
// VALVE version
//
// bot_specific.h
//

#ifndef BOT_SPECIFIC_H
#define BOT_SPECIFIC_H


// weapon ID values for Half-Life
#define VALVE_WEAPON_CROWBAR 1
#define VALVE_WEAPON_GLOCK 2
#define VALVE_WEAPON_PYTHON 3
#define VALVE_WEAPON_MP5 4
#define VALVE_WEAPON_CHAINGUN 5
#define VALVE_WEAPON_CROSSBOW 6
#define VALVE_WEAPON_SHOTGUN 7
#define VALVE_WEAPON_RPG 8
#define VALVE_WEAPON_GAUSS 9
#define VALVE_WEAPON_EGON 10
#define VALVE_WEAPON_HORNETGUN 11
#define VALVE_WEAPON_HANDGRENADE 12
#define VALVE_WEAPON_TRIPMINE 13
#define VALVE_WEAPON_SATCHEL 14
#define VALVE_WEAPON_SNARK 15


// bot_client.cpp function prototypes
void BotClient_Valve_WeaponList (void *p, int bot_index);
void BotClient_Valve_CurrentWeapon (void *p, int bot_index);
void BotClient_Valve_AmmoX (void *p, int bot_index);
void BotClient_Valve_AmmoPickup (void *p, int bot_index);
void BotClient_Valve_WeaponPickup (void *p, int bot_index);
void BotClient_Valve_ItemPickup (void *p, int bot_index);
void BotClient_Valve_Health (void *p, int bot_index);
void BotClient_Valve_Battery (void *p, int bot_index);
void BotClient_Valve_Damage (void *p, int bot_index);
void BotClient_Valve_SayText (void *p, int bot_index);
void BotClient_Valve_DeathMsg (void *p, int bot_index);
void BotClient_Valve_ScreenFade (void *p, int bot_index);


// bot.cpp function prototypes
void BotReset (bot_t *pBot);
void BotCreate (const char *name, const char *skin, const char *logo, int nationality, int skill);
bool BotCheckForSpecialZones (bot_t *pBot);
bool BotCheckForGrenades (bot_t *pBot);
void BotCheckForItems (bot_t *pBot);
void BotThink (bot_t *pBot);
void BotFindGoal (bot_t *pBot);
void BotAnswerToOrder (bot_t *pBot);
void UpdateBulletSounds (edict_t *pEdict);


// bot_combat.cpp function prototypes
edict_t *BotCheckForEnemies (bot_t *pBot);
void BotSelectItem (bot_t *pBot, char *item_name);
void BotSwitchToBestWeapon (bot_t *pBot);
void BotFireWeapon (Vector v_enemy, bot_t *pBot, int weapon_choice);
void BotShootAtEnemy (bot_t *pBot);


#endif // BOT_SPECIFIC_H

