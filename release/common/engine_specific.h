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
// multi_engine_interface.h
//


#include <sys/types.h>
#include <sys/stat.h>


// engine-related include files
#include "extdll.h"
#include "enginecallback.h"
#include "util.h"
#include "cbase.h"
#include "entity_state.h"
#include "pm_materials.h"


// operating system specific macros, functions and typedefs
#ifdef _WIN32
   #define MOD_LIBRARY_NAME MOD_LIBRARY_NAME_WIN32
   #define METAMOD_LIBRARY_NAME METAMOD_LIBRARY_NAME_WIN32
   #define DLL_ENTRYPOINT int WINAPI DllMain (HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
   #define DLL_DETACHING (fdwReason == DLL_PROCESS_DETACH)
   #define RETURN_ENTRYPOINT return (TRUE)
   #define DLL_GIVEFNPTRSTODLL void
   #define DECLARE_CUSTOM_EXPORTS_ARRAY WORD *p_Ordinals; DWORD *p_Functions; DWORD *p_Names; char *p_FunctionNames[1024]; int num_ordinals; unsigned long base_offset;
   typedef HINSTANCE dll_t;
   typedef FARPROC dll_symbol_t;
   typedef int (FAR *GETENTITYAPI) (DLL_FUNCTIONS *, int);
   typedef int (FAR *GETNEWDLLFUNCTIONS) (NEW_DLL_FUNCTIONS *, int *);
   typedef void (DLLEXPORT *GIVEFNPTRSTODLL) (enginefuncs_t *, globalvars_t *);
   typedef void (FAR *LINK_ENTITY_FUNC) (entvars_t *);
   inline dll_t LoadDynamicLibrary (const char *filename) { return (LoadLibrary (filename)); }
   inline dll_symbol_t GetSymbolInDynamicLibrary (dll_t dll_handle, const char *symbol_name) { return (GetProcAddress (dll_handle, symbol_name)); }
   inline int CloseDynamicLibrary (dll_t dll_handle) { return (!FreeLibrary (dll_handle)); }
#else
   #include <dlfcn.h>
   #define MOD_LIBRARY_NAME MOD_LIBRARY_NAME_UNIX
   #define METAMOD_LIBRARY_NAME METAMOD_LIBRARY_NAME_UNIX
   #define DLL_ENTRY_POINT void _fini (void)
   #define DLL_DETACHING (TRUE)
   #define RETURN_ENTRYPOINT return
   #define DLL_GIVEFNPTRSTODLL extern "C"
   #define DECLARE_CUSTOM_EXPORTS_ARRAY
   typedef (void *) dll_t;
   typedef (void *) dll_symbol_t;
   typedef int (*GETENTITYAPI) (DLL_FUNCTIONS *, int);
   typedef int (*GETNEWDLLFUNCTIONS) (NEW_DLL_FUNCTIONS *, int *);
   typedef void (*GIVEFNPTRSTODLL) (enginefuncs_t *, globalvars_t *);
   typedef void (*LINK_ENTITY_FUNC) (entvars_t *);
   inline dll_t LoadDynamicLibrary (const char *filename) { return (dlopen (filename, RTLD_NOW)); }
   inline dll_symbol_t GetSymbolInDynamicLibrary (dll_t dll_handle, const char *symbol_name) { return (dlsym (dll_handle, symbol_name)); }
   inline int CloseDynamicLibrary (dll_t dll_handle) { return (dlclose (dll_handle)); }
#endif // _WIN32


// maximum number of players in the game at the same time the engine can support
#define MAX_CLIENTS_SUPPORTED_BY_ENGINE 32

// maximum number of server variables that can be declared externally (arbitrary)
#define MAX_SERVER_VARIABLES 32

// maximum number of user messages the engine can register (hardcoded in the engine)
#define MAX_USERMSG_TYPES 256

// maximum number of sounds the engine can precache at the same time
#define MAX_SOUNDS 512

// maximum number of textures the engine can precache at the same time
#define MAX_TEXTURES 512

// maximum distance at which a sound can be heard in the virtual world
#define MAX_HEARING_DISTANCE 3250

// maximum number of chat messages that can be seen on the screen at once in Counter-Strike
#define MAX_CHAT_MESSAGES 4


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


// DOS file-related structure definitions
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
   WORD Machine; // machine ID
   WORD NumberOfSections; // number of sections
   DWORD TimeDateStamp; // date and time stamp
   DWORD PointerToSymbolTable; // pointer to symbols table
   DWORD NumberOfSymbols; // number of symbols in table
   WORD SizeOfOptionalHeader; // size of optional header
   WORD Characteristics; // characteristics
} PE_HEADER, *P_PE_HEADER;


