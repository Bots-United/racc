// RACC - AI development project for first-person shooter games
// (http://racc.bots-united.com/)
//
// Rational Autonomous Cybernetic Commandos AI
//
// racc.h
//

// DIRECT ENGINE CALLS:
//
// - INDEXENT ()
// - ENTINDEX ()
// - TRACE_HULL ()
// - POINT_CONTENTS ()
// - SERVER_PRINT ()
// - CVAR_GET_STRING ()
// - PRECACHE_MODEL ()              - beams
// - CREATE_NAMED_ENTITY ()         - part of illum bugfix
// - SET_ORIGIN ()
// - SET_MODEL ()
// - CLIENT_COMMAND ()              - bot radio
// - SERVER_COMMAND ()
// - INFOKEY_VALUE (GET_INFOKEYBUFFER ())
// - GETENTITYILLUM ()
// - STRING ()
// - MAKE_STRING ()
// - CMD_ARGV ()
// - FNullEnt ()
// - (*g_engfuncs.pfnCreateFakeClient) ()
// - (*g_engfuncs.pfnRunPlayerMove) ()
//
// - MESSAGE_BEGIN ()
// - WRITE_nnn ()
// - MESSAGE_END ()
//
// - MDLL_ClientCommand ()
// - MDLL_ClientConnect ()
// - MDLL_ClientPutInServer ()
// - MDLL_Spawn ()                  - part of illum bugfix
// - MDLL_PM_FindTextureType ()
// - MDLL_ClientKill ()             - could be removed and replaced by damage messages ?


// NOTE: bot version is automatically generated in MakeVersion()


// generic include files
#include <limits.h>
#include <time.h>
#include <sys/stat.h>
#include <io.h>


// maximum number of players in the game at the same time the engine can support
#define RACC_MAX_CLIENTS 32

// maximum number of like/dislike levels for special node reachabilities
#define RACC_MAX_LIKELEVELS 32


// the weird constant itself
#define MATH_PI 3.14159265358979323846


// AI debug channels
#define CHANNEL_EYES 1
#define CHANNEL_EARS 2
#define CHANNEL_BODY 3
#define CHANNEL_LEGS 4
#define CHANNEL_HAND 5
#define CHANNEL_CHAT 6
#define CHANNEL_COGNITION 126
#define CHANNEL_NAVIGATION 127


// HUD icon states
#define HUD_ICON_OFF 0
#define HUD_ICON_LIT 1
#define HUD_ICON_BLINKING 2


// relative directions
#define DIRECTION_NONE 0
#define DIRECTION_FRONT (1 << 0)
#define DIRECTION_BACK (1 << 1)
#define DIRECTION_LEFT (1 << 2)
#define DIRECTION_RIGHT (1 << 3)


// input keys
#define INPUT_KEY_NONE 0
#define INPUT_KEY_FORWARD (1 << 0)
#define INPUT_KEY_BACKWARDS (1 << 1)
#define INPUT_KEY_TURNLEFT (1 << 2)
#define INPUT_KEY_TURNRIGHT (1 << 3)
#define INPUT_KEY_STRAFELEFT (1 << 4)
#define INPUT_KEY_STRAFERIGHT (1 << 5)
#define INPUT_KEY_JUMP (1 << 6)
#define INPUT_KEY_DUCK (1 << 7)
#define INPUT_KEY_PRONE (1 << 8)
#define INPUT_KEY_WALK (1 << 9)
#define INPUT_KEY_USE (1 << 10)
#define INPUT_KEY_FIRE1 (1 << 11)
#define INPUT_KEY_FIRE2 (1 << 12)
#define INPUT_KEY_RELOAD (1 << 13)
#define INPUT_KEY_SPRAY (1 << 14)
#define INPUT_KEY_LIGHT (1 << 15)
#define INPUT_KEY_DISPLAYSCORE (1 << 16)


// player environment
#define ENVIRONMENT_UNKNOWN 0
#define ENVIRONMENT_GROUND 1
#define ENVIRONMENT_MIDAIR 2
#define ENVIRONMENT_SLOSHING 3
#define ENVIRONMENT_WATER 4
#define ENVIRONMENT_LADDER 5


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


// internode special reachabilities
#define REACHABILITY_NORMAL 0
#define REACHABILITY_LADDER (1 << 0)
#define REACHABILITY_FALLEDGE (1 << 1)
#define REACHABILITY_ELEVATOR (1 << 2)
#define REACHABILITY_PLATFORM (1 << 3)
#define REACHABILITY_CONVEYOR (1 << 4)
#define REACHABILITY_TRAIN (1 << 5)
#define REACHABILITY_LONGJUMP (1 << 6)
#define REACHABILITY_SWIM (1 << 7)
#define REACHABILITY_TELEPORTER (1 << 8)
#define REACHABILITY_JUMP (1 << 9)
#define REACHABILITY_CROUCH (1 << 10)
#define REACHABILITY_UNKNOWN1 (1 << 11)
#define REACHABILITY_UNKNOWN2 (1 << 12)
#define REACHABILITY_UNKNOWN3 (1 << 13)
#define REACHABILITY_UNKNOWN4 (1 << 14)
#define REACHABILITY_UNKNOWN5 (1 << 15)


// BSP map file constants
#define MAX_MAP_HULLS 4 // hard limit
#define MAX_MAP_MODELS 400 // variable, but more would stress out the engine and network code
#define MAX_MAP_PLANES 32767 // more than this in a map and the engine will drop faces
#define MAX_MAP_VERTS 65535 // hard limit (data structures store them as unsigned shorts)
#define MAX_MAP_FACES 65535 // hard limit (data structures store them as unsigned shorts)
#define MAX_MAP_EDGES 256000 // arbitrary
#define MAX_MAP_SURFEDGES 512000 // arbitrary
#define MAXLIGHTMAPS 4
#define LUMP_PLANES 1
#define LUMP_VERTEXES 3
#define LUMP_FACES 7
#define LUMP_EDGES 12
#define LUMP_SURFEDGES 13
#define LUMP_MODELS 14
#define HEADER_LUMPS 15


// msec calculation methods
#define METHOD_DEBUG 0
#define METHOD_WHITEHOUSE 1
#define METHOD_HARTWIG 2
#define METHOD_HEIMANN 3


// weapon classes
#define WEAPON_CLASS_PRIMARY 1 // primary (main) weapon
#define WEAPON_CLASS_SECONDARY 2 // side weapon, instant hit capability
#define WEAPON_CLASS_GRENADE 3 // side weapon, explosive


// weapon rail properties
#define WEAPONRAIL_PROPERTY_DISABLER (1 << 0) // disabling effect (e.g, blinds or freezes enemy)
#define WEAPONRAIL_PROPERTY_WATERPROOF (1 << 1) // can be used underwater
#define WEAPONRAIL_PROPERTY_LIGHTDAMAGE (1 << 2) // light damage, almost never lethal
#define WEAPONRAIL_PROPERTY_MEDIUMDAMAGE (1 << 3) // medium damage, may be lethal
#define WEAPONRAIL_PROPERTY_HEAVYDAMAGE (1 << 4) // heavy damage, almost always lethal
#define WEAPONRAIL_PROPERTY_RADIUSEFFECT (1 << 5) // radius effect (e.g, grenade blast)
#define WEAPONRAIL_PROPERTY_AUTOMATIC (1 << 6) // automatic firing capability
#define WEAPONRAIL_PROPERTY_BUCKSHOT (1 << 7) // bucket firing capability (several impacts at once)
#define WEAPONRAIL_PROPERTY_SCOPED (1 << 8) // scoping capability (medium range lens)
#define WEAPONRAIL_PROPERTY_SNIPER (1 << 9) // zooming capability (long range lens)
#define WEAPONRAIL_PROPERTY_SILENCED (1 << 10) // silent shot capability
#define WEAPONRAIL_PROPERTY_MISSILE (1 << 11) // missile (no instant hit)
#define WEAPONRAIL_PROPERTY_HOMING (1 << 12) // homing capability (only relevant on missiles)
#define WEAPONRAIL_PROPERTY_TOSS (1 << 13) // needs to be tossed at target
#define WEAPONRAIL_PROPERTY_PLACE (1 << 14) // needs to be placed (proximity bomb)
#define WEAPONRAIL_PROPERTY_BIT16 (1 << 15) // unused yet


