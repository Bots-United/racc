// RACC - AI development project for first-person shooter games derived from Valve's Half-Life
// (http://www.racc-ai.com/)
//
// The game to engine interfacing code is based on the work done by Jeffrey 'botman' Broome
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
// racc.h
//


// bot version globalized in mod_specific.h, and generated in MakeVersion()


// maximum number of players in the game at the same time the engine can support
#define RACC_MAX_CLIENTS 32

// maximum number of bot personalities
#define RACC_MAX_PROFILES 100

// maximum number of different weapons the bots can use (arbitrary)
#define RACC_MAX_WEAPONS 100 // already defined in the Half-Life SDK (cldll_dll.h)

// maximum number of user messages the engine can register (hardcoded in the engine)
#define RACC_MAX_USERMSGS 256

// maximum number of sounds the engine can precache at the same time
#define RACC_MAX_SOUNDS (512 + RACC_MAX_LOCAL_SOUNDS)

// maximum number of gunshot sounds the bots can distinguish
#define RACC_MAX_WEAPONSOUNDS 100

// maximum number of ricochet sounds the bots can distinguish
#define RACC_MAX_RICOCHETSOUNDS 100

// maximum distance at which a sound can be heard in the virtual world
#define MAX_HEARING_DISTANCE 3250

// maximum number of chat messages that can be seen on the screen at once in Counter-Strike
#define MAX_CHAT_MESSAGES 4


// third-party bot entity flag
#define FL_THIRDPARTYBOT (1 << 27)


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


// types of damage to ignore
#define IGNORE_DAMAGE (DMG_CRUSH | DMG_FREEZE | DMG_FALL | DMG_SHOCK | \
                       DMG_DROWN | DMG_NERVEGAS | DMG_RADIATION | \
                       DMG_DROWNRECOVER | DMG_ACID | DMG_SLOWBURN | \
                       DMG_SLOWFREEZE | 0xFF000000)


// map topology granularity
#define MAX_MAP_PARALLELS 64
#define MAX_MAP_MERIDIANS 64


// internode special reachabilities
#define REACHABILITY_LADDER (1 << 0)
#define REACHABILITY_FALLEDGE (1 << 1)
#define REACHABILITY_ELEVATOR (1 << 2)
#define REACHABILITY_PLATFORM (1 << 3)
#define REACHABILITY_CONVEYOR (1 << 4)
#define REACHABILITY_TRAIN (1 << 5)
#define REACHABILITY_LONGJUMP (1 << 6)


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
#define METHOD_PM 1
#define METHOD_RICH 2
#define METHOD_LEON 3
#define METHOD_TOBIAS 4


// bot nationalities
#define NATIONALITY_ENGLISH 0
#define NATIONALITY_FRENCH 1
#define NATIONALITY_GERMAN 2
#define NATIONALITY_ITALIAN 3
#define NATIONALITY_SPANISH 4


// bot HAL Markov model order
#define BOT_HAL_MODEL_ORDER 5

// amount of simultaneous entities bot can distinguish at once
#define BOT_EYE_SENSITIVITY 50

// amount of simultaneous sounds bot can distinguish at once
#define BOT_EAR_SENSITIVITY 8


#define LADDER_UNKNOWN 0
#define LADDER_UP 1
#define LADDER_DOWN 2


// common bot orders
#define BOT_ORDER_NOORDER 0
#define BOT_ORDER_REPORT 1
#define BOT_ORDER_FOLLOW 2
#define BOT_ORDER_STAY 3
#define BOT_ORDER_GO 4


// bot tasks
#define BOT_TASK_IDLE 0
#define BOT_TASK_PLANTING (1 << 0)
#define BOT_TASK_DEFUSING (1 << 1)
#define BOT_TASK_USINGCHARGER (1 << 2)


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
#define BOT_SAYAUDIO_REPORTING 8
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


// generic include files
#include <time.h>


// engine-related include files
#include "extdll.h"
#include "enginecallback.h"
#include "util.h"
#include "cbase.h"
#include "entity_state.h"
#include "pm_materials.h"
#include "usercmd.h"
#include "studio.h"


// stuff for Win32 builds...
typedef int (FAR *GETENTITYAPI) (DLL_FUNCTIONS *, int);
typedef int (FAR *GETNEWDLLFUNCTIONS) (NEW_DLL_FUNCTIONS *, int *);
typedef void (DLLEXPORT *GIVEFNPTRSTODLL) (enginefuncs_t *, globalvars_t *);
typedef int (*SERVER_GETBLENDINGINTERFACE) (int, struct sv_blending_interface_s **, struct engine_studio_api_s *, float (*)[3][4], float (*)[MAXSTUDIOBONES][3][4]);
typedef void (FAR *LINK_ENTITY_FUNC) (entvars_t *);


// global variables wrapper and null vector definition
#ifdef DEFINE_GLOBALS
#define GLOBAL
const Vector g_vecZero = Vector (0, 0, 0);
#else
#define GLOBAL extern
#endif


