﻿#include "angband.h"
#include "util.h"

#include "dungeon.h"
#include "floor.h"
#include "grid.h"
#include "quest.h"
#include "artifact.h"
#include "object/object-kind.h"
#include "object-flavor.h"
#include "object-hook.h"

#include "io/write-diary.h"
#include "cmd/cmd-basic.h"
#include "cmd/cmd-dump.h"

#include "floor-events.h"
#include "floor-save.h"
#include "player-damage.h"
#include "player-effects.h"
#include "player-move.h"
#include "feature.h"
#include "view-mainwindow.h"

#include "monster-status.h"

#include "spells.h"
#include "spells-floor.h"

/*
 * Light up the dungeon using "clairvoyance"
 *
 * This function "illuminates" every grid in the dungeon, memorizes all
 * "objects", memorizes all grids as with magic mapping, and, under the
 * standard option settings (view_perma_grids but not view_torch_grids)
 * memorizes all floor grids too.
 *
 * Note that if "view_perma_grids" is not set, we do not memorize floor
 * grids, since this would defeat the purpose of "view_perma_grids", not
 * that anyone seems to play without this option.
 *
 * Note that if "view_torch_grids" is set, we do not memorize floor grids,
 * since this would prevent the use of "view_torch_grids" as a method to
 * keep track of what grids have been observed directly.
 */
void wiz_lite(player_type *caster_ptr, bool ninja)
{
	/* Memorize objects */
	for (OBJECT_IDX i = 1; i < caster_ptr->current_floor_ptr->o_max; i++)
	{
		object_type *o_ptr = &caster_ptr->current_floor_ptr->o_list[i];
		if (!OBJECT_IS_VALID(o_ptr)) continue;
		if (OBJECT_IS_HELD_MONSTER(o_ptr)) continue;
		o_ptr->marked |= OM_FOUND;
	}

	/* Scan all normal grids */
	for (POSITION y = 1; y < caster_ptr->current_floor_ptr->height - 1; y++)
	{
		/* Scan all normal grids */
		for (POSITION x = 1; x < caster_ptr->current_floor_ptr->width - 1; x++)
		{
			grid_type *g_ptr = &caster_ptr->current_floor_ptr->grid_array[y][x];

			/* Memorize terrain of the grid */
			g_ptr->info |= (CAVE_KNOWN);

			/* Feature code (applying "mimic" field) */
			FEAT_IDX feat = get_feat_mimic(g_ptr);
			feature_type *f_ptr;
			f_ptr = &f_info[feat];

			/* Process all non-walls */
			if (have_flag(f_ptr->flags, FF_WALL)) continue;

			/* Scan all neighbors */
			for (OBJECT_IDX i = 0; i < 9; i++)
			{
				POSITION yy = y + ddy_ddd[i];
				POSITION xx = x + ddx_ddd[i];
				g_ptr = &caster_ptr->current_floor_ptr->grid_array[yy][xx];

				/* Feature code (applying "mimic" field) */
				f_ptr = &f_info[get_feat_mimic(g_ptr)];

				/* Perma-lite the grid */
				if (!(d_info[caster_ptr->dungeon_idx].flags1 & DF1_DARKNESS) && !ninja)
				{
					g_ptr->info |= (CAVE_GLOW);
				}

				/* Memorize normal features */
				if (have_flag(f_ptr->flags, FF_REMEMBER))
				{
					/* Memorize the grid */
					g_ptr->info |= (CAVE_MARK);
				}

				/* Perma-lit grids (newly and previously) */
				else if (g_ptr->info & CAVE_GLOW)
				{
					/* Normally, memorize floors (see above) */
					if (view_perma_grids && !view_torch_grids)
					{
						/* Memorize the grid */
						g_ptr->info |= (CAVE_MARK);
					}
				}
			}
		}
	}

	caster_ptr->update |= (PU_MONSTERS);
	caster_ptr->redraw |= (PR_MAP);
	caster_ptr->window |= (PW_OVERHEAD | PW_DUNGEON);

	if (caster_ptr->special_defense & NINJA_S_STEALTH)
	{
		if (caster_ptr->current_floor_ptr->grid_array[caster_ptr->y][caster_ptr->x].info & CAVE_GLOW) set_superstealth(caster_ptr, FALSE);
	}
}


/*
 * Forget the dungeon map (ala "Thinking of Maud...").
 */
