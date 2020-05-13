﻿#include "angband.h"
#include "util.h"
#include "main/music-definitions-table.h"

#include "core/system-variables.h"
#include "dungeon/dungeon.h"
#include "floor.h"
#include "floor-save.h"
#include "floor-events.h"
#include "grid.h"
#include "quest.h"
#include "monsterrace-hook.h"
#include "monster.h"
#include "player-status.h"
#include "player-personality.h"
#include "artifact.h"
#include "feature.h"
#include "world/world.h"
#include "io/write-diary.h"
#include "cmd/cmd-dump.h"
#include "english.h"
#include "view/display-main-window.h"

quest_type *quest; /*!< Quest info */
QUEST_IDX max_q_idx; /*!< Maximum number of quests */
char quest_text[10][80]; /*!< Quest text */
int quest_text_line; /*!< Current line of the quest text */
int leaving_quest = 0;

/*!
 * @brief クエスト突入時のメッセージテーブル / Array of places to find an inscription
 */
static concptr find_quest[] =
{
	_("床にメッセージが刻まれている:", "You find the following inscription in the floor"),
	_("壁にメッセージが刻まれている:", "You see a message inscribed in the wall"),
	_("メッセージを見つけた:", "There is a sign saying"),
	_("何かが階段の上に書いてある:", "Something is written on the staircase"),
	_("巻物を見つけた。メッセージが書いてある:", "You find a scroll with the following message"),
};


/*!
 * @brief ランダムクエストの討伐ユニークを決める / Determine the random quest uniques
 * @param q_ptr クエスト構造体の参照ポインタ
 * @return なし
 */
void determine_random_questor(player_type *player_ptr, quest_type *q_ptr)
{
	get_mon_num_prep(player_ptr, mon_hook_quest, NULL);

	MONRACE_IDX r_idx;
	while (TRUE)
	{
		/*
		 * Random monster 5 - 10 levels out of depth
		 * (depending on level)
		 */
		r_idx = get_mon_num(player_ptr, q_ptr->level + 5 + randint1(q_ptr->level / 10), GMN_ARENA);
		monster_race *r_ptr;
		r_ptr = &r_info[r_idx];

		if (!(r_ptr->flags1 & RF1_UNIQUE)) continue;
		if (r_ptr->flags1 & RF1_QUESTOR) continue;
		if (r_ptr->rarity > 100) continue;
		if (r_ptr->flags7 & RF7_FRIENDLY) continue;
		if (r_ptr->flags7 & RF7_AQUATIC) continue;
		if (r_ptr->flags8 & RF8_WILD_ONLY) continue;
		if (no_questor_or_bounty_uniques(r_idx)) continue;

		/*
		 * Accept monsters that are 2 - 6 levels
		 * out of depth depending on the quest level
		 */
		if (r_ptr->level > (q_ptr->level + (q_ptr->level / 20))) break;
	}

	q_ptr->r_idx = r_idx;
}


/*!
 * @brief クエストを達成状態にする /
 * @param player_ptr プレーヤーへの参照ポインタ
 * @param quest_num 達成状態にしたいクエストのID
 * @return なし
 */
void complete_quest(player_type *player_ptr, QUEST_IDX quest_num)
{
	quest_type* const q_ptr = &quest[quest_num];

	switch (q_ptr->type)
	{
	case QUEST_TYPE_RANDOM:
		if (record_rand_quest) exe_write_diary(player_ptr, DIARY_RAND_QUEST_C, quest_num, NULL);
		break;
	default:
		if (record_fix_quest) exe_write_diary(player_ptr, DIARY_FIX_QUEST_C, quest_num, NULL);
		break;
	}

	q_ptr->status = QUEST_STATUS_COMPLETED;
	q_ptr->complev = player_ptr->lev;
	update_playtime();
	q_ptr->comptime = current_world_ptr->play_time;

	if (q_ptr->flags & QUEST_FLAG_SILENT) return;

	play_music(TERM_XTRA_MUSIC_BASIC, MUSIC_BASIC_QUEST_CLEAR);
	msg_print(_("クエストを達成した！", "You just completed your quest!"));
	msg_print(NULL);
}


/*!
 * @brief 特定の敵を倒した際にクエスト達成処理 /
 * Check for "Quest" completion when a quest monster is killed or charmed.
 * @param player_ptr プレーヤーへの参照ポインタ
 * @param m_ptr 撃破したモンスターの構造体参照ポインタ
 * @return なし
 */
