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
// bot_specific.h
//

#ifndef BOT_SPECIFIC_H
#define BOT_SPECIFIC_H


// weapon ID values for Opposing Force
#define GEARBOX_WEAPON_CROWBAR 1
#define GEARBOX_WEAPON_GLOCK 2
#define GEARBOX_WEAPON_PYTHON 3
#define GEARBOX_WEAPON_MP5 4
#define GEARBOX_WEAPON_CHAINGUN 5
#define GEARBOX_WEAPON_CROSSBOW 6
#define GEARBOX_WEAPON_SHOTGUN 7
#define GEARBOX_WEAPON_RPG 8
#define GEARBOX_WEAPON_GAUSS 9
#define GEARBOX_WEAPON_EGON 10
#define GEARBOX_WEAPON_HORNETGUN 11
#define GEARBOX_WEAPON_HANDGRENADE 12
#define GEARBOX_WEAPON_TRIPMINE 13
#define GEARBOX_WEAPON_SATCHEL 14
#define GEARBOX_WEAPON_SNARK 15
#define GEARBOX_WEAPON_GRAPPLE 16
#define GEARBOX_WEAPON_EAGLE 17
#define GEARBOX_WEAPON_PIPEWRENCH 18
#define GEARBOX_WEAPON_M249 19
#define GEARBOX_WEAPON_DISPLACER 20
#define GEARBOX_WEAPON_UNKNOWN21 21
#define GEARBOX_WEAPON_SHOCKRIFLE 22
#define GEARBOX_WEAPON_SPORELAUNCHER 23
#define GEARBOX_WEAPON_SNIPERRIFLE 24
#define GEARBOX_WEAPON_KNIFE 25


// game start messages for Opposing Force
#define MSG_OPFOR_IDLE 1
#define MSG_OPFOR_TEAMSELECT_TEAMMENU 2
#define MSG_OPFOR_TEAMSELECT_CLASSMENU 3


// bot_client.cpp function prototypes
void BotClient_Gearbox_VGUI (void *p, int bot_index);
void BotClient_Gearbox_WeaponList (void *p, int bot_index);
void BotClient_Gearbox_CurrentWeapon (void *p, int bot_index);
void BotClient_Gearbox_AmmoX (void *p, int bot_index);
void BotClient_Gearbox_AmmoPickup (void *p, int bot_index);
void BotClient_Gearbox_WeaponPickup (void *p, int bot_index);
void BotClient_Gearbox_ItemPickup (void *p, int bot_index);
void BotClient_Gearbox_Health (void *p, int bot_index);
void BotClient_Gearbox_Battery (void *p, int bot_index);
void BotClient_Gearbox_Damage (void *p, int bot_index);
void BotClient_Gearbox_SayText (void *p, int bot_index);
void BotClient_Gearbox_DeathMsg (void *p, int bot_index);
void BotClient_Gearbox_ScreenFade (void *p, int bot_index);


// bot.cpp function prototypes
void BotReset (bot_t *pBot);
void BotCreate (const char *name, const char *skin, const char *logo, int nationality, int skill, int team);
bool BotCheckForSpecialZones (bot_t *pBot);
bool BotCheckForGrenades (bot_t *pBot);
void BotCheckForItems (bot_t *pBot);
void BotThink (bot_t *pBot);
void BotFindGoal (bot_t *pBot);
void BotAnswerToOrder (bot_t *pBot);
void UpdateBulletSounds (edict_t *pEdict);


// bot_start.cpp function prototypes
void BotStartGame (bot_t *pBot);


// bot_combat.cpp function prototypes
edict_t *BotCheckForEnemies (bot_t *pBot);
void BotSelectItem (bot_t *pBot, char *item_name);
void BotSwitchToBestWeapon (bot_t *pBot);
void BotFireWeapon (Vector v_enemy, bot_t *pBot, int weapon_choice);
void BotShootAtEnemy (bot_t *pBot);


#endif // BOT_SPECIFIC_H

