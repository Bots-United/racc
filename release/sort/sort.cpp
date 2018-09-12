// sort.cpp
//
// Executable to sort alphabetically RACC HAL keyword files, output to stdout.
// Usage: sort input_file
//
// Tree code courtesy of Paul 'Cheesemonster' Murphy. w00t w00t!


#include <stdio.h>
#include <string.h>
#include <malloc.h>


typedef struct treenode_s
{
   struct treenode_s *left;
   struct treenode_s *right;
   char *value;
} treenode_t;


typedef struct
{
   treenode_t *root;
} tree_t;


typedef struct
{
   char value[64];
} entry_t;


treenode_t *CreateNode (char *value)
{
   // create a node that will hold the value passed by value, mallocating space for it on the
   // fly. Using malloc() the memory space used by the new node isn't freed when the function
   // exits, thus leaving it for further processing.

   treenode_t *new_node = (treenode_t *) malloc (sizeof (treenode_t));
   if (new_node == NULL)
      return (NULL); // error: couldn't allocate memory

   new_node->left = NULL;
   new_node->right = NULL;
   new_node->value = value;

   return (new_node);
}


treenode_t *AddNode (treenode_t *root, treenode_t *new_node)
{
   // this function attachs a new node in the tree whose root node is pointed to by root, at the
   // right position to have the tree still sorted once the new node will have been attached.

   if (new_node == NULL)
      return (root); // reliability check, malloc() may have failed in CreateNode earlier on

   if (root == NULL)
      return (new_node); // no root yet, new node is now root

   // else the tree already exists, we need to figure out the right place to attach new node to
   if (strcmp (new_node->value, root->value) <= 0)
   {
      // add to left if less or equal
      if (root->left == NULL)
         root->left = new_node; // no descendant on the left, we can add new node here
      else
         AddNode (root->left, new_node); // else we need to keep searching on the left side
   }
   else
   {
      // bigger, so add to right
      if (root->right == NULL)
         root->right = new_node; // no descendant on the right, we can add new node here
      else
         AddNode (root->right, new_node); // else we need to keep searching on the left side
   }

   return (root);
}


int TreeToArray (treenode_t *root, char **array, int array_index)
{
   // this function converts the tree data into a flat array (array_index will hold the position
   // of the array being written during the in-order traversal).

   // if no subbranch here...
   if (root == NULL)
      return (array_index); // then return, nothing has been written

   // now, we are sure this subbranch is valid

   // recursively dump the left branches and their descendants
   array_index = TreeToArray (root->left, array, array_index);

   // now dump the top of the tree (root)
   array[array_index] = root->value;
   array_index++;

   // and then recursively dump the right branches and their descendants
   array_index = TreeToArray (root->right, array, array_index);

   return (array_index); // finally, return the position in the array we have written up to
}


void DestroyTree (treenode_t *root)
{
   // this function recursively frees the memory space used by the tree and each of its branches.

   // is this node okay to be freed ?
   if (root)
   {
      DestroyTree (root->left); // in case it has descendants on the left, free them as well
      DestroyTree (root->right); // in case it has descendants on the right, free them as well
      free (root); // when each of its descendants are freed, one can free the node itself
   }

   return; // finished, everything is freed
}


int main (int argc, char **argv)
{
   tree_t tree;
   treenode_t *new_node;
   FILE *fp;
   char line_buffer[256];
   entry_t *in_array; // unsorted array of strings
   char **out_array; // contains sorted pointers to strings in in_array
   int num_entries; // how many strings are being used.
   int i;

   // check for argument
   if ((argv[1] == NULL) || (*argv[1] == 0))
   {
      printf ("Usage: %s filename [ > out_file ]\n", argv[0]);
      printf ("where filename is the name of the keywords file to sort\n");
      return (1);
   }

   // open file read/write
   fp = fopen (argv[1], "r");
   if (fp == NULL)
   {
      printf ("Error: unable to open file \"%s\"\n", argv[1]);
      printf ("Make sure the file exists and that the path you specified is correct\n");
      return (1);
   }

   fseek (fp, 0, SEEK_SET); // seek at start of file
   in_array = (entry_t *) malloc (1 * sizeof (entry_t)); // start allocating for 1 entry
   num_entries = 0;

   // while there are lines to read...
   while (fgets (line_buffer, 256, fp) != NULL)
   {
      if ((line_buffer[0] == '#') || (line_buffer[0] == '\n'))
         continue; // ignore line if commented or void

      // reallocate input array for new entry
      in_array = (entry_t *) realloc (in_array, (num_entries + 1) * sizeof (entry_t));

      // put the entry in the input array
      strcpy (in_array[num_entries].value, line_buffer);
      num_entries++; // there is one entry more in the list
   }

   out_array = (char **) malloc (num_entries * sizeof (char *));

   fclose (fp); // close the file

   tree.root = NULL; // initialise tree

   // build the tree by inserting each entry at the right position to have it sorted
   for (i = 0; i < num_entries; i++)
   {
      new_node = CreateNode (in_array[i].value);
      tree.root = AddNode (tree.root, new_node);
   }

   // now flatten it to an array
   TreeToArray (tree.root, out_array, 0);

   // and dump it to stdout
   for (i = 0; i < num_entries; i++)
      printf ("%s", out_array[i]);

   // we can now free the memory
   DestroyTree (tree.root);

   return (0);
} 