// handy definition for exporting the MOD entities
#define LINK_ENTITY_TO_FUNC(mapClassName)\
extern "C" EXPORT void mapClassName (entvars_t *pev);\
\
void mapClassName (entvars_t *pev)\
{\
   static LINK_ENTITY_FUNC otherClassName = NULL;\
   static bool missing = FALSE;\
\
   if (missing)\
      return;\
\
   if (otherClassName == NULL)\
      otherClassName = (LINK_ENTITY_FUNC) GetProcAddress (h_Library, #mapClassName);\
\
   if (otherClassName == NULL)\
   {\
      missing = TRUE;\
      return;\
   }\
\
   (*otherClassName) (pev);\
}


#ifndef RACC_H
#define RACC_H

// file-related structure definitions
typedef struct
{                       
   WORD e_magic; // magic number
   WORD e_cblp; // bytes on last page of file
   WORD e_cp; // pages in file
   WORD e_crlc; // relocations
   WORD e_cparhdr; // size of header in paragraphs
   WORD e_minalloc; // minimum extra paragraphs needed
   WORD e_maxalloc; // maximum extra paragraphs needed
   WORD e_ss; // initial (relative) SS value
   WORD e_sp; // initial SP value
   WORD e_csum; // checksum
   WORD e_ip; // initial IP value
   WORD e_cs; // initial (relative) CS value
   WORD e_lfarlc; // file address of relocation table
   WORD e_ovno; // overlay number
   WORD e_res[4]; // reserved words
   WORD e_oemid; // OEM identifier (for e_oeminfo)
   WORD e_oeminfo; // OEM information; e_oemid specific
   WORD e_res2[10]; // reserved words
   LONG e_lfanew; // file address of new exe header
} DOS_HEADER, *P_DOS_HEADER; // DOS .EXE header


typedef struct
{
   WORD Machine;
   WORD NumberOfSections;
   DWORD TimeDateStamp;
   DWORD PointerToSymbolTable;
   DWORD NumberOfSymbols;
   WORD SizeOfOptionalHeader;
   WORD Characteristics;
} PE_HEADER, *P_PE_HEADER;


typedef struct
{
   BYTE Name[8];
   union
   {
      DWORD PhysicalAddress;
      DWORD VirtualSize;
   } Misc;
   DWORD VirtualAddress;
   DWORD SizeOfRawData;
   DWORD PointerToRawData;
   DWORD PointerToRelocations;
   DWORD PointerToLinenumbers;
   WORD NumberOfRelocations;
   WORD NumberOfLinenumbers;
   DWORD Characteristics;
} SECTION_HEADER, *P_SECTION_HEADER;


typedef struct
{
   DWORD VirtualAddress;
   DWORD Size;
} DATA_DIRECTORY, *P_DATA_DIRECTORY;


typedef struct
{
   WORD Magic;
   BYTE MajorLinkerVersion;
   BYTE MinorLinkerVersion;
   DWORD SizeOfCode;
   DWORD SizeOfInitializedData;
   DWORD SizeOfUninitializedData;
   DWORD AddressOfEntryPoint;
   DWORD BaseOfCode;
   DWORD BaseOfData;
   DWORD ImageBase;
   DWORD SectionAlignment;
   DWORD FileAlignment;
   WORD MajorOperatingSystemVersion;
   WORD MinorOperatingSystemVersion;
   WORD MajorImageVersion;
   WORD MinorImageVersion;
   WORD MajorSubsystemVersion;
   WORD MinorSubsystemVersion;
   DWORD Win32VersionValue;
   DWORD SizeOfImage;
   DWORD SizeOfHeaders;
   DWORD CheckSum;
   WORD Subsystem;
   WORD DllCharacteristics;
   DWORD SizeOfStackReserve;
   DWORD SizeOfStackCommit;
   DWORD SizeOfHeapReserve;
   DWORD SizeOfHeapCommit;
   DWORD LoaderFlags;
   DWORD NumberOfRvaAndSizes;
   DATA_DIRECTORY DataDirectory[16];
} OPTIONAL_HEADER, *P_OPTIONAL_HEADER;


typedef struct
{
   DWORD Characteristics;
   DWORD TimeDateStamp;
   WORD MajorVersion;
   WORD MinorVersion;
   DWORD Name;
   DWORD Base;
   DWORD NumberOfFunctions;
   DWORD NumberOfNames;
   DWORD AddressOfFunctions; // RVA from base of image
   DWORD AddressOfNames; // RVA from base of image
   DWORD AddressOfNameOrdinals; // RVA from base of image
} EXPORT_DIRECTORY, *P_EXPORT_DIRECTORY;


// BSP file related stuff
typedef enum
{
   plane_x = 0,
   plane_y,
   plane_z,
   plane_anyx,
   plane_anyy,
   plane_anyz
} planetypes;


typedef struct
{
   int fileofs; // offset of the lump in the BSP file
   int filelen; // length of the lump in the BSP file
} lump_t;


typedef struct
{
   Vector mins; // vector location of the lower corner of the model's bounding box
   Vector maxs; // vector location of the upper corner of the model's bounding box
   Vector origin; // vector origin of the model
   int headnode[MAX_MAP_HULLS]; // WTF is that ???
   int visleafs; // not including the solid leaf 0
   int firstface; // index of the model's first face
   int numfaces; // number of faces the model has
} dmodel_t;


typedef struct
{
   int version; // BSP header version number
   lump_t lumps[HEADER_LUMPS]; // number of lumps the BSP file has
} dheader_t;


typedef struct
{
   Vector point; // vector coordinates of the vertex (point in space delimiting an angle)
} dvertex_t;


typedef struct
{
   Vector normal; // vector coordinates of the plane normal
   float dist; // distance from the center (0, 0, 0) of the map, I assume
   planetypes type; // PLANE_X - PLANE_ANYZ ?remove? trivial to regenerate -- WTF ???
} dplane_t;


typedef struct
{
   unsigned short v[2]; // indices numbers of the vertices bounding this edge, I assume
} dedge_t;


typedef struct
{
   unsigned short planenum; // index number of the face
   short side; // index of the side(s) that are textured (or not) on this face, I assume
   int firstedge; // index number of the first edge of this face
   short numedges; // number of edges this face has
   short texinfo; // texture info for this face, I assume
   unsigned char styles[MAXLIGHTMAPS]; // lightmap styles for this face -- don't ask me more
   int lightofs; // start of [numstyles*surfsize] samples -- light offset ???
} dface_t;


typedef struct
{
   dmodel_t dmodels[MAX_MAP_MODELS]; // map's model data array
   dplane_t dplanes[MAX_MAP_PLANES]; // map's plane data array
   dvertex_t dvertexes[MAX_MAP_VERTS]; // map's vertice data array
   dface_t dfaces[MAX_MAP_FACES]; // map's face data array
   dedge_t dedges[MAX_MAP_EDGES]; // map's edge data array
   int dsurfedges[MAX_MAP_SURFEDGES]; // map's surfedge data array
} bsp_file_t;


// memory-loaded file structure definition
typedef struct
{
   char *data;
   long read_pointer_index;
   long file_size;
   char path[256];
} MFILE;


// debug levels structure definition
typedef struct
{
   FILE *fp; // pointer to the file where the debug messages are logged
   unsigned char eyes; // vision debug level
   unsigned char ears; // hearing debug level
   unsigned char body; // feeling debug level
   unsigned char legs; // movement debug level
   unsigned char hand; // weaponry usage debug level
   unsigned char chat; // chat debug level
   unsigned char navigation; // navigation debug level
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
} debug_level_t;


// server structure definition
typedef struct
{
   bool is_dedicated; // set to TRUE if the server is a dedicated (standalone) server
   char language[32]; // locale to use in game (english, french, german, italian or spanish)
   char mod_name[32]; // name of the game currently running on the server
   char map_name[32]; // name of the map currently playing on the server
   int *max_clients; // maximum number of connected clients this server allows at a time
   bool is_multiplayer; // set to TRUE if the game going on is multiplayer (generally yes, duh)
   bool is_teamplay; // set to TRUE if the game going on is a team play game
   bool does_footsteps; // set to TRUE if the game rules allow footstep sounds
   short developer_level; // the current internal debug level this server is running on
   float *time; // pointer to current time on server
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
} server_t;


// walkable faces structure definition
typedef struct
{
   Vector *v_corners; // pointer to array of face edges vector locations (mallocated)
   Vector *v_delimiters; // pointer to array of face delimiters vector locations (mallocated)
   int corner_count; // number of edges (and thus delimiters) this face has
} walkface_t;


// topologic sector structure definition
typedef struct
{
   walkface_t **faces; // mallocated array of pointers to walkable faces located in this sector
   int faces_count; // number of walkable faces in this array
} sector_t;


// the virtual world itself
typedef struct
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
   char reachability; // type of reachability it is (normal, ladder, edge fall, elevator, etc.)
   Vector v_origin; // vector origin of this link (passage between 2 nodes) in the virtual world
};