// weapon rail ranges
#define WEAPONRAIL_RANGE_MELEE (1 << 0) // melee only (close combat)
#define WEAPONRAIL_RANGE_CLOSE (1 << 1) // close range (0 - 6 meters)
#define WEAPONRAIL_RANGE_MEDIUM (1 << 2) // medium range (6 - 40 meters)
#define WEAPONRAIL_RANGE_FAR (1 << 3) // long range (> 40 meters)


// bot HAL Markov model order
#define BOT_HAL_MODEL_ORDER 5

// amount of vision cells in the bot's eye
#define BOT_FOV_WIDTH 52

// amount of simultaneous entities bot can distinguish at once
#define BOT_EYE_SENSITIVITY 50

// amount of simultaneous sounds bot can distinguish at once
#define BOT_EAR_SENSITIVITY 8


// transport entity directions of movement
#define TRANSPORT_UNKNOWN 0
#define TRANSPORT_FORWARD 1
#define TRANSPORT_BACKWARDS 2


// common bot orders
#define BOT_ORDER_NOORDER 0
#define BOT_ORDER_REPORT 1
#define BOT_ORDER_FOLLOW 2
#define BOT_ORDER_STAY 3
#define BOT_ORDER_GO 4


// bot goals (foremost top duties in the cognitive hierarchy)
#define BOT_GOAL_NONE 0
#define BOT_GOAL_PICKBOMB (1 << 0)
#define BOT_GOAL_PLANTBOMB (1 << 1)
#define BOT_GOAL_DEFUSEBOMB (1 << 2)
#define BOT_GOAL_PROTECTSITE (1 << 3)
#define BOT_GOAL_REACHSAFETYZONE (1 << 4)
#define BOT_GOAL_ASSASSINATEVIP (1 << 5)
#define BOT_GOAL_RESCUEHOSTAGE (1 << 6)


// bot tasks (sub-elements of goals)
#define BOT_TASK_IDLE 0
#define BOT_TASK_WANDER 1
#define BOT_TASK_FINDPATH 2
#define BOT_TASK_WALKPATH 3
#define BOT_TASK_PICKBOMB 4
#define BOT_TASK_PLANTBOMB 5
#define BOT_TASK_DEFUSEBOMB 6
#define BOT_TASK_USECHARGER 7
#define BOT_TASK_CAMP 8
#define BOT_TASK_AVOIDEXPLOSION 9


// bomb states
#define BOMB_NONE 0
#define BOMB_CARRIED 1
#define BOMB_DROPPED 2
#define BOMB_PLANTED 3


// VIP states
#define VIP_NONE 0
#define VIP_ALIVE 1
#define VIP_DEAD 2


// hostage states
#define HOSTAGE_NONE 0
#define HOSTAGE_ATSTART 1
#define HOSTAGE_HALFWAY 2
#define HOSTAGE_RESCUED 3


// bot chat
#define BOT_CHAT_NONE 0
#define BOT_CHAT_TEXTONLY 1
#define BOT_CHAT_AUDIOONLY 2
#define BOT_CHAT_TEXTAUDIO 3


// bot sayings (text)
#define BOT_SAYTEXT_NOTHING 0
#define BOT_SAYTEXT_HELLO 1
#define BOT_SAYTEXT_BYE 2
#define BOT_SAYTEXT_LAUGH 3
#define BOT_SAYTEXT_WHINE 4
#define BOT_SAYTEXT_ALONE 5
#define BOT_SAYTEXT_NEEDBACKUP 6
#define BOT_SAYTEXT_AFFIRMATIVE 7
#define BOT_SAYTEXT_NEGATIVE 8
#define BOT_SAYTEXT_FOLLOWOK 9
#define BOT_SAYTEXT_STOPOK 10
#define BOT_SAYTEXT_HOLDPOSITIONOK 11
#define BOT_SAYTEXT_CANTFOLLOW 12


// bot sayings (audio)
#define BOT_SAYAUDIO_NOTHING 0
#define BOT_SAYAUDIO_AFFIRMATIVE 1
#define BOT_SAYAUDIO_ALERT 2
#define BOT_SAYAUDIO_ATTACKING 3
#define BOT_SAYAUDIO_FIRSTSPAWN 4
#define BOT_SAYAUDIO_INPOSITION 5
#define BOT_SAYAUDIO_NEGATIVE 6
#define BOT_SAYAUDIO_REPORT 7
#define BOT_SAYAUDIO_REPORTINGIN 8
#define BOT_SAYAUDIO_SEEGRENADE 9
#define BOT_SAYAUDIO_TAKINGDAMAGE 10
#define BOT_SAYAUDIO_THROWGRENADE 11
#define BOT_SAYAUDIO_VICTORY 12


// BSP map file constants
#define MAX_MAP_HULLS 4 // hard limit
#define MAX_MAP_MODELS 400 // variable, but more would stress out the engine and network code
#define MAX_MAP_PLANES 32767 // more than this in a map and the engine will drop faces
#define MAX_MAP_VERTS 65535 // hard limit (data structures store them as unsigned shorts)
#define MAX_MAP_FACES 65535 // hard limit (data structures store them as unsigned shorts)
#define MAX_MAP_EDGES 256000 // arbitrary
#define MAX_MAP_SURFEDGES 512000 // arbitrary
#define MAXLIGHTMAPS 4
#define LUMP_PLANES 1
#define LUMP_VERTEXES 3
#define LUMP_FACES 7
#define LUMP_EDGES 12
#define LUMP_SURFEDGES 13
#define LUMP_MODELS 14
#define HEADER_LUMPS 15


// type of packets in network messages
#define PACKET_BYTE 1
#define PACKET_CHAR 2
#define PACKET_SHORT 3
#define PACKET_LONG 4
#define PACKET_ANGLE 5
#define PACKET_COORD 6
#define PACKET_STRING 7
#define PACKET_ENTITY 8


// engine-related include files
#include <extdll.h>
#include <dllapi.h>
#include <h_export.h>
#include <meta_api.h>
#include <entity_state.h>


// global variables wrapper and null vector definition
#ifdef DEFINE_GLOBALS
#define GLOBAL
const Vector g_vecZero = Vector (0, 0, 0);
#else
#define GLOBAL extern
#endif


#ifndef RACC_H
#define RACC_H


// network messages used by this engine
typedef struct network_message_header_s
{
   bool is_broadcasted; // set to TRUE if the network message is a broadcasted message
   int message_type; // type of message it is
   int player_index; // index of the player this message is for
} network_message_header_t;


typedef struct network_message_s
{
   int packet_type; // type of this packet in the network message
   union
   {
      long iValuePassed; // value passed in the packet, in case it is an integer
      float flValuePassed; // value passed in the packet, in case it is a float
      const char *szValuePassed; // value passed in the packet, in case it is a string
   };
} network_message_t;


// 3D referential structure definition
typedef struct referential_s
{
   Vector v_forward; // absolute coordinates of the FORWARD vector
   Vector v_right; // absolute coordinates of the RIGHT vector
   Vector v_up; // absolute coordinates of the UPWARDS vector
} referential_t;


// hull/line test results structure definition
typedef struct test_result_s
{
   float fraction; // fraction of 1 of the distance completed
   Vector v_endposition; // final position the test could reach
   Vector v_normal; // hit plane normal
   edict_t *pHit; // entity the surface is on
} test_result_t;


// BSP file related stuff
typedef struct bsp_lump_s
{
   int fileofs; // offset of the lump in the BSP file
   int filelen; // length of the lump in the BSP file
} bsp_lump_t;


typedef struct bsp_dmodel_s
{
   Vector mins; // vector location of the lower corner of the model's bounding box
   Vector maxs; // vector location of the upper corner of the model's bounding box
   Vector origin; // vector origin of the model
   int headnode[MAX_MAP_HULLS]; // WTF is that ???
   int visleafs; // not including the solid leaf 0
   int firstface; // index of the model's first face
   int numfaces; // number of faces the model has
} bsp_dmodel_t;


typedef struct bsp_dheader_s
{
   int version; // BSP header version number
   bsp_lump_t lumps[HEADER_LUMPS]; // number of lumps the BSP file has
} bsp_dheader_t;