void check_quest_completion(player_type *player_ptr, monster_type *m_ptr)
{
	POSITION y = m_ptr->fy;
	POSITION x = m_ptr->fx;

	floor_type *floor_ptr = player_ptr->current_floor_ptr;
	QUEST_IDX quest_num = floor_ptr->inside_quest;

	/* Search for an active quest on this dungeon level */
	if (!quest_num)
	{
		QUEST_IDX i;
		for (i = max_q_idx - 1; i > 0; i--)
		{
			quest_type* const q_ptr = &quest[i];

			/* Quest is not active */
			if (q_ptr->status != QUEST_STATUS_TAKEN)
				continue;

			/* Quest is not a dungeon quest */
			if (q_ptr->flags & QUEST_FLAG_PRESET)
				continue;

			/* Quest is not on this level */
			if ((q_ptr->level != floor_ptr->dun_level) &&
				(q_ptr->type != QUEST_TYPE_KILL_ANY_LEVEL))
				continue;

			/* Not a "kill monster" quest */
			if ((q_ptr->type == QUEST_TYPE_FIND_ARTIFACT) ||
				(q_ptr->type == QUEST_TYPE_FIND_EXIT))
				continue;

			/* Interesting quest */
			if ((q_ptr->type == QUEST_TYPE_KILL_NUMBER) ||
				(q_ptr->type == QUEST_TYPE_TOWER) ||
				(q_ptr->type == QUEST_TYPE_KILL_ALL))
				break;

			/* Interesting quest */
			if (((q_ptr->type == QUEST_TYPE_KILL_LEVEL) ||
				(q_ptr->type == QUEST_TYPE_KILL_ANY_LEVEL) ||
				(q_ptr->type == QUEST_TYPE_RANDOM)) &&
				(q_ptr->r_idx == m_ptr->r_idx))
				break;
		}

		quest_num = i;
	}

	/* Handle the current quest */
	bool create_stairs = FALSE;
	bool reward = FALSE;
	if (quest_num && (quest[quest_num].status == QUEST_STATUS_TAKEN))
	{
		/* Current quest */
		quest_type* const q_ptr = &quest[quest_num];

		switch (q_ptr->type)
		{
		case QUEST_TYPE_KILL_NUMBER:
		{
			q_ptr->cur_num++;

			if (q_ptr->cur_num >= q_ptr->num_mon)
			{
				complete_quest(player_ptr, quest_num);
				q_ptr->cur_num = 0;
			}

			break;
		}
		case QUEST_TYPE_KILL_ALL:
		{
			if (!is_hostile(m_ptr)) break;

			if (count_all_hostile_monsters(floor_ptr) != 1) break;

			if (q_ptr->flags & QUEST_FLAG_SILENT)
			{
				q_ptr->status = QUEST_STATUS_FINISHED;
			}
			else
			{
				complete_quest(player_ptr, quest_num);
			}

			break;
		}
		case QUEST_TYPE_KILL_LEVEL:
		case QUEST_TYPE_RANDOM:
		{
			/* Only count valid monsters */
			if (q_ptr->r_idx != m_ptr->r_idx)
				break;

			q_ptr->cur_num++;

			if (q_ptr->cur_num < q_ptr->max_num) break;

			complete_quest(player_ptr, quest_num);

			if (!(q_ptr->flags & QUEST_FLAG_PRESET))
			{
				create_stairs = TRUE;
				floor_ptr->inside_quest = 0;
			}

			/* Finish the two main quests without rewarding */
			if ((quest_num == QUEST_OBERON) || (quest_num == QUEST_SERPENT))
			{
				q_ptr->status = QUEST_STATUS_FINISHED;
			}

			if (q_ptr->type == QUEST_TYPE_RANDOM)
			{
				reward = TRUE;
				q_ptr->status = QUEST_STATUS_FINISHED;
			}

			break;
		}
		case QUEST_TYPE_KILL_ANY_LEVEL:
		{
			q_ptr->cur_num++;
			if (q_ptr->cur_num >= q_ptr->max_num)
			{
				complete_quest(player_ptr, quest_num);
				q_ptr->cur_num = 0;
			}

			break;
		}
		case QUEST_TYPE_TOWER:
		{
			if (!is_hostile(m_ptr)) break;

			if (count_all_hostile_monsters(floor_ptr) == 1)
			{
				q_ptr->status = QUEST_STATUS_STAGE_COMPLETED;

				if ((quest[QUEST_TOWER1].status == QUEST_STATUS_STAGE_COMPLETED) &&
					(quest[QUEST_TOWER2].status == QUEST_STATUS_STAGE_COMPLETED) &&
					(quest[QUEST_TOWER3].status == QUEST_STATUS_STAGE_COMPLETED))
				{

					complete_quest(player_ptr, QUEST_TOWER1);
				}
			}

			break;
		}
		}
	}

	/* Create a magical staircase */
	if (create_stairs)
	{
		POSITION ny, nx;

		/* Stagger around */
		while (cave_perma_bold(floor_ptr, y, x) || floor_ptr->grid_array[y][x].o_idx || (floor_ptr->grid_array[y][x].info & CAVE_OBJECT))
		{
			/* Pick a location */
			scatter(player_ptr, &ny, &nx, y, x, 1, 0);

			/* Stagger */
			y = ny; x = nx;
		}

		/* Explain the staircase */
		msg_print(_("魔法の階段が現れた...", "A magical staircase appears..."));

		/* Create stairs down */
		cave_set_feat(player_ptr, y, x, feat_down_stair);

		/* Remember to update everything */
		player_ptr->update |= (PU_FLOW);
	}

	if (!reward) return;

	object_type forge;
	object_type *o_ptr;
	for (int i = 0; i < (floor_ptr->dun_level / 15) + 1; i++)
	{
		o_ptr = &forge;
		object_wipe(o_ptr);

		/* Make a great object */
		make_object(player_ptr, o_ptr, AM_GOOD | AM_GREAT);
		(void)drop_near(player_ptr, o_ptr, -1, y, x);
	}
}


