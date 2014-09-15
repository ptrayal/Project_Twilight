/* Table structs */
struct flag_type
{
    char *name;
    int bit;
    int settable;
};

struct clan_type
{
    char 	*name;				/* Long Name */
    char 	*who_name;			/* Who Name*/
    bool	independent;		/* true for loners */
    int		available;			/* What Race can pick it? */
    long	totem;				/* Totem # */
    int		powers[3];			/* Disciplines and/or powers */
    char 	*alt_race;			/* Alternate forms? */
    char 	*crinos_desc;		/* Crinos Form? */
};

struct virtue_type
{
    char *name;
};

struct position_type
{
    char *name;
    char *short_name;
};

struct exit_table_type
{
    char *name;
    int   dir;
};

struct sex_type
{
    char *name;
};

struct size_type
{
    char *name;
    int  obj_weight;
    int  mob_weight;
};

struct health_type
{
    char       *display_name;
    sh_int	dice;
};

struct attrib_type
{
    char *name;
};

struct disc_type
{
    char *vname;
    char *wname;
    char *fname;
    char *hname;
    int  index;
};

struct move_type
{
    bool endmove;
    at_part_type part;
    int  move_type;
};

struct combo_type
{
    int moves[4];
    char *to_char;
    char *to_room;
    char *to_vict;
    char *botch_to_char;
    char *botch_to_room;
    char *botch_to_vict;
    int diff;
    int damage;
    bool footstrike;
};

struct material
{
    char *name;
    char *colour;
    int is_liquid;
    int is_edible;
    int is_explosive;
    int is_flammable;
    int is_metal;
    int is_poison;
    int is_fabric;
    int is_acidproof;
    int  drunk;
    int  high;
    int  trip;
    int  thirst;
    int  full;
    int  hunger;
    int  value;
    int  weight;
    int  agg;
    int  aggsoak;
    int  normsoak;
};

struct job_cmd_type
{
    char	*name;
    int		delay;
    int		pos_pts;
    JOB_FUN	*func;
};

struct office_type
{
    char letter;
    char *name;
    const struct job_cmd_type *cmd_table;
    int  fame;
};

struct job_type
{
    char *name;
    int  available;
    const struct job_cmd_type *cmd_table;
    int stat;
    int abil;
    int acts_per_xp;
};

struct auspice_type
{
    char *name;
    int  power;
    int  rage;
};

struct breed_type
{
    char *name;
    int  power;
    int  gnosis;
};

struct colcode_type
{
    char letter;
    char *name;
    char *tag;
};

struct sire_type
{
    int gen;		/* Character generation minus 1 */
    char *clan;		/* Character Clan*/
    char *name;		/* Sire Name */
    char *sex;		/* Sire Sex */
    char *blurb;	/* Blurb about the sire */
};

struct influence_cmd_type
{
    char	*name;
    int		type;
    int		level;
    int		delay;
    INFL_FUN	*func;
};

struct news_cmd_type
{
    char	*name;
    int		level;
    INFL_FUN	*func;
};

struct smarket_cmd_type
{
    char	*name;
    int		level;
    INFL_FUN	*func;
};

struct spirit_power_type
{
    char	*name;
    long	flag;
    int		totem_group;
    SPIRIT_FUN	*func;
};

struct totem_power_type
{
    char	*name;
    long	flag;
    char	*gift[5];
};

struct gift_type
{
    char	*name;
    char	*desc;
    long	flag;
    int		level;
};

struct gift_spirit_teacher
{
    int		level;
    long	flag;
    char	*spirit[4];
};

struct group_gift_type
{
    char	*name;
    long	flag[5];
};

struct qt_type
{
    int         time_offset;
    QUEST_FUN * q_fun;
};

struct colour_type
{
    char *code;
    char *coltag;
    bool iscolour;
};

struct home_cmd_type
{
    char	*name;
    int		level;
    int		delay;
    BLD_FUN	*func;
};

