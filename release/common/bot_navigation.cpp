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
// bot_navigation.cpp
//

#include "racc.h"

extern char mod_name[256];
extern char map_name[256];
extern entity_t *pListenserverEntity;
extern debug_level_t DebugLevel;
extern player_t players[MAX_CLIENTS_SUPPORTED_BY_ENGINE];
extern map_t map;


void PrepareNavBrainForPersonality (bot_personality_t *personality)
{
   // this function sets up the navigation nodes in the bot personality's memory. Either by
   // loading them from disk, or by inferring brand new ones based on the map's contents. They
   // will anyhow be saved back to disk when the bot will leave the server.

   FILE *fp;
   char filename[256];
   int recorded_walkfaces_count, node_index, dummy = 0;

   if (personality == NULL)
      return; // reliability check

   // allocate enough memory for the navigation nodes
   personality->PathMemory = (navnode_t *) malloc (map.walkfaces_count * sizeof (navnode_t));
   if (personality->PathMemory == NULL)
      TerminateOnError ("PrepareNavBrainForPersonality(): unable to allocate enough memory to infer a new nav brain to %s\n", personality->name); // bomb out on error

   // build the navigation nodes file name
   sprintf (filename, "racc/profiles/%s/%s/%s/racc-nav.brn", personality->name, mod_name, map_name);

   // try to open the file
   fp = fopen (filename, "rb");
   if (fp != NULL)
   {
      // file has been found, check its validity (recorded for the same number of walkfaces)
      fseek (fp, 0, SEEK_SET); // seek at start of file
      fseek (fp, sizeof ("RACCNOD"), SEEK_CUR); // skip the RACCNOD tag
      fread (&recorded_walkfaces_count, sizeof (int), 1, fp); // get the recorded # of walkfaces
      fclose (fp); // close the file (we just wanted the number of walkfaces)

      // is the recorded number of walkfaces the same as the map's ?
      if (recorded_walkfaces_count == map.walkfaces_count)
         return; // file is definitely okay, return
   }

   // either navnodes file is inexistent, or it is bad ; start with an empty one

   // there is a problem with the brain, infer a brand new one
   ServerConsole_printf ("RACC: %s's nav brain damaged!\n", personality->name);

   if (DeveloperMode () >= DEVELOPER_VERBOSE)
      ServerConsole_printf ("RACC: inferring a new nav brain to %s\n", personality->name);

   // open the file
   fp = fopen (filename, "wb");
   if (fp == NULL)
      TerminateOnError ("PrepareNavBrainForPersonality(): %s's nav brain refuses surgery!\n", personality->name); // bomb out on error

   // file has been found, check its validity (recorded for the same number of walkfaces)
   fseek (fp, 0, SEEK_SET); // seek at start of file
   fwrite ("RACCNOD", sizeof ("RACCNOD"), 1, fp); // write the RACCNOD tag
   fwrite (&map.walkfaces_count, sizeof (int), 1, fp); // write the # of walkfaces on map

   // for each navigation node...
   for (node_index = 0; node_index < map.walkfaces_count; node_index++)
      fwrite (&dummy, sizeof (int), 1, fp); // write the number of links this node has (0 so far)
   fclose (fp); // everything is saved, close the file

   return; // ok, now it is guarantee that this personality has an associated pathfinding ability
}


