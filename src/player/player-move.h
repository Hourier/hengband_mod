﻿#pragma once

extern void disturb(player_type *creature_ptr, bool stop_search, bool flush_output);
extern void move_player(player_type *creature_ptr, DIRECTION dir, bool do_pickup, bool break_trap);
extern void run_step(player_type *creature_ptr, DIRECTION dir);

#define MPE_STAYING       0x00000001
#define MPE_FORGET_FLOW   0x00000002
#define MPE_HANDLE_STUFF  0x00000004
#define MPE_ENERGY_USE    0x00000008
#define MPE_DONT_PICKUP   0x00000010
#define MPE_DO_PICKUP     0x00000020
#define MPE_BREAK_TRAP    0x00000040
#define MPE_DONT_SWAP_MON 0x00000080
extern bool move_player_effect(player_type *creature_ptr, POSITION ny, POSITION nx, BIT_FLAGS mpe_mode);

extern void py_pickup_aux(player_type *owner_ptr, OBJECT_IDX o_idx);
extern bool pattern_seq(player_type *creature_ptr, POSITION c_y, POSITION c_x, POSITION n_y, POSITION n_x);
extern bool trap_can_be_ignored(player_type *creature_ptr, FEAT_IDX feat);
extern void search(player_type *creature_ptr);
extern void carry(player_type *creature_ptr, bool pickup);
extern void do_cmd_travel(player_type *creature_ptr);
extern void travel_step(player_type *creature_ptr);

/* Types of pattern tiles */
#define NOT_PATTERN_TILE      -1
#define PATTERN_TILE_START    0
#define PATTERN_TILE_1        1
#define PATTERN_TILE_2        2
#define PATTERN_TILE_3        3
#define PATTERN_TILE_4        4
#define PATTERN_TILE_END      5
#define PATTERN_TILE_OLD      6
#define PATTERN_TILE_TELEPORT 7
#define PATTERN_TILE_WRECKED  8

 /*
  *  A structure type for travel command
  */
typedef struct {
	int run; /* Remaining grid number */
	int cost[MAX_HGT][MAX_WID];
	POSITION x; /* Target X */
	POSITION y; /* Target Y */
	DIRECTION dir; /* Running direction */
} travel_type;

extern travel_type travel;
