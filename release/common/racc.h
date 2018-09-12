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
// racc.h
//


// bot version
#define RACC_VERSION "[template1]"
#define RACC_WELCOMETEXT "RACC version " RACC_VERSION " - http://www.racc-ai.com"
#define RACC_WELCOMESOUND "barney/guyresponsible.wav"
#define RACC_LOGFILEPATH "racc/release/messages.log"


// developer levels
#define DEVELOPER_OFF 0
#define DEVELOPER_ON 1
#define DEVELOPER_VERBOSE 2


// maximum number of bot personalities
#define MAX_BOT_PERSONALITIES 100


// bot HAL Markov model order
#define BOT_HAL_MODEL_ORDER 5

// amount of simultaneous entities bot can distinguish at once
#define BOT_EYE_SENSITIVITY 50

// amount of simultaneous sounds bot can distinguish at once
#define BOT_EAR_SENSITIVITY 10

// bot memory size in elements
#define BOT_MEMORY_SIZE 100


// HUD icon states
#define HUD_ICON_OFF 0
#define HUD_ICON_LIT 1
#define HUD_ICON_BLINKING 2


// immediate obstacle definitions
#define OBSTACLE_NONE 0
#define OBSTACLE_LEFT_LOWWALL (1 << 0)
#define OBSTACLE_LEFT_LOWCEILING (1 << 1)
#define OBSTACLE_LEFT_WALL (1 << 2)
#define OBSTACLE_LEFT_FALL (1 << 3)
#define OBSTACLE_FRONT_LOWWALL (1 << 4)
#define OBSTACLE_FRONT_LOWCEILING (1 << 5)
#define OBSTACLE_FRONT_WALL (1 << 6)
#define OBSTACLE_FRONT_FALL (1 << 7)
#define OBSTACLE_RIGHT_LOWWALL (1 << 8)
#define OBSTACLE_RIGHT_LOWCEILING (1 << 9)
#define OBSTACLE_RIGHT_WALL (1 << 10)
#define OBSTACLE_RIGHT_FALL (1 << 11)


// map topology granularity
#define MAX_MAP_PARALLELS 64
#define MAX_MAP_MERIDIANS 64


// interzone reachabilities
#define REACHABILITY_NORMAL 0
#define REACHABILITY_LADDER (1 << 0)
#define REACHABILITY_FALLEDGE (1 << 1)
#define REACHABILITY_ELEVATOR (1 << 2)
#define REACHABILITY_PLATFORM (1 << 3)


// water levels (whether an entity is in water or not)
#define WATERLEVEL_NOT 0
#define WATERLEVEL_TOUCH 1
#define WATERLEVEL_PARTLY 2
#define WATERLEVEL_COMPLETELY 3


// contents of a spot in the world
#define MATTER_EMPTY 0
#define MATTER_SOLID 1
#define MATTER_WATER 2
#define MATTER_SLIME 3
#define MATTER_LAVA 4
#define MATTER_SKY 5


// common bot orders
#define BOT_ORDER_NOORDER 0
#define BOT_ORDER_COVERME 1
#define BOT_ORDER_YOUTAKETHEPOINT 2
#define BOT_ORDER_HOLDTHISPOSITION 3
#define BOT_ORDER_STICKTOGETHERTEAM 4
#define BOT_ORDER_FOLLOWME 5
#define BOT_ORDER_TAKINGFIRE 6
#define BOT_ORDER_GOGOGO 7
#define BOT_ORDER_TEAMFALLBACK 8
#define BOT_ORDER_TAKECOVER 9
#define BOT_ORDER_STAYANDWAIT 10
#define BOT_ORDER_STORMTHEFRONT 11
#define BOT_ORDER_REPORTINTEAM 12
#define BOT_ORDER_NEEDBACKUP 13
#define BOT_ORDER_GETOUTOFTHERE 14


// engine-specific include files
#include "engine_specific.h"


#ifndef RACC_H
#define RACC_H


// debug levels structure definition
typedef struct
{
   int eyes; // vision debug level
   int ears; // hearing debug level
   int body; // feeling debug level
   int legs; // movement debug level
   int hand; // weaponry usage debug level
   int chat; // chat debug level
   int navigation; // navigation debug level
   bool is_observer; // set to TRUE if listen server client is in observer mode (bots ignore him)
   bool is_peacemode; // set to TRUE if the bots are told not to shoot each other unless provoked
} debug_level_t;


// walkable faces structure definition
typedef struct
{
   vector *v_corners; // pointer to array of face edges vector locations (mallocated)
   vector *v_delimiters; // pointer to array of face delimiters vector locations (mallocated)
   int corner_count; // number of edges (and thus delimiters) this face has
} walkface_t;


// topologic zone structure definition
typedef struct
{
   walkface_t **faces; // mallocated array of pointers to walkable faces located in this zone
   int faces_count; // number of walkable faces in this array
} zone_t;


// the virtual world itself
typedef struct
{
   vector v_worldmins; // quick access to the world's bounding box bottom left corner
   vector v_worldmaxs; // quick access to the world's bounding box top right corner
   int walkfaces_count; // number of walkable faces in this map
   walkface_t *walkfaces; // pointer to map's walkable faces memory space (mallocated)
   int parallels_count; // number of parallels dividing this map into sectors
   int meridians_count; // number of meridians dividing this map into sectors
   zone_t topology[MAX_MAP_PARALLELS][MAX_MAP_MERIDIANS]; // map spatial topology
} map_t;


// navigation link structure definition
typedef struct
{
   walkface_t *walkface; // pointer to the walkable face this link indicates to be reachable
   char reachability; // type of reachability it is (normal, ladder, edge fall, elevator, etc.)
} navlink_t;


// navigation node structure definition
typedef struct
{
   walkface_t *walkface; // the walkable face this node concerns
   navlink_t *links; // array of navigation links to walkfaces reachable from this one
   int links_count; // number of navigation links in this navigation node
} navnode_t;


