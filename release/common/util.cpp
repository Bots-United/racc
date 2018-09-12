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
// util.cpp
//

#include "racc.h"

extern map_t map;
extern char mod_name[256];
extern char map_name[256];
extern bool is_dedicated_server;
extern entity_t *listenserver_edict;
extern char *g_argv;
extern bool isFakeClientCommand;
extern int fake_arg_count;
extern char arg[128];
extern sound_t sounds[MAX_SOUNDS + MAX_LOCAL_SOUNDS];
extern int sound_count;
extern texture_t textures[MAX_TEXTURES];
extern int texture_count;
extern bot_personality_t bot_personalities[MAX_BOT_PERSONALITIES];
extern int personality_count;
extern player_t players[MAX_CLIENTS_SUPPORTED_BY_ENGINE];
extern int player_count;
extern bot_t bots[MAX_CLIENTS_SUPPORTED_BY_ENGINE];
extern int bot_count;
extern usermsg_t usermsgs[MAX_USERMSG_TYPES];
extern int usermsgs_count;
extern debug_level_t DebugLevel;


void UTIL_TraceLine (const vector &vecStart, const vector &vecEnd, IGNORE_MONSTERS igmon, IGNORE_GLASS ignoreGlass, entity_t *pentIgnore, TraceResult *ptr)
{
   // this function traces a line dot by dot, starting from vecStart in the direction of vecEnd,
   // ignoring or not monsters (depending on the value of igmon, ignore_monsters or
   // dont_ignore_monsters), and stops at the first obstacle encountered, returning the results
   // of the trace in the TraceResult structure ptr. Such results are (amongst others) the
   // distance traced, the hit surface, the hit plane vector normal, etc. See the TraceResult
   // structure for details. This function allows to specify whether the trace starts "inside"
   // an entity's polygonal model, and if so, to specify that entity in pentIgnore in order to
   // ignore it as a possible obstacle.
   // this is an overloaded prototype to add IGNORE_GLASS in the same way as IGNORE_MONSTERS work.

   // do not use the engine macro TRACE_LINE since we might want to hook this function too...
   pfnTraceLine (vecStart, vecEnd, (igmon == ignore_monsters ? TRUE : FALSE) | (ignoreGlass ? 0x100 : 0), pentIgnore, ptr);
}


void UTIL_TraceLine (const vector &vecStart, const vector &vecEnd, IGNORE_MONSTERS igmon, entity_t *pentIgnore, TraceResult *ptr)
{
   // this function traces a line dot by dot, starting from vecStart in the direction of vecEnd,
   // ignoring or not monsters (depending on the value of igmon, ignore_monsters or
   // dont_ignore_monsters), and stops at the first obstacle encountered, returning the results
   // of the trace in the TraceResult structure ptr. Such results are (amongst others) the
   // distance traced, the hit surface, the hit plane vector normal, etc. See the TraceResult
   // structure for details. This function allows to specify whether the trace starts "inside"
   // an entity's polygonal model, and if so, to specify that entity in pentIgnore in order to
   // ignore it as a possible obstacle.

   // do not use the engine macro TRACE_LINE since we might want to hook this function too...
   pfnTraceLine (vecStart, vecEnd, (igmon == ignore_monsters ? TRUE : FALSE), pentIgnore, ptr);
}


void UTIL_TraceHull (const vector &vecStart, const vector &vecEnd, IGNORE_MONSTERS igmon, int hullNumber, entity_t *pentIgnore, TraceResult *ptr)
{
   // this function traces a hull dot by dot, starting from vecStart in the direction of vecEnd,
   // ignoring or not monsters (depending on the value of igmon, ignore_monsters or
   // dont_ignore_monsters), and stops at the first obstacle encountered, returning the results
   // of the trace in the TraceResult structure ptr, just like TraceLine. Hulls that can be traced
   // (by parameter hull_type) are point_hull (a line), head_hull (the size of a player's head),
   // human_hull (a normal body size) and large_hull (for monsters?). Not all the hulls in the
   // game can be traced here, this function is just useful to give a relative idea of spatial
   // reachability (i.e. can a hostage pass through that tiny hole ?) Also like TraceLine, this
   // function allows to specify whether the trace starts "inside" an entity's polygonal model,
   // and if so, to specify that entity in pentIgnore in order to ignore it as an obstacle.

   // do not use the engine macro TRACE_HULL since we might want to hook this function too...
   pfnTraceHull (vecStart, vecEnd, (igmon == ignore_monsters ? TRUE : FALSE), hullNumber, pentIgnore, ptr);
}


entity_t *UTIL_FindEntityInSphere (entity_t *pentStart, const vector &vecCenter, float flRadius)
{
   // this function returns the first successive element in the list of entities that is
   // located inside a sphere which center is vecCenter and within a radius of flRadius,
   // starting the search at entity pentStart.

   // do not use the engine macro FIND_ENTITY_IN_SPHERE since we might want to hook this function too...
   entity_t *pEntity = pfnFindEntityInSphere (pentStart, vecCenter, flRadius);

   if (!IsNull (pEntity))
      return pEntity;

   return NULL;
}


entity_t *UTIL_FindEntityByString (entity_t *pentStart, const char *szKeyword, const char *szValue)
{
   // this function returns the first successive element in the list of entities that have
   // szValue in the szKeyword field of their entity variables entvars_t structure, starting the
   // search at entity pentStart. Called by UTIL_FindEntityByClassname and such.

   // do not use the engine macro FIND_ENTITY_BY_STRING since we might want to hook this function too...
   entity_t *pEntity = pfnFindEntityByString (pentStart, szKeyword, szValue);

   if (!IsNull (pEntity))
      return pEntity;

   return NULL;
}


entity_t *UTIL_FindEntityByClassname (entity_t *pentStart, const char *szName)
{
   // this function returns the first successive element in the list of entities that have
   // szName as classname in their entity variables entvars_t structure, starting the
   // search at entity pentStart. Classnames are type of entities, e.g. weapon_handgrenade, etc.

   return UTIL_FindEntityByString (pentStart, "classname", szName);
}


entity_t *UTIL_FindEntityByTargetname (entity_t *pentStart, const char *szName)
{
   // this function returns the first successive element in the list of entities that have
   // szName as target name in their entity variables entvars_t structure, starting the
   // search at entity pentStart. Target names are used for buttons that activate doors, etc.

   return UTIL_FindEntityByString (pentStart, "targetname", szName);
}


int GetUserMsgId (const char *msg_name)
{
   // this function returns the user message id of the recorded message named msg_name. Local
   // variables have been made static to speedup recurrent calls of this function.

   static int i;

   // is it a standard engine message (i.e, NOT a user message, already registered by engine) ?
   if (strcmp ("TempEntity", msg_name) == 0)
      return (SVC_TEMPENTITY); // return the correct message ID
   else if (strcmp ("Intermission", msg_name) == 0)
      return (SVC_INTERMISSION); // return the correct message ID
   else if (strcmp ("CDTrack", msg_name) == 0)
      return (SVC_CDTRACK); // return the correct message ID
   else if (strcmp ("WeaponAnim", msg_name) == 0)
      return (SVC_WEAPONANIM); // return the correct message ID
   else if (strcmp ("RoomType", msg_name) == 0)
      return (SVC_ROOMTYPE); // return the correct message ID
   else if (strcmp ("Director", msg_name) == 0)
      return (SVC_DIRECTOR); // return the correct message ID

   // cycle through our known user message types array
   for (i = 0; i < usermsgs_count; i++)
      if (strcmp (usermsgs[i].name, msg_name) == 0)
         return (usermsgs[i].id); // return the id of the user message named msg_name

   // unregistered user message, have the engine register it
   return (pfnRegUserMsg (msg_name, -1)); // ask the engine to register this new message
}


const char *GetUserMsgName (int msg_type)
{
   // this function returns the user message name of the recorded message index msg_type. Local
   // variables have been made static to speedup recurrent calls of this function.

   static int i;

   // is it a standard engine message (i.e, NOT a user message, already registered by engine) ?
   if (msg_type == SVC_TEMPENTITY)
      return ("TempEntity"); // return the correct message name
   else if (msg_type == SVC_INTERMISSION)
      return ("Intermission"); // return the correct message name
   else if (msg_type == SVC_CDTRACK)
      return ("CDTrack"); // return the correct message name
   else if (msg_type == SVC_WEAPONANIM)
      return ("WeaponAnim"); // return the correct message name
   else if (msg_type == SVC_ROOMTYPE)
      return ("RoomType"); // return the correct message name
   else if (msg_type == SVC_DIRECTOR)
      return ("Director"); // return the correct message name

   // cycle through our known user message types array
   for (i = 0; i < usermsgs_count; i++)
      if (usermsgs[i].id == msg_type)
         return (usermsgs[i].name); // return the name of the user message having the msg_type id

   return (NULL); // unregistered user message
}


