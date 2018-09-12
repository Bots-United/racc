// RACC - AI development project for first-person shooter games derivated from Valve's Half-Life
// (http://www.racc-ai.com/)
//
// The game to engine interfacing code is based on the work done by Jeffrey 'Botman' Broome
// (http://planethalflife.com/botman/)
//
// This project is partially based on the work done by Eric Bieschke in his BSDbot
// (http://gamershomepage.com/csbot/)
//
// This project is partially based on the work done by Brendan 'Spyro' McCarthy in his ODD Bot
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
// engine_specific.cpp
//

#include "racc.h"


extern bool is_dedicated_server;
extern entity_t *pListenserverEntity;
extern char map_name[256];
extern map_t map;

extern "C" EXPORT void player (entvars_t *pev); // here's a very engine-specific function

bsp_file_t bsp_file; // this one is too big to be put on the stack (thanks botman)
cvar_t server_variables[MAX_SERVER_VARIABLES];
int server_variables_count = 0;
int beam_texture_index;
int speaker_texture_index;



bool IsNull (const entity_t *pEntity)
{
   // this function returns TRUE if the pointer to entity pEntity is void or null, FALSE if it
   // looks correct and actually points to a valid entity. We ask the engine directly for the
   // pointer offset.

   // return TRUE if either the entity pointer is void, or the entity slot is free
   return ((pEntity == NULL) || (pfnEntOffsetOfPEntity (pEntity) == 0) || pEntity->free);
}


const char *GetModName (void)
{
   // this function asks the engine for the MOD directory path, then takes its last element
   // which is the MOD's directory name (i.e. for Counter-Strike, it is "cstrike"), and fills
   // in the mod_name global string variable.

   static char mod_name[256];
   unsigned int i, j;

   pfnGetGameDir (mod_name); // ask the engine for the MOD directory path

   // get to the last directory separator
   for (i = strlen (mod_name) - 1; i >= 0; i--)
      if ((mod_name[i] == '\\') || (mod_name[i] == '/'))
         break; // break when we've found it

   i++; // now get to the next character (which is the first character of the mod directory name)
   j = i; // remember the position of that character

   // shift the string so as to start when the MOD's directory name starts
   for (i; i < strlen (mod_name); i++)
      mod_name[i - j] = mod_name[i];

   mod_name[i - j] = 0; // terminate the string
   return (&mod_name[0]); // and return a pointer to it
}


const char *GetMapName (void)
{
   // this function gets the map name and store it in the map_name global string variable.

   static char map_name[256];

   // get the map name from the globalvars_t global server variables structure
   sprintf (map_name, (const char *) (gpGlobals->pStringBase + (int) (gpGlobals->mapname)));
   return (&map_name[0]); // and return a pointer to it
}


bool IsDedicatedServer (void)
{
   // this function returns whether the server we're running is a dedicated server or not

   return (pfnIsDedicatedServer () > 0); // ask the engine for this
}


bool IsMultiplayerGame (void)
{
   // this function returns whether the server we're running is a multiplayer server or not

   return (GetServerVariable ("deathmatch") > 0); // there's a CVAR for this
}


int DeveloperMode (void)
{
   // this function return the developer debug level the game is currently running under

   static int developer_level = GetServerVariable ("developer"); // read from CVAR

   if (developer_level > 1)
      return (DEVELOPER_VERBOSE); // high developer mode
   else if (developer_level == 1)
      return (DEVELOPER_ON); // developer mode

   return (DEVELOPER_OFF);
}


void PrecacheStuff (void)
{
   // this function precaches stuff we need for displaying temp entities or various reasons

   beam_texture_index = pfnPrecacheModel ("sprites/lgtning.spr"); // used to trace beams
   speaker_texture_index = pfnPrecacheModel ("sprites/voiceicon.spr"); // used to display speaker icon
   pfnPrecacheModel ("models/mechgibs.mdl"); // used to create fake entities
}


void TerminateOnError (const char *fmt, ...)
{
   // this function terminates the game because of an error and prints the message string pointed
   // to by fmt either in a messagebox or on the screen.

   va_list argptr;
   static char string[1024];

   // concatenate all the arguments in one string
   va_start (argptr, fmt);
   vsprintf (string, fmt, argptr);
   va_end (argptr);

   MessageBox (0, string, "RACC - Error", NULL); // print to message box

   // if developer mode is on...
   if (DeveloperMode () != DEVELOPER_OFF)
      LogToFile ("FATAL ERROR: %s", string); // also log this error to the logfile

   // free everything that's freeable
   FreeAllTheStuff ();

   // once everything is freed, just exit
   pfnAlertMessage (at_error, string); // tell the engine to exit with an error code
   exit (1);
}


int printf (const char *fmt, ...)
{
   // this function prints a message on the screen. If we are running a dedicated server, the
   // text will be printed on the server console, else if we are running a listen server, it
   // will be printed in game on the listen server client's screen. Since it's basically a
   // redefinition of the standard C libraries printf() function, it has to be the same type,
   // hence the integer return value.

   va_list argptr;
   static char string[1024];

   // concatenate all the arguments in one string
   va_start (argptr, fmt);
   vsprintf (string, fmt, argptr);
   va_end (argptr);

   // are we running a listen server ?
   if (!is_dedicated_server && (pListenserverEntity != NULL))
   {
      pfnMessageBegin (MSG_ONE, GetUserMsgId ("SayText"), NULL, pListenserverEntity); // then print to HUD
      pfnWriteByte (EntityIndexOf (pListenserverEntity));
      pfnWriteString (string);
      pfnMessageEnd ();
   }
   else
      pfnServerPrint (string); // else print to console

   // if developer mode is on...
   if (DeveloperMode () != DEVELOPER_OFF)
      LogToFile ("(server HUD): %s", string); // also log this message to the logfile

   return (0); // printf() HAS to return a value
}


int ServerConsole_printf (const char *fmt, ...)
{
   // this function asks the engine to print a message on the server console

   va_list argptr;
   static char string[1024];

   // concatenate all the arguments in one string
   va_start (argptr, fmt);
   vsprintf (string, fmt, argptr);
   va_end (argptr);

   pfnServerPrint (string); // print to console

   // if developer mode is on...
   if (DeveloperMode () != DEVELOPER_OFF)
      if (string[0] == '.')
         LogToFile (string); // also log this message to the logfile (not prefixing dots)
      else
         LogToFile ("(server console): %s", string); // also log this message to the logfile

   return (0); // printf() HAS to return a value
}


void InitLogFile (void)
{
   // this function reinitializes the log file, erasing any previous content. It is meant to be
   // called when the server boots up.

   FILE *fp;

   fp = fopen (RACC_LOGFILEPATH, "w"); // open the log file in ASCII write mode, discard content
   fprintf (fp, "RACC log file started\n\n"); // dump an init string
   fclose (fp); // close the file

   return; // and return
}


void LogToFile (const char *fmt, ...)
{
   // this function logs a message to the message log file somewhere in the racc directory

   FILE *fp;
   va_list argptr;
   static char string[1024];

   // concatenate all the arguments in one string
   va_start (argptr, fmt);
   vsprintf (string, fmt, argptr);
   va_end (argptr);

   fp = fopen (RACC_LOGFILEPATH, "a"); // open the log file in ASCII append mode, keep content
   fprintf (fp, string); // dump the string into the file
   fclose (fp); // close back the file

   return; // and return
}


void ServerCommand (const char *fmt, ...)
{
   // this function asks the engine to execute a server command

   va_list argptr;
   static char string[1024];

   // concatenate all the arguments in one string
   va_start (argptr, fmt);
   vsprintf (string, fmt, argptr);
   va_end (argptr);

   pfnServerCommand (string); // print to console
}


long RandomInteger (long from, long to)
{
   // this function returns a random integer number choosen between from and to.

   return (pfnRandomLong (from, to));
}


float RandomFloat (float from, float to)
{
   // this function returns a random floating-point number choosen between from and to.

   return (pfnRandomFloat (from, to));
}


const char *ClassnameOf (const entity_t *pEntity)
{
   // this function returns a pointer to the classname string of an entity (classnames are
   // entity 'family' names, such as player, weapon_mp5, func_door, etc...)

   if (IsNull (pEntity))
      return (""); // reliability check

   // read classname directly from entity's entvars_t variables structure
   return ((const char *) (gpGlobals->pStringBase + (int) (pEntity->v.classname)));
}