// user message record structure definition
typedef struct
{
   const char *name; // name of user message as called by the MOD DLL
   int id; // identification number the engine recorded this user message under
   int size; // size of this message in packets
} usermsg_t;


// game round structure definition
typedef struct
{
   bool has_finished; // set to TRUE when the round is over and is about to restart
   float f_start_time; // date at which the round started
} round_t;


// WAV file related structure definitions
typedef struct
{
   char riff_chunk_id[4]; // RIFF chunk id ("RIFF")
   unsigned long wav_package_size; // total length of package (all following chunks) in bytes
   char wave_chunk_id[4]; // WAVE chunk id ("WAVE")
   char fmt_chunk_id[4]; // FORMAT chunk id ("fmt ")
   unsigned long fmt_chunk_length; // length of FORMAT chunk in bytes
   unsigned short dummy; // unknown, always 0x01 ???
   unsigned short channels; // number of channels (0x01 = mono, 0x02 = stereo)
   unsigned long sample_rate; // sample rate in Hertz
   unsigned long bytes_per_second; // bytes of data read per second (bps) when playing the file
   unsigned short bytes_per_sample; // bytes per sample (including all channels)
   unsigned short bits_per_sample; // bits per sample, bytes_per_sample * 8
   char data_chunk_id[4]; // DATA chunk id ("data")
   unsigned long data_chunk_length; // length of DATA chunk in bytes
} wav_header_t;


// HAL-related structure definitions
typedef struct
{
   unsigned char length; // length of word (for quick access)
   char *word; // the string itself
} HAL_STRING;


typedef struct
{
   unsigned long size; // size of dictionary (for quick access)
   HAL_STRING *entry; // string entry at this position of the dictionary
   unsigned short *index; // pointer to index
} HAL_DICTIONARY;


typedef struct
{
   unsigned short size; // size of the word swap structure
   HAL_STRING *from; // word to be changed into the next
   HAL_STRING *to; // word for replacing the previous
} HAL_SWAP;


typedef struct HAL_NODE
{
   unsigned short symbol; // symbol ID
   unsigned long usage; // usage information
   unsigned short count; // usage count (?)
   unsigned short branch; // branch of the tree
   struct HAL_NODE **tree; // pointer to tree node pointer
} HAL_TREE;


typedef struct
{
   unsigned char order; // model order (complexity)
   HAL_TREE *forward; // pointer to the model's forward tree
   HAL_TREE *backward; // pointer to the model's backwards tree
   HAL_TREE **context; // pointer to context tree pointer
   HAL_DICTIONARY *dictionary; // pointer to the model's dictionary
} HAL_MODEL;


// texture related structure definition
typedef struct
{
   char name[CBTEXTURENAMEMAX]; // name of the texture in the WAD file
   char type; // type of texture it is according to the materials.txt file
} texture_t;


// audible events related structure definitions
typedef struct
{
   char file_path[256]; // relative path to the sound sample
   float loudness; // violence of the sample (low value for footsteps, high value for gunshot)
   float duration; // amount of time our hearing capabilities will be occupied by this sound
} sound_t;


typedef struct
{
   char file_path[256]; // relative path to the sound sample being heard
   vector v_origin; // vector origin of the sound in the virtual world
   float loudness; // violence of the sample (low value for footsteps, high value for gunshot)
   float fade_date; // date at which this sound will have faded out
} noise_t;


// weapon related structure definitions
typedef struct
{
   char szClassname[64]; // weapon entity classname
   int iAmmo1; // ammo index for primary ammo
   int iAmmo1Max; // max primary ammo
   int iAmmo2; // ammo index for secondary ammo
   int iAmmo2Max; // max secondary ammo
   int iSlot; // HUD slot (0 based)
   int iPosition; // slot position
   int iId; // weapon ID
   int iFlags; // flags ???
} weapon_t;


// fov line structure definition
typedef struct
{
   vector scan_angles; // angles describing the traceline direction for this scan element
   float distance; // distance to first obstacle when tracing at scan_angles
   vector vecEndPos; // location of first obstacle when tracing at scan_angles
   vector Normal; // normal of first obstacle surface when tracing at scan_angles (to describe the plane)
   entity_t *pHit; // pointer to obstacle entity
} fov_line_t;


// chat string structure definition
typedef struct
{
   char sender[32]; // name of the sender of the chat line
   char text[128]; // the actual chat text (in uppercase)
} chat_line_t;


// player HUD structure definition
typedef struct
{
   chat_line_t chat_line[MAX_CHAT_MESSAGES]; // chat messages one can see on the screen
   bool new_chat_message; // set to TRUE if the chat messages have just scrolled down
   int menu_notify; // VGUI menu state on this HUD
   int bomb_icon_state; // bomb icon state (either off, lit, or blinking)
   int defuser_icon_state; // defuse kit icon state (either off, lit, or blinking)
} hud_t;


// player related structure definition
typedef struct
{
   entity_t *pEntity; // this player's entity the engine knows
   float welcome_time; // date at which this player will be sent a welcome message
   texture_t *pTextureAtFeet; // pointer to the last texture this player was walking on
   walkface_t *pFaceAtFeet; // pointer to the last face this player was walking on
   float step_sound_time; // date under which this player will NOT make a footstep sound
   float proximityweapon_swing_time; // date under which this player will NOT make a swing sound
   TraceResult prev_tr; // results of last traceline issued by this player (how handy :))
   entvars_t prev_v; // this player's entity variables state last frame (past pEntity->v)
} player_t;


// bot structures

// sensitive part of the AI
typedef struct
{
   float f_sample_time; // date at which the bot will next sample his field of view
   float f_blinded_time; // date under which the bot is blinded by too much light (flashbang...)
   fov_line_t BotFOV[52]; // bot's field of view is 52 elements width
   entity_t *pEntitiesInSight[BOT_EYE_SENSITIVITY]; // entities the bot distinguishes at a time
   hud_t BotHUD; // the bot's Head Up Display (where icons and chat messages are)
} bot_eyes_t;