bool LoadNavBrainForPersonality (bot_personality_t *personality)
{
   // this function sets up the navigation nodes in the bot personality's memory. Either by
   // loading them from disk, or by inferring brand new ones based on the map's contents. They
   // will anyhow be saved back to disk when the bot will leave the server.

   FILE *fp;
   char filename[256];
   int recorded_walkfaces_count, node_index, link_index, array_index;

   if (personality == NULL)
      return (TRUE); // reliability check, return an error if personality is invalid

   // allocate enough memory for the navigation nodes
   personality->PathMemory = (navnode_t *) malloc (map.walkfaces_count * sizeof (navnode_t));
   if (personality->PathMemory == NULL)
      TerminateOnError ("LoadNavBrainForPersonality(): unable to allocate enough memory to infer a new nav brain to %s\n", personality->name); // bomb out on error

   // build the navigation nodes file name
   sprintf (filename, "racc/profiles/%s/%s/%s/racc-nav.brn", personality->name, mod_name, map_name);

   // try to open the file
   fp = fopen (filename, "rb");
   if (fp == NULL)
   {
      LogToFile ("LoadNavBrainForPersonality(): unable to find %s's nav brain on disk!\n", personality->name);
      return (TRUE); // bad brain, return TRUE (error)
   }

   // file has been found, check its validity (recorded for the same number of walkfaces)
   fseek (fp, 0, SEEK_SET); // seek at start of file
   fseek (fp, sizeof ("RACCNOD"), SEEK_CUR); // skip the RACCNOD tag
   fread (&recorded_walkfaces_count, sizeof (int), 1, fp); // get the recorded # of walkfaces

   // is the recorded number of walkfaces NOT the same as the map's ?
   if (recorded_walkfaces_count != map.walkfaces_count)
   {
      LogToFile ("LoadNavBrainForPersonality(): incoherent data in %s's nav brain!\n", personality->name);
      fclose (fp); // close file
      return (TRUE); // bad brain, error, return TRUE
   }

   // file is okay, cycle through all navigation nodes...
   for (node_index = 0; node_index < map.walkfaces_count; node_index++)
   {
      // link a pointer to the walkface it is for
      personality->PathMemory[node_index].walkface = &map.walkfaces[node_index];

      // read the number of links this node has
      fread (&personality->PathMemory[node_index].links_count, sizeof (int), 1, fp);

      // whether we need to mallocate or reallocate space for the links array, do it
      if (personality->PathMemory[node_index].links_count == 0)
         personality->PathMemory[node_index].links = (navlink_t *) malloc (sizeof (navlink_t));
      else
         personality->PathMemory[node_index].links = (navlink_t *) realloc (personality->PathMemory[node_index].links, personality->PathMemory[node_index].links_count * sizeof (navlink_t));

      if (personality->PathMemory[node_index].links == NULL)
         TerminateOnError ("LoadNavBrainForPersonality(): unable to allocate enough memory to infer pathfinding ability to %s\n", personality->name); // bomb out on error

      // init the first link pointer to some failsafe value
      personality->PathMemory[node_index].links[0].walkface = &map.walkfaces[0];

      // translate each face array index into a pointer
      for (link_index = 0; link_index < personality->PathMemory[node_index].links_count; link_index++)
      {
         fread (&array_index, sizeof (int), 1, fp); // get the index in the walkface array it points to

         // test this index against overflow
         if ((array_index < 0) || (array_index >= map.walkfaces_count))
            TerminateOnError ("LoadNavBrainForPersonality(): bad face array index %d (max %d), index %d/%d\n", array_index, map.walkfaces_count - 1, link_index, personality->PathMemory[node_index].links_count);

         personality->PathMemory[node_index].links[link_index].walkface = (walkface_t *) ((unsigned long) map.walkfaces + array_index * sizeof (walkface_t));

         // test this pointer against access violation (pointers are plain evil)
         if ((personality->PathMemory[node_index].links[link_index].walkface < &map.walkfaces[0]) || (personality->PathMemory[node_index].links[link_index].walkface > &map.walkfaces[map.walkfaces_count - 1]))
            TerminateOnError ("LoadNavBrainForPersonality(): bad face pointer %d (range %d - %d), index %d/%d\n", personality->PathMemory[node_index].links[link_index].walkface, &map.walkfaces[0], &map.walkfaces[map.walkfaces_count - 1], link_index, personality->PathMemory[node_index].links_count);

         // read the reachability type for this link (normal, ladder, elevator...)
         fread (&personality->PathMemory[node_index].links[link_index].reachability, sizeof (char), 1, fp);
      }
   }

   fclose (fp); // everything is loaded, close the file
   return (FALSE); // no error, return FALSE
}


void SaveNavBrainForPersonality (bot_personality_t *personality)
{
   // this function saves the navigation nodes in the bot personality's memory in a file to disk.

   FILE *fp;
   char filename[256];
   int node_index, link_index, array_index;

   if (personality == NULL)
      return; // reliability check

   // build the navigation nodes file name
   sprintf (filename, "racc/profiles/%s/%s/%s/racc-nav.brn", personality->name, mod_name, map_name);

   // open the file
   fp = fopen (filename, "wb");
   if (fp == NULL)
   {
      ServerConsole_printf ("RACC: Unable to save %s's nav brain to %s\n", personality->name, filename); // bomb out on error
      return;
   }

   // file has been successfully open, dump the nav brain to it
   fseek (fp, 0, SEEK_SET); // seek at start of file
   fwrite ("RACCNOD", sizeof ("RACCNOD"), 1, fp); // write the RACCNOD tag
   fwrite (&map.walkfaces_count, sizeof (int), 1, fp); // write the # of walkfaces on map

   // for each navigation node...
   for (node_index = 0; node_index < map.walkfaces_count; node_index++)
   {
      // write the number of links this node has
      fwrite (&personality->PathMemory[node_index].links_count, sizeof (int), 1, fp);

      // translate each pointer into a face array index
      for (link_index = 0; link_index < personality->PathMemory[node_index].links_count; link_index++)
      {
         // translate the pointer address into an array relative index
         array_index = ((unsigned long) personality->PathMemory[node_index].links[link_index].walkface - (unsigned long) map.walkfaces) / sizeof (walkface_t);
         if ((array_index < 0) || (array_index >= map.walkfaces_count))
            TerminateOnError ("SaveNavBrainForPersonality(): bad face array index %d (max %d), index %d/%d\n", array_index, map.walkfaces_count - 1, link_index, personality->PathMemory[node_index].links_count);
         fwrite (&array_index, sizeof (int), 1, fp);

         // write the reachability type for this link (normal, ladder, elevator...)
         fwrite (&personality->PathMemory[node_index].links[link_index].reachability, sizeof (char), 1, fp);
      }
   }

   fclose (fp); // everything is saved, close the file
   return; // and return
}