/*!
 * @brief 特定のアーティファクトを入手した際のクエスト達成処理 /
 * Check for "Quest" completion when a quest monster is killed or charmed.
 * @param player_ptr プレーヤーへの参照ポインタ
 * @param o_ptr 入手したオブジェクトの構造体参照ポインタ
 * @return なし
 */
void check_find_art_quest_completion(player_type *player_ptr, object_type *o_ptr)
{
	/* Check if completed a quest */
	for (QUEST_IDX i = 0; i < max_q_idx; i++)
	{
		if ((quest[i].type == QUEST_TYPE_FIND_ARTIFACT) &&
			(quest[i].status == QUEST_STATUS_TAKEN) &&
			(quest[i].k_idx == o_ptr->name1))
		{
			complete_quest(player_ptr, i);
		}
	}
}


/*!
 * @brief クエストの導入メッセージを表示する / Discover quest
 * @param q_idx 開始されたクエストのID
 */
void quest_discovery(QUEST_IDX q_idx)
{
	quest_type *q_ptr = &quest[q_idx];
	monster_race *r_ptr = &r_info[q_ptr->r_idx];
	MONSTER_NUMBER q_num = q_ptr->max_num;

	if (!q_idx) return;

	GAME_TEXT name[MAX_NLEN];
	strcpy(name, (r_name + r_ptr->name));

	msg_print(find_quest[rand_range(0, 4)]);
	msg_print(NULL);

	if (q_num != 1)
	{
#ifdef JP
#else
		plural_aux(name);
#endif
		msg_format(_("注意しろ！この階は%d体の%sによって守られている！", "Be warned, this level is guarded by %d %s!"), q_num, name);
		return;
	}

	bool is_random_quest_skipped = (r_ptr->flags1 & RF1_UNIQUE) != 0;
	is_random_quest_skipped &= r_ptr->max_num == 0;
	if (!is_random_quest_skipped)
	{
		msg_format(_("注意せよ！この階は%sによって守られている！", "Beware, this level is protected by %s!"), name);
		return;
	}

	msg_print(_("この階は以前は誰かによって守られていたようだ…。", "It seems that this level was protected by someone before..."));
	quest[q_idx].status = QUEST_STATUS_FINISHED;
	q_ptr->complev = 0;
	update_playtime();
	q_ptr->comptime = current_world_ptr->play_time;
}


/*!
 * @brief 新しく入ったダンジョンの階層に固定されている一般のクエストを探し出しIDを返す。
 * / Hack -- Check if a level is a "quest" level
 * @param player_ptr プレーヤーへの参照ポインタ
 * @param level 検索対象になる階
 * @return クエストIDを返す。該当がない場合0を返す。
 */
QUEST_IDX quest_number(player_type *player_ptr, DEPTH level)
{
	floor_type *floor_ptr = player_ptr->current_floor_ptr;
	if (floor_ptr->inside_quest)
		return (floor_ptr->inside_quest);

	for (QUEST_IDX i = 0; i < max_q_idx; i++)
	{
		if (quest[i].status != QUEST_STATUS_TAKEN) continue;

		if ((quest[i].type == QUEST_TYPE_KILL_LEVEL) &&
			!(quest[i].flags & QUEST_FLAG_PRESET) &&
			(quest[i].level == level) &&
			(quest[i].dungeon == player_ptr->dungeon_idx))
			return i;
	}

	return random_quest_number(player_ptr, level);
}


/*!
 * @brief 新しく入ったダンジョンの階層に固定されているランダムクエストを探し出しIDを返す。
 * @param player_ptr プレーヤーへの参照ポインタ
 * @param level 検索対象になる階
 * @return クエストIDを返す。該当がない場合0を返す。
 */