void wiz_dark(player_type *caster_ptr)
{
	/* Forget every grid */
	for (POSITION y = 1; y < caster_ptr->current_floor_ptr->height - 1; y++)
	{
		for (POSITION x = 1; x < caster_ptr->current_floor_ptr->width - 1; x++)
		{
			grid_type *g_ptr = &caster_ptr->current_floor_ptr->grid_array[y][x];

			/* Process the grid */
			g_ptr->info &= ~(CAVE_MARK | CAVE_IN_DETECT | CAVE_KNOWN);
			g_ptr->info |= (CAVE_UNSAFE);
		}
	}

	/* Forget every grid on horizontal edge */
	for (POSITION x = 0; x < caster_ptr->current_floor_ptr->width; x++)
	{
		caster_ptr->current_floor_ptr->grid_array[0][x].info &= ~(CAVE_MARK);
		caster_ptr->current_floor_ptr->grid_array[caster_ptr->current_floor_ptr->height - 1][x].info &= ~(CAVE_MARK);
	}

	/* Forget every grid on vertical edge */
	for (POSITION y = 1; y < (caster_ptr->current_floor_ptr->height - 1); y++)
	{
		caster_ptr->current_floor_ptr->grid_array[y][0].info &= ~(CAVE_MARK);
		caster_ptr->current_floor_ptr->grid_array[y][caster_ptr->current_floor_ptr->width - 1].info &= ~(CAVE_MARK);
	}

	/* Forget all objects */
	for (OBJECT_IDX i = 1; i < caster_ptr->current_floor_ptr->o_max; i++)
	{
		object_type *o_ptr = &caster_ptr->current_floor_ptr->o_list[i];

		if (!OBJECT_IS_VALID(o_ptr)) continue;
		if (OBJECT_IS_HELD_MONSTER(o_ptr)) continue;

		/* Forget the object */
		o_ptr->marked &= OM_TOUCHED;
	}

	/* Forget travel route when we have forgotten map */
	forget_travel_flow(caster_ptr->current_floor_ptr);

	caster_ptr->update |= (PU_UN_VIEW | PU_UN_LITE);
	caster_ptr->update |= (PU_VIEW | PU_LITE | PU_MON_LITE);
	caster_ptr->update |= (PU_MONSTERS);
	caster_ptr->redraw |= (PR_MAP);
	caster_ptr->window |= (PW_OVERHEAD | PW_DUNGEON);
}


/*!
 * @brief 守りのルーン設置処理 /
 * Leave a "glyph of warding" which prevents monster movement
 * @return 実際に設置が行われた場合TRUEを返す
 */
bool warding_glyph(player_type *caster_ptr)
{
	if (!cave_clean_bold(caster_ptr->current_floor_ptr, caster_ptr->y, caster_ptr->x))
	{
		msg_print(_("床上のアイテムが呪文を跳ね返した。", "The object resists the spell."));
		return FALSE;
	}

	/* Create a glyph */
	caster_ptr->current_floor_ptr->grid_array[caster_ptr->y][caster_ptr->x].info |= CAVE_OBJECT;
	caster_ptr->current_floor_ptr->grid_array[caster_ptr->y][caster_ptr->x].mimic = feat_glyph;

	note_spot(caster_ptr, caster_ptr->y, caster_ptr->x);
	lite_spot(caster_ptr, caster_ptr->y, caster_ptr->x);

	return TRUE;
}


/*!
 * @brief 爆発のルーン設置処理 /
 * Leave an "explosive rune" which prevents monster movement
 * @param caster_ptr プレーヤーへの参照ポインタ
 * @param y 設置場所
 * @param x 設置場所
 * @return 実際に設置が行われた場合TRUEを返す
 */
bool explosive_rune(player_type *caster_ptr, POSITION y, POSITION x)
{
	floor_type *floor_ptr = caster_ptr->current_floor_ptr;
	if (!cave_clean_bold(floor_ptr, y, x))
	{
		msg_print(_("床上のアイテムが呪文を跳ね返した。", "The object resists the spell."));
		return FALSE;
	}

	/* Create a glyph */
	floor_ptr->grid_array[y][x].info |= CAVE_OBJECT;
	floor_ptr->grid_array[y][x].mimic = feat_explosive_rune;

	note_spot(caster_ptr, y, x);
	lite_spot(caster_ptr, y, x);

	return TRUE;
}


/*!
 * @brief 鏡設置処理
 * @return 実際に設置が行われた場合TRUEを返す
 */
bool place_mirror(player_type *caster_ptr)
{
	if (!cave_clean_bold(caster_ptr->current_floor_ptr, caster_ptr->y, caster_ptr->x))
	{
		msg_print(_("床上のアイテムが呪文を跳ね返した。", "The object resists the spell."));
		return FALSE;
	}

	/* Create a mirror */
	caster_ptr->current_floor_ptr->grid_array[caster_ptr->y][caster_ptr->x].info |= CAVE_OBJECT;
	caster_ptr->current_floor_ptr->grid_array[caster_ptr->y][caster_ptr->x].mimic = feat_mirror;

	/* Turn on the light */
	caster_ptr->current_floor_ptr->grid_array[caster_ptr->y][caster_ptr->x].info |= CAVE_GLOW;

	note_spot(caster_ptr, caster_ptr->y, caster_ptr->x);
	lite_spot(caster_ptr, caster_ptr->y, caster_ptr->x);
	update_local_illumination(caster_ptr, caster_ptr->y, caster_ptr->x);

	return TRUE;
}


