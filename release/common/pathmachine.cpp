// RACC - AI development project for first-person shooter games
// (http://racc.bots-united.com/)
//
// Rational Autonomous Cybernetic Commandos AI
//
// pathmachine.cpp
//

#include "racc.h"


void BotInitPathMachine (player_t *pPlayer)
{
   // this function prepare a bot's pathfinding machine, initializing the lists to the number
   // of nodes the current map has, and cleansing the whole crap out. Local variables have been
   // declared static to speed the function up a bit.

   static pathmachine_t *pathmachine;

   pathmachine = &pPlayer->Bot.BotBrain.PathMachine; // first get a quick access to the bot's pathmachine

   // make sure the OPEN and PATH lists are empty
   if (pathmachine->open)
      free (pathmachine->open); // free the list to have it in a clean state
   if (pathmachine->path)
      free (pathmachine->path); // free the list to have it in a clean state

   // now initialize them so as they can hold pointers to the max number of nodes this map has
   pathmachine->open = (navnode_t **) malloc (map.walkfaces_count * sizeof (navnode_t *));
   pathmachine->path = (navlink_t **) malloc (map.walkfaces_count * sizeof (navlink_t *));

   // and clean the whole machine
   memset (pathmachine->open, 0, map.walkfaces_count * sizeof (navnode_t *));
   pathmachine->open_count = 0;
   memset (pathmachine->path, 0, map.walkfaces_count * sizeof (navlink_t *));
   pathmachine->path_count = 0;

   pathmachine->path_cost = 0;

   return; // now the lists can be safely considered as valid arrays of pointers
}