typedef struct bsp_dvertex_s
{
   Vector point; // vector coordinates of the vertex (point in space delimiting an angle)
} bsp_dvertex_t;


typedef struct bsp_dplane_s
{
   Vector normal; // vector coordinates of the plane normal
   float dist; // distance from the center (0, 0, 0) of the map, I assume
   int type; // plane type, probably whether it is axial or orthogonal or not at all...
} bsp_dplane_t;


typedef struct bsp_dedge_s
{
   unsigned short v[2]; // indices numbers of the vertices bounding this edge, I assume
} bsp_dedge_t;


typedef struct bsp_dface_s
{
   unsigned short planenum; // index number of the face
   short side; // index of the side(s) that are textured (or not) on this face, I assume
   int firstedge; // index number of the first edge of this face
   short numedges; // number of edges this face has
   short texinfo; // texture info for this face, I assume
   unsigned char styles[MAXLIGHTMAPS]; // lightmap styles for this face -- don't ask me more
   int lightofs; // start of [numstyles*surfsize] samples -- light offset ???
} bsp_dface_t;


typedef struct bsp_file_s
{
   bsp_dmodel_t dmodels[MAX_MAP_MODELS]; // map's model data array
   bsp_dplane_t dplanes[MAX_MAP_PLANES]; // map's plane data array
   bsp_dvertex_t dvertexes[MAX_MAP_VERTS]; // map's vertice data array
   bsp_dface_t dfaces[MAX_MAP_FACES]; // map's face data array
   bsp_dedge_t dedges[MAX_MAP_EDGES]; // map's edge data array
   int dsurfedges[MAX_MAP_SURFEDGES]; // map's surfedge data array
} bsp_file_t;


// memory-loaded file structure definition
typedef struct MFILE_s
{
   char *data;
   long read_pointer_index;
   long file_size;
   char path[256];
} MFILE;


// debug levels structure definition
typedef struct debug_level_s
{
   FILE *fp; // pointer to the file where the debug messages are logged
   bool aiconsole; // set to TRUE when the AI console is enabled
   unsigned char eyes; // vision debug level
   unsigned char ears; // hearing debug level
   unsigned char body; // feeling debug level
   unsigned char legs; // movement debug level
   unsigned char hand; // weaponry usage debug level
   unsigned char chat; // chat debug level
   unsigned char cognition; // cognition debug level
   unsigned char navigation; // navigation debug level
   char text_eyes[6][64]; // vision debug text
   char text_ears[6][64]; // hearing debug text
   char text_body[6][64]; // feeling debug text
   char text_legs[6][64]; // movement debug text
   char text_hand[6][64]; // weaponry usage debug text
   char text_chat[6][64]; // chat debug text
   char text_cognition[6][64]; // cognition debug text
   char text_navigation[6][64]; // navigation debug text
   bool is_observer; // set to TRUE if listen server client is in observer mode (bots ignore him)
   bool is_peacemode; // set to TRUE if the bots are told not to shoot each other unless provoked
   bool is_dontfindmode; // set to TRUE if the bots don't check for pickable entities around
   bool is_inhumanturns; // set to TRUE if the bots can face immediately their ideal angles
   bool eyes_disabled; // set to TRUE if the AI is completely blind (doesn't see anything)
   bool ears_disabled; // set to TRUE if the AI is completely deaf (doesn't hear anything)
   bool body_disabled; // set to TRUE if the AI is completely insensitive (doesn't feel anything)
   bool legs_disabled; // set to TRUE if the AI is rendered unable to move
   bool hand_disabled; // set to TRUE if the AI is rendered unable to act and use things
   bool chat_disabled; // set to TRUE if the AI is rendered unable to chat
   bool is_paused; // set to TRUE if all AI is completely frozen (doesn't run at all)
   bool is_broke; // set to TRUE to tell the bot code to break in the current frame
} debug_level_t;


// game configuration structure definitions
typedef struct playerbones_s
{
   char pelvis; // bone number for the "pelvis" bone of players (bottom of spine)
   char spine; // bone number for the "spine" bone of players (bottom 1/4 of spine)
   char spine1; // bone number for the "spine1" bone of players (half of spine)
   char spine2; // bone number for the "spine2" bone of players (bottom 3/4 of spine)
   char spine3; // bone number for the "spine3" bone of players (top of spine, between clavicles)
   char neck; // bone number for the "neck" bone of players (bottom of neck)
   char head; // bone number for the "head" bone of players (center of neck)
   char left_clavicle; // bone number for the "left clavicle" bone of players (left shoulder)
   char left_upperarm; // bone number for the "left upperarm" bone of players (left elbow)
   char left_forearm; // bone number for the "left forearm" bone of players (left wrist)
   char left_hand; // bone number for the "left hand" bone of players (center of hand)
   char left_finger0; // bone number for the "left_finger0" bone of players (half of thumb)
   char left_finger01; // bone number for the "left_finger01" bone of players (extremity of thumb)
   char left_finger1; // bone number for the "left_finger1" bone of players (half of other fingers)
   char left_finger11; // bone number for the "left_finger11" bone of players (extremity of fingers)
   char left_thigh; // bone number for the "left_thigh" bone of players (what it says)
   char left_calf; // bone number for the "left_calf" bone of players (what it says)
   char left_foot; // bone number for the "left_foot" bone of players (extremity of toes)
   char right_clavicle; // bone number for the "right clavicle" bone of players (right shoulder)
   char right_upperarm; // bone number for the "right upperarm" bone of players (right elbow)
   char right_forearm; // bone number for the "right forearm" bone of players (right wrist)
   char right_hand; // bone number for the "right hand" bone of players (center of hand)
   char right_finger0; // bone number for the "right_finger0" bone of players (half of thumb)
   char right_finger01; // bone number for the "right_finger01" bone of players (extremity of thumb)
   char right_finger1; // bone number for the "right_finger1" bone of players (half of other fingers)
   char right_finger11; // bone number for the "right_finger11" bone of players (extremity of fingers)
   char right_thigh; // bone number for the "right_thigh" bone of players (what it says)
   char right_calf; // bone number for the "right_calf" bone of players (what it says)
   char right_foot; // bone number for the "right_foot" bone of players (extremity of toes)
} playerbones_t;


typedef struct game_config_s
{
   char language[32]; // locale to use in game (english, french, german, italian or spanish)
   char mod_name[32]; // name of the game currently running on the server
   char racc_basedir[256]; // relative path to the RACC base directory
   char logfile_path[256]; // relative path to the RACC log file (starting from the basedir)
   char welcomesound_path[256]; // relative path to the RACC welcome sound (starting from basedir)
   char distributor_name[256]; // RACC distributer (should be "Pierre-Marie Baty <pm@bots-united.com>)"
   char distributor_url[256]; // RACC URL (should be "http://racc.bots-united.com")
   float max_walk_speed; // maximum walk step before making audible footstep noises
   float max_safefall_speed; // maximum fall speed above which a player gets landing damage
   float walk_speed_factor; // fraction of 1 of the full speed that is called "walk speed"
   float max_hearing_distance; // maximum distance at which a sound can REASONABLY be heard
   float bb_width; // size of a player's bounding box (width)
   float bb_depth; // size of a player's bounding box (depth)
   float bb_height; // size of a player's bounding box (height)
   float standing_origin_height; // distance from the ground to the origin of a standing player
   float ducking_origin_height; // distance from the ground to the origin of a ducking player
   playerbones_t playerbones; // the player bone numbers database for this game
} game_config_t;


