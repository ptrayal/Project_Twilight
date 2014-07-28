Project_Twilight
================

This is the repository for Project Twilight MUD.

This is the Project Twilight codebase, which is based off of Rom2.4.  It was originally started by Dsarky and is now 
being maintained by Rayal with help from others.  Before putting it up on Github, work was done on it to get it more 
stable.  Work continues on the code in order to get it stable and running and then eventually begin adding new features.


Pre-Github Log:
December
December 17, 2012

    Long break I know. Burned out. Got an itch and added the MSSP variables for a bunch of stuff. Nothing significant on the player end, but will be cool once I start posting it places. 

November
November 9, 2012

    Removed the sewer area. It was unfinished and bad.
    Cleaned up the area directory. 

November 5, 2012

    Removed the new school I was working on. Going to do something else.
    Fixed a problem in the bank.
    Fixed feed command. 

November 2, 2012

    Back after a little break.
    Fixed a problem in mob_interpret.
    Fixed do_exits problem.
    smarket delete command bug fixed. 

October
October 23, 2012

    Continuing to work on areas:
        Removed old city hall. Was never finished.
        Removed bad object from zoo.are
        Fixed a bunch of objects in the hospital area. 

October 22, 2012

    Fixed some objects without descriptions.
        City.are
        tenement.are
        police.are
        arclamp.are
        pawnshop.are
        bluedoor.are 
    Replaced the old command checks to see if an NPC is executing the command with the new CheckChNPC.
    Found a problem where mob_progs and triggers cause a conflict and sometimes segfault the mud. Removed the ones I could find, there may be more.
        City.area
        shopping.are
        fbibuilding.are
        hotel.are
        newschool.are 

October 17, 2012

    With the new format_string added, I removed the ".w" in the string editor.
    Added more CheckPC's to various code locations. 

October 16, 2012

    But some code in create_object to hopefully help catch objects without descriptions. 

October 15, 2012

    Looking at it, this is the most work (especially documented work) I have done in a while. These fixes are either cosmetic or important, but they are documented either way.
    Fixed a bug in find_keeper. Had to do with mob's having a NULL profession. Now fixed to match the table.
    Fixed a bug in do_score which wasn't really a bug, but Mortal's don't have "Clan" or "Generation". So, they needed to be removed from the score.
    Fixed a bug in do_whois. Extra "%s" was in it.
    Fixed a bug where it was not recognizing the ch->player variable. It is because it was saving as "Playd" but loading as "Plyd". Now both changed to "Playd".
    All "fail" are now properly declared. One variable declared through the entire code, who knows how many more to do. :)
    ch->material was never properly being set, so now it is set for new_char to be "flesh". Go figure.
    Fixing ch->material allowed me to find a bug in the mob_stat command which was displaying parts wrong. That's now fixed. mob_stat now also shows the material of the mob (which I used to check to make sure it was being set properly).
    Mob_stat had information about herd timers and power timers added. Good for staff to know.
    Replaced format_string with one from mudbytes (http://www.mudbytes.net/file-2768). I edited it to comply with current standards. This should allow us to move more quickly through OLC. 

October 12, 2012

    Many of the calls to void bug had "[*****] BUG:" in it, when that information was also in the actual call to bug, so it was repeating it twice. Removed it to make logging cleaner.
    Gave myself a break and focused on some area stuff.
        Removed the incomplete harbor area.
        Started creating some standard equipment for combat. Want to get that put in so when combat is functional, everything will be in place for that. 

October 10, 2012

    Fixed a bug in do_wear.
    Fixed several starting objects without descriptions or longs.
    Added some additional debugging.
    Tweaked the do_score command. Now more WoD-like. Not completely, but a step in the right direction. 

October 9, 2012

    Working on some of the save features to get them to be a bit cleaner and less likely to make us go boom. Been getting help with this (Darien), so thanks. 

October 5, 2012

    OK, title is now really working. The truncation was solved, but now I solved the saving. Of course, if I get rid of Title, then it's all moot, but at least it's fixed.
    Upgrade to KaVir's new Protcol snippet, version 7. 

October 4, 2012

    OK, well, wasn't planning on much today, but Darien helped me fix a few things.
        Title is now working properly.
        String_add is now working properly (which means most of OLC should be good). 

October 2, 2012

    Fixed most of the string bugs, but there is still one with pretty_proc. Frustrating. But, at least I can now edit most things from the OLC editor now. That's always an improvement.
    Played with the testscore command some more. Will probably implement parts of it in the existing score command soon.
    Went through a bunch of files and declared a bunch of int's. This, not surprisingly, caused a bunch of jumping when I shutdown the mud in the various bugs I found. Hopefully, I'll get shutdown so it shuts down safely someday. :)
    Still a leak in free_char I need to track down. Makes me sad. 

October 1, 2012

    Uncovered a few more bugs. Working on fixing them. Found them because I started plugging away at something else and it uncovered them.
    Began working on a new score function. Trying to make it feel more WoD-like. Starting to look good. Need to learn how to work with columns better. Then I'll definitely be able to make it rock out. I want to be able to have the look and feel of a WoD sheet. It's getting there, but it's allowed me to uncover other problems. 