void BotRunPathMachine (player_t *pPlayer)
{
   // this function runs a time-sliced A* path machine belonging to a particular bot. Each bot
   // has its own pathmachine, allowing several pathfinding sessions to run in parallel.
   // paths are computed in the REVERSE order, because since a pathfinding process may extend on
   // several frames, the "start" of the path, which can be where the bot is, is likely to change,
   // because of the bot being still running, or moving elseway.
   // In order to compute a path, one must:
   // 1. wait for the pathmachine to finish a previous task or forcefully reset it
   // 2. place the GOAL node (the start node for the pathfinder) on the open list
   // These operations are done automatically upon each call of BotFindPathTo().
   // Then the path machine looks for a path, several cycles per frame depending on the value of
   // the cycles_per_frame variable, and if one path is available, it sets a flag to indicate
   // it has finished, and builds the path under the form of an array of nodes (in the right
   // order this time) in the final "path" field of the pathmachine.
   // Local variables have been declared static to speedup recurrent calls of this function.

   static char display_string[256];
   static int cycles_per_frame;
   static pathmachine_t *pathmachine;
   static walkface_t *walkface_at_feet;
   static navnode_t *pathmemory;
   static navnode_t *examined_node, *ancestor_node, *goal_node;
   register int cycle_count, node_index, link_index;
   static float ancestor_travel_cost;
   static float debug_update_time = 0;

   pathmachine = &pPlayer->Bot.BotBrain.PathMachine; // quick access to pathmachine
   pathmemory = pPlayer->Bot.BotBrain.PathMemory; // quick access to pathmemory

   // have we NOT been asked to compute a path yet ?
   if ((pathmachine->open[0] == NULL) || (pathmachine->open_count == 0))
      return; // just return when there's nothing to do

   // looks like a pathfinding session is in progress or has just started...
   pathmachine->busy = TRUE; // so mark the machine as busy

   // timeslice duration is inversely adaptative to frame duration, for example:
   // 100 fps => 100 cycles
   // 10 fps => 1000 cycles
   // 1 fps => 10000 cycles (roughly means: do not slice at all)
   // this is meant to preserve some sort of homogeneity in the bot's reaction time
   cycles_per_frame = 10 * server.msecval; // compute timeslice cycles quota
   cycle_count = 0; // reset the pathfinding machine's cycle count

   // does the bot want to find a path from its current location ?
   if (pathmachine->should_update_goal)
   {
      // find and get the navnode under this bot's feet (by linking a pointer to it). The node
      // at the bot's feet is the start node of the path from the bot's point of view, but it is
      // the GOAL node in the search routine since we look for the path in the reverse order.
      walkface_at_feet = pPlayer->pFaceAtFeet;

      // reliability check: is the walkface at feet not found ?
      if (walkface_at_feet == NULL)
      {
         if (pPlayer->is_watched && (DebugLevel.navigation > 1))
            ServerConsole_printf ("Bot %s can't do pathfinding (can't find walkface at feet)\n", pPlayer->connection_name);
         return; // if so, don't compute anything this frame (the bot doesn't know where it is)
      }

      // with this we can keep the goal node updated dynamically all along the search :)
      node_index = WalkfaceIndexOf (walkface_at_feet);
      goal_node = &pathmemory[node_index]; // node indices are the same as walkface indices, btw
   }
   else
      goal_node = pathmachine->goal_node; // else use the goal node the pathmachine was told

   // who said I would never use A* ? me?? err... okay, I admit. Stop nagging, you there.

   // while there are nodes on the open list, examine them...
   while (pathmachine->open_count > 0)
   {
      if (cycle_count == cycles_per_frame)
         return; // break the loop if we've searched for too long (we'll continue next frame)

      if (debug_update_time > server.time)
         return; // also break the loop when debug mode and not time to update yet

      // take the most interesting node among the list of those we haven't examined yet
      examined_node = PopFromOpenList (pathmachine);

      // if navigation debug level is VERY high, slow down the pathmachine
      if (pPlayer->is_watched && (DebugLevel.navigation > 2))
      {
         // and display its progress visually
         printf ("Examining node %d\n", WalkfaceIndexOf (examined_node->walkface));
         UTIL_DrawWalkface (examined_node->walkface, 25, 255, 255, 255);
         debug_update_time = server.time + 3.0; // next pass in 3 seconds
      }

      // is this node the goal node ?
      if (examined_node == goal_node)
      {
         pathmachine->path_count = 0; // we have found the path ! so build it...

         // while we've not built the entire path...
         while (examined_node->entrypoint != NULL)
         {
            // append this link to the path array of link pointers
            pathmachine->path[pathmachine->path_count] = examined_node->entrypoint;
            pathmachine->path_count++; // the path is now one node longer
            examined_node = examined_node->parent;
         }

         // remember the path cost, while we're at it...
         pathmachine->path_cost = goal_node->total_cost;

         // if navigation debug level is high, print out the path
         if (pPlayer->is_watched && (DebugLevel.navigation > 1))
         {
            printf ("PATH FOUND: %d links involved, cost: %.1f\n", pathmachine->path_count, pathmachine->path_cost);
            for (link_index = 0; link_index < pathmachine->path_count; link_index++)
               ServerConsole_printf ("Link at (%.0f, %.0f, %.0f): reachability %d\n",
                                     pathmachine->path[link_index]->v_origin.x,
                                     pathmachine->path[link_index]->v_origin.y,
                                     pathmachine->path[link_index]->v_origin.z,
                                     pathmachine->path[link_index]->reachability);
            UTIL_DrawPath (pathmachine);
         }

         // clean up the machine and notify the bot that we have finished computing the path
         memset (pathmachine->open, 0, map.walkfaces_count * sizeof (navnode_t *));
         pathmachine->open_count = 0;
         pathmachine->finished = TRUE;
         pathmachine->busy = FALSE;

         return; // and return
      }

      // this node is NOT the goal node, but since it might be on the path (we don't know yet)
      // we have to check for its ancestors in order to know where they lead to...
      for (link_index = 0; link_index < examined_node->links_count; link_index++)
      {
         // get an entrypoint to the node (by linking a pointer to it)
         ancestor_node = examined_node->links[link_index].node_from;

         // if navigation debug level is VERY high, display the pathmachine's progress visually
         if (pPlayer->is_watched && (DebugLevel.navigation > 2))
         {
            printf ("Examining entrypoint %d\n", link_index);
            UTIL_DrawWalkface (ancestor_node->walkface, 25, 255, 0, 0);
         }

         // compute its travel cost by adding this of its parent to its own
         ancestor_travel_cost = examined_node->travel_cost + BotEstimateTravelCost (pPlayer, examined_node->entrypoint, &examined_node->links[link_index]);

         // check whether this node is already in the open list AND is cheaper there
         if (ancestor_node->is_in_open_list && (ancestor_node->travel_cost <= ancestor_travel_cost))
            continue; // then skip this node

         // check whether this node is already in the closed list AND is cheaper there
         if (ancestor_node->is_in_closed_list && (ancestor_node->travel_cost <= ancestor_travel_cost))
            continue; // then skip this node

         ancestor_node->parent = examined_node; // remember this node's parent
         ancestor_node->entrypoint = &examined_node->links[link_index]; // remember its entry point
         ancestor_node->travel_cost = ancestor_travel_cost; // remember its travel cost
         ancestor_node->remaining_cost = EstimateTravelCostFromTo (ancestor_node, pathmachine->open[0]);
         ancestor_node->total_cost = ancestor_travel_cost + ancestor_node->remaining_cost;

         // was this node on the closed list again ?
         if (ancestor_node->is_in_closed_list)
            ancestor_node->is_in_closed_list = FALSE; // take it off the list

         // is this node NOT in the OPEN list yet ?
         if (!ancestor_node->is_in_open_list)
            PushToOpenList (pathmachine, ancestor_node); // push it there
      }

      examined_node->is_in_closed_list = TRUE; // done examining this node and its entrypoints
      cycle_count++; // the pathfinding machine has elapsed one cycle more
   }

   // if we get there, that's we have explored all the nodes ascending from the start point,
   // and that none of them has been able to reach the destination point. It just means that
   // no path exists, so clean up the machine and return.

   // do we want to debug the navigation ?
   if (pPlayer->is_watched && (DebugLevel.navigation > 0))
      printf ("PATH UNAVAILABLE\n"); // if so, inform us that no path could be found

   memset (pathmachine->open, 0, map.walkfaces_count * sizeof (navnode_t *));
   pathmachine->open_count = 0;
   memset (pathmachine->path, 0, map.walkfaces_count * sizeof (navlink_t *));
   pathmachine->path_count = 0;

   pathmachine->path_cost = 0;
   pathmachine->finished = TRUE;
   pathmachine->busy = FALSE;

   return; // pathmachine unable to find any path, so return
}


