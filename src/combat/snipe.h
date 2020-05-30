﻿#pragma once

/* Sniper */
#define SP_NONE          0
#define SP_LITE          1
#define SP_AWAY          2
#define SP_FIRE          3
#define SP_KILL_WALL     4
#define SP_COLD          5
#define SP_KILL_TRAP     6
#define SP_ELEC          7
#define SP_PIERCE        8
#define SP_RUSH          9
#define SP_DOUBLE        10
#define SP_EXPLODE       11
#define SP_EVILNESS      12
#define SP_HOLYNESS      13
#define SP_FINAL         14
#define SP_NEEDLE        15

/* snipe.c */
extern void reset_concentration(player_type *creature_ptr, bool msg);
extern void display_snipe_list(player_type *sniper_ptr);
extern MULTIPLY calc_snipe_damage_with_slay(player_type *sniper_ptr, MULTIPLY mult, monster_type *m_ptr, SPELL_IDX snipe_type);
extern void do_cmd_snipe(player_type *sniper_ptr);
extern void do_cmd_snipe_browse(player_type *sniper_ptr);
extern int boost_concentration_damage(player_type *creature_ptr, int tdam);