September
September 28, 2012

    Fixed a few more bugs. Still need to get the OLC editor up and working. That will go a long way towards getting the mud more stable.
    I think once the OLC editor is fixed (string.c), then I'll be able to really fix a lot of crash bugs dealing with null's in the objects and mobs.
    Still not sure where the memory leak in free_char is, but it's not as big of a deal. I'd like to get it fixed, but again, not a huge issue yet. Though, I imagine that closing those leaks will help with other issues, I just don't know where it is going wrong.
    Started working on a side project to make it a bit more user friendly. We'll see how it works as I get the time. 

September 26, 2012

    Back in the saddle. Thanks to Davion from mudbytes for his help with them. Got a bunch of pretty major bugs squashed and he showed me techniques to squash more myself. Been a BIG step forward.
    Sadly, I also decided to bring down the old version of the mud. I'm not restarting it until I have a more stable version of the mud ready for play.
    Using the issue Tracking more now and archiving problems as they are fixed. Stability is slowly coming.
    Removed channels which duplicate existing functionality or were not being used. 

September 10, 2012

    squashed a few more bugs.
    Worked on fixing char array problems. Hopeful to figure out where the trash in the heap is. No such luck yet. 

September 3, 2012

    Removed the 8ball command. This is part of my effort to remove code whose purpose is iffy at best and clean up the code.
    Found another bug (inventory bug). Need to squash it.
    OK, found a lot more bugs. Working on squashing them.
    OLC is currently broke (for the most part). Need to get the string editor fixed. 

August
August 31, 2012

    Ripped out the gambling code. It was buggy and if it ever added, it will be done differently. This also fixed some other problems for things which were related.
    Continuing rewriting some powers.
    Kind of goes without saying, but removed the Casino area.
    Redid the gsn system. Should be easier to modify and add in the future. 

August 29, 2012

    Fixed a number of bugs.
    Began rewriting some of the powers. Finding some bugs along the way.
    Fixed a problem with items not being recognized. Uncovered another bug that I'm still working on. 

July
July 30, 2012

    Mud crashed over the weekend on production port. Now up and running. 

July 27, 2012

    Removed the faerie classes from the right table in table.c. Since I know nothing about the Fey and don't really care too, I'm removing them. Long-term, it's for the best.
    Removed a number of calls to various from magic.c. Need to be careful doing so. Lots of things tied together.
    Added check for SpiritTouch, found another bug in it to work on though. 

July 26, 2012

    OK, so it has been a while. I'm back in the saddle.
    Fixed a crash concerning new players.
    Added a number of checks to avoid other crashes in handler.c (lookup checks).
    FOCUSING: I've decided that I am going to focus my work. I'm going to work on continuing to smash bugs and make the vampire side of things work as well as possible. This means I will need to work on a number of things, but I want one-portion of the game to be really well-done before I move on to others. My priorities in terms of characters will be Kindred, Mortals, Garou. 

April
April 25, 2012

    Been a little remiss on not updating as much as I should. We have the development port up and running, which means the code is semi-stable on the development port. Which is huge progress. It has also enabled us to begin doing serious bug fixes and we have begun implementing a lot of stability features and bug tracking/hunting features. 

March
March 28, 2012

    So much has gone on. Lots of dev code updates have been going on. New code is in which will be help stabilize the code base.
    Help files
        Most of the help files have been updated from the old format code to the new format.
        Several help files have been deleted because they were duplicates of other help files. 

February
February 25, 2012

    We have decided to make a preliminary move to C++. This is going to make a big help in dealing with our memory issues and allow us to do much more. It unfortunately, means a bit longer before it all moves to production. In the meantime, work continues to be done on the area to bring them up to speed. 

February 24, 2012

    OK, it's been a long while since I've updated, but I've not been silent. I continue to work at trying to resolve the memory issues and every time I get something fixed, another broken thing get's revealed. So, it's gone much slower than I hoped.
    I've in the mean time been doing some work on areas in Production:
        Removed four areas which are going to be out of theme.
        Fixed up the mall a bit. Made it not so cheesy.
        Continue work on the schools.
        A number of other minor area issues have been fixed (mobs with no long descriptions). This came about from issues with the code. Long story. 

February 9, 2012

    It's been a week since I've updated, but it's not that I have not been doing anything. I have. So, here we go:
        Most of the memory system has been rewritten at this point. I've also been making sure that pointers are initialize and cleared properly (I hope), enough so that...
        The stack is no longer corrupt. This is great. Means I can start doing serious debugging. Bad news is, I can start doing serious debugging.
        Numerous fixes were made where NULL's were being called poorly and the social_table was a mess.
        Found some pieces of code that were not working correctly and need to start getting to work correctly.
        Redid parts of the bug code. May end up scrapping the whole log system entirely and going with something else. Not sure yet.
        All of this development is now taking place on a development port until I can get the code stable enough to port over to the main port.
        Working on help files. Looked through the bug file and have begun tweaking and adding help files.
        Still working...Hopefully should have it resolved soon. 