void BotShutdownPathMachine (player_t *pPlayer)
{
   // this function frees all the memory space that has been allocated for the specified bot's
   // pathmachine and resets its pointers to zero. Beware NOT to use the machine afterwards !!!
   // Local variables have been declared static to speed the function up a bit.

   static pathmachine_t *pathmachine;

   pathmachine = &pPlayer->Bot.BotBrain.PathMachine; // quick access to the bot's pathmachine

   // do we need to free this bot's pathmachine ?
   if (pathmachine->open)
      free (pathmachine->open); // free the node queue
   pathmachine->open = NULL;
   if (pathmachine->path)
      free (pathmachine->path); // free the path list
   pathmachine->path = NULL;

   return; // finished, everything is freed
}


bool BotFindPathTo (player_t *pPlayer, Vector v_goal, bool urgent)
{
   // the purpose of this function is to setup bot pBot's pathmachine so that it computes a
   // path to the destination described by the spatial vector v_goal. If additionally the urgent
   // flag is set, and the pathmachine is already computing another path, the function forcibly
   // resets the pathmachine and make it compute the new path immediately instead. First the bot
   // will try to determine if it knows a navnode that would enclose the destination location,
   // and if one is found, it is put on the pathmachine's open list to tell it to compute a path
   // to this location.

   // remember: the startface for the pathmachine is our GOAL face !!!

   register int index;
   static pathmachine_t *pathmachine;
   static walkface_t *startface;
   static navnode_t *pathmemory, *startnode;

   if (pPlayer->Bot.findpath_time > server.time)
      return (FALSE); // cancel if not time to

   pathmachine = &pPlayer->Bot.BotBrain.PathMachine; // quick access to pathmachine
   pathmemory = pPlayer->Bot.BotBrain.PathMemory; // quick access to pathmemory

   // if bot is already computing another path and there's no emergency, give up
   if (pathmachine->busy && !urgent)
   {
      pPlayer->Bot.findpath_time = server.time + 1.0; // try again in one second
      return (FALSE); // but not now
   }

   // try to find the walkface corresponding to the location we want to go to
   startface = NULL; // don't forget to reset it since it is static
   startface = WalkfaceAt (v_goal);

   // if we can't find that walkface, then the bot does not know about its destination
   if (startface == NULL)
   {
      pPlayer->Bot.findpath_time = server.time + 1.0; // try again in one second
      return (FALSE); // howd'ya want to go anywhere if you didn't knew that place existed ?
   }

   // clean up the machine before proceeding
   memset (pathmachine->open, 0, map.walkfaces_count * sizeof (navnode_t *));
   pathmachine->open_count = 0;
   pathmachine->finished = FALSE;

   // do not forget to empty all the lists !
   for (index = 0; index < map.walkfaces_count; index++)
   {
      pathmemory[index].parent = NULL;
      pathmemory[index].entrypoint = NULL;
      pathmemory[index].is_in_open_list = FALSE;
      pathmemory[index].is_in_closed_list = FALSE;
      pathmemory[index].travel_cost = 0;
      pathmemory[index].remaining_cost = 0;
      pathmemory[index].total_cost = 0;
   }

   // tell the pathmachine that it should update the goal node dynamically
   pathmachine->should_update_goal = TRUE;

   // find the destination node (its index is the same as the destination face index) and put
   // it on the open list of the pathmachine, setting the open list count to 1
   startnode = &pathmemory[WalkfaceIndexOf (startface)];
   startnode->parent = NULL;
   startnode->entrypoint = NULL;
   startnode->travel_cost = 0;
   startnode->remaining_cost = (v_goal - pPlayer->v_origin).Length ();
   startnode->total_cost = startnode->remaining_cost;
   PushToOpenList (pathmachine, startnode);

   pPlayer->Bot.findpath_time = server.time + RandomFloat (1.0, 3.0); // next try in a few secs

   return (TRUE); // and let the pathmachine do its job
}


