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
// bot_specific.h
//

#ifndef BOT_SPECIFIC_H
#define BOT_SPECIFIC_H


// class ID values for Team Fortress Classic
#define TFC_CLASS_CIVILIAN 0
#define TFC_CLASS_SCOUT 1
#define TFC_CLASS_SNIPER 2
#define TFC_CLASS_SOLDIER 3
#define TFC_CLASS_DEMOMAN 4
#define TFC_CLASS_MEDIC 5
#define TFC_CLASS_HWGUY 6
#define TFC_CLASS_PYRO 7
#define TFC_CLASS_SPY 8
#define TFC_CLASS_ENGINEER 9


// weapon ID values for Team Fortress Classic
#define TF_WEAPON_UNKNOWN1 1
#define TF_WEAPON_UNKNOWN2 2
#define TF_WEAPON_MEDIKIT 3
#define TF_WEAPON_SPANNER 4
#define TF_WEAPON_AXE 5
#define TF_WEAPON_SNIPERRIFLE 6
#define TF_WEAPON_AUTORIFLE 7
#define TF_WEAPON_SHOTGUN 8
#define TF_WEAPON_SUPERSHOTGUN 9
#define TF_WEAPON_NAILGUN 10
#define TF_WEAPON_SUPERNAILGUN 11
#define TF_WEAPON_GL 12
#define TF_WEAPON_FLAMETHROWER 13
#define TF_WEAPON_RPG 14
#define TF_WEAPON_IC 15
#define TF_WEAPON_UNKNOWN16 16
#define TF_WEAPON_AC 17
#define TF_WEAPON_UNKNOWN18 18
#define TF_WEAPON_UNKNOWN19 19
#define TF_WEAPON_TRANQ 20
#define TF_WEAPON_RAILGUN 21
#define TF_WEAPON_PL 22
#define TF_WEAPON_KNIFE 23
#define TF_WEAPON_GRENADE 24 // this one doesn't actually exists, it's a hack


// game start messages for Team Fortress Classic
#define MSG_TFC_IDLE 1
#define MSG_TFC_TEAMSELECT_TEAMMENU 2
#define MSG_TFC_TEAMSELECT_CLASSMENU 3


// specific bot orders for Team Fortress Classic
#define BOT_ORDER_DETONATEPLASTIC_5SECONDS 5
#define BOT_ORDER_DETONATEPLASTIC_20SECONDS 6
#define BOT_ORDER_DISARMPLASTIC 7
#define BOT_ORDER_DISGUISEENEMY 8
#define BOT_ORDER_DISGUISEFRIENDLY 9
#define BOT_ORDER_FEIGN 10
#define BOT_ORDER_STOPFEIGN 11
#define BOT_ORDER_BUILDSENTRY 12
#define BOT_ORDER_ROTATESENTRY_180DEGREES 13
#define BOT_ORDER_ROTATESENTRY_45DEGREES 14
#define BOT_ORDER_DETONATESENTRY 15
#define BOT_ORDER_BUILDDISPENSER 16
#define BOT_ORDER_DETONATEDISPENSER 17


// bot_client.cpp function prototypes
void BotClient_TFC_VGUI (void *p, int bot_index);
void BotClient_TFC_WeaponList (void *p, int bot_index);
void BotClient_TFC_CurrentWeapon (void *p, int bot_index);
void BotClient_TFC_AmmoX (void *p, int bot_index);
void BotClient_TFC_AmmoPickup (void *p, int bot_index);
void BotClient_TFC_SecAmmoVal (void *p, int bot_index);
void BotClient_TFC_WeaponPickup (void *p, int bot_index);
void BotClient_TFC_ItemPickup (void *p, int bot_index);
void BotClient_TFC_Health (void *p, int bot_index);
void BotClient_TFC_Battery (void *p, int bot_index);
void BotClient_TFC_Damage (void *p, int bot_index);
void BotClient_TFC_SayText (void *p, int bot_index);
void BotClient_TFC_DeathMsg (void *p, int bot_index);
void BotClient_TFC_ScreenFade (void *p, int bot_index);


// bot.cpp function prototypes
void BotReset (bot_t *pBot);
void BotCreate (const char *name, const char *logo, int nationality, int skill, int team, int bot_class);
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
void BotFireGrenade (bot_t *pBot, int grenade_type);
void BotShootAtEnemy (bot_t *pBot);


#endif // BOT_SPECIFIC_H