const char *ModelOf (entity_t *pEntity)
{
   // this function returns a pointer to the model string of an entity, the 3D model used by
   // the engine for displaying this entity in the game. Player models are not handled the exact
   // same way other entity models are, they are stored in the entity's "infobuffer", and not
   // in the entity's entvars_t variable structure. Hence the check.

   if (IsNull (pEntity))
      return (""); // reliability check

   // is this entity a player ?
   if (IsAPlayer (pEntity))
      return (pfnInfoKeyValue (pfnGetInfoKeyBuffer (pEntity), "model")); // get model from infobuffer
   else
   {
      // else normal entity, read model directly from entity's entvars_t variables structure
      return ((const char *) (gpGlobals->pStringBase + (int) (pEntity->v.model)));
   }
}


const char *WeaponModelOf (const entity_t *pEntity)
{
   // this function returns a pointer to the weapon model string of an entity, the 3D model used
   // by the engine for displaying the weapon this entity is holding in the game. Currently only
   // players and monster entities are known to hold weapons, but who knows...

   if (IsNull (pEntity))
      return (""); // reliability check

   // read weapon model directly from entity's entvars_t variables structure
   return ((const char *) (gpGlobals->pStringBase + (int) (pEntity->v.weaponmodel)));
}


const char *NetnameOf (const entity_t *pEntity)
{
   // this function returns a pointer to the netname string of an entity. Netnames are player
   // names as shown in the game score board. Not only players may have a netname, it has been
   // said that a secret door in Half-Life crossfire is a 'func_door' type of entity which has
   // "secret_door" as netname. Here we only want to retrieve netnames that are available for
   // players to know, in extenso only player names. Hence the player check again.

   if (IsNull (pEntity))
      return (""); // reliability check

   // is this entity a player ?
   if (IsAPlayer (pEntity))
   {
      // read player name directly from entity's entvars_t variables structure
      return ((const char *) (gpGlobals->pStringBase + (int) (pEntity->v.netname)));
   }
   else
      return (""); // only players have names
}


const char *TargetnameOf (const entity_t *pEntity)
{
   // this function returns a pointer to the targetname string of an entity. Target names are
   // used for entities that act upon each other, e.g. for a door entity to know which button
   // opens it, and for the button to know whether the door is locked or not. It is also used
   // for triggering events, such as explosions.

   if (IsNull (pEntity))
      return (""); // reliability check

   // read targetname directly from entity's entvars_t variables structure
   return ((const char *) (gpGlobals->pStringBase + (int) (pEntity->v.targetname)));
}


bool IsOnFloor (entity_t *pEntity)
{
   // this function returns whether pEntity is supported by a floor or not

   if (IsNull (pEntity))
      return (FALSE); // reliability check

   // is this entity a player ?
   if (IsAPlayer (pEntity))
      return ((pEntity->v.flags & (FL_ONGROUND | FL_PARTIALGROUND)) != 0); // read player ground state directly from flags
   else
      return (pfnEntIsOnFloor (pEntity) == TRUE); // ask the engine for normal entities
}


bool IsFlying (entity_t *pEntity)
{
   // this function returns whether pEntity isn't currently affected by gravity (but still
   // collides with the world) or not. Useful for checking for players who are on ladders.

   if (IsNull (pEntity))
      return (FALSE); // reliability check

   return (pEntity->v.movetype == MOVETYPE_FLY); // read the gravity state directly from entvars
}


vector AnglesOfVector (vector &v_vector)
{
   // the purpose of this function is to convert a spatial location determined by vec to
   // absolute angles from the origin of the world.

   float angles[3];

   // avoid null vectors
   if (v_vector != NULLVEC)
      pfnVecToAngles (v_vector, angles); // the engine has a handy function for this

   return (WrapAngles (vector (angles))); // don't forget to wrap the resulting angle...
}


referential_t ReferentialOfAngles (vector &v_angles)
{
   // this function considers a system of angles (most common example, a view angle), and sets
   // a vector referential upon it, in order to have the relative direction of "up", "right" and
   // "forward".

   referential_t referential;

   // the engine has a handy function for this too
   pfnAngleVectors (WrapAngles (v_angles), (float *) &referential.v_forward, (float *) &referential.v_right, (float *) &referential.v_up);

   return (referential); // return the new referential
}


bool HasBoundingBox (const entity_t *pEntity)
{
   // this function returns TRUE if pEntity has a valid bounding box, FALSE otherwise

   return ((pEntity->v.absmin.x != 0) || (pEntity->v.absmin.y != 0) || (pEntity->v.absmin.z != 0)
           || (pEntity->v.absmax.x != 0) || (pEntity->v.absmax.y != 0) || (pEntity->v.absmax.z != 0));
}


void GetEntityBoundingBox (entity_t *pEntity, vector &v_min, vector &v_max)
{
   // this function reads the bounding box limits coordinates of the entity pointed to by pEntity
   // and writes them in the vectors pointed to by the v_min and v_max pointers, which correspond
   // to the lower left and upper right corners of the bounding box. Note that bounding boxes are
   // always parallel to the map axises.

   if (IsNull (pEntity))
      return; // reliability check

   v_min = (vector) pEntity->v.absmin; // read the bounding box mins...
   v_max = (vector) pEntity->v.absmax; // read the bounding box maxs...
   return; // simple, eh ?
}


void SetEntityBoundingBox (entity_t *pEntity, vector &v_min, vector &v_max)
{
   // the purpose of this function is to set the bounding box size of pEntity. The v_min and
   // v_max values correspond to the lower left and upper right corners of the bounding box.
   // Note that bounding boxes are always parallel to the map axises.

   if (IsNull (pEntity))
      return; // reliability check

   pfnSetSize (pEntity, v_min, v_max); // ask the engine for this...
   return; // ... and that's all
}


void SetEntityOrigin (entity_t *pEntity, vector &v_origin)
{
   // this function sets the spatial location (origin) of entity pEntity to v_origin.

   if (IsNull (pEntity))
      return; // reliability check

   pfnSetOrigin (pEntity, v_origin); // just tell the engine...
   return; // ... and that's all
}


void SetEntityModel (entity_t *pEntity, const char *model_name)
{
   // this function sets the model (visual appearance) of entity pEntity to the .mdl file
   // specified by model_name.

   if (IsNull (pEntity))
      return; // reliability check

   pfnSetModel (pEntity, model_name); // just tell the engine...
   return; // ... and that's all
}


void SetEntityClassname (entity_t *pEntity, const char *class_name)
{
   // this function sets the classname of entity pEntity to the string pointed to by class_name.

   if (IsNull (pEntity))
      return; // reliability check

   // set the classname string pointer offset directly in the entvars
   pEntity->v.classname = (int) class_name - (int) gpGlobals->pStringBase;
   return;
}


vector TopOriginOf (entity_t *pEntity)
{
   // this function returns the spatial location of the bottom of pEntity. Local variables are
   // made static for speeding up recursive calls of this function.

   static vector v_top_origin;

   if (IsNull (pEntity))
      return (NULLVEC); // reliability check

   // has this entity got a bounding box ?
   if (HasBoundingBox (pEntity))
   {
      v_top_origin = (vector) (pEntity->v.absmin + pEntity->v.absmax) / 2; // take center of bounding box
      v_top_origin.z = pEntity->v.absmax.z; // raise z coordinate to entity's absmax
   }
   else
      v_top_origin = (vector) pEntity->v.origin; // else take the origin the engine knows

   return (v_top_origin); // return the origin we computed
}


vector OriginOf (entity_t *pEntity)
{
   // this function returns the spatial location (origin) of entity pEntity. If entity has a
   // bounding box, it's the center of the bounding box which is returned. Else it's the normal
   // entity origin, as known by the engine.

   if (IsNull (pEntity))
      return (NULLVEC); // reliability check

   // has this entity got a bounding box ?
   if (HasBoundingBox (pEntity))
      return ((vector) (pEntity->v.absmin + pEntity->v.absmax) / 2); // return center of bounding box
   else
      return ((vector) pEntity->v.origin); // else return the origin the engine knows
}