// server structure definition
typedef struct server_s
{
   bool is_dedicated; // set to TRUE if the server is a dedicated (standalone) server
   char map_name[32]; // name of the map currently playing on the server
   int max_clients; // maximum number of connected clients this server allows at a time
   int max_entities; // maximum number of entities this server supports at a time
   bool is_multiplayer; // set to TRUE if the game going on is multiplayer (generally yes, duh)
   bool is_teamplay; // set to TRUE if the game going on is a team play game
   bool does_footsteps; // set to TRUE if the game rules allow footstep sounds
   short developer_level; // the current internal debug level this server is running on
   bool just_booted; // set to TRUE if this server has just booted on a particular map
   float start_time; // time at which the server started
   float time; // pointer to current time on server
   float previous_time; // past time on server, after the last frame has been rendered
   float bot_check_time; // date at which the server will check the bot population
   char msec_method; // method to use to estimate the frame durations
   int msecnum; // used in TheFatal's method for calculating the msecval delay
   float msecdel; // used in TheFatal's method for calculating the msecval delay
   float msecval; // amount of time the movement of the bots (pfnRunPlayerMove) should extend
   int min_bots; // minimal amount of bots that will stay present on the server
   int max_bots; // maximal amount of bots allowed at once on the server
   bool is_autofill; // set to TRUE if bots are automatically filling the server
   bool is_internetmode; // set to TRUE if internet mode (bots join and leave randomly)
   char bot_chat_mode; // type of bot chat allowed on this server
   char bot_forced_team[32]; // non-void if bots should be forced to the specified team
   char server_command[128]; // server command to be issued this frame
} server_t;


// walkable surface structure definition
typedef struct walkface_s
{
   Vector *v_corners; // pointer to array of face edges vector locations (mallocated)
   int corner_count; // number of edges this face has
} walkface_t;


// topologic sector structure definition
typedef struct sector_s
{
   walkface_t **faces; // mallocated array of pointers to walkable faces located in this sector
   int faces_count; // number of walkable faces in this array
} sector_t;


// the virtual world itself
typedef struct map_s
{
   Vector v_worldmins; // quick access to the world's bounding box bottom left corner
   Vector v_worldmaxs; // quick access to the world's bounding box top right corner
   int walkfaces_count; // number of walkable faces in this map
   walkface_t *walkfaces; // pointer to map's walkable faces memory space (mallocated)
   int parallels_count; // number of parallels dividing this map into sectors
   int meridians_count; // number of meridians dividing this map into sectors
   sector_t topology[MAX_MAP_PARALLELS][MAX_MAP_MERIDIANS]; // map spatial topology
} map_t;


// navigation link structure definition
struct navlink_t
{
   struct navnode_t *node_from; // pointer to the navigation node beyond this entrypoint
   short reachability; // type of reachability it is (normal, ladder, edge fall, elevator, etc.)
   Vector v_origin; // vector origin of this link (passage between 2 nodes) in the virtual world
   Vector v_connectvelocity; // connect velocity - I WILL KILL THE GUY WHO ASK ME WHAT IT IS!
};


// navigation node structure definition
struct navnode_t
{
   walkface_t *walkface; // the walkable face this node concerns
   struct navlink_t links[8]; // array of navigation links (entry points from other navnodes)
   char links_count; // number of navigation links in this navigation node

   // dynamic data used by the bot's pathmachine
   struct navnode_t *parent; // pointer to this element's parent in the queue during path search
   struct navlink_t *entrypoint; // pointer to this element's parent navlink that led to this node
   bool is_in_open_list; // set to TRUE if this node is in the pathmachine's OPEN list
   bool is_in_closed_list; // set to TRUE if this node is in the pathmachine's CLOSED list
   float travel_cost; // minimal cost of travel to this node during a path search
   float remaining_cost; // estimated cost of travel from this node to the goal (heuristic)
   float total_cost; // weight of this element in the priority queue (sum of the 2 above)
};


// pathfinding machine
typedef struct pathmachine_s
{
   bool busy; // set to TRUE if the pathfinding machine is busy computing a path
   bool finished; // set to TRUE if the pathfinding machine has finished computing its path.
   navnode_t *goal_node; // pointer to the current goal node (node FROM since reverse pathfinding)
   bool should_update_goal; // set to TRUE if the goal node should be kept updated after bot's position
   navnode_t **open; // pathfinding machine's OPEN list (priority queue of nodes to search)
   int open_count; // number of elements in the OPEN list (fixed-size heap of the queue)
   navlink_t **path; // resulting PATH list (array of pointers to navlinks, so far)
   int path_count; // number of elements in the PATH list (i.e., number of navlinks in the path)
   float path_cost; // cost of the path (variated distance to goal, according to bot's likelevels)
} pathmachine_t;


// WAV file related structure definitions
typedef struct wav_header_s
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


// game mission structure definition
typedef struct mission_s
{
   bool finished; // set to TRUE when the round is over and is about to restart
   float start_time; // date at which the round started
   char bomb; // bomb state, set to one of the BOMB_ states (see #define's far above)
   char vip; // VIP state, set to one of the VIP_ states (see #define's far above)
   char hostages[32]; // array of each hostage's individual states, see #define's.
   int hostage_count; // number of hostages to rescue, when appliable
} mission_t;


// game weapons structure definition
typedef struct weapon_rail_s
{
   short properties; // bitmask of available properties (see WEAPONRAIL_PROPERTY_xx #define's)
   char range; // best range for maximum rail efficiency (WEAPONRAIL_RANGE_xx)
   char type_of_ammo; // type of ammo this rail uses (i.e, index to use in the ammo array)
   short min_ammo; // minimum ammout of ammo this rail needs to fire
   short max_ammo; // maximum amount of ammo this rail can hold
   float charge_delay; // amount of time needed to charge the rail before firing
   char sound1[64]; // relative path to sound sample for normal mode firing in this rail
   char sound2[64]; // relative path to sound sample for alternate mode firing in this rail
   float min_delay[5]; // array of 5 skill-dependent minimal reaction delay values for the AI
   float max_delay[5]; // array of 5 skill-dependent maximal reaction delay values for the AI
} weapon_rail_t;


typedef struct weapon_s
{
   char classname[64]; // the weapon entity classname, in all letters
   char model[64]; // relative path to the weapon model ("models/p_" model)
   int id; // weapon ID
   float weight; // weight of weapon, if appliable
   char weapon_class; // class of weapon (primary, secondary, grenade - see WEAPON_CLASS_xx)
   int price; // price of weapon, if it needs to be bought
   char buy_command[64]; // command to be issued by a player in order to buy this weapon
   weapon_rail_t primary; // primary weapon rail
   weapon_rail_t secondary; // secondary weapon rail
} weapon_t;


// audible events related structure definitions
typedef struct footstepsound_s
{
   char texture_type; // type of texture according to the materials.txt file ('*' catches all)
   float volume; // volume out of 1 at which the sound sample shall be played
   char file_path[64]; // relative path to the footstep sound sample for this type of texture
} footstepsound_t;


typedef struct ricochetsound_s
{
   char texture_type; // type of texture according to the materials.txt file ('*' catches all)
   char file_path[64]; // relative path to the ricochet sound sample for this type of texture
} ricochetsound_t;


typedef struct sound_s
{
   char file_path[64]; // relative path to the sound sample
   float loudness; // violence of the sample (low value for footsteps, high value for gunshot)
   float duration; // amount of time our hearing capabilities will be occupied by this sound
} sound_t;


typedef struct noise_s
{
   char *file_path; // pointer to relative path to sound sample heard (in sound_t array)
   char direction; // relative direction (left, right, front, back) of noise around the listener
   float loudness; // violence of the sample (low value for footsteps, high value for gunshot)
   float fade_date; // date at which this sound will have faded out
} noise_t;


// HAL-related structure definitions
typedef struct HAL_STRING_s
{
   unsigned char length; // length of string (for quick access)
   char *word; // the string itself
} HAL_STRING;


typedef struct HAL_DICTIONARY_s
{
   unsigned long size; // size of dictionary (for quick access)
   HAL_STRING *entry; // array of string entries in that dictionary
   unsigned short *index; // pointer to index
} HAL_DICTIONARY;


typedef struct HAL_SWAP_s
{
   unsigned short size; // size of the word swap structure
   HAL_STRING *from; // array of words to be changed (mallocated)
   HAL_STRING *to; // array of word for replacing the previous ones (mallocated)
} HAL_SWAP;


typedef struct HAL_NODE
{
   unsigned short symbol; // symbol ID
   unsigned long usage; // usage information
   unsigned short count; // usage count (?)
   unsigned short branch; // branch of the tree
   struct HAL_NODE **tree; // pointer to tree node pointer
} HAL_TREE;


typedef struct HAL_MODEL_s
{
   unsigned char order; // model order (complexity)
   HAL_TREE *forward; // pointer to the model's forward tree
   HAL_TREE *backward; // pointer to the model's backwards tree
   HAL_TREE **context; // pointer to context tree pointer
   HAL_DICTIONARY *dictionary; // pointer to the model's dictionary
} HAL_MODEL;


