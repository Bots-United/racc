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
// bot_specific.h
//

#ifndef BOT_SPECIFIC_H
#define BOT_SPECIFIC_H


// team ID values for Counter-Strike
#define CS_TERRORIST 0
#define CS_COUNTER_TERRORIST 1


// weapon ID values for Counter-Strike
#define CS_WEAPON_P228 1
#define CS_WEAPON_UNKNOWN2 2
#define CS_WEAPON_SCOUT 3
#define CS_WEAPON_HEGRENADE 4
#define CS_WEAPON_XM1014 5
#define CS_WEAPON_C4 6
#define CS_WEAPON_MAC10 7
#define CS_WEAPON_AUG 8
#define CS_WEAPON_SMOKEGRENADE 9
#define CS_WEAPON_ELITE 10
#define CS_WEAPON_FIVESEVEN 11
#define CS_WEAPON_UMP45 12
#define CS_WEAPON_SG550 13
#define CS_WEAPON_UNKNOWN14 14
#define CS_WEAPON_UNKNOWN15 15
#define CS_WEAPON_USP 16
#define CS_WEAPON_GLOCK18 17
#define CS_WEAPON_AWP 18
#define CS_WEAPON_MP5NAVY 19
#define CS_WEAPON_M249 20
#define CS_WEAPON_M3 21
#define CS_WEAPON_M4A1 22
#define CS_WEAPON_TMP 23
#define CS_WEAPON_G3SG1 24
#define CS_WEAPON_FLASHBANG 25
#define CS_WEAPON_DEAGLE 26
#define CS_WEAPON_SG552 27
#define CS_WEAPON_AK47 28
#define CS_WEAPON_KNIFE 29
#define CS_WEAPON_P90 30


// buy menu hierarchy for Counter-Strike
#define BUY_PISTOLMENU "menuselect 1"
#define BUY_USP "menuselect 1"
#define BUY_GLOCK18 "menuselect 2"
#define BUY_DEAGLE "menuselect 3"
#define BUY_P228 "menuselect 4"
#define BUY_ELITE "menuselect 5"
#define BUY_FIVESEVEN "menuselect 6"
#define BUY_SHOTGUNMENU "menuselect 2"
#define BUY_M3 "menuselect 1"
#define BUY_XM1014 "menuselect 2"
#define BUY_SUBMACHINEGUNMENU "menuselect 3"
#define BUY_MP5NAVY "menuselect 1"
#define BUY_TMP "menuselect 2"
#define BUY_P90 "menuselect 3"
#define BUY_MAC10 "menuselect 4"
#define BUY_UMP45 "menuselect 5"
#define BUY_RIFLEMENU "menuselect 4"
#define BUY_AK47 "menuselect 1"
#define BUY_SG552 "menuselect 2"
#define BUY_M4A1 "menuselect 3"
#define BUY_AUG "menuselect 4"
#define BUY_SCOUT "menuselect 5"
#define BUY_AWP "menuselect 6"
#define BUY_G3SG1 "menuselect 7"
#define BUY_SG550 "menuselect 8"
#define BUY_MACHINEGUNMENU "menuselect 5"
#define BUY_M249 "menuselect 1"
#define BUY_PRIMARYAMMO "menuselect 6"
#define BUY_SECONDARYAMMO "menuselect 7"
#define BUY_EQUIPMENTMENU "menuselect 8"
#define BUY_KEVLAR "menuselect 1"
#define BUY_KEVLARHELMET "menuselect 2"
#define BUY_FLASHBANG "menuselect 3"
#define BUY_HEGRENADE "menuselect 4"
#define BUY_SMOKEGRENADE "menuselect 5"
#define BUY_DEFUSEKIT "menuselect 6"
#define BUY_NIGHTVISION "menuselect 7"
#define BUY_CANCEL "menuselect 0"