struct home_price_type
{
    char	*name;
    char	*type;
    long	cost;
};

struct org_cmd_type
{
    char	*name;
    DO_FUN	*func;
};

struct trait_struct
{
    char *name;
    int bit;
    int cost;
    int settable;
};

struct ritemove_type
{
    char *name;
    char *to_char;
    char *to_room;
    int beats;
};

struct ritual_type
{
    char *name;
    char *races;
    int disc_test;
    int level;
    int actions[MAX_RITE_STEPS];
    int beats;
};

struct bit_type
{
       const   struct  flag_type *     table;
       char *                          help;
};

/* Functions */
extern char *flag_string( const struct flag_type *flag_table, int bits );

/* game tables */
extern	const	struct	clan_type	clan_table[MAX_CLAN];
extern	const	struct	position_type	position_table[];
extern	const	struct	sex_type	sex_table[];
extern	const	struct	size_type	size_table[];
extern  const   struct  health_type     health_table[];
extern  const   struct  virtue_type     virtue_table[MAX_VIRTUES];
extern	const	struct	disc_type	disc_table[];
extern  const	struct  combo_type	combo_table[];
extern  const	struct	move_type	combat_move_table[];
extern  const	struct	material	material_table[];
extern  const	int	race_predisp_table[22][22];
extern  const	int	clan_predisp_table[22][22];
extern	const	struct	office_type	office_table[];
extern	const	struct	job_type	job_table[];
extern	const	struct	job_type	game_table[];
extern	const	struct	breed_type	breed_table[];
extern	const	struct	auspice_type	auspice_table[];
extern  const   struct  colcode_type	colcode_table[];
extern  const   struct  sire_type	sire_table[];
extern  const   struct  influence_cmd_type	influence_cmd_table[];
extern  const   struct  influence_cmd_type	bg_cmd_table[];
extern	const	struct	news_cmd_type	news_cmd_table[];
extern	const	struct	smarket_cmd_type smarket_cmd_table[];
extern	const	struct	spirit_power_type spirit_power_table[];
extern	const	struct	totem_power_type totem_table[];
extern	const	struct	gift_type	gift_table[];
extern	const	struct	group_gift_type	group_gift_table[];
extern	const	struct	gift_type	gift_table[];
extern	const	struct	gift_spirit_teacher spirit_teacher_table[];
extern	const	struct	qt_type		quest_table[];
extern	const	struct	colour_type	colour_table[];
extern  const   struct  home_cmd_type	home_cmd_table[];
extern  const   struct  home_price_type	home_price_table[];
extern  const   struct  org_cmd_type	org_cmd_table[];
extern	const	struct	trait_struct	merit_table[];
extern	const	struct	trait_struct	flaw_table[];
extern	const	struct	trait_struct	derangement_table[];
extern	const	struct	ritual_type	ritual_table[];
extern	const	struct	ritemove_type	rite_actions[];

/* flag tables */
extern	const	struct	flag_type	act_flags[];
extern	const	struct	flag_type	act2_flags[];
extern	const	struct	flag_type	plr_flags[];
extern	const	struct	flag_type	affect_flags[];
extern	const	struct	flag_type	affect_flags2[];
extern	const	struct	flag_type	off_flags[];
extern	const	struct	flag_type	imm_flags[];
extern	const	struct	flag_type	form_flags[];
extern	const	struct	flag_type	part_flags[];
extern	const	struct	flag_type	comm_flags[];
extern	const	struct	flag_type	con_state_flags[];
extern	const	struct	flag_type	admin_comm_flags[];
extern	const	struct	flag_type	extra_flags[];
extern	const	struct	flag_type	extra2_flags[];
extern	const	struct	flag_type	wear_flags[];
extern	const	struct	flag_type	weapon_flags[];
extern	const	struct	flag_type	container_flags[];
extern	const	struct	flag_type	portal_flags[];
extern	const	struct	flag_type	room_flags[];
extern	const	struct	flag_type	exit_flags[];
extern	const	struct	flag_type	mprog_flags[];

