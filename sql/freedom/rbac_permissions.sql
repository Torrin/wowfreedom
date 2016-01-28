/** 
* -- WOW FREEDOM PERMISSION SETS --
* Description: main SQL script to launch to change RBAC permissions to fit WoW Freedom 548 needs
* Usage: Launch this script and use '.reload rbac' (without quotes) in-game to reload RBAC perms.
* Modification: Remember to update this script when making permanent changes to RBAC permissions.
*/

/**
* PERMISSION GROUP STRUCTURE:
* NOTE: 	Lesser ID PERM GROUP inherits all permissions from higher ID PERM GROUP.
*			e.g. PERM GROUP 198 (mod) has all permissions of PERM GROUP 199 (player)
* 190 - Role: [GM3] Administrator
* 191 - Role: [GM2] Gamemaster
* 198 - Role: [GM1] Moderator, ST
* 199 - Role: [GM0] Player
*/
START TRANSACTION;

-- [DELETION: LINKED PERMS TO DEFAULT SEC LEVELS]
DELETE FROM auth.rbac_default_permissions;
-- [DELETION: LINKED PERMS]
DELETE FROM auth.rbac_linked_permissions;
-- [DELETION: CUSTOM PERMS]
DELETE FROM auth.rbac_permissions WHERE `id` IN (190, 191, 192, 193, 194, 195, 196, 197, 198, 199);
DELETE FROM auth.rbac_permissions WHERE `id` >= 1000;

-- [RECREATION: CUSTOM PERMS]
INSERT INTO auth.rbac_permissions (`id`, `name`) VALUES
-- roles
(190, 'Role: [GM3] Administrator'),
(191, 'Role: [GM2] Gamemaster'),
(197, 'Role: [GM1] Moderator, STG'),
(198, 'Role: [GM1] Moderator, ST'),
(199, 'Role: [GM0] Player'),
-- [.freedom]
(1000, 'Command: freedom'), 					(1001, 'Command: freedom summon'), 		(1002, 'Command: freedom teleport'),
(1003, 'Command: freedom pteleport'), 		(1004, 'Command: freedom morph'), 		(1005, 'Command: freedom demorph'),
(1006, 'Command: freedom bank'), 			(1007, 'Command: freedom mailbox'), 	(1008, 'Command: freedom revive'),
(1009, 'Command: freedom unaura'),			(1010, 'Command: freedom fix'),			(1011, 'Command: freedom money'),
(1012, 'Command: freedom walk'),				(1013, 'Command: freedom run'),			(1014, 'Command: freedom swim'),
(1015, 'Command: freedom speed'),			(1016, 'Command: freedom scale'),		(1017, 'Command: freedom drunk'),
(1018, 'Command: freedom waterwalk'),		(1019, 'Command: freedom fly'),			(1020, 'Command: freedom teleport add/del'),
(1021, 'Command: freedom morph add/del'), (1022, 'Command: freedom morph list'), (1023, 'Command: freedom ptele list'),
(1024, 'Command: freedom tele list'),     (1025, 'Command: freedom customize'), (1026, 'Command: freedom racechange'),
(1027, 'Command: freedom factionchange'), (1028, 'Command: freedom spell'), (1029, 'Command: freedom spell add/del'),
-- [.gobject]
(3000, 'Command: gobject set scale'), (3001, 'Command: gobject phase'), (3002, 'Command: gobject select'),
(3003, 'Command: gobject disable'), (3004, 'Command: gobject enable'),
-- [.goto]
(4000, 'Command: goto local/relative'),
-- [.npc]
(5000, 'Command: npc disable'), (5001, 'Command: npc enable'), (5002, 'Command: npc select/unselect'),
(5003, 'Command: npc set scale'), (5004, 'Command: npc set emote'), (5005, 'Command: npc set state'),
-- [misc]
(8000, 'Allow adding hidden items through additem command'),
(8001, 'Command: hideitem'),
(8002, 'Command: unhideitem'),
(8003, 'Allow showing private information when using certain commands'),
(8004, 'Command: promoteto/demoteto'),
(8005, 'Command: raid/party')
;