vector BottomOriginOf (entity_t *pEntity)
{
   // this function returns the spatial location of the bottom of pEntity. Local variables are
   // made static for speeding up recursive calls of this function.

   static vector v_bottom_origin;

   if (IsNull (pEntity))
      return (NULLVEC); // reliability check

   // has this entity got a bounding box ?
   if (HasBoundingBox (pEntity))
   {
      v_bottom_origin = (vector) (pEntity->v.absmin + pEntity->v.absmax) / 2; // take center of bounding box
      v_bottom_origin.z = pEntity->v.absmin.z; // lower z coordinate to entity's absmin
   }
   else
      v_bottom_origin = (vector) pEntity->v.origin; // else take the origin the engine knows

   return (v_bottom_origin); // return the origin we computed
}


vector EyeOriginOf (const entity_t *pEntity)
{
   // this function returns the spatial location of the eyes pEntity is supposed to look from

   if (IsNull (pEntity))
      return (NULLVEC); // reliability check

   return ((vector) (pEntity->v.origin + pEntity->v.view_ofs)); // read eyes position directly from entvars
}


vector GunOriginOf (const entity_t *pEntity)
{
   // this function returns the spatial location of the gun pEntity is supposed to hold

   if (IsNull (pEntity))
      return (NULLVEC); // reliability check

   // FIXME: I don't know how to get the gun position of an entity, so I always assume the gun
   // is at eyes position. It's not true in most of the case, the gun is a little lower.

   return ((vector) (pEntity->v.origin + pEntity->v.view_ofs)); // always assume the gun is at eyes position
}


float HealthOf (const entity_t *pEntity)
{
   // this function returns IN ABSOLUTE the amount of health the entity pointed to by pEntity has

   if (IsNull (pEntity))
      return (0); // reliability check

   // read the entity's health variables directly from the entvars
   return (pEntity->v.health);
}


float HealthPercentageOf (const entity_t *pEntity)
{
   // this function returns the amount of health IN PERCENTAGE the entity pointed to by pEntity has

   if (IsNull (pEntity))
      return (0); // reliability check

   // read the entity's health variables directly from the entvars and compute the percentage
   return (100 * pEntity->v.health / pEntity->v.max_health);
}


float ArmorOf (const entity_t *pEntity)
{
   // this function returns IN ABSOLUTE the amount of armor the entity pointed to by pEntity has

   if (IsNull (pEntity))
      return (0); // reliability check

   return (pEntity->v.armorvalue); // read the entity's armor variables directly from the entvars
}


float ArmorPercentageOf (const entity_t *pEntity)
{
   // this function returns the amount of armor IN PERCENTAGE the entity pointed to by pEntity has

   if (IsNull (pEntity))
      return (0); // reliability check

   // FIXME: I didn't find the equivalence of max_health for the armor in Half-Life entitie's
   // entvars_t structure, so I assume the maximum amount of armor one can have is 100.

   return (pEntity->v.armorvalue); // read the entity's armor variables directly from the entvars
}


float CurrentDamageOf (const entity_t *pEntity)
{
   // this function returns the amount of damage the entity pointed to by pEntity is taking
   // during this very frame (most of the time, this function obviously returns zero).

   if (IsNull (pEntity))
      return (0); // reliability check

   return (pEntity->v.dmg_take); // read the entity's current damage directly from the entvars
}


float CurrentDamagePercentageOf (const entity_t *pEntity)
{
   // this function returns the amount of damage IN PERCENTAGE the entity pointed to by pEntity
   // is taking during this very frame (most of the time, this function obviously returns zero).

   if (IsNull (pEntity))
      return (0); // reliability check

   // read the entity's current damage directly from the entvars and compute the percentage
   return (100 * pEntity->v.dmg_take / pEntity->v.max_health);
}


float IlluminationPercentageOf (entity_t *pEntity)
{
   // this function returns a value between 0 and 100 corresponding to the entity's illumination.
   // Thanks to William van der Sterren for the human-like illumination filter computation. We
   // only consider noticeable the illuminations between 0 and 30 percent of the maximal value,
   // else it's too bright to be taken in account and we return the full illumination. The HL
   // engine allows entities to have illuminations up to 300 (hence the 75 as 30% of 300).

   if (IsNull (pEntity))
      return (0); // reliability check

   // ask the engine for the illumination of pEntity and filter it so as to return a usable value
   return (100 * sqrt (min (75, pfnGetEntityIllum (pEntity)) / 75));
}


int WeaponsOf (const entity_t *pPlayer)
{
   // this functions returns a bitmap of the weapons the player entity pointed to by pPlayer has.

   if (IsNull (pPlayer))
      return (0); // reliability check

   return (pPlayer->v.weapons); // read this player's weapon bitmap directly from his entvars
}


int WeaponModeOf (const entity_t *pPlayer)
{
   // this functions returns the weapon mode the weapon of the player entity pointed to by
   // pPlayer is (for weapons supporting several switchable fire modes, silenced or not, etc).

   if (IsNull (pPlayer))
      return (0); // reliability check

   return (pPlayer->v.weaponanim); // read this player's weapon bitmap directly from his entvars
}


int FragsOf (const entity_t *pPlayer)
{
   // this function returns the amount of frags the player entity pointed to by pPlayer has.

   if (IsNull (pPlayer))
      return (0); // reliability check

   return (pPlayer->v.weapons); // read this player's frags directly from his entvars
}


void SetFrags (entity_t *pPlayer, int frags)
{
   // this function sets the player entity pointed to by pPlayer so as to have the desired
   // amount of frags.

   if (IsNull (pPlayer))
      return; // reliability check

   pPlayer->v.frags = frags; // set this player's frags directly in his entvars
}


vector AnglesOf (entity_t *pEntity)
{
   // this function returns the body angles (axial angles) by which entity pointed to by pEntity
   // is oriented in the virtual world

   if (IsNull (pEntity))
      return (NULLVEC); // reliability check

   // get the entity's model angles directly from its entvars (and wrap it on the fly)
   return (WrapAngles ((vector) pEntity->v.angles));
}


vector ViewAnglesOf (entity_t *pEntity)
{
   // this function returns the VIEW angles (axial angles) by which entity pointed to by pEntity
   // orients his HEAD in the virtual world (independent from body angles). Only players and
   // monsters are supposed to have view angles.

   if (IsNull (pEntity))
      return (NULLVEC); // reliability check

   // get the entity's view angles directly from its entvars (and wrap it on the fly)
   return (WrapAngles ((vector) pEntity->v.v_angle));
}


vector PunchAnglesOf (entity_t *pEntity)
{
   // this function returns the PUNCH angles (axial angles) by which entity pointed to by pEntity
   // is made his head oriented each time a shock happens in the virtual world (independent from
   // body angles). Only players (and possibly monsters) are supposed to have punch angles. The
   // punch angle value simulates an amount of flinch which decreases with time.

   if (IsNull (pEntity))
      return (NULLVEC); // reliability check

   // get the entity's view angles directly from its entvars (and wrap it on the fly)
   return (WrapAngles ((vector) pEntity->v.punchangle));
}


void SetAngles (entity_t *pEntity, vector &v_angles)
{
   // this function sets (and wraps) an entity's model angles to the value of angle v_angles

   pEntity->v.angles = (vec3_t) WrapAngles (v_angles); // wrap wrap wrap all day long
   return;
}


void SetViewAngles (entity_t *pEntity, vector &v_angles)
{
   // this function sets (and wraps) an entity's view angles to the value of angle v_angles

   pEntity->v.v_angle = (vec3_t) WrapAngles (v_angles); // wrap wrap wrap while I sing this song
   return;
}


void ChangePlayerAngles (entity_t *pPlayer, vector &v_angles)
{
   // this function changes the view and body angles of a player entity according to the view
   // angles value v_angles

   if (IsNull (pPlayer))
      return; // reliability check

   // move the aim cursor
   pPlayer->v.v_angle = (vec3_t) WrapAngles (v_angles); // you never wrap angles often enough !

   // set the body angles to point the gun correctly
   pPlayer->v.angles.x = pPlayer->v.v_angle.x / 3;
   pPlayer->v.angles.y = pPlayer->v.v_angle.y;
   pPlayer->v.angles.z = 0;

   return;
}


float VelocityOf (const entity_t *pEntity)
{
   // this function returns the speed in units at which the entity pEntity is moving

   if (IsNull (pEntity))
      return (0); // reliability check

   return (pEntity->v.velocity.Length ()); // compute velocity directly from entity variables
}