typedef struct
{
   noise_t noises[BOT_EAR_SENSITIVITY]; // sounds the bot distinguishes at a time
   float average_noise; // average loudness the bot hears (average of noises.loudness)
   bool new_sound; // set to TRUE if a new sound has come to the bot's ears
   int bot_order; // integer representing the current order from a teammate bot is following
   float f_order_time; // date at which the bot received its last order
   entity_t *pAskingEntity; // pointer to the bot's current giving order entity
} bot_ears_t;


typedef struct
{
   short hit_state; // bit map description of the terrain in the immediate surroundings of the bot
   vector v_fall_plane_normal; // vector describing the normal of a fall edge in front of bot
} bot_body_t;


// motile part of the AI
typedef struct
{
   bool b_emergency_walkback; // set to TRUE if the bot has the reflex to step back facing a danger
   float f_max_speed; // maximum speed the engine allows the bot to move
   vector movement_speed; // bot's movement speeds (x = forward, y = side, z = vertical)
   vector aim_speed; // amount of degrees the bot will turn in one frame (it's a MOTILE event)
   vector ideal_angles; // bot's ideal view angles (the angles it wants to look at)
   playerbuttons_t input_buttons; // bot's input buttons state (the bot's virtual "keyboard")
   bool is_walking_straight; // set to TRUE if the bot is walking almost straight ahead
} bot_legs_t;


typedef struct
{
   int weapon_id; // ID number of weapon the bot is holding in its hands
   int iClip; // amount of ammo in the clip
   int iAmmo1; // amount of ammo in primary reserve
   int iAmmo2; // amount of ammo in secondary reserve
   int weapons; // bit map of weapons the bot is carrying
   float f_fire1_delay; // date at which the bot will next press the FIRE button
   float f_fire2_delay; // date at which the bot will press FIRE2 on its weapon
   int grenades_1; // bot's first grenades amount
   int grenades_2; // bot's second grenades amount
   float f_throwgrenade_time; // date at which the bot intends to throw a grenade
   int ammo[MAX_WEAPONS]; // total ammo amounts for this bot
} bot_hand_t;


typedef struct
{
   char say_message[128]; // chat message the bot is typing
   bool b_saytext_halreply; // set to TRUE when the bot plans to use its HAL brain for replying
   bool b_saytext_hello; // set to TRUE when the bot plans to say hello to other players
   bool b_saytext_kill; // set to TRUE when the bot plans to laugh at its victim
   bool b_saytext_killed; // set to TRUE when the bot plans to whine at its murderer
   bool b_saytext_idle; // set to TRUE when the bot plans to taunt other players
   bool b_saytext_follow; // set to TRUE when the bot plans to acknowledge to a 'follow me' order
   bool b_saytext_stop; // set to TRUE when the bot plans to acknowledge to a 'stop following' order
   bool b_saytext_stay; // set to TRUE when the bot plans to acknowledge to a 'stay here' order
   bool b_saytext_help; // set to TRUE when the bot plans to ask help from teammates
   bool b_saytext_cant; // set to TRUE when the bot plans to tell his caller it can't follow him
   bool b_saytext_affirmative; // set to TRUE when the bot plans to acknowledge some other order
   bool b_saytext_negative; // set to TRUE when the bot plans to deny an order
   bool b_saytext_bye; // set to TRUE when the bot plans to say goodbye to other players
   float f_saytext_time; // date at which the bot will actually send its text
   bool b_sayaudio_affirmative; // set to TRUE when the bot plans to acknowledge to an order
   bool b_sayaudio_alert; // set to TRUE when the bot plans to alert its teammates about a hostile
   bool b_sayaudio_attacking; // set to TRUE when the bot plans to tell its team it is attacking
   bool b_sayaudio_firstspawn; // set to TRUE when the bot plans to taunt at spawn time
   bool b_sayaudio_inposition; // set to TRUE when the bot plans to tell its team it holds a position
   bool b_sayaudio_negative; // set to TRUE when the bot plans to deny an order
   bool b_sayaudio_report; // set to TRUE when the bot plans to request check-in from its teammates
   bool b_sayaudio_reporting; // set to TRUE when the bot plans to answer to a check-in order
   bool b_sayaudio_seegrenade; // set to TRUE when the bot plans to yell about an incoming grenade
   bool b_sayaudio_takingdamage; // set to TRUE when the bot plans to tell it is taking damage
   bool b_sayaudio_throwgrenade; // set to TRUE when the bot plans to tell about a thrown grenade
   bool b_sayaudio_victory; // set to TRUE when the bot plans to tell its team its enemy is dead
   float f_sayaudio_time; // date at which the bot will actually speak
} bot_chat_t;


// memory structure definition
typedef struct
{
   float time_of_creation; // date at which the event has been stored in bot's memory
   float time_of_death; // date at which the event will be forgotten (the more important the event, the more longer bot wants to keep it in mind)
   int event_type; // shortcut to handle differently different types of events (camp spot, zone, killer name, etc.)
   vector location; // location where bot was when he decided to remember this event
   fov_line_t BotFOV[52]; // bot's field of view at location
   entity_t *pInterActor; // pointer to the entity that ''impulsed'' the reminder (the killer player, for example)
   char description[128]; // litteral description of the event (can be an intercepted chat string)
   int estimated_worth; // here bot can store personal estimation stuff like danger, etc.
} bot_memory_t;