// navigation node structure definition
struct navnode_t
{
   walkface_t *walkface; // the walkable face this node concerns
   struct navlink_t links[8]; // array of navigation links to walkfaces reachable from this one
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
typedef struct
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
   bool b_bomb_map; // set to TRUE if the round goal is to plant a bomb
   bool b_bomb_planted; // set to TRUE when the bomb has been planted
   bool b_hostage_map; // set to TRUE if the round goal is to rescue hostages
   int hostage_count; // number of hostages to rescue, when appliable
} round_t;


// flag structure definition (for CTF-style games)
typedef struct
{
   bool mdl_match;
   int team_no;
   edict_t *edict;
} flag_t;


// game weapons structure definition
typedef struct
{
   int use_percent; // times out of 100 to use this rail
   float min_range; // mimimal distance to target required to use this rail safely
   float max_range; // maximum distance to target this rail can reach
   char type_of_ammo; // type of ammo this rail uses (i.e, index to use in the ammo array)
   short min_ammo; // minimum ammout of ammo this rail needs to fire
   short max_ammo; // maximum amount of ammo this rail can hold
   bool can_use_underwater; // set to TRUE if this rail can be used under water
   bool should_hold; // set to TRUE if this rail's trigger needs to be held down to fire
   bool should_charge; // set to TRUE if this rail needs to be charged OR should burst-fire
   float charge_delay; // amount of time needed to charge the rail before firing
   char sound1[64]; // relative path to sound sample for normal mode firing in this rail
   char sound2[64]; // relative path to sound sample for alternate mode firing in this rail
   float min_delay[5]; // array of 5 skill-dependent minimal reaction delay values for the AI
   float max_delay[5]; // array of 5 skill-dependent maximal reaction delay values for the AI
   float power; // relative power out of 100 of this rail (empirical)
   float radius; // maximum radius of the damage effect at the projectile hit point
   bool can_penetrate; // set to TRUE if this rail may be fired through material (doors, walls)
} weapon_rail_t;


typedef struct
{
   char classname[64]; // the weapon entity classname, in all letters
   char model[64]; // relative path to the weapon model ("models/p_" model)
   int id; // weapon ID
   int slot; // HUD slot (0 based)
   int position; // slot position
   int flags; // flags ???
   int price; // price of weapon, if it needs to be bought
   weapon_rail_t primary; // primary weapon rail
   weapon_rail_t secondary; // secondary weapon rail
} weapon_t;


// audible events related structure definitions
typedef struct
{
   char texture_type; // type of texture according to the materials.txt file ('*' catches all)
   char file_path[64]; // relative path to the ricochet sound sample for this type of texture
} ricochetsound_t;


typedef struct
{
   char file_path[64]; // relative path to the sound sample
   float loudness; // violence of the sample (low value for footsteps, high value for gunshot)
   float duration; // amount of time our hearing capabilities will be occupied by this sound
} sound_t;


typedef struct
{
   char *file_path; // pointer to relative path to sound sample heard (in sound_t array)
   char direction; // relative direction (left, right, front, back) of noise around the listener
   float loudness; // violence of the sample (low value for footsteps, high value for gunshot)
   float fade_date; // date at which this sound will have faded out
} noise_t;


// HAL-related structure definitions
typedef struct
{
   unsigned char length; // length of string (for quick access)
   char *word; // the string itself
} HAL_STRING;


typedef struct
{
   unsigned long size; // size of dictionary (for quick access)
   HAL_STRING *entry; // array of string entries in that dictionary
   unsigned short *index; // pointer to index
} HAL_DICTIONARY;


typedef struct
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


typedef struct
{
   unsigned char order; // model order (complexity)
   HAL_TREE *forward; // pointer to the model's forward tree
   HAL_TREE *backward; // pointer to the model's backwards tree
   HAL_TREE **context; // pointer to context tree pointer
   HAL_DICTIONARY *dictionary; // pointer to the model's dictionary
} HAL_MODEL;


// fov line structure definition
typedef struct
{
   Vector scan_angles; // angles describing the traceline direction for this scan element
   float distance; // distance to first obstacle when tracing at scan_angles
   Vector vecEndPos; // location of first obstacle when tracing at scan_angles
   Vector Normal; // normal of first obstacle surface when tracing at scan_angles (to describe the plane)
   edict_t *pHit; // pointer to obstacle entity
} fov_line_t;


// chat string structure definition
typedef struct
{
   bool new_message; // set to TRUE if the chat messages have just scrolled down
   edict_t *pSender; // pointer to the chat line's sender player entity
   char text[128]; // the actual chat text (in uppercase)
} chat_t;


// player HUD structure definition
typedef struct
{
   chat_t chat; // chat message one can see on the screen
   int menu_state; // VGUI menu state on this HUD
   char icons_state[32]; // array of icons states (either off, lit, or blinking)
   bool has_progress_bar; // set to TRUE when a progress bar is displayed on the HUD
} hud_t;


// player-related structure definition
typedef struct
{
   short pelvis; // bone number for the "pelvis" bone of players (bottom of spine)
   short spine; // bone number for the "spine" bone of players (bottom 1/4 of spine)
   short spine1; // bone number for the "spine1" bone of players (half of spine)
   short spine2; // bone number for the "spine2" bone of players (bottom 3/4 of spine)
   short spine3; // bone number for the "spine3" bone of players (top of spine, between clavicles)
   short neck; // bone number for the "neck" bone of players (bottom of neck)
   short head; // bone number for the "head" bone of players (center of neck)
   short left_clavicle; // bone number for the "left clavicle" bone of players (left shoulder)
   short left_upperarm; // bone number for the "left upperarm" bone of players (left elbow)
   short left_forearm; // bone number for the "left forearm" bone of players (left wrist)
   short left_hand; // bone number for the "left hand" bone of players (center of hand)
   short left_finger0; // bone number for the "left_finger0" bone of players (half of thumb)
   short left_finger01; // bone number for the "left_finger01" bone of players (extremity of thumb)
   short left_finger1; // bone number for the "left_finger1" bone of players (half of other fingers)
   short left_finger11; // bone number for the "left_finger11" bone of players (extremity of fingers)
   short left_thigh; // bone number for the "left_thigh" bone of players (what it says)
   short left_calf; // bone number for the "left_calf" bone of players (what it says)
   short left_foot; // bone number for the "left_foot" bone of players (extremity of toes)
   short right_clavicle; // bone number for the "right clavicle" bone of players (right shoulder)
   short right_upperarm; // bone number for the "right upperarm" bone of players (right elbow)
   short right_forearm; // bone number for the "right forearm" bone of players (right wrist)
   short right_hand; // bone number for the "right hand" bone of players (center of hand)
   short right_finger0; // bone number for the "right_finger0" bone of players (half of thumb)
   short right_finger01; // bone number for the "right_finger01" bone of players (extremity of thumb)
   short right_finger1; // bone number for the "right_finger1" bone of players (half of other fingers)
   short right_finger11; // bone number for the "right_finger11" bone of players (extremity of fingers)
   short right_thigh; // bone number for the "right_thigh" bone of players (what it says)
   short right_calf; // bone number for the "right_calf" bone of players (what it says)
   short right_foot; // bone number for the "right_foot" bone of players (extremity of toes)
} playerbones_t;