-- [RECREATION: LINKED PERMS]
SET @PLAYER			:= 199;
SET @MODERATOR		:= 198;
SET @GM				:= 191;
SET @ADMIN			:= 190;
INSERT INTO auth.rbac_linked_permissions (`id`, `linkedId`) VALUES
-- [199 - Role: [GM0] Player]
-- [SPECIAL]
(@PLAYER, 1), 	-- Instant logout
(@PLAYER, 2), 	-- Skip Queue
(@PLAYER, 3), 	-- Join Normal Battleground
(@PLAYER, 4), 	-- Join Random Battleground
(@PLAYER, 5), 	-- Join Arenas
(@PLAYER, 6), 	-- Join Dungeon Finder
(@PLAYER, 13),	-- Skip Instance required bosses check
(@PLAYER, 14),	-- Skip character creation team mask check
(@PLAYER, 15), -- Skip character creation class mask check
(@PLAYER, 16), -- Skip character creation race mask check
(@PLAYER, 17), -- Skip character creation reserved name check
(@PLAYER, 18), -- Skip character creation heroic min level check
(@PLAYER, 19), -- Skip needed requirements to use channel check
(@PLAYER, 20), -- Skip disable map check
(@PLAYER, 21), -- Skip reset talents when used more than allowed check
(@PLAYER, 22), -- Skip spam chat check
(@PLAYER, 23), -- Skip over-speed ping check
(@PLAYER, 24), -- Two side faction characters on the same account
(@PLAYER, 25), -- Allow say chat between factions
(@PLAYER, 26), -- Allow channel chat between factions
(@PLAYER, 27), -- Two side mail interaction
(@PLAYER, 28), -- See two side who list
(@PLAYER, 29), -- Add friends of other faction
(@PLAYER, 30), -- Save character without delay with .save command
(@PLAYER, 33), -- Notify if a command was not found
(@PLAYER, 38), -- Resurrect with full Health Points
(@PLAYER, 40), -- Allows to add a gm to friend list
(@PLAYER, 50), -- Allow user to check his own email with .account

-- [DEFAULT COMMANDS]
(@PLAYER, 374), -- gm ingame
(@PLAYER, 442), (@PLAYER, 447), -- lookup, lookup item
(@PLAYER, 488), -- additem
(@PLAYER, 496), -- commands
(@PLAYER, 501), -- dismount
(@PLAYER, 502), -- distance
(@PLAYER, 505), -- gps
(@PLAYER, 507), -- help

-- [CUSTOM COMMANDS]
(@PLAYER, 1000), (@PLAYER, 1001), (@PLAYER, 1002), (@PLAYER, 1003), (@PLAYER, 1004), (@PLAYER, 1005), (@PLAYER, 1006), (@PLAYER, 1007), (@PLAYER, 1008), (@PLAYER, 1009), -- freedom 
(@PLAYER, 1010), (@PLAYER, 1011), (@PLAYER, 1012), (@PLAYER, 1013), (@PLAYER, 1014), (@PLAYER, 1015), (@PLAYER, 1016), (@PLAYER, 1017), (@PLAYER, 1018), (@PLAYER, 1019), -- freedom 
(@PLAYER, 1022), (@PLAYER, 1023), (@PLAYER, 1024), (@PLAYER, 1025), (@PLAYER, 1026), (@PLAYER, 1027), (@PLAYER, 1028), -- freedom
(@PLAYER, 8005), -- raid/party

-- [198 - Role: [GM1] Moderator, ST]
(@MODERATOR, @PLAYER),
-- [SPECIAL]
(@MODERATOR, 31), -- Use params with .unstuck command
(@MODERATOR, 34), -- Check if should appear in list using .gm ingame command
(@MODERATOR, 39), -- Restore saved gm setting states

