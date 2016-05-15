#ifndef SUBCMDS_H
#define SUBCMDS_H

/*
 * Job commands.
 */
DECLARE_JOB_FUN( mayor_tax		);
DECLARE_JOB_FUN( mayor_appoint		);
DECLARE_JOB_FUN( judge_marry		);
DECLARE_JOB_FUN( job_commands		);
DECLARE_JOB_FUN( job_advance		);
DECLARE_JOB_FUN( job_apply		);
DECLARE_JOB_FUN( job_quit		);
DECLARE_JOB_FUN( job_nohire		);
DECLARE_JOB_FUN( sales_sell		);
DECLARE_JOB_FUN( sales_buy		);
DECLARE_JOB_FUN( sales_markup		);
DECLARE_JOB_FUN( janitor_clean		);
DECLARE_JOB_FUN( janitor_empty		);
DECLARE_JOB_FUN( maker_creation		);
DECLARE_JOB_FUN( maker_desc		);
DECLARE_JOB_FUN( maker_long_desc	);
DECLARE_JOB_FUN( maker_short_desc	);
DECLARE_JOB_FUN( maker_name		);
DECLARE_JOB_FUN( maker_sell		);
DECLARE_JOB_FUN( maker_type		);
DECLARE_JOB_FUN( maker_wear		);
DECLARE_JOB_FUN( maker_size		);
DECLARE_JOB_FUN( maker_materials	);
DECLARE_JOB_FUN( maker_typehelp		);
DECLARE_JOB_FUN( police_arrest		);
DECLARE_JOB_FUN( police_breathalise	);
DECLARE_JOB_FUN( police_background	);
DECLARE_JOB_FUN( crim_pick		);
DECLARE_JOB_FUN( crim_hotwire		);
DECLARE_JOB_FUN( crim_whack		);
DECLARE_JOB_FUN( crim_stickup		);
DECLARE_JOB_FUN( crim_case		);
DECLARE_JOB_FUN( reporter_subject	);
DECLARE_JOB_FUN( reporter_category	);
DECLARE_JOB_FUN( reporter_body		);
DECLARE_JOB_FUN( reporter_post		);
DECLARE_JOB_FUN( muso_jam		);
DECLARE_JOB_FUN( muso_sing		);
DECLARE_JOB_FUN( muso_play		);


/*
 * Influence commands.
 */
DECLARE_INFL_FUN( influence_commands	);
DECLARE_INFL_FUN( influence_advance	);
DECLARE_INFL_FUN( influence_adminlist	);
DECLARE_INFL_FUN( church_collection	);
DECLARE_INFL_FUN( church_research	);
DECLARE_INFL_FUN( church_tipoff		);
DECLARE_INFL_FUN( church_findrelic  );
DECLARE_INFL_FUN( criminal_racket	);
DECLARE_INFL_FUN( criminal_scout    );
DECLARE_INFL_FUN( political_raise	);
DECLARE_INFL_FUN( political_campaign	);
DECLARE_INFL_FUN( political_negcampaign	);
DECLARE_INFL_FUN( police_warrant	);
DECLARE_INFL_FUN( police_apb		);
DECLARE_INFL_FUN( judicial_sentence	);
DECLARE_INFL_FUN( judicial_pardon	);
DECLARE_INFL_FUN( media_suppress	);
DECLARE_INFL_FUN( media_promote		);
DECLARE_INFL_FUN( media_articles	);
DECLARE_INFL_FUN( economic_market	);
DECLARE_INFL_FUN( economic_raise	);
DECLARE_INFL_FUN( economic_trade	);
DECLARE_INFL_FUN( scientific_materials	);
DECLARE_INFL_FUN( scientific_tipoff	);

/*
 * Background commands.
 */
DECLARE_INFL_FUN( background_commands	);
DECLARE_INFL_FUN( background_advance	);
DECLARE_INFL_FUN( background_herd	);

/*
 * Newspaper commands (Staff only.)
 */
DECLARE_INFL_FUN( newspaper_commands	);
DECLARE_INFL_FUN( newspaper_new		);
DECLARE_INFL_FUN( newspaper_list	);
DECLARE_INFL_FUN( newspaper_clear	);
DECLARE_INFL_FUN( newspaper_show	);
DECLARE_INFL_FUN( newspaper_articles	);
DECLARE_INFL_FUN( newspaper_delete	);
DECLARE_INFL_FUN( newspaper_rename	);
DECLARE_INFL_FUN( newspaper_price	);
DECLARE_INFL_FUN( newspaper_place	);
DECLARE_INFL_FUN( newspaper_save	);
DECLARE_INFL_FUN( newspaper_release	);
DECLARE_INFL_FUN( newspaper_stop	);

/*
 * Stock market commands (Staff only.)
 */
DECLARE_INFL_FUN( smarket_list		);
DECLARE_INFL_FUN( smarket_show		);
DECLARE_INFL_FUN( smarket_commands	);
DECLARE_INFL_FUN( smarket_rename	);
DECLARE_INFL_FUN( smarket_ticker	);
DECLARE_INFL_FUN( smarket_create	);
DECLARE_INFL_FUN( smarket_delete	);
DECLARE_INFL_FUN( smarket_save		);
DECLARE_INFL_FUN( smarket_price		);

/*
 * Housing commands.
 */
DECLARE_BLD_FUN( home_commands		);
DECLARE_BLD_FUN( home_prices		);
DECLARE_BLD_FUN( home_buy		);
DECLARE_BLD_FUN( home_sell		);
DECLARE_BLD_FUN( home_name		);
DECLARE_BLD_FUN( home_desc		);
DECLARE_BLD_FUN( home_list		);

#endif