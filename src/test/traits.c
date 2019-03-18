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
#include "grid.h"

/* LOCAL FUNCTIONS */
void assign_traits(void);
int compare_traits(const void *x, const void *y);
void sort_traits(void);
int has_feat(struct char_data *ch, int traitnum);
void traitto(int traitnum, char *name, int in_game, int can_stack, char *prerequisites, char *description);

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

/* checks if the char has the feat either saved to file or in the process
 of acquiring it in study */
int has_trait(struct char_data *ch, int traitnum) 
{

  if (ch->desc ) 
  {
    return (HAS_FEAT(ch, traitnum));
  }

  return HAS_FEAT(ch, traitnum);
}


void traitto(int traitnum, char *name, int in_game, int can_stack, char *prerequisites, char *description)
{
  trait_list[traitnum].name = name;
  trait_list[traitnum].in_game = in_game;
  trait_list[traitnum].can_stack = can_stack;
  trait_list[traitnum].prerequisites = prerequisites;
  trait_list[traitnum].description = description;
}

void free_traits(void)
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

// The follwing function is used to check if the character satisfies the various prerequisite(s) (if any)
// of a feat in order to learn it.
int feat_is_available(struct char_data *ch, int traitnum, int iarg, char *sarg)
{
  if (traitnum > NUM_TRAIT_DEFINED)
  {
    return FALSE;
  }

  if (has_trait(ch, traitnum) && !trait_list[traitnum].can_stack)
  {
    return FALSE;
  }

  switch (traitnum) 
  {
    default:
    return TRUE;

  }
}


void list_traits_available(struct char_data *ch, char *arg) 
{
  char buf[MAX_STRING_LENGTH]={'\0'}, buf2[MAX_STRING_LENGTH]={'\0'};
  int sortpos = 0;
  int mode = 0;
  int none_shown = TRUE;

  GRID_DATA *grid;
  GRID_ROW *row;
  // GRID_CELL *cell;
  
  if (*arg && is_abbrev(arg, "descriptions")) 
  {
    mode = 1;
  }
  else if (*arg && is_abbrev(arg, "requisites")) 
  {
    mode = 2;
  }

    // NEW GRID LAYOUT FOR MERITS OR FLAWS.
    grid = create_grid(75);
    row = create_row(grid);
    row_append_cell(row, 75, "Merits or Flaws Available");
    row = create_row(grid);

    // row_append_cell(row, 75, "You can learn %d feat%s and %d class feat%s right now.", GET_FEAT_POINTS(ch), (GET_FEAT_POINTS(ch) == 1 ? "" : "s"), GET_CLASS_FEATS(ch, GET_CLASS(ch)), 
    // (GET_CLASS_FEATS(ch, GET_CLASS(ch)) == 1 ? "" : "s"));

    row = create_row(grid);
    row_append_cell(row, 35, "Feats");
    if (mode == 2)
    {
      row_append_cell(row, 40, "Prerequisites");
    }
    else
    {
      row_append_cell(row, 40, "Benefits");
    }

  // LIST OF AVAILABLE TRAITS
  for (sortpos = 1; sortpos <= NUM_TRAIT_DEFINED; sortpos++) 
  {
    int i = trait_sort_info[sortpos];

    if (strlen(buf2) >= MAX_STRING_LENGTH - 32) 
    {
      strcat(buf2, "**OVERFLOW**\r\n"); 
      break;   
    }

    if (feat_is_available(ch, i, 0, NULL) && trait_list[i].in_game) 
    {
        row = create_row(grid);
        row_append_cell(row, 35, "@W%s@n", trait_list[i].name);
        if (mode == 2) 
        {
          row_append_cell(row, 40, "%s", trait_list[i].prerequisites);
        } 
        else 
        {
          row_append_cell(row, 40, "%s", trait_list[i].description);
        } 
    }
  }

  if (none_shown)
  {
    row = create_row(grid);
    row_append_cell(row, 75, "There are no merits or flaws available for you to learn at this point.");
  }
  
  row = create_row(grid);
  row_append_cell(row, 75, "@WSyntax:\nfeats <known|available> <description|requisites>\n(both arguments optional)@n");

  grid_to_char(grid, ch, TRUE);

  // END NEW GRID LAYOUT
 
  strcpy(buf2, buf);
}