// bot chat resources per language definition
typedef struct text_language_s
{
   char affirmative[100][256]; // text samples per category
   int recent_affirmative[10]; // recently used samples indices in this category
   int affirmative_count; // number of samples in this category
   char negative[100][256]; // text samples per category
   int recent_negative[10]; // recently used samples indices in this category
   int negative_count; // number of samples in this category
   char hello[100][256]; // text samples per category
   int recent_hello[10]; // recently used samples indices in this category
   int hello_count; // number of samples in this category
   char laugh[100][256]; // text samples per category
   int recent_laugh[10]; // recently used samples indices in this category
   int laugh_count; // number of samples in this category
   char whine[100][256]; // text samples per category
   int recent_whine[10]; // recently used samples indices in this category
   int whine_count; // number of samples in this category
   char idle[100][256]; // text samples per category
   int recent_idle[10]; // recently used samples indices in this category
   int idle_count; // number of samples in this category
   char follow[100][256]; // text samples per category
   int recent_follow[10]; // recently used samples indices in this category
   int follow_count; // number of samples in this category
   char stop[100][256]; // text samples per category
   int recent_stop[10]; // recently used samples indices in this category
   int stop_count; // number of samples in this category
   char stay[100][256]; // text samples per category
   int recent_stay[10]; // recently used samples indices in this category
   int stay_count; // number of samples in this category
   char help[100][256]; // text samples per category
   int recent_help[10]; // recently used samples indices in this category
   int help_count; // number of samples in this category
   char cant[100][256]; // text samples per category
   int recent_cant[10]; // recently used samples indices in this category
   int cant_count; // number of samples in this category
   char bye[100][256]; // text samples per category
   int recent_bye[10]; // recently used samples indices in this category
   int bye_count; // number of samples in this category
} text_language_t;


typedef struct audio_language_s
{
   int affirmative_count; // number of samples in this category
   int alert_count; // number of samples in this category
   int attacking_count; // number of samples in this category
   int firstspawn_count; // number of samples in this category
   int inposition_count; // number of samples in this category
   int negative_count; // number of samples in this category
   int report_count; // number of samples in this category
   int reporting_count; // number of samples in this category
   int seegrenade_count; // number of samples in this category
   int takingdamage_count; // number of samples in this category
   int throwgrenade_count; // number of samples in this category
   int victory_count; // number of samples in this category
} audio_language_t;


typedef struct bot_language_s
{
   char language[32]; // language in full letters (also the language subdirectory name)
   text_language_t text; // text language resources
   audio_language_t audio; // audio language resources
} bot_language_t;


// fov line structure definition
typedef struct fov_line_s
{
   Vector scan_angles; // angles describing the traceline direction for this scan element
   float distance; // distance to first obstacle when tracing at scan_angles
   Vector vecEndPos; // location of first obstacle when tracing at scan_angles
   Vector Normal; // normal of first obstacle surface when tracing at scan_angles (to describe the plane)
   edict_t *pHit; // pointer to obstacle entity
} fov_line_t;


// chat string structure definition
typedef struct chat_s
{
   bool new_message; // set to TRUE if the chat messages have just scrolled down
   int sender_index; // index of the chat line's sender player structure in the players array
   char text[128]; // the actual chat text (in uppercase)
} chat_t;


// player HUD structure definition
typedef struct hud_s
{
   chat_t chat; // chat message one can see on the screen
   int menu_state; // VGUI menu state on this HUD
   char icons_state[32]; // array of icons states (either off, lit, or blinking)
   bool has_progress_bar; // set to TRUE when a progress bar is displayed on the HUD
} hud_t;


// bot-related structure definitions
typedef struct profile_s
{
   char name[32]; // the bot's name
   char skin[32]; // the bot's preferred skin
   char logo[32]; // the bot's preferred logo
   char nationality[32]; // the bot's nationality
   int skill; // the bot's skill
   int team; // the bot's team stored as an integer for team-based MODs
   int subclass; // the bot's class (i.e. skin) for class-based MODs
} profile_t;


typedef struct bot_weapon_s
{
   float worth; // given situation, percentage describing suitability of weapon compared to others
   weapon_t *hardware; // pointer to the weapon data structure corresponding to this weapon
   short clip_ammo; // amount of ammo currently in the clip
   short *primary_ammo; // pointer to amount of ammo currently in reserve for rail 1
   short *secondary_ammo; // pointer to amount of ammo currently in reserve for rail 2
   float primary_charging_time; // date at which the bot will stop charging the primary rail
   float secondary_charging_time; // date at which the bot will stop charging the secondary rail
} bot_weapon_t;


typedef struct bot_enemy_s
{
   float worth; // given situation, percentage describing the danger this enemy causes to bot
   edict_t *pEdict; // pointer to this enemy's entity
   Vector v_targetpoint; // location where the bot should aim for this enemy
   float appearance_time; // date at which the bot first saw this enemy
   float disappearance_time; // date at which the bot lost this enemy
   bool is_hiding; // set to TRUE if the bot had the impression that this enemy went hiding
} bot_enemy_t;


// sensitive part of the AI
typedef struct bot_eyes_s
{
   float sample_time; // date at which the bot will next sample his field of view
   float blinded_time; // date under which the bot is blinded by too much light (flashbang...)
   fov_line_t BotFOV[BOT_FOV_WIDTH]; // bot's field of view is BOT_FOV_WIDTH elements width
   Vector v_capture_point; // spatial location of the "eye" of the FOV when the capture was taken
   edict_t *pEntitiesInSight[BOT_EYE_SENSITIVITY]; // entities the bot distinguishes at a time
   int entity_count; // number of entities currently registered in the pEntitiesInSight array
   hud_t BotHUD; // the bot's Head Up Display (where icons and chat messages are)
} bot_eyes_t;


typedef struct bot_ears_s
{
   float average_noise; // average loudness the bot hears (average of noises.loudness)
   noise_t noises[BOT_EAR_SENSITIVITY]; // sounds the bot distinguishes at a time
   bool new_sound; // set to TRUE when a new sound has just arrived in the bot's ear
   char new_sound_index; // slot index of the new sound that has just arrived in the bot's ear
   int bot_order; // integer representing the current order from a teammate bot is following
   float order_time; // date at which the bot received its last order
   edict_t *pAskingEntity; // pointer to the bot's current giving order entity
} bot_ears_t;


typedef struct bot_body_s
{
   float left_check_dst; // distance on the left at which the terrain will be checked for obstacles
   float front_check_dst; // distance in front at which the terrain will be checked for obstacles
   float right_check_dst; // distance on the right at which the terrain will be checked for obstacles
   unsigned short hit_state; // bit map description of the terrain in the surroundings of the bot
   unsigned short prev_hit_state; // the same description of surrounding terrain one frame before
   float fall_time; // date at which the bot last felt fast enough to get some damage
   Vector v_fall_plane_normal; // vector describing the normal of a fall edge in front of bot
} bot_body_t;


// motile part of the AI
typedef struct bot_legs_s
{
   float forward_time; // date under which the bot will be pressing the FORWARD key
   float backwards_time; // date under which the bot will be pressing the BACKWARDS key
   float jump_time; // date under which the bot will be pressing the JUMP key
   float duck_time; // date under which the bot will be pressing the DUCK key
   float strafeleft_time; // date under which the bot will be pressing the STRAFE LEFT key
   float straferight_time; // date under which the bot will be pressing the STRAFE RIGHT key
   float walk_time; // date under which the bot will be pressing the WALK key
   bool emergency_walkback; // set to TRUE when the bot should override any forward/back movement
   float forward_speed; // the bot's ideal forward movement speed inside the current frame
   float strafe_speed; // the bot's ideal strafe right movement speed for the current frame
   int path_index; // index of the next navlink in the path array the bot is heading to
//   float nextlink_distance; // current distance to the next navlink in the path array
//   float nextlink_distance_updatetime; // next date at which this distance will be updated
   float avoid_teammates_time; // next date at which the bot will think about players to avoid
   float avoid_obstacles_time; // next date at which the bot will think about obstacles to avoid
} bot_legs_t;