float HorizontalVelocityOf (const entity_t *pEntity)
{
   // this function returns the speed in units at which the entity pEntity is moving in 2D top view

   if (IsNull (pEntity))
      return (0); // reliability check

   return (pEntity->v.velocity.Length2D ()); // compute velocity directly from entity variables
}


float VerticalVelocityOf (const entity_t *pEntity)
{
   // this function returns the speed in units at which the entity pEntity is moving vertically

   if (IsNull (pEntity))
      return (0); // reliability check

   return (pEntity->v.velocity.z); // return the vertical component of the entity's velocity
}


entity_t *OwnerOf (const entity_t *pEntity)
{
   // this function returns a pointer to the entity owning the entity pointed to by pEntity, or
   // a NULL pointer if pEntity has no owner

   if (IsNull (pEntity))
      return (NULL); // reliability check

   return (pEntity->v.owner); // get pointer to entity owner directly from entity variables
}


entity_t *DamageInflictorOf (const entity_t *pEntity)
{
   // this function returns a pointer to the entity responsible of the last damage the entity
   // pointed to by pEntity has taken (if pEntity has just spawned and/or hasn't taken any damage
   // yet, the pointer returned is NULL)

   if (IsNull (pEntity))
      return (NULL); // reliability check

   return (pEntity->v.dmg_inflictor); // get pointer to inflictor directly from entity variables
}


entity_t *EntityUnder (const entity_t *pEntity)
{
   // this function returns a pointer to the entity supporting the entity pointed to by pEntity
   // (in other words, the entity under pEntity). If pEntity is in mid-air or is not supported by
   // another one, the pointer returned is NULL)

   if (IsNull (pEntity))
      return (NULL); // reliability check

   return (pEntity->v.groundentity); // get pointer to ground entity directly from entity variables
}


int PlayerIndexOf (const entity_t *pPlayer)
{
   // this function returns the player index from 0 to maxplayers - 1 of player entity pPlayer

   if (IsNull (pPlayer))
      return (0); // reliability check

   return (pfnIndexOfEdict (pPlayer) - 1); // remember that player entity indices start at 1
}


entity_t *PlayerAtIndex (const int player_index)
{
   // this function returns a pointer to the player entity whose player index is player_index

   return (pfnPEntityOfEntIndex (player_index + 1)); // remember player entity indices start at 1
}


int EntityIndexOf (const entity_t *pEntity)
{
   // this function returns the entity index of entity pEntity

   if (IsNull (pEntity))
      return (0); // reliability check

   return (pfnIndexOfEdict (pEntity)); // ask the engine directly
}


entity_t *EntityAtIndex (const int entity_index)
{
   // this function returns a pointer to the entity whose index is entity_index

   return (pfnPEntityOfEntIndex (entity_index)); // ask the engine directly
}


bool IsAPlayer (const entity_t *pEntity)
{
   // this function returns TRUE if the entity pointed to by pEntity is a player, FALSE otherwise

   if (IsNull (pEntity))
      return (FALSE); // reliability check

   // if this entity has both the client flag and its player index is valid, it must be a player
   return ((pEntity->v.flags & FL_CLIENT) && (PlayerIndexOf (pEntity) < gpGlobals->maxClients));
}


bool IsABot (const entity_t *pPlayer)
{
   // this function returns TRUE if the player whose entity is pPlayer is a bot, FALSE otherwise

   if (IsNull (pPlayer))
      return (FALSE); // reliability check

   // if this player has both the fakeclient flag and its WonID is 0, then it must be a bot
   return ((pPlayer->v.flags & FL_FAKECLIENT) && (pfnGetPlayerWONId ((entity_t *) pPlayer) == 0));
}


void MarkPlayerAsBot (entity_t *pPlayer)
{
   // this function sets a flag on pPlayer's player entity that tells this player is a bot

   if (IsNull (pPlayer))
      return; // reliability check

   pPlayer->v.flags |= FL_FAKECLIENT; // set the fake client flag on this player
}


bool IsAlive (entity_t *pEntity)
{
   // this function returns whether the entity pointed to by pEntity is alive or not, considering
   // alive any visible and damageable entity not lying dead still. Entities have a handy flag
   // in their entvars_t structure to tell if they are alive or not, so just use it :)

   if (IsNull (pEntity))
      return (FALSE); // reliability check

   // returns TRUE if this entity is not flagged as dead (consider dying entities as still alive)
   // and, of course, if it is visible in the game for players
   return (((pEntity->v.deadflag == DEAD_NO) || (pEntity->v.deadflag == DEAD_DYING))
           && !(pEntity->v.flags & FL_SPECTATOR) && !IsInvisible (pEntity));
}


bool IsInvisible (entity_t *pEntity)
{
   // this function returns whether the entity pointed to by pEntity is invisible or not,
   // considering that invisible entities are either not drawn, or have no model.

   if (IsNull (pEntity))
      return (FALSE); // reliability check

   // returns TRUE if this entity is not visible (i.e, not drawn or no model set)
   return ((pEntity->v.effects & EF_NODRAW) || (ModelOf (pEntity)[0] == 0));
}


int WaterLevelOf (entity_t *pEntity)
{
   // this function returns whether an entity is in water or not. If it's a player, just read
   // from his entity variables the water level. If it's an entity, go for checking the top and
   // the bottom of the bounding box.

   static bool water_top, water_middle, water_bottom;

   if (IsNull (pEntity))
      return (WATERLEVEL_NOT); // reliability check

   // is this entity a player ?
   if (IsAPlayer (pEntity))
   {
      if (pEntity->v.waterlevel > 2)
         return (WATERLEVEL_COMPLETELY); // water surrounds completely player
      else if (pEntity->v.waterlevel == 2)
         return (WATERLEVEL_PARTLY); // player is partly in water
      else if (pEntity->v.waterlevel == 1)
         return (WATERLEVEL_TOUCH); // there is water at player's feet

      return (WATERLEVEL_NOT); // else assume player is not in water
   }

   // else just go for some content checks
   water_top = (ContentsOf (TopOriginOf (pEntity)) == MATTER_WATER);
   water_middle = (ContentsOf (OriginOf (pEntity) + vector (0, 0, -1)) == MATTER_WATER);
   water_bottom = (ContentsOf (BottomOriginOf (pEntity)) == MATTER_WATER);

   if (water_top && water_bottom)
      return (WATERLEVEL_COMPLETELY); // water surrounds completely pEntity
   else if (water_middle && water_bottom)
      return (WATERLEVEL_PARTLY); // pEntity is partly in water
   else if (water_bottom)
      return (WATERLEVEL_TOUCH); // water only at bottom of pEntity (can't possibly be at top :))

   return (WATERLEVEL_NOT); // no water at bottom of pEntity, consider pEntity is not in water
}


int ContentsOf (const vector &v_origin)
{
   // this function returns whether the spatial location vec inside the virtual world is in
   // open space, in water, in lava or outside the map (CONTENTS_EMPTY, CONTENTS_WATER etc).
   // Local variables are made static for speeding up recursive calls of this function.

   static int matter = pfnPointContents (v_origin); // ask the engine for the matter type

   // given the type of material the engine reports it is, return an abstracted type
   if (matter == CONTENTS_SOLID)
      return (MATTER_SOLID);
   else if (matter == CONTENTS_WATER)
      return (MATTER_WATER);
   else if (matter == CONTENTS_SLIME)
      return (MATTER_SLIME);
   else if (matter == CONTENTS_LAVA)
      return (MATTER_LAVA);
   else if (matter == CONTENTS_SKY)
      return (MATTER_SKY);

   return (MATTER_EMPTY); // default material is "empty"
}


float CurrentTime (void)
{
   // this function returns the time elapsed since the server booted up

   return (gpGlobals->time);
}


float FrameDuration (void)
{
   // this function returns the duration of the last video frame on this server

   return (gpGlobals->frametime);
}


int MaxClientsOnServer (void)
{
   // this function returns the maximum number of clients this server allows

   return (gpGlobals->maxClients);
}


int MaxEntitiesOnServer (void)
{
   // this function returns the maximum number of entities this server can handle

   return (gpGlobals->maxEntities);
}