// personality structure definition
typedef struct
{
   char name[33]; // name of the bot as seen in the game
   char skin[33]; // skin used by the bot
   int skill; // skill coefficient of the bot from 1 to 5 (1 is bad, 5 is very good)
   char text_affirmative[100][256]; // array of sentences the bot can say as an affirmative answer
   int text_affirmative_count; // number of these sentences
   char text_negative[100][256]; // array of sentences the bot can say as a negative answer
   int text_negative_count; // number of these sentences
   char text_hello[100][256]; // array of sentences the bot can say as a hello statment
   int text_hello_count; // number of these sentences
   char text_laugh[100][256]; // array of sentences the bot can say as a taunt statment
   int text_laugh_count; // number of these sentences
   char text_whine[100][256]; // array of sentences the bot can say as a whine statment
   int text_whine_count; // number of these sentences
   char text_idle[100][256]; // array of sentences the bot can say as an idle chat
   int text_idle_count; // number of these sentences
   char text_follow[100][256]; // array of sentences the bot can say as a 'follow me' answer
   int text_follow_count; // number of these sentences
   char text_stop[100][256]; // array of sentences the bot can say as a 'stop following' answer
   int text_stop_count; // number of these sentences
   char text_stay[100][256]; // array of sentences the bot can say as a 'stay here' answer
   int text_stay_count; // number of these sentences
   char text_help[100][256]; // array of sentences the bot can say as a help request
   int text_help_count; // number of these sentences
   char text_cant[100][256]; // array of sentences the bot can say as an impossibility statment
   int text_cant_count; // number of these sentences
   char text_bye[100][256]; // array of sentences the bot can say as a goodbye statment
   int text_bye_count; // number of these sentences
   int audio_affirmative_count; // number of sound samples the bot can speak as an affirmative answer
   int audio_alert_count; // number of sound samples the bot can speak as an alert statment
   int audio_attacking_count; // number of sound samples the bot can speak as an attack statment
   int audio_firstspawn_count; // number of sound samples the bot can speak as a spawn declaration
   int audio_inposition_count; // number of sound samples the bot can speak as an 'i'm in position' statment
   int audio_negative_count; // number of sound samples the bot can speak as a negative answer
   int audio_report_count; // number of sound samples the bot can speak as a report order
   int audio_reporting_count; // number of sound samples the bot can speak as a report statment
   int audio_seegrenade_count; // number of sound samples the bot can speak as a grenade alert
   int audio_takingdamage_count; // number of sound samples the bot can speak as a damage declaration
   int audio_throwgrenade_count; // number of sound samples the bot can speak as a grenade throw declaration
   int audio_victory_count; // number of sound samples the bot can speak as a victory declaration
   HAL_DICTIONARY *banned_keywords; // dictionary of words that must never be used as keywords
   HAL_DICTIONARY *auxiliary_keywords; // dictionary of auxiliary keywords
   HAL_SWAP *swappable_keywords; // array of swappable keywords with their equivalences
   HAL_MODEL *bot_model; // Markov model of the bot
   HAL_DICTIONARY *input_words; // global chat's dictionary of words
   HAL_DICTIONARY *bot_words; // bot's own dictionary of words
   bool keyword_is_used; // set to TRUE when the first direction generated reply uses the keyword
   bot_memory_t GeneralMemory[BOT_MEMORY_SIZE]; // bot's individual memory is an array of elements
   navnode_t *PathMemory; // pointer to the array of navigation nodes the bot remembers
} bot_personality_t;


// the bot
typedef struct
{
   bool is_active; // set to TRUE if this slot in the bots array belongs to a bot currently playing
   entity_t *pEntity; // the bot entity itself from the engine
   entity_t *pLightEntity; // dummy entity for getting the correct bot illumination (engine bug)
   bot_personality_t *pPersonality; // pointer to the personality of this bot
   char buy_state; // state machine state for the bot's buy procedures

   int msecnum; // used in TheFatal's method for calculating the msecval delay
   float msecdel; // used in TheFatal's method for calculating the msecval delay
   float msecval; // amount of time the movement of the bot (pfnRunPlayerMove) should extend
   float f_prev_playermove_time; // date at which the last pfnRunPlayerMove call happened

   bot_eyes_t BotEyes; // structure describing what the bot sees
   bot_ears_t BotEars; // structure describing what the bot hears
   bot_body_t BotBody; // structure describing what the bot touches (or is about to)

   bot_legs_t BotLegs; // structure handling the directions in which the bot will move/jump/duck
   bot_hand_t BotHand; // structure handling the weapon the bot is holding in its hands
   bot_chat_t BotChat; // structure handling what the bot has to say and when it will say it

   entity_t *pVictimEntity; // pointer to the bot's last victim entity
   entity_t *pKillerEntity; // pointer to the bot's last murderer entity

   // SHOULD GO INTO BOT MEMORY
   int bot_team; // bot's team stored as an integer for team-based MODs
   int bot_class; // bot's class (i.e. skin) for class-based MODs
   int bot_money; // amount of money the bot owns for buy action based MODs

   // IMPLEMENT A TRUE REACTION TIME
   float f_buy_time; // date at which the bot will issue the next buy command

   entity_t *pBotEnemy; // pointer to the bot's current enemy entity (NULL if no enemy)
   vector v_lastseenenemy_position; // location at which the bot last saw its last enemy
   float f_see_enemy_time; // date at which the bot first saw its last enemy
   float f_lost_enemy_time; // date at which the bot lost sight of its last enemy
   bool b_enemy_hidden; // set to TRUE if the bot lost sight of an enemy still likely to be alive

   vector v_place_to_keep; // location the bot intends to stay at
   float f_place_time; // date at which the bot last saw its last location to stay at

   float f_fallcheck_time; // date under which the bot won't mind about falls in its neighbourhood
   float f_turncorner_time; // date under which the bot won't mind about corners on its sides
   float f_avoid_time; // date under which the bot won't mind about walls in its neighbourhood
   float f_camp_time; // date under which the bot will be camping (staying ambushed)

   vector v_reach_point; // location the bot is immediately heading to
   float f_reach_time; // date under which the bot should not worry about heading to the above
   float f_findreachpoint_time; // date at which the bot will determine a new reach point

   vector v_prev_position; // location at which the bot was standing during the last stuck check
   float f_check_stuck_time; // date under which the bot won't check for being stuck at all
   bool b_is_stuck; // set to TRUE if the bot is not moving as fast as it would to
} bot_t;