-- [DEFAULT COMMANDS]
(@MODERATOR, 267), (@MODERATOR, 268), (@MODERATOR, 269), (@MODERATOR, 270), (@MODERATOR, 271), (@MODERATOR, 272), -- cast
(@MODERATOR, 377), (@MODERATOR, 378), (@MODERATOR, 379), (@MODERATOR, 380), (@MODERATOR, 381), (@MODERATOR, 382), (@MODERATOR, 383), (@MODERATOR, 384), (@MODERATOR, 385), (@MODERATOR, 386),-- goto
(@MODERATOR, 387), (@MODERATOR, 388), (@MODERATOR, 389), (@MODERATOR, 390), (@MODERATOR, 391), (@MODERATOR, 392), (@MODERATOR, 393), (@MODERATOR, 394), (@MODERATOR, 395), (@MODERATOR, 396), (@MODERATOR, 397), (@MODERATOR, 398), (@MODERATOR, 399),-- gobject
(@MODERATOR, 436), (@MODERATOR, 437), (@MODERATOR, 439), (@MODERATOR, 440), (@MODERATOR, 441),-- list
(@MODERATOR, 443), (@MODERATOR, 444), (@MODERATOR, 445), (@MODERATOR, 446), (@MODERATOR, 449), (@MODERATOR, 455), (@MODERATOR, 456), (@MODERATOR, 457), (@MODERATOR, 458), (@MODERATOR, 459), (@MODERATOR, 460), (@MODERATOR, 461),-- lookup
(@MODERATOR, 472), (@MODERATOR, 478), -- group, group summon
(@MODERATOR, 490), -- appear
(@MODERATOR, 491), -- aura
(@MODERATOR, 492), -- bank
(@MODERATOR, 493), -- bindsight
(@MODERATOR, 494), -- combatstop
(@MODERATOR, 495), -- cometome
(@MODERATOR, 497), -- cooldown
(@MODERATOR, 506), -- guid
(@MODERATOR, 519), -- possess
(@MODERATOR, 520), -- recall
(@MODERATOR, 523), -- revive
(@MODERATOR, 528), -- summon
(@MODERATOR, 529), -- unaura
(@MODERATOR, 530), -- unbindsight
(@MODERATOR, 533), -- unpossess
(@MODERATOR, 535), -- wchange
(@MODERATOR, 542), -- morph
(@MODERATOR, 543), -- demorph
(@MODERATOR, 544), (@MODERATOR, 547), (@MODERATOR, 555), (@MODERATOR, 556), (@MODERATOR, 560), (@MODERATOR, 568), (@MODERATOR, 562), (@MODERATOR, 564), (@MODERATOR, 565), (@MODERATOR, 566), -- modify, modify drunk, modify mount, modify phase, modify scale, modify standstate, modify all, modify fly, modify run, modify swim
(@MODERATOR, 570), (@MODERATOR, 571), (@MODERATOR, 576), (@MODERATOR, 578), (@MODERATOR, 579), (@MODERATOR, 580), (@MODERATOR, 583), (@MODERATOR, 585), (@MODERATOR, 587), (@MODERATOR, 589), (@MODERATOR, 593), (@MODERATOR, 594), (@MODERATOR, 595), (@MODERATOR, 596), (@MODERATOR, 597), (@MODERATOR, 598), (@MODERATOR, 599), (@MODERATOR, 600), (@MODERATOR,601), -- npc
(@MODERATOR, 737), (@MODERATOR, 738), (@MODERATOR, 739), (@MODERATOR, 740), -- tele
(@MODERATOR, 767), (@MODERATOR, 768), (@MODERATOR, 769), (@MODERATOR, 770), (@MODERATOR, 771), (@MODERATOR, 772), (@MODERATOR, 773), (@MODERATOR, 774), -- wp

-- [CUSTOM COMMANDS]
(@MODERATOR, 3000), (@MODERATOR, 3001), (@MODERATOR, 3002), -- gobject: scale/phase/select
(@MODERATOR, 4000), -- goto local
(@MODERATOR, 5002), (@MODERATOR, 5003), (@MODERATOR, 5004), (@MODERATOR, 5005), -- npc: select/unselect, [set] scale, set emote, set state

