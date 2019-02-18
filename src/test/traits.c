/*****************************************************************************
** TRAITS.C                                                                 **
** Code by Rayal (Brandon Morrison)                                         **
** Created Thursday, February 18, 2019                                      **
**                                                                          **
*****************************************************************************/

#if defined(Macintosh)
#include <types.h>
#include <time.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <malloc.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

#include "twilight.h"
#include "tables.h"
#include "lookup.h"
#include "recycle.h"
#include "interp.h"
#include "traits.h"

/* LOCAL FUNCTIONS */
void assign_traits(void);
int compare_traits(const void *x, const void *y);
void sort_traits(void);
void traitto(int featnum, char *name, int in_game, int can_stack, char *prerequisites, char *description);

/* Global Variables and Structures */
struct trait_info trait_list[NUM_TRAIT_DEFINED+1];
int trait_sort_info[MAX_TRAITS + 1];

/* Begin File stuff */
int compare_traits(const void *x, const void *y)
{
  int   a = *(const int *)x,
        b = *(const int *)y;
  
  return strcmp(trait_list[a].name, trait_list[b].name);
}

/* sort feats called at boot up */
void sort_traits(void)
{
    int a = 0;

    /* initialize array, avoiding reserved. */
    for (a = 1; a <= NUM_TRAIT_DEFINED; a++)
        {
            trait_sort_info[a] = a;
        }

    qsort(&trait_sort_info[1], NUM_TRAIT_DEFINED, sizeof(int), compare_traits);
}

void traitto(int featnum, char *name, int in_game, int can_stack, char *prerequisites, char *description)
{
  trait_list[featnum].name = name;
  trait_list[featnum].in_game = in_game;
  trait_list[featnum].can_stack = can_stack;
  trait_list[featnum].prerequisites = prerequisites;
  trait_list[featnum].description = description;
}

void traits(void)
{
  /* Nothing to do right now */
}

void assign_traits(void)
{

	int i = 0;

	// Initialize the list of traits.

	for (i = 0; i <= NUM_TRAIT_DEFINED; i++) 
	{
		trait_list[i].name = "Unused Trait";
		trait_list[i].in_game = FALSE;
		trait_list[i].can_stack = FALSE;
		trait_list[i].prerequisites = "ask staff";
		trait_list[i].description = "ask staff";
	}

/* feat-number | in-game name | in-game? | stackable? | prerequisite | description |*/

	traitto(TRAIT_BERSERKER, "Berserker", TRUE, FALSE, "prerequisite", "Description");
}