typedef struct
{
   BYTE Name[8]; // name
   union
   {
      DWORD PhysicalAddress; // physical address
      DWORD VirtualSize; // virtual size
   } Misc;
   DWORD VirtualAddress; // virtual address
   DWORD SizeOfRawData; // size of raw data
   DWORD PointerToRawData; // pointer to raw data
   DWORD PointerToRelocations; // pointer to relocations
   DWORD PointerToLinenumbers; // pointer to line numbers
   WORD NumberOfRelocations; // number of relocations
   WORD NumberOfLinenumbers; // number of line numbers
   DWORD Characteristics; // characteristics
} SECTION_HEADER, *P_SECTION_HEADER;


typedef struct
{
   DWORD VirtualAddress; // virtual address
   DWORD Size; // size
} DATA_DIRECTORY, *P_DATA_DIRECTORY;


typedef struct
{
   WORD Magic; // magic number
   BYTE MajorLinkerVersion; // major version number of linker
   BYTE MinorLinkerVersion; // minor version number of linker
   DWORD SizeOfCode; // code size
   DWORD SizeOfInitializedData; // initialized data size
   DWORD SizeOfUninitializedData; // uninitialized data size
   DWORD AddressOfEntryPoint; // entry point address
   DWORD BaseOfCode; // code base
   DWORD BaseOfData; // data base
   DWORD ImageBase; // image base
   DWORD SectionAlignment; // section alignment
   DWORD FileAlignment; // file alignment
   WORD MajorOperatingSystemVersion; // major version number of operating system
   WORD MinorOperatingSystemVersion; // minor version number of operating system
   WORD MajorImageVersion; // major version number of image
   WORD MinorImageVersion; // minor version number of image
   WORD MajorSubsystemVersion; // major version number of subsystem
   WORD MinorSubsystemVersion; // minor version number of subsystem
   DWORD Win32VersionValue; // value of Win32 version
   DWORD SizeOfImage; // image size
   DWORD SizeOfHeaders; // size of headers
   DWORD CheckSum; // checksum
   WORD Subsystem; // subsystem ID
   WORD DllCharacteristics; // DLL characteristics
   DWORD SizeOfStackReserve; // ???
   DWORD SizeOfStackCommit; // ???
   DWORD SizeOfHeapReserve; // ???
   DWORD SizeOfHeapCommit; // ???
   DWORD LoaderFlags; // loader flags
   DWORD NumberOfRvaAndSizes; // ???
   DATA_DIRECTORY DataDirectory[16]; // data directory
} OPTIONAL_HEADER, *P_OPTIONAL_HEADER;


typedef struct
{
   DWORD Characteristics; // characteristics
   DWORD TimeDateStamp; // date and time stamp
   WORD MajorVersion; // major version
   WORD MinorVersion; // minor version
   DWORD Name; // name
   DWORD Base; // base
   DWORD NumberOfFunctions; // number of functions
   DWORD NumberOfNames; // number of names
   DWORD AddressOfFunctions; // RVA from base of image
   DWORD AddressOfNames; // RVA from base of image
   DWORD AddressOfNameOrdinals; // RVA from base of image
} EXPORT_DIRECTORY, *P_EXPORT_DIRECTORY;


// null vector definition
#define NULLVEC 0xFFFFFFFFFFFFFFFF


// vector class (local copy kept here for portability reasons)
class vector
{
   public:

      // construction
      inline vector (void)
      {
         ;
      }

      inline vector (float coordinate_x, float coordinate_y, float coordinate_z)
      {
         x = coordinate_x;
         y = coordinate_y;
         z = coordinate_z;
      }

      inline vector (const vector &v_vector)
      {
         x = v_vector.x;
         y = v_vector.y;
         z = v_vector.z;
      }

      inline vector (float coordinates[3])
      {
         x = coordinates[0];
         y = coordinates[1];
         z = coordinates[2];
      }

      inline vector (const unsigned __int64 nullvec)
      {
         x = 0xFFFF;
         y = 0xFFFF;
         z = 0xFFFF;
      }

      // operators
      operator float *()
      {
         return (&x);
      }

      operator const float *() const
      {
         return (&x);
      }

      inline bool operator ! (void) const
      {
         return ((x == 0xFFFF) && (y == 0xFFFF) && (z == 0xFFFF));
      }

      inline int operator != (const unsigned __int64 nullvec) const
      {
         return ((x != 0xFFFF) && (y != 0xFFFF) && (z != 0xFFFF));
      }

      inline int operator == (const unsigned __int64 nullvec) const
      {
         return ((x == 0xFFFF) && (y == 0xFFFF) && (z == 0xFFFF));
      }

      inline vector operator - (void) const
      {
         return (vector (-x, -y, -z));
      }

      inline int operator == (const vector &v_vector) const
      {
         return ((x == v_vector.x) && (y == v_vector.y) && (z == v_vector.z));
      }