entity_t *ConnectABot (const char *bot_name)
{
   // this function puts a bot player entity into the server as a player, connected under the
   // name bot_name. In Half-Life, a player's connection is a two-step process. First the player
   // contacts the server and asks it for joining the game ; it's the ClientConnect() process.
   // The server can refuse connection, either because it has already too much players connected,
   // or for some other reason ; in which case, it writes the reason in the szRejectReason string
   // (that's why space for this string has to be reserved first). Then, once the player has
   // successfully downloaded all the resource it needs and its resources are synchronized with
   // those of the other players and the server itself, it asks the server for actually entering
   // the game as a new player and to be listed as such ; it's the ClientPutInServer() step.

   entity_t *pBotEntity;
   char *infobuffer, szRejectReason[128];
   int entity_index;

   if (bot_name == NULL)
      return (NULL); // reliability check

   // create the fake client...
   pBotEntity = pfnCreateFakeClient (bot_name);
   if (pBotEntity == NULL)
      return (NULL); // reliability check

   // we have to free the bot's private date for safety reasons since the engine does not do it
   if (pBotEntity->pvPrivateData != NULL)
      pfnFreeEntPrivateData (pBotEntity); // free our predecessor's private data
   pBotEntity->pvPrivateData = NULL; // fools the private data pointer 

   SetFrags (pBotEntity, 0); // reset his frag count 

   // create the player entity by calling MOD's player() function
   player (&pBotEntity->v);

   entity_index = EntityIndexOf (pBotEntity); // get its client index
   infobuffer = pfnGetInfoKeyBuffer (pBotEntity); // get its info buffer

   // set him some parameters in the infobuffer that are required for playing this game
   pfnSetClientKeyValue (entity_index, infobuffer, "model", "helmet");
   pfnSetClientKeyValue (entity_index, infobuffer, "rate", "3500.000000");
   pfnSetClientKeyValue (entity_index, infobuffer, "cl_updaterate", "20");
   pfnSetClientKeyValue (entity_index, infobuffer, "cl_lw", "1");
   pfnSetClientKeyValue (entity_index, infobuffer, "cl_lc", "1");
   pfnSetClientKeyValue (entity_index, infobuffer, "tracker", "0");
   pfnSetClientKeyValue (entity_index, infobuffer, "cl_dlmax", "128");
   pfnSetClientKeyValue (entity_index, infobuffer, "lefthand", "1");
   pfnSetClientKeyValue (entity_index, infobuffer, "friends", "0");
   pfnSetClientKeyValue (entity_index, infobuffer, "dm", "0");
   pfnSetClientKeyValue (entity_index, infobuffer, "ah", "1");

   // make the bot entity ask for a connection to the server
   szRejectReason[0] = 0; // reset the reject reason template string
   ClientConnect (pBotEntity, bot_name, "127.0.0.1", szRejectReason);
   if (szRejectReason[0] != 0)
   {
      ServerCommand ("kick \"%s\"\n", NetnameOf (pBotEntity)); // kick the bot player if the server refused it
      DeleteEntity (pBotEntity); // and clean things up
      return (NULL); // reliability check, server refused connection
   }

   // print a notification message on the dedicated server console if in developer mode
   if ((is_dedicated_server) && (DeveloperMode () != DEVELOPER_OFF))
   {
      if (DeveloperMode () == DEVELOPER_VERBOSE)
      {
         ServerConsole_printf ("Server requiring authentication\n");
         ServerConsole_printf ("Client %s connected\n", NetnameOf (pBotEntity));
         ServerConsole_printf ("Adr: 127.0.0.1:27005\n");
      }
      ServerConsole_printf ("Verifying and uploading resources...\n");
      ServerConsole_printf ("Custom resources total 0 bytes\n");
      ServerConsole_printf ("  Decals:  0 bytes\n");
      ServerConsole_printf ("----------------------\n");
      ServerConsole_printf ("Resources to request: 0 bytes\n");
   }

   ClientPutInServer (pBotEntity); // let him actually join the game

   MarkPlayerAsBot (pBotEntity); // forcibly set fakeclient flag (engine usually forgets)

   return (pBotEntity); // and return a pointer to this bot's player entity
}


entity_t *CreateBotLightEntity (void)
{
   // this function creates a dummy entity we can use to retrieve fakeclients illuminations.
   // Because of an assumed bug in the HL engine, calling IlluminationPercentageOf() on bots
   // always return zero, possibly because the illumination is only computed on entities that
   // Think(). This issue has not been investigated yet, however Tom Simpson from FoxBot kindly
   // provided this workaround, consisting in attributing each bot an invisible entity they
   // carry around, which one we use for retrieving the bot's illumination.

   // create, spawn and prepare the light entity to be used together with the bot
   entity_t *pLightEntity = pfnCreateNamedEntity ((int) "info_target" - (int) gpGlobals->pStringBase);
   Spawn (pLightEntity); // spawn it
   pLightEntity->v.movetype = MOVETYPE_NOCLIP; // set its movement to no clipping
   pLightEntity->v.nextthink = CurrentTime (); // needed to make it think
   SetEntityClassname (pLightEntity, "entity_botlightvalue"); // sets its name
   SetEntityModel (pLightEntity, "models/mechgibs.mdl"); // sets it a model

   return (pLightEntity); // and return a pointer to it
}


void DeleteEntity (entity_t *pEntity)
{
   // this function notifies the engine that the entity pointed to by pEntity has to be removed

   pEntity->v.flags |= FL_KILLME; // mark this entity for removal
}


float GetServerVariable (const char *var_name)
{
   // this function asks the engine for the value of the CVAR var_name. Note that server CVARs
   // are only string variables. But since we only use these for storing numbers, better format
   // this wrapper function for use with floating-point values.

   if (var_name[0] == 0)
      return (0); // reliability check

   return (pfnCVarGetFloat (var_name)); // ask the engine for this CVAR
}


void SetServerVariable (const char *var_name, float var_value)
{
   // this function asks the engine to set the CVAR var_name to the value var_value. Note that
   // server CVARs are only string variables. But since we only use these for storing numbers,
   // better format this wrapper function for use with floating-point values.

   if (var_name[0] == 0)
      return; // reliability check

   pfnCVarSetFloat (var_name, var_value); // ask the engine to update this CVAR
}


void RegisterServerCommand (char *command_name, void function (void))
{
   // this function tells the engine that a new server command is being declared, in addition
   // to the standard ones, whose name is command_name. The engine is thus supposed to be aware
   // that for every "command_name" server command it receives, it should call the function
   // pointed to by "function" in order to handle it.

   if ((command_name[0] == 0) || (function == NULL))
      return; // reliability check

   pfnAddServerCommand (command_name, function); // ask the engine to register this new command
}


void RegisterServerVariable (char *variable_name, char *initial_value)
{
   // this function tells the engine that a new server variable is being declared, in addition
   // to the standard ones, whose name is variable_name. The engine is thus supposed to be aware
   // that every change commited and every information retrieved from the "variable_name" server
   // variable should be directed to the memory space this variable has been declared from, i.e
   // here in our own DLL, instead of in the engine's own array of variables data.

   if ((variable_name[0] == 0) || (initial_value[0] == 0))
      return; // reliability check

   // if we've reached the maximum number of registerable server variables
   if (server_variables_count == MAX_SERVER_VARIABLES)
      TerminateOnError ("RegisterServerVariable(): Too many registered server variables\n"); // bomb out

   server_variables[server_variables_count].name = (char *) malloc (128); // mallocate 128 chars
   server_variables[server_variables_count].string = (char *) malloc (128); // mallocate 128 chars
   strcpy (server_variables[server_variables_count].name, variable_name); // set the variable name
   strcpy (server_variables[server_variables_count].string, initial_value); // set the variable value
   server_variables[server_variables_count].flags = FCVAR_EXTDLL; // set the "external DLL" flag

   pfnCVarRegister (&server_variables[server_variables_count]); // ask the engine to register this new variable
   server_variables_count++; // we have registered one variable more
}


void FreeServerVariables (void)
{
   // this function frees the memory space allocated for server variables

   int index;

   // for each of all the possible variables...
   for (index = 0; index < MAX_SERVER_VARIABLES; index++)
   {
      if (server_variables[index].name)
         free (server_variables[index].name); // free the server variable name space
      server_variables[index].name = NULL; // fools the pointer

      if (server_variables[index].string)
         free (server_variables[index].string); // free the server variable value space
      server_variables[index].string = NULL; // fools the pointer
   }
}


