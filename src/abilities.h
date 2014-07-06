struct skill_list_entry {

char * name;
int gsn;
sh_int race;

};

struct abilities {

sh_int sn;
sh_int value;

};

#define RACE_NONE	 0
#define RACE_HUMAN	 1
#define RACE_WEREWOLF	 2
#define RACE_VAMPIRE	 3
#define RACE_CHANGELING	 4
#define RACE_MAGE	 5
#define RACE_HUNTER	 6
#define RACE_WRAITH	 7
#define RACE_RISEN	 8
#define RACE_DEMON	 9
#define RACE_MUMMY	 10
#define MAX_PC_RACE      11