      inline int operator != (const vector &v_vector) const
      {
         return ((x != v_vector.x) || (y != v_vector.y) || (z != v_vector.z));
      }

      inline bool operator <= (const vector &v_vector) const
      {
         return ((x <= v_vector.x) && (y <= v_vector.y) && (z <= v_vector.z));
      }

      inline bool operator >= (const vector &v_vector) const
      {
         return ((x >= v_vector.x) && (y >= v_vector.y) && (z >= v_vector.z));
      }

      inline bool operator > (const vector &v_vector) const
      {
         return ((x > v_vector.x) && (y > v_vector.y) && (z > v_vector.z));
      }

      inline bool operator < (const vector &v_vector) const
      {
         return ((x < v_vector.x) && (y < v_vector.y) && (z < v_vector.z));
      }

      inline vector operator + (const vector &v_vector) const
      {
         return (vector (x + v_vector.x, y + v_vector.y, z + v_vector.z));
      }

      inline vector operator - (const vector &v_vector) const
      {
         return (vector (x - v_vector.x, y - v_vector.y, z - v_vector.z));
      }

      inline vector operator * (float f_factor) const
      {
         return (vector (x * f_factor, y * f_factor, z * f_factor));
      }

      inline vector operator / (float f_factor) const
      {
         return (vector (x / f_factor, y / f_factor, z / f_factor));
      }

      // methods
      inline void ToArray (float *f_array) const
      {
         f_array[0] = x;
         f_array[1] = y;
         f_array[2] = z;
      }

      inline float Length (void) const
      {
         return (sqrt (x * x + y * y + z * z));
      }

      inline vector Normalize (void) const
      {
         float length = Length ();

         if (length == 0)
            return (vector (0, 0, 1));

         length = 1 / length;

         return (vector (x * length, y * length, z * length));
      }

      inline vector Make2D (void) const
      {
         return (vector (x, y, 0));
      }

      inline float Length2D (void) const
      {
         return (sqrt (x * x + y * y));
      }

      // members
      float x;
      float y;
      float z;
};


// 3D referential structure definition
typedef struct
{
   vector v_forward; // normalized vector pointing forward, perpendicular both to v_up and v_right
   vector v_up; // normalized vector pointing upwards, perpendicular both to v_forward and v_right
   vector v_right; // normalized vector pointing right, perpendicular both to v_forward and v_up
} referential_t;


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
   vector mins; // vector location of the lower corner of the model's bounding box
   vector maxs; // vector location of the upper corner of the model's bounding box
   vector origin; // vector origin of the model
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
   vector point; // vector coordinates of the vertex (point in space delimiting an angle)
} dvertex_t;