typedef struct
{
   bool is_alive; // handy shortcut that tells us each frame if the player is alive or not
   edict_t *pEntity; // this player's entity the engine knows
   float welcome_time; // date at which this player will be sent a welcome message
   walkface_t *pFaceAtFeet; // pointer to the last face this player was walking on
   int face_reachability; // type of reachability from this player's last walkface to the new one
   float step_sound_time; // date under which this player will NOT make a footstep sound
   float proximityweapon_swing_time; // date under which this player will NOT make a swing sound
   TraceResult tr; // results of last traceline issued by this player (how handy :))
   entvars_t prev_v; // this player's entity variables state last frame (past pEntity->v)
} player_t;


// bot-related structure definitions
typedef struct
{
   char name[32]; // the bot's name
   char skin[32]; // the bot's preferred skin
   char logo[32]; // the bot's preferred logo
   char nationality; // the bot's nationality
   int skill; // the bot's skill
   int team; // the bot's team stored as an integer for team-based MODs
   int subclass; // the bot's class (i.e. skin) for class-based MODs
} profile_t;


typedef struct
{
   float worth; // given situation, percentage describing suitability of weapon compared to others
   weapon_t *hardware; // pointer to the weapon data structure corresponding to this weapon
   short clip_ammo; // amount of ammo currently in the clip
   short *primary_ammo; // pointer to amount of ammo currently in reserve for rail 1
   short *secondary_ammo; // pointer to amount of ammo currently in reserve for rail 2
   float primary_charging_time; // date at which the bot will stop charging the primary rail
   float secondary_charging_time; // date at which the bot will stop charging the secondary rail
} bot_weapon_t;


typedef struct
{
   float worth; // given situation, percentage describing the danger this enemy causes to bot
   edict_t *pEdict; // pointer to this enemy's entity
   Vector v_targetpoint; // location where the bot should aim for this enemy
   float appearance_time; // date at which the bot first saw this enemy
   float disappearance_time; // date at which the bot lost this enemy
   bool is_hiding; // set to TRUE if the bot had the impression that this enemy went hiding
} bot_enemy_t;


// sensitive part of the AI
typedef struct
{
   float sample_time; // date at which the bot will next sample his field of view
   float blinded_time; // date under which the bot is blinded by too much light (flashbang...)
   fov_line_t BotFOV[100]; // bot's field of view is 52 elements width
   edict_t *pEntitiesInSight[BOT_EYE_SENSITIVITY]; // entities the bot distinguishes at a time
   hud_t BotHUD; // the bot's Head Up Display (where icons and chat messages are)
} bot_eyes_t;


typedef struct
{
   float average_noise; // average loudness the bot hears (average of noises.loudness)
   noise_t noises[BOT_EAR_SENSITIVITY]; // sounds the bot distinguishes at a time
   bool new_sound; // set to TRUE when a new sound has just arrived in the bot's ear
   char new_sound_index; // slot index of the new sound that has just arrived in the bot's ear
   int bot_order; // integer representing the current order from a teammate bot is following
   float f_order_time; // date at which the bot received its last order
   edict_t *pAskingEntity; // pointer to the bot's current giving order entity
} bot_ears_t;


typedef struct
{
   short hit_state; // bit map description of the terrain in the immediate surroundings of the bot
   float fall_time; // date at which the bot last felt fast enough to get some damage
   Vector v_fall_plane_normal; // vector describing the normal of a fall edge in front of bot
} bot_body_t;


// motile part of the AI
typedef struct
{
   float f_forward_time;
   float f_backwards_time;
   float f_jump_time;
   float f_duck_time;
   float f_strafeleft_time;
   float f_straferight_time;
   float f_walk_time;
   bool b_emergency_walkback;
   float f_max_speed;
   float f_move_speed;
   float f_strafe_speed;
} bot_move_t;


typedef struct
{
   Vector v_ideal_angles; // ENGINEBUGBUGBUG -- AVOID TOUCHING IDEAL ANGLES DIRECTLY !!!
   Vector v_turn_speed;
   Vector v_eyeposition;
   Vector v_forward;
   Vector v_right;
   Vector v_up;
} bot_aim_t;


typedef struct
{
   short bot_saytext;
   float f_saytext_time;
   short bot_sayaudio;
   float f_sayaudio_time;
   char saytext_message[128];
   char sayaudio_message[128];
} bot_chat_t;


typedef struct
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
   float likelevel_ladder; // reachability #1: this bot's like level of ladders
   float likelevel_falledge; // reachability #2: this bot's like level of falls
   float likelevel_elevator; // reachability #3: this bot's like level about elevators
   float likelevel_platform; // reachability #4: this bot's like level of bobbing platforms
   float likelevel_conveyor; // reachability #5: this bot's like level of conveyors (belts and travolators)
   float likelevel_train; // reachability #6: this bot's like level of trains
   float likelevel_longjump; // reachability #7: this bot's like level of long jump modules
} bot_brain_t;