-- [191 - Role: [GM2] Gamemaster]
(@GM, @MODERATOR),
-- [SPECIAL]
(@GM, 32),   -- Can be assigned tickets with .assign ticket command
(@GM, 44),   -- Receive global GM messages/texts
(@GM, 46),   -- Change channel settings without being channel moderator
(@GM, 47),   -- Enables lower security than target check
(@GM, 48),   -- Enable IP, Last Login and EMail output in pinfo
(@GM, 8000), -- Allow adding hidden items through additem command
(@GM, 8003), -- Allow showing private information when using certain commands
-- [DEFAULT COMMANDS]
(@GM, 239), (@GM, 240), (@GM, 241), (@GM, 242), (@GM, 243), -- ban
(@GM, 244), (@GM, 245), (@GM, 246), (@GM, 247), -- baninfo
(@GM, 248), (@GM, 249), (@GM, 250), (@GM, 251), -- banlist
(@GM, 252), (@GM, 253), (@GM, 254), (@GM, 255), (@GM, 256), -- unban
(@GM, 273), (@GM, 274), (@GM, 275), (@GM, 276), (@GM, 277), (@GM, 279), (@GM, 280), (@GM, 283), (@GM, 284), (@GM, 285), (@GM, 286), -- character
(@GM, 291), (@GM, 292), (@GM, 293), (@GM, 294), -- cheat
(@GM, 371), (@GM, 372), (@GM, 373), (@GM, 374), (@GM, 375), (@GM, 376), -- gm
(@GM, 401), (@GM, 402), (@GM, 403), (@GM, 404), (@GM, 405), (@GM, 406), (@GM, 407), -- guild
-- honor (408 - 411) skipped, not useful
(@GM, 417), (@GM, 418), (@GM, 419), (@GM, 420), (@GM, 421), (@GM, 422), (@GM, 423), (@GM, 424), (@GM, 425), (@GM, 426), (@GM, 427), (@GM, 428), -- learn
(@GM, 429), -- unlearn
-- lfg (430 - 435) skipped, not useful
(@GM, 438), -- .list item
(@GM, 451), (@GM, 452), (@GM, 453), (@GM, 454), -- lookup player
(@GM, 448), -- lookup itemset
(@GM, 462), (@GM, 466), (@GM, 467), (@GM, 468), (@GM, 469), (@GM, 470), (@GM, 471), -- announce, gmannounce, gmnameannounce, gmnotify, nameannounce, whispers
(@GM, 463), (@GM, 464), (@GM, 465), -- channel
(@GM, 473), (@GM, 474), (@GM, 475), (@GM, 476), (@GM, 477), -- group
(@GM, 479), (@GM, 480), (@GM, 481), (@GM, 482), -- pet
(@GM, 489), -- additemset
(@GM, 500), -- die
(@GM, 510), -- kick
(@GM, 511), -- linkgrave
(@GM, 513), -- maxskill
(@GM, 515), -- mute
(@GM, 516), -- neargrave
(@GM, 517), -- pinfo
(@GM, 522), -- respawn
(@GM, 524), -- saveall
(@GM, 525), -- save
(@GM, 526), -- setskill
(@GM, 532), -- unmute
(@GM, 567), -- modify spell
(@GM, 573), (@GM, 575), -- npc add formation/item/temp
(@GM, 577), -- npc del item
(@GM, 582), -- npc set entry
(@GM, 738), (@GM, 739), (@GM, 741), -- tele add/del/group
(@GM, 742), (@GM, 743), (@GM, 744), (@GM, 745), (@GM, 746), (@GM, 747), (@GM, 748), (@GM, 749), (@GM, 750), (@GM, 751), (@GM, 752), (@GM, 753), (@GM, 754), (@GM, 755), (@GM, 756), (@GM, 757), (@GM, 758), (@GM, 759), (@GM, 760), -- ticket
(@GM, 761), (@GM, 762), (@GM, 763), (@GM, 764), (@GM, 765), (@GM, 766), -- titles

-- [CUSTOM COMMANDS]
(@GM, 1020), (@GM, 1021), (@GM, 1029), -- freedom tele add/del, freedom morph add/del, freedom spell add/del
(@GM, 3003), (@GM, 3004), -- gobject disable/enable
(@GM, 5000), (@GM, 5001), -- npc disable/enable
(@GM, 8001), (@GM, 8002), -- hide/unhide items for the public (GMLVL 0 and 1 / Players & Moderators)
(@GM, 8004), -- promoteto & demoteto commands