bool VectorIsInPlayerFOV (entity_t *pPlayer, vector v_location)
{
   // this function returns TRUE if the spatial vector location v_location is located inside
   // the player whose entity is pPlayer's field of view cone, FALSE otherwise.

   static referential_t referential;
   static vector v_deviation;

   if (IsNull (pPlayer))
      return (FALSE); // reliability check

   // compute deviation angles (angles between pPlayer's forward direction and v_location)
   v_deviation = WrapAngles (AnglesOfVector (v_location - EyeOriginOf (pPlayer)) - ViewAnglesOf (pPlayer));

   // is v_location outside pPlayer's FOV width (90 degree) ?
   if (abs (v_deviation.x) > MAX_PLAYER_FOV / 2)
      return (FALSE); // then v_location is not visible

   // is v_location outside pPlayer's FOV height (consider the 4:3 screen ratio) ?
   if (abs (v_deviation.y) > (4 * MAX_PLAYER_FOV) / (3 * 2))
      return (FALSE); // then v_location is not visible

   return (TRUE); // else v_location has to be in pPlayer's field of view cone
}


playerbuttons_t InputButtonsOf (entity_t *pPlayer)
{
   // this function returns a structure describing the current input button states of the player
   // entity pointed to by pPlayer.

   static playerbuttons_t playerbuttons;

   memset (&playerbuttons, 0, sizeof (playerbuttons)); // first reset the player buttons structure

   if (IsNull (pPlayer))
      return (playerbuttons); // reliability check

   // now check successively for each of this player's buttons
   if (pPlayer->v.button & IN_FORWARD)
      playerbuttons.f_forward_time = CurrentTime (); // this player is moving forward
   if (pPlayer->v.button & IN_BACK)
      playerbuttons.f_backwards_time = CurrentTime (); // this player is moving backwards
   if (pPlayer->v.button & IN_LEFT)
      playerbuttons.f_turnleft_time = CurrentTime (); // this player is turning left
   if (pPlayer->v.button & IN_RIGHT)
      playerbuttons.f_turnright_time = CurrentTime (); // this player is turning right
   if (pPlayer->v.button & IN_MOVELEFT)
      playerbuttons.f_strafeleft_time = CurrentTime (); // this player is strafing left
   if (pPlayer->v.button & IN_MOVERIGHT)
      playerbuttons.f_straferight_time = CurrentTime (); // this player is strafing right
   if (pPlayer->v.button & IN_JUMP)
      playerbuttons.f_jump_time = CurrentTime (); // this player is jumping
   if (pPlayer->v.button & IN_DUCK)
      playerbuttons.f_duck_time = CurrentTime (); // this player is ducking
   if (pPlayer->v.button & IN_ALT1)
      playerbuttons.f_prone_time = CurrentTime (); // this player is proning
   if (pPlayer->v.button & IN_RUN)
      playerbuttons.f_walk_time = CurrentTime (); // this player is walking instead of running
   if (pPlayer->v.button & IN_USE)
      playerbuttons.f_use_time = CurrentTime (); // this player is taking an action upon some interactive entity
   if (pPlayer->v.button & IN_ATTACK)
      playerbuttons.f_fire1_time = CurrentTime (); // this player is firing his primary fire
   if (pPlayer->v.button & IN_ATTACK2)
      playerbuttons.f_fire2_time = CurrentTime (); // this player is firing his secondary fire
   if (pPlayer->v.button & IN_RELOAD)
      playerbuttons.f_reload_time = CurrentTime (); // this player is reloading his weapon
   if (pPlayer->v.impulse == 201)
      playerbuttons.f_spray_time = CurrentTime (); // this player is spraying his logo
   if (pPlayer->v.impulse == 100)
      playerbuttons.f_flashlight_time = CurrentTime (); // this player is flashing his light
   if (pPlayer->v.button & IN_SCORE)
      playerbuttons.f_displayscore_time = CurrentTime (); // this player is displaying the scores grid

   return (playerbuttons); // return the filled player button state structure
}


unsigned short TranslateButtons (playerbuttons_t &playerbuttons)
{
   // this function sets the right input buttons bitmap variable in the player entity pointed to
   // by pPlayer according to the player button states structure playerbuttons.

   static unsigned short button;

   button = 0; // first reset the buttons integer bitmap

   // now check successively for each of this player's buttons
   if (playerbuttons.f_forward_time >= CurrentTime ())
      button |= IN_FORWARD; // this player is moving forward
   if (playerbuttons.f_backwards_time >= CurrentTime ())
      button |= IN_BACK; // this player is moving backwards
   if (playerbuttons.f_turnleft_time >= CurrentTime ())
      button |= IN_LEFT; // this player is turning left
   if (playerbuttons.f_turnright_time >= CurrentTime ())
      button |= IN_RIGHT; // this player is turning right
   if (playerbuttons.f_strafeleft_time >= CurrentTime ())
      button |= IN_MOVELEFT; // this player is strafing left
   if (playerbuttons.f_straferight_time >= CurrentTime ())
      button |= IN_MOVERIGHT; // this player is strafing right
   if (playerbuttons.f_jump_time >= CurrentTime ())
      button |= IN_JUMP; // this player is jumping
   if (playerbuttons.f_duck_time >= CurrentTime ())
      button |= IN_DUCK; // this player is ducking
   if (playerbuttons.f_prone_time >= CurrentTime ())
      button |= IN_ALT1; // this player is proning
   if (playerbuttons.f_walk_time >= CurrentTime ())
      button |= IN_RUN; // this player is walking instead of running
   if (playerbuttons.f_use_time >= CurrentTime ())
      button |= IN_USE; // this player is taking an action upon some interactive entity
   if (playerbuttons.f_fire1_time >= CurrentTime ())
      button |= IN_ATTACK; // this player is firing his primary fire
   if (playerbuttons.f_fire2_time >= CurrentTime ())
      button |= IN_ATTACK2; // this player is firing his secondary fire
   if (playerbuttons.f_reload_time >= CurrentTime ())
      button |= IN_RELOAD; // this player is reloading his weapon
   if (playerbuttons.f_displayscore_time >= CurrentTime ())
      button |= IN_SCORE; // this player is displaying the scores grid

   return (button); // return the engine-readable input buttons integer bitmap
}


unsigned short TranslateImpulse (playerbuttons_t &playerbuttons)
{
   // this function sets the right input impulse variable in the player entity pointed to
   // by pPlayer according to the player button states structure playerbuttons.

   static unsigned short impulse;

   impulse = 0; // first reset the impulse integer

   // now check successively for each of this player's buttons
   if (playerbuttons.f_spray_time >= CurrentTime ())
   {
      impulse = 201; // this player is spraying his logo
      playerbuttons.f_spray_time = 0; // reset the field since it's an instant command
   }
   if (playerbuttons.f_flashlight_time >= CurrentTime ())
   {
      impulse = 100; // this player is flashing his light
      playerbuttons.f_flashlight_time = 0; // reset the field since it's an instant command
   }

   return (impulse); // return the engine-readable input buttons integer bitmap
}


void PerformPlayerMovement (entity_t *pPlayer, entity_t *pLightEntity, vector movement_speed, playerbuttons_t input_buttons, float duration)
{
   // this function asks the engine to make the player entity pointed to by pPlayer perform a
   // movement animation (such as walk, run, jump, etc.) according to the specified parameters
   // which are the forward, sidewards and upwards movement speeds, and the input buttons state
   // of this player (what keys this player is currently holding down). Additionally for bots,
   // we pass their light entity so that it gets updated as they move, always because of that
   // engine bug that doesn't allow to get directly a bot's illumination. The last parameter
   // specifies the amount of time in milliseconds that movement should extend (it should be
   // sensibly equal to the frame time since this function is supposed to be called once per
   // frame for each bot). That's anyway a curious (read: bad) implementation of bot movement
   // simulation in this engine, it would have been best for the engine to let the animation
   // happen and last until the next call of this function (which is obviously what is done for
   // players which are updated aperiodically on the engine following the network bandwidth).

   static short button, impulse;

   if (IsNull (pPlayer))
      return; // reliability check

   // translate this player's input buttons states into an engine-readable integer bitmap
   button = TranslateButtons (input_buttons);
   impulse = TranslateImpulse (input_buttons);

   // perform the player movement
   pfnRunPlayerMove (pPlayer, ViewAnglesOf (pPlayer), movement_speed.x, movement_speed.y, movement_speed.z, button, impulse, duration);

   // if a light entity has been specified
   if (!IsNull (pLightEntity))
   {
      SetEntityOrigin (pLightEntity, OriginOf (pPlayer)); // make his light entity follow him
      if (pLightEntity->v.nextthink + 0.1 < CurrentTime ())
         pLightEntity->v.nextthink = CurrentTime () + 0.2; // make it think at 3 Hertz
   }
}