// dll.cpp function prototypes
//
// interface :      [META]MOD DLL <===1===> racc.dll (BOT DLL) <===2===> hw.dll (ENGINE DLL)
//
// This file contains the functions that interface the MOD DLL with the BOT DLL (interface 1)
//
DLL_GIVEFNPTRSTODLL DLLEXPORT GiveFnptrsToDll (enginefuncs_t* pengfuncsFromEngine, globalvars_t *pGlobals);
void GameDLLInit (void);
int Spawn (entity_t *pent);
void Think (entity_t *pent);
void Use (entity_t *pentUsed, entity_t *pentOther);
void Touch (entity_t *pentTouched, entity_t *pentOther);
void Blocked (entity_t *pentBlocked, entity_t *pentOther);
void KeyValue (entity_t *pentKeyvalue, KeyValueData *pkvd);
void Save (entity_t *pent, SAVERESTOREDATA *pSaveData);
int Restore (entity_t *pent, SAVERESTOREDATA *pSaveData, int globalEntity);
void SetAbsBox (entity_t *pent);
void SaveWriteFields (SAVERESTOREDATA *pSaveData, const char *pname, void *pBaseData, TYPEDESCRIPTION *pFields, int fieldCount);
void SaveReadFields (SAVERESTOREDATA *pSaveData, const char *pname, void *pBaseData, TYPEDESCRIPTION *pFields, int fieldCount);
void SaveGlobalState (SAVERESTOREDATA *pSaveData);
void RestoreGlobalState (SAVERESTOREDATA *pSaveData);
void ResetGlobalState (void);
BOOL ClientConnect (entity_t *pEntity, const char *pszName, const char *pszAddress, char szRejectReason[ 128 ] );
void ClientDisconnect (entity_t *pEntity);
void ClientKill (entity_t *pEntity);
void ClientPutInServer (entity_t *pEntity);
void ClientCommand (entity_t *pEntity);
void ClientUserInfoChanged (entity_t *pEntity, char *infobuffer);
void ServerActivate (entity_t *pEdictList, int edictCount, int clientMax);
void ServerDeactivate (void);
void PlayerPreThink (entity_t *pEntity);
void PlayerPostThink (entity_t *pEntity);
void StartFrame (void);
void ParmsNewLevel (void);
void ParmsChangeLevel (void);
const char *GetGameDescription (void);
void PlayerCustomization (entity_t *pEntity, customization_t *pCust);
void SpectatorConnect (entity_t *pEntity);
void SpectatorDisconnect (entity_t *pEntity);
void SpectatorThink (entity_t *pEntity);
void Sys_Error (const char *error_string);
void PM_Move (struct playermove_s *ppmove, int server);
void PM_Init (struct playermove_s *ppmove);
char PM_FindTextureType (char *name);
void SetupVisibility (entity_t *pViewEntity, entity_t *pClient, unsigned char **pvs, unsigned char **pas);
void UpdateClientData (const struct edict_s *ent, int sendweapons, struct clientdata_s *cd);
int AddToFullPack (struct entity_state_s *state, int e, entity_t *ent, entity_t *host, int hostflags, int player, unsigned char *pSet);
void CreateBaseline (int player, int eindex, struct entity_state_s *baseline, struct edict_s *entity, int playermodelindex, vector player_mins, vector player_maxs);
void RegisterEncoders (void);
int GetWeaponData (struct edict_s *player, struct weapon_data_s *info);
void CmdStart (const entity_t *player, const struct usercmd_s *cmd, unsigned int random_seed);
void CmdEnd (const entity_t *player);
int ConnectionlessPacket (const struct netadr_s *net_from, const char *args, char *response_buffer, int *response_buffer_size);
int GetHullBounds (int hullnumber, float *mins, float *maxs);
void CreateInstancedBaselines (void);
int InconsistentFile (const entity_t *player, const char *filename, char *disconnect_message);
int AllowLagCompensation (void);
void ServerCommand (void);
void PlayClientSoundsForBots (entity_t *pPlayer);
void PlayBulletSoundsForBots (entity_t *pPlayer);
void LoadSymbols (const char *filename);
void FreeAllTheStuff (void);