// radio messages definitions for Counter-Strike
#define RADIOMSG_COVERME "radio1", "menuselect 1"
#define RADIOMSG_YOUTAKETHEPOINT "radio1", "menuselect 2"
#define RADIOMSG_KEEPTHEPOSITION "radio1", "menuselect 3"
#define RADIOMSG_STICKTOGETHER "radio1", "menuselect 4"
#define RADIOMSG_FOLLOWME "radio1", "menuselect 5"
#define RADIOMSG_UNDERFIRE "radio1", "menuselect 6"
#define RADIOMSG_GOGOGO "radio2", "menuselect 1"
#define RADIOMSG_FALLBACK "radio2", "menuselect 2"
#define RADIOMSG_TAKECOVER "radio2", "menuselect 3"
#define RADIOMSG_STAYANDWAIT "radio2", "menuselect 4"
#define RADIOMSG_STORMTHEFRONT "radio2", "menuselect 5"
#define RADIOMSG_REPORTINGIN "radio2", "menuselect 6"
#define RADIOMSG_AFFIRMATIVE "radio3", "menuselect 1"
#define RADIOMSG_ENEMYSPOTTED "radio3", "menuselect 2"
#define RADIOMSG_NEEDBACKUP "radio3", "menuselect 3"
#define RADIOMSG_SECTORCLEAR "radio3", "menuselect 4"
#define RADIOMSG_INPOSITION "radio3", "menuselect 5"
#define RADIOMSG_REPORT "radio3", "menuselect 6"
#define RADIOMSG_GONNABLOW "radio3", "menuselect 7"
#define RADIOMSG_NEGATIVE "radio3", "menuselect 8"
#define RADIOMSG_ENEMYDOWN "radio3", "menuselect 9"


// game start messages for Counter-Strike
#define MSG_CS_IDLE 1
#define MSG_CS_TEAMSELECT_MAINMENU 2
#define MSG_CS_TEAMSELECT_COUNTERMENU 3
#define MSG_CS_TEAMSELECT_TERRMENU 4
#define MSG_CS_BUY_MAINMENU 5
#define MSG_CS_BUY_PISTOLMENU 6
#define MSG_CS_BUY_SHOTGUNMENU 7
#define MSG_CS_BUY_RIFLEMENU 8
#define MSG_CS_BUY_SUBMACHINEGUNMENU 9
#define MSG_CS_BUY_MACHINEGUNMENU 10
#define MSG_CS_BUY_EQUIPMENTMENU 11


// bot_client.cpp function prototypes
void BotClient_CS_VGUI (void *p, int bot_index);
void BotClient_CS_ShowMenu (void *p, int bot_index);
void BotClient_CS_WeaponList (void *p, int bot_index);
void BotClient_CS_CurrentWeapon (void *p, int bot_index);
void BotClient_CS_AmmoX (void *p, int bot_index);
void BotClient_CS_AmmoPickup (void *p, int bot_index);
void BotClient_CS_WeaponPickup (void *p, int bot_index);
void BotClient_CS_ItemPickup (void *p, int bot_index);
void BotClient_CS_Health (void *p, int bot_index);
void BotClient_CS_Battery (void *p, int bot_index);
void BotClient_CS_Damage (void *p, int bot_index);
void BotClient_CS_Money (void *p, int bot_index);
void BotClient_CS_SayText (void *p, int bot_index);
void BotClient_CS_TextMsg (void *p, int bot_index);
void BotClient_CS_TextMsgAll (void *p, int bot_index);
void BotClient_CS_DeathMsg (void *p, int bot_index);
void BotClient_CS_ScreenFade (void *p, int bot_index);
void BotClient_CS_StatusIcon (void *p, int bot_index);


// bot.cpp function prototypes
void BotReset (bot_t *pBot);
void BotCreate (const char *name, const char *logo, int nationality, int skill, int team, int bot_class);
void BotBuyStuff (bot_t *pBot);
bool BotCheckForSpecialZones (bot_t *pBot);
bool BotCheckForGrenades (bot_t *pBot);
void BotCheckForItems (bot_t *pBot);
void BotThink (bot_t *pBot);
bool BotItemIsInteresting (bot_t *pBot, edict_t *pItem);
void BotDiscardItem (bot_t *pBot, edict_t *pItem);
void BotTalkOnTheRadio (bot_t *pBot, char *radiocmd, char *radiomsg);
void BotFindGoal (bot_t *pBot);
void BotAnswerToOrder (bot_t *pBot);
bool BotHasPrimary (bot_t *pBot);
bool BotHasSecondary (bot_t *pBot);
bool BotHoldsPrimary (bot_t *pBot);
bool BotHoldsSecondary (bot_t *pBot);
bool ItemIsPrimary (edict_t *pItem);
bool ItemIsSecondary (edict_t *pItem);
void UpdateBulletSounds (edict_t *pEdict);


// bot_start.cpp function prototypes
void BotStartGame (bot_t *pBot);


// bot_combat.cpp function prototypes
edict_t *BotCheckForEnemies (bot_t *pBot);
void BotSelectItem (bot_t *pBot, char *item_name);
void BotSwitchToBestWeapon (bot_t *pBot);
void BotFireWeapon (Vector v_enemy, bot_t *pBot, int weapon_choice);
void BotShootAtEnemy (bot_t *pBot);
void BotPlantBomb (bot_t *pBot, Vector v_target);
void BotDefuseBomb (bot_t *pBot, edict_t *pBomb);


#endif // BOT_SPECIFIC_H