// the bot itself
typedef struct
{
   bool is_active; // set to TRUE if this slot in the bots array belongs to a bot currently playing
   edict_t *pEdict; // the bot entity itself from the engine
   profile_t *pProfile; // pointer to this bot's profile in the profiles array
   edict_t *pIllumination; // dummy entity for getting the correct bot illumination (engine bug)
   bool is_controlled; // set to TRUE if this bot is currently controlled by a player (debug feature)
   bool b_not_started;
   float time_to_live;
   float quit_game_time;

   int bot_money; // amount of money the bot owns for buy action based MODs
   char buy_state; // state machine state for the bot's buy procedures
   float f_buy_time; // date at which the bot will issue the next buy command (TODO: true reaction time)

   bot_eyes_t BotEyes; // structure describing what the bot sees
   bot_ears_t BotEars; // structure describing what the bot hears
   bot_body_t BotBody; // structure describing what the bot touches (or is about to)

   bot_move_t BotMove; // structure handling the directions in which the bot will move/jump/duck
   bot_aim_t BotAim; // structure handling the weapon the bot is holding in its hands
   bot_chat_t BotChat; // structure handling what the bot has to say and when it will say it

   bot_brain_t BotBrain;

   short bot_task;
   float f_find_item_time;
   bool b_is_picking_item;

   edict_t *pBotLadder;
   char ladder_direction;
   float f_start_use_ladder_time;
   float f_end_use_ladder_time;

   bool b_is_fearful;
   Vector v_prev_position; // location at which the bot was standing during the last stuck check
   bool b_is_stuck; // set to TRUE if the bot is not moving as fast as it would like to
   float f_check_stuck_time; // date under which the bot won't check for being stuck at all

   bot_enemy_t BotEnemy; // structure describing the bot's current enemy in sight
   bot_enemy_t LastSeenEnemy; // structure describing the state of the last enemy in sight
   edict_t *pVictimEntity; // pointer to the bot's last victim entity
   edict_t *pKillerEntity; // pointer to the bot's last murderer entity

   edict_t *pBotUser;
   Vector v_lastseenuser_position;
   float f_bot_use_time;
   float f_randomturn_time;
   Vector v_place_to_keep; // location the bot intends to stay at
   float f_place_time; // date at which the bot last saw its last location to stay at

   float f_bot_alone_timer;
   bool b_help_asked;

   float f_rush_time;
   float f_pause_time;
   float f_sound_update_time;

   float f_fallcheck_time; // date under which the bot won't mind about falls in its neighbourhood
   float f_turncorner_time; // date under which the bot won't mind about corners on its sides
   float f_avoid_time;
   bool b_is_walking_straight;
   float f_find_goal_time;
   Vector v_goal;
   bool b_has_valuable;
   float f_camp_time; // date under which the bot will be camping (staying ambushed)
   float f_getreachpoint_time; // date at which the bot will determine a new reach point
   Vector v_reach_point; // location the bot is immediately heading to
   float f_reach_time; // date under which the bot should not worry about heading to the above

   bool b_interact;
   float f_interact_time;
   Vector v_interactive_entity;
   float f_use_station_time;
   Vector v_station_entity;

   bool b_lift_moving;
   float f_spraying_logo_time;
   bool b_logo_sprayed;
   bool b_can_plant;
   bool has_defuse_kit;

   bot_weapon_t bot_weapons[MAX_WEAPONS]; // array of weapons the bot has
   bot_weapon_t *current_weapon; // pointer to the slot corresponding to the bot's current weapon
   short bot_ammos[MAX_WEAPONS]; // array of ammunition types the bot has
   float f_shoot_time; // date under which the bot should not fire its weapon
   float f_reload_time; // date at which the bot will press the RELOAD key to reload its weapon
   int bot_grenades_1; // first grenades amount
   int bot_grenades_2; // second grenades amount
   float f_throwgrenade_time;
} bot_t;


// dll.cpp function prototypes
//
// interface :      [META]MOD DLL <===1===> racc.dll (BOT DLL) <===2===> hw.dll (ENGINE DLL)
//
// This file contains the functions that interface the MOD DLL with the BOT DLL (interface 1)
//
void DLLEXPORT GiveFnptrsToDll (enginefuncs_t* pengfuncsFromEngine, globalvars_t *pGlobals);
void GameDLLInit (void);
int Spawn (edict_t *pent);
void Think (edict_t *pent);
void Use (edict_t *pentUsed, edict_t *pentOther);
void Touch (edict_t *pentTouched, edict_t *pentOther);
void Blocked (edict_t *pentBlocked, edict_t *pentOther);
void KeyValue (edict_t *pentKeyvalue, KeyValueData *pkvd);
void Save (edict_t *pent, SAVERESTOREDATA *pSaveData);
int Restore (edict_t *pent, SAVERESTOREDATA *pSaveData, int globalEntity);
void SetAbsBox (edict_t *pent);
void SaveWriteFields (SAVERESTOREDATA *pSaveData, const char *pname, void *pBaseData, TYPEDESCRIPTION *pFields, int fieldCount);
void SaveReadFields (SAVERESTOREDATA *pSaveData, const char *pname, void *pBaseData, TYPEDESCRIPTION *pFields, int fieldCount);
void SaveGlobalState (SAVERESTOREDATA *pSaveData);
void RestoreGlobalState (SAVERESTOREDATA *pSaveData);
void ResetGlobalState (void);
int ClientConnect (edict_t *pEntity, const char *pszName, const char *pszAddress, char szRejectReason[ 128 ] );
void ClientDisconnect (edict_t *pEntity);
void ClientKill (edict_t *pEntity);
void ClientPutInServer (edict_t *pEntity);
void ClientCommand (edict_t *pEntity);
void ClientUserInfoChanged (edict_t *pEntity, char *infobuffer);
void ServerActivate (edict_t *pEdictList, int edictCount, int clientMax);
void ServerDeactivate (void);
void PlayerPreThink (edict_t *pPlayer);
void PlayerPostThink (edict_t *pPlayer);
void StartFrame (void);
void ParmsNewLevel (void);
void ParmsChangeLevel (void);
const char *GetGameDescription (void);
void PlayerCustomization (edict_t *pEntity, customization_t *pCust);
void SpectatorConnect (edict_t *pEntity);
void SpectatorDisconnect (edict_t *pEntity);
void SpectatorThink (edict_t *pEntity);
void Sys_Error (const char *error_string);
void PM_Move (struct playermove_s *ppmove, int server);
void PM_Init (struct playermove_s *ppmove);
char PM_FindTextureType (char *name);
void SetupVisibility (edict_t *pViewEntity, edict_t *pClient, unsigned char **pvs, unsigned char **pas);
void UpdateClientData (const struct edict_s *ent, int sendweapons, struct clientdata_s *cd);
int AddToFullPack (struct entity_state_s *state, int e, edict_t *ent, edict_t *host, int hostflags, int player, unsigned char *pSet);
void CreateBaseline (int player, int eindex, struct entity_state_s *baseline, struct edict_s *entity, int playermodelindex, vec3_t player_mins, vec3_t player_maxs);
void RegisterEncoders (void);
int GetWeaponData (struct edict_s *player, struct weapon_data_s *info);
void CmdStart (const edict_t *player, const struct usercmd_s *cmd, unsigned int random_seed);
void CmdEnd (const edict_t *player);
int ConnectionlessPacket (const struct netadr_s *net_from, const char *args, char *response_buffer, int *response_buffer_size);
int GetHullBounds (int hullnumber, float *mins, float *maxs);
void CreateInstancedBaselines (void);
int InconsistentFile (const edict_t *player, const char *filename, char *disconnect_message);
int AllowLagCompensation (void);
int GetTeam (edict_t *pEntity);