typedef struct
{
   vector normal; // vector coordinates of the plane normal
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


// player inputs structure definition
typedef struct
{
   float f_forward_time; // date under which this player will be moving forward
   float f_backwards_time; // date under which this player will go backwards
   float f_turnleft_time; // date under which this player will be turning left
   float f_turnright_time; // date under which this player will be turning right
   float f_strafeleft_time; // date under which this player will be strafing left
   float f_straferight_time; // date under which this player will be strafing right
   float f_jump_time; // date under which this player will be issuing a jump
   float f_duck_time; // date under which this player will be ducking
   float f_prone_time; // date under which this player will be proning
   float f_walk_time; // date under which this player will be walking instead of running
   float f_use_time; // date under which this player will be using some interactive entity
   float f_fire1_time; // date under which this player will keep pressing FIRE on his weapon
   float f_fire2_time; // date under which this player will keep pressing FIRE2 on his weapon
   float f_reload_time; // date under which this player will be reloading his weapon
   float f_spray_time; // date under which this player will be spraying its logo
   float f_flashlight_time; // date under which this player will be turning his flashlight on/off
   float f_displayscore_time; // date under which this player will be showing the scores grid
} playerbuttons_t;


typedef struct edict_s entity_t;


// multi_engine_interface.cpp function prototypes
// This file contains all the very engine-specific code and each function acts so as a "wrapper"
// interface to the game and engine APIs. Changes primarily take place here when attempting a
// port RACC onto another game/engine pair platform.
bool IsNull (const entity_t *pEntity);
const char *GetModName (void);
const char *GetMapName (void);
bool IsDedicatedServer (void);
bool IsMultiplayerGame (void);
int DeveloperMode (void);
void PrecacheStuff (void);
void TerminateOnError (const char *fmt, ...);
int printf (const char *fmt, ...);
int ServerConsole_printf (const char *fmt, ...);
void InitLogFile (void);
void LogToFile (const char *fmt, ...);
void ServerCommand (const char *fmt, ...);
long RandomInteger (long from, long to);
float RandomFloat (float from, float to);
const char *ClassnameOf (const entity_t *pEntity);
const char *ModelOf (entity_t *pEntity);
const char *WeaponModelOf (const entity_t *pEntity);
const char *NetnameOf (const entity_t *pEntity);
const char *TargetnameOf (const entity_t *pEntity);
bool IsOnFloor (entity_t *pEntity);
bool IsFlying (entity_t *pEntity);
vector AnglesOfVector (vector &v_vector);
referential_t ReferentialOfAngles (vector &v_angles);
bool HasBoundingBox (const entity_t *pEntity);
void GetEntityBoundingBox (entity_t *pEntity, vector &v_min, vector &v_max);
void SetEntityBoundingBox (entity_t *pEntity, vector &v_min, vector &v_max);
void SetEntityOrigin (entity_t *pEntity, vector &v_origin);
void SetEntityModel (entity_t *pEntity, const char *model_name);
void SetEntityClassname (entity_t *pEntity, const char *class_name);
vector TopOriginOf (entity_t *pEntity);
vector OriginOf (entity_t *pEntity);
vector BottomOriginOf (entity_t *pEntity);
vector EyeOriginOf (const entity_t *pEntity);
vector GunOriginOf (const entity_t *pEntity);
float HealthOf (const entity_t *pEntity);
float HealthPercentageOf (const entity_t *pEntity);
float ArmorOf (const entity_t *pEntity);
float ArmorPercentageOf (const entity_t *pEntity);
float CurrentDamageOf (const entity_t *pEntity);
float CurrentDamagePercentageOf (const entity_t *pEntity);
float IlluminationPercentageOf (entity_t *pEntity);
int WeaponsOf (const entity_t *pPlayer);
int WeaponModeOf (const entity_t *pPlayer);
int FragsOf (const entity_t *pPlayer);
void SetFrags (entity_t *pPlayer, int frags);
vector AnglesOf (entity_t *pEntity);
vector ViewAnglesOf (entity_t *pEntity);
vector PunchAnglesOf (entity_t *pEntity);
void SetAngles (entity_t *pEntity, vector &v_angles);
void SetViewAngles (entity_t *pEntity, vector &v_angles);
void ChangePlayerAngles (entity_t *pPlayer, vector &v_angles);
float VelocityOf (const entity_t *pEntity);
float HorizontalVelocityOf (const entity_t *pEntity);
float VerticalVelocityOf (const entity_t *pEntity);
entity_t *OwnerOf (const entity_t *pEntity);
entity_t *DamageInflictorOf (const entity_t *pEntity);
entity_t *EntityUnder (const entity_t *pEntity);
int PlayerIndexOf (const entity_t *pPlayer);
entity_t *PlayerAtIndex (const int player_index);
int EntityIndexOf (const entity_t *pEntity);
entity_t *EntityAtIndex (const int entity_index);
bool IsAPlayer (const entity_t *pEntity);
bool IsABot (const entity_t *pPlayer);
void MarkPlayerAsBot (entity_t *pPlayer);
bool IsAlive (entity_t *pEntity);
bool IsInvisible (entity_t *pEntity);
int WaterLevelOf (entity_t *pEntity);
int ContentsOf (const vector &v_origin);
float CurrentTime (void);
float FrameDuration (void);
int MaxClientsOnServer (void);
int MaxEntitiesOnServer (void);
entity_t *ConnectABot (const char *bot_name);
entity_t *CreateBotLightEntity (void);
void DeleteEntity (entity_t *pEntity);
float GetServerVariable (const char *var_name);
void SetServerVariable (const char *var_name, float var_value);
void RegisterServerCommand (char *command_name, void function (void));
void RegisterServerVariable (char *variable_name, char *initial_value); 
void FreeServerVariables (void);
bool VectorIsInPlayerFOV (entity_t *pPlayer, vector v_location);
playerbuttons_t InputButtonsOf (entity_t *pPlayer);
unsigned short TranslateButtons (playerbuttons_t &playerbuttons);
unsigned short TranslateImpulse (playerbuttons_t &playerbuttons);
void PerformPlayerMovement (entity_t *pPlayer, entity_t *pLightEntity, vector movement_speed, playerbuttons_t input_buttons, float duration);
const char *TextureNameOn (entity_t *pTexturedEntity, vector v_viewpoint, vector v_viewangles);
void LookDownOnTheWorld (void);
void SendWelcomeMessageTo (entity_t *pPlayer);
void DisplaySpeakerIcon (entity_t *pPlayer, entity_t *pViewerClient, int duration);
void DestroySpeakerIcon (entity_t *pPlayer, entity_t *pViewerClient);
void DrawLine (entity_t *pClient, vector start, vector end, int life, int red, int green, int blue);