const char *TextureNameOn (entity_t *pTexturedEntity, vector v_viewpoint, vector v_viewangles)
{
   // this functions fire a test line from the vector location specified by v_viewpoint in the
   // direction specified by the v_viewangles angles until it finds a texture that belongs to
   // the entity pointed to by pTexturedEntity, then it returns the name of that texture. If the
   // function couldn't find a texture, a NULL pointer is returned.

   // ask the engine to trace for this texture and return the texture name
   return (pfnTraceTexture (pTexturedEntity, (vec3_t) v_viewpoint, (vec3_t) (v_viewangles + ReferentialOfAngles (v_viewangles).v_forward * 9999)));
}


void LookDownOnTheWorld (void)
{
   // this function loads and interprets the map BSP file at server boot start. It opens the map
   // file named filename, reads its contents, parses through the different BSP lumps and fills
   // the map BSP data structure "map", so that we can access to much more geometric info about
   // the virtual world than the engine would lets us know normally. The BSP loading code comes
   // with heavy simplification and major rewriting from Zoner's Half-Life tools source code.
   // Once this process has been done once, a .map file is created which holds the relevant map
   // data the bots make use of, and this file is loaded instead of redoing the BSP analysis
   // process from the start the next time the server is booted. In extenso, this function
   // sort the BSP data and the walkable faces in a handy hashtable). It checks whether a world
   // map file already exists for the map currently booted, and if so, opens it and use the data
   // therein. Else if it is a new map that has never been used with RACC which is booted for
   // the first time, its purpose is to retrieve, compute and sort various map data used by the
   // bots such as the walkable faces, the location of the face delimiters, and finally a world
   // map is "drawn", separating each walkable face to the parallel and meridian it refers to.
   // Such a world map is very useful for the bots not to have to cycle through all the huge map
   // data when they want to bind just one face or just one face delimiter. It speeds up the
   // search by square the resolution of the world map, ie by parallels * meridians times.

   char bsp_file_path[256], *mfile;
   int bsp_file_size, map_file_size = 0;
   dface_t *face;
   int face_index, edge_index, edge, edge_boundary, walkfaces_index, i, j;
   vector v_bound1, v_bound2, v_middle;

   // if a world map already exists...
   if (LoadWorldMap ())
      return; // return if the loading process completed successfully 

   // map not found, out of date or failed loading, we have to build a new one
   ServerConsole_printf ("RACC: Looking down on the world"); // tell people what we are doing

   // load the bsp file and get its actual size (can't fail to do this, the map already booted)
   sprintf (bsp_file_path, "maps/%s.bsp", map_name); // build BSP file path
   mfile = (char *) pfnLoadFileForMe (bsp_file_path, &bsp_file_size); // load bsp file

   // read the MODELS, VERTEXES, PLANES, FACES, SURFEDGES and EDGES lumps of the BSP file
   memcpy (bsp_file.dmodels, mfile + ((dheader_t *) mfile)->lumps[LUMP_MODELS].fileofs, ((dheader_t *) mfile)->lumps[LUMP_MODELS].filelen);
   memcpy (bsp_file.dvertexes, mfile + ((dheader_t *) mfile)->lumps[LUMP_VERTEXES].fileofs, ((dheader_t *) mfile)->lumps[LUMP_VERTEXES].filelen);
   memcpy (bsp_file.dplanes, mfile + ((dheader_t *) mfile)->lumps[LUMP_PLANES].fileofs, ((dheader_t *) mfile)->lumps[LUMP_PLANES].filelen);
   memcpy (bsp_file.dfaces, mfile + ((dheader_t *) mfile)->lumps[LUMP_FACES].fileofs, ((dheader_t *) mfile)->lumps[LUMP_FACES].filelen);
   memcpy (bsp_file.dsurfedges, mfile + ((dheader_t *) mfile)->lumps[LUMP_SURFEDGES].fileofs, ((dheader_t *) mfile)->lumps[LUMP_SURFEDGES].filelen);
   memcpy (bsp_file.dedges, mfile + ((dheader_t *) mfile)->lumps[LUMP_EDGES].fileofs, ((dheader_t *) mfile)->lumps[LUMP_EDGES].filelen);

   free (mfile); // everything is loaded, free the BSP file

   // get a quick access to the world's bounding box
   map.v_worldmins = bsp_file.dmodels[0].mins;
   map.v_worldmaxs = bsp_file.dmodels[0].maxs;

   // reset the number of walkable faces we know this map has
   map.walkfaces_count = 0;

   // completely reset all the topology array
   memset (&map.topology, 0, sizeof (map.topology));

   // compute the number of sectors this map should have (200 units-sized sectors by default)
   map.parallels_count = (map.v_worldmaxs.x - map.v_worldmins.x) / 200;
   map.meridians_count = (map.v_worldmaxs.y - map.v_worldmins.y) / 200;

   // don't allow the parallels and meridians count to be higher than the maximal value allowed
   if (map.parallels_count > MAX_MAP_PARALLELS)
      map.parallels_count = MAX_MAP_PARALLELS; // bound the number of parallels up to MAX_MAP_PARALLELS
   if (map.meridians_count > MAX_MAP_PARALLELS)
      map.meridians_count = MAX_MAP_MERIDIANS; // bound the number of meridians up to MAX_MAP_MERIDIANS

   // loop through all the faces of the BSP file and count the number of walkable faces...
   for (face_index = 0; face_index < bsp_file.dmodels[0].numfaces; face_index++)
   {
      face = &bsp_file.dfaces[bsp_file.dmodels[0].firstface + face_index]; // quick access to the face

      // if this face is walkable (i.e, plane normal pointing straight up)
      if (bsp_file.dplanes[face->planenum].normal.z < 0.707106)
         map.walkfaces_count++; // we know this map to have a walkable face more
   }

   // now allocate enough memory for the faces array to hold the right number of walkable faces
   map.walkfaces = (walkface_t *) malloc (map.walkfaces_count * sizeof (walkface_t));

   // then, translate each walkable face of the BSP file into an engine-independent structure,
   // describing each face by its corners and its face delimiters (middle of face edges)

   walkfaces_index = 0; // first reset the walkable faces index

   // loop through all the faces of the BSP file...
   for (face_index = 0; face_index < bsp_file.dmodels[0].numfaces; face_index++)
   {
      face = &bsp_file.dfaces[bsp_file.dmodels[0].firstface + face_index]; // quick access to the face

      // discard that plane if it's not walkable (i.e, normal NOT pointing straight up)
      if (bsp_file.dplanes[face->planenum].normal.z < 0.707106)
         continue;

      // allocate enough memory to hold all this face's corner information
      map.walkfaces[walkfaces_index].corner_count = (int) face->numedges; // number of edges
      map.walkfaces[walkfaces_index].v_corners = (vector *) malloc (face->numedges * sizeof (vector));
      map.walkfaces[walkfaces_index].v_delimiters = (vector *) malloc (face->numedges * sizeof (vector));

      // face is walkable, loop though the edges and get the vertexes...
      for (edge_index = 0; edge_index < face->numedges; edge_index++)
      {
         // get the coordinates of the vertex of this edge...
         edge = bsp_file.dsurfedges[face->firstedge + edge_index];

         // if its index in the BSP tree is negative...
         if (edge < 0)
         {
            edge = -edge; // revert it
            edge_boundary = 1; // consider the other side of the segment
         }
         else
            edge_boundary = 0; // else consider the first side of the segment

         // locate the first vertice of this edge
         v_bound1 = bsp_file.dvertexes[bsp_file.dedges[edge].v[edge_boundary]].point;

         // get the coordinates of the vertex of the next edge...
         edge = bsp_file.dsurfedges[face->firstedge + ((edge_index + 1) % face->numedges)];

         // if its index in the BSP tree is negative...
         if (edge < 0)
         {
            edge = -edge; // revert it
            edge_boundary = 1; // consider the other side of the segment
         }
         else
            edge_boundary = 0; // else consider the first side of the segment

         // locate the second vertice of this edge
         v_bound2 = bsp_file.dvertexes[bsp_file.dedges[edge].v[edge_boundary]].point;

         // compute the middle of this edge (i.e, the face delimiter)
         v_middle = (v_bound1 + v_bound2) / 2;

         map.walkfaces[walkfaces_index].v_corners[edge_index] = v_bound1; // store corner
         map.walkfaces[walkfaces_index].v_delimiters[edge_index] = v_middle; // store delimiter

         // for each latitude/longitude sector of the map topology array...
         for (i = 0; i < map.parallels_count; i++)
            for (j = 0; j < map.meridians_count; j++)
            {
               // has some memory space not been allocated yet for this sector ?
               if (map.topology[i][j].faces == NULL)
               {
                  map.topology[i][j].faces = (walkface_t **) malloc (sizeof (walkface_t *));
                  map.topology[i][j].faces[0] = &map.walkfaces[0]; // failsafe pointer
               }

               // does this segment cut one of the topologic sector's boundaries ?
               if (SegmentBelongsToSector (v_bound1, v_bound2, i, j))
               {
                  // reallocate enough space for this zone to hold one walkable face more
                  map.topology[i][j].faces = (walkface_t **) realloc (map.topology[i][j].faces, (map.topology[i][j].faces_count + 1) * sizeof (walkface_t *));

                  // now store a pointer to this walkable face in this topological zone
                  map.topology[i][j].faces[map.topology[i][j].faces_count] = &map.walkfaces[walkfaces_index];
                  map.topology[i][j].faces_count++; // this topological zone holds now one face more
               }
            }

         // if we've computed 256 segments more...
         if (!(face_index & 0xFF))
            ServerConsole_printf ("."); // print a trailing dot as a progress bar
      }

      walkfaces_index++; // we have processed one walkable face more
   }

   // once we've got all the sorting done, it's time to save our worldmap to disk
   map_file_size = SaveWorldMap (bsp_file_size);

   // and terminate the progress bar
   ServerConsole_printf (" done\n   %d parallels, %d meridians, %.2f kilobytes world data\n",
                         map.parallels_count, map.meridians_count, (float) map_file_size / 1024);

   return;
}