bool IsReachable (vector v_dest, entity_t *pEntity)
{
   TraceResult tr;
   float curr_height, last_height, distance;
   vector v_check, v_direction;

   if (IsNull (pEntity))
      return FALSE; // reliability check

   v_check = OriginOf (pEntity);
   v_direction = (v_dest - v_check).Normalize (); // 1 unit long

   // check for special case of both the bot and its destination being underwater...
   if ((ContentsOf (OriginOf (pEntity)) == MATTER_WATER) && (ContentsOf (v_dest) == MATTER_WATER))
      return TRUE; // if so, assume it's reachable

   // now check if distance to ground increases more than jump height
   // at points between source and destination...

   UTIL_TraceLine (v_check, v_check + vector (0, 0, -1000), ignore_monsters, pEntity, &tr);

   last_height = tr.flFraction * 1000.0; // height from ground
   distance = (v_dest - v_check).Length (); // distance from goal

   while (distance > 40.0)
   {
      v_check = v_check + v_direction * 40.0; // move 40 units closer to the goal...

      UTIL_TraceLine (v_check, v_check + vector (0, 0, -1000), ignore_monsters, pEntity, &tr);

      curr_height = tr.flFraction * 1000.0; // height from ground

      // is the difference between last and current height higher that the max jump height ?
      if ((last_height - curr_height) > 63.0)
         return FALSE; // if so, assume it's NOT reachable

      last_height = curr_height; // backup current height
      distance = (v_dest - v_check).Length (); // update distance to goal
   }

   return TRUE; // this point is reachable
}


bool IsAtHumanHeight (vector v_location)
{
   TraceResult tr;

   // trace down from v_location to see if it is at human standing height from the ground
   UTIL_TraceLine (v_location, v_location + vector (0, 0, -72), ignore_monsters, NULL, &tr);

   if (tr.flFraction < 1.0)
      return TRUE; // ground was found

   return FALSE; // ground was not found, seems like v_location is in mid-air or outside the map
}


vector DropAtHumanHeight (vector v_location)
{
   TraceResult tr;

   // trace down from v_location and return a vector at human standing height from the ground
   UTIL_TraceLine (v_location, v_location + vector (0, 0, -9999), ignore_monsters, NULL, &tr);

   if (tr.flFraction < 1.0)
      return ((vector) tr.vecEndPos + vector (0, 0, 54)); // ground was found, return a lowered vector

   return (NULLVEC); // aargh, ground was not found !
}


void UTIL_DrawWalkface (entity_t *pClient, walkface_t *pFace, int life, int red, int green, int blue)
{
   // this function is a higher level wrapper for DrawLine() which purpose is to simplify the
   // computation of the boundaries of the face pointed to by face. It draws lines visible from
   // the client side of the player whose player entity is pointed to by pClient, around the
   // perimeter of the face pointed to by pFace, which is supposed to last life tenths seconds,
   // and having the color defined by RGB.

   int corner_index;
   vector v_bound1, v_bound2;

   if (IsNull (pClient) || (pFace == NULL))
      return; // reliability check

   // draw the perimeter around the face
   for (corner_index = 0; corner_index < pFace->corner_count; corner_index++)
   {
      // locate the first vertice of this corner and raise it 2 units up for better visibility
      v_bound1 = pFace->v_corners[corner_index] + vector (0, 0, 2);

      // locate the second vertice of this corner and raise it 2 units up for better visibility
      if (corner_index < pFace->corner_count - 1)
         v_bound2 = pFace->v_corners[corner_index + 1] + vector (0, 0, 2); // next corner in the array
      else
         v_bound2 = pFace->v_corners[0] + vector (0, 0, 2); // loop back to corner zero at last corner

      // draw a line between these 2 points
      DrawLine (pClient, v_bound1, v_bound2, life, red, green, blue);
   }
}


void UTIL_DrawSector (entity_t *pClient, vector v_origin, int life, int red, int green, int blue)
{
   // this function is a higher level wrapper for DrawLine() which purpose is to simplify the
   // computation of the boundaries of the map topologic sector to which belongs the spatial
   // location v_origin. It draws a line visible from the client side of the player whose player
   // entity is pointed to by pClient, around the perimeter of the sector to which v_origin
   // belongs, supposed to last life tenths seconds, and having the color defined by RGB.

   static int i, j, prev_i, prev_j, face_index;
   static float sector_left, sector_right, sector_top, sector_bottom;

   if (IsNull (pClient))
      return; // reliability check

   // first determine the sector to which v_origin belongs
   i = (int) ((v_origin.x - map.v_worldmins.x) / (map.v_worldmaxs.x - map.v_worldmins.x) * map.parallels_count);
   j = (int) ((v_origin.y - map.v_worldmins.y) / (map.v_worldmaxs.y - map.v_worldmins.y) * map.meridians_count);

   // now compute the left, right, top and bottom coordinates indices of the sector
   sector_left = map.v_worldmins.x + (map.v_worldmaxs.x - map.v_worldmins.x) / map.parallels_count * i;
   sector_right = map.v_worldmins.x + (map.v_worldmaxs.x - map.v_worldmins.x) / map.parallels_count * (i + 1);
   sector_top = map.v_worldmins.y + (map.v_worldmaxs.y - map.v_worldmins.y) / map.meridians_count * j;
   sector_bottom = map.v_worldmins.y + (map.v_worldmaxs.y - map.v_worldmins.y) / map.meridians_count * (j + 1);

   // and draw the perimeter around the sector (provided the player sees it)
   DrawLine (pClient, vector (sector_left, sector_top, v_origin.z + 1), vector (sector_left, sector_bottom, v_origin.z + 1), life, red, green, blue);
   DrawLine (pClient, vector (sector_left, sector_bottom, v_origin.z + 1), vector (sector_right, sector_bottom, v_origin.z + 1), life, red, green, blue);
   DrawLine (pClient, vector (sector_right, sector_bottom, v_origin.z + 1), vector (sector_right, sector_top, v_origin.z + 1), life, red, green, blue);
   DrawLine (pClient, vector (sector_right, sector_top, v_origin.z + 1), vector (sector_left, sector_top, v_origin.z + 1), life, red, green, blue);

   if (map.topology[i][j].faces_count == 0)
      return; // reliability check

   // if nothing has changed since last call...
   if ((i == prev_i) && (j == prev_j))
      face_index++; // increment the index of the face to draw (one per frame to avoid overflows)
   else
      face_index = 0; // else reset it to zero

   // anyway, don't exceed the limit number of faces we have to draw
   if (face_index == map.topology[i][j].faces_count)
      face_index = 0;

   // draw this face since it belongs to the sector
   UTIL_DrawWalkface (pClient, map.topology[i][j].faces[face_index], 20, 0, 0, 255);

   prev_i = i; // remember which latitude we were in the topologic array
   prev_j = j; // remember which longitude we were in the topologic array
}