// engine.cpp function prototypes...
//
// interface :      [META]MOD DLL <===1===> racc.dll (BOT DLL) <===2===> hw.dll (ENGINE DLL)
//
// This file contains the functions that interface the BOT DLL with the ENGINE DLL (interface 2)
//
int pfnPrecacheModel (char *s);
int pfnPrecacheSound (char *s);
void pfnSetModel (edict_t *e, const char *m);
int pfnModelIndex (const char *m);
int pfnModelFrames (int modelIndex);
void pfnSetSize (edict_t *e, const float *rgflMin, const float *rgflMax);
void pfnChangeLevel (char *s1, char *s2);
void pfnGetSpawnParms (edict_t *ent);
void pfnSaveSpawnParms (edict_t *ent);
float pfnVecToYaw (const float *rgflVector);
void pfnVecToAngles (const float *rgflVectorIn, float *rgflVectorOut);
void pfnMoveToOrigin (edict_t *ent, const float *pflGoal, float dist, int iMoveType);
void pfnChangeYaw (edict_t *ent);
void pfnChangePitch (edict_t *ent);
edict_t *pfnFindEntityByString (edict_t *pEdictStartSearchAfter, const char *pszField, const char *pszValue);
int pfnGetEntityIllum (edict_t *pEnt);
edict_t *pfnFindEntityInSphere (edict_t *pEdictStartSearchAfter, const float *org, float rad);
edict_t *pfnFindClientInPVS (edict_t *pEdict);
edict_t *pfnEntitiesInPVS (edict_t *pplayer);
void pfnMakeVectors (const float *rgflVector);
void pfnAngleVectors (const float *rgflVector, float *forward, float *right, float *up);
edict_t *pfnCreateEntity (void);
void pfnRemoveEntity (edict_t *e);
edict_t *pfnCreateNamedEntity (int className);
void pfnMakeStatic (edict_t *ent);
int pfnEntIsOnFloor (edict_t *e);
int pfnDropToFloor (edict_t *e);
int pfnWalkMove (edict_t *ent, float yaw, float dist, int iMode);
void pfnSetOrigin (edict_t *e, const float *rgflOrigin);
void pfnEmitSound (edict_t *entity, int channel, const char *sample, float volume, float attenuation, int fFlags, int pitch);
void pfnEmitAmbientSound (edict_t *entity, float *pos, const char *samp, float vol, float attenuation, int fFlags, int pitch);
void pfnTraceLine (const float *v1, const float *v2, int fNoMonsters, edict_t *pentToSkip, TraceResult *ptr);
void pfnTraceToss (edict_t *pent, edict_t *pentToIgnore, TraceResult *ptr);
int pfnTraceMonsterHull (edict_t *pEdict, const float *v1, const float *v2, int fNoMonsters, edict_t *pentToSkip, TraceResult *ptr);
void pfnTraceHull (const float *v1, const float *v2, int fNoMonsters, int hullNumber, edict_t *pentToSkip, TraceResult *ptr);
void pfnTraceModel (const float *v1, const float *v2, int hullNumber, edict_t *pent, TraceResult *ptr);
const char *pfnTraceTexture (edict_t *pTextureEntity, const float *v1, const float *v2);
void pfnTraceSphere (const float *v1, const float *v2, int fNoMonsters, float radius, edict_t *pentToSkip, TraceResult *ptr);
void pfnGetAimVector (edict_t *ent, float speed, float *rgflReturn);
void pfnServerCommand (char *str);
void pfnServerExecute (void);
void pfnClientCommand (edict_t *pEdict, char *szFmt, ...);
void pfnParticleEffect (const float *org, const float *dir, float color, float count);
void pfnLightStyle (int style, char *val);
int pfnDecalIndex (const char *name);
int pfnPointContents (const float *rgflVector);
void pfnMessageBegin (int msg_dest, int msg_type, const float *pOrigin, edict_t *ed);
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
const char *pfnCVarGetString (const char *szVarName);
void pfnCVarSetFloat (const char *szVarName, float flValue);
void pfnCVarSetString (const char *szVarName, const char *szValue);
void pfnAlertMessage (ALERT_TYPE atype, char *szFmt, ...);
void pfnEngineFprintf (FILE *pfile, char *szFmt, ...);
void *pfnPvAllocEntPrivateData (edict_t *pEdict, long cb);
void *pfnPvEntPrivateData (edict_t *pEdict);
void pfnFreeEntPrivateData (edict_t *pEdict);
const char *pfnSzFromIndex (int iString);
int pfnAllocString (const char *szValue);
struct entvars_s *pfnGetVarsOfEnt (edict_t *pEdict);
edict_t *pfnPEntityOfEntOffset (int iEntOffset);
int pfnEntOffsetOfPEntity (const edict_t *pEdict);
int pfnIndexOfEdict (const edict_t *pEdict);
edict_t *pfnPEntityOfEntIndex (int iEntIndex);
edict_t *pfnFindEntityByVars (struct entvars_s *pvars);
void *pfnGetModelPtr (edict_t *pEdict);
int pfnRegUserMsg (const char *pszName, int iSize);
void pfnAnimationAutomove (const edict_t *pEdict, float flTime);
void pfnGetBonePosition (const edict_t *pEdict, int iBone, float *rgflOrigin, float *rgflAngles);
unsigned long pfnFunctionFromName (const char *pName);
const char *pfnNameForFunction (unsigned long function);
void pfnClientPrintf (edict_t *pEdict, PRINT_TYPE ptype, const char *szMsg);
void pfnServerPrint (const char *szMsg);
const char *pfnCmd_Args (void);
const char *pfnCmd_Argv (int argc);
int pfnCmd_Argc (void);
void pfnGetAttachment (const edict_t *pEdict, int iAttachment, float *rgflOrigin, float *rgflAngles);
void pfnCRC32_Init (CRC32_t *pulCRC);
void pfnCRC32_ProcessBuffer (CRC32_t *pulCRC, void *p, int len);
void pfnCRC32_ProcessByte (CRC32_t *pulCRC, unsigned char ch);
CRC32_t pfnCRC32_Final (CRC32_t pulCRC);
long pfnRandomLong (long  lLow,  long  lHigh);
float pfnRandomFloat (float flLow, float flHigh);
void pfnSetView (const edict_t *pClient, const edict_t *pViewent);
float pfnTime (void);
void pfnCrosshairAngle (const edict_t *pClient, float pitch, float yaw);
unsigned char *pfnLoadFileForMe (char *filename, int *pLength);
void pfnFreeFile (void *buffer);
void pfnEndSection (const char *pszSectionName);
int pfnCompareFileTime (char *filename1, char *filename2, int *iCompare);
void pfnGetGameDir (char *szGetGameDir);
void pfnCvar_RegisterVariable (cvar_t *variable);
void pfnFadeClientVolume (const edict_t *pEdict, int fadePercent, int fadeOutSeconds, int holdTime, int fadeInSeconds);
void pfnSetClientMaxspeed (const edict_t *pEdict, float fNewMaxspeed);
edict_t *pfnCreateFakeClient (const char *netname);
void pfnRunPlayerMove (edict_t *fakeclient, const float *viewangles, float forwardmove, float sidemove, float upmove, unsigned short buttons, unsigned char impulse, unsigned char msec);
int pfnNumberOfEntities (void);
char *pfnGetInfoKeyBuffer (edict_t *e);
char *pfnInfoKeyValue (char *infobuffer, char *key);
void pfnSetKeyValue (char *infobuffer, char *key, char *value);
void pfnSetClientKeyValue (int clientIndex, char *infobuffer, char *key, char *value);
int pfnIsMapValid (char *filename);
void pfnStaticDecal (const float *origin, int decalIndex, int entityIndex, int modelIndex);
int pfnPrecacheGeneric (char *s);
int pfnGetPlayerUserId (edict_t *e);
void pfnBuildSoundMsg (edict_t *entity, int channel, const char *sample, /*int*/float volume, float attenuation, int fFlags, int pitch, int msg_dest, int msg_type, const float *pOrigin, edict_t *ed);
int pfnIsDedicatedServer (void);
cvar_t *pfnCVarGetPointer (const char *szVarName);
unsigned int pfnGetPlayerWONId (edict_t *e);
void pfnInfo_RemoveKey (char *s, const char *key);
const char *pfnGetPhysicsKeyValue (const edict_t *pClient, const char *key);
void pfnSetPhysicsKeyValue (const edict_t *pClient, const char *key, const char *value);
const char *pfnGetPhysicsInfoString (const edict_t *pClient);
unsigned short pfnPrecacheEvent (int type, const char *psz);
void pfnPlaybackEvent (int flags, const edict_t *pInvoker, unsigned short eventindex, float delay, float *origin, float *angles, float fparam1, float fparam2, int iparam1, int iparam2, int bparam1, int bparam2);
unsigned char *pfnSetFatPVS (float *org);
unsigned char *pfnSetFatPAS (float *org);
int pfnCheckVisibility (const edict_t *entity, unsigned char *pset);
void pfnDeltaSetField (struct delta_s *pFields, const char *fieldname);
void pfnDeltaUnsetField (struct delta_s *pFields, const char *fieldname);
void pfnDeltaAddEncoder (char *name, void conditionalencode (struct delta_s *pFields, const unsigned char *from, const unsigned char *to));
int pfnGetCurrentPlayer (void);
int pfnCanSkipPlayer (const edict_t *player);
int pfnDeltaFindField (struct delta_s *pFields, const char *fieldname);
void pfnDeltaSetFieldByIndex (struct delta_s *pFields, int fieldNumber);
void pfnDeltaUnsetFieldByIndex (struct delta_s *pFields, int fieldNumber);
void pfnSetGroupMask (int mask, int op);
int pfnCreateInstancedBaseline (int classname, struct entity_state_s *baseline);
void pfnCvar_DirectSet (struct cvar_s *var, char *value);
void pfnForceUnmodified (FORCE_TYPE type, float *mins, float *maxs, const char *filename);
void pfnGetPlayerStats (const edict_t *pClient, int *ping, int *packet_loss);
void pfnAddServerCommand (char *cmd_name, void function (void));
int pfnVoice_GetClientListening (int iReceiver, int iSender);
int pfnVoice_SetClientListening (int iReceiver, int iSender, int bListen);
const char *pfnGetPlayerAuthId (edict_t *e);