// engine.cpp function prototypes...
//
// interface :      [META]MOD DLL <===1===> racc.dll (BOT DLL) <===2===> hw.dll (ENGINE DLL)
//
// This file contains the functions that interface the BOT DLL with the ENGINE DLL (interface 2)
//
int pfnPrecacheModel (char* s);
int pfnPrecacheSound (char* s);
void pfnSetModel (entity_t *e, const char *m);
int pfnModelIndex (const char *m);
int pfnModelFrames (int modelIndex);
void pfnSetSize (entity_t *e, const float *rgflMin, const float *rgflMax);
void pfnChangeLevel (char* s1, char* s2);
void pfnGetSpawnParms (entity_t *ent);
void pfnSaveSpawnParms (entity_t *ent);
float pfnVecToYaw (const float *rgflVector);
void pfnVecToAngles (const float *rgflVectorIn, float *rgflVectorOut);
void pfnMoveToOrigin (entity_t *ent, const float *pflGoal, float dist, int iMoveType);
void pfnChangeYaw (entity_t* ent);
void pfnChangePitch (entity_t* ent);
entity_t* pfnFindEntityByString (entity_t *pEdictStartSearchAfter, const char *pszField, const char *pszValue);
int pfnGetEntityIllum (entity_t* pEnt);
entity_t* pfnFindEntityInSphere (entity_t *pEdictStartSearchAfter, const float *org, float rad);
entity_t* pfnFindClientInPVS (entity_t *pEntity);
entity_t* pfnEntitiesInPVS (entity_t *pplayer);
void pfnMakeVectors (const float *rgflVector);
void pfnAngleVectors (const float *rgflVector, float *forward, float *right, float *up);
entity_t* pfnCreateEntity (void);
void pfnRemoveEntity (entity_t* e);
entity_t* pfnCreateNamedEntity (int className);
void pfnMakeStatic (entity_t *ent);
int pfnEntIsOnFloor (entity_t *e);
int pfnDropToFloor (entity_t* e);
int pfnWalkMove (entity_t *ent, float yaw, float dist, int iMode);
void pfnSetOrigin (entity_t *e, const float *rgflOrigin);
void pfnEmitSound (entity_t *entity, int channel, const char *sample, float volume, float attenuation, int fFlags, int pitch);
void pfnEmitAmbientSound (entity_t *entity, float *pos, const char *samp, float vol, float attenuation, int fFlags, int pitch);
void pfnTraceLine (const float *v1, const float *v2, int fNoMonsters, entity_t *pentToSkip, TraceResult *ptr);
void pfnTraceToss (entity_t* pent, entity_t* pentToIgnore, TraceResult *ptr);
int pfnTraceMonsterHull (entity_t *pEntity, const float *v1, const float *v2, int fNoMonsters, entity_t *pentToSkip, TraceResult *ptr);
void pfnTraceHull (const float *v1, const float *v2, int fNoMonsters, int hullNumber, entity_t *pentToSkip, TraceResult *ptr);
void pfnTraceModel (const float *v1, const float *v2, int hullNumber, entity_t *pent, TraceResult *ptr);
const char *pfnTraceTexture (entity_t *pTextureEntity, const float *v1, const float *v2);
void pfnTraceSphere (const float *v1, const float *v2, int fNoMonsters, float radius, entity_t *pentToSkip, TraceResult *ptr);
void pfnGetAimVector (entity_t* ent, float speed, float *rgflReturn);
void pfnServerCommand (char* str);
void pfnServerExecute (void);
void pfnClientCommand (entity_t* pEntity, char* szFmt, ...);
void pfnParticleEffect (const float *org, const float *dir, float color, float count);
void pfnLightStyle (int style, char* val);
int pfnDecalIndex (const char *name);
int pfnPointContents (const float *rgflVector);
void pfnMessageBegin (int msg_dest, int msg_type, const float *pOrigin, entity_t *ed);
void pfnMessageEnd (void);
void pfnWriteByte (int iValue);
void pfnWriteChar (int iValue);
void pfnWriteShort (int iValue);
void pfnWriteLong (int iValue);
void pfnWriteAngle (float flValue);
void pfnWriteCoord (float flValue);
void pfnWriteString (const char *sz);
void pfnWriteEntity (int iValue);
void pfnCVarRegister (cvar_t *pCvar);
float pfnCVarGetFloat (const char *szVarName);
const char* pfnCVarGetString (const char *szVarName);
void pfnCVarSetFloat (const char *szVarName, float flValue);
void pfnCVarSetString (const char *szVarName, const char *szValue);
void pfnAlertMessage (ALERT_TYPE atype, char *szFmt, ...);
void pfnEngineFprintf (FILE *pfile, char *szFmt, ...);
void* pfnPvAllocEntPrivateData (entity_t *pEntity, long cb);
void* pfnPvEntPrivateData (entity_t *pEntity);
void pfnFreeEntPrivateData (entity_t *pEntity);
const char* pfnSzFromIndex (int iString);
int pfnAllocString (const char *szValue);
struct entvars_s* pfnGetVarsOfEnt (entity_t *pEntity);
entity_t* pfnPEntityOfEntOffset (int iEntOffset);
int pfnEntOffsetOfPEntity (const entity_t *pEntity);
int pfnIndexOfEdict (const entity_t *pEntity);
entity_t* pfnPEntityOfEntIndex (int iEntIndex);
entity_t* pfnFindEntityByVars (struct entvars_s* pvars);
void* pfnGetModelPtr (entity_t* pEntity);
int pfnRegUserMsg (const char *pszName, int iSize);
void pfnAnimationAutomove (const entity_t* pEntity, float flTime);
void pfnGetBonePosition (const entity_t* pEntity, int iBone, float *rgflOrigin, float *rgflAngles);
unsigned long pfnFunctionFromName (const char *pName);
const char *pfnNameForFunction (unsigned long function);
void pfnClientPrintf (entity_t* pEntity, PRINT_TYPE ptype, const char *szMsg);
void pfnServerPrint (const char *szMsg);
const char *pfnCmd_Args (void);
const char *pfnCmd_Argv (int argc);
int pfnCmd_Argc (void);
void pfnGetAttachment (const entity_t *pEntity, int iAttachment, float *rgflOrigin, float *rgflAngles);
void pfnCRC32_Init (CRC32_t *pulCRC);
void pfnCRC32_ProcessBuffer (CRC32_t *pulCRC, void *p, int len);
void pfnCRC32_ProcessByte (CRC32_t *pulCRC, unsigned char ch);
CRC32_t pfnCRC32_Final (CRC32_t pulCRC);
long pfnRandomLong (long  lLow,  long  lHigh);
float pfnRandomFloat (float flLow, float flHigh);
void pfnSetView (const entity_t *pClient, const entity_t *pViewent);
float pfnTime (void);
void pfnCrosshairAngle (const entity_t *pClient, float pitch, float yaw);
byte *pfnLoadFileForMe (char *filename, int *pLength);
void pfnFreeFile (void *buffer);
void pfnEndSection (const char *pszSectionName);
int pfnCompareFileTime (char *filename1, char *filename2, int *iCompare);
void pfnGetGameDir (char *szGetGameDir);
void pfnCvar_RegisterVariable (cvar_t *variable);
void pfnFadeClientVolume (const entity_t *pEntity, int fadePercent, int fadeOutSeconds, int holdTime, int fadeInSeconds);
void pfnSetClientMaxspeed (const entity_t *pEntity, float fNewMaxspeed);
entity_t *pfnCreateFakeClient (const char *netname);
void pfnRunPlayerMove (entity_t *fakeclient, const float *viewangles, float forwardmove, float sidemove, float upmove, unsigned short buttons, byte impulse, byte msec);
int pfnNumberOfEntities (void);
char* pfnGetInfoKeyBuffer (entity_t *e);
char* pfnInfoKeyValue (char *infobuffer, char *key);
void pfnSetKeyValue (char *infobuffer, char *key, char *value);
void pfnSetClientKeyValue (int clientIndex, char *infobuffer, char *key, char *value);
int pfnIsMapValid (char *filename);
void pfnStaticDecal (const float *origin, int decalIndex, int entityIndex, int modelIndex);
int pfnPrecacheGeneric (char* s);
int pfnGetPlayerUserId (entity_t *e);
void pfnBuildSoundMsg (entity_t *entity, int channel, const char *sample, float volume, float attenuation, int fFlags, int pitch, int msg_dest, int msg_type, const float *pOrigin, entity_t *ed);
int pfnIsDedicatedServer (void);
cvar_t *pfnCVarGetPointer (const char *szVarName);
unsigned int pfnGetPlayerWONId (entity_t *e);
void pfnInfo_RemoveKey (char *s, const char *key);
const char *pfnGetPhysicsKeyValue (const entity_t *pClient, const char *key);
void pfnSetPhysicsKeyValue (const entity_t *pClient, const char *key, const char *value);
const char *pfnGetPhysicsInfoString (const entity_t *pClient);
unsigned short pfnPrecacheEvent (int type, const char*psz);
void pfnPlaybackEvent (int flags, const entity_t *pInvoker, unsigned short eventindex, float delay, float *origin, float *angles, float fparam1, float fparam2, int iparam1, int iparam2, int bparam1, int bparam2);
unsigned char *pfnSetFatPVS (float *org);
unsigned char *pfnSetFatPAS (float *org);
int pfnCheckVisibility (const entity_t *entity, unsigned char *pset);
void pfnDeltaSetField (struct delta_s *pFields, const char *fieldname);
void pfnDeltaUnsetField (struct delta_s *pFields, const char *fieldname);
void pfnDeltaAddEncoder (char *name, void conditionalencode (struct delta_s *pFields, const unsigned char *from, const unsigned char *to));
int pfnGetCurrentPlayer (void);
int pfnCanSkipPlayer (const entity_t *player);
int pfnDeltaFindField (struct delta_s *pFields, const char *fieldname);
void pfnDeltaSetFieldByIndex (struct delta_s *pFields, int fieldNumber);
void pfnDeltaUnsetFieldByIndex (struct delta_s *pFields, int fieldNumber);
void pfnSetGroupMask (int mask, int op);
int pfnCreateInstancedBaseline (int classname, struct entity_state_s *baseline);
void pfnCvar_DirectSet (struct cvar_s *var, char *value);
void pfnForceUnmodified (FORCE_TYPE type, float *mins, float *maxs, const char *filename);
void pfnGetPlayerStats (const entity_t *pClient, int *ping, int *packet_loss);
void pfnAddServerCommand (char *cmd_name, void function (void));
qboolean pfnVoice_GetClientListening (int iReceiver, int iSender);
qboolean pfnVoice_SetClientListening (int iReceiver, int iSender, qboolean bListen);
const char *pfnGetPlayerAuthId (entity_t *e);