typedef struct bot_hand_s
{
   Vector ideal_angles; // NEVER TOUCH THIS DIRECTLY -- use BotSetIdealAngles()
   Vector turn_speed; // the bot's turn speed inside the current frame
   float fire1_time; // date under which the bot will be pressing the PRIMARY FIRE key
   float fire2_time; // date under which the bot will be pressing the SECONDARY FIRE key
   float reload_time; // date under which the bot will be pressing the RELOAD key
   float use_time; // date under which the bot will be pressing the USE key
   float light_time; // date under which the bot will be pressing the LIGHT key
   float spray_time; // date under which the bot will be pressing the SPRAY key
   float displayscore_time; // date under which the bot will be pressing the SCORES key
} bot_hand_t;


typedef struct bot_chat_s
{
   short bot_saytext; // #define BOT_SAYTEXT_ of what the bot is about to say (text)
   float saytext_time; // date at which the bot will have finished typing text on keyboard
   char saytext_message[128]; // string containing the text the bot is about to say
   short bot_sayaudio; // #define BOT_SAYAUDIO_ of what the bot is about to say (audio)
   float sayaudio_time; // date at which the bot will have finished fiddling with microphone
   char sayaudio_message[128]; // string containing the sound sample the bot is about to speak
   float speaker_time; // date at which the bot will stop talking into its microphone
} bot_chat_t;


typedef struct likelevel_s
{
   float ladder; // reachability #1: this bot's like level of ladders
   float falledge; // reachability #2: this bot's like level of falls
   float elevator; // reachability #3: this bot's like level about elevators
   float platform; // reachability #4: this bot's like level of bobbing platforms
   float conveyor; // reachability #5: this bot's like level of conveyors (belts and travolators)
   float train; // reachability #6: this bot's like level of trains
   float longjump; // reachability #7: this bot's like level of long jump modules
   float swim; // reachability #8: this bot's like level of deep water
   float teleporter; // reachability #9: this bot's like level of teleporters
   float jump; // reachability #10: this bot's like level of jumps
   float crouch; // reachability #11: this bot's like level of crouched passages
   float unknown1; // reachability #12
   float unknown2; // reachability #13
   float unknown3; // reachability #14
   float unknown4; // reachability #15
   float unknown5; // reachability #16
} likelevel_t;


typedef struct bot_brain_s
{
   HAL_DICTIONARY *banned_keywords; // dictionary of words that must never be used as keywords
   HAL_DICTIONARY *auxiliary_keywords; // dictionary of auxiliary keywords
   HAL_SWAP *swappable_keywords; // array of swappable keywords with their equivalences
   HAL_MODEL HAL_model; // Markov model of the bot
   HAL_DICTIONARY *input_words; // global chat's dictionary of words
   HAL_DICTIONARY *bot_words; // bot's own dictionary of words
   HAL_DICTIONARY *keys; // bot's own temporary dictionary of keywords
   HAL_DICTIONARY *replies; // bot's own temporary dictionary of replies
   bool keyword_is_used; // set to TRUE when the first direction generated reply uses the keyword
   navnode_t *PathMemory; // pointer to the array of navigation nodes the bot remembers
   pathmachine_t PathMachine; // this bot's pathfinding machine
   likelevel_t likelevel; // this bot's likeness of various obstacles
   short bot_goal; // bitmap of the current goals the bot is actually pursuing
   short bot_task; // the current task the bot is actually executing in order to reach its goal
   float listen_time; // date at which the bot will STOP listening for a particular noise
} bot_brain_t;


// the bot itself
typedef struct bot_s
{
   profile_t *pProfile; // pointer to this bot's profile in the profiles array
   edict_t *pIllumination; // dummy entity for getting the correct bot illumination (engine bug)
   bool is_controlled; // set to TRUE if this bot is currently controlled by a player (debug feature)
   bool not_started;
   float time_to_live;
   float quit_game_time;

   char buy_state; // state machine state for the bot's buy procedures
   float buy_time; // date at which the bot will issue the next buy command (TODO: true reaction time)

   // input channels of the AI
   bot_eyes_t BotEyes; // structure describing what the bot sees
   bot_ears_t BotEars; // structure describing what the bot hears
   bot_body_t BotBody; // structure describing what the bot touches (or is about to)

   // output channels of the AI
   bot_legs_t BotLegs; // structure handling the directions in which the bot will move/jump/duck
   bot_hand_t BotHand; // structure handling the weapon the bot is holding in its hands
   bot_chat_t BotChat; // structure handling what the bot has to say and when it will say it

   // core center of the AI
   bot_brain_t BotBrain; // structure containing what the bot knows about itself and the world

   float finditem_time;
   bool is_picking_item;

   edict_t *pTransportEntity;
   int transport_type;
   char transport_direction;
   float start_use_transport_time;
   float end_use_transport_time;

   bool is_fearful;
   float average_velocity; // average velocity of this bot over the past 0.5 seconds
   int average_velocity_frames_count; // number of frames the above average elapsed over
   bool is_stuck; // set to TRUE if the bot is not moving as fast as it would like to
   float check_stuck_time; // date under which the bot won't check for being stuck at all

   bot_enemy_t BotEnemy; // structure describing the bot's current enemy in sight
   bot_enemy_t LastSeenEnemy; // structure describing the state of the last enemy in sight
   int victim_index; // index of the bot's last victim's player structure in the players array
   int killer_index; // index of the bot's last killer's player structure in the players array

   float randomturn_time;
   Vector v_place_to_keep; // location the bot intends to stay at
   float place_time; // date at which the bot last saw its last location to stay at

   float bot_alone_time;
   bool already_asked_help;

   float rush_time;
   float pause_time;
   float nextpause_time;
   float checkfootsteps_time;

   Vector v_goal;
   float findpath_time;
   navlink_t *previous_navlink; // pointer to the previous navlink from the path the bot is following
   navlink_t *current_navlink; // pointer to the current navlink from the path the bot is following
   navlink_t *next_navlink; // pointer to the next navlink in the path the bot is following
   float fallcheck_time; // next date at which the bot will worry about falls in its neighbourhood
   float cornercheck_time; // next date at which the bot will worry about corners on its sides
   bool is_walking_straight;
   float getreachpoint_time; // date at which the bot will determine a new reach point
   Vector v_reach_point; // location the bot is immediately heading to
   float reach_time; // date under which the bot should not worry about heading to the above

   bool is_interacting;
   float interact_time;
   edict_t *pInteractiveEntity;
   float use_station_time;

   bool is_lift_moving;
   float spraylogo_time;
   bool has_sprayed_logo;

   bool has_valuable;
   noise_t LastSuspiciousSound;

   bot_weapon_t bot_weapons[MAX_WEAPONS]; // array of weapons the bot has
   bot_weapon_t *current_weapon; // pointer to the slot corresponding to the bot's current weapon
   short bot_ammos[MAX_WEAPONS]; // array of ammunition types the bot has
   float shoot_time; // date under which the bot should not fire its weapon
   float reload_time; // date at which the bot will press the RELOAD key to reload its weapon
   int bot_grenades_1; // first grenades amount
   int bot_grenades_2; // second grenades amount
   float throwgrenade_time;
} bot_t;


// previous values for player structure members
typedef struct player_prev_s
{
   #include "player.h" // this struct's members are read from an include file
} player_prev_t;

// player structure definition
typedef struct player_s
{
   #include "player.h" // this struct's members are read from an include file
   player_prev_t prev; // previous values for the above
   bot_t Bot; // bot structure attached to this player slot, used in case player is a bot
} player_t;