/*!
 * @brief プレイヤーの手による能動的な階段生成処理 /
 * Create stairs at or move previously created stairs into the player location.
 * @return なし
 */
void stair_creation(player_type *caster_ptr)
{
	/* Forbid up staircases on Ironman mode */
	bool up = TRUE;
	if (ironman_downward) up = FALSE;

	/* Forbid down staircases on quest level */
	bool down = TRUE;
	floor_type *floor_ptr = caster_ptr->current_floor_ptr;
	if (quest_number(caster_ptr, floor_ptr->dun_level) || (floor_ptr->dun_level >= d_info[caster_ptr->dungeon_idx].maxdepth)) down = FALSE;

	/* No effect out of standard dungeon floor */
	if (!floor_ptr->dun_level || (!up && !down) ||
		(floor_ptr->inside_quest && is_fixed_quest_idx(floor_ptr->inside_quest)) ||
		floor_ptr->inside_arena || caster_ptr->phase_out)
	{
		/* arena or quest */
		msg_print(_("効果がありません！", "There is no effect!"));
		return;
	}

	/* Artifacts resists */
	if (!cave_valid_bold(floor_ptr, caster_ptr->y, caster_ptr->x))
	{
		msg_print(_("床上のアイテムが呪文を跳ね返した。", "The object resists the spell."));
		return;
	}

	/* Destroy all objects in the grid */
	delete_object(caster_ptr, caster_ptr->y, caster_ptr->x);

	/* Extract current floor data */
	saved_floor_type *sf_ptr;
	sf_ptr = get_sf_ptr(caster_ptr->floor_id);
	if (!sf_ptr)
	{
		/* No floor id? -- Create now! */
		caster_ptr->floor_id = get_new_floor_id(caster_ptr);
		sf_ptr = get_sf_ptr(caster_ptr->floor_id);
	}

	/* Choose randomly */
	if (up && down)
	{
		if (randint0(100) < 50) up = FALSE;
		else down = FALSE;
	}

	/* Destination is already fixed */
	FLOOR_IDX dest_floor_id = 0;
	if (up)
	{
		if (sf_ptr->upper_floor_id) dest_floor_id = sf_ptr->upper_floor_id;
	}
	else
	{
		if (sf_ptr->lower_floor_id) dest_floor_id = sf_ptr->lower_floor_id;
	}
	
	/* Search old stairs leading to the destination */
	if (dest_floor_id)
	{
		POSITION x, y;

		for (y = 0; y < floor_ptr->height; y++)
		{
			for (x = 0; x < floor_ptr->width; x++)
			{
				grid_type *g_ptr = &floor_ptr->grid_array[y][x];

				if (!g_ptr->special) continue;
				if (feat_uses_special(g_ptr->feat)) continue;
				if (g_ptr->special != dest_floor_id) continue;

				/* Remove old stairs */
				g_ptr->special = 0;
				cave_set_feat(caster_ptr, y, x, feat_ground_type[randint0(100)]);
			}
		}
	}

	/* No old destination -- Get new one now */
	else
	{
		dest_floor_id = get_new_floor_id(caster_ptr);

		/* Fix it */
		if (up)
			sf_ptr->upper_floor_id = dest_floor_id;
		else
			sf_ptr->lower_floor_id = dest_floor_id;
	}

	/* Extract destination floor data */
	saved_floor_type *dest_sf_ptr;
	dest_sf_ptr = get_sf_ptr(dest_floor_id);
	
	/* Create a staircase */
	if (up)
	{
		cave_set_feat(caster_ptr, caster_ptr->y, caster_ptr->x,
			(dest_sf_ptr->last_visit && (dest_sf_ptr->dun_level <= floor_ptr->dun_level - 2)) ?
			feat_state(caster_ptr, feat_up_stair, FF_SHAFT) : feat_up_stair);
	}
	else
	{
		cave_set_feat(caster_ptr, caster_ptr->y, caster_ptr->x,
			(dest_sf_ptr->last_visit && (dest_sf_ptr->dun_level >= floor_ptr->dun_level + 2)) ?
			feat_state(caster_ptr, feat_down_stair, FF_SHAFT) : feat_down_stair);
	}

	/* Connect this stairs to the destination */
	floor_ptr->grid_array[caster_ptr->y][caster_ptr->x].special = dest_floor_id;
}

/*
 * Hack -- map the current panel (plus some) ala "magic mapping"
 */