February 2, 2012

    Figured I would post an update. There is a serious bug in the memory of the code and as such, it's going to require an entire rewrite of the memory system. I anticipate this taking a week or two in order to get it up and running (the new memory system). As such, there is a copy of the MUD up and running right now and it's usable. However, nothing else is getting added because of the existing problems.
    This is a very big project and requires rewriting large portions of the code. This will undoubtedly cause other bugs to surface (which is kind of what we want, in order to find the big problem(s)), but it will allow us to accurately find out what the problems are. 

January
January 31, 2012

    There is a serious bug still somewhere in the code. I've begun the process of trying to track it down, but until I can find it, nothing further is getting done. I'm in the midst of modifying some 3000 lines of code just to track it down and it's flushing stuff up to the top that needs to be fix. At the time of writing, I've got some 700 lines left to modify before I can hopefully get it figured out. 

January 25, 2012

    Identified there is a serious memory leak in the code. Have begun work on tracking it down. All else is stopping until I can get it fixed.
    Lowered the value of MAX_STRING_LENGTH and MAX_INPUT_LENGTH to help combat memory errors.
    Began converting sprintf to snprintf to help with overflows. 

January 25, 2012

    Auspex bug fixed:
        Fixed some grammar issues in Auspex 2 (aura perception).
        Could not turn off Auspex level 4 power (telepathy). 
    Obfuscate bug fixed:
        Fixed a bug in Unseen where you couldn't end the power, effectively having permanent invisibility. 
    Presence bug fixed:
        Majesty allowed you to use it even if you had zero willpower. 
    Became consolidating code for powers. Started consolidating the various checks to their own functions in order to make it cleaner. 

January 24, 2012

    Fixed Dominate 1 and 2 powers to resist willpower.
        Exposed a few bugs in other dominate powers. 
    Fixed a bug in score. Max Willpower was actually showing before current willpower. - Fixed.
    Bug fixed in the step and taxi command.
        Made stepping and using a taxi click-able now to take you to your destination.
        Removed duplicate school entry. 

January 23, 2012

    All areas now are flagged for neighborhood type. Close to implementing hunting feature.
    Bug with Influence Timer not clearing when using the timerclear command is fixed. 

January 20, 2012

    Finally got us to a clean compile.
    Tore out the old polling and survey system. Going to end up reworking the whole thing. 

January 18, 2012

    OK, put in the beginning of the hunting tool for Kindred. The work right now is on the backend, but will allow Kindred without the Herd background to try feeding. It'll be based on either Social or Physical, plus a stat. All area's will need to be coded to designate a type. Success will give blood, failure nothing, and botching a masquerade breach! More to come. 

January 16, 2012

    Began reworking the character creation process.
        Taking the parts of it and making it their own functions.
        As that is done, each individual part get's revised as necessary and "prettied up". 

January 13, 2012

    Code cleaning on the back end.
    Began working on putting in the rest of the sire information to have that entered. 

January 12, 2012

    Began reworking parts of character creation. Trying to make it more user friendly and easier to do.
    Forgot to mention it, but ELEVATORS WORK!!!
    Ended up tweaking the channels command. 

January 11, 2012

    More Stock Market fixes
        Fixed the way stocks were saved and loaded. Easier for me to work with.
        Fixed a bug where a successful use of influence market would result in a loss of influence if successful. 
    Made some more updates to whois.
    Made updates to the org command. 

January 9, 2012

    Ritual bug which locks the mud up is now fixed.
    whois now gives much different information and is formatted differently.
    Who is still in the works.
    Stocks added to stock market. 

January 6, 2012

    Fixed a bug in rpavail. Discovered it wasn't saving the rptitles to the pfile nor obviously loading them correctly. Fixed.
        See Roleplaying for more details. 
    Fixed a bug in do_who.
    Added appearance_string to help deal with appearances.
    Who command being revised.
        Removing information which is duplicated by the whois command.
        Revising color formatting to be consistent. 

January 5, 2012

    Continued work on interp.c
        Race commands now sorted.
        Revised command so it now excludes race specific commands. This means that command shows commands everyone can use and racecommand shows only commands for your specific race. This helps to keep the command command list more manageable.
        Finished work in the command list. Removed almost a 100 duplicate commands. 

January 4, 2012

    Improved versioning. Now will keep track of major, minor, and builds.
        For simplicity sake, we are starting at 1.1.1.1 
    Fixed a bug in aura perception.
    Began reworking the interp.c file to sort and clean up command table.
        Immortal commands are done and sorted. 

January 3, 2012

    More work on colors (removing Lopes color code and using UTF-8 color standards)
    Many influences now tell the player if the succeed/fail/botch.
    Herd tells you if you succeed/fail/botch.
    When your herd/influence/power timer reset, it now let's you know in White.
    Made some updates to the OLC editor. Only builders will see this.
    Began to put in the beginning of the version system.
        Revised the do_update command
        Added the code engine declaration. 
    Made a revision to the RSS feed.
    Fixed a bug in the smarket code where it was not showing the cents correctly.
    Did some formatting in the notes area.
    Basic versioning added. 