// bot_eyes.cpp function prototypes
void BotSee (bot_t *pBot);
vector BotCanSeeOfEntity (bot_t *pBot, entity_t *pEntity);


// bot_ears.cpp function prototypes
void BotHear (bot_t *pBot);
void BotFeedEar (bot_t *pBot, sound_t *sound, vector v_origin, float volume);
void DispatchSound (const char *sample, vector v_origin, float volume, float attenuation);
void PrecacheSoundForBots (char *sound_path, int sound_index);
void InitTextureSoundsForBots (void);


// bot_body.cpp function prototypes
void BotTouch (bot_t *pBot);
void BotCheckForObstaclesAtFeet (bot_t *pBot);
void BotCheckIfStuck (bot_t *pBot);


// bot_legs.cpp function prototypes
void BotMove (bot_t *pBot);
void BotSetIdealAngles (bot_t *pBot, vector v_ideal_angles);
void BotPointGun (bot_t *pBot);
void BotAvoidObstacles (bot_t *pBot);


// bot_hand.cpp function prototypes


// bot_chat.cpp function prototypes
void BotChat (bot_t *pBot);
void BotSayText (bot_t *pBot);
void BotSayAudio (bot_t *pBot);
void BotTalk (bot_t *pBot, char *sound_path);
const char *Name (const char *string);
const char *HumanizeChat (const char *string);
const char *StripBlanks (const char *string);
const char *StripTags (const char *string);
void UpperCase (char *string);
void LowerCase (char *string);
void HAL_LoadTree (FILE *file, HAL_TREE *node);
void HAL_LoadDictionary (FILE *file, HAL_DICTIONARY *dictionary);
void HAL_SaveTree (FILE *file, HAL_TREE *node);
void HAL_SaveDictionary (FILE *file, HAL_DICTIONARY *dictionary);
void HAL_Learn (HAL_MODEL *model, HAL_DICTIONARY *words);
unsigned short HAL_AddWord (HAL_DICTIONARY *dictionary, HAL_STRING word);
int HAL_SearchDictionary (HAL_DICTIONARY *dictionary, HAL_STRING word, bool *find);
unsigned short HAL_FindWord (HAL_DICTIONARY *dictionary, HAL_STRING word);
int HAL_CompareWords (HAL_STRING word1, HAL_STRING word2);
void HAL_InitializeDictionary (HAL_DICTIONARY *dictionary);
HAL_DICTIONARY *HAL_NewDictionary (void);
HAL_TREE *HAL_NewNode (void);
HAL_MODEL *HAL_NewModel (int order);
void HAL_UpdateModel (HAL_MODEL *model, int symbol);
void HAL_UpdateContext (HAL_MODEL *model, int symbol);
HAL_TREE *HAL_AddSymbol (HAL_TREE *tree, unsigned short symbol);
HAL_TREE *HAL_FindSymbol (HAL_TREE *node, int symbol);
HAL_TREE *HAL_FindSymbolAdd (HAL_TREE *node, int symbol);
void HAL_AddNode (HAL_TREE *tree, HAL_TREE *node, int position);
int HAL_SearchNode (HAL_TREE *node, int symbol, bool *found_symbol);
void HAL_InitializeContext (HAL_MODEL *model);
void BotHALTrainModel (bot_t *pBot, HAL_MODEL *model);
void HAL_ShowDictionary (HAL_DICTIONARY *dictionary);
void HAL_MakeWords (char *input, HAL_DICTIONARY *words);
bool HAL_BoundaryExists (char *string, int position);
void BotHALGenerateReply (bot_t *pBot, char *output);
bool HAL_DictionariesDiffer (HAL_DICTIONARY *words1, HAL_DICTIONARY *words2);
HAL_DICTIONARY *BotHALMakeKeywords (bot_t *pBot, HAL_DICTIONARY *words);
void BotHALAddKeyword (bot_t *pBot, HAL_DICTIONARY *keys, HAL_STRING word);
void BotHALAddAuxiliaryKeyword (bot_t *pBot, HAL_DICTIONARY *keys, HAL_STRING word);
HAL_DICTIONARY *BotHALBuildReplyDictionary (bot_t *pBot, HAL_DICTIONARY *keys);
int BotHALBabble (bot_t *pBot, HAL_DICTIONARY *keys, HAL_DICTIONARY *words);
bool HAL_WordExists (HAL_DICTIONARY *dictionary, HAL_STRING word);
int BotHALSeedReply (bot_t *pBot, HAL_DICTIONARY *keys);
HAL_SWAP *HAL_NewSwap (void);
void HAL_AddSwap (HAL_SWAP *list, char *s, char *d);
HAL_SWAP *HAL_InitializeSwap (char *filename);
HAL_DICTIONARY *HAL_InitializeList (char *filename);
void HAL_EmptyDictionary (HAL_DICTIONARY *dictionary);
void HAL_FreeModel (HAL_MODEL *model);
void HAL_FreeTree (HAL_TREE *tree);
void HAL_FreeSwap (HAL_SWAP *swap);
void PrepareHALBrainForPersonality (bot_personality_t *personality);
bool LoadHALBrainForPersonality (bot_personality_t *personality);
void SaveHALBrainForPersonality (bot_personality_t *personality);