void map_area(player_type *caster_ptr, POSITION range)
{
	if (d_info[caster_ptr->dungeon_idx].flags1 & DF1_DARKNESS) range /= 3;

	/* Scan that area */
	for (POSITION y = 1; y < caster_ptr->current_floor_ptr->height - 1; y++)
	{
		for (POSITION x = 1; x < caster_ptr->current_floor_ptr->width - 1; x++)
		{
			if (distance(caster_ptr->y, caster_ptr->x, y, x) > range) continue;

			grid_type *g_ptr;
			g_ptr = &caster_ptr->current_floor_ptr->grid_array[y][x];

			/* Memorize terrain of the grid */
			g_ptr->info |= (CAVE_KNOWN);

			/* Feature code (applying "mimic" field) */
			FEAT_IDX feat = get_feat_mimic(g_ptr);
			feature_type *f_ptr;
			f_ptr = &f_info[feat];

			/* All non-walls are "checked" */
			if (have_flag(f_ptr->flags, FF_WALL)) continue;

			/* Memorize normal features */
			if (have_flag(f_ptr->flags, FF_REMEMBER))
			{
				/* Memorize the object */
				g_ptr->info |= (CAVE_MARK);
			}

			/* Memorize known walls */
			for (int i = 0; i < 8; i++)
			{
				g_ptr = &caster_ptr->current_floor_ptr->grid_array[y + ddy_ddd[i]][x + ddx_ddd[i]];

				/* Feature code (applying "mimic" field) */
				feat = get_feat_mimic(g_ptr);
				f_ptr = &f_info[feat];

				/* Memorize walls (etc) */
				if (have_flag(f_ptr->flags, FF_REMEMBER))
				{
					/* Memorize the walls */
					g_ptr->info |= (CAVE_MARK);
				}
			}
		}
	}

	caster_ptr->redraw |= (PR_MAP);
	caster_ptr->window |= (PW_OVERHEAD | PW_DUNGEON);
}



/*!
 * @brief *破壊*処理を行う / The spell of destruction
 * @param y1 破壊の中心Y座標
 * @param x1 破壊の中心X座標
 * @param r 破壊の半径
 * @param in_generate ダンジョンフロア生成中の処理ならばTRUE
 * @return 効力があった場合TRUEを返す
 * @details
 * <pre>
 * This spell "deletes" monsters (instead of "killing" them).
 *
 * Later we may use one function for both "destruction" and
 * "earthquake" by using the "full" to select "destruction".
 * </pre>
 */