bool BotFindPathFromTo (player_t *pPlayer, Vector v_start, Vector v_goal, bool urgent)
{
   // the purpose of this function is to setup bot pBot's pathmachine so that it computes a
   // path from and to the locations specified by v_start and v_goal. If additionally the urgent
   // flag is set, and the pathmachine is already computing another path, the function forcibly
   // resets the pathmachine and make it compute the new path immediately instead. First the bot
   // will try to determine if it knows a navnode that would enclose the destination location,
   // and if one is found, it is put on the pathmachine's open list to tell it to compute a path
   // to this location.

   register int index;
   static pathmachine_t *pathmachine;
   static walkface_t *startface, *goalface;
   static navnode_t *pathmemory, *startnode;

   if (pPlayer->Bot.findpath_time > server.time)
      return (FALSE); // cancel if not time to

   pathmachine = &pPlayer->Bot.BotBrain.PathMachine; // quick access to pathmachine
   pathmemory = pPlayer->Bot.BotBrain.PathMemory; // quick access to pathmemory

   // if bot is already computing another path and there's no emergency, give up
   if (pathmachine->busy && !urgent)
   {
      pPlayer->Bot.findpath_time = server.time + 1.0; // try again in one second
      return (FALSE); // but not now
   }

   // try to find the walkface corresponding to the location we want to go to
   startface = NULL; // don't forget to reset it since it is static
   startface = WalkfaceAt (v_goal);

   // if we can't find that walkface, then the bot does not know about its destination
   if (startface == NULL)
   {
      pPlayer->Bot.findpath_time = server.time + 1.0; // try again in one second
      return (FALSE); // howd'ya want to go anywhere if you didn't knew that place existed ?
   }

   // try to find the walkface corresponding to the location we want to start from
   goalface = NULL; // don't forget to reset it since it is static
   goalface = WalkfaceAt (v_start);

   // if we can't find that walkface, then the bot does not know where the path starts
   if (goalface == NULL)
   {
      pPlayer->Bot.findpath_time = server.time + 1.0; // try again in one second
      return (FALSE); // howd'ya want to go anywhere if you didn't knew that place existed ?
   }

   // clean up the machine before proceeding
   memset (pathmachine->open, 0, map.walkfaces_count * sizeof (navnode_t *));
   pathmachine->open_count = 0;
   pathmachine->finished = FALSE;

   // do not forget to empty all the lists !
   for (index = 0; index < map.walkfaces_count; index++)
   {
      pathmemory[index].parent = NULL;
      pathmemory[index].entrypoint = NULL;
      pathmemory[index].is_in_open_list = FALSE;
      pathmemory[index].is_in_closed_list = FALSE;
      pathmemory[index].travel_cost = 0;
      pathmemory[index].remaining_cost = 0;
      pathmemory[index].total_cost = 0;
   }

   // push the goal node (start node for us) onto the pathmachine
   pathmachine->goal_node = &pathmemory[WalkfaceIndexOf (goalface)];
   pathmachine->should_update_goal = FALSE; // this is NOT a goal to update dynamically

   // find the destination node (its index is the same as the destination face index) and put
   // it on the open list of the pathmachine, setting the open list count to 1
   startnode = &pathmemory[WalkfaceIndexOf (startface)];
   startnode->parent = NULL;
   startnode->entrypoint = NULL;
   startnode->travel_cost = 0;
   startnode->remaining_cost = (v_goal - pPlayer->v_origin).Length ();
   startnode->total_cost = startnode->remaining_cost;
   PushToOpenList (pathmachine, startnode);

   pPlayer->Bot.findpath_time = server.time + RandomFloat (1.0, 3.0); // next try in a few secs

   return (TRUE); // and let the pathmachine do its job
}