// bot_navigation.cpp function prototypes
void BotSetIdealAngles (bot_t *pBot, Vector ideal_angles);
void BotSetIdealPitch (bot_t *pBot, float ideal_pitch);
void BotSetIdealYaw (bot_t *pBot, float ideal_yaw);
void BotAddIdealPitch (bot_t *pBot, float pitch_to_add);
void BotAddIdealYaw (bot_t *pBot, float yaw_to_add);
void BotPointGun (bot_t *pBot);
void BotMove (bot_t *pBot);
void BotOnLadder (bot_t *pBot);
void BotFollowOnLadder (bot_t *pBot);
void BotUnderWater (bot_t *pBot);
void BotUseLift (bot_t *pBot);
bool BotCanUseInteractives (bot_t *pBot);
void BotInteractWithWorld (bot_t *pBot);
void BotTurnAtFall (bot_t *pBot);
bool BotCantSeeForward (bot_t *pBot);
bool BotCanJumpUp (bot_t *pBot);
bool BotCanDuckUnder (bot_t *pBot);
void BotRandomTurn (bot_t *pBot);
void BotFollowUser (bot_t *pBot);
void BotFindLadder (bot_t *pBot);
void BotStayInPosition (bot_t *pBot);
bool BotCheckForWall (bot_t *pBot, Vector v_direction);
void BotCheckForCorners (bot_t *pBot);
void BotWander (bot_t *pBot);
char BotEstimateDirection (bot_t *pBot, Vector v_location);
void BotMoveTowardsPosition (bot_t *pBot, Vector v_position);
bool BotReachPosition (bot_t *pBot, Vector v_position);
void BotGetReachPoint (bot_t *pBot);
void BotUnstuck (bot_t *pBot);
void BotAvoidObstacles (bot_t *pBot);
bool BotCanSeeThis (bot_t *pBot, Vector v_destination);
bool BotCanCampNearHere (bot_t *pBot, Vector v_here);
void BotIsBewitched (bot_t *pBot);
void BotNavLoadBrain (bot_t *pBot);
void BotNavSaveBrain (bot_t *pBot);

// bot_eyes.cpp function prototypes
void BotSee (bot_t *pBot);
Vector BotCanSeeOfEntity (bot_t *pBot, edict_t *pEntity);

// bot_ears.cpp function prototypes
void BotHear (bot_t *pBot);
void BotFeedEar (bot_t *pBot, sound_t *sound, Vector v_origin, float volume);
void DispatchSound (const char *sample, Vector v_origin, float volume, float attenuation);
void PlayBulletSoundsForBots (edict_t *pPlayer);
void PrecacheSoundForBots (char *sound_path, int sound_index);
void InitRicochetSounds (void);

// bot_body.cpp function prototypes
void BotTouch (bot_t *pBot);
void BotCheckForObstaclesAtFeet (bot_t *pBot);
void BotCheckIfStuck (bot_t *pBot);