bool destroy_area(player_type *caster_ptr, POSITION y1, POSITION x1, POSITION r, bool in_generate)
{
	/* Prevent destruction of quest levels and town */
	floor_type *floor_ptr = caster_ptr->current_floor_ptr;
	if ((floor_ptr->inside_quest && is_fixed_quest_idx(floor_ptr->inside_quest)) || !floor_ptr->dun_level)
	{
		return FALSE;
	}

	/* Lose monster light */
	if (!in_generate) clear_mon_lite(floor_ptr);

	/* Big area of affect */
	bool flag = FALSE;
	for (POSITION y = (y1 - r); y <= (y1 + r); y++)
	{
		for (POSITION x = (x1 - r); x <= (x1 + r); x++)
		{
			if (!in_bounds(floor_ptr, y, x)) continue;

			/* Extract the distance */
			int k = distance(y1, x1, y, x);

			/* Stay in the circle of death */
			if (k > r) continue;
			grid_type *g_ptr;
			g_ptr = &floor_ptr->grid_array[y][x];

			/* Lose room and vault */
			g_ptr->info &= ~(CAVE_ROOM | CAVE_ICKY);

			/* Lose light and knowledge */
			g_ptr->info &= ~(CAVE_MARK | CAVE_GLOW | CAVE_KNOWN);

			if (!in_generate) /* Normal */
			{
				/* Lose unsafety */
				g_ptr->info &= ~(CAVE_UNSAFE);

				/* Hack -- Notice player affect */
				if (player_bold(caster_ptr, y, x))
				{
					/* Hurt the player later */
					flag = TRUE;

					/* Do not hurt this grid */
					continue;
				}
			}

			/* Hack -- Skip the epicenter */
			if ((y == y1) && (x == x1)) continue;

			if (g_ptr->m_idx)
			{
				monster_type *m_ptr = &floor_ptr->m_list[g_ptr->m_idx];
				monster_race *r_ptr = &r_info[m_ptr->r_idx];

				if (in_generate) /* In generation */
				{
					/* Delete the monster (if any) */
					delete_monster(caster_ptr, y, x);
				}
				else if (r_ptr->flags1 & RF1_QUESTOR)
				{
					/* Heal the monster */
					m_ptr->hp = m_ptr->maxhp;

					/* Try to teleport away quest monsters */
					if (!teleport_away(caster_ptr, g_ptr->m_idx, (r * 2) + 1, TELEPORT_DEC_VALOUR)) continue;
				}
				else
				{
					if (record_named_pet && is_pet(m_ptr) && m_ptr->nickname)
					{
						GAME_TEXT m_name[MAX_NLEN];

						monster_desc(caster_ptr, m_name, m_ptr, MD_INDEF_VISIBLE);
						exe_write_diary(caster_ptr, DIARY_NAMED_PET, RECORD_NAMED_PET_DESTROY, m_name);
					}

					/* Delete the monster (if any) */
					delete_monster(caster_ptr, y, x);
				}
			}

			/* During generation, destroyed artifacts are "preserved" */
			if (preserve_mode || in_generate)
			{
				OBJECT_IDX this_o_idx, next_o_idx = 0;

				/* Scan all objects in the grid */
				for (this_o_idx = g_ptr->o_idx; this_o_idx; this_o_idx = next_o_idx)
				{
					object_type *o_ptr;
					o_ptr = &floor_ptr->o_list[this_o_idx];
					next_o_idx = o_ptr->next_o_idx;

					/* Hack -- Preserve unknown artifacts */
					if (object_is_fixed_artifact(o_ptr) && (!object_is_known(o_ptr) || in_generate))
					{
						/* Mega-Hack -- Preserve the artifact */
						a_info[o_ptr->name1].cur_num = 0;

						if (in_generate && cheat_peek)
						{
							GAME_TEXT o_name[MAX_NLEN];
							object_desc(caster_ptr, o_name, o_ptr, (OD_NAME_ONLY | OD_STORE));
							msg_format(_("伝説のアイテム (%s) は生成中に*破壊*された。", "Artifact (%s) was *destroyed* during generation."), o_name);
						}
					}
					else if (in_generate && cheat_peek && o_ptr->art_name)
					{
						msg_print(_("ランダム・アーティファクトの1つは生成中に*破壊*された。",
							"One of the random artifacts was *destroyed* during generation."));
					}
				}
			}

			delete_object(caster_ptr, y, x);

			/* Destroy "non-permanent" grids */
			if (cave_perma_grid(g_ptr)) continue;

			/* Wall (or floor) type */
			int t = randint0(200);

			if (!in_generate) /* Normal */
			{
				if (t < 20)
				{
					/* Create granite wall */
					cave_set_feat(caster_ptr, y, x, feat_granite);
				}
				else if (t < 70)
				{
					/* Create quartz vein */
					cave_set_feat(caster_ptr, y, x, feat_quartz_vein);
				}
				else if (t < 100)
				{
					/* Create magma vein */
					cave_set_feat(caster_ptr, y, x, feat_magma_vein);
				}
				else
				{
					/* Create floor */
					cave_set_feat(caster_ptr, y, x, feat_ground_type[randint0(100)]);
				}

				continue;
			}
			
			if (t < 20)
			{
				/* Create granite wall */
				place_grid(caster_ptr, g_ptr, gb_extra);
			}
			else if (t < 70)
			{
				/* Create quartz vein */
				g_ptr->feat = feat_quartz_vein;
			}
			else if (t < 100)
			{
				/* Create magma vein */
				g_ptr->feat = feat_magma_vein;
			}
			else
			{
				/* Create floor */
				place_grid(caster_ptr, g_ptr, gb_floor);
			}

			/* Clear garbage of hidden trap or door */
			g_ptr->mimic = 0;
		}
	}

	if (in_generate) return TRUE;

	/* Process "re-glowing" */
	for (POSITION y = (y1 - r); y <= (y1 + r); y++)
	{
		for (POSITION x = (x1 - r); x <= (x1 + r); x++)
		{
			if (!in_bounds(floor_ptr, y, x)) continue;

			/* Extract the distance */
			int k = distance(y1, x1, y, x);

			/* Stay in the circle of death */
			if (k > r) continue;
			grid_type *g_ptr;
			g_ptr = &floor_ptr->grid_array[y][x];

			if (is_mirror_grid(g_ptr))
			{
				g_ptr->info |= CAVE_GLOW;
				continue;
			}
			
			if ((d_info[floor_ptr->dungeon_idx].flags1 & DF1_DARKNESS)) continue;

			DIRECTION i;
			POSITION yy, xx;
			grid_type *cc_ptr;

			for (i = 0; i < 9; i++)
			{
				yy = y + ddy_ddd[i];
				xx = x + ddx_ddd[i];
				if (!in_bounds2(floor_ptr, yy, xx)) continue;
				cc_ptr = &floor_ptr->grid_array[yy][xx];
				if (have_flag(f_info[get_feat_mimic(cc_ptr)].flags, FF_GLOW))
				{
					g_ptr->info |= CAVE_GLOW;
					break;
				}
			}
		}
	}

	/* Hack -- Affect player */
	if (flag)
	{
		msg_print(_("燃えるような閃光が発生した！", "There is a searing blast of light!"));

		/* Blind the player */
		if (!caster_ptr->resist_blind && !caster_ptr->resist_lite)
		{
			/* Become blind */
			(void)set_blind(caster_ptr, caster_ptr->blind + 10 + randint1(10));
		}
	}

	forget_flow(floor_ptr);

	/* Mega-Hack -- Forget the view and lite */
	caster_ptr->update |= (PU_UN_VIEW | PU_UN_LITE | PU_VIEW | PU_LITE | PU_FLOW | PU_MON_LITE | PU_MONSTERS);
	caster_ptr->redraw |= (PR_MAP);
	caster_ptr->window |= (PW_OVERHEAD | PW_DUNGEON);

	if (caster_ptr->special_defense & NINJA_S_STEALTH)
	{
		if (floor_ptr->grid_array[caster_ptr->y][caster_ptr->x].info & CAVE_GLOW) set_superstealth(caster_ptr, FALSE);
	}

	return TRUE;
}