float EstimateTravelCostFromTo (navnode_t *node_from, navnode_t *node_to)
{
   // this function estimates the travel cost between two navigation links. It's a heuristic
   // used in the pathfinding process. Currently we assume the travel cost is roughly equal
   // to the straight distance between both nodes (more accurately between the center of the
   // walkfaces they represent).

   register int index;
   static walkface_t *walkface_from, *walkface_to;
   static Vector v_from, v_to;

   if ((node_from == NULL) || (node_to == NULL))
      return (0); // reliability check

   // get a quick access to the involved walkfaces
   walkface_from = node_from->walkface;
   walkface_to = node_to->walkface;

   // figure out where the center of the first walkface is
   v_from = g_vecZero;
   for (index = 0; index < walkface_from->corner_count; index++)
      v_from = v_from + walkface_from->v_corners[index];
   v_from = v_from / (float) walkface_from->corner_count;

   // figure out where the center of the second walkface is
   v_to = g_vecZero;
   for (index = 0; index < walkface_to->corner_count; index++)
      v_to = v_to + walkface_to->v_corners[index];
   v_to = v_to / (float) walkface_to->corner_count;

   // and return a base cost according to the distance
   return ((v_to - v_from).Length ());
}


float BotEstimateTravelCost (player_t *pPlayer, navlink_t *link_from, navlink_t *link_to)
{
   // this function returns a weighted measurement of the travel cost between two navnodes
   // under the estimation the specified personality makes of it (regarding its memories about
   // either of these nodes). The basis for the estimation is the raw distance between these
   // two nodes, weighted by the type of reachability between them. At least link_to must point
   // to a valid navlink that is present in personality's nav memory. If link_from is NULL or
   // unspecified, the returned cost will be zero.

   static float f_cost;

   if (link_to == NULL)
      return (0); // don't know where we want to go, can't tell how far it is !

   if (link_from == NULL)
      return (0); // don't know where we came from, can't tell how far it is !

   // compute a base cost according to the distance
   f_cost = (link_to->v_origin - link_from->v_origin).Length ();

   // now given the type of reachability it is, weighten this base cost according to an
   // overall like/dislike factor (this factor changes when the bot's happiness suddently
   // changes at these locations, f.ex. when the bot is hurt, when it kills someone or when
   // it is killed here)...
   if (link_to->reachability & REACHABILITY_LADDER)
      f_cost *= pPlayer->Bot.BotBrain.likelevel.ladder;
   if (link_to->reachability & REACHABILITY_FALLEDGE)
      f_cost *= pPlayer->Bot.BotBrain.likelevel.falledge;
   if (link_to->reachability & REACHABILITY_ELEVATOR)
      f_cost *= pPlayer->Bot.BotBrain.likelevel.elevator;
   if (link_to->reachability & REACHABILITY_PLATFORM)
      f_cost *= pPlayer->Bot.BotBrain.likelevel.platform;
   if (link_to->reachability & REACHABILITY_CONVEYOR)
      f_cost *= pPlayer->Bot.BotBrain.likelevel.conveyor;
   if (link_to->reachability & REACHABILITY_TRAIN)
      f_cost *= pPlayer->Bot.BotBrain.likelevel.train;
   if (link_to->reachability & REACHABILITY_LONGJUMP)
      f_cost *= pPlayer->Bot.BotBrain.likelevel.longjump;
   if (link_to->reachability & REACHABILITY_SWIM)
      f_cost *= pPlayer->Bot.BotBrain.likelevel.swim;
   if (link_to->reachability & REACHABILITY_TELEPORTER)
      f_cost *= pPlayer->Bot.BotBrain.likelevel.teleporter;
   if (link_to->reachability & REACHABILITY_JUMP)
      f_cost *= pPlayer->Bot.BotBrain.likelevel.jump;
   if (link_to->reachability & REACHABILITY_CROUCH)
      f_cost *= pPlayer->Bot.BotBrain.likelevel.crouch;
   if (link_to->reachability & REACHABILITY_UNKNOWN1)
      f_cost *= pPlayer->Bot.BotBrain.likelevel.unknown1;
   if (link_to->reachability & REACHABILITY_UNKNOWN2)
      f_cost *= pPlayer->Bot.BotBrain.likelevel.unknown2;
   if (link_to->reachability & REACHABILITY_UNKNOWN3)
      f_cost *= pPlayer->Bot.BotBrain.likelevel.unknown3;
   if (link_to->reachability & REACHABILITY_UNKNOWN4)
      f_cost *= pPlayer->Bot.BotBrain.likelevel.unknown4;
   if (link_to->reachability & REACHABILITY_UNKNOWN5)
      f_cost *= pPlayer->Bot.BotBrain.likelevel.unknown5;

   // *** TODO *** Make the bot question its individual memory about this link
   // i.e: this spot is dangerous, it's a weapon spot, a cover passage, etc.

   return (f_cost); // and return the weightened cost
}