-- [190 - Role: [GM3] Administrator]
(@ADMIN, @GM),
-- [DEFAULT COMMANDS]
(@ADMIN, 200), (@ADMIN, 201), (@ADMIN, 202), (@ADMIN, 203), (@ADMIN, 204), (@ADMIN, 205), (@ADMIN, 206), -- rbac
(@ADMIN, 217), (@ADMIN, 218), (@ADMIN, 220), (@ADMIN, 221), (@ADMIN, 222), (@ADMIN, 223), (@ADMIN, 224), (@ADMIN, 225), (@ADMIN, 226), (@ADMIN, 227), (@ADMIN, 228), (@ADMIN, 229), (@ADMIN, 263), (@ADMIN, 264), (@ADMIN, 265), (@ADMIN, 266), -- account
(@ADMIN, 230), (@ADMIN, 231), -- achievement
(@ADMIN, 232), (@ADMIN, 233), (@ADMIN, 234), (@ADMIN, 235), (@ADMIN, 236), (@ADMIN, 237), (@ADMIN, 238), -- arena
(@ADMIN, 257), (@ADMIN, 258), (@ADMIN, 259), (@ADMIN, 260), (@ADMIN, 261), (@ADMIN, 262), -- bf
(@ADMIN, 287), -- level up
(@ADMIN, 288), (@ADMIN, 289), (@ADMIN, 290), -- pdump
(@ADMIN, 300), (@ADMIN, 301), (@ADMIN, 302), (@ADMIN, 303), (@ADMIN, 304), (@ADMIN, 305), (@ADMIN, 306), (@ADMIN, 307), (@ADMIN, 308), (@ADMIN, 309), (@ADMIN, 310), (@ADMIN, 311), (@ADMIN, 312), (@ADMIN, 313), (@ADMIN, 314), (@ADMIN, 315), (@ADMIN, 316), (@ADMIN, 317), (@ADMIN, 318), (@ADMIN, 319), (@ADMIN, 320), (@ADMIN, 321), (@ADMIN, 322), (@ADMIN, 323), (@ADMIN, 324), (@ADMIN, 325), (@ADMIN, 326), (@ADMIN, 327), (@ADMIN, 328), (@ADMIN, 329), (@ADMIN, 330), (@ADMIN, 331), (@ADMIN, 332), (@ADMIN, 333), (@ADMIN, 334), (@ADMIN, 335), (@ADMIN, 336), (@ADMIN, 337), (@ADMIN, 338), (@ADMIN, 339), (@ADMIN, 400),-- debug
(@ADMIN, 340), -- wgps
(@ADMIN, 348), (@ADMIN, 349), (@ADMIN, 350), (@ADMIN, 351), (@ADMIN, 352), (@ADMIN, 353), (@ADMIN, 354), (@ADMIN, 355), (@ADMIN, 356), (@ADMIN, 357), (@ADMIN, 358), (@ADMIN, 359), (@ADMIN, 360), (@ADMIN, 361), (@ADMIN, 362), (@ADMIN, 363), (@ADMIN, 364), (@ADMIN, 365), (@ADMIN, 366), -- disable 
(@ADMIN, 412), (@ADMIN, 413), (@ADMIN, 414), (@ADMIN, 415), (@ADMIN, 416), -- instance
(@ADMIN, 483), (@ADMIN, 484), (@ADMIN, 485), (@ADMIN, 486), (@ADMIN, 487), -- send
(@ADMIN, 499), -- dev
(@ADMIN, 508), -- hidearea
(@ADMIN, 514), -- movegens
(@ADMIN, 518), -- playall
(@ADMIN, 527), -- showarea
(@ADMIN, 536), (@ADMIN, 537), (@ADMIN, 538), (@ADMIN, 539), (@ADMIN, 540), (@ADMIN, 541), -- mmap
(@ADMIN, 584), (@ADMIN, 590), (@ADMIN, 591), (@ADMIN, 592), -- npc set flag/data
(@ADMIN, 607), (@ADMIN, 608), (@ADMIN, 609), (@ADMIN, 610), (@ADMIN, 611), (@ADMIN, 612), (@ADMIN, 613), (@ADMIN, 614), (@ADMIN, 615), (@ADMIN, 616), (@ADMIN, 617), (@ADMIN, 618), (@ADMIN, 619), (@ADMIN, 620), (@ADMIN, 621), (@ADMIN, 622), (@ADMIN, 623), (@ADMIN, 624), (@ADMIN, 625), (@ADMIN, 626), (@ADMIN, 627), (@ADMIN, 628), (@ADMIN, 629), (@ADMIN, 630), (@ADMIN, 631), (@ADMIN, 632), (@ADMIN, 633), (@ADMIN, 634), (@ADMIN, 635), (@ADMIN, 636), (@ADMIN, 637), (@ADMIN, 638), (@ADMIN, 639), (@ADMIN, 640), (@ADMIN, 641), (@ADMIN, 642), (@ADMIN, 643), (@ADMIN, 644), (@ADMIN, 645), (@ADMIN, 646), (@ADMIN, 647), (@ADMIN, 648), (@ADMIN, 649), (@ADMIN, 650), (@ADMIN, 651), (@ADMIN, 652), (@ADMIN, 653), (@ADMIN, 654), (@ADMIN, 655), (@ADMIN, 656), (@ADMIN, 657), (@ADMIN, 658), (@ADMIN, 659), (@ADMIN, 660), (@ADMIN, 661), (@ADMIN, 662), (@ADMIN, 663), (@ADMIN, 664), (@ADMIN, 665), (@ADMIN, 666), (@ADMIN, 667), (@ADMIN, 668), (@ADMIN, 669), (@ADMIN, 670), (@ADMIN, 671), (@ADMIN, 672), (@ADMIN, 673), (@ADMIN, 674), (@ADMIN, 675), (@ADMIN, 676), (@ADMIN, 677), (@ADMIN, 678), (@ADMIN, 679), (@ADMIN, 680), (@ADMIN, 681), (@ADMIN, 682), (@ADMIN, 683), (@ADMIN, 684), (@ADMIN, 685), (@ADMIN, 686), (@ADMIN, 687), (@ADMIN, 688), (@ADMIN, 689), (@ADMIN, 690), (@ADMIN, 691), (@ADMIN, 692), (@ADMIN, 693), (@ADMIN, 694), (@ADMIN, 695), (@ADMIN, 696), (@ADMIN, 697), (@ADMIN, 698), (@ADMIN, 699), (@ADMIN, 700), (@ADMIN, 701), (@ADMIN, 702), (@ADMIN, 703), (@ADMIN, 704), (@ADMIN, 705), (@ADMIN, 706), (@ADMIN, 707), (@ADMIN, 708), (@ADMIN, 709), -- reload
(@ADMIN, 710), (@ADMIN, 711), (@ADMIN, 712), (@ADMIN, 713), (@ADMIN, 714), (@ADMIN, 715), (@ADMIN, 716), (@ADMIN, 717), -- reset
(@ADMIN, 718), (@ADMIN, 719), (@ADMIN, 720), (@ADMIN, 721), (@ADMIN, 722), (@ADMIN, 723), (@ADMIN, 724), (@ADMIN, 725), (@ADMIN, 726), (@ADMIN, 727), (@ADMIN, 728), (@ADMIN, 729), (@ADMIN, 730), (@ADMIN, 731), (@ADMIN, 732), (@ADMIN, 733), (@ADMIN, 734), (@ADMIN, 735), (@ADMIN, 736) -- server
;

-- [RECREATION: LINKED PERMS TO DEFAULT SEC LEVELS]
INSERT INTO auth.rbac_default_permissions (`secId`, `permissionId`) VALUES 
(0, 199), -- GM LVL 0 / PLAYER 	<-- Role: [GM0] Player
(1, 198), -- GM LVL 1 / MOD 		<-- Role: [GM1] Moderator, ST
(2, 191), -- GM LVL 2 / GM			<-- Role: [GM2] Gamemaster
(3, 190)  -- GM LVL 3 / ADMIN		<-- Role: [GM3] Administrator
;
COMMIT;