/*!
 * @brief 地震処理(サブルーチン) /
 * Induce an "earthquake" of the given radius at the given location.
 * @param caster_ptrプレーヤーへの参照ポインタ
 * @param cy 中心Y座標
 * @param cx 中心X座標
 * @param r 効果半径
 * @param m_idx 地震を起こしたモンスターID(0ならばプレイヤー)
 * @return 効力があった場合TRUEを返す
 * @details
 * <pre>
 *
 * This will turn some walls into floors and some floors into walls.
 *
 * The player will take damage and "jump" into a safe grid if possible,
 * otherwise, he will "tunnel" through the rubble instantaneously.
 *
 * Monsters will take damage, and "jump" into a safe grid if possible,
 * otherwise they will be "buried" in the rubble, disappearing from
 * the level in the same way that they do when genocided.
 *
 * Note that thus the player and monsters (except eaters of walls and
 * passers through walls) will never occupy the same grid as a wall.
 * Note that as of now (2.7.8) no monster may occupy a "wall" grid, even
 * for a single turn, unless that monster can pass_walls or kill_walls.
 * This has allowed massive simplification of the "monster" code.
 * </pre>
 */
bool earthquake(player_type *caster_ptr, POSITION cy, POSITION cx, POSITION r, MONSTER_IDX m_idx)
{
	/* Prevent destruction of quest levels and town */
	floor_type *floor_ptr = caster_ptr->current_floor_ptr;
	if ((floor_ptr->inside_quest && is_fixed_quest_idx(floor_ptr->inside_quest)) || !floor_ptr->dun_level)
	{
		return FALSE;
	}

	/* Paranoia -- Enforce maximum range */
	if (r > 12) r = 12;

	/* Clear the "maximal blast" area */
	POSITION y, x;
	bool map[32][32];
	for (y = 0; y < 32; y++)
	{
		for (x = 0; x < 32; x++)
		{
			map[y][x] = FALSE;
		}
	}

	/* Check around the epicenter */
	POSITION yy, xx, dy, dx;
	int damage = 0;
	bool hurt = FALSE;
	for (dy = -r; dy <= r; dy++)
	{
		for (dx = -r; dx <= r; dx++)
		{
			yy = cy + dy;
			xx = cx + dx;

			if (!in_bounds(floor_ptr, yy, xx)) continue;

			/* Skip distant grids */
			if (distance(cy, cx, yy, xx) > r) continue;
			grid_type *g_ptr;
			g_ptr = &floor_ptr->grid_array[yy][xx];

			/* Lose room and vault / Lose light and knowledge */
			g_ptr->info &= ~(CAVE_ROOM | CAVE_ICKY | CAVE_UNSAFE);
			g_ptr->info &= ~(CAVE_GLOW | CAVE_MARK | CAVE_KNOWN);

			/* Skip the epicenter */
			if (!dx && !dy) continue;

			/* Skip most grids */
			if (randint0(100) < 85) continue;

			/* Damage this grid */
			map[16 + yy - cy][16 + xx - cx] = TRUE;

			/* Hack -- Take note of player damage */
			if (player_bold(caster_ptr, yy, xx)) hurt = TRUE;
		}
	}

	/* First, affect the player (if necessary) */
	int sn = 0;
	POSITION sy = 0, sx = 0;
	if (hurt && !caster_ptr->pass_wall && !caster_ptr->kill_wall)
	{
		/* Check around the player */
		for (DIRECTION i = 0; i < 8; i++)
		{
			y = caster_ptr->y + ddy_ddd[i];
			x = caster_ptr->x + ddx_ddd[i];

			/* Skip non-empty grids */
			if (!is_cave_empty_bold(caster_ptr, y, x)) continue;

			/* Important -- Skip "quake" grids */
			if (map[16 + y - cy][16 + x - cx]) continue;

			if (floor_ptr->grid_array[y][x].m_idx) continue;

			/* Count "safe" grids */
			sn++;

			/* Randomize choice */
			if (randint0(sn) > 0) continue;

			/* Save the safe location */
			sy = y; sx = x;
		}

		/* Random message */
		switch (randint1(3))
		{
		case 1:
		{
			msg_print(_("ダンジョンの壁が崩れた！", "The dungeon's ceiling collapses!"));
			break;
		}
		case 2:
		{
			msg_print(_("ダンジョンの床が不自然にねじ曲がった！", "The dungeon's floor twists in an unnatural way!"));
			break;
		}
		default:
		{
			msg_print(_("ダンジョンが揺れた！崩れた岩が頭に降ってきた！", "The dungeon quakes!  You are pummeled with debris!"));
			break;
		}
		}

		/* Hurt the player a lot */
		if (!sn)
		{
			/* Message and damage */
			msg_print(_("あなたはひどい怪我を負った！", "You are severely crushed!"));
			damage = 200;
		}

		/* Destroy the grid, and push the player to safety */
		else
		{
			/* Calculate results */
			switch (randint1(3))
			{
			case 1:
			{
				msg_print(_("降り注ぐ岩をうまく避けた！", "You nimbly dodge the blast!"));
				damage = 0;
				break;
			}
			case 2:
			{
				msg_print(_("岩石があなたに直撃した!", "You are bashed by rubble!"));
				damage = damroll(10, 4);
				(void)set_stun(caster_ptr, caster_ptr->stun + randint1(50));
				break;
			}
			case 3:
			{
				msg_print(_("あなたは床と壁との間に挟まれてしまった！", "You are crushed between the floor and ceiling!"));
				damage = damroll(10, 4);
				(void)set_stun(caster_ptr, caster_ptr->stun + randint1(50));
				break;
			}
			}

			/* Move the player to the safe location */
			(void)move_player_effect(caster_ptr, sy, sx, MPE_DONT_PICKUP);
		}

		/* Important -- no wall on player */
		map[16 + caster_ptr->y - cy][16 + caster_ptr->x - cx] = FALSE;

		if (damage)
		{
			concptr killer;

			if (m_idx)
			{
				GAME_TEXT m_name[MAX_NLEN];
				monster_type *m_ptr = &floor_ptr->m_list[m_idx];
				monster_desc(caster_ptr, m_name, m_ptr, MD_WRONGDOER_NAME);
				killer = format(_("%sの起こした地震", "an earthquake caused by %s"), m_name);
			}
			else
			{
				killer = _("地震", "an earthquake");
			}

			take_hit(caster_ptr, DAMAGE_ATTACK, damage, killer, -1);
		}
	}

	/* Examine the quaked region */
	for (dy = -r; dy <= r; dy++)
	{
		for (dx = -r; dx <= r; dx++)
		{
			yy = cy + dy;
			xx = cx + dx;

			/* Skip unaffected grids */
			if (!map[16 + yy - cy][16 + xx - cx]) continue;

			grid_type *g_ptr;
			g_ptr = &floor_ptr->grid_array[yy][xx];

			if (g_ptr->m_idx == caster_ptr->riding) continue;

			/* Process monsters */
			if (!g_ptr->m_idx) continue;

			monster_type *m_ptr = &floor_ptr->m_list[g_ptr->m_idx];
			monster_race *r_ptr = &r_info[m_ptr->r_idx];

			/* Quest monsters */
			if (r_ptr->flags1 & RF1_QUESTOR)
			{
				/* No wall on quest monsters */
				map[16 + yy - cy][16 + xx - cx] = FALSE;

				continue;
			}

			/* Most monsters cannot co-exist with rock */
			if ((r_ptr->flags2 & RF2_KILL_WALL) || (r_ptr->flags2 & RF2_PASS_WALL)) continue;

			GAME_TEXT m_name[MAX_NLEN];

			/* Assume not safe */
			sn = 0;

			/* Monster can move to escape the wall */
			if (!(r_ptr->flags1 & RF1_NEVER_MOVE))
			{
				/* Look for safety */
				for (DIRECTION i = 0; i < 8; i++)
				{
					y = yy + ddy_ddd[i];
					x = xx + ddx_ddd[i];

					/* Skip non-empty grids */
					if (!is_cave_empty_bold(caster_ptr, y, x)) continue;

					/* Hack -- no safety on glyph of warding */
					if (is_glyph_grid(&floor_ptr->grid_array[y][x])) continue;
					if (is_explosive_rune_grid(&floor_ptr->grid_array[y][x])) continue;

					/* ... nor on the Pattern */
					if (pattern_tile(floor_ptr, y, x)) continue;

					/* Important -- Skip "quake" grids */
					if (map[16 + y - cy][16 + x - cx]) continue;

					if (floor_ptr->grid_array[y][x].m_idx) continue;
					if (player_bold(caster_ptr, y, x)) continue;

					/* Count "safe" grids */
					sn++;

					/* Randomize choice */
					if (randint0(sn) > 0) continue;

					/* Save the safe grid */
					sy = y; sx = x;
				}
			}

			monster_desc(caster_ptr, m_name, m_ptr, 0);

			/* Scream in pain */
			if (!ignore_unview || is_seen(m_ptr)) msg_format(_("%^sは苦痛で泣きわめいた！", "%^s wails out in pain!"), m_name);

			/* Take damage from the quake */
			damage = (sn ? damroll(4, 8) : (m_ptr->hp + 1));

			/* Monster is certainly awake */
			(void)set_monster_csleep(caster_ptr, g_ptr->m_idx, 0);

			/* Apply damage directly */
			m_ptr->hp -= damage;

			/* Delete (not kill) "dead" monsters */
			if (m_ptr->hp < 0)
			{
				if (!ignore_unview || is_seen(m_ptr))
					msg_format(_("%^sは岩石に埋もれてしまった！", "%^s is embedded in the rock!"), m_name);

				if (g_ptr->m_idx)
				{
					if (record_named_pet && is_pet(&floor_ptr->m_list[g_ptr->m_idx]) && floor_ptr->m_list[g_ptr->m_idx].nickname)
					{
						char m2_name[MAX_NLEN];

						monster_desc(caster_ptr, m2_name, m_ptr, MD_INDEF_VISIBLE);
						exe_write_diary(caster_ptr, DIARY_NAMED_PET, RECORD_NAMED_PET_EARTHQUAKE, m2_name);
					}
				}

				delete_monster(caster_ptr, yy, xx);

				sn = 0;
			}

			/* Hack -- Escape from the rock */
			if (sn == 0) continue;

			IDX m_idx_aux = floor_ptr->grid_array[yy][xx].m_idx;

			/* Update the old location */
			floor_ptr->grid_array[yy][xx].m_idx = 0;

			/* Update the new location */
			floor_ptr->grid_array[sy][sx].m_idx = m_idx_aux;

			/* Move the monster */
			m_ptr->fy = sy;
			m_ptr->fx = sx;

			update_monster(caster_ptr, m_idx, TRUE);
			lite_spot(caster_ptr, yy, xx);
			lite_spot(caster_ptr, sy, sx);
		}
	}

	/* Lose monster light */
	clear_mon_lite(floor_ptr);

	/* Examine the quaked region */
	for (dy = -r; dy <= r; dy++)
	{
		for (dx = -r; dx <= r; dx++)
		{
			yy = cy + dy;
			xx = cx + dx;

			/* Skip unaffected grids */
			if (!map[16 + yy - cy][16 + xx - cx]) continue;

			/* Destroy location (if valid) */
			if (!cave_valid_bold(floor_ptr, yy, xx)) continue;

			delete_object(caster_ptr, yy, xx);

			/* Wall (or floor) type */
			int t = cave_have_flag_bold(floor_ptr, yy, xx, FF_PROJECT) ? randint0(100) : 200;

			/* Create granite wall */
			if (t < 20)
			{
				cave_set_feat(caster_ptr, yy, xx, feat_granite);
				continue;
			}

			/* Create quartz vein */
			if (t < 70)
			{
				cave_set_feat(caster_ptr, yy, xx, feat_quartz_vein);
				continue;
			}

			/* Create magma vein */
			if (t < 100)
			{
				cave_set_feat(caster_ptr, yy, xx, feat_magma_vein);
				continue;
			}

			/* Create floor */
			cave_set_feat(caster_ptr, yy, xx, feat_ground_type[randint0(100)]);
		}
	}

	/* Process "re-glowing" */
	for (dy = -r; dy <= r; dy++)
	{
		for (dx = -r; dx <= r; dx++)
		{
			yy = cy + dy;
			xx = cx + dx;

			if (!in_bounds(floor_ptr, yy, xx)) continue;

			/* Skip distant grids */
			if (distance(cy, cx, yy, xx) > r) continue;
			grid_type *g_ptr;
			g_ptr = &floor_ptr->grid_array[yy][xx];

			if (is_mirror_grid(g_ptr))
			{
				g_ptr->info |= CAVE_GLOW;
				continue;
			}
			
			if ((d_info[caster_ptr->dungeon_idx].flags1 & DF1_DARKNESS)) continue;
			
			DIRECTION ii;
			POSITION yyy, xxx;
			grid_type *cc_ptr;

			for (ii = 0; ii < 9; ii++)
			{
				yyy = yy + ddy_ddd[ii];
				xxx = xx + ddx_ddd[ii];
				if (!in_bounds2(floor_ptr, yyy, xxx)) continue;
				cc_ptr = &floor_ptr->grid_array[yyy][xxx];
				if (have_flag(f_info[get_feat_mimic(cc_ptr)].flags, FF_GLOW))
				{
					g_ptr->info |= CAVE_GLOW;
					break;
				}
			}
		}
	}

	/* Mega-Hack -- Forget the view and lite */
	caster_ptr->update |= (PU_UN_VIEW | PU_UN_LITE | PU_VIEW | PU_LITE | PU_FLOW | PU_MON_LITE | PU_MONSTERS);
	caster_ptr->redraw |= (PR_HEALTH | PR_UHEALTH | PR_MAP);
	caster_ptr->window |= (PW_OVERHEAD | PW_DUNGEON);

	if (caster_ptr->special_defense & NINJA_S_STEALTH)
	{
		if (floor_ptr->grid_array[caster_ptr->y][caster_ptr->x].info & CAVE_GLOW) set_superstealth(caster_ptr, FALSE);
	}

	/* Success */
	return TRUE;
}