void LoadBotProfiles (void)
{
   // in a certain manner, this function "awakes" the cybernetic population. In extenso, it fills
   // one by one the bot personality slots with the brain, chat files, and info read from the
   // bot personalities directory structure. This directory structure is as follows: each folder
   // in the "racc/profiles" directory is a bot's personality. It *is* a bot. The name of the
   // folder gives the bot its name. Then, for each MOD played, subdirectories are found ; i.e.
   // the bot "dude" will have its personality for Half-Life stored in the "racc/profiles/dude
   // /valve", whereas its personality for Counter-Strike will be stored in the "racc/profiles
   // /dude/cstrike". And so on... The last folder layer is the map being played. For a bot, each
   // map is another world. Each MOD is another world too. The "MOD + map" key/value pair has a
   // unique bot personality associated to it. It would be a mistake to merge the personalities
   // at layers down (regroup by MOD, for example...): would you imagine yourself being dropped
   // suddently on Mars and still trying to act as a Terrian ? Let's sum up: for a bot, each
   // MOD, each map is the whole Universe. The time they play on the server are their Lives.
   // Since living beings get one-life-one-brain, the same bot "dude" you will be seeing playing
   // successively to Half-Life, Deathmatch Classic, TFC on different maps will actually be a
   // brand "new" bot each time. In Half-Life, playing the map "crossfire", this bot's brain
   // will be loaded from, and saved to, the path "racc/profiles/dude/valve/crossfire/". Idem,
   // in Deathmatch Classic, when the map "dcdm5" is being played, this bot's brain will be
   // loaded from, and saved to, the path "racc/profiles/dude/dmc/dcdm5/". So what does this
   // function do ? First counting the number of folders in the "racc/profiles" directory. These
   // are the bots personalities. Then, according to the MOD we play, and to the map we are in,
   // opening that bot's "profile.cfg" file in the right subdirectories, containing some useful
   // data such as the bot's preferred skin for that map, and its skill. Hence this is done, we
   // build that bot's vocal abilities, by filling the array of chat strings about what the bot
   // can say, filling the sound samples paths array about what the bot can speak, and finally
   // opening its HAL brain. If all those actions performed right, the personality is declared
   // valid and this bot can now wait to be called for joining the game. Else, the personality
   // is declared buggy, and we decrease the personality count, whereas the next bot will be
   // set to overwrite that personality. Cruel world...

   // TODO: make the search through directories OS-independent

   HANDLE hFile;
   WIN32_FIND_DATA pFindFileData;
   HRESULT search_result = TRUE;
   FILE *fp;
   char path[256];
   char line_buffer[256];
   char folder_names[MAX_BOT_PERSONALITIES][33];
   int personality_index, length, index, i, fieldstart, fieldstop;
   bool error_in_personality;

   personality_count = 0; // reset the personality count

   // see what's in the "profiles" directory
   hFile = FindFirstFile ("racc/profiles/*", &pFindFileData);
   if (hFile == INVALID_HANDLE_VALUE)
      TerminateOnError ("RACC: profiles directory not found!\n"); // if nonexistent, then stop

   // for each element in this directory...
   while (search_result)
   {
      // if it is a subdirectory...
      if ((pFindFileData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
          && (strcmp (pFindFileData.cFileName, ".") != 0)
          && (strcmp (pFindFileData.cFileName, "..") != 0))
      {
         strncpy (folder_names[personality_count], pFindFileData.cFileName, 32); // save the name
         folder_names[personality_count][32] = 0; // terminate the string
         personality_count++; // increment the personality count
      }

      search_result = FindNextFile (hFile, &pFindFileData); // go and find a handle on the next file
   }

   FindClose (hFile); // close the directory search

   if (personality_count == 0)
      TerminateOnError ("RACC: uninhabited profiles directory!\n"); // if not a single personality, stop

   // tell people what we are doing
   ServerConsole_printf ("RACC: waking up cybernetic population");

   // for each folder in the profiles directory...
   for (personality_index = 0; personality_index < personality_count; )
   {
      error_in_personality = FALSE; // start with no a priori about this personality

      // read the name of the bot from the folder name
      sprintf (bot_personalities[personality_index].name, folder_names[personality_index]);

      // read the bot's skin and skill from the profile.cfg file
      sprintf (path, "racc/profiles/%s/%s/%s/profile.cfg", bot_personalities[personality_index].name, mod_name, map_name);
      fp = fopen (path, "r"); // opens file readonly
      if (fp != NULL)
      {
         while (fgets (line_buffer, 256, fp) != NULL) // reads line per line
         {
            length = strlen (line_buffer); // get length of line
            if ((length > 0) && (line_buffer[length - 1] == '\n'))
               length--; // remove any final '\n'
            line_buffer[length] = 0; // terminate the string

            // is it the "model" line ?
            if (strncmp (line_buffer, "model", 5) == 0)
            {
               index = 0; // let's now parse the line to get the field value

               while ((index < length) && ((line_buffer[index] != ' ') && (line_buffer[index] != '\t')))
                  index++; // reach end of field "model"
               while ((index < length) && ((line_buffer[index] == ' ') || (line_buffer[index] == '\t')))
                  index++; // ignore any tabs or spaces

               fieldstart = index; // save field start position (first character)
               while ((index < length) && ((line_buffer[index] != ' ') && (line_buffer[index] != '\t')))
                  index++; // reach end of field
               fieldstop = index - 1; // save field stop position (last character)
               if (fieldstop > fieldstart + 31)
                  fieldstop = fieldstart + 31; // avoid stack overflows
               for (i = fieldstart; i <= fieldstop; i++)
                  bot_personalities[personality_index].skin[i - fieldstart] = line_buffer[i]; // store the field value in a string
               bot_personalities[personality_index].skin[i - fieldstart] = 0; // terminate the string
            }

            // else is it the "skill" line ?
            if (strncmp (line_buffer, "skill", 5) == 0)
            {
               index = 0; // let's now parse the line to get the field value

               while ((index < length) && ((line_buffer[index] != ' ') && (line_buffer[index] != '\t')))
                  index++; // reach end of field "model"
               while ((index < length) && ((line_buffer[index] == ' ') || (line_buffer[index] == '\t')))
                  index++; // ignore any tabs or spaces

               bot_personalities[personality_index].skill = atoi (&line_buffer[index]); // read the skill
               if (bot_personalities[personality_index].skill < 1)
                  bot_personalities[personality_index].skill = 1; // force skill in lower bounds
               else if (bot_personalities[personality_index].skill > 5)
                  bot_personalities[personality_index].skill = 5; // force skill in upper bounds
            }
         }
         fclose (fp); // close file
      }
      else
         error_in_personality = TRUE;

      // build affirmative messages array
      sprintf (path, "racc/profiles/%s/%s/%s/affirmative.txt", bot_personalities[personality_index].name, mod_name, map_name);
      fp = fopen (path, "r");
      if (fp != NULL)
      {
         while ((bot_personalities[personality_index].text_affirmative_count < 100) && (fgets (line_buffer, 255, fp) != NULL))
         {
            if ((line_buffer[0] == 0) || (line_buffer[0] == '\n') || (line_buffer[0] == '#') || (line_buffer[0] == ';'))
               continue; // ignore line if void or commented
            if (line_buffer[strlen (line_buffer) - 1] == '\n')
               line_buffer[strlen (line_buffer) - 1] = 0; // remove trailing '\n'
            strcpy (bot_personalities[personality_index].text_affirmative[bot_personalities[personality_index].text_affirmative_count], line_buffer); // we have a valid line
            bot_personalities[personality_index].text_affirmative_count++;
         }
         fclose (fp);
      }
      else
         error_in_personality = TRUE;

      // build bye messages array
      sprintf (path, "racc/profiles/%s/%s/%s/bye.txt", bot_personalities[personality_index].name, mod_name, map_name);
      fp = fopen (path, "r");
      if (fp != NULL)
      {
         while ((bot_personalities[personality_index].text_bye_count < 100) && (fgets (line_buffer, 255, fp) != NULL))
         {
            if ((line_buffer[0] == 0) || (line_buffer[0] == '\n') || (line_buffer[0] == '#') || (line_buffer[0] == ';'))
               continue; // ignore line if void or commented
            if (line_buffer[strlen (line_buffer) - 1] == '\n')
               line_buffer[strlen (line_buffer) - 1] = 0; // remove trailing '\n'
            strcpy (bot_personalities[personality_index].text_bye[bot_personalities[personality_index].text_bye_count], line_buffer); // we have a valid line
            bot_personalities[personality_index].text_bye_count++;
         }
         fclose (fp);
      }
      else
         error_in_personality = TRUE;

      // build cant messages array
      sprintf (path, "racc/profiles/%s/%s/%s/cantfollow.txt", bot_personalities[personality_index].name, mod_name, map_name);
      fp = fopen (path, "r");
      if (fp != NULL)
      {
         while ((bot_personalities[personality_index].text_cant_count < 100) && (fgets (line_buffer, 255, fp) != NULL))
         {
            if ((line_buffer[0] == 0) || (line_buffer[0] == '\n') || (line_buffer[0] == '#') || (line_buffer[0] == ';'))
               continue; // ignore line if void or commented
            if (line_buffer[strlen (line_buffer) - 1] == '\n')
               line_buffer[strlen (line_buffer) - 1] = 0; // remove trailing '\n'
            strcpy (bot_personalities[personality_index].text_cant[bot_personalities[personality_index].text_cant_count], line_buffer); // we have a valid line
            bot_personalities[personality_index].text_cant_count++;
         }
         fclose (fp);
      }
      else
         error_in_personality = TRUE;

      // build follow messages array
      sprintf (path, "racc/profiles/%s/%s/%s/follow.txt", bot_personalities[personality_index].name, mod_name, map_name);
      fp = fopen (path, "r");
      if (fp != NULL)
      {
         while ((bot_personalities[personality_index].text_follow_count < 100) && (fgets (line_buffer, 255, fp) != NULL))
         {
            if ((line_buffer[0] == 0) || (line_buffer[0] == '\n') || (line_buffer[0] == '#') || (line_buffer[0] == ';'))
               continue; // ignore line if void or commented
            if (line_buffer[strlen (line_buffer) - 1] == '\n')
               line_buffer[strlen (line_buffer) - 1] = 0; // remove trailing '\n'
            strcpy (bot_personalities[personality_index].text_follow[bot_personalities[personality_index].text_follow_count], line_buffer); // we have a valid line
            bot_personalities[personality_index].text_follow_count++;
         }
         fclose (fp);
      }
      else
         error_in_personality = TRUE;

      // build hello messages array
      sprintf (path, "racc/profiles/%s/%s/%s/hello.txt", bot_personalities[personality_index].name, mod_name, map_name);
      fp = fopen (path, "r");
      if (fp != NULL)
      {
         while ((bot_personalities[personality_index].text_hello_count < 100) && (fgets (line_buffer, 255, fp) != NULL))
         {
            if ((line_buffer[0] == 0) || (line_buffer[0] == '\n') || (line_buffer[0] == '#') || (line_buffer[0] == ';'))
               continue; // ignore line if void or commented
            if (line_buffer[strlen (line_buffer) - 1] == '\n')
               line_buffer[strlen (line_buffer) - 1] = 0; // remove trailing '\n'
            strcpy (bot_personalities[personality_index].text_hello[bot_personalities[personality_index].text_hello_count], line_buffer); // we have a valid line
            bot_personalities[personality_index].text_hello_count++;
         }
         fclose (fp);
      }
      else
         error_in_personality = TRUE;

      // build help messages array
      sprintf (path, "racc/profiles/%s/%s/%s/help.txt", bot_personalities[personality_index].name, mod_name, map_name);
      fp = fopen (path, "r");
      if (fp != NULL)
      {
         while ((bot_personalities[personality_index].text_help_count < 100) && (fgets (line_buffer, 255, fp) != NULL))
         {
            if ((line_buffer[0] == 0) || (line_buffer[0] == '\n') || (line_buffer[0] == '#') || (line_buffer[0] == ';'))
               continue; // ignore line if void or commented
            if (line_buffer[strlen (line_buffer) - 1] == '\n')
               line_buffer[strlen (line_buffer) - 1] = 0; // remove trailing '\n'
            strcpy (bot_personalities[personality_index].text_help[bot_personalities[personality_index].text_help_count], line_buffer); // we have a valid line
            bot_personalities[personality_index].text_help_count++;
         }
         fclose (fp);
      }
      else
         error_in_personality = TRUE;

      // build idle messages array
      sprintf (path, "racc/profiles/%s/%s/%s/idle.txt", bot_personalities[personality_index].name, mod_name, map_name);
      fp = fopen (path, "r");
      if (fp != NULL)
      {
         while ((bot_personalities[personality_index].text_idle_count < 100) && (fgets (line_buffer, 255, fp) != NULL))
         {
            if ((line_buffer[0] == 0) || (line_buffer[0] == '\n') || (line_buffer[0] == '#') || (line_buffer[0] == ';'))
               continue; // ignore line if void or commented
            if (line_buffer[strlen (line_buffer) - 1] == '\n')
               line_buffer[strlen (line_buffer) - 1] = 0; // remove trailing '\n'
            strcpy (bot_personalities[personality_index].text_idle[bot_personalities[personality_index].text_idle_count], line_buffer); // we have a valid line
            bot_personalities[personality_index].text_idle_count++;
         }
         fclose (fp);
      }
      else
         error_in_personality = TRUE;

      // build laugh messages array
      sprintf (path, "racc/profiles/%s/%s/%s/laugh.txt", bot_personalities[personality_index].name, mod_name, map_name);
      fp = fopen (path, "r");
      if (fp != NULL)
      {
         while ((bot_personalities[personality_index].text_laugh_count < 100) && (fgets (line_buffer, 255, fp) != NULL))
         {
            if ((line_buffer[0] == 0) || (line_buffer[0] == '\n') || (line_buffer[0] == '#') || (line_buffer[0] == ';'))
               continue; // ignore line if void or commented
            if (line_buffer[strlen (line_buffer) - 1] == '\n')
               line_buffer[strlen (line_buffer) - 1] = 0; // remove trailing '\n'
            strcpy (bot_personalities[personality_index].text_laugh[bot_personalities[personality_index].text_laugh_count], line_buffer); // we have a valid line
            bot_personalities[personality_index].text_laugh_count++;
         }
         fclose (fp);
      }
      else
         error_in_personality = TRUE;

      // build negative messages array
      sprintf (path, "racc/profiles/%s/%s/%s/negative.txt", bot_personalities[personality_index].name, mod_name, map_name);
      fp = fopen (path, "r");
      if (fp != NULL)
      {
         while ((bot_personalities[personality_index].text_negative_count < 100) && (fgets (line_buffer, 255, fp) != NULL))
         {
            if ((line_buffer[0] == 0) || (line_buffer[0] == '\n') || (line_buffer[0] == '#') || (line_buffer[0] == ';'))
               continue; // ignore line if void or commented
            if (line_buffer[strlen (line_buffer) - 1] == '\n')
               line_buffer[strlen (line_buffer) - 1] = 0; // remove trailing '\n'
            strcpy (bot_personalities[personality_index].text_negative[bot_personalities[personality_index].text_negative_count], line_buffer); // we have a valid line
            bot_personalities[personality_index].text_negative_count++;
         }
         fclose (fp);
      }
      else
         error_in_personality = TRUE;

      // build stay messages array
      sprintf (path, "racc/profiles/%s/%s/%s/stay.txt", bot_personalities[personality_index].name, mod_name, map_name);
      fp = fopen (path, "r");
      if (fp != NULL)
      {
         while ((bot_personalities[personality_index].text_stay_count < 100) && (fgets (line_buffer, 255, fp) != NULL))
         {
            if ((line_buffer[0] == 0) || (line_buffer[0] == '\n') || (line_buffer[0] == '#') || (line_buffer[0] == ';'))
               continue; // ignore line if void or commented
            if (line_buffer[strlen (line_buffer) - 1] == '\n')
               line_buffer[strlen (line_buffer) - 1] = 0; // remove trailing '\n'
            strcpy (bot_personalities[personality_index].text_stay[bot_personalities[personality_index].text_stay_count], line_buffer); // we have a valid line
            bot_personalities[personality_index].text_stay_count++;
         }
         fclose (fp);
      }
      else
         error_in_personality = TRUE;

      // build stop messages array
      sprintf (path, "racc/profiles/%s/%s/%s/stop.txt", bot_personalities[personality_index].name, mod_name, map_name);
      fp = fopen (path, "r");
      if (fp != NULL)
      {
         while ((bot_personalities[personality_index].text_stop_count < 100) && (fgets (line_buffer, 255, fp) != NULL))
         {
            if ((line_buffer[0] == 0) || (line_buffer[0] == '\n') || (line_buffer[0] == '#') || (line_buffer[0] == ';'))
               continue; // ignore line if void or commented
            if (line_buffer[strlen (line_buffer) - 1] == '\n')
               line_buffer[strlen (line_buffer) - 1] = 0; // remove trailing '\n'
            strcpy (bot_personalities[personality_index].text_stop[bot_personalities[personality_index].text_stop_count], line_buffer); // we have a valid line
            bot_personalities[personality_index].text_stop_count++;
         }
         fclose (fp);
      }
      else
         error_in_personality = TRUE;

      // build whine messages array
      sprintf (path, "racc/profiles/%s/%s/%s/whine.txt", bot_personalities[personality_index].name, mod_name, map_name);
      fp = fopen (path, "r");
      if (fp != NULL)
      {
         while ((bot_personalities[personality_index].text_whine_count < 100) && (fgets (line_buffer, 255, fp) != NULL))
         {
            if ((line_buffer[0] == 0) || (line_buffer[0] == '\n') || (line_buffer[0] == '#') || (line_buffer[0] == ';'))
               continue; // ignore line if void or commented
            if (line_buffer[strlen (line_buffer) - 1] == '\n')
               line_buffer[strlen (line_buffer) - 1] = 0; // remove trailing '\n'
            strcpy (bot_personalities[personality_index].text_whine[bot_personalities[personality_index].text_whine_count], line_buffer); // we have a valid line
            bot_personalities[personality_index].text_whine_count++;
         }
         fclose (fp);
      }
      else
         error_in_personality = TRUE;

      // look for any affirmative voice samples
      bot_personalities[personality_index].audio_affirmative_count = -1; // first reset the count
      search_result = TRUE; // reset the error flag to its initial state
      sprintf (path, "racc/profiles/%s/%s/%s/affirmative*.wav", bot_personalities[personality_index].name, mod_name, map_name);
      hFile = FindFirstFile (path, &pFindFileData);
      if (hFile == INVALID_HANDLE_VALUE)
         search_result = FALSE; // if nonexistent, then stop
      while (search_result) // for each element found...
      {
         bot_personalities[personality_index].audio_affirmative_count++; // increment the count
         search_result = FindNextFile (hFile, &pFindFileData); // go and find a handle on the next file
      }
      FindClose (hFile); // close the file search

      // look for any alert voice samples
      bot_personalities[personality_index].audio_alert_count = -1; // first reset the count
      search_result = TRUE; // reset the error flag to its initial state
      sprintf (path, "racc/profiles/%s/%s/%s/alert*.wav", bot_personalities[personality_index].name, mod_name, map_name);
      hFile = FindFirstFile (path, &pFindFileData);
      if (hFile == INVALID_HANDLE_VALUE)
         search_result = FALSE; // if nonexistent, then stop
      while (search_result) // for each element found...
      {
         bot_personalities[personality_index].audio_alert_count++; // increment the count
         search_result = FindNextFile (hFile, &pFindFileData); // go and find a handle on the next file
      }
      FindClose (hFile); // close the file search

      // look for any attacking voice samples
      bot_personalities[personality_index].audio_attacking_count = -1; // first reset the count
      search_result = TRUE; // reset the error flag to its initial state
      sprintf (path, "racc/profiles/%s/%s/%s/attacking*.wav", bot_personalities[personality_index].name, mod_name, map_name);
      hFile = FindFirstFile (path, &pFindFileData);
      if (hFile == INVALID_HANDLE_VALUE)
         search_result = FALSE; // if nonexistent, then stop
      while (search_result) // for each element found...
      {
         bot_personalities[personality_index].audio_attacking_count++; // increment the count
         search_result = FindNextFile (hFile, &pFindFileData); // go and find a handle on the next file
      }
      FindClose (hFile); // close the file search

      // look for any firstspawn voice samples
      bot_personalities[personality_index].audio_firstspawn_count = -1; // first reset the count
      search_result = TRUE; // reset the error flag to its initial state
      sprintf (path, "racc/profiles/%s/%s/%s/firstspawn*.wav", bot_personalities[personality_index].name, mod_name, map_name);
      hFile = FindFirstFile (path, &pFindFileData);
      if (hFile == INVALID_HANDLE_VALUE)
         search_result = FALSE; // if nonexistent, then stop
      while (search_result) // for each element found...
      {
         bot_personalities[personality_index].audio_firstspawn_count++; // increment the count
         search_result = FindNextFile (hFile, &pFindFileData); // go and find a handle on the next file
      }
      FindClose (hFile); // close the file search

      // look for any inposition voice samples
      bot_personalities[personality_index].audio_inposition_count = -1; // first reset the count
      search_result = TRUE; // reset the error flag to its initial state
      sprintf (path, "racc/profiles/%s/%s/%s/inposition*.wav", bot_personalities[personality_index].name, mod_name, map_name);
      hFile = FindFirstFile (path, &pFindFileData);
      if (hFile == INVALID_HANDLE_VALUE)
         search_result = FALSE; // if nonexistent, then stop
      while (search_result) // for each element found...
      {
         bot_personalities[personality_index].audio_inposition_count++; // increment the count
         search_result = FindNextFile (hFile, &pFindFileData); // go and find a handle on the next file
      }
      FindClose (hFile); // close the file search

      // look for any negative voice samples
      bot_personalities[personality_index].audio_negative_count = -1; // first reset the count
      search_result = TRUE; // reset the error flag to its initial state
      sprintf (path, "racc/profiles/%s/%s/%s/negative*.wav", bot_personalities[personality_index].name, mod_name, map_name);
      hFile = FindFirstFile (path, &pFindFileData);
      if (hFile == INVALID_HANDLE_VALUE)
         search_result = FALSE; // if nonexistent, then stop
      while (search_result) // for each element found...
      {
         bot_personalities[personality_index].audio_negative_count++; // increment the count
         search_result = FindNextFile (hFile, &pFindFileData); // go and find a handle on the next file
      }
      FindClose (hFile); // close the file search

      // look for any report voice samples
      bot_personalities[personality_index].audio_report_count = -1; // first reset the count
      search_result = TRUE; // reset the error flag to its initial state
      sprintf (path, "racc/profiles/%s/%s/%s/report*.wav", bot_personalities[personality_index].name, mod_name, map_name);
      hFile = FindFirstFile (path, &pFindFileData);
      if (hFile == INVALID_HANDLE_VALUE)
         search_result = FALSE; // if nonexistent, then stop
      while (search_result) // for each element found...
      {
         bot_personalities[personality_index].audio_report_count++; // increment the count
         search_result = FindNextFile (hFile, &pFindFileData); // go and find a handle on the next file
      }
      FindClose (hFile); // close the file search

      // look for any reporting voice samples
      bot_personalities[personality_index].audio_reporting_count = -1; // first reset the count
      search_result = TRUE; // reset the error flag to its initial state
      sprintf (path, "racc/profiles/%s/%s/%s/reporting*.wav", bot_personalities[personality_index].name, mod_name, map_name);
      hFile = FindFirstFile (path, &pFindFileData);
      if (hFile == INVALID_HANDLE_VALUE)
         search_result = FALSE; // if nonexistent, then stop
      while (search_result) // for each element found...
      {
         bot_personalities[personality_index].audio_reporting_count++; // increment the count
         search_result = FindNextFile (hFile, &pFindFileData); // go and find a handle on the next file
      }
      FindClose (hFile); // close the file search

      // look for any seegrenade voice samples
      bot_personalities[personality_index].audio_seegrenade_count = -1; // first reset the count
      search_result = TRUE; // reset the error flag to its initial state
      sprintf (path, "racc/profiles/%s/%s/%s/seegrenade*.wav", bot_personalities[personality_index].name, mod_name, map_name);
      hFile = FindFirstFile (path, &pFindFileData);
      if (hFile == INVALID_HANDLE_VALUE)
         search_result = FALSE; // if nonexistent, then stop
      while (search_result) // for each element found...
      {
         bot_personalities[personality_index].audio_seegrenade_count++; // increment the count
         search_result = FindNextFile (hFile, &pFindFileData); // go and find a handle on the next file
      }
      FindClose (hFile); // close the file search

      // look for any takingdamage voice samples
      bot_personalities[personality_index].audio_takingdamage_count = -1; // first reset the count
      search_result = TRUE; // reset the error flag to its initial state
      sprintf (path, "racc/profiles/%s/%s/%s/takingdamage*.wav", bot_personalities[personality_index].name, mod_name, map_name);
      hFile = FindFirstFile (path, &pFindFileData);
      if (hFile == INVALID_HANDLE_VALUE)
         search_result = FALSE; // if nonexistent, then stop
      while (search_result) // for each element found...
      {
         bot_personalities[personality_index].audio_takingdamage_count++; // increment the count
         search_result = FindNextFile (hFile, &pFindFileData); // go and find a handle on the next file
      }
      FindClose (hFile); // close the file search

      // look for any throwgrenade voice samples
      bot_personalities[personality_index].audio_throwgrenade_count = -1; // first reset the count
      search_result = TRUE; // reset the error flag to its initial state
      sprintf (path, "racc/profiles/%s/%s/%s/throwgrenade*.wav", bot_personalities[personality_index].name, mod_name, map_name);
      hFile = FindFirstFile (path, &pFindFileData);
      if (hFile == INVALID_HANDLE_VALUE)
         search_result = FALSE; // if nonexistent, then stop
      while (search_result) // for each element found...
      {
         bot_personalities[personality_index].audio_throwgrenade_count++; // increment the count
         search_result = FindNextFile (hFile, &pFindFileData); // go and find a handle on the next file
      }
      FindClose (hFile); // close the file search

      // look for any victory voice samples
      bot_personalities[personality_index].audio_victory_count = -1; // first reset the count
      search_result = TRUE; // reset the error flag to its initial state
      sprintf (path, "racc/profiles/%s/%s/%s/victory*.wav", bot_personalities[personality_index].name, mod_name, map_name);
      hFile = FindFirstFile (path, &pFindFileData);
      if (hFile == INVALID_HANDLE_VALUE)
         search_result = FALSE; // if nonexistent, then stop
      while (search_result) // for each element found...
      {
         bot_personalities[personality_index].audio_victory_count++; // increment the count
         search_result = FindNextFile (hFile, &pFindFileData); // go and find a handle on the next file
      }
      FindClose (hFile); // close the file search

      // prepare the bot's HAL brain for awakening and wake it up
      if (!error_in_personality)
      {
         PrepareHALBrainForPersonality (&bot_personalities[personality_index]); // check the bot HAL brain
         error_in_personality = LoadHALBrainForPersonality (&bot_personalities[personality_index]); // wake the bot's HAL brain up
      }

      // prepare the bot's navigation brain for awakening and wake it up
      if (!error_in_personality)
      {
         PrepareNavBrainForPersonality (&bot_personalities[personality_index]); // check the bot navigation nodes
         error_in_personality = LoadNavBrainForPersonality (&bot_personalities[personality_index]); // wake the bot's navigation nodes brain up
      }

      // has this personality finally woke up without problems ?
      if (!error_in_personality)
      {
         ServerConsole_printf ("."); // print a trailing dot as a progress bar
         personality_index++; // okay, this personality is valid, process the next one
      }
      else
         personality_count--; // damn, this one is buggy, forget it
   }

   ServerConsole_printf (" done\n"); // finished, end the progress bar
   return;
}


void FakeClientCommand (entity_t *pFakeClient, const char *fmt, ...)
{
   // the purpose of this function is to provide fakeclients (bots) with the same client
   // command-scripting advantages (putting multiple commands in one line between semicolons)
   // as real players. It is an improved version of botman's FakeClientCommand, in which you
   // supply directly the whole string as if you were typing it in the bot's "console". It
   // is supposed to work exactly like the pfnClientCommand (server-sided client command).

   va_list argptr;
   static char command[256];
   int length, fieldstart, fieldstop, i, index, stringindex = 0;

   if (IsNull (pFakeClient))
      return; // reliability check

   // concatenate all the arguments in one string
   va_start (argptr, fmt);
   vsprintf (command, fmt, argptr);
   va_end (argptr);

   if ((command == NULL) || (*command == 0))
      return; // if nothing in the command buffer, return

   isFakeClientCommand = TRUE; // set the "fakeclient command" flag
   length = strlen (command); // get the total length of the command string

   // process all individual commands (separated by a semicolon) one each a time
   while (stringindex < length)
   {
      fieldstart = stringindex; // save field start position (first character)
      while ((stringindex < length) && (command[stringindex] != ';'))
         stringindex++; // reach end of field
      if (command[stringindex - 1] == '\n')
         fieldstop = stringindex - 2; // discard any trailing '\n' if needed
      else
         fieldstop = stringindex - 1; // save field stop position (last character before semicolon or end)
      for (i = fieldstart; i <= fieldstop; i++)
         g_argv[i - fieldstart] = command[i]; // store the field value in the g_argv global string
      g_argv[i - fieldstart] = 0; // terminate the string
      stringindex++; // move the overall string index one step further to bypass the semicolon

      index = 0;
      fake_arg_count = 0; // let's now parse that command and count the different arguments

      // count the number of arguments
      while (index < i - fieldstart)
      {
         while ((index < i - fieldstart) && (g_argv[index] == ' '))
            index++; // ignore spaces

         // is this field a group of words between quotes or a single word ?
         if (g_argv[index] == '"')
         {
            index++; // move one step further to bypass the quote
            while ((index < i - fieldstart) && (g_argv[index] != '"'))
               index++; // reach end of field
            index++; // move one step further to bypass the quote
         }
         else
            while ((index < i - fieldstart) && (g_argv[index] != ' '))
               index++; // this is a single word, so reach the end of field

         fake_arg_count++; // we have processed one argument more
      }

      ClientCommand (pFakeClient); // tell now the MOD DLL to execute this ClientCommand...
   }

   g_argv[0] = 0; // when it's done, reset the g_argv field
   isFakeClientCommand = FALSE; // reset the "fakeclient command" flag
   fake_arg_count = 0; // and the argument count
}


const char *GetArg (const char *command, int arg_number)
{
   // the purpose of this function is to provide fakeclients (bots) with the same Cmd_Argv
   // convenience the engine provides to real clients. This way the handling of real client
   // commands and bot client commands is exactly the same, just have a look in engine.cpp
   // for the hooking of pfnCmd_Argc, pfnCmd_Args and pfnCmd_Argv, which redirects the call
   // either to the actual engine functions (when the caller is a real client), either on
   // our function here, which does the same thing, when the caller is a bot.

   int length, i, index = 0, arg_count = 0, fieldstart, fieldstop;

   arg[0] = 0; // reset arg
   length = strlen (command); // get length of command

   // while we have not reached end of line
   while ((index < length) && (arg_count <= arg_number))
   {
      while ((index < length) && (command[index] == ' '))
         index++; // ignore spaces

      // is this field multi-word between quotes or single word ?
      if (command[index] == '"')
      {
         index++; // move one step further to bypass the quote
         fieldstart = index; // save field start position
         while ((index < length) && (command[index] != '"'))
            index++; // reach end of field
         fieldstop = index - 1; // save field stop position
         index++; // move one step further to bypass the quote
      }
      else
      {
         fieldstart = index; // save field start position
         while ((index < length) && (command[index] != ' '))
            index++; // reach end of field
         fieldstop = index - 1; // save field stop position
      }

      // is this argument we just processed the wanted one ?
      if (arg_count == arg_number)
      {
         for (i = fieldstart; i <= fieldstop; i++)
            arg[i - fieldstart] = command[i]; // store the field value in a string
         arg[i - fieldstart] = 0; // terminate the string
      }

      arg_count++; // we have processed one argument more
   }

   return (&arg[0]); // returns the wanted argument
}


texture_t *FindTextureByName (const char *texture_name)
{
   // given a texture name, this function finds the actual texture entry in the global texture
   // database, so that we get access to its type (metal, wood, concrete, dust, computer etc.)
   // If texture name is not found in the database, return the default one. NOTE: this routine
   // should ONLY be called if the current texture under a player changes !

   // TODO: pre-sort texture names and perform faster binary search here.

   int index;

   if (texture_name == NULL)
      return (&textures[0]); // reliability check

   // strip eventual leading '-0' or '+0~' or '{' or '!'
   if ((*texture_name == '-') || (*texture_name == '+'))
      texture_name += 2;
   if ((*texture_name == '{') || (*texture_name == '!') || (*texture_name == '~') || (*texture_name == ' '))
      texture_name++;

   // for each texture in the global array, compare its name case-insensitively with that one
   for (index = 0; index < texture_count; index++)
      if (strnicmp (texture_name, textures[index].name, CBTEXTURENAMEMAX - 1) == 0)
         return (&textures[index]); // when found, return a pointer to the texture entry

   // damnit, texture not found !
   if (DebugLevel.ears > 1)
      ServerConsole_printf ("RACC: FindTextureByName(): texture \"%s\" not found in database\n", texture_name);

   return (&textures[0]); // return default texture
}


bool FileExists (char *filename)
{
   // this function tests if a file exists by attempting to open it

   FILE *fp = fopen (filename, "rb"); // try to open the file

   // have we got a valid file pointer in return ?
   if (fp != NULL)
   {
      fclose (fp); // then the file exists, close it
      return (TRUE); // ...and return TRUE
   }

   return (FALSE); // failed to open the file, assume it doesn't exist
}


walkface_t *WalkfaceUnder (entity_t *pEntity)
{
   // this function returns the ground face supporting pEntity on floor. All the local variables
   // have been made static to speedup recurrent calls of this function.

   static int i, j, face_index, corner_index;
   static float angle;
   static walkface_t *pFace;
   static vector v_entity_bottom, v_bound1, v_bound2;

   // first reset the face pointer
   pFace = NULL;

   if (IsNull (pEntity) || !IsOnFloor (pEntity))
      return (pFace); // reliability check

   // get a quick access to the entity bottom origin
   v_entity_bottom = BottomOriginOf (pEntity); // get entity bottom origin

   // compute the latitude and longitude in the topologic array
   i = (int) ((v_entity_bottom.x - map.v_worldmins.x) / (map.v_worldmaxs.x - map.v_worldmins.x) * map.parallels_count);
   j = (int) ((v_entity_bottom.y - map.v_worldmins.y) / (map.v_worldmaxs.y - map.v_worldmins.y) * map.meridians_count);

   // handle the cases where the delimiter is just on the right or upper edge of the array...
   if (i > map.parallels_count - 1)
      i = map.parallels_count - 1;
   if (j > map.meridians_count - 1)
      j = map.meridians_count - 1;

   // loop through all the face we know to be in this topological zone
   for (face_index = 0; face_index < map.topology[i][j].faces_count; face_index++)
   {
      pFace = map.topology[i][j].faces[face_index]; // quick access to the face

      angle = 0; // reset angle

      // loop though the corners of this face...
      for (corner_index = 0; corner_index < pFace->corner_count; corner_index++)
      {
         // locate the first vertice of this corner
         v_bound1 = pFace->v_corners[corner_index];

         // locate the second vertice of this corner
         if (corner_index < pFace->corner_count - 1)
            v_bound2 = pFace->v_corners[corner_index + 1]; // next corner in the array
         else
            v_bound2 = pFace->v_corners[0]; // loop back to corner zero at last corner

         // sum up all the angles corner - entity - next corner and check if we have 360 (thx botman)
         angle += abs (AngleBetweenVectors ((v_bound1 - v_entity_bottom), (v_bound2 - v_entity_bottom)));
      }

      // if the resulting angle is close to 360, then the point is likely to be on the face
      if (abs (WrapAngle (angle)) < 5)
         return (pFace); // assume entity is on this face
   }

   return (NULL); // not found a face on which entity could be on...
}


vector NearestDelimiterOf (entity_t *pEntity)
{
   // this function is a superset of the WalkfaceUnder() function. It returns, amongst the
   // delimiters of the ground face supporting pEntity on the floor, the closest delimiter to
   // pEntity. Local variables have been made static to speedup recurrent calls of this function.

   static walkface_t *pFace;
   static vector v_entity_bottom, v_nearest;
   static float distance, nearest_distance;
   static int index;

   if (IsNull (pEntity) || !IsOnFloor (pEntity))
      return (NULLVEC); // reliability check

   // get a quick access to the entity bottom origin and to the face it stands on
   v_entity_bottom = BottomOriginOf (pEntity); // get entity bottom origin
   pFace = WalkfaceUnder (pEntity); // get the walkable face it is on

   if (pFace == NULL)
      return (NULLVEC); // reliability check

   nearest_distance = 9999.0;

   // for each delimiter on this face, loop for the nearest
   for (index = 0; index < pFace->corner_count; index++)
   {
      distance = (pFace->v_delimiters[index] - v_entity_bottom).Length (); // delimiter to entity
      if (distance < nearest_distance)
      {
         nearest_distance = distance; // remember the nearest distance
         v_nearest = pFace->v_delimiters[index]; // remember the nearest delimiter
      }
   }

   return (v_nearest); // and return it
}


bool SegmentBelongsToSector (const vector &v_bound1, const vector &v_bound2, int sector_i, int sector_j)
{
   // this function returns TRUE if the *flattened* segment bounded by the v_bound1 and v_bound2
   // vertices beclongs to the topological sector whose position in the global array is [sector_i]
   // [sector_j], FALSE otherwise. The local variables of this function are defined static in
   // order to speedup recursive calls of this function, which is extensively used in
   // LookDownOnTheWorld().

   // Code courtesy of Paul "Cheesemonster" Murphy (thanks mate !).

   static float sector_left, sector_right, sector_top, sector_bottom, x, y, m, b;
   vector sector_min, sector_max, line_min,line_max;

   // first compute the left, right, top and bottom coordinates indices of the sector
   sector_left = map.v_worldmins.x + (map.v_worldmaxs.x - map.v_worldmins.x) / map.parallels_count * sector_i;
   sector_right = map.v_worldmins.x + (map.v_worldmaxs.x - map.v_worldmins.x) / map.parallels_count * (sector_i + 1);
   sector_top = map.v_worldmins.y + (map.v_worldmaxs.y - map.v_worldmins.y) / map.meridians_count * sector_j;
   sector_bottom = map.v_worldmins.y + (map.v_worldmaxs.y - map.v_worldmins.y) / map.meridians_count * (sector_j + 1);

   // get minimum and maximum vectors and stored in vector
   sector_min = vector (sector_left, sector_bottom, 0);
   sector_max = vector (sector_right, sector_top, 0);

   // its obvious it is in sector, if one or both points are inside the box
   if (((v_bound1.x >= sector_left) && (v_bound1.x <= sector_right) && (v_bound1.y >= sector_bottom) && (v_bound1.y <= sector_top))
       || ((v_bound2.x >= sector_left) && (v_bound2.x <= sector_right) && (v_bound2.y >= sector_bottom) && (v_bound2.y <= sector_top)))
      return (TRUE); // if so, segment belongs to sector

   // vertical line ? (!!!undefined gradient!!!)
   if (v_bound1.x == v_bound2.x)
   {
      if ((v_bound1.x >= sector_left) && (v_bound1.x <= sector_right))
      {
         if (v_bound1.y > v_bound2.y)
            return ((v_bound1.y >= sector_top) && (v_bound2.y <= sector_bottom)); // both above or below (not both)
         else
            return ((v_bound2.y >= sector_top) && (v_bound1.y <= sector_bottom));
      }

      return FALSE; // only vertical line can hit top or bottom of sector
   }

   // horizontal line ? (!!!zero gradient!!!)
   else if (v_bound1.y == v_bound2.y)
   {
      if ((v_bound1.y >= sector_bottom) && (v_bound1.y <= sector_top))
      {
         if (v_bound1.x > v_bound2.x)
            return ((v_bound1.x >= sector_right) && (v_bound2.x <= sector_left)); // both left and right (not left and left/right and right of box etc)
         else
            return ((v_bound2.x >= sector_right) && (v_bound1.x <= sector_left));
      }

      return (FALSE); // only vertical line can hit top or bottom of sector
   }

   // arrange the bounds of the segment in the right order
   else if (v_bound1.x < v_bound2.x)
   {
      line_min = v_bound1;
      line_max = v_bound2;
   }
   else
   {
      line_min = v_bound2;
      line_max = v_bound1;
   }

   m = (v_bound2.x - v_bound1.y) / (v_bound2.x - v_bound1.x);
   b = v_bound1.y - m * v_bound1.x; // y = mx + b

   y = m * sector_left + b; // point must lie inside box AND inside line region.

   // check the left side of the sector....
   if ((y >= sector_bottom) && (y <= sector_top) && (y >= line_min.y) && (y <= line_max.y))
   {
      // intersects box, must also check if this point is INSIDE the length
      // of the line (by checking a bounding box around the line)

      // x = (y - b) / m
      x = (y - b) / m;

      if ((x >= line_min.x) && (x <= line_max.x))
         return (TRUE);
   }

   y = m * sector_right + b;

   // check the right side...
   if ((y >= sector_bottom) && (y <= sector_top) && (y >= line_min.y) && (y <= line_max.y))
   {
      x = (y - b) / m;

      if ((x >= line_min.x) && (x <= line_max.x))
         return (TRUE);
   }

   // must now change subject to x (checking for y point now)
   // plug y into the equation which is now sector_top/sector_bottom
   x = (sector_top - b) / m;

   // check to see where it intersects on x axis with top of the box
   if ((x >= sector_left) && (x <= sector_right) && (x >= line_min.x) && (x <= line_max.x))
   {
      // need to also find Y !!! Check if its also inside line region!

      y = m * x + b;

      if ((y >= line_min.y) && (y <= line_max.y))
         return (TRUE);
   }

   x = (sector_bottom - b) / m;

   // check to see where it intersects on x axis with bottom of the box
   if ((x >= sector_left) && (x <= sector_right) && (x >= line_min.x) && (x <= line_max.x))
   {
      y = m * x + b;

      if ((y >= line_min.y) && (y <= line_max.y))
         return (TRUE);
   }

   return (FALSE); // definitely no intersection possible
}


bool LoadWorldMap (void)
{
   // this function checks if a world map created by LookDownOnTheWorld() exists in a .map file
   // on disk, and in case it does, opens and loads it, and returns TRUE if the loading process
   // completed successfully. If such a map doesn't exist or an error occured in the loading
   // process, this function returns FALSE. The format of a .map file is divided into chunks.
   // For an explanation of the different chunks of which .map files are made, refer to the
   // SaveWorldMap() function.

   FILE *fp, mfp;
   char bsp_file_path[256], map_file_path[256];
   int bsp_file_size, map_file_size, i, j, face_index, corner_index, array_index;

   // first look for a valid worldmap file...
   sprintf (bsp_file_path, "maps/%s.bsp", map_name); // build BSP file path
   sprintf (map_file_path, "racc/worldmaps/%s/%s.map", mod_name, map_name); // build map file path

   // load the bsp file and get its actual size (can't fail to do this, the map is already booting)
   memset (&mfp, 0, sizeof (mfp));
   mfp._ptr = (char *) pfnLoadFileForMe (bsp_file_path, &bsp_file_size); // load bsp file
   free (&mfp._ptr); // and close it

   // load the map file and get the size it claims the BSP file to have
   fp = fopen (map_file_path, "rb");
   if (fp == NULL)
      return (FALSE); // map file not found, return an error condition
   fseek (fp, 18, SEEK_SET); // get the recorded file size directly at its offset
   fread (&map_file_size, sizeof (map_file_size), 1, fp);
   fclose (fp); // close the worldmap file (we just needed this data)

   // is the recorded file size NOT the same as the actual BSP file size ?
   if (bsp_file_size != map_file_size)
      return (FALSE); // map file is badly authenticated, return an error condition

   // else world map file is OK, load its data completely this time
   fp = fopen (map_file_path, "rb"); // open such a file in binary read mode
   if (fp == NULL)
      return (FALSE); // error, can't open worldmap file

   fseek (fp, 0, SEEK_SET); // seek at start of file

   fseek (fp, sizeof ("RACCMAP"), SEEK_CUR); // skip the RACCMAP tag
   fseek (fp, sizeof ("[filesize]") - 1, SEEK_CUR); // skip the filesize chunk tag
   fseek (fp, sizeof (int), SEEK_CUR); // skip the filesize chunk data
   fseek (fp, sizeof ("[worldsize]") - 1, SEEK_CUR); // skip the worldsize chunk tag

   // read the world size chunk data
   fread (&map.v_worldmins, sizeof (vector), 1, fp);
   fread (&map.v_worldmaxs, sizeof (vector), 1, fp);

   fseek (fp, sizeof ("[walkfaces]") - 1, SEEK_CUR); // seek at start of walkable faces chunk data

   // read the walkable faces lump (and mallocate for it on the fly...)
   fread (&map.walkfaces_count, sizeof (int), 1, fp);
   if (map.walkfaces_count == 0)
      return (FALSE); // error, no walkable faces have been recorded

   ServerConsole_printf ("RACC: Fetching world map"); // tell people what we are doing

   map.walkfaces = (walkface_t *) malloc (map.walkfaces_count * sizeof (walkface_t));
   if (map.walkfaces == NULL)
      TerminateOnError ("Fatal: Unable to allocate enough memory for world analysis\n");
   for (face_index = 0; face_index < map.walkfaces_count; face_index++)
   {
      fread (&map.walkfaces[face_index].corner_count, sizeof (int), 1, fp); // read # of corners for this face
      map.walkfaces[face_index].v_corners = (vector *) malloc (map.walkfaces[face_index].corner_count * sizeof (vector));
      if (map.walkfaces[face_index].v_corners == NULL)
         TerminateOnError ("Fatal: Unable to allocate enough memory for world analysis\n");
      map.walkfaces[face_index].v_delimiters = (vector *) malloc (map.walkfaces[face_index].corner_count * sizeof (vector));
      if (map.walkfaces[face_index].v_delimiters == NULL)
         TerminateOnError ("Fatal: Unable to allocate enough memory for world analysis\n");
      for (corner_index = 0; corner_index < map.walkfaces[face_index].corner_count; corner_index++)
      {
         fread (&map.walkfaces[face_index].v_corners[corner_index], sizeof (vector), 1, fp);
         fread (&map.walkfaces[face_index].v_delimiters[corner_index], sizeof (vector), 1, fp);
      }
   }

   fseek (fp, sizeof ("[topology]") - 1, SEEK_CUR); // seek at start of topology chunk data

   // read the topology chunk data (and mallocate for it on the fly...)
   fread (&map.parallels_count, sizeof (map.parallels_count), 1, fp);
   fread (&map.meridians_count, sizeof (map.meridians_count), 1, fp);
   for (i = 0; i < map.parallels_count; i++)
   {
      for (j = 0; j < map.meridians_count; j++)
      {
         fread (&map.topology[i][j].faces_count, sizeof (int), 1, fp); // read # of faces for this zone

         // if we actually need to mallocate some space for the face pointers, do it
         if (map.topology[i][j].faces_count > 0)
            map.topology[i][j].faces = (walkface_t **) malloc (map.topology[i][j].faces_count * sizeof (walkface_t *));
         else
            map.topology[i][j].faces = (walkface_t **) malloc (sizeof (walkface_t *)); // failsafe pointer

         // check for validity of malloced space
         if (map.topology[i][j].faces == NULL)
            TerminateOnError ("Fatal: Unable to allocate enough memory for world analysis\n");

         // init the first pointer to some failsafe value
         map.topology[i][j].faces[0] = &map.walkfaces[0];

         // translate each face array index into a pointer
         for (face_index = 0; face_index < map.topology[i][j].faces_count; face_index++)
         {
            fread (&array_index, sizeof (int), 1, fp);
            
            // test this index against overflow
            if ((array_index < 0) || (array_index >= map.walkfaces_count))
               TerminateOnError ("LoadWorldMap(): bad face array index %d (max %d) at [%d][%d], index %d/%d\n", array_index, map.walkfaces_count - 1, i, j, face_index, map.topology[i][j].faces_count);

            map.topology[i][j].faces[face_index] = (walkface_t *) ((unsigned long) map.walkfaces + array_index * sizeof (walkface_t));

            // test this pointer against access violation (pointers are plain evil)
            if ((map.topology[i][j].faces[face_index] < &map.walkfaces[0]) || (map.topology[i][j].faces[face_index] > &map.walkfaces[map.walkfaces_count - 1]))
               TerminateOnError ("LoadWorldMap(): bad face pointer %d (range %d - %d) at [%d][%d], index %d/%d\n", map.topology[i][j].faces[face_index], &map.walkfaces[0], &map.walkfaces[map.walkfaces_count - 1], i, j, face_index, map.topology[i][j].faces_count);
         }
      }

      ServerConsole_printf ("."); // print a trailing dot as a progress bar
   }

   // finished, terminate the progress bar
   ServerConsole_printf (" done\n   %d parallels, %d meridians, %.2f kilobytes world data\n",
                         map.parallels_count, map.meridians_count, (float) ftell (fp) / 1024);

   fclose (fp); // close the file
   return (TRUE); // and return the error state (no error)
}


int SaveWorldMap (int bsp_file_size)
{
   // this function saves the loaded world map created by LookDownOnTheWorld() in a .map file on
   // disk. The format of this file is divided into chunks. The first chunk is an authentication
   // tag, which has the value "RACCMAP" followed by an end of string null marker. All the other
   // chunks are explicitly named in ASCII characters in the file, preceding the chunk data. The
   // chunks coming after the "RACCMAP" marker are :
   // [filesize] - tells the size of the BSP file this world map has been drawn for
   // [worldsize] - gives info about the virtual world's bounding box mins and maxs limits
   // [walkfaces] - array of the walkable faces data, involving their corners and delimiters
   // [topology] - number of parallels and meridians in this map, followed by the zone hashtable

   FILE *fp;
   char map_file_path[256];
   int i, j, face_index, corner_index, array_index, size;

   // build the world map file path
   sprintf (map_file_path, "racc/worldmaps/%s/%s.map", mod_name, map_name);

   fp = fopen (map_file_path, "wb"); // open or create such a file in binary write mode
   if (fp == NULL)
      TerminateOnError ("Unable to save new worldmap to %s\n", map_file_path);

   fseek (fp, 0, SEEK_SET); // seek at start

   // write the authentication chunk
   fwrite ("RACCMAP", sizeof ("RACCMAP"), 1, fp);

   // don't write the file size chunk yet (will do this when the map will be fully saved)
   // this ensure an error in the saving process won't let a badly authenticated file on disk
   fwrite ("[filesize]", sizeof ("[filesize]") - 1, 1, fp);
   fwrite ("\0\0\0\0", sizeof (int), 1, fp); // fill the field with zeroes (temporarily)

   // write the world size chunk
   fwrite ("[worldsize]", sizeof ("[worldsize]") - 1, 1, fp);
   fwrite (&map.v_worldmins, sizeof (vector), 1, fp);
   fwrite (&map.v_worldmaxs, sizeof (vector), 1, fp);

   // write the walkable faces chunk
   fwrite ("[walkfaces]", sizeof ("[walkfaces]") - 1, 1, fp);
   fwrite (&map.walkfaces_count, sizeof (int), 1, fp); // write the number of faces
   for (face_index = 0; face_index < map.walkfaces_count; face_index++)
   {
      fwrite (&map.walkfaces[face_index].corner_count, sizeof (int), 1, fp); // write # of corners for this face
      for (corner_index = 0; corner_index < map.walkfaces[face_index].corner_count; corner_index++)
      {
         fwrite (&map.walkfaces[face_index].v_corners[corner_index], sizeof (vector), 1, fp); // write the corner
         fwrite (&map.walkfaces[face_index].v_delimiters[corner_index], sizeof (vector), 1, fp); // write the face delimiter
      }
   }

   // write the topology chunk
   fwrite ("[topology]", sizeof ("[topology]") - 1, 1, fp);
   fwrite (&map.parallels_count, sizeof (int), 1, fp);
   fwrite (&map.meridians_count, sizeof (int), 1, fp);
   for (i = 0; i < map.parallels_count; i++)
   {
      for (j = 0; j < map.meridians_count; j++)
      {
         fwrite (&map.topology[i][j].faces_count, sizeof (int), 1, fp); // write # of faces for this zone
         for (face_index = 0; face_index < map.topology[i][j].faces_count; face_index++)
         {
            // translate the pointer address into an array relative index
            array_index = ((unsigned long) map.topology[i][j].faces[face_index] - (unsigned long) map.walkfaces) / sizeof (walkface_t);
            if ((array_index < 0) || (array_index >= map.walkfaces_count))
               TerminateOnError ("SaveWorldMap(): bad face array index %d (max %d) at [%d][%d], index %d/%d\n", array_index, map.walkfaces_count - 1, i, j, face_index, map.topology[i][j].faces_count);
            fwrite (&array_index, sizeof (int), 1, fp);
         }
      }

      ServerConsole_printf ("."); // print a trailing dot as a progress bar
   }

   size = ftell (fp); // get the file size, i.e our current position before we rewind

   // now we're ready to write the authentication lump
   fseek (fp, 0, SEEK_SET); // rewind the file
   fseek (fp, sizeof ("RACCMAP"), SEEK_CUR); // skip the RACCMAP tag
   fseek (fp, sizeof ("[filesize]") - 1, SEEK_CUR); // skip the filesize chunk tag
   fwrite (&bsp_file_size, sizeof (int), 1, fp); // and write the file size

   fclose (fp); // finished, close the file

   return (size); // and return the world data size
}