void SendWelcomeMessageTo (entity_t *pPlayer)
{
   // the purpose of this (simple) function, is to display the welcome message to newly joined
   // clients. The welcome message is a white text line containing the RACC_WELCOMETEXT, which
   // fades in, stays a few seconds, and fades out on the client's screen. A welcome sound is
   // also played on the client. Since this sound sample has to exist on the client's machine,
   // it is recommended to use a sound sample from the original Half-Life sound pack.

   if (IsNull (pPlayer) || IsABot (pPlayer))
      return; // reliability check, also don't send welcome message to bots

   // send the welcome message to this client
   // do not use the engine macros here (such as MESSAGE_BEGIN, WRITE_BYTE etc.) since we
   // have to hook these messages too as if they were coming from outside the bot DLL...
   pfnMessageBegin (MSG_ONE_UNRELIABLE, GetUserMsgId ("TempEntity"), NULL, pPlayer);
   pfnWriteByte (TE_TEXTMESSAGE);
   pfnWriteByte (1); // channel
   pfnWriteShort (-8192); // x coordinates * 8192
   pfnWriteShort (-8192); // y coordinates * 8192
   pfnWriteByte (0); // effect (fade in/out)
   pfnWriteByte (255); // initial RED
   pfnWriteByte (255); // initial GREEN
   pfnWriteByte (255); // initial BLUE
   pfnWriteByte (1); // initial ALPHA
   pfnWriteByte (255); // effect RED
   pfnWriteByte (255); // effect GREEN
   pfnWriteByte (255); // effect BLUE
   pfnWriteByte (1); // effect ALPHA
   pfnWriteShort (256); // fade-in time in seconds * 256
   pfnWriteShort (512); // fade-out time in seconds * 256
   pfnWriteShort (256); // hold time in seconds * 256
   pfnWriteString (RACC_WELCOMETEXT); // send welcome message
   pfnMessageEnd (); // end

   // play welcome sound on this client
   pfnClientCommand (pPlayer, "play " RACC_WELCOMESOUND "\n");

   return;
}


void DisplaySpeakerIcon (entity_t *pPlayer, entity_t *pViewerClient, int duration)
{
   // this function is supposed to display that tiny speaker icon above the head of the player
   // whose entity is pointed to by pPlayer, so that pViewerClient sees it, during duration * 10
   // seconds long. That's not exactly what the engine does when a client uses the voice system,
   // but that's all I've found so far to simulate it.

   if (IsNull (pPlayer) || IsNull (pViewerClient))
      return; // reliability check

   // do not use the engine macros here (such as MESSAGE_BEGIN, WRITE_BYTE etc.) since we
   // have to hook these messages too as if they were coming from outside the bot DLL...
   pfnMessageBegin (MSG_ONE, GetUserMsgId ("TempEntity"), NULL, pViewerClient);
   pfnWriteByte (TE_PLAYERATTACHMENT); // thanks to Count Floyd for the trick !
   pfnWriteByte (EntityIndexOf (pPlayer)); // byte (entity index of pPlayer)
   pfnWriteCoord (45 + EyeOriginOf (pPlayer).z - OriginOf (pPlayer).z + 34 * (InputButtonsOf (pPlayer).f_duck_time >= CurrentTime ())); // coord (vertical offset)
   pfnWriteShort (speaker_texture_index); // short (model index of tempent)
   pfnWriteShort (duration); // short (life * 10) e.g. 40 = 4 seconds
   pfnMessageEnd ();
}


void DestroySpeakerIcon (entity_t *pPlayer, entity_t *pViewerClient)
{
   // this function stops displaying any speaker icon above the head of the player whose entity
   // is pointed to by pPlayer, so that pViewerClient doesn't see them anymore. Actually it also
   // stops displaying any player attachment temporary entity. One day I swear I'll get rid of
   // that crap. Soon.

   if (IsNull (pPlayer) || IsNull (pViewerClient))
      return; // reliability check

   // do not use the engine macros here (such as MESSAGE_BEGIN, WRITE_BYTE etc.) since we
   // have to hook these messages too as if they were coming from outside the bot DLL...
   pfnMessageBegin (MSG_ONE, GetUserMsgId ("TempEntity"), NULL, pViewerClient);
   pfnWriteByte (TE_KILLPLAYERATTACHMENTS); // destroy all temporary entities attached to player
   pfnWriteByte (EntityIndexOf (pPlayer)); // byte (entity index of pPlayer)
   pfnMessageEnd ();
}


void DrawLine (entity_t *pClient, vector start, vector end, int life, int red, int green, int blue)
{
   // this function draws a line visible from the client side of the player whose player entity
   // is pointed to by pClient, from the vector location start to the vector location end,
   // which is supposed to last life tenths seconds, and having the color defined by RGB.

   if (IsNull (pClient) || (start == NULLVEC) || (end == NULLVEC))
      return; // reliability check

   // do not use the engine macros here (such as MESSAGE_BEGIN, WRITE_BYTE etc.) since we
   // have to hook these messages too as if they were coming from outside the bot DLL...
   pfnMessageBegin (MSG_ONE_UNRELIABLE, GetUserMsgId ("TempEntity"), NULL, pClient);
   pfnWriteByte (TE_BEAMPOINTS);
   pfnWriteCoord (start.x);
   pfnWriteCoord (start.y);
   pfnWriteCoord (start.z);
   pfnWriteCoord (end.x);
   pfnWriteCoord (end.y);
   pfnWriteCoord (end.z);
   pfnWriteShort (beam_texture_index);
   pfnWriteByte (1); // framestart
   pfnWriteByte (10); // framerate
   pfnWriteByte (life); // life in 0.1's
   pfnWriteByte (10); // width
   pfnWriteByte (0); // noise
   pfnWriteByte (red); // r, g, b
   pfnWriteByte (green); // r, g, b
   pfnWriteByte (blue); // r, g, b
   pfnWriteByte (255); // brightness
   pfnWriteByte (255); // speed of sprite animation
   pfnMessageEnd ();
}