// bot_navigation.cpp function prototypes
//////////////////////////////////////////////
// WRITE YOUR OWN NAVIGATION FUNCTIONS HERE //
//////////////////////////////////////////////
void PrepareNavBrainForPersonality (bot_personality_t *personality);
bool LoadNavBrainForPersonality (bot_personality_t *personality);
void SaveNavBrainForPersonality (bot_personality_t *personality);


// util.cpp function prototypes
void UTIL_TraceLine (const vector &vecStart, const vector &vecEnd, IGNORE_MONSTERS igmon, IGNORE_GLASS ignoreGlass, entity_t *pentIgnore, TraceResult *ptr);
void UTIL_TraceLine (const vector &vecStart, const vector &vecEnd, IGNORE_MONSTERS igmon, entity_t *pentIgnore, TraceResult *ptr);
void UTIL_TraceHull (const vector &vecStart, const vector &vecEnd, IGNORE_MONSTERS igmon, int hullNumber, entity_t *pentIgnore, TraceResult *ptr);
entity_t *UTIL_FindEntityInSphere (entity_t *pentStart, const vector &vecCenter, float flRadius);
entity_t *UTIL_FindEntityByString (entity_t *pentStart, const char *szKeyword, const char *szValue);
entity_t *UTIL_FindEntityByClassname (entity_t *pentStart, const char *szName);
entity_t *UTIL_FindEntityByTargetname (entity_t *pentStart, const char *szName);
int GetUserMsgId (const char *msg_name);
const char *GetUserMsgName (int msg_type);
bool IsReachable (vector v_dest, entity_t *pEntity);
bool IsAtHumanHeight (vector v_location);
vector DropAtHumanHeight (vector v_location);
void UTIL_DrawWalkface (entity_t *pClient, walkface_t *face, int life, int red, int green, int blue);
void UTIL_DrawSector (entity_t *pClient, vector v_location, int life, int red, int green, int blue);
void LoadBotProfiles (void);
void FakeClientCommand (entity_t *pFakeClient, const char *fmt, ...);
const char *GetArg (const char *command, int arg_number);
texture_t *FindTextureByName (const char *texture_name);
bool FileExists (char *filename);
walkface_t *WalkfaceUnder (entity_t *pEntity);
vector NearestDelimiterOf (entity_t *pEntity);
bool SegmentBelongsToSector (const vector &v_bound1, const vector &v_bound2, int sector_i, int sector_j);
bool LoadWorldMap (void);
int SaveWorldMap (int bsp_file_size);


// math.cpp function prototypes
float WrapAngle (float angle_to_wrap);
float WrapAngle360 (float angle_to_wrap);
vector WrapAngles (vector &angles_to_wrap);
vector WrapAngles360 (vector &angles_to_wrap);
float AngleBetweenVectors (vector &vec1, vector &vec2);
float DotProduct (vector &vec1, vector &vec2);
vector CrossProduct (vector &vec1, vector &vec2);


// mfile.cpp function prototypes
FILE *mfopen (const char *file_path, const char *mode);
long mftell (FILE *fp);
int mfseek (FILE *fp, long offset, int offset_mode);
int mfeof (FILE *fp);
size_t mfread (void *destination, size_t block_size, size_t num_blocks, FILE *fp);
int mfgetc (FILE *fp);
const char *mfgets (char *line_buffer, int buffer_size, FILE *fp);
void mfclose (FILE *fp);


// MOD-specific include files
#ifdef ASHEEP_DLL
#include "../asheep/mod_specific.h"
#elif CSTRIKE_DLL
#include "../cstrike/mod_specific.h"
#elif DMC_DLL
#include "../dmc/mod_specific.h"
#elif GEARBOX_DLL
#include "../gearbox/mod_specific.h"
#elif TFC_DLL
#include "../tfc/mod_specific.h"
#elif VALVE_DLL
#include "../valve/mod_specific.h"
#else
#error Unrecognized MOD (must define a MOD_DLL - cf bottom of racc.h)
#endif


#endif // RACC_H