// bot_navigation.cpp function prototypes
void BotLookAt (player_t *pPlayer, Vector v_location);
void BotSetIdealAngles (player_t *pPlayer, Vector ideal_angles);
void BotSetIdealPitch (player_t *pPlayer, float ideal_pitch);
void BotSetIdealYaw (player_t *pPlayer, float ideal_yaw);
void BotAddIdealPitch (player_t *pPlayer, float pitch_to_add);
void BotAddIdealYaw (player_t *pPlayer, float yaw_to_add);
void BotPointGun (player_t *pPlayer);
void BotUseHand (player_t *pPlayer);
void BotMove (player_t *pPlayer);
void BotOnLadder (player_t *pPlayer);
void BotUnderWater (player_t *pPlayer);
void BotUseLift (player_t *pPlayer);
bool BotCanUseInteractives (player_t *pPlayer);
void BotInteractWithWorld (player_t *pPlayer);
void BotTurnAtFall (player_t *pPlayer);
bool BotCantSeeForward (player_t *pPlayer);
void BotTurnTowardsDirection (player_t *pPlayer, char direction);
void BotRandomTurn (player_t *pPlayer);
void BotFindTransportEntity (player_t *pPlayer, int transport_type);
void BotCamp (player_t *pPlayer);
void BotCheckForCorners (player_t *pPlayer);
void BotWalkPath (player_t *pPlayer);
void BotWander (player_t *pPlayer);
char BotEstimateDirection (player_t *pPlayer, Vector v_location);
void BotMoveTowardsReachPoint (player_t *pPlayer);
bool BotReachPosition (player_t *pPlayer, Vector v_position);
void BotFindReachPoint (player_t *pPlayer);
void BotUnstuck (player_t *pPlayer);
void BotAvoidTeammates (player_t *pPlayer);
void BotAvoidObstacles (player_t *pPlayer);
bool BotCanSeeThis (player_t *pPlayer, Vector v_destination);
bool BotCanCampNearHere (player_t *pPlayer, Vector v_here);
void BotNavLoadBrain (player_t *pPlayer);
void BotNavSaveBrain (player_t *pPlayer);

// bot_eyes.cpp function prototypes
void BotSee (player_t *pPlayer);
edict_t *BotDoesSee (player_t *pPlayer, const char *model);
Vector BotCanSeeOfEntity (player_t *pPlayer, edict_t *pEntity);

// bot_ears.cpp function prototypes
void BotHear (player_t *pPlayer);
noise_t *BotDoesHear (player_t *pPlayer, const char *sample);
void BotFeedEar (player_t *pPlayer, sound_t *sound, Vector v_origin, float volume);
void DispatchSound (const char *sample, Vector v_origin, float volume, float attenuation);
void PlayBulletSoundsForBots (player_t *pPlayer);
void InitSounds (void);
void InitFootstepSounds (void);
void InitRicochetSounds (void);
void EvaluateSoundForBots (const char *sound_path);
sound_t *FindSoundByFilename (const char *sound_filename);

// bot_body.cpp function prototypes
void BotTouch (player_t *pPlayer);
void BotCheckForObstacles (player_t *pPlayer);
void BotCheckIfStuck (player_t *pPlayer);

// bot_chat.cpp function prototypes
void BotChat (player_t *pPlayer);
void BotSayHAL (player_t *pPlayer);
void BotSayText (player_t *pPlayer);
void BotSayAudio (player_t *pPlayer);
void BotStartTalking (player_t *pPlayer);
void BotStopTalking (player_t *pPlayer);
void DisplaySpeakerIcon (player_t *pPlayer, player_t *pViewerClient);
void DestroySpeakerIcon (player_t *pPlayer, player_t *pViewerClient);
player_t *RandomPlayerOtherThan (player_t *pOtherPlayer, bool want_enemy, bool want_alive);
const char *Name (const char *string);
const char *HumanizeChat (const char *string);
const char *StripBlanks (const char *string);
const char *StripTags (const char *string);
const char *NormalizeChars (const char *string);
char *UpperCase (const char *string);
char *LowerCase (const char *string);
void HAL_LoadTree (MFILE *file, HAL_TREE *node);
void HAL_LoadDictionary (MFILE *file, HAL_DICTIONARY *dictionary);
void HAL_SaveTree (FILE *file, HAL_TREE *node);
void HAL_SaveDictionary (FILE *file, HAL_DICTIONARY *dictionary);
void HAL_Learn (HAL_MODEL *model, HAL_DICTIONARY *words);
unsigned short HAL_AddWord (HAL_DICTIONARY *dictionary, HAL_STRING word);
bool HAL_SearchDictionary (HAL_DICTIONARY *dictionary, HAL_STRING word, int *position);
unsigned short HAL_FindWord (HAL_DICTIONARY *dictionary, HAL_STRING word);
int HAL_CompareWords (HAL_STRING word1, HAL_STRING word2);
void HAL_InitializeDictionary (HAL_DICTIONARY *dictionary);
HAL_DICTIONARY *HAL_NewDictionary (void);
HAL_TREE *HAL_NewNode (void);
void HAL_UpdateModel (HAL_MODEL *model, int symbol);
void HAL_UpdateContext (HAL_MODEL *model, int symbol);
HAL_TREE *HAL_AddSymbol (HAL_TREE *tree, unsigned short symbol);
HAL_TREE *HAL_FindSymbol (HAL_TREE *node, int symbol);
HAL_TREE *HAL_FindSymbolAdd (HAL_TREE *node, int symbol);
void HAL_AddNode (HAL_TREE *tree, HAL_TREE *node, int position);
bool HAL_SearchNode (HAL_TREE *node, int symbol, int *position);
void HAL_InitializeContext (HAL_MODEL *model);
void BotHALTrainModel (player_t *pPlayer, HAL_MODEL *model);
void HAL_ShowDictionary (HAL_DICTIONARY *dictionary);
void HAL_MakeWords (char *input, HAL_DICTIONARY *words);
bool HAL_BoundaryExists (char *string, int position);
const char *BotHALGenerateReply (player_t *pPlayer);
bool HAL_DictionariesDiffer (HAL_DICTIONARY *words1, HAL_DICTIONARY *words2);
HAL_DICTIONARY *BotHALMakeKeywords (player_t *pPlayer, HAL_DICTIONARY *words);
void BotHALAddKeyword (player_t *pPlayer, HAL_DICTIONARY *keys, HAL_STRING word);
void BotHALAddAuxiliaryKeyword (player_t *pPlayer, HAL_DICTIONARY *keys, HAL_STRING word);
HAL_DICTIONARY *BotHALBuildReplyDictionary (player_t *pPlayer, HAL_DICTIONARY *keys);
int BotHALBabble (player_t *pPlayer, HAL_DICTIONARY *keys, HAL_DICTIONARY *words);
bool HAL_WordExists (HAL_DICTIONARY *dictionary, HAL_STRING word);
int BotHALSeedReply (player_t *pPlayer, HAL_DICTIONARY *keys);
HAL_SWAP *HAL_NewSwap (void);
void HAL_AddSwap (HAL_SWAP *list, char *s, char *d);
HAL_SWAP *HAL_InitializeSwap (char *filename);
HAL_DICTIONARY *HAL_InitializeList (char *filename);
void HAL_EmptyDictionary (HAL_DICTIONARY *dictionary);
void HAL_FreeDictionary (HAL_DICTIONARY *dictionary);
void HAL_EmptyModel (HAL_MODEL *model);
void HAL_FreeModel (HAL_MODEL *model);
void HAL_FreeTree (HAL_TREE *tree);
void HAL_FreeSwap (HAL_SWAP *swap);
void BotHALLoadBrain (player_t *pPlayer);
void BotHALSaveBrain (player_t *pPlayer);

// pathmachine.cpp function prototypes
void BotInitPathMachine (player_t *pPlayer);
void BotRunPathMachine (player_t *pPlayer);
void BotShutdownPathMachine (player_t *pPlayer);
bool BotFindPathTo (player_t *pPlayer, Vector v_goal, bool urgent);
bool BotFindPathFromTo (player_t *pPlayer, Vector v_start, Vector v_goal, bool urgent);
float EstimateTravelCostFromTo (navnode_t *node_from, navnode_t *node_to);
float BotEstimateTravelCost (player_t *pPlayer, navlink_t *link_from, navlink_t *link_to);
void PushToOpenList (pathmachine_t *pathmachine, navnode_t *queue_element);
navnode_t *PopFromOpenList (pathmachine_t *pathmachine);

