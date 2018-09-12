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
// This project is partially based on the work done by Johannes '@$3.1415rin' Lampel in his JoeBot
// (http://www.joebot.net/)
//
// Rational Autonomous Cybernetic Commandos AI
//
// CSTRIKE version
//
// mod_specific.h
//

#ifndef MOD_SPECIFIC_H
#define MOD_SPECIFIC_H


// dynamic library names for Counter-Strike
#define METAMOD_LIBRARY_NAME_WIN32 "cstrike/addons/metamod/dlls/metamod.dll"
#define METAMOD_LIBRARY_NAME_UNIX "cstrike/addons/metamod/dlls/metamod_i386.so"
#define MOD_LIBRARY_NAME_WIN32 "cstrike/dlls/mp.dll"
#define MOD_LIBRARY_NAME_UNIX "cstrike/dlls/mp_i386.so"


// max player field of view size in degrees
#define MAX_PLAYER_FOV 90


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


// weapon mode values for Counter-Strike
#define CS_WEAPON_GLOCK18_BURST 0
#define CS_WEAPON_M4A1_SILENCED 0
#define CS_WEAPON_USP_SILENCED 0


// player model bone numbers for Counter-Strike
#define PLAYERBONE_PELVIS 1
#define PLAYERBONE_SPINE 2
#define PLAYERBONE_SPINE1 3
#define PLAYERBONE_SPINE2 4
#define PLAYERBONE_SPINE3 5
#define PLAYERBONE_NECK 6
#define PLAYERBONE_HEAD 7
#define PLAYERBONE_L_CLAVICLE 9
#define PLAYERBONE_L_UPPERARM 10
#define PLAYERBONE_L_FOREARM 11
#define PLAYERBONE_L_HAND 12
#define PLAYERBONE_L_FINGER0 13
#define PLAYERBONE_L_FINGER01 14
#define PLAYERBONE_L_FINGER1 15
#define PLAYERBONE_L_FINGER11 16
#define PLAYERBONE_L_THIGH 40
#define PLAYERBONE_L_CALF 41
#define PLAYERBONE_L_FOOT 42
#define PLAYERBONE_R_CLAVICLE 23
#define PLAYERBONE_R_UPPERARM 24
#define PLAYERBONE_R_FOREARM 25
#define PLAYERBONE_R_HAND 26
#define PLAYERBONE_R_FINGER0 27
#define PLAYERBONE_R_FINGER01 28
#define PLAYERBONE_R_FINGER1 29
#define PLAYERBONE_R_FINGER11 30
#define PLAYERBONE_R_THIGH 46
#define PLAYERBONE_R_CALF 47
#define PLAYERBONE_R_FOOT 48



// buy menu hierarchy for Counter-Strike
#define BUY_PISTOL_USP "buy;menuselect 1;menuselect 1"
#define BUY_PISTOL_GLOCK18 "buy;menuselect 1;menuselect 2"
#define BUY_PISTOL_DEAGLE "buy;menuselect 1;menuselect 3"
#define BUY_PISTOL_P228 "buy;menuselect 1;menuselect 4"
#define BUY_PISTOL_ELITE "buy;menuselect 1;menuselect 5"
#define BUY_PISTOL_FIVESEVEN "buy;menuselect 1;menuselect 6"
#define BUY_SHOTGUN_M3 "buy;menuselect 2;menuselect 1"
#define BUY_SHOTGUN_XM1014 "buy;menuselect 2;menuselect 2"
#define BUY_SUBMACHINEGUN_MP5NAVY "buy;menuselect 3;menuselect 1"
#define BUY_SUBMACHINEGUN_TMP "buy;menuselect 3;menuselect 2"
#define BUY_SUBMACHINEGUN_P90 "buy;menuselect 3;menuselect 3"
#define BUY_SUBMACHINEGUN_MAC10 "buy;menuselect 3;menuselect 4"
#define BUY_SUBMACHINEGUN_UMP45 "buy;menuselect 3;menuselect 5"
#define BUY_RIFLE_AK47 "buy;menuselect 4;menuselect 1"
#define BUY_RIFLE_SG552 "buy;menuselect 4;menuselect 2"
#define BUY_RIFLE_M4A1 "buy;menuselect 4;menuselect 3"
#define BUY_RIFLE_AUG "buy;menuselect 4;menuselect 4"
#define BUY_RIFLE_SCOUT "buy;menuselect 4;menuselect 5"
#define BUY_RIFLE_AWP "buy;menuselect 4;menuselect 6"
#define BUY_RIFLE_G3SG1 "buy;menuselect 4;menuselect 7"
#define BUY_RIFLE_SG550 "buy;menuselect 4;menuselect 8"
#define BUY_MACHINEGUN_M249 "buy;menuselect 5;menuselect 1"
#define BUY_PRIMARYAMMO "buyammo1"
#define BUY_SECONDARYAMMO "buyammo2"
#define BUY_EQUIPMENT_KEVLAR "buyequip;menuselect 1"
#define BUY_EQUIPMENT_KEVLARHELMET "buyequip;menuselect 2"
#define BUY_EQUIPMENT_FLASHBANG "buyequip;menuselect 3"
#define BUY_EQUIPMENT_HEGRENADE "buyequip;menuselect 4"
#define BUY_EQUIPMENT_SMOKEGRENADE "buyequip;menuselect 5"
#define BUY_EQUIPMENT_DEFUSEKIT "buyequip;menuselect 6"
#define BUY_EQUIPMENT_NIGHTVISION "buyequip;menuselect 7"