QUEST_IDX random_quest_number(player_type *player_ptr, DEPTH level)
{
	if (player_ptr->dungeon_idx != DUNGEON_ANGBAND) return 0;

	for (QUEST_IDX i = MIN_RANDOM_QUEST; i < MAX_RANDOM_QUEST + 1; i++)
	{
		if ((quest[i].type == QUEST_TYPE_RANDOM) &&
			(quest[i].status == QUEST_STATUS_TAKEN) &&
			(quest[i].level == level) &&
			(quest[i].dungeon == DUNGEON_ANGBAND))
		{
			return i;
		}
	}

	return 0;
}


/*!
 * @brief クエスト階層から離脱する際の処理
 * @param player_ptr プレーヤーへの参照ポインタ
 * @return なし
 */
void leave_quest_check(player_type *player_ptr)
{
	leaving_quest = player_ptr->current_floor_ptr->inside_quest;
	if (!leaving_quest) return;

	quest_type* const q_ptr = &quest[leaving_quest];
	bool is_one_time_quest = ((q_ptr->flags & QUEST_FLAG_ONCE) || (q_ptr->type == QUEST_TYPE_RANDOM)) &&
		(q_ptr->status == QUEST_STATUS_TAKEN);
	if (!is_one_time_quest) return;

	q_ptr->status = QUEST_STATUS_FAILED;
	q_ptr->complev = player_ptr->lev;
	update_playtime();
	q_ptr->comptime = current_world_ptr->play_time;

	/* Additional settings */
	switch (q_ptr->type)
	{
	case QUEST_TYPE_TOWER:
		quest[QUEST_TOWER1].status = QUEST_STATUS_FAILED;
		quest[QUEST_TOWER1].complev = player_ptr->lev;
		break;
	case QUEST_TYPE_FIND_ARTIFACT:
		a_info[q_ptr->k_idx].gen_flags &= ~(TRG_QUESTITEM);
		break;
	case QUEST_TYPE_RANDOM:
		r_info[q_ptr->r_idx].flags1 &= ~(RF1_QUESTOR);

		/* Floor of random quest will be blocked */
		prepare_change_floor_mode(player_ptr, CFM_NO_RETURN);
		break;
	}

	/* Record finishing a quest */
	if (q_ptr->type == QUEST_TYPE_RANDOM)
	{
		if (record_rand_quest)
			exe_write_diary(player_ptr, DIARY_RAND_QUEST_F, leaving_quest, NULL);
		return;
	}

	if (record_fix_quest)
		exe_write_diary(player_ptr, DIARY_FIX_QUEST_F, leaving_quest, NULL);
}


/*!
 * @brief 「塔」クエストの各階層から離脱する際の処理
 * @return なし
 */
void leave_tower_check(player_type *player_ptr)
{
	leaving_quest = player_ptr->current_floor_ptr->inside_quest;
	bool is_leaving_from_tower = leaving_quest != 0;
	is_leaving_from_tower &= quest[leaving_quest].type == QUEST_TYPE_TOWER;
	is_leaving_from_tower &= quest[QUEST_TOWER1].status != QUEST_STATUS_COMPLETED;
	if (!is_leaving_from_tower) return;
	if (quest[leaving_quest].type != QUEST_TYPE_TOWER) return;

	quest[QUEST_TOWER1].status = QUEST_STATUS_FAILED;
	quest[QUEST_TOWER1].complev = player_ptr->lev;
	update_playtime();
	quest[QUEST_TOWER1].comptime = current_world_ptr->play_time;
}


/*!
 * @brief クエスト入り口にプレイヤーが乗った際の処理 / Do building commands
 * @param player_ptr プレーヤーへの参照ポインタ
 * @return なし
 */
void do_cmd_quest(player_type *player_ptr)
{
	if (player_ptr->wild_mode) return;

	take_turn(player_ptr, 100);

	if (!cave_have_flag_bold(player_ptr->current_floor_ptr, player_ptr->y, player_ptr->x, FF_QUEST_ENTER))
	{
		msg_print(_("ここにはクエストの入口はない。", "You see no quest level here."));
		return;
	}

	msg_print(_("ここにはクエストへの入口があります。", "There is an entry of a quest."));
	if (!get_check(_("クエストに入りますか？", "Do you enter? "))) return;
	if (IS_ECHIZEN(player_ptr))
		msg_print(_("『とにかく入ってみようぜぇ。』", ""));
	else if (player_ptr->pseikaku == SEIKAKU_CHARGEMAN) msg_print("『全滅してやるぞ！』");

	/* Player enters a new quest */
	player_ptr->oldpy = 0;
	player_ptr->oldpx = 0;

	leave_quest_check(player_ptr);

	if (quest[player_ptr->current_floor_ptr->inside_quest].type != QUEST_TYPE_RANDOM) player_ptr->current_floor_ptr->dun_level = 1;
	player_ptr->current_floor_ptr->inside_quest = player_ptr->current_floor_ptr->grid_array[player_ptr->y][player_ptr->x].special;

	player_ptr->leaving = TRUE;
}