// mapdata.cpp function prototypes
void LookDownOnTheWorld (void);
Vector GetDFaceCornerByIndex (bsp_dface_t *dface, int corner_index);
bool WalkfaceBelongsToSector (const walkface_t *pFace, int sector_i, int sector_j);
bool LoadWorldMap (void);
int SaveWorldMap (int bsp_file_size);
void InsertNavLink (navnode_t *node, navnode_t *node_from, Vector v_origin, short reachability, Vector v_connectvelocity);
void RemoveNavLink (navnode_t *node, navlink_t *bad_link);
void ShowTheWayAroundToBots (player_t *pPlayer);
sector_t *SectorUnder (Vector v_location);
walkface_t *WalkfaceUnder (player_t *pPlayer);
walkface_t *WalkfaceAt (Vector v_location);
int WalkfaceIndexOf (walkface_t *walkface);
Vector WalkfaceCenterOf (walkface_t *walkface);
void FreeMapData (void);

// weapons.cpp function prototypes
void InitWeapons (void);
weapon_t *FindWeaponByName (const char *weapon_name);
weapon_t *FindWeaponByModel (const char *weapon_model);
weapon_t *FindWeaponById (const int weapon_id);
int WeaponIndexOf (weapon_t *weapon);
bool PlayerHasWeaponOfClass (player_t *pPlayer, char weapon_class);
bool PlayerHoldsWeaponOfClass (player_t *pPlayer, char weapon_class);
bool ItemIsWeaponOfClass (edict_t *pItem, char weapon_class);
bool BotSelectWeaponOfClass (player_t *pPlayer, char weapon_class);
bool PlayerHasWeaponOfType (player_t *pPlayer, short weaponrail_property);
bool PlayerHoldsWeaponOfType (player_t *pPlayer, short weaponrail_property);
bool ItemIsWeaponOfType (edict_t *pItem, short weaponrail_property);
bool BotSelectWeaponOfType (player_t *pPlayer, short weaponrail_property);
bool PlayerHasWeaponOfRange (player_t *pPlayer, char weaponrail_range);
bool PlayerHoldsWeaponOfRange (player_t *pPlayer, char weaponrail_range);
bool ItemIsWeaponOfRange (edict_t *pItem, char weaponrail_range);
bool BotSelectWeaponOfRange (player_t *pPlayer, char weaponrail_range);
int BotRateWeapon (player_t *pPlayer, weapon_t *weapon);

// mfile.cpp function prototypes
MFILE *mfopen (const char *file_path, const char *mode);
long mftell (MFILE *fp);
int mfseek (MFILE *fp, long offset, int offset_mode);
int mfseekAtSection (MFILE *fp, const char *section_name);
int mfseekAfterSection (MFILE *fp, const char *section_name);
int mfeof (MFILE *fp);
size_t mfread (void *destination, size_t block_size, size_t num_blocks, MFILE *fp);
int mfgetc (MFILE *fp);
const char *mfgets (char *line_buffer, int buffer_size, MFILE *fp);
void mfclose (MFILE *fp);

// lrand.cpp function prototypes
void lsrand (unsigned long initial_seed);
long lrand (void);
long RandomLong (long from, long to);
float RandomFloat (float from, float to);

// dxffile.cpp function prototypes
void InitDebugDXF (void);
void DrawLineInDebugDXF (const Vector v_from, const Vector v_to, unsigned char color, const char *layer_name);
void DrawWalkfaceInDebugDXF (const walkface_t *walkface, unsigned char color, const char *layer_name);
void DrawSectorInDebugDXF (int sector_i, int sector_j, unsigned char color, const char *layer_name);
void WriteDebugDXF (const char *filename);

// bmpfile.cpp function prototypes
void InitDebugBitmap (void);
void DrawLineInDebugBitmap (const Vector v_from, const Vector v_to, unsigned char color);
void DrawWalkfaceInDebugBitmap (const walkface_t *walkface, unsigned char color);
void DrawSectorInDebugBitmap (int sector_i, int sector_j, unsigned char color);
void WriteDebugBitmap (const char *filename);

// display.cpp function prototypes
void UTIL_DrawDots (Vector start, Vector end);
void UTIL_DrawLine (Vector start, Vector end, int life, unsigned char red, unsigned char green, unsigned char blue);
void UTIL_DrawBox (Vector bbmin, Vector bbmax, int life, unsigned char red, unsigned char green, unsigned char blue);
void UTIL_DrawWalkface (walkface_t *pFace, int life, unsigned char red, unsigned char green, unsigned char blue);
void UTIL_DrawNavlink (navlink_t *pLink, int life);
void UTIL_DrawPath (pathmachine_t *pPathmachine);
void UTIL_DrawSector (sector_t *pSector, int life, unsigned char red, unsigned char green, unsigned char blue);
int AIConsole_printf (char channel, char pos, const char *fmt, ...);
void DisplayAIConsole (player_t *pPlayer);
void DisplayHUDText (char channel, float x, float y, unsigned char r, unsigned char g, unsigned char b, const char *string);

// console.cpp function prototypes
void ServerCommand (void);

// math.cpp function prototypes
void BuildReferential (const Vector &v_angles);
void BuildPlayerReferential (const Vector &v_angles, player_t *pPlayer);
Vector VecToAngles (const Vector &v_forward);
float WrapAngle (float angle);
float WrapAngle360 (float angle);
Vector WrapAngles (Vector &angles);
Vector WrapAngles360 (Vector &angles);
float AngleOfVectors (Vector v1, Vector v2);

// util.cpp function prototypes
player_t *CreateFakeClient (profile_t *pProfile);
void MoveFakeClient (player_t *pPlayer);
edict_t *FindEntityInSphere (edict_t *pStartEntity, const Vector &v_center, float radius);
edict_t *FindEntityByString (edict_t *pStartEntity, const char *keyword, const char *value);
test_result_t PlayerTestLine (player_t *pPlayer, const Vector &vecStart, const Vector &vecEnd);
test_result_t PlayerTestHull (player_t *pPlayer, const Vector &vecStart, const Vector &vecEnd, bool crouching_player);
test_result_t TestVisibility (const Vector &vecStart, const Vector &vecEnd, edict_t *pTarget);
bool PlayerCanReach (player_t *pPlayer, Vector v_destination);
bool PlayerAimIsOver (player_t *pPlayer, edict_t *pTarget);
bool IsAtHumanHeight (Vector v_location);
Vector DropAtHumanHeight (Vector v_location);
Vector DropToFloor (Vector v_location);
/*inline */Vector GetGunPosition (edict_t *pEdict);
/*inline */Vector OriginOf (edict_t *pEdict);
/*inline */Vector BottomOriginOf (edict_t *pEdict);
/*inline */Vector ReachableOriginOf (edict_t *pEdict);
int printf (const char *fmt, ...);
int ServerConsole_printf (const char *fmt, ...);
void TerminateOnError (const char *fmt, ...);
void InitGameLocale (void);
void InitLogFile (void);
void LogToFile (const char *fmt, ...);
void InitPlayerBones (void);
void InitDefaultLikelevels (void);
void LoadBotProfiles (void);
void PrecacheStuff (void);
void SpawnDoor (edict_t *pDoorEntity);
void FakeClientCommand (edict_t *pFakeClient, const char *fmt, ...);
const char *GetField (const char *string, int field_number);
const char *GetConfigKey (const char *config_string);
const char *GetConfigValue (const char *config_string);
int GetUserMsgId (const char *msg_name);
const char *GetUserMsgName (int msg_type);
void EstimateNextFrameDuration (void);
void APlayerHasConnected (player_t *pPlayer);
void APlayerHasDisconnected (player_t *pPlayer);
void SendWelcomeMessage (player_t *pPlayer);
void MakeVersion (void);
bool IsMyBirthday (void);
bool FileExists (const char *pathname);
void DllAttaching (void);
void DllDetaching (void);
float ProcessTime (void);
bool IsValidPlayer (player_t *pPlayer);
bool IsAlive (edict_t *pEdict);
bool IsBreakable (edict_t *pEdict);
bool IsInFieldOfView (edict_t *pEdict, Vector v_location);
float IlluminationOf (edict_t *pEdict);
bool IsInvisible (edict_t *pEdict);
unsigned char EnvironmentOf (edict_t *pEdict);
unsigned long InputButtonsOf (edict_t *pEdict);
void TheServerHasJustStarted (void);
void FreeAllTheStuff (void);


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
#elif TS_DLL
#include "../ts/mod_specific.h"
#elif VALVE_DLL
#include "../valve/mod_specific.h"
#else
#error Unrecognized MOD (must define a MOD_DLL - cf. bottom of racc.h)
#endif


#endif // RACC_H

