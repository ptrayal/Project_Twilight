#ifndef TRAITS_H
#define TRAITS_H

#define MAX_TRAITS			100
#define FEAT_LAST_TRAIT		99
#define NUM_TRAIT_DEFINED	100

/* Functions defined in traits.c */
extern struct trait_info trait_list[];

struct trait_info 
{
  char *name;      // The name of the trait 
  bool in_game;    // TRUE or FALSE, is the trait in the game yet?
  bool can_stack;  // TRUE or FALSE, can the feat be learned more than once?
  char *prerequisites;
  char *description;
};

#define TRAIT_UNDEFINED 0
#define TRAIT_BERSERKER 1

#endif