extern const   struct  flag_type       area_flags[];
extern const   struct  flag_type       sector_flags[];
extern const   struct  flag_type       door_resets[];
extern const   struct  flag_type       wear_loc_strings[];
extern const   struct  flag_type       wear_loc_flags[];
extern const   struct  flag_type       res_flags[];
extern const   struct  flag_type       imm_flags[];
extern const   struct  flag_type       vuln_flags[];
extern const   struct  flag_type       type_flags[];
extern const   struct  flag_type       apply_flags[];
extern const   struct  flag_type       sex_flags[];
extern const   struct  flag_type       furniture_flags[];
extern const   struct  flag_type       weapon_class[];
extern const   struct  flag_type       apply_types[];
extern const   struct  flag_type       weapon_type2[];
extern const   struct  flag_type       apply_types[];
extern const   struct  flag_type       size_flags[];
extern const   struct  flag_type       position_flags[];
extern const   struct  bit_type        bitvector_type[];

extern	const	struct	flag_type	liquid_flags[];
extern	const	struct	flag_type	liquid_special_flags[];
extern	const	struct	flag_type	personality_archetypes[];
extern	const	struct	flag_type	staff_status[];
extern	const	struct	flag_type	status_table[];
extern	const	struct	flag_type	background_table[];
extern	const	struct	flag_type	influence_table[];
extern	const	struct	flag_type	rp_area_table[];
extern	const	struct	flag_type	taxi_area_table[];
extern	const	struct	flag_type	spirit_table[];
extern	const	struct	flag_type	totem_attitudes[];
extern	const	struct	flag_type	org_type[];
extern	const	struct	flag_type	org_auths[];
extern	const	struct	flag_type	insult_table[];
extern	const	struct	flag_type	quality_flags[];
extern	const	struct	flag_type	raw_material_table[];
extern	const	struct	flag_type	trait_type[];
extern	const	struct	flag_type	quirk_type[];
extern	const	struct	flag_type	compulsion_table[];
extern	const	struct	flag_type	mania_table[];
extern	const	struct	flag_type	phobia_table[];
extern	const	struct	flag_type	feeding_restriction[];
extern	const	struct	flag_type	flag_list[];
extern	const	struct	flag_type	weapon_dambonus[];
extern	const	struct	flag_type	fame_table[];
extern	const	struct	flag_type	train_messages[];
extern	const	struct	flag_type	concede_flags[];
extern	const	struct	flag_type	plot_races[];
extern	const	struct	flag_type	material_type[];

extern	const	struct	flag_type	card_face_char[];
extern	const	struct	flag_type	card_suit[];
extern	const	struct	flag_type	card_values[];

extern  const   struct  weapon_type             weapon_table    [];
extern  const   struct  item_type               item_table      [];
extern  const   struct  wiznet_type             wiznet_table    [];
extern  const   struct  attack_type             attack_table    [];
extern  const   struct  race_type               race_table      [];
extern  const   struct  pc_race_type            pc_race_table   [];
extern  const   struct  spec_type               spec_table      [];
extern  const   struct  liq_type                liq_table       [];

extern  const   struct  attrib_type             stat_table      [MAX_STATS];
extern  const   struct  skill_type              skill_table     [MAX_SKILL];
extern  const   struct  skill_list_entry        ability_table   [MAX_ABIL];

extern  const   struct  disc_type               disc_table      [];
extern  const   struct  virtue_type             virtue_table    [];
extern  const   struct  exit_table_type         exit_table      [MAX_EXITS];
extern          struct	social_type		*social_table;
extern  char *  const                           title_table     [MAX_CLAN]
                                                                [MAX_LEVEL+1]
                                                                [2];

extern          char *          const dir_name  [];
extern          const sh_int    rev_dir         [];
