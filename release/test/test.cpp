#include <stdio.h>
#include <malloc.h>


// navigation node structure definition
struct navnode_t
{
   // dynamic data used by the bot's pathmachine
   struct navnode_t *parent; // pointer to this element's parent in the queue during path search
   bool is_in_open_list; // set to TRUE if this node is in the pathmachine's OPEN list
   bool is_in_closed_list; // set to TRUE if this node is in the pathmachine's CLOSED list
   float total_cost; // weight of this element in the priority queue (sum of the 2 above)
};


// pathfinding machine
typedef struct
{
   navnode_t **open; // pathfinding machine's OPEN list (priority queue of nodes to search)
   int open_count; // number of elements in the OPEN list (fixed-size heap of the queue)
} pathmachine_t;





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

   queue_element->is_in_open_list = true; // flag this element as a member of the OPEN list
   queue_element->is_in_closed_list = false; // this element cannot be on the CLOSED list now
   pathmachine->open[index] = queue_element; // attach the item at this remaining location
   pathmachine->open_count++; // there is now one element more in the heap

   return; // done, heap is sorted
}


navnode_t *PopFromOpenList (pathmachine_t *pathmachine)
{
   // this function takes the element with the lower weight from the OPEN priority queue of the
   // specified pathmachine and rearrange the heap so as to always have the item which has the
   // lower weight at the root of the heap.

   navnode_t *return_element;
	navnode_t *temp_element;
	register int i = 0, j;

	if ((pathmachine == NULL) || (pathmachine->open_count == 0))
      return (NULL); // reliability check

	return_element = pathmachine->open[0]; // get the element with highest priority from the top
   return_element->is_in_open_list = false; // flag this element as NOT a member of the OPEN list

	temp_element = pathmachine->open[pathmachine->open_count - 1]; // take the last inserted element
   pathmachine->open_count--; // there is now one element less in the heap

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





void main (int argc, char **argv)
{
   FILE *fp;
   char line_buffer[256];

   fp = fopen ("D:/jeux/Half-Life/racc/knowledge/HAL training sample - Iliad.txt", "r");
   if (fp == NULL)
   {
      printf ("error opening file\n");
      return;
   }

   while (!feof (fp))
   {
      fgets (line_buffer, 256, fp);
      printf (line_buffer);
   }

   fclose (fp);

   return;
}