void PushToOpenList (pathmachine_t *pathmachine, navnode_t *queue_element)
{
   // this function inserts an element in the OPEN priority queue of the specified pathmachine 
   // and rearrange the heap so as to always have the item which has the lower weight at the
   // root of the heap.

   register int index;

   if (pathmachine == NULL)
      return; // reliability check

   index = pathmachine->open_count; // first insert position at the trailing end of the heap

   // while the item is not at the right position in the heap so as to have it sorted...
   while ((index > 0) && (queue_element->total_cost < pathmachine->open[(index - 1) / 2]->total_cost))
   {
      pathmachine->open[index] = pathmachine->open[(index - 1) / 2]; // shuffle the branch down
      index = (index - 1) / 2; // proceed to the parent element
   }

   queue_element->is_in_open_list = TRUE; // flag this element as a member of the OPEN list
   queue_element->is_in_closed_list = FALSE; // this element cannot be on the CLOSED list now
   pathmachine->open[index] = queue_element; // attach the item at this remaining location
   pathmachine->open_count++; // there is now one element more in the heap

   return; // done, heap is sorted
}


navnode_t *PopFromOpenList (pathmachine_t *pathmachine)
{
   // this function takes the element with the lower weight from the OPEN priority queue of the
   // specified pathmachine and rearrange the heap so as to always have the item which has the
   // lower weight at the root of the heap.

   static navnode_t *return_element, *temp_element;
	register int i, j;

	if ((pathmachine == NULL) || (pathmachine->open_count == 0))
      return (NULL); // reliability check

	return_element = pathmachine->open[0]; // get the element with highest priority from the top
   return_element->is_in_open_list = FALSE; // flag this element as NOT a member of the OPEN list
   pathmachine->open_count--; // there is now one element less in the heap

	temp_element = pathmachine->open[pathmachine->open_count]; // take the last inserted element
   i = 0; // start searching for its new position at root level

   // while we've not found the place to insert this temporary element so as to have the heap
   // sorted again, shuffle the tree down
	while (i < (pathmachine->open_count + 1) / 2)
   {
		j = (2 * i) + 1; // consider i's first child and name it j

		if ((j < pathmachine->open_count - 1)
          && (pathmachine->open[j]->total_cost > pathmachine->open[j + 1]->total_cost))
			j++; // now take the lowest weighted of i's two siblings (either j, or its brother)

      if (pathmachine->open[j]->total_cost >= temp_element->total_cost)
			break; // stop here when this child is more weighted than the element we want to shuffle

      pathmachine->open[i] = pathmachine->open[j]; // else shuffle this child one rank up
		i = j; // reach the position of the lowest weighted child for next pass
	}

	pathmachine->open[i] = temp_element; // now here is a good place to dump that element to
	return (return_element); // done, heap is sorted again.
}