// bot_chat.cpp function prototypes
void BotChat (bot_t *pBot);
void BotSayHAL (bot_t *pBot);
void BotSayText (bot_t *pBot);
void BotSayAudio (bot_t *pBot);
void BotTalk (bot_t *pBot, char *sound_path);
void DisplaySpeakerIcon (bot_t *pBot, edict_t *pViewerClient, int duration);
void DestroySpeakerIcon (bot_t *pBot, edict_t *pViewerClient);
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
void BotHALTrainModel (bot_t *pBot, HAL_MODEL *model);
void HAL_ShowDictionary (HAL_DICTIONARY *dictionary);
void HAL_MakeWords (char *input, HAL_DICTIONARY *words);
bool HAL_BoundaryExists (char *string, int position);
const char *BotHALGenerateReply (bot_t *pBot);
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
void HAL_FreeDictionary (HAL_DICTIONARY *dictionary);
void HAL_EmptyModel (HAL_MODEL *model);
void HAL_FreeModel (HAL_MODEL *model);
void HAL_FreeTree (HAL_TREE *tree);
void HAL_FreeSwap (HAL_SWAP *swap);
void BotHALLoadBrain (bot_t *pBot);
void BotHALSaveBrain (bot_t *pBot);

// pathmachine.cpp function prototypes
void BotInitPathMachine (bot_t *pBot);
void BotRunPathMachine (bot_t *pBot);
void BotShutdownPathMachine (bot_t *pBot);
bool BotFindPathTo (bot_t *pBot, Vector v_goal, bool urgent);
bool BotFindPathFromTo (bot_t *pBot, Vector v_start, Vector v_goal, bool urgent);
float EstimateTravelCostFromTo (navnode_t *node_from, navnode_t *node_to);
float BotEstimateTravelCost (bot_t *pBot, navlink_t *link_from, navlink_t *link_to);
void PushToOpenList (pathmachine_t *pathmachine, navnode_t *queue_element);
navnode_t *PopFromOpenList (pathmachine_t *pathmachine);

// mapdata.cpp function prototypes
void LookDownOnTheWorld (void);
bool WalkfaceBelongsToSector (const walkface_t *pFace, int sector_i, int sector_j);
bool LoadWorldMap (void);
int SaveWorldMap (int bsp_file_size);
void ShowTheWayAroundToBots (edict_t *pPlayer);
sector_t *SectorUnder (edict_t *pEntity);
sector_t *SectorUnder (Vector v_location);
walkface_t *WalkfaceUnder (edict_t *pEntity);
walkface_t *WalkfaceUnder (Vector v_location);
Vector NearestDelimiterOf (edict_t *pEntity);
int WalkfaceIndexOf (walkface_t *walkface);
void FreeMapData (void);

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

// util.cpp function prototypes
Vector UTIL_VecToAngles (const Vector &v_VectorIn);
void UTIL_TraceLine (const Vector &vecStart, const Vector &vecEnd, IGNORE_MONSTERS igmon, IGNORE_GLASS ignoreGlass, edict_t *pentIgnore, TraceResult *ptr);
void UTIL_TraceLine (const Vector &vecStart, const Vector &vecEnd, IGNORE_MONSTERS igmon, edict_t *pentIgnore, TraceResult *ptr);
void UTIL_TraceHull (const Vector &vecStart, const Vector &vecEnd, IGNORE_MONSTERS igmon, int hullNumber, edict_t *pentIgnore, TraceResult *ptr);
edict_t *UTIL_FindEntityInSphere (edict_t *pentStart, const Vector &vecCenter, float flRadius);
edict_t *UTIL_FindEntityByString (edict_t *pentStart, const char *szKeyword, const char *szValue);
bool IsAlive (edict_t *pEdict);
bool IsInPlayerFOV (edict_t *pPlayer, Vector v_location);
bool FVisible (const Vector &vecOrigin, edict_t *pEdict);
bool IsReachable (Vector v_dest, edict_t *pEdict);
bool IsAtHumanHeight (Vector v_location);
Vector DropAtHumanHeight (Vector v_location);
Vector DropToFloor (Vector v_location);
inline Vector GetGunPosition (edict_t *pEdict);
inline Vector VecBModelOrigin (edict_t *pEdict);
void UTIL_DrawDots (edict_t *pClient, Vector start, Vector end);
void UTIL_DrawLine (edict_t *pClient, Vector start, Vector end, int life, unsigned char red, unsigned char green, unsigned char blue);
void UTIL_DrawBox (edict_t *pClient, Vector bbmin, Vector bbmax, int life, unsigned char red, unsigned char green, unsigned char blue);
void UTIL_DrawWalkface (edict_t *pClient, walkface_t *pFace, int life, unsigned char red, unsigned char green, unsigned char blue);
void UTIL_DrawNavlink (edict_t *pClient, navlink_t *pLink, int life);
void UTIL_DrawSector (edict_t *pClient, sector_t *pSector, int life, unsigned char red, unsigned char green, unsigned char blue);
int printf (const char *fmt, ...);
int ServerConsole_printf (const char *fmt, ...);
float WrapAngle (float angle_to_wrap);
float WrapAngle360 (float angle_to_wrap);
Vector WrapAngles (Vector &angles_to_wrap);
Vector WrapAngles360 (Vector &angles_to_wrap);
float AngleOfVectors (Vector v1, Vector v2);
Vector BotAngleToLocation (bot_t *pBot, Vector v_dest);
int UTIL_GetNearestOrderableBotIndex (edict_t *pAskingEntity);
int UTIL_GetNearestUsableBotIndex (edict_t *pAskingEntity);
int UTIL_GetNearestUsedBotIndex (edict_t *pAskingEntity);
void TerminateOnError (const char *fmt, ...);
void MakeTeams (void);
void InitGameLocale (void);
void InitLogFile (void);
void LogToFile (const char *fmt, ...);
void InitPlayerBones (void);
void InitWeapons (void);
void LoadBotProfiles (void);
void PrecacheStuff (void);
void SpawnDoor (edict_t *pDoorEntity);
void LoadSymbols (const char *filename);
void FakeClientCommand (edict_t *pFakeClient, const char *fmt, ...);
const char *GetField (const char *string, int field_number);
const char *GetConfigKey (const char *config_string);
const char *GetConfigValue (const char *config_string);
int GetUserMsgId (const char *msg_name);
const char *GetUserMsgName (int msg_type);
weapon_t *FindWeaponByName (const char *weapon_name);
weapon_t *FindWeaponByModel (const char *weapon_model);
weapon_t *FindWeaponById (const int weapon_id);
int WeaponIndexOf (weapon_t *weapon);
sound_t *FindSoundByFilename (const char *sound_filename);
void EstimateNextFrameDuration (void);
void SendWelcomeMessage (edict_t *pClient);
void MakeVersion (void);
bool IsMyBirthday (void);
const char *GetModName (void);
bool FileExists (char *filename);
bool IsValidPlayer (edict_t *pPlayer);
bool IsOnFloor (edict_t *pEntity);
bool IsFlying (edict_t *pEntity);
void FreeAllTheStuff (void);
void ServerCommand (void);


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
#error Unrecognized MOD (must define a MOD_DLL - cf. bottom of racc.h)
#endif


#endif // RACC_H