// maximum number of known local (radio) sounds in Counter-Strike
#define MAX_LOCAL_SOUNDS 45


// maximum walk speed (without making footstep noises) in Counter-Strike
#define MAX_WALK_SPEED 150


// radio messages definitions for Counter-Strike
#define RADIOMSG_COVERME "radio1;menuselect 1"
#define RADIOMSG_YOUTAKETHEPOINT "radio1;menuselect 2"
#define RADIOMSG_HOLDTHISPOSITION "radio1;menuselect 3"
#define RADIOMSG_STICKTOGETHERTEAM "radio1;menuselect 4"
#define RADIOMSG_FOLLOWME "radio1;menuselect 5"
#define RADIOMSG_TAKINGFIRE "radio1;menuselect 6"
#define RADIOMSG_GOGOGO "radio2;menuselect 1"
#define RADIOMSG_TEAMFALLBACK "radio2;menuselect 2"
#define RADIOMSG_REGROUP "radio2;menuselect 3"
#define RADIOMSG_STAYANDWAIT "radio2;menuselect 4"
#define RADIOMSG_STORMTHEFRONT "radio2;menuselect 5"
#define RADIOMSG_REPORTINGIN "radio2;menuselect 6"
#define RADIOMSG_AFFIRMATIVE "radio3;menuselect 1"
#define RADIOMSG_ENEMYSPOTTED "radio3;menuselect 2"
#define RADIOMSG_NEEDBACKUP "radio3;menuselect 3"
#define RADIOMSG_SECTORCLEAR "radio3;menuselect 4"
#define RADIOMSG_INPOSITION "radio3;menuselect 5"
#define RADIOMSG_REPORT "radio3;menuselect 6"
#define RADIOMSG_GONNABLOW "radio3;menuselect 7"
#define RADIOMSG_NEGATIVE "radio3;menuselect 8"
#define RADIOMSG_ENEMYDOWN "radio3;menuselect 9"


// game start messages for Counter-Strike
#define MSG_CS_IDLE 0
#define MSG_CS_TEAMSELECT_MAINMENU 1
#define MSG_CS_TEAMSELECT_COUNTERMENU 2
#define MSG_CS_TEAMSELECT_TERRMENU 3
#define MSG_CS_BUY_MAINMENU 4
#define MSG_CS_BUY_PISTOLMENU 5
#define MSG_CS_BUY_SHOTGUNMENU 6
#define MSG_CS_BUY_RIFLEMENU 7
#define MSG_CS_BUY_SUBMACHINEGUNMENU 8
#define MSG_CS_BUY_MACHINEGUNMENU 9
#define MSG_CS_BUY_EQUIPMENTMENU 10


// mod_specific.cpp function prototypes
int GetTeam (entity_t *pPlayer);
bool ItemIsPrimary (entity_t *pItem);
bool ItemIsSecondary (entity_t *pItem);


// bot_client.cpp function prototypes
void BotClient_CS_VGUIMenu (void *p, int bot_index);
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
void BotClient_CS_ReloadSound (void *p, int bot_index);
void BotClient_CS_SayText (void *p, int bot_index);
void BotClient_CS_TextMsg (void *p, int bot_index);
void BotClient_CS_ScreenFade (void *p, int bot_index);
void BotClient_CS_StatusIcon (void *p, int bot_index);
void BotClient_CS_DeathMsgAll (void *p, int dont_use_me);
void BotClient_CS_TextMsgAll (void *p, int dont_use_me);


// bot.cpp function prototypes
void BotReset (bot_t *pBot);
void BotCreate (void);
void BotPreThink (bot_t *pBot);
void BotThink (bot_t *pBot);
void BotPostThink (bot_t *pBot);
void BotDecideWhatToDoNow (bot_t *pBot);
bool BotItemIsInteresting (bot_t *pBot, entity_t *pItem);
void BotDiscardStuffInOrderToGetItem (bot_t *pBot, entity_t *pItem);
void BotTalkOnTheRadio (bot_t *pBot, char *radiomsg);
void BotAnswerToOrder (bot_t *pBot);
bool BotHasPrimary (bot_t *pBot);
bool BotHasSecondary (bot_t *pBot);
bool BotHoldsPrimary (bot_t *pBot);
bool BotHoldsSecondary (bot_t *pBot);
void BotSelectTeamAndClass (bot_t *pBot);
void BotBuyStuff (bot_t *pBot);
void ComputeBotMsecVal (bot_t *pBot);


// bot_combat.cpp function prototypes


#endif // MOD_SPECIFIC_H
