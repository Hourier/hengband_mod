﻿/*!
 * @file spells1.c
 * @brief 魔法による遠隔処理の実装 / Spell projection
 * @date 2014/07/10
 * @author
 * <pre>
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 * </pre>
 */

#include "angband.h"
#include "core.h"
#include "util.h"
#include "main/sound-definitions-table.h"

#include "io/write-diary.h"
#include "cmd/cmd-pet.h"
#include "cmd/cmd-dump.h"
#include "floor.h"
#include "object-curse.h"
#include "player-damage.h"
#include "player-effects.h"
#include "player/mimic-info-table.h"
#include "player-class.h"

#include "monster.h"
#include "monster-status.h"
#include "monster-spell.h"
#include "spells.h"
#include "spells-status.h"
#include "spells-diceroll.h"
#include "spells-summon.h"
#include "monsterrace-hook.h"

#include "melee.h"
#include "world.h"
#include "mutation.h"
#include "artifact.h"
#include "avatar.h"
#include "player-status.h"
#include "player-move.h"
#include "object-broken.h"
#include "quest.h"
#include "gameterm.h"
#include "grid.h"
#include "feature.h"
#include "view/display-main-window.h"

#include "realm.h"
#include "realm-arcane.h"
#include "realm-chaos.h"
#include "realm-craft.h"
#include "realm-crusade.h"
#include "realm-daemon.h"
#include "realm-death.h"
#include "realm-hex.h"
#include "realm-hissatsu.h"
#include "realm-life.h"
#include "realm-nature.h"
#include "realm-song.h"
#include "realm-sorcery.h"
#include "realm-trump.h"

#include "effect/effect-feature.h"
#include "effect/effect-item.h"


static int rakubadam_m; /*!< 振り落とされた際のダメージ量 */
static int rakubadam_p; /*!< 落馬した際のダメージ量 */
bool sukekaku;

int project_length = 0; /*!< 投射の射程距離 */

int cap_mon;
int cap_mspeed;
HIT_POINT cap_hp;
HIT_POINT cap_maxhp;
STR_OFFSET cap_nickname;

/*!
 * @brief 歌、剣術、呪術領域情報テーブル
 */
const magic_type technic_info[NUM_TECHNIC][32] =
{
	{
		/* Music */
		{ 1,  1,  10,   2},
		{ 2,  1,  10,   2},
		{ 3,  2,  20,   3},
		{ 4,  2,  20,   4},
		{ 5,  2,  20,   6},
		{ 7,  4,  30,   8},
		{ 9,  3,  30,   10},
		{ 10, 2,  30,   12},

		{ 12,  3,   40,   20},
		{ 15, 16,  42,   35},
		{ 17, 18,  40,   25},
		{ 18,  2,  45,   30},
		{ 23,  8,  50,   38},
		{ 28, 30,  50,   41},
		{ 33, 35,  60,   42},
		{ 38, 35,  70,   46},

		{ 10,  4,  20,   13},
		{ 22,  5,  30,   26},
		{ 23,  3,  35,   27},
		{ 26,  28,  37,   29},
		{ 32,  37,  41,   36},
		{ 33,  22,  43,   40},
		{ 37,  35,  46,   42},
		{ 45,  60,  50,   56},

		{ 23,  18,  20,   23},
		{ 30,  30,  30,   26},
		{ 33,  65,  41,   30},
		{ 37,  35,  43,   35},
		{ 40,  30,  46,   50},
		{ 42,  75,  50,   68},
		{ 45,  58,  62,   73},
		{ 49,  48,  70,  200}
	},

	{
		/* Hissatsu */
		{ 1,   15,   0,   0},
		{ 3,   10,   0,   0},
		{ 6,   15,   0,   0},
		{ 9,    8,   0,   0},
		{ 10,  12,   0,   0},
		{ 12,  25,   0,   0},
		{ 14,   7,   0,   0},
		{ 17,  20,   0,   0},

		{ 19,  10,   0,   0},
		{ 22,  20,   0,   0},
		{ 24,  30,   0,   0},
		{ 25,  10,   0,   0},
		{ 27,  15,   0,   0},
		{ 29,  45,   0,   0},
		{ 32,  70,   0,   0},
		{ 35,  50,   0,   0},

		{ 18,  40,   0,   0},
		{ 22,  22,   0,   0},
		{ 24,  30,   0,   0},
		{ 26,  35,   0,   0},
		{ 30,  30,   0,   0},
		{ 32,  60,   0,   0},
		{ 36,  40,   0,   0},
		{ 39,  80,   0,   0},

		{ 26,  20,   0,   0},
		{ 29,  40,   0,   0},
		{ 31,  35,   0,   0},
		{ 36,  80,   0,   0},
		{ 39, 100,   0,   0},
		{ 42, 110,   0,   0},
		{ 45, 130,   0,   0},
		{ 50, 255,   0,   0}
	},

	{
		/* Hex */
		{  1,  2, 20,   2},
		{  1,  2, 20,   2},
		{  3,  2, 30,   3},
		{  5,  3, 30,   4},
		{  7,  3, 40,   6},
		{  8, 10, 60,   8},
		{  9,  3, 30,  10},
		{ 10,  5, 40,  12},

		{ 12,  8, 40,  15},
		{ 12,  9, 35,  15},
		{ 15, 10, 50,  20},
		{ 20, 12, 45,  35},
		{ 25, 15, 50,  50},
		{ 30, 12, 60,  70},
		{ 35, 10, 60,  80},
		{ 40, 16, 70, 100},

		{ 15,  8, 20,  20},
		{ 18, 15, 50,  20},
		{ 22, 10, 65,  35},
		{ 25, 28, 70,  50},
		{ 28, 10, 70,  60},
		{ 30, 20, 60,  60},
		{ 36, 22, 70,  80},
		{ 40, 28, 70, 100},

		{  5,  6, 35,   5},
		{ 22, 24, 70,  40},
		{ 25,  2, 65,  50},
		{ 32, 20, 50,  70},
		{ 35, 35, 70,  80},
		{ 38, 32, 70,  90},
		{ 42, 24, 70, 120},
		{ 46, 45, 80, 200}
	},
};


/*!
 * @brief 魔法処理のメインルーチン
 * @param realm 魔法領域のID
 * @param spell 各領域の魔法ID
 * @param mode 求める処理
 * @return 各領域魔法に各種テキストを求めた場合は文字列参照ポインタ、そうでない場合はNULLポインタを返す。
 */
concptr exe_spell(player_type *caster_ptr, REALM_IDX realm, SPELL_IDX spell, BIT_FLAGS mode)
{
	switch (realm)
	{
	case REALM_LIFE:     return do_life_spell(caster_ptr, spell, mode);
	case REALM_SORCERY:  return do_sorcery_spell(caster_ptr, spell, mode);
	case REALM_NATURE:   return do_nature_spell(caster_ptr, spell, mode);
	case REALM_CHAOS:    return do_chaos_spell(caster_ptr, spell, mode);
	case REALM_DEATH:    return do_death_spell(caster_ptr, spell, mode);
	case REALM_TRUMP:    return do_trump_spell(caster_ptr, spell, mode);
	case REALM_ARCANE:   return do_arcane_spell(caster_ptr, spell, mode);
	case REALM_CRAFT:    return do_craft_spell(caster_ptr, spell, mode);
	case REALM_DAEMON:   return do_daemon_spell(caster_ptr, spell, mode);
	case REALM_CRUSADE:  return do_crusade_spell(caster_ptr, spell, mode);
	case REALM_MUSIC:    return do_music_spell(caster_ptr, spell, mode);
	case REALM_HISSATSU: return do_hissatsu_spell(caster_ptr, spell, mode);
	case REALM_HEX:      return do_hex_spell(caster_ptr, spell, mode);
	}

	return NULL;
}

/*!
 * @brief 配置した鏡リストの次を取得する /
 * Get another mirror. for SEEKER
 * @param next_y 次の鏡のy座標を返す参照ポインタ
 * @param next_x 次の鏡のx座標を返す参照ポインタ
 * @param cury 現在の鏡のy座標
 * @param curx 現在の鏡のx座標
 */
static void next_mirror(player_type *creature_ptr, POSITION* next_y, POSITION* next_x, POSITION cury, POSITION curx)
{
	POSITION mirror_x[10], mirror_y[10]; /* 鏡はもっと少ない */
	int mirror_num = 0;			  /* 鏡の数 */
	POSITION x, y;
	int num;

	for (x = 0; x < creature_ptr->current_floor_ptr->width; x++)
	{
		for (y = 0; y < creature_ptr->current_floor_ptr->height; y++)
		{
			if (is_mirror_grid(&creature_ptr->current_floor_ptr->grid_array[y][x])) {
				mirror_y[mirror_num] = y;
				mirror_x[mirror_num] = x;
				mirror_num++;
			}
		}
	}
	if (mirror_num)
	{
		num = randint0(mirror_num);
		*next_y = mirror_y[num];
		*next_x = mirror_x[num];
		return;
	}
	*next_y = cury + randint0(5) - 2;
	*next_x = curx + randint0(5) - 2;
	return;
}


static int project_m_n; /*!< 魔法効果範囲内にいるモンスターの数 */
static POSITION project_m_x; /*!< 処理中のモンスターX座標 */
static POSITION project_m_y; /*!< 処理中のモンスターY座標 */
static POSITION monster_target_x; /*!< モンスターの攻撃目標X座標 */
static POSITION monster_target_y; /*!< モンスターの攻撃目標Y座標 */


/*!
 * @brief 汎用的なビーム/ボルト/ボール系によるモンスターへの効果処理 / Handle a beam/bolt/ball causing damage to a monster.
 * @param caster_ptr プレーヤーへの参照ポインタ
 * @param who 魔法を発動したモンスター(0ならばプレイヤー) / Index of "source" monster (zero for "player")
 * @param r 効果半径(ビーム/ボルト = 0 / ボール = 1以上) / Radius of explosion (0 = beam/bolt, 1 to 9 = ball)
 * @param y 目標Y座標 / Target y location (or location to travel "towards")
 * @param x 目標X座標 / Target x location (or location to travel "towards")
 * @param dam 基本威力 / Base damage roll to apply to affected monsters (or player)
 * @param typ 効果属性 / Type of damage to apply to monsters (and objects)
 * @param flg 効果フラグ
 * @param see_s_msg TRUEならばメッセージを表示する
 * @return 何か一つでも効力があればTRUEを返す / TRUE if any "effects" of the projection were observed, else FALSE
 * @details
 * <pre>
 * This routine takes a "source monster" (by index) which is mostly used to
 * determine if the player is causing the damage, and a "radius" (see below),
 * which is used to decrease the power of explosions with distance, and a
 * location, via integers which are modified by certain types of attacks
 * (polymorph and teleport being the obvious ones), a default damage, which
 * is modified as needed based on various properties, and finally a "damage
 * type" (see below).
 * </pre>
 * <pre>
 * Note that this routine can handle "no damage" attacks (like teleport) by
 * taking a "zero" damage, and can even take "parameters" to attacks (like
 * confuse) by accepting a "damage", using it to calculate the effect, and
 * then setting the damage to zero.  Note that the "damage" parameter is
 * divided by the radius, so monsters not at the "epicenter" will not take
 * as much damage (or whatever)...
 * </pre>
 * <pre>
 * Note that "polymorph" is dangerous, since a failure in "place_monster()"'
 * may result in a dereference of an invalid pointer.
 * </pre>
 * <pre>
 * Various messages are produced, and damage is applied.
 * </pre>
 * <pre>
 * Just "casting" a substance (i.e. plasma) does not make you immune, you must
 * actually be "made" of that substance, or "breathe" big balls of it.
 * We assume that "Plasma" monsters, and "Plasma" breathers, are immune
 * to plasma.
 * We assume "Nether" is an evil, necromantic force, so it doesn't hurt undead,
 * and hurts evil less.  If can breath nether, then it resists it as well.
 * </pre>
 * <pre>
 * Damage reductions use the following formulas:
 *   Note that "dam = dam * 6 / (randint1(6) + 6);"
 *	 gives avg damage of .655, ranging from .858 to .500
 *   Note that "dam = dam * 5 / (randint1(6) + 6);"
 *	 gives avg damage of .544, ranging from .714 to .417
 *   Note that "dam = dam * 4 / (randint1(6) + 6);"
 *	 gives avg damage of .444, ranging from .556 to .333
 *   Note that "dam = dam * 3 / (randint1(6) + 6);"
 *	 gives avg damage of .327, ranging from .427 to .250
 *   Note that "dam = dam * 2 / (randint1(6) + 6);"
 *	 gives something simple.
 * </pre>
 * <pre>
 * In this function, "result" messages are postponed until the end, where
 * the "note" string is appended to the monster name, if not NULL.  So,
 * to make a spell have "no effect" just set "note" to NULL.  You should
 * also set "notice" to FALSE, or the player will learn what the spell does.
 * </pre>
 * <pre>
 * We attempt to return "TRUE" if the player saw anything "useful" happen.
 * "flg" was added.
 * </pre>
 */
static bool project_m(player_type *caster_ptr, MONSTER_IDX who, POSITION r, POSITION y, POSITION x, HIT_POINT dam, EFFECT_ID typ, BIT_FLAGS flg, bool see_s_msg)
{
	int tmp;

	floor_type *floor_ptr = caster_ptr->current_floor_ptr;
	grid_type *g_ptr = &floor_ptr->grid_array[y][x];

	monster_type *m_ptr = &floor_ptr->m_list[g_ptr->m_idx];
	monster_type *m_caster_ptr = (who > 0) ? &floor_ptr->m_list[who] : NULL;

	monster_race *r_ptr = &r_info[m_ptr->r_idx];

	char killer[80];

	bool seen = m_ptr->ml;
	bool seen_msg = is_seen(m_ptr);
	bool slept = (bool)MON_CSLEEP(m_ptr);
	bool obvious = FALSE;
	bool known = ((m_ptr->cdis <= MAX_SIGHT) || caster_ptr->phase_out);
	bool skipped = FALSE;
	bool get_angry = FALSE;
	bool do_poly = FALSE;
	int do_dist = 0;
	int do_conf = 0;
	int do_stun = 0;
	int do_sleep = 0;
	int do_fear = 0;
	int do_time = 0;
	bool heal_leper = FALSE;
	GAME_TEXT m_name[MAX_NLEN];
	char m_poss[10];
	PARAMETER_VALUE photo = 0;
	concptr note = NULL;
	concptr note_dies = extract_note_dies(real_r_idx(m_ptr));
	DEPTH caster_lev = (who > 0) ? r_info[m_caster_ptr->r_idx].level : (caster_ptr->lev * 2);

	if (!g_ptr->m_idx) return FALSE;

	/* Never affect projector */
	if (who && (g_ptr->m_idx == who)) return FALSE;
	if ((g_ptr->m_idx == caster_ptr->riding) && !who && !(typ == GF_OLD_HEAL) && !(typ == GF_OLD_SPEED) && !(typ == GF_STAR_HEAL)) return FALSE;
	if (sukekaku && ((m_ptr->r_idx == MON_SUKE) || (m_ptr->r_idx == MON_KAKU))) return FALSE;

	/* Don't affect already death monsters */
	/* Prevents problems with chain reactions of exploding monsters */
	if (m_ptr->hp < 0) return FALSE;

	dam = (dam + r) / (r + 1);

	/* Get the monster name (BEFORE polymorphing) */
	monster_desc(caster_ptr, m_name, m_ptr, 0);

	/* Get the monster possessive ("his"/"her"/"its") */
	monster_desc(caster_ptr, m_poss, m_ptr, MD_PRON_VISIBLE | MD_POSSESSIVE);

	if (caster_ptr->riding && (g_ptr->m_idx == caster_ptr->riding)) disturb(caster_ptr, TRUE, TRUE);

	if (r_ptr->flagsr & RFR_RES_ALL &&
		typ != GF_OLD_CLONE && typ != GF_STAR_HEAL && typ != GF_OLD_HEAL
		&& typ != GF_OLD_SPEED && typ != GF_CAPTURE && typ != GF_PHOTO)
	{
		note = _("には完全な耐性がある！", " is immune.");
		dam = 0;
		if (is_original_ap_and_seen(caster_ptr, m_ptr)) r_ptr->r_flagsr |= (RFR_RES_ALL);
		if (typ == GF_LITE_WEAK || typ == GF_KILL_WALL) skipped = TRUE;
	}
	else
	{
		switch (typ)
		{
		case GF_MISSILE:
		{
			if (seen) obvious = TRUE;
			break;
		}
		case GF_ACID:
		{
			if (seen) obvious = TRUE;
			if (r_ptr->flagsr & RFR_IM_ACID)
			{
				note = _("にはかなり耐性がある！", " resists a lot.");
				dam /= 9;
				if (is_original_ap_and_seen(caster_ptr, m_ptr)) r_ptr->r_flagsr |= (RFR_IM_ACID);
			}
			break;
		}
		case GF_ELEC:
		{
			if (seen) obvious = TRUE;
			if (r_ptr->flagsr & RFR_IM_ELEC)
			{
				note = _("にはかなり耐性がある！", " resists a lot.");
				dam /= 9;
				if (is_original_ap_and_seen(caster_ptr, m_ptr)) r_ptr->r_flagsr |= (RFR_IM_ELEC);
			}
			break;
		}
		case GF_FIRE:
		{
			if (seen) obvious = TRUE;
			if (r_ptr->flagsr & RFR_IM_FIRE)
			{
				note = _("にはかなり耐性がある！", " resists a lot.");
				dam /= 9;
				if (is_original_ap_and_seen(caster_ptr, m_ptr)) r_ptr->r_flagsr |= (RFR_IM_FIRE);
			}
			else if (r_ptr->flags3 & (RF3_HURT_FIRE))
			{
				note = _("はひどい痛手をうけた。", " is hit hard.");
				dam *= 2;
				if (is_original_ap_and_seen(caster_ptr, m_ptr)) r_ptr->r_flags3 |= (RF3_HURT_FIRE);
			}
			break;
		}
		case GF_COLD:
		{
			if (seen) obvious = TRUE;
			if (r_ptr->flagsr & RFR_IM_COLD)
			{
				note = _("にはかなり耐性がある！", " resists a lot.");
				dam /= 9;
				if (is_original_ap_and_seen(caster_ptr, m_ptr)) r_ptr->r_flagsr |= (RFR_IM_COLD);
			}
			else if (r_ptr->flags3 & (RF3_HURT_COLD))
			{
				note = _("はひどい痛手をうけた。", " is hit hard.");
				dam *= 2;
				if (is_original_ap_and_seen(caster_ptr, m_ptr)) r_ptr->r_flags3 |= (RF3_HURT_COLD);
			}
			break;
		}
		case GF_POIS:
		{
			if (seen) obvious = TRUE;
			if (r_ptr->flagsr & RFR_IM_POIS)
			{
				note = _("にはかなり耐性がある！", " resists a lot.");
				dam /= 9;
				if (is_original_ap_and_seen(caster_ptr, m_ptr)) r_ptr->r_flagsr |= (RFR_IM_POIS);
			}
			break;
		}
		case GF_NUKE:
		{
			if (seen) obvious = TRUE;
			if (r_ptr->flagsr & RFR_IM_POIS)
			{
				note = _("には耐性がある。", " resists.");
				dam *= 3; dam /= randint1(6) + 6;
				if (is_original_ap_and_seen(caster_ptr, m_ptr)) r_ptr->r_flagsr |= (RFR_IM_POIS);
			}
			else if (one_in_(3)) do_poly = TRUE;
			break;
		}
		case GF_HELL_FIRE:
		{
			if (seen) obvious = TRUE;
			if (r_ptr->flags3 & RF3_GOOD)
			{
				note = _("はひどい痛手をうけた。", " is hit hard.");
				dam *= 2;
				if (is_original_ap_and_seen(caster_ptr, m_ptr)) r_ptr->r_flags3 |= (RF3_GOOD);
			}
			break;
		}
		case GF_HOLY_FIRE:
		{
			if (seen) obvious = TRUE;
			if (r_ptr->flags3 & RF3_EVIL)
			{
				dam *= 2;
				note = _("はひどい痛手をうけた。", " is hit hard.");
				if (is_original_ap_and_seen(caster_ptr, m_ptr)) r_ptr->r_flags3 |= RF3_EVIL;
			}
			else
			{
				note = _("には耐性がある。", " resists.");
				dam *= 3; dam /= randint1(6) + 6;
			}
			break;
		}
		case GF_ARROW:
		{
			if (seen) obvious = TRUE;
			break;
		}
		case GF_PLASMA:
		{
			if (seen) obvious = TRUE;
			if (r_ptr->flagsr & RFR_RES_PLAS)
			{
				note = _("には耐性がある。", " resists.");
				dam *= 3; dam /= randint1(6) + 6;
				if (is_original_ap_and_seen(caster_ptr, m_ptr)) r_ptr->r_flagsr |= (RFR_RES_PLAS);
			}

			break;
		}
		case GF_NETHER:
		{
			if (seen) obvious = TRUE;
			if (r_ptr->flagsr & RFR_RES_NETH)
			{
				if (r_ptr->flags3 & RF3_UNDEAD)
				{
					note = _("には完全な耐性がある！", " is immune.");
					dam = 0;
					if (is_original_ap_and_seen(caster_ptr, m_ptr)) r_ptr->r_flags3 |= (RF3_UNDEAD);
				}
				else
				{
					note = _("には耐性がある。", " resists.");
					dam *= 3; dam /= randint1(6) + 6;
				}
				if (is_original_ap_and_seen(caster_ptr, m_ptr)) r_ptr->r_flagsr |= (RFR_RES_NETH);
			}
			else if (r_ptr->flags3 & RF3_EVIL)
			{
				note = _("はいくらか耐性を示した。", " resists somewhat.");
				dam /= 2;
				if (is_original_ap_and_seen(caster_ptr, m_ptr)) r_ptr->r_flags3 |= (RF3_EVIL);
			}

			break;
		}
		case GF_WATER:
		{
			if (seen) obvious = TRUE;
			if (r_ptr->flagsr & RFR_RES_WATE)
			{
				if ((m_ptr->r_idx == MON_WATER_ELEM) || (m_ptr->r_idx == MON_UNMAKER))
				{
					note = _("には完全な耐性がある！", " is immune.");
					dam = 0;
				}
				else
				{
					note = _("には耐性がある。", " resists.");
					dam *= 3; dam /= randint1(6) + 6;
				}
				if (is_original_ap_and_seen(caster_ptr, m_ptr)) r_ptr->r_flagsr |= (RFR_RES_WATE);
			}

			break;
		}
		case GF_CHAOS:
		{
			if (seen) obvious = TRUE;
			if (r_ptr->flagsr & RFR_RES_CHAO)
			{
				note = _("には耐性がある。", " resists.");
				dam *= 3; dam /= randint1(6) + 6;
				if (is_original_ap_and_seen(caster_ptr, m_ptr)) r_ptr->r_flagsr |= (RFR_RES_CHAO);
			}
			else if ((r_ptr->flags3 & RF3_DEMON) && one_in_(3))
			{
				note = _("はいくらか耐性を示した。", " resists somewhat.");
				dam *= 3; dam /= randint1(6) + 6;
				if (is_original_ap_and_seen(caster_ptr, m_ptr)) r_ptr->r_flags3 |= (RF3_DEMON);
			}
			else
			{
				do_poly = TRUE;
				do_conf = (5 + randint1(11) + r) / (r + 1);
			}

			break;
		}
		case GF_SHARDS:
		{
			if (seen) obvious = TRUE;
			if (r_ptr->flagsr & RFR_RES_SHAR)
			{
				note = _("には耐性がある。", " resists.");
				dam *= 3; dam /= randint1(6) + 6;
				if (is_original_ap_and_seen(caster_ptr, m_ptr)) r_ptr->r_flagsr |= (RFR_RES_SHAR);
			}

			break;
		}
		case GF_ROCKET:
		{
			if (seen) obvious = TRUE;
			if (r_ptr->flagsr & RFR_RES_SHAR)
			{
				note = _("はいくらか耐性を示した。", " resists somewhat.");
				dam /= 2;
				if (is_original_ap_and_seen(caster_ptr, m_ptr)) r_ptr->r_flagsr |= (RFR_RES_SHAR);
			}

			break;
		}
		case GF_SOUND:
		{
			if (seen) obvious = TRUE;
			if (r_ptr->flagsr & RFR_RES_SOUN)
			{
				note = _("には耐性がある。", " resists.");
				dam *= 2; dam /= randint1(6) + 6;
				if (is_original_ap_and_seen(caster_ptr, m_ptr)) r_ptr->r_flagsr |= (RFR_RES_SOUN);
			}
			else
				do_stun = (10 + randint1(15) + r) / (r + 1);

			break;
		}
		case GF_CONFUSION:
		{
			if (seen) obvious = TRUE;
			if (r_ptr->flags3 & RF3_NO_CONF)
			{
				note = _("には耐性がある。", " resists.");
				dam *= 3; dam /= randint1(6) + 6;
				if (is_original_ap_and_seen(caster_ptr, m_ptr)) r_ptr->r_flags3 |= (RF3_NO_CONF);
			}
			else
				do_conf = (10 + randint1(15) + r) / (r + 1);

			break;
		}
		case GF_DISENCHANT:
		{
			if (seen) obvious = TRUE;
			if (r_ptr->flagsr & RFR_RES_DISE)
			{
				note = _("には耐性がある。", " resists.");
				dam *= 3; dam /= randint1(6) + 6;
				if (is_original_ap_and_seen(caster_ptr, m_ptr)) r_ptr->r_flagsr |= (RFR_RES_DISE);
			}

			break;
		}
		case GF_NEXUS:
		{
			if (seen) obvious = TRUE;
			if (r_ptr->flagsr & RFR_RES_NEXU)
			{
				note = _("には耐性がある。", " resists.");
				dam *= 3; dam /= randint1(6) + 6;
				if (is_original_ap_and_seen(caster_ptr, m_ptr)) r_ptr->r_flagsr |= (RFR_RES_NEXU);
			}

			break;
		}
		case GF_FORCE:
		{
			if (seen) obvious = TRUE;
			if (r_ptr->flagsr & RFR_RES_WALL)
			{
				note = _("には耐性がある。", " resists.");
				dam *= 3; dam /= randint1(6) + 6;
				if (is_original_ap_and_seen(caster_ptr, m_ptr)) r_ptr->r_flagsr |= (RFR_RES_WALL);
			}
			else
				do_stun = (randint1(15) + r) / (r + 1);

			break;
		}
		case GF_INERTIAL:
		{
			if (seen) obvious = TRUE;
			if (r_ptr->flagsr & RFR_RES_INER)
			{
				note = _("には耐性がある。", " resists.");
				dam *= 3; dam /= randint1(6) + 6;
				if (is_original_ap_and_seen(caster_ptr, m_ptr)) r_ptr->r_flagsr |= (RFR_RES_INER);
			}
			else
			{
				/* Powerful monsters can resist */
				if ((r_ptr->flags1 & (RF1_UNIQUE)) ||
					(r_ptr->level > randint1((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
				{
					obvious = FALSE;
				}
				/* Normal monsters slow down */
				else
				{
					if (set_monster_slow(caster_ptr, g_ptr->m_idx, MON_SLOW(m_ptr) + 50))
					{
						note = _("の動きが遅くなった。", " starts moving slower.");
					}
				}
			}

			break;
		}
		case GF_TIME:
		{
			if (seen) obvious = TRUE;
			if (r_ptr->flagsr & RFR_RES_TIME)
			{
				note = _("には耐性がある。", " resists.");
				dam *= 3; dam /= randint1(6) + 6;
				if (is_original_ap_and_seen(caster_ptr, m_ptr)) r_ptr->r_flagsr |= (RFR_RES_TIME);
			}
			else
				do_time = (dam + 1) / 2;

			break;
		}
		case GF_GRAVITY:
		{
			bool resist_tele = FALSE;

			if (seen) obvious = TRUE;
			if (r_ptr->flagsr & RFR_RES_TELE)
			{
				if (r_ptr->flags1 & (RF1_UNIQUE))
				{
					if (is_original_ap_and_seen(caster_ptr, m_ptr)) r_ptr->r_flagsr |= RFR_RES_TELE;
					note = _("には効果がなかった。", " is unaffected!");
					resist_tele = TRUE;
				}
				else if (r_ptr->level > randint1(100))
				{
					if (is_original_ap_and_seen(caster_ptr, m_ptr)) r_ptr->r_flagsr |= RFR_RES_TELE;
					note = _("には耐性がある！", " resists!");
					resist_tele = TRUE;
				}
			}

			if (!resist_tele) do_dist = 10;
			else do_dist = 0;

			if (caster_ptr->riding && (g_ptr->m_idx == caster_ptr->riding)) do_dist = 0;

			if (r_ptr->flagsr & RFR_RES_GRAV)
			{
				note = _("には耐性がある！", " resists!");
				dam *= 3; dam /= randint1(6) + 6;
				do_dist = 0;
				if (is_original_ap_and_seen(caster_ptr, m_ptr)) r_ptr->r_flagsr |= (RFR_RES_GRAV);
			}
			else
			{
				/* 1. slowness */
				/* Powerful monsters can resist */
				if ((r_ptr->flags1 & (RF1_UNIQUE)) ||
					(r_ptr->level > randint1((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
				{
					obvious = FALSE;
				}
				/* Normal monsters slow down */
				else
				{
					if (set_monster_slow(caster_ptr, g_ptr->m_idx, MON_SLOW(m_ptr) + 50))
					{
						note = _("の動きが遅くなった。", " starts moving slower.");
					}
				}

				/* 2. stun */
				do_stun = damroll((caster_lev / 20) + 3, (dam)) + 1;

				/* Attempt a saving throw */
				if ((r_ptr->flags1 & (RF1_UNIQUE)) ||
					(r_ptr->level > randint1((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
				{
					/* Resist */
					do_stun = 0;
					/* No obvious effect */
					note = _("には効果がなかった。", " is unaffected!");
					obvious = FALSE;
				}
			}

			break;
		}
		case GF_MANA:
		case GF_SEEKER:
		case GF_SUPER_RAY:
		{
			if (seen) obvious = TRUE;
			break;
		}
		case GF_DISINTEGRATE:
		{
			if (seen) obvious = TRUE;
			if (r_ptr->flags3 & RF3_HURT_ROCK)
			{
				if (is_original_ap_and_seen(caster_ptr, m_ptr)) r_ptr->r_flags3 |= (RF3_HURT_ROCK);
				note = _("の皮膚がただれた！", " loses some skin!");
				note_dies = _("は蒸発した！", " evaporates!");
				dam *= 2;
			}

			break;
		}
		case GF_PSI:
		{
			if (seen) obvious = TRUE;
			if (!(los(caster_ptr, m_ptr->fy, m_ptr->fx, caster_ptr->y, caster_ptr->x)))
			{
				if (seen_msg)
					msg_format(_("%sはあなたが見えないので影響されない！", "%^s can't see you, and isn't affected!"), m_name);
				skipped = TRUE;
				break;
			}

			if (r_ptr->flags2 & RF2_EMPTY_MIND)
			{
				dam = 0;
				note = _("には完全な耐性がある！", " is immune.");
				if (is_original_ap_and_seen(caster_ptr, m_ptr)) r_ptr->r_flags2 |= (RF2_EMPTY_MIND);

			}
			else if ((r_ptr->flags2 & (RF2_STUPID | RF2_WEIRD_MIND)) ||
				(r_ptr->flags3 & RF3_ANIMAL) ||
				(r_ptr->level > randint1(3 * dam)))
			{
				note = _("には耐性がある！", " resists!");
				dam /= 3;

				/*
				 * Powerful demons & undead can turn a mindcrafter's
				 * attacks back on them
				 */
				if ((r_ptr->flags3 & (RF3_UNDEAD | RF3_DEMON)) &&
					(r_ptr->level > caster_ptr->lev / 2) &&
					one_in_(2))
				{
					note = NULL;
					msg_format(_("%^sの堕落した精神は攻撃を跳ね返した！",
						(seen ? "%^s's corrupted mind backlashes your attack!" :
							"%^ss corrupted mind backlashes your attack!")), m_name);

					if ((randint0(100 + r_ptr->level / 2) < caster_ptr->skill_sav) && !CHECK_MULTISHADOW(caster_ptr))
					{
						msg_print(_("しかし効力を跳ね返した！", "You resist the effects!"));
					}
					else
					{
						/* Injure +/- confusion */
						monster_desc(caster_ptr, killer, m_ptr, MD_WRONGDOER_NAME);
						take_hit(caster_ptr, DAMAGE_ATTACK, dam, killer, -1);  /* has already been /3 */
						if (one_in_(4) && !CHECK_MULTISHADOW(caster_ptr))
						{
							switch (randint1(4))
							{
							case 1:
								set_confused(caster_ptr, caster_ptr->confused + 3 + randint1(dam));
								break;
							case 2:
								set_stun(caster_ptr, caster_ptr->stun + randint1(dam));
								break;
							case 3:
							{
								if (r_ptr->flags3 & RF3_NO_FEAR)
									note = _("には効果がなかった。", " is unaffected.");
								else
									set_afraid(caster_ptr, caster_ptr->afraid + 3 + randint1(dam));
								break;
							}
							default:
								if (!caster_ptr->free_act)
									(void)set_paralyzed(caster_ptr, caster_ptr->paralyzed + randint1(dam));
								break;
							}
						}
					}

					dam = 0;
				}
			}

			if ((dam > 0) && one_in_(4))
			{
				switch (randint1(4))
				{
				case 1:
					do_conf = 3 + randint1(dam);
					break;
				case 2:
					do_stun = 3 + randint1(dam);
					break;
				case 3:
					do_fear = 3 + randint1(dam);
					break;
				default:
					note = _("は眠り込んでしまった！", " falls asleep!");
					do_sleep = 3 + randint1(dam);
					break;
				}
			}

			note_dies = _("の精神は崩壊し、肉体は抜け殻となった。", " collapses, a mindless husk.");
			break;
		}
		case GF_PSI_DRAIN:
		{
			if (seen) obvious = TRUE;
			if (r_ptr->flags2 & RF2_EMPTY_MIND)
			{
				dam = 0;
				note = _("には完全な耐性がある！", " is immune.");
			}
			else if ((r_ptr->flags2 & (RF2_STUPID | RF2_WEIRD_MIND)) ||
				(r_ptr->flags3 & RF3_ANIMAL) ||
				(r_ptr->level > randint1(3 * dam)))
			{
				note = _("には耐性がある！", " resists!");
				dam /= 3;

				/*
				 * Powerful demons & undead can turn a mindcrafter's
				 * attacks back on them
				 */
				if ((r_ptr->flags3 & (RF3_UNDEAD | RF3_DEMON)) &&
					(r_ptr->level > caster_ptr->lev / 2) &&
					(one_in_(2)))
				{
					note = NULL;
					msg_format(_("%^sの堕落した精神は攻撃を跳ね返した！",
						(seen ? "%^s's corrupted mind backlashes your attack!" :
							"%^ss corrupted mind backlashes your attack!")), m_name);
					if ((randint0(100 + r_ptr->level / 2) < caster_ptr->skill_sav) && !CHECK_MULTISHADOW(caster_ptr))
					{
						msg_print(_("あなたは効力を跳ね返した！", "You resist the effects!"));
					}
					else
					{
						monster_desc(caster_ptr, killer, m_ptr, MD_WRONGDOER_NAME);
						if (!CHECK_MULTISHADOW(caster_ptr))
						{
							msg_print(_("超能力パワーを吸いとられた！", "Your psychic energy is drained!"));
							caster_ptr->csp -= damroll(5, dam) / 2;
							if (caster_ptr->csp < 0) caster_ptr->csp = 0;
							caster_ptr->redraw |= PR_MANA;
							caster_ptr->window |= (PW_SPELL);
						}
						take_hit(caster_ptr, DAMAGE_ATTACK, dam, killer, -1);  /* has already been /3 */
					}

					dam = 0;
				}
			}
			else if (dam > 0)
			{
				int b = damroll(5, dam) / 4;
				concptr str = (caster_ptr->pclass == CLASS_MINDCRAFTER) ? _("超能力パワー", "psychic energy") : _("魔力", "mana");
				concptr msg = _("あなたは%sの苦痛を%sに変換した！",
					(seen ? "You convert %s's pain into %s!" :
						"You convert %ss pain into %s!"));
				msg_format(msg, m_name, str);

				b = MIN(caster_ptr->msp, caster_ptr->csp + b);
				caster_ptr->csp = b;
				caster_ptr->redraw |= PR_MANA;
				caster_ptr->window |= (PW_SPELL);
			}

			note_dies = _("の精神は崩壊し、肉体は抜け殻となった。", " collapses, a mindless husk.");
			break;
		}
		case GF_TELEKINESIS:
		{
			if (seen) obvious = TRUE;
			if (one_in_(4))
			{
				if (caster_ptr->riding && (g_ptr->m_idx == caster_ptr->riding)) do_dist = 0;
				else do_dist = 7;
			}

			do_stun = damroll((caster_lev / 20) + 3, dam) + 1;
			if ((r_ptr->flags1 & RF1_UNIQUE) ||
				(r_ptr->level > 5 + randint1(dam)))
			{
				do_stun = 0;
				obvious = FALSE;
			}

			break;
		}
		case GF_PSY_SPEAR:
		{
			if (seen) obvious = TRUE;
			break;
		}
		case GF_METEOR:
		{
			if (seen) obvious = TRUE;
			break;
		}
		case GF_DOMINATION:
		{
			if (!is_hostile(m_ptr)) break;
			if (seen) obvious = TRUE;
			if ((r_ptr->flags1 & (RF1_UNIQUE | RF1_QUESTOR)) ||
				(r_ptr->flags3 & RF3_NO_CONF) ||
				(r_ptr->level > randint1((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
			{
				if (r_ptr->flags3 & RF3_NO_CONF)
				{
					if (is_original_ap_and_seen(caster_ptr, m_ptr)) r_ptr->r_flags3 |= (RF3_NO_CONF);
				}

				do_conf = 0;

				/*
				 * Powerful demons & undead can turn a mindcrafter's
				 * attacks back on them
				 */
				if ((r_ptr->flags3 & (RF3_UNDEAD | RF3_DEMON)) &&
					(r_ptr->level > caster_ptr->lev / 2) &&
					(one_in_(2)))
				{
					note = NULL;
					msg_format(_("%^sの堕落した精神は攻撃を跳ね返した！",
						(seen ? "%^s's corrupted mind backlashes your attack!" :
							"%^ss corrupted mind backlashes your attack!")), m_name);

					/* Saving throw */
					if (randint0(100 + r_ptr->level / 2) < caster_ptr->skill_sav)
					{
						msg_print(_("しかし効力を跳ね返した！", "You resist the effects!"));
					}
					else
					{
						/* Confuse, stun, terrify */
						switch (randint1(4))
						{
						case 1:
							set_stun(caster_ptr, caster_ptr->stun + dam / 2);
							break;
						case 2:
							set_confused(caster_ptr, caster_ptr->confused + dam / 2);
							break;
						default:
						{
							if (r_ptr->flags3 & RF3_NO_FEAR)
								note = _("には効果がなかった。", " is unaffected.");
							else
								set_afraid(caster_ptr, caster_ptr->afraid + dam);
						}
						}
					}
				}
				else
				{
					note = _("には効果がなかった。", " is unaffected.");
					obvious = FALSE;
				}
			}
			else
			{
				if (!common_saving_throw_charm(caster_ptr, dam, m_ptr))
				{
					note = _("があなたに隷属した。", " is in your thrall!");
					set_pet(caster_ptr, m_ptr);
				}
				else
				{
					switch (randint1(4))
					{
					case 1:
						do_stun = dam / 2;
						break;
					case 2:
						do_conf = dam / 2;
						break;
					default:
						do_fear = dam;
					}
				}
			}

			dam = 0;
			break;
		}
		case GF_ICE:
		{
			if (seen) obvious = TRUE;
			do_stun = (randint1(15) + 1) / (r + 1);
			if (r_ptr->flagsr & RFR_IM_COLD)
			{
				note = _("にはかなり耐性がある！", " resists a lot.");
				dam /= 9;
				if (is_original_ap_and_seen(caster_ptr, m_ptr)) r_ptr->r_flagsr |= (RFR_IM_COLD);
			}
			else if (r_ptr->flags3 & (RF3_HURT_COLD))
			{
				note = _("はひどい痛手をうけた。", " is hit hard.");
				dam *= 2;
				if (is_original_ap_and_seen(caster_ptr, m_ptr)) r_ptr->r_flags3 |= (RF3_HURT_COLD);
			}

			break;
		}
		case GF_HYPODYNAMIA:
		{
			if (seen) obvious = TRUE;
			if (!monster_living(m_ptr->r_idx))
			{
				if (is_original_ap_and_seen(caster_ptr, m_ptr))
				{
					if (r_ptr->flags3 & RF3_DEMON) r_ptr->r_flags3 |= (RF3_DEMON);
					if (r_ptr->flags3 & RF3_UNDEAD) r_ptr->r_flags3 |= (RF3_UNDEAD);
					if (r_ptr->flags3 & RF3_NONLIVING) r_ptr->r_flags3 |= (RF3_NONLIVING);
				}
				note = _("には効果がなかった。", " is unaffected.");
				obvious = FALSE;
				dam = 0;
			}
			else
				do_time = (dam + 7) / 8;

			break;
		}
		case GF_DEATH_RAY:
		{
			if (seen) obvious = TRUE;
			if (!monster_living(m_ptr->r_idx))
			{
				if (is_original_ap_and_seen(caster_ptr, m_ptr))
				{
					if (r_ptr->flags3 & RF3_DEMON) r_ptr->r_flags3 |= (RF3_DEMON);
					if (r_ptr->flags3 & RF3_UNDEAD) r_ptr->r_flags3 |= (RF3_UNDEAD);
					if (r_ptr->flags3 & RF3_NONLIVING) r_ptr->r_flags3 |= (RF3_NONLIVING);
				}
				note = _("には完全な耐性がある！", " is immune.");
				obvious = FALSE;
				dam = 0;
			}
			else if (((r_ptr->flags1 & RF1_UNIQUE) &&
				(randint1(888) != 666)) ||
				(((r_ptr->level + randint1(20)) > randint1((caster_lev / 2) + randint1(10))) &&
					randint1(100) != 66))
			{
				note = _("には耐性がある！", " resists!");
				obvious = FALSE;
				dam = 0;
			}

			break;
		}
		case GF_OLD_POLY:
		{
			if (seen) obvious = TRUE;
			do_poly = TRUE;

			/* Powerful monsters can resist */
			if ((r_ptr->flags1 & RF1_UNIQUE) ||
				(r_ptr->flags1 & RF1_QUESTOR) ||
				(r_ptr->level > randint1((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
			{
				note = _("には効果がなかった。", " is unaffected.");
				do_poly = FALSE;
				obvious = FALSE;
			}

			dam = 0;
			break;
		}
		case GF_OLD_CLONE:
		{
			if (seen) obvious = TRUE;

			if ((floor_ptr->inside_arena) || is_pet(m_ptr) || (r_ptr->flags1 & (RF1_UNIQUE | RF1_QUESTOR)) || (r_ptr->flags7 & (RF7_NAZGUL | RF7_UNIQUE2)))
			{
				note = _("には効果がなかった。", " is unaffected.");
			}
			else
			{
				m_ptr->hp = m_ptr->maxhp;
				if (multiply_monster(caster_ptr, g_ptr->m_idx, TRUE, 0L))
				{
					note = _("が分裂した！", " spawns!");
				}
			}

			dam = 0;
			break;
		}
		case GF_STAR_HEAL:
		{
			if (seen) obvious = TRUE;

			(void)set_monster_csleep(caster_ptr, g_ptr->m_idx, 0);

			if (m_ptr->maxhp < m_ptr->max_maxhp)
			{
				if (seen_msg) msg_format(_("%^sの強さが戻った。", "%^s recovers %s vitality."), m_name, m_poss);
				m_ptr->maxhp = m_ptr->max_maxhp;
			}

			if (!dam)
			{
				if (caster_ptr->health_who == g_ptr->m_idx) caster_ptr->redraw |= (PR_HEALTH);
				if (caster_ptr->riding == g_ptr->m_idx) caster_ptr->redraw |= (PR_UHEALTH);
				break;
			}
		}
			/* Fall through */
		case GF_OLD_HEAL:
		{
			if (seen) obvious = TRUE;

			/* Wake up */
			(void)set_monster_csleep(caster_ptr, g_ptr->m_idx, 0);
			if (MON_STUNNED(m_ptr))
			{
				if (seen_msg) msg_format(_("%^sは朦朧状態から立ち直った。", "%^s is no longer stunned."), m_name);
				(void)set_monster_stunned(caster_ptr, g_ptr->m_idx, 0);
			}
			if (MON_CONFUSED(m_ptr))
			{
				if (seen_msg) msg_format(_("%^sは混乱から立ち直った。", "%^s is no longer confused."), m_name);
				(void)set_monster_confused(caster_ptr, g_ptr->m_idx, 0);
			}
			if (MON_MONFEAR(m_ptr))
			{
				if (seen_msg) msg_format(_("%^sは勇気を取り戻した。", "%^s recovers %s courage."), m_name, m_poss);
				(void)set_monster_monfear(caster_ptr, g_ptr->m_idx, 0);
			}

			if (m_ptr->hp < 30000) m_ptr->hp += dam;
			if (m_ptr->hp > m_ptr->maxhp) m_ptr->hp = m_ptr->maxhp;

			if (!who)
			{
				chg_virtue(caster_ptr, V_VITALITY, 1);

				if (r_ptr->flags1 & RF1_UNIQUE)
					chg_virtue(caster_ptr, V_INDIVIDUALISM, 1);

				if (is_friendly(m_ptr))
					chg_virtue(caster_ptr, V_HONOUR, 1);
				else if (!(r_ptr->flags3 & RF3_EVIL))
				{
					if (r_ptr->flags3 & RF3_GOOD)
						chg_virtue(caster_ptr, V_COMPASSION, 2);
					else
						chg_virtue(caster_ptr, V_COMPASSION, 1);
				}

				if (r_ptr->flags3 & RF3_ANIMAL)
					chg_virtue(caster_ptr, V_NATURE, 1);
			}

			if (m_ptr->r_idx == MON_LEPER)
			{
				heal_leper = TRUE;
				if (!who) chg_virtue(caster_ptr, V_COMPASSION, 5);
			}

			if (caster_ptr->health_who == g_ptr->m_idx) caster_ptr->redraw |= (PR_HEALTH);
			if (caster_ptr->riding == g_ptr->m_idx) caster_ptr->redraw |= (PR_UHEALTH);

			note = _("は体力を回復したようだ。", " looks healthier.");

			dam = 0;
			break;
		}
		case GF_OLD_SPEED:
		{
			if (seen) obvious = TRUE;

			if (set_monster_fast(caster_ptr, g_ptr->m_idx, MON_FAST(m_ptr) + 100))
			{
				note = _("の動きが速くなった。", " starts moving faster.");
			}

			if (!who)
			{
				if (r_ptr->flags1 & RF1_UNIQUE)
					chg_virtue(caster_ptr, V_INDIVIDUALISM, 1);
				if (is_friendly(m_ptr))
					chg_virtue(caster_ptr, V_HONOUR, 1);
			}

			dam = 0;
			break;
		}
		case GF_OLD_SLOW:
		{
			if (seen) obvious = TRUE;

			/* Powerful monsters can resist */
			if ((r_ptr->flags1 & RF1_UNIQUE) ||
				(r_ptr->level > randint1((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
			{
				note = _("には効果がなかった。", " is unaffected.");
				obvious = FALSE;
			}
			else
			{
				if (set_monster_slow(caster_ptr, g_ptr->m_idx, MON_SLOW(m_ptr) + 50))
				{
					note = _("の動きが遅くなった。", " starts moving slower.");
				}
			}

			dam = 0;
			break;
		}
		case GF_OLD_SLEEP:
		{
			if (seen) obvious = TRUE;

			if ((r_ptr->flags1 & RF1_UNIQUE) ||
				(r_ptr->flags3 & RF3_NO_SLEEP) ||
				(r_ptr->level > randint1((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
			{
				if (r_ptr->flags3 & RF3_NO_SLEEP)
				{
					if (is_original_ap_and_seen(caster_ptr, m_ptr)) r_ptr->r_flags3 |= (RF3_NO_SLEEP);
				}

				note = _("には効果がなかった。", " is unaffected.");
				obvious = FALSE;
			}
			else
			{
				note = _("は眠り込んでしまった！", " falls asleep!");
				do_sleep = 500;
			}

			dam = 0;
			break;
		}
		case GF_STASIS_EVIL:
		{
			if (seen) obvious = TRUE;

			if ((r_ptr->flags1 & RF1_UNIQUE) ||
				!(r_ptr->flags3 & RF3_EVIL) ||
				(r_ptr->level > randint1((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
			{
				note = _("には効果がなかった。", " is unaffected.");
				obvious = FALSE;
			}
			else
			{
				note = _("は動けなくなった！", " is suspended!");
				do_sleep = 500;
			}

			dam = 0;
			break;
		}
		case GF_STASIS:
		{
			if (seen) obvious = TRUE;

			if ((r_ptr->flags1 & RF1_UNIQUE) ||
				(r_ptr->level > randint1((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
			{
				note = _("には効果がなかった。", " is unaffected.");
				obvious = FALSE;
			}
			else
			{
				note = _("は動けなくなった！", " is suspended!");
				do_sleep = 500;
			}

			dam = 0;
			break;
		}
		case GF_CHARM:
		{
			int vir;
			vir = virtue_number(caster_ptr, V_HARMONY);
			if (vir)
			{
				dam += caster_ptr->virtues[vir - 1] / 10;
			}

			vir = virtue_number(caster_ptr, V_INDIVIDUALISM);
			if (vir)
			{
				dam -= caster_ptr->virtues[vir - 1] / 20;
			}

			if (seen) obvious = TRUE;

			if (common_saving_throw_charm(caster_ptr, dam, m_ptr))
			{
				note = _("には効果がなかった。", " is unaffected.");
				obvious = FALSE;

				if (one_in_(4)) m_ptr->mflag2 |= MFLAG2_NOPET;
			}
			else if (caster_ptr->cursed & TRC_AGGRAVATE)
			{
				note = _("はあなたに敵意を抱いている！", " hates you too much!");
				if (one_in_(4)) m_ptr->mflag2 |= MFLAG2_NOPET;
			}
			else
			{
				note = _("は突然友好的になったようだ！", " suddenly seems friendly!");
				set_pet(caster_ptr, m_ptr);

				chg_virtue(caster_ptr, V_INDIVIDUALISM, -1);
				if (r_ptr->flags3 & RF3_ANIMAL)
					chg_virtue(caster_ptr, V_NATURE, 1);
			}

			dam = 0;
			break;
		}
		case GF_CONTROL_UNDEAD:
		{
			int vir;
			if (seen) obvious = TRUE;

			vir = virtue_number(caster_ptr, V_UNLIFE);
			if (vir)
			{
				dam += caster_ptr->virtues[vir - 1] / 10;
			}

			vir = virtue_number(caster_ptr, V_INDIVIDUALISM);
			if (vir)
			{
				dam -= caster_ptr->virtues[vir - 1] / 20;
			}

			if (common_saving_throw_control(caster_ptr, dam, m_ptr) ||
				!(r_ptr->flags3 & RF3_UNDEAD))
			{
				note = _("には効果がなかった。", " is unaffected.");
				obvious = FALSE;
				if (one_in_(4)) m_ptr->mflag2 |= MFLAG2_NOPET;
			}
			else if (caster_ptr->cursed & TRC_AGGRAVATE)
			{
				note = _("はあなたに敵意を抱いている！", " hates you too much!");
				if (one_in_(4)) m_ptr->mflag2 |= MFLAG2_NOPET;
			}
			else
			{
				note = _("は既にあなたの奴隷だ！", " is in your thrall!");
				set_pet(caster_ptr, m_ptr);
			}

			dam = 0;
			break;
		}
		case GF_CONTROL_DEMON:
		{
			int vir;
			if (seen) obvious = TRUE;

			vir = virtue_number(caster_ptr, V_UNLIFE);
			if (vir)
			{
				dam += caster_ptr->virtues[vir - 1] / 10;
			}

			vir = virtue_number(caster_ptr, V_INDIVIDUALISM);
			if (vir)
			{
				dam -= caster_ptr->virtues[vir - 1] / 20;
			}

			if (common_saving_throw_control(caster_ptr, dam, m_ptr) ||
				!(r_ptr->flags3 & RF3_DEMON))
			{
				note = _("には効果がなかった。", " is unaffected.");
				obvious = FALSE;
				if (one_in_(4)) m_ptr->mflag2 |= MFLAG2_NOPET;
			}
			else if (caster_ptr->cursed & TRC_AGGRAVATE)
			{
				note = _("はあなたに敵意を抱いている！", " hates you too much!");
				if (one_in_(4)) m_ptr->mflag2 |= MFLAG2_NOPET;
			}
			else
			{
				note = _("は既にあなたの奴隷だ！", " is in your thrall!");
				set_pet(caster_ptr, m_ptr);
			}

			dam = 0;
			break;
		}
		case GF_CONTROL_ANIMAL:
		{
			int vir;
			if (seen) obvious = TRUE;

			vir = virtue_number(caster_ptr, V_NATURE);
			if (vir)
			{
				dam += caster_ptr->virtues[vir - 1] / 10;
			}

			vir = virtue_number(caster_ptr, V_INDIVIDUALISM);
			if (vir)
			{
				dam -= caster_ptr->virtues[vir - 1] / 20;
			}

			if (common_saving_throw_control(caster_ptr, dam, m_ptr) ||
				!(r_ptr->flags3 & RF3_ANIMAL))
			{
				note = _("には効果がなかった。", " is unaffected.");
				obvious = FALSE;
				if (one_in_(4)) m_ptr->mflag2 |= MFLAG2_NOPET;
			}
			else if (caster_ptr->cursed & TRC_AGGRAVATE)
			{
				note = _("はあなたに敵意を抱いている！", " hates you too much!");
				if (one_in_(4)) m_ptr->mflag2 |= MFLAG2_NOPET;
			}
			else
			{
				note = _("はなついた。", " is tamed!");
				set_pet(caster_ptr, m_ptr);
				if (r_ptr->flags3 & RF3_ANIMAL)
					chg_virtue(caster_ptr, V_NATURE, 1);
			}

			dam = 0;
			break;
		}
		case GF_CHARM_LIVING:
		{
			int vir;

			vir = virtue_number(caster_ptr, V_UNLIFE);
			if (seen) obvious = TRUE;

			vir = virtue_number(caster_ptr, V_UNLIFE);
			if (vir)
			{
				dam -= caster_ptr->virtues[vir - 1] / 10;
			}

			vir = virtue_number(caster_ptr, V_INDIVIDUALISM);
			if (vir)
			{
				dam -= caster_ptr->virtues[vir - 1] / 20;
			}

			msg_format(_("%sを見つめた。", "You stare into %s."), m_name);

			if (common_saving_throw_charm(caster_ptr, dam, m_ptr) ||
				!monster_living(m_ptr->r_idx))
			{
				note = _("には効果がなかった。", " is unaffected.");
				obvious = FALSE;
				if (one_in_(4)) m_ptr->mflag2 |= MFLAG2_NOPET;
			}
			else if (caster_ptr->cursed & TRC_AGGRAVATE)
			{
				note = _("はあなたに敵意を抱いている！", " hates you too much!");
				if (one_in_(4)) m_ptr->mflag2 |= MFLAG2_NOPET;
			}
			else
			{
				note = _("を支配した。", " is tamed!");
				set_pet(caster_ptr, m_ptr);
				if (r_ptr->flags3 & RF3_ANIMAL)
					chg_virtue(caster_ptr, V_NATURE, 1);
			}

			dam = 0;
			break;
		}
		case GF_OLD_CONF:
		{
			if (seen) obvious = TRUE;

			do_conf = damroll(3, (dam / 2)) + 1;
			if ((r_ptr->flags1 & (RF1_UNIQUE)) ||
				(r_ptr->flags3 & (RF3_NO_CONF)) ||
				(r_ptr->level > randint1((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
			{
				if (r_ptr->flags3 & (RF3_NO_CONF))
				{
					if (is_original_ap_and_seen(caster_ptr, m_ptr)) r_ptr->r_flags3 |= (RF3_NO_CONF);
				}

				do_conf = 0;
				note = _("には効果がなかった。", " is unaffected.");
				obvious = FALSE;
			}

			dam = 0;
			break;
		}
		case GF_STUN:
		{
			if (seen) obvious = TRUE;

			do_stun = damroll((caster_lev / 20) + 3, (dam)) + 1;
			if ((r_ptr->flags1 & (RF1_UNIQUE)) ||
				(r_ptr->level > randint1((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
			{
				do_stun = 0;
				note = _("には効果がなかった。", " is unaffected.");
				obvious = FALSE;
			}

			dam = 0;
			break;
		}
		case GF_LITE_WEAK:
		{
			if (!dam)
			{
				skipped = TRUE;
				break;
			}

			if (r_ptr->flags3 & (RF3_HURT_LITE))
			{
				if (seen) obvious = TRUE;

				if (is_original_ap_and_seen(caster_ptr, m_ptr)) r_ptr->r_flags3 |= (RF3_HURT_LITE);

				note = _("は光に身をすくめた！", " cringes from the light!");
				note_dies = _("は光を受けてしぼんでしまった！", " shrivels away in the light!");
			}
			else
			{
				dam = 0;
			}

			break;
		}
		case GF_LITE:
		{
			if (seen) obvious = TRUE;

			if (r_ptr->flagsr & RFR_RES_LITE)
			{
				note = _("には耐性がある！", " resists!");
				dam *= 2; dam /= (randint1(6) + 6);
				if (is_original_ap_and_seen(caster_ptr, m_ptr)) r_ptr->r_flagsr |= (RFR_RES_LITE);
			}
			else if (r_ptr->flags3 & (RF3_HURT_LITE))
			{
				if (is_original_ap_and_seen(caster_ptr, m_ptr)) r_ptr->r_flags3 |= (RF3_HURT_LITE);
				note = _("は光に身をすくめた！", " cringes from the light!");
				note_dies = _("は光を受けてしぼんでしまった！", " shrivels away in the light!");
				dam *= 2;
			}
			break;
		}
		case GF_DARK:
		{
			if (seen) obvious = TRUE;

			if (r_ptr->flagsr & RFR_RES_DARK)
			{
				note = _("には耐性がある！", " resists!");
				dam *= 2; dam /= (randint1(6) + 6);
				if (is_original_ap_and_seen(caster_ptr, m_ptr)) r_ptr->r_flagsr |= (RFR_RES_DARK);
			}

			break;
		}
		case GF_KILL_WALL:
		{
			if (r_ptr->flags3 & (RF3_HURT_ROCK))
			{
				if (seen) obvious = TRUE;

				if (is_original_ap_and_seen(caster_ptr, m_ptr)) r_ptr->r_flags3 |= (RF3_HURT_ROCK);

				note = _("の皮膚がただれた！", " loses some skin!");
				note_dies = _("はドロドロに溶けた！", " dissolves!");
			}
			else
			{
				dam = 0;
			}

			break;
		}
		case GF_AWAY_UNDEAD:
		{
			if (r_ptr->flags3 & (RF3_UNDEAD))
			{
				bool resists_tele = FALSE;

				if (r_ptr->flagsr & RFR_RES_TELE)
				{
					if ((r_ptr->flags1 & (RF1_UNIQUE)) || (r_ptr->flagsr & RFR_RES_ALL))
					{
						if (is_original_ap_and_seen(caster_ptr, m_ptr)) r_ptr->r_flagsr |= RFR_RES_TELE;
						note = _("には効果がなかった。", " is unaffected.");
						resists_tele = TRUE;
					}
					else if (r_ptr->level > randint1(100))
					{
						if (is_original_ap_and_seen(caster_ptr, m_ptr)) r_ptr->r_flagsr |= RFR_RES_TELE;
						note = _("には耐性がある！", " resists!");
						resists_tele = TRUE;
					}
				}

				if (!resists_tele)
				{
					if (seen) obvious = TRUE;
					if (is_original_ap_and_seen(caster_ptr, m_ptr)) r_ptr->r_flags3 |= (RF3_UNDEAD);
					do_dist = dam;
				}
			}
			else
			{
				skipped = TRUE;
			}

			dam = 0;
			break;
		}
		case GF_AWAY_EVIL:
		{
			if (r_ptr->flags3 & (RF3_EVIL))
			{
				bool resists_tele = FALSE;

				if (r_ptr->flagsr & RFR_RES_TELE)
				{
					if ((r_ptr->flags1 & (RF1_UNIQUE)) || (r_ptr->flagsr & RFR_RES_ALL))
					{
						if (is_original_ap_and_seen(caster_ptr, m_ptr)) r_ptr->r_flagsr |= RFR_RES_TELE;
						note = _("には効果がなかった。", " is unaffected.");
						resists_tele = TRUE;
					}
					else if (r_ptr->level > randint1(100))
					{
						if (is_original_ap_and_seen(caster_ptr, m_ptr)) r_ptr->r_flagsr |= RFR_RES_TELE;
						note = _("には耐性がある！", " resists!");
						resists_tele = TRUE;
					}
				}

				if (!resists_tele)
				{
					if (seen) obvious = TRUE;
					if (is_original_ap_and_seen(caster_ptr, m_ptr)) r_ptr->r_flags3 |= (RF3_EVIL);
					do_dist = dam;
				}
			}
			else
			{
				skipped = TRUE;
			}

			dam = 0;
			break;
		}
		case GF_AWAY_ALL:
		{
			bool resists_tele = FALSE;
			if (r_ptr->flagsr & RFR_RES_TELE)
			{
				if ((r_ptr->flags1 & (RF1_UNIQUE)) || (r_ptr->flagsr & RFR_RES_ALL))
				{
					if (is_original_ap_and_seen(caster_ptr, m_ptr)) r_ptr->r_flagsr |= RFR_RES_TELE;
					note = _("には効果がなかった。", " is unaffected.");
					resists_tele = TRUE;
				}
				else if (r_ptr->level > randint1(100))
				{
					if (is_original_ap_and_seen(caster_ptr, m_ptr)) r_ptr->r_flagsr |= RFR_RES_TELE;
					note = _("には耐性がある！", " resists!");
					resists_tele = TRUE;
				}
			}

			if (!resists_tele)
			{
				if (seen) obvious = TRUE;

				do_dist = dam;
			}

			dam = 0;
			break;
		}
		case GF_TURN_UNDEAD:
		{
			if (r_ptr->flags3 & (RF3_UNDEAD))
			{
				if (seen) obvious = TRUE;

				if (is_original_ap_and_seen(caster_ptr, m_ptr)) r_ptr->r_flags3 |= (RF3_UNDEAD);

				do_fear = damroll(3, (dam / 2)) + 1;
				if (r_ptr->level > randint1((dam - 10) < 1 ? 1 : (dam - 10)) + 10)
				{
					note = _("には効果がなかった。", " is unaffected.");
					obvious = FALSE;
					do_fear = 0;
				}
			}
			else
			{
				skipped = TRUE;
			}

			dam = 0;
			break;
		}
		case GF_TURN_EVIL:
		{
			if (r_ptr->flags3 & (RF3_EVIL))
			{
				if (seen) obvious = TRUE;

				if (is_original_ap_and_seen(caster_ptr, m_ptr)) r_ptr->r_flags3 |= (RF3_EVIL);

				do_fear = damroll(3, (dam / 2)) + 1;
				if (r_ptr->level > randint1((dam - 10) < 1 ? 1 : (dam - 10)) + 10)
				{
					note = _("には効果がなかった。", " is unaffected.");
					obvious = FALSE;
					do_fear = 0;
				}
			}
			else
			{
				skipped = TRUE;
			}

			dam = 0;
			break;
		}
		case GF_TURN_ALL:
		{
			if (seen) obvious = TRUE;

			do_fear = damroll(3, (dam / 2)) + 1;
			if ((r_ptr->flags1 & (RF1_UNIQUE)) ||
				(r_ptr->flags3 & (RF3_NO_FEAR)) ||
				(r_ptr->level > randint1((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
			{
				note = _("には効果がなかった。", " is unaffected.");
				obvious = FALSE;
				do_fear = 0;
			}

			dam = 0;
			break;
		}
		case GF_DISP_UNDEAD:
		{
			if (r_ptr->flags3 & (RF3_UNDEAD))
			{
				if (seen) obvious = TRUE;

				/* Learn about type */
				if (is_original_ap_and_seen(caster_ptr, m_ptr)) r_ptr->r_flags3 |= (RF3_UNDEAD);

				note = _("は身震いした。", " shudders.");
				note_dies = _("はドロドロに溶けた！", " dissolves!");
			}
			else
			{
				skipped = TRUE;
				dam = 0;
			}

			break;
		}
		case GF_DISP_EVIL:
		{
			if (r_ptr->flags3 & (RF3_EVIL))
			{
				if (seen) obvious = TRUE;

				if (is_original_ap_and_seen(caster_ptr, m_ptr)) r_ptr->r_flags3 |= (RF3_EVIL);

				note = _("は身震いした。", " shudders.");
				note_dies = _("はドロドロに溶けた！", " dissolves!");
			}
			else
			{
				skipped = TRUE;
				dam = 0;
			}

			break;
		}
		case GF_DISP_GOOD:
		{
			if (r_ptr->flags3 & (RF3_GOOD))
			{
				if (seen) obvious = TRUE;

				if (is_original_ap_and_seen(caster_ptr, m_ptr)) r_ptr->r_flags3 |= (RF3_GOOD);

				note = _("は身震いした。", " shudders.");
				note_dies = _("はドロドロに溶けた！", " dissolves!");
			}
			else
			{
				skipped = TRUE;
				dam = 0;
			}

			break;
		}
		case GF_DISP_LIVING:
		{
			if (monster_living(m_ptr->r_idx))
			{
				if (seen) obvious = TRUE;

				note = _("は身震いした。", " shudders.");
				note_dies = _("はドロドロに溶けた！", " dissolves!");
			}
			else
			{
				skipped = TRUE;
				dam = 0;
			}

			break;
		}
		case GF_DISP_DEMON:
		{
			if (r_ptr->flags3 & (RF3_DEMON))
			{
				if (seen) obvious = TRUE;

				if (is_original_ap_and_seen(caster_ptr, m_ptr)) r_ptr->r_flags3 |= (RF3_DEMON);

				note = _("は身震いした。", " shudders.");
				note_dies = _("はドロドロに溶けた！", " dissolves!");
			}
			else
			{
				skipped = TRUE;
				dam = 0;
			}

			break;
		}
		case GF_DISP_ALL:
		{
			if (seen) obvious = TRUE;
			note = _("は身震いした。", " shudders.");
			note_dies = _("はドロドロに溶けた！", " dissolves!");
			break;
		}
		case GF_DRAIN_MANA:
		{
			if (seen) obvious = TRUE;
			if ((r_ptr->flags4 & ~(RF4_NOMAGIC_MASK)) || (r_ptr->a_ability_flags1 & ~(RF5_NOMAGIC_MASK)) || (r_ptr->a_ability_flags2 & ~(RF6_NOMAGIC_MASK)))
			{
				if (who > 0)
				{
					if (m_caster_ptr->hp < m_caster_ptr->maxhp)
					{
						m_caster_ptr->hp += dam;
						if (m_caster_ptr->hp > m_caster_ptr->maxhp) m_caster_ptr->hp = m_caster_ptr->maxhp;
						if (caster_ptr->health_who == who) caster_ptr->redraw |= (PR_HEALTH);
						if (caster_ptr->riding == who) caster_ptr->redraw |= (PR_UHEALTH);

						if (see_s_msg)
						{
							monster_desc(caster_ptr, killer, m_caster_ptr, 0);
							msg_format(_("%^sは気分が良さそうだ。", "%^s appears healthier."), killer);
						}
					}
				}
				else
				{
					msg_format(_("%sから精神エネルギーを吸いとった。", "You draw psychic energy from %s."), m_name);
					(void)hp_player(caster_ptr, dam);
				}
			}
			else
			{
				if (see_s_msg) msg_format(_("%sには効果がなかった。", "%s is unaffected."), m_name);
			}

			dam = 0;
			break;
		}
		case GF_MIND_BLAST:
		{
			if (seen) obvious = TRUE;
			if (!who) msg_format(_("%sをじっと睨んだ。", "You gaze intently at %s."), m_name);

			if ((r_ptr->flags1 & RF1_UNIQUE) ||
				(r_ptr->flags3 & RF3_NO_CONF) ||
				(r_ptr->level > randint1((caster_lev - 10) < 1 ? 1 : (caster_lev - 10)) + 10))
			{
				if (r_ptr->flags3 & (RF3_NO_CONF))
				{
					if (is_original_ap_and_seen(caster_ptr, m_ptr)) r_ptr->r_flags3 |= (RF3_NO_CONF);
				}

				note = _("には効果がなかった。", " is unaffected.");
				dam = 0;
			}
			else if (r_ptr->flags2 & RF2_EMPTY_MIND)
			{
				if (is_original_ap_and_seen(caster_ptr, m_ptr)) r_ptr->r_flags2 |= (RF2_EMPTY_MIND);
				note = _("には完全な耐性がある！", " is immune.");
				dam = 0;
			}
			else if (r_ptr->flags2 & RF2_WEIRD_MIND)
			{
				if (is_original_ap_and_seen(caster_ptr, m_ptr)) r_ptr->r_flags2 |= (RF2_WEIRD_MIND);
				note = _("には耐性がある。", " resists.");
				dam /= 3;
			}
			else
			{
				note = _("は精神攻撃を食らった。", " is blasted by psionic energy.");
				note_dies = _("の精神は崩壊し、肉体は抜け殻となった。", " collapses, a mindless husk.");

				if (who > 0) do_conf = randint0(4) + 4;
				else do_conf = randint0(8) + 8;
			}

			break;
		}
		case GF_BRAIN_SMASH:
		{
			if (seen) obvious = TRUE;
			if (!who) msg_format(_("%sをじっと睨んだ。", "You gaze intently at %s."), m_name);

			if ((r_ptr->flags1 & RF1_UNIQUE) ||
				(r_ptr->flags3 & RF3_NO_CONF) ||
				(r_ptr->level > randint1((caster_lev - 10) < 1 ? 1 : (caster_lev - 10)) + 10))
			{
				if (r_ptr->flags3 & (RF3_NO_CONF))
				{
					if (is_original_ap_and_seen(caster_ptr, m_ptr)) r_ptr->r_flags3 |= (RF3_NO_CONF);
				}

				note = _("には効果がなかった。", " is unaffected.");
				dam = 0;
			}
			else if (r_ptr->flags2 & RF2_EMPTY_MIND)
			{
				if (is_original_ap_and_seen(caster_ptr, m_ptr)) r_ptr->r_flags2 |= (RF2_EMPTY_MIND);
				note = _("には完全な耐性がある！", " is immune.");
				dam = 0;
			}
			else if (r_ptr->flags2 & RF2_WEIRD_MIND)
			{
				if (is_original_ap_and_seen(caster_ptr, m_ptr)) r_ptr->r_flags2 |= (RF2_WEIRD_MIND);
				note = _("には耐性がある！", " resists!");
				dam /= 3;
			}
			else
			{
				note = _("は精神攻撃を食らった。", " is blasted by psionic energy.");
				note_dies = _("の精神は崩壊し、肉体は抜け殻となった。", " collapses, a mindless husk.");
				if (who > 0)
				{
					do_conf = randint0(4) + 4;
					do_stun = randint0(4) + 4;
				}
				else
				{
					do_conf = randint0(8) + 8;
					do_stun = randint0(8) + 8;
				}
				(void)set_monster_slow(caster_ptr, g_ptr->m_idx, MON_SLOW(m_ptr) + 10);
			}

			break;
		}
		case GF_CAUSE_1:
		{
			if (seen) obvious = TRUE;
			if (!who) msg_format(_("%sを指差して呪いをかけた。", "You point at %s and curse."), m_name);
			if (randint0(100 + (caster_lev / 2)) < (r_ptr->level + 35))
			{
				note = _("には効果がなかった。", " is unaffected.");
				dam = 0;
			}

			break;
		}
		case GF_CAUSE_2:
		{
			if (seen) obvious = TRUE;
			if (!who) msg_format(_("%sを指差して恐ろしげに呪いをかけた。", "You point at %s and curse horribly."), m_name);

			if (randint0(100 + (caster_lev / 2)) < (r_ptr->level + 35))
			{
				note = _("には効果がなかった。", " is unaffected.");
				dam = 0;
			}

			break;
		}
		case GF_CAUSE_3:
		{
			if (seen) obvious = TRUE;
			if (!who) msg_format(_("%sを指差し、恐ろしげに呪文を唱えた！", "You point at %s, incanting terribly!"), m_name);

			if (randint0(100 + (caster_lev / 2)) < (r_ptr->level + 35))
			{
				note = _("には効果がなかった。", " is unaffected.");
				dam = 0;
			}

			break;
		}
		case GF_CAUSE_4:
		{
			if (seen) obvious = TRUE;
			if (!who)
				msg_format(_("%sの秘孔を突いて、「お前は既に死んでいる」と叫んだ。",
					"You point at %s, screaming the word, 'DIE!'."), m_name);

			if ((randint0(100 + (caster_lev / 2)) < (r_ptr->level + 35)) && ((who <= 0) || (m_caster_ptr->r_idx != MON_KENSHIROU)))
			{
				note = _("には効果がなかった。", " is unaffected.");
				dam = 0;
			}
			break;
		}
		case GF_HAND_DOOM:
		{
			if (seen) obvious = TRUE;
			if (r_ptr->flags1 & RF1_UNIQUE)
			{
				note = _("には効果がなかった。", " is unaffected.");
				dam = 0;
			}
			else
			{
				if ((who > 0) ? ((caster_lev + randint1(dam)) > (r_ptr->level + 10 + randint1(20))) :
					(((caster_lev / 2) + randint1(dam)) > (r_ptr->level + randint1(200))))
				{
					dam = ((40 + randint1(20)) * m_ptr->hp) / 100;

					if (m_ptr->hp < dam) dam = m_ptr->hp - 1;
				}
				else
				{
					/* todo 乱数で破滅のを弾いた結果が「耐性を持っている」ことになるのはおかしい */
					note = _("は耐性を持っている！", "resists!");
					dam = 0;
				}
			}

			break;
		}
		case GF_CAPTURE:
		{
			int nokori_hp;
			if ((floor_ptr->inside_quest && (quest[floor_ptr->inside_quest].type == QUEST_TYPE_KILL_ALL) && !is_pet(m_ptr)) ||
				(r_ptr->flags1 & (RF1_UNIQUE)) || (r_ptr->flags7 & (RF7_NAZGUL)) || (r_ptr->flags7 & (RF7_UNIQUE2)) || (r_ptr->flags1 & RF1_QUESTOR) || m_ptr->parent_m_idx)
			{
				msg_format(_("%sには効果がなかった。", "%s is unaffected."), m_name);
				skipped = TRUE;
				break;
			}

			if (is_pet(m_ptr)) nokori_hp = m_ptr->maxhp * 4L;
			else if ((caster_ptr->pclass == CLASS_BEASTMASTER) && monster_living(m_ptr->r_idx))
				nokori_hp = m_ptr->maxhp * 3 / 10;
			else
				nokori_hp = m_ptr->maxhp * 3 / 20;

			if (m_ptr->hp >= nokori_hp)
			{
				msg_format(_("もっと弱らせないと。", "You need to weaken %s more."), m_name);
				skipped = TRUE;
			}
			else if (m_ptr->hp < randint0(nokori_hp))
			{
				if (m_ptr->mflag2 & MFLAG2_CHAMELEON) choose_new_monster(caster_ptr, g_ptr->m_idx, FALSE, MON_CHAMELEON);
				msg_format(_("%sを捕えた！", "You capture %^s!"), m_name);
				cap_mon = m_ptr->r_idx;
				cap_mspeed = m_ptr->mspeed;
				cap_hp = m_ptr->hp;
				cap_maxhp = m_ptr->max_maxhp;
				cap_nickname = m_ptr->nickname;
				if (g_ptr->m_idx == caster_ptr->riding)
				{
					if (rakuba(caster_ptr, -1, FALSE))
					{
						msg_format(_("地面に落とされた。", "You have fallen from %s."), m_name);
					}
				}

				delete_monster_idx(caster_ptr, g_ptr->m_idx);

				return TRUE;
			}
			else
			{
				msg_format(_("うまく捕まえられなかった。", "You failed to capture %s."), m_name);
				skipped = TRUE;
			}

			break;
		}
		case GF_ATTACK:
		{
			return py_attack(caster_ptr, y, x, dam);
		}
		case GF_ENGETSU:
		{
			int effect = 0;
			bool done = TRUE;

			if (seen) obvious = TRUE;
			if (r_ptr->flags2 & RF2_EMPTY_MIND)
			{
				note = _("には効果がなかった。", " is unaffected.");
				dam = 0;
				skipped = TRUE;
				if (is_original_ap_and_seen(caster_ptr, m_ptr)) r_ptr->r_flags2 |= (RF2_EMPTY_MIND);
				break;
			}
			if (MON_CSLEEP(m_ptr))
			{
				note = _("には効果がなかった。", " is unaffected.");
				dam = 0;
				skipped = TRUE;
				break;
			}

			if (one_in_(5)) effect = 1;
			else if (one_in_(4)) effect = 2;
			else if (one_in_(3)) effect = 3;
			else done = FALSE;

			if (effect == 1)
			{
				if ((r_ptr->flags1 & RF1_UNIQUE) ||
					(r_ptr->level > randint1((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
				{
					note = _("には効果がなかった。", " is unaffected.");
					obvious = FALSE;
				}
				else
				{
					if (set_monster_slow(caster_ptr, g_ptr->m_idx, MON_SLOW(m_ptr) + 50))
					{
						note = _("の動きが遅くなった。", " starts moving slower.");
					}
				}
			}
			else if (effect == 2)
			{
				do_stun = damroll((caster_ptr->lev / 10) + 3, (dam)) + 1;
				if ((r_ptr->flags1 & (RF1_UNIQUE)) ||
					(r_ptr->level > randint1((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
				{
					do_stun = 0;
					note = _("には効果がなかった。", " is unaffected.");
					obvious = FALSE;
				}
			}
			else if (effect == 3)
			{
				if ((r_ptr->flags1 & RF1_UNIQUE) ||
					(r_ptr->flags3 & RF3_NO_SLEEP) ||
					(r_ptr->level > randint1((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
				{
					if (r_ptr->flags3 & RF3_NO_SLEEP)
					{
						if (is_original_ap_and_seen(caster_ptr, m_ptr)) r_ptr->r_flags3 |= (RF3_NO_SLEEP);
					}

					note = _("には効果がなかった。", " is unaffected.");
					obvious = FALSE;
				}
				else
				{
					/* Go to sleep (much) later */
					note = _("は眠り込んでしまった！", " falls asleep!");
					do_sleep = 500;
				}
			}

			if (!done)
			{
				note = _("には効果がなかった。", " is unaffected.");
			}

			dam = 0;
			break;
		}
		case GF_GENOCIDE:
		{
			if (seen) obvious = TRUE;
			if (genocide_aux(caster_ptr, g_ptr->m_idx, dam, !who, (r_ptr->level + 1) / 2, _("モンスター消滅", "Genocide One")))
			{
				if (seen_msg) msg_format(_("%sは消滅した！", "%^s disappeared!"), m_name);
				chg_virtue(caster_ptr, V_VITALITY, -1);
				return TRUE;
			}

			skipped = TRUE;
			break;
		}
		case GF_PHOTO:
		{
			if (!who)
				msg_format(_("%sを写真に撮った。", "You take a photograph of %s."), m_name);

			if (r_ptr->flags3 & (RF3_HURT_LITE))
			{
				if (seen) obvious = TRUE;

				if (is_original_ap_and_seen(caster_ptr, m_ptr)) r_ptr->r_flags3 |= (RF3_HURT_LITE);

				note = _("は光に身をすくめた！", " cringes from the light!");
				note_dies = _("は光を受けてしぼんでしまった！", " shrivels away in the light!");
			}
			else
			{
				dam = 0;
			}

			photo = m_ptr->r_idx;
			break;
		}
		case GF_BLOOD_CURSE:
		{
			if (seen) obvious = TRUE;
			break;
		}
		case GF_CRUSADE:
		{
			bool success = FALSE;
			if (seen) obvious = TRUE;

			if ((r_ptr->flags3 & (RF3_GOOD)) && !floor_ptr->inside_arena)
			{
				if (r_ptr->flags3 & (RF3_NO_CONF)) dam -= 50;
				if (dam < 1) dam = 1;

				if (is_pet(m_ptr))
				{
					note = _("の動きが速くなった。", " starts moving faster.");
					(void)set_monster_fast(caster_ptr, g_ptr->m_idx, MON_FAST(m_ptr) + 100);
					success = TRUE;
				}
				else if ((r_ptr->flags1 & (RF1_QUESTOR)) ||
					(r_ptr->flags1 & (RF1_UNIQUE)) ||
					(m_ptr->mflag2 & MFLAG2_NOPET) ||
					(caster_ptr->cursed & TRC_AGGRAVATE) ||
					((r_ptr->level + 10) > randint1(dam)))
				{
					if (one_in_(4)) m_ptr->mflag2 |= MFLAG2_NOPET;
				}
				else
				{
					note = _("を支配した。", " is tamed!");
					set_pet(caster_ptr, m_ptr);
					(void)set_monster_fast(caster_ptr, g_ptr->m_idx, MON_FAST(m_ptr) + 100);

					if (is_original_ap_and_seen(caster_ptr, m_ptr)) r_ptr->r_flags3 |= (RF3_GOOD);
					success = TRUE;
				}
			}

			if (!success)
			{
				if (!(r_ptr->flags3 & RF3_NO_FEAR))
				{
					do_fear = randint1(90) + 10;
				}
				else if (is_original_ap_and_seen(caster_ptr, m_ptr))
					r_ptr->r_flags3 |= (RF3_NO_FEAR);
			}

			dam = 0;
			break;
		}
		case GF_WOUNDS:
		{
			if (seen) obvious = TRUE;

			if (randint0(100 + dam) < (r_ptr->level + 50))
			{
				note = _("には効果がなかった。", " is unaffected.");
				dam = 0;
			}
			break;
		}
		default:
		{
			skipped = TRUE;
			dam = 0;
			break;
		}
		}
	}

	if (skipped) return FALSE;

	if (r_ptr->flags1 & (RF1_UNIQUE)) do_poly = FALSE;
	if (r_ptr->flags1 & RF1_QUESTOR) do_poly = FALSE;
	if (caster_ptr->riding && (g_ptr->m_idx == caster_ptr->riding))
		do_poly = FALSE;

	if (((r_ptr->flags1 & (RF1_UNIQUE | RF1_QUESTOR)) || (r_ptr->flags7 & RF7_NAZGUL)) && !caster_ptr->phase_out)
	{
		if (who && (dam > m_ptr->hp)) dam = m_ptr->hp;
	}

	if (!who && slept)
	{
		if (!(r_ptr->flags3 & RF3_EVIL) || one_in_(5)) chg_virtue(caster_ptr, V_COMPASSION, -1);
		if (!(r_ptr->flags3 & RF3_EVIL) || one_in_(5)) chg_virtue(caster_ptr, V_HONOUR, -1);
	}

	tmp = dam;
	dam = mon_damage_mod(caster_ptr, m_ptr, dam, (bool)(typ == GF_PSY_SPEAR));
	if ((tmp > 0) && (dam == 0)) note = _("はダメージを受けていない。", " is unharmed.");

	if (dam > m_ptr->hp)
	{
		note = note_dies;
	}
	else
	{
		if (do_stun &&
			!(r_ptr->flagsr & (RFR_RES_SOUN | RFR_RES_WALL)) &&
			!(r_ptr->flags3 & RF3_NO_STUN))
		{
			if (seen) obvious = TRUE;

			if (MON_STUNNED(m_ptr))
			{
				note = _("はひどくもうろうとした。", " is more dazed.");
				tmp = MON_STUNNED(m_ptr) + (do_stun / 2);
			}
			else
			{
				note = _("はもうろうとした。", " is dazed.");
				tmp = do_stun;
			}

			(void)set_monster_stunned(caster_ptr, g_ptr->m_idx, tmp);
			get_angry = TRUE;
		}

		if (do_conf &&
			!(r_ptr->flags3 & RF3_NO_CONF) &&
			!(r_ptr->flagsr & RFR_EFF_RES_CHAO_MASK))
		{
			if (seen) obvious = TRUE;

			if (MON_CONFUSED(m_ptr))
			{
				note = _("はさらに混乱したようだ。", " looks more confused.");
				tmp = MON_CONFUSED(m_ptr) + (do_conf / 2);
			}
			else
			{
				note = _("は混乱したようだ。", " looks confused.");
				tmp = do_conf;
			}

			(void)set_monster_confused(caster_ptr, g_ptr->m_idx, tmp);
			get_angry = TRUE;
		}

		if (do_time)
		{
			if (seen) obvious = TRUE;

			if (do_time >= m_ptr->maxhp) do_time = m_ptr->maxhp - 1;

			if (do_time)
			{
				note = _("は弱くなったようだ。", " seems weakened.");
				m_ptr->maxhp -= do_time;
				if ((m_ptr->hp - dam) > m_ptr->maxhp) dam = m_ptr->hp - m_ptr->maxhp;
			}

			get_angry = TRUE;
		}

		if (do_poly && (randint1(90) > r_ptr->level))
		{
			if (polymorph_monster(caster_ptr, y, x))
			{
				if (seen) obvious = TRUE;

				note = _("が変身した！", " changes!");
				dam = 0;
			}

			m_ptr = &floor_ptr->m_list[g_ptr->m_idx];
			r_ptr = &r_info[m_ptr->r_idx];
		}

		if (do_dist)
		{
			if (seen) obvious = TRUE;

			note = _("が消え去った！", " disappears!");

			if (!who) chg_virtue(caster_ptr, V_VALOUR, -1);

			teleport_away(caster_ptr, g_ptr->m_idx, do_dist,
				(!who ? TELEPORT_DEC_VALOUR : 0L) | TELEPORT_PASSIVE);

			y = m_ptr->fy;
			x = m_ptr->fx;
			g_ptr = &floor_ptr->grid_array[y][x];
		}

		if (do_fear)
		{
			(void)set_monster_monfear(caster_ptr, g_ptr->m_idx, MON_MONFEAR(m_ptr) + do_fear);
			get_angry = TRUE;
		}
	}

	if (typ == GF_DRAIN_MANA)
	{
		/* Drain mana does nothing */
	}

	/* If another monster did the damage, hurt the monster by hand */
	else if (who)
	{
		if (caster_ptr->health_who == g_ptr->m_idx) caster_ptr->redraw |= (PR_HEALTH);
		if (caster_ptr->riding == g_ptr->m_idx) caster_ptr->redraw |= (PR_UHEALTH);

		(void)set_monster_csleep(caster_ptr, g_ptr->m_idx, 0);
		m_ptr->hp -= dam;
		if (m_ptr->hp < 0)
		{
			bool sad = FALSE;

			if (is_pet(m_ptr) && !(m_ptr->ml))
				sad = TRUE;

			if (known && note)
			{
				monster_desc(caster_ptr, m_name, m_ptr, MD_TRUE_NAME);
				if (see_s_msg)
				{
					msg_format("%^s%s", m_name, note);
				}
				else
				{
					floor_ptr->monster_noise = TRUE;
				}
			}

			if (who > 0) monster_gain_exp(caster_ptr, who, m_ptr->r_idx);

			monster_death(caster_ptr, g_ptr->m_idx, FALSE);
			delete_monster_idx(caster_ptr, g_ptr->m_idx);
			if (sad)
			{
				msg_print(_("少し悲しい気分がした。", "You feel sad for a moment."));
			}
		}
		else
		{
			if (note && seen_msg)
				msg_format("%^s%s", m_name, note);
			else if (see_s_msg)
			{
				message_pain(caster_ptr, g_ptr->m_idx, dam);
			}
			else
			{
				floor_ptr->monster_noise = TRUE;
			}

			if (do_sleep) (void)set_monster_csleep(caster_ptr, g_ptr->m_idx, do_sleep);
		}
	}
	else if (heal_leper)
	{
		if (seen_msg)
			msg_print(_("不潔な病人は病気が治った！", "The Mangy looking leper is healed!"));

		if (record_named_pet && is_pet(m_ptr) && m_ptr->nickname)
		{
			char m2_name[MAX_NLEN];

			monster_desc(caster_ptr, m2_name, m_ptr, MD_INDEF_VISIBLE);
			exe_write_diary(caster_ptr, DIARY_NAMED_PET, RECORD_NAMED_PET_HEAL_LEPER, m2_name);
		}

		delete_monster_idx(caster_ptr, g_ptr->m_idx);
	}

	/* If the player did it, give him experience, check fear */
	else
	{
		bool fear = FALSE;
		if (mon_take_hit(caster_ptr, g_ptr->m_idx, dam, &fear, note_dies))
		{
			/* Dead monster */
		}
		else
		{
			if (do_sleep) anger_monster(caster_ptr, m_ptr);

			if (note && seen_msg)
				msg_format(_("%s%s", "%^s%s"), m_name, note);
			else if (known && (dam || !do_fear))
			{
				message_pain(caster_ptr, g_ptr->m_idx, dam);
			}

			if (((dam > 0) || get_angry) && !do_sleep)
				anger_monster(caster_ptr, m_ptr);

			if ((fear || do_fear) && seen)
			{
				sound(SOUND_FLEE);
				msg_format(_("%^sは恐怖して逃げ出した！", "%^s flees in terror!"), m_name);
			}

			if (do_sleep) (void)set_monster_csleep(caster_ptr, g_ptr->m_idx, do_sleep);
		}
	}

	if ((typ == GF_BLOOD_CURSE) && one_in_(4))
	{
		blood_curse_to_enemy(caster_ptr, who);
	}

	if (caster_ptr->phase_out)
	{
		caster_ptr->health_who = g_ptr->m_idx;
		caster_ptr->redraw |= (PR_HEALTH);
		handle_stuff(caster_ptr);
	}

	if (m_ptr->r_idx) update_monster(caster_ptr, g_ptr->m_idx, FALSE);

	lite_spot(caster_ptr, y, x);
	if ((caster_ptr->monster_race_idx == m_ptr->r_idx) && (seen || !m_ptr->r_idx))
	{
		caster_ptr->window |= (PW_MONSTER);
	}

	if ((dam > 0) && !is_pet(m_ptr) && !is_friendly(m_ptr))
	{
		if (!who)
		{
			if (!(flg & PROJECT_NO_HANGEKI))
			{
				set_target(m_ptr, monster_target_y, monster_target_x);
			}
		}
		else if ((who > 0) && is_pet(m_caster_ptr) && !player_bold(caster_ptr, m_ptr->target_y, m_ptr->target_x))
		{
			set_target(m_ptr, m_caster_ptr->fy, m_caster_ptr->fx);
		}
	}

	if (caster_ptr->riding && (caster_ptr->riding == g_ptr->m_idx) && (dam > 0))
	{
		if (m_ptr->hp > m_ptr->maxhp / 3) dam = (dam + 1) / 2;
		rakubadam_m = (dam > 200) ? 200 : dam;
	}

	if (photo)
	{
		object_type *q_ptr;
		object_type forge;
		q_ptr = &forge;
		object_prep(q_ptr, lookup_kind(TV_STATUE, SV_PHOTO));
		q_ptr->pval = photo;
		q_ptr->ident |= (IDENT_FULL_KNOWN);
		(void)drop_near(caster_ptr, q_ptr, -1, caster_ptr->y, caster_ptr->x);
	}

	project_m_n++;
	project_m_x = x;
	project_m_y = y;
	return (obvious);
}


/*!
 * @brief 汎用的なビーム/ボルト/ボール系によるプレイヤーへの効果処理 / Helper function for "project()" below.
 * @param who 魔法を発動したモンスター(0ならばプレイヤー) / Index of "source" monster (zero for "player")
 * @param who_name 効果を起こしたモンスターの名前
 * @param r 効果半径(ビーム/ボルト = 0 / ボール = 1以上) / Radius of explosion (0 = beam/bolt, 1 to 9 = ball)
 * @param y 目標Y座標 / Target y location (or location to travel "towards")
 * @param x 目標X座標 / Target x location (or location to travel "towards")
 * @param dam 基本威力 / Base damage roll to apply to affected monsters (or player)
 * @param typ 効果属性 / Type of damage to apply to monsters (and objects)
 * @param flg 効果フラグ
 * @param monspell 効果元のモンスター魔法ID
 * @return 何か一つでも効力があればTRUEを返す / TRUE if any "effects" of the projection were observed, else FALSE
 * @details
 * Handle a beam/bolt/ball causing damage to the player.
 * This routine takes a "source monster" (by index), a "distance", a default
 * "damage", and a "damage type".  See "project_m()" above.
 * If "rad" is non-zero, then the blast was centered elsewhere, and the damage
 * is reduced (see "project_m()" above).  This can happen if a monster breathes
 * at the player and hits a wall instead.
 * NOTE (Zangband): 'Bolt' attacks can be reflected back, so we need
 * to know if this is actually a ball or a bolt spell
 * We return "TRUE" if any "obvious" effects were observed.  XXX XXX Actually,
 * we just assume that the effects were obvious, for historical reasons.
 */
static bool project_p(MONSTER_IDX who, player_type *target_ptr, concptr who_name, int r, POSITION y, POSITION x, HIT_POINT dam, EFFECT_ID typ, BIT_FLAGS flg, int monspell)
{
	int k = 0;
	DEPTH rlev = 0;
	bool obvious = TRUE;
	bool blind = (target_ptr->blind ? TRUE : FALSE);
	bool fuzzy = FALSE;
	monster_type *m_ptr = NULL;
	GAME_TEXT m_name[MAX_NLEN];
	char killer[80];
	concptr act = NULL;
	int get_damage = 0;
	if (!player_bold(target_ptr, y, x)) return FALSE;

	if ((target_ptr->special_defense & NINJA_KAWARIMI) && dam && (randint0(55) < (target_ptr->lev * 3 / 5 + 20)) && who && (who != target_ptr->riding))
	{
		if (kawarimi(target_ptr, TRUE)) return FALSE;
	}

	if (!who) return FALSE;
	if (who == target_ptr->riding) return FALSE;

	if ((target_ptr->reflect || ((target_ptr->special_defense & KATA_FUUJIN) && !target_ptr->blind)) && (flg & PROJECT_REFLECTABLE) && !one_in_(10))
	{
		POSITION t_y, t_x;
		int max_attempts = 10;
		sound(SOUND_REFLECT);

		if (blind)
			msg_print(_("何かが跳ね返った！", "Something bounces!"));
		else if (target_ptr->special_defense & KATA_FUUJIN)
			msg_print(_("風の如く武器を振るって弾き返した！", "The attack bounces!"));
		else
			msg_print(_("攻撃が跳ね返った！", "The attack bounces!"));

		if (who > 0)
		{
			do
			{
				t_y = target_ptr->current_floor_ptr->m_list[who].fy - 1 + randint1(3);
				t_x = target_ptr->current_floor_ptr->m_list[who].fx - 1 + randint1(3);
				max_attempts--;
			} while (max_attempts && in_bounds2u(target_ptr->current_floor_ptr, t_y, t_x) && !projectable(target_ptr, target_ptr->y, target_ptr->x, t_y, t_x));

			if (max_attempts < 1)
			{
				t_y = target_ptr->current_floor_ptr->m_list[who].fy;
				t_x = target_ptr->current_floor_ptr->m_list[who].fx;
			}
		}
		else
		{
			t_y = target_ptr->y - 1 + randint1(3);
			t_x = target_ptr->x - 1 + randint1(3);
		}

		project(target_ptr, 0, 0, t_y, t_x, dam, typ, (PROJECT_STOP | PROJECT_KILL | PROJECT_REFLECTABLE), monspell);
		disturb(target_ptr, TRUE, TRUE);
		return TRUE;
	}

	if (dam > 1600) dam = 1600;

	dam = (dam + r) / (r + 1);
	if (blind) fuzzy = TRUE;

	if (who > 0)
	{
		m_ptr = &target_ptr->current_floor_ptr->m_list[who];
		rlev = (((&r_info[m_ptr->r_idx])->level >= 1) ? (&r_info[m_ptr->r_idx])->level : 1);
		monster_desc(target_ptr, m_name, m_ptr, 0);
		strcpy(killer, who_name);
	}
	else
	{
		switch (who)
		{
		case PROJECT_WHO_UNCTRL_POWER:
			strcpy(killer, _("制御できない力の氾流", "uncontrollable power storm"));
			break;

		case PROJECT_WHO_GLASS_SHARDS:
			strcpy(killer, _("ガラスの破片", "shards of glass"));
			break;

		default:
			strcpy(killer, _("罠", "a trap"));
			break;
		}
		strcpy(m_name, killer);
	}

	switch (typ)
	{
	case GF_ACID:
	{
		if (fuzzy) msg_print(_("酸で攻撃された！", "You are hit by acid!"));

		get_damage = acid_dam(target_ptr, dam, killer, monspell, FALSE);
		break;
	}
	case GF_FIRE:
	{
		if (fuzzy) msg_print(_("火炎で攻撃された！", "You are hit by fire!"));

		get_damage = fire_dam(target_ptr, dam, killer, monspell, FALSE);
		break;
	}
	case GF_COLD:
	{
		if (fuzzy) msg_print(_("冷気で攻撃された！", "You are hit by cold!"));

		get_damage = cold_dam(target_ptr, dam, killer, monspell, FALSE);
		break;
	}
	case GF_ELEC:
	{
		if (fuzzy) msg_print(_("電撃で攻撃された！", "You are hit by lightning!"));

		get_damage = elec_dam(target_ptr, dam, killer, monspell, FALSE);
		break;
	}
	case GF_POIS:
	{
		bool double_resist = is_oppose_pois(target_ptr);
		if (fuzzy) msg_print(_("毒で攻撃された！", "You are hit by poison!"));

		if (target_ptr->resist_pois) dam = (dam + 2) / 3;
		if (double_resist) dam = (dam + 2) / 3;

		if ((!(double_resist || target_ptr->resist_pois)) && one_in_(HURT_CHANCE) && !CHECK_MULTISHADOW(target_ptr))
		{
			do_dec_stat(target_ptr, A_CON);
		}

		get_damage = take_hit(target_ptr, DAMAGE_ATTACK, dam, killer, monspell);

		if (!(double_resist || target_ptr->resist_pois) && !CHECK_MULTISHADOW(target_ptr))
			set_poisoned(target_ptr, target_ptr->poisoned + randint0(dam) + 10);

		break;
	}
	case GF_NUKE:
	{
		bool double_resist = is_oppose_pois(target_ptr);
		if (fuzzy) msg_print(_("放射能で攻撃された！", "You are hit by radiation!"));

		if (target_ptr->resist_pois) dam = (2 * dam + 2) / 5;
		if (double_resist) dam = (2 * dam + 2) / 5;
		get_damage = take_hit(target_ptr, DAMAGE_ATTACK, dam, killer, monspell);
		if ((double_resist || target_ptr->resist_pois) || CHECK_MULTISHADOW(target_ptr))
			break;

		set_poisoned(target_ptr, target_ptr->poisoned + randint0(dam) + 10);

		if (one_in_(5)) /* 6 */
		{
			msg_print(_("奇形的な変身を遂げた！", "You undergo a freakish metamorphosis!"));
			if (one_in_(4)) /* 4 */
				do_poly_self(target_ptr);
			else
				status_shuffle(target_ptr);
		}

		if (one_in_(6))
		{
			inventory_damage(target_ptr, set_acid_destroy, 2);
		}

		break;
	}
	case GF_MISSILE:
	{
		if (fuzzy) msg_print(_("何かで攻撃された！", "You are hit by something!"));
		get_damage = take_hit(target_ptr, DAMAGE_ATTACK, dam, killer, monspell);
		break;
	}
	case GF_HOLY_FIRE:
	{
		if (fuzzy) msg_print(_("何かで攻撃された！", "You are hit by something!"));
		if (target_ptr->align > 10)
			dam /= 2;
		else if (target_ptr->align < -10)
			dam *= 2;
		get_damage = take_hit(target_ptr, DAMAGE_ATTACK, dam, killer, monspell);
		break;
	}
	case GF_HELL_FIRE:
	{
		if (fuzzy) msg_print(_("何かで攻撃された！", "You are hit by something!"));
		if (target_ptr->align > 10)
			dam *= 2;
		get_damage = take_hit(target_ptr, DAMAGE_ATTACK, dam, killer, monspell);
		break;
	}
	case GF_ARROW:
	{
		if (fuzzy)
		{
			msg_print(_("何か鋭いもので攻撃された！", "You are hit by something sharp!"));
		}
		else if ((target_ptr->inventory_list[INVEN_RARM].name1 == ART_ZANTETSU) || (target_ptr->inventory_list[INVEN_LARM].name1 == ART_ZANTETSU))
		{
			msg_print(_("矢を斬り捨てた！", "You cut down the arrow!"));
			break;
		}

		get_damage = take_hit(target_ptr, DAMAGE_ATTACK, dam, killer, monspell);
		break;
	}
	case GF_PLASMA:
	{
		if (fuzzy) msg_print(_("何かとても熱いもので攻撃された！", "You are hit by something *HOT*!"));
		get_damage = take_hit(target_ptr, DAMAGE_ATTACK, dam, killer, monspell);

		if (!target_ptr->resist_sound && !CHECK_MULTISHADOW(target_ptr))
		{
			int plus_stun = (randint1((dam > 40) ? 35 : (dam * 3 / 4 + 5)));
			(void)set_stun(target_ptr, target_ptr->stun + plus_stun);
		}

		if (!(target_ptr->resist_fire || is_oppose_fire(target_ptr) || target_ptr->immune_fire))
		{
			inventory_damage(target_ptr, set_acid_destroy, 3);
		}

		break;
	}
	case GF_NETHER:
	{
		if (fuzzy) msg_print(_("地獄の力で攻撃された！", "You are hit by nether forces!"));
		if (target_ptr->resist_neth)
		{
			if (!PRACE_IS_(target_ptr, RACE_SPECTRE))
			{
				dam *= 6; dam /= (randint1(4) + 7);
			}
		}
		else if (!CHECK_MULTISHADOW(target_ptr)) drain_exp(target_ptr, 200 + (target_ptr->exp / 100), 200 + (target_ptr->exp / 1000), 75);

		if (PRACE_IS_(target_ptr, RACE_SPECTRE) && !CHECK_MULTISHADOW(target_ptr))
		{
			msg_print(_("気分がよくなった。", "You feel invigorated!"));
			hp_player(target_ptr, dam / 4);
			learn_spell(target_ptr, monspell);
		}
		else
		{
			get_damage = take_hit(target_ptr, DAMAGE_ATTACK, dam, killer, monspell);
		}

		break;
	}
	case GF_WATER:
	{
		if (fuzzy) msg_print(_("何か湿ったもので攻撃された！", "You are hit by something wet!"));
		if (CHECK_MULTISHADOW(target_ptr))
		{
			get_damage = take_hit(target_ptr, DAMAGE_ATTACK, dam, killer, monspell);
			break;
		}

		if (!target_ptr->resist_sound && !target_ptr->resist_water)
		{
			set_stun(target_ptr, target_ptr->stun + randint1(40));
		}
		if (!target_ptr->resist_conf && !target_ptr->resist_water)
		{
			set_confused(target_ptr, target_ptr->confused + randint1(5) + 5);
		}

		if (one_in_(5) && !target_ptr->resist_water)
		{
			inventory_damage(target_ptr, set_cold_destroy, 3);
		}

		if (target_ptr->resist_water) get_damage /= 4;

		get_damage = take_hit(target_ptr, DAMAGE_ATTACK, dam, killer, monspell);
		break;
	}
	case GF_CHAOS:
	{
		if (fuzzy) msg_print(_("無秩序の波動で攻撃された！", "You are hit by a wave of anarchy!"));
		if (target_ptr->resist_chaos)
		{
			dam *= 6; dam /= (randint1(4) + 7);
		}

		if (CHECK_MULTISHADOW(target_ptr))
		{
			get_damage = take_hit(target_ptr, DAMAGE_ATTACK, dam, killer, monspell);
			break;
		}
		
		if (!target_ptr->resist_conf)
		{
			(void)set_confused(target_ptr, target_ptr->confused + randint0(20) + 10);
		}
		if (!target_ptr->resist_chaos)
		{
			(void)set_image(target_ptr, target_ptr->image + randint1(10));
			if (one_in_(3))
			{
				msg_print(_("あなたの身体はカオスの力で捻じ曲げられた！", "Your body is twisted by chaos!"));
				(void)gain_mutation(target_ptr, 0);
			}
		}
		if (!target_ptr->resist_neth && !target_ptr->resist_chaos)
		{
			drain_exp(target_ptr, 5000 + (target_ptr->exp / 100), 500 + (target_ptr->exp / 1000), 75);
		}

		if (!target_ptr->resist_chaos || one_in_(9))
		{
			inventory_damage(target_ptr, set_elec_destroy, 2);
			inventory_damage(target_ptr, set_fire_destroy, 2);
		}

		get_damage = take_hit(target_ptr, DAMAGE_ATTACK, dam, killer, monspell);
		break;
	}
	case GF_SHARDS:
	{
		if (fuzzy) msg_print(_("何か鋭いもので攻撃された！", "You are hit by something sharp!"));
		if (target_ptr->resist_shard)
		{
			dam *= 6; dam /= (randint1(4) + 7);
		}
		else if (!CHECK_MULTISHADOW(target_ptr))
		{
			(void)set_cut(target_ptr, target_ptr->cut + dam);
		}

		if (!target_ptr->resist_shard || one_in_(13))
		{
			inventory_damage(target_ptr, set_cold_destroy, 2);
		}

		get_damage = take_hit(target_ptr, DAMAGE_ATTACK, dam, killer, monspell);
		break;
	}
	case GF_SOUND:
	{
		if (fuzzy) msg_print(_("轟音で攻撃された！", "You are hit by a loud noise!"));
		if (target_ptr->resist_sound)
		{
			dam *= 5; dam /= (randint1(4) + 7);
		}
		else if (!CHECK_MULTISHADOW(target_ptr))
		{
			int plus_stun = (randint1((dam > 90) ? 35 : (dam / 3 + 5)));
			(void)set_stun(target_ptr, target_ptr->stun + plus_stun);
		}

		if (!target_ptr->resist_sound || one_in_(13))
		{
			inventory_damage(target_ptr, set_cold_destroy, 2);
		}

		get_damage = take_hit(target_ptr, DAMAGE_ATTACK, dam, killer, monspell);
		break;
	}
	case GF_CONFUSION:
	{
		if (fuzzy) msg_print(_("何か混乱するもので攻撃された！", "You are hit by something puzzling!"));
		if (target_ptr->resist_conf)
		{
			dam *= 5; dam /= (randint1(4) + 7);
		}
		else if (!CHECK_MULTISHADOW(target_ptr))
		{
			(void)set_confused(target_ptr, target_ptr->confused + randint1(20) + 10);
		}
		get_damage = take_hit(target_ptr, DAMAGE_ATTACK, dam, killer, monspell);
		break;
	}
	case GF_DISENCHANT:
	{
		if (fuzzy) msg_print(_("何かさえないもので攻撃された！", "You are hit by something static!"));
		if (target_ptr->resist_disen)
		{
			dam *= 6; dam /= (randint1(4) + 7);
		}
		else if (!CHECK_MULTISHADOW(target_ptr))
		{
			(void)apply_disenchant(target_ptr, 0);
		}
		get_damage = take_hit(target_ptr, DAMAGE_ATTACK, dam, killer, monspell);
		break;
	}
	case GF_NEXUS:
	{
		if (fuzzy) msg_print(_("何か奇妙なもので攻撃された！", "You are hit by something strange!"));
		if (target_ptr->resist_nexus)
		{
			dam *= 6; dam /= (randint1(4) + 7);
		}
		else if (!CHECK_MULTISHADOW(target_ptr))
		{
			apply_nexus(m_ptr, target_ptr);
		}

		get_damage = take_hit(target_ptr, DAMAGE_ATTACK, dam, killer, monspell);
		break;
	}
	case GF_FORCE:
	{
		if (fuzzy) msg_print(_("運動エネルギーで攻撃された！", "You are hit by kinetic force!"));
		if (!target_ptr->resist_sound && !CHECK_MULTISHADOW(target_ptr))
		{
			(void)set_stun(target_ptr, target_ptr->stun + randint1(20));
		}

		get_damage = take_hit(target_ptr, DAMAGE_ATTACK, dam, killer, monspell);
		break;
	}
	case GF_ROCKET:
	{
		if (fuzzy) msg_print(_("爆発があった！", "There is an explosion!"));
		if (!target_ptr->resist_sound && !CHECK_MULTISHADOW(target_ptr))
		{
			(void)set_stun(target_ptr, target_ptr->stun + randint1(20));
		}

		if (target_ptr->resist_shard)
		{
			dam /= 2;
		}
		else if (!CHECK_MULTISHADOW(target_ptr))
		{
			(void)set_cut(target_ptr, target_ptr->cut + (dam / 2));
		}

		if (!target_ptr->resist_shard || one_in_(12))
		{
			inventory_damage(target_ptr, set_cold_destroy, 3);
		}

		get_damage = take_hit(target_ptr, DAMAGE_ATTACK, dam, killer, monspell);
		break;
	}
	case GF_INERTIAL:
	{
		if (fuzzy) msg_print(_("何か遅いもので攻撃された！", "You are hit by something slow!"));
		if (!CHECK_MULTISHADOW(target_ptr)) (void)set_slow(target_ptr, target_ptr->slow + randint0(4) + 4, FALSE);

		get_damage = take_hit(target_ptr, DAMAGE_ATTACK, dam, killer, monspell);
		break;
	}
	case GF_LITE:
	{
		if (fuzzy) msg_print(_("何かで攻撃された！", "You are hit by something!"));
		if (target_ptr->resist_lite)
		{
			dam *= 4; dam /= (randint1(4) + 7);
		}
		else if (!blind && !target_ptr->resist_blind && !CHECK_MULTISHADOW(target_ptr))
		{
			(void)set_blind(target_ptr, target_ptr->blind + randint1(5) + 2);
		}

		if (PRACE_IS_(target_ptr, RACE_VAMPIRE) || (target_ptr->mimic_form == MIMIC_VAMPIRE))
		{
			if (!CHECK_MULTISHADOW(target_ptr)) msg_print(_("光で肉体が焦がされた！", "The light scorches your flesh!"));
			dam *= 2;
		}
		else if (PRACE_IS_(target_ptr, RACE_S_FAIRY))
		{
			dam = dam * 4 / 3;
		}

		if (target_ptr->wraith_form) dam *= 2;
		get_damage = take_hit(target_ptr, DAMAGE_ATTACK, dam, killer, monspell);

		if (!target_ptr->wraith_form || CHECK_MULTISHADOW(target_ptr))
			break;

		target_ptr->wraith_form = 0;
		msg_print(_("閃光のため非物質的な影の存在でいられなくなった。",
			"The light forces you out of your incorporeal shadow form."));

		target_ptr->redraw |= (PR_MAP | PR_STATUS);
		target_ptr->update |= (PU_MONSTERS);
		target_ptr->window |= (PW_OVERHEAD | PW_DUNGEON);
		break;
	}
	case GF_DARK:
	{
		if (fuzzy) msg_print(_("何かで攻撃された！", "You are hit by something!"));
		if (target_ptr->resist_dark)
		{
			dam *= 4; dam /= (randint1(4) + 7);

			if (PRACE_IS_(target_ptr, RACE_VAMPIRE) || (target_ptr->mimic_form == MIMIC_VAMPIRE) || target_ptr->wraith_form) dam = 0;
		}
		else if (!blind && !target_ptr->resist_blind && !CHECK_MULTISHADOW(target_ptr))
		{
			(void)set_blind(target_ptr, target_ptr->blind + randint1(5) + 2);
		}

		get_damage = take_hit(target_ptr, DAMAGE_ATTACK, dam, killer, monspell);
		break;
	}
	case GF_TIME:
	{
		if (fuzzy) msg_print(_("過去からの衝撃に攻撃された！", "You are hit by a blast from the past!"));

		if (target_ptr->resist_time)
		{
			dam *= 4;
			dam /= (randint1(4) + 7);
			msg_print(_("時間が通り過ぎていく気がする。", "You feel as if time is passing you by."));
			get_damage = take_hit(target_ptr, DAMAGE_ATTACK, dam, killer, monspell);
			break;
		}
		
		if (CHECK_MULTISHADOW(target_ptr))
		{
			get_damage = take_hit(target_ptr, DAMAGE_ATTACK, dam, killer, monspell);
			break;
		}

		switch (randint1(10))
		{
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		{
			if (target_ptr->prace == RACE_ANDROID) break;

			msg_print(_("人生が逆戻りした気がする。", "You feel like a chunk of the past has been ripped away."));
			lose_exp(target_ptr, 100 + (target_ptr->exp / 100) * MON_DRAIN_LIFE);
			break;
		}
		case 6:
		case 7:
		case 8:
		case 9:
		{
			switch (randint1(6))
			{
			case 1: k = A_STR; act = _("強く", "strong"); break;
			case 2: k = A_INT; act = _("聡明で", "bright"); break;
			case 3: k = A_WIS; act = _("賢明で", "wise"); break;
			case 4: k = A_DEX; act = _("器用で", "agile"); break;
			case 5: k = A_CON; act = _("健康で", "hale"); break;
			case 6: k = A_CHR; act = _("美しく", "beautiful"); break;
			}

			msg_format(_("あなたは以前ほど%sなくなってしまった...。", "You're not as %s as you used to be..."), act);
			target_ptr->stat_cur[k] = (target_ptr->stat_cur[k] * 3) / 4;
			if (target_ptr->stat_cur[k] < 3) target_ptr->stat_cur[k] = 3;

			target_ptr->update |= (PU_BONUS);
			break;
		}
		case 10:
		{
			msg_print(_("あなたは以前ほど力強くなくなってしまった...。", "You're not as powerful as you used to be..."));
			for (k = 0; k < A_MAX; k++)
			{
				target_ptr->stat_cur[k] = (target_ptr->stat_cur[k] * 7) / 8;
				if (target_ptr->stat_cur[k] < 3) target_ptr->stat_cur[k] = 3;
			}

			target_ptr->update |= (PU_BONUS);
			break;
		}
		}

		get_damage = take_hit(target_ptr, DAMAGE_ATTACK, dam, killer, monspell);
		break;
	}
	case GF_GRAVITY:
	{
		if (fuzzy) msg_print(_("何か重いもので攻撃された！", "You are hit by something heavy!"));
		msg_print(_("周辺の重力がゆがんだ。", "Gravity warps around you."));

		if (!CHECK_MULTISHADOW(target_ptr))
		{
			teleport_player(target_ptr, 5, TELEPORT_PASSIVE);
			if (!target_ptr->levitation)
				(void)set_slow(target_ptr, target_ptr->slow + randint0(4) + 4, FALSE);
			if (!(target_ptr->resist_sound || target_ptr->levitation))
			{
				int plus_stun = (randint1((dam > 90) ? 35 : (dam / 3 + 5)));
				(void)set_stun(target_ptr, target_ptr->stun + plus_stun);
			}
		}

		if (target_ptr->levitation)
		{
			dam = (dam * 2) / 3;
		}

		if (!target_ptr->levitation || one_in_(13))
		{
			inventory_damage(target_ptr, set_cold_destroy, 2);
		}

		get_damage = take_hit(target_ptr, DAMAGE_ATTACK, dam, killer, monspell);
		break;
	}
	case GF_DISINTEGRATE:
	{
		if (fuzzy) msg_print(_("純粋なエネルギーで攻撃された！", "You are hit by pure energy!"));

		get_damage = take_hit(target_ptr, DAMAGE_ATTACK, dam, killer, monspell);
		break;
	}
	case GF_OLD_HEAL:
	{
		if (fuzzy) msg_print(_("何らかの攻撃によって気分がよくなった。", "You are hit by something invigorating!"));

		(void)hp_player(target_ptr, dam);
		dam = 0;
		break;
	}
	case GF_OLD_SPEED:
	{
		if (fuzzy) msg_print(_("何かで攻撃された！", "You are hit by something!"));
		(void)set_fast(target_ptr, target_ptr->fast + randint1(5), FALSE);
		dam = 0;
		break;
	}
	case GF_OLD_SLOW:
	{
		if (fuzzy) msg_print(_("何か遅いもので攻撃された！", "You are hit by something slow!"));
		(void)set_slow(target_ptr, target_ptr->slow + randint0(4) + 4, FALSE);
		break;
	}
	case GF_OLD_SLEEP:
	{
		if (target_ptr->free_act)  break;
		if (fuzzy) msg_print(_("眠ってしまった！", "You fall asleep!"));

		if (ironman_nightmare)
		{
			msg_print(_("恐ろしい光景が頭に浮かんできた。", "A horrible vision enters your mind."));
			/* Have some nightmares */
			sanity_blast(target_ptr, NULL, FALSE);
		}

		set_paralyzed(target_ptr, target_ptr->paralyzed + dam);
		dam = 0;
		break;
	}
	case GF_MANA:
	case GF_SEEKER:
	case GF_SUPER_RAY:
	{
		if (fuzzy) msg_print(_("魔法のオーラで攻撃された！", "You are hit by an aura of magic!"));

		get_damage = take_hit(target_ptr, DAMAGE_ATTACK, dam, killer, monspell);
		break;
	}
	case GF_PSY_SPEAR:
	{
		if (fuzzy) msg_print(_("エネルギーの塊で攻撃された！", "You are hit by an energy!"));

		get_damage = take_hit(target_ptr, DAMAGE_FORCE, dam, killer, monspell);
		break;
	}
	case GF_METEOR:
	{
		if (fuzzy) msg_print(_("何かが空からあなたの頭上に落ちてきた！", "Something falls from the sky on you!"));

		get_damage = take_hit(target_ptr, DAMAGE_ATTACK, dam, killer, monspell);
		if (!target_ptr->resist_shard || one_in_(13))
		{
			if (!target_ptr->immune_fire) inventory_damage(target_ptr, set_fire_destroy, 2);
			inventory_damage(target_ptr, set_cold_destroy, 2);
		}

		break;
	}
	case GF_ICE:
	{
		if (fuzzy) msg_print(_("何か鋭く冷たいもので攻撃された！", "You are hit by something sharp and cold!"));

		get_damage = cold_dam(target_ptr, dam, killer, monspell, FALSE);
		if (CHECK_MULTISHADOW(target_ptr)) break;

		if (!target_ptr->resist_shard)
		{
			(void)set_cut(target_ptr, target_ptr->cut + damroll(5, 8));
		}

		if (!target_ptr->resist_sound)
		{
			(void)set_stun(target_ptr, target_ptr->stun + randint1(15));
		}

		if ((!(target_ptr->resist_cold || is_oppose_cold(target_ptr))) || one_in_(12))
		{
			if (!target_ptr->immune_cold) inventory_damage(target_ptr, set_cold_destroy, 3);
		}

		break;
	}
	case GF_DEATH_RAY:
	{
		if (fuzzy) msg_print(_("何か非常に冷たいもので攻撃された！", "You are hit by something extremely cold!"));

		if (target_ptr->mimic_form)
		{
			if (!(mimic_info[target_ptr->mimic_form].MIMIC_FLAGS & MIMIC_IS_NONLIVING))
				get_damage = take_hit(target_ptr, DAMAGE_ATTACK, dam, killer, monspell);

			break;
		}

		switch (target_ptr->prace)
		{
		case RACE_GOLEM:
		case RACE_SKELETON:
		case RACE_ZOMBIE:
		case RACE_VAMPIRE:
		case RACE_DEMON:
		case RACE_SPECTRE:
		{
			dam = 0;
			break;
		}
		default:
		{
			get_damage = take_hit(target_ptr, DAMAGE_ATTACK, dam, killer, monspell);
			break;
		}
		}

		break;
	}
	case GF_DRAIN_MANA:
	{
		if (CHECK_MULTISHADOW(target_ptr))
		{
			msg_print(_("攻撃は幻影に命中し、あなたには届かなかった。", "The attack hits Shadow, but you are unharmed!"));
			dam = 0;
			break;
		}
		
		if (target_ptr->csp == 0)
		{
			dam = 0;
			break;
		}

		if (who > 0)
			msg_format(_("%^sに精神エネルギーを吸い取られてしまった！", "%^s draws psychic energy from you!"), m_name);
		else
			msg_print(_("精神エネルギーを吸い取られてしまった！", "Your psychic energy is drawn!"));

		if (dam >= target_ptr->csp)
		{
			dam = target_ptr->csp;
			target_ptr->csp = 0;
			target_ptr->csp_frac = 0;
		}
		else
		{
			target_ptr->csp -= dam;
		}

		learn_spell(target_ptr, monspell);
		target_ptr->redraw |= (PR_MANA);
		target_ptr->window |= (PW_PLAYER | PW_SPELL);

		if ((who <= 0) || (m_ptr->hp >= m_ptr->maxhp))
		{
			dam = 0;
			break;
		}

		m_ptr->hp += dam;
		if (m_ptr->hp > m_ptr->maxhp) m_ptr->hp = m_ptr->maxhp;

		if (target_ptr->health_who == who) target_ptr->redraw |= (PR_HEALTH);
		if (target_ptr->riding == who) target_ptr->redraw |= (PR_UHEALTH);

		if (m_ptr->ml)
		{
			msg_format(_("%^sは気分が良さそうだ。", "%^s appears healthier."), m_name);
		}

		dam = 0;
		break;
	}
	case GF_MIND_BLAST:
	{
		if ((randint0(100 + rlev / 2) < MAX(5, target_ptr->skill_sav)) && !CHECK_MULTISHADOW(target_ptr))
		{
			msg_print(_("しかし効力を跳ね返した！", "You resist the effects!"));
			learn_spell(target_ptr, monspell);
			break;
		}

		if (CHECK_MULTISHADOW(target_ptr))
		{
			get_damage = take_hit(target_ptr, DAMAGE_ATTACK, dam, killer, monspell);
			break;
		}

		msg_print(_("霊的エネルギーで精神が攻撃された。", "Your mind is blasted by psionic energy."));
		if (!target_ptr->resist_conf)
		{
			(void)set_confused(target_ptr, target_ptr->confused + randint0(4) + 4);
		}

		if (!target_ptr->resist_chaos && one_in_(3))
		{
			(void)set_image(target_ptr, target_ptr->image + randint0(250) + 150);
		}

		target_ptr->csp -= 50;
		if (target_ptr->csp < 0)
		{
			target_ptr->csp = 0;
			target_ptr->csp_frac = 0;
		}

		target_ptr->redraw |= PR_MANA;
		get_damage = take_hit(target_ptr, DAMAGE_ATTACK, dam, killer, monspell);
		break;
	}
	case GF_BRAIN_SMASH:
	{
		if ((randint0(100 + rlev / 2) < MAX(5, target_ptr->skill_sav)) && !CHECK_MULTISHADOW(target_ptr))
		{
			msg_print(_("しかし効力を跳ね返した！", "You resist the effects!"));
			learn_spell(target_ptr, monspell);
			break;
		}

		if (!CHECK_MULTISHADOW(target_ptr))
		{
			msg_print(_("霊的エネルギーで精神が攻撃された。", "Your mind is blasted by psionic energy."));

			target_ptr->csp -= 100;
			if (target_ptr->csp < 0)
			{
				target_ptr->csp = 0;
				target_ptr->csp_frac = 0;
			}
			target_ptr->redraw |= PR_MANA;
		}

		get_damage = take_hit(target_ptr, DAMAGE_ATTACK, dam, killer, monspell);
		if (CHECK_MULTISHADOW(target_ptr)) break;

		if (!target_ptr->resist_blind)
		{
			(void)set_blind(target_ptr, target_ptr->blind + 8 + randint0(8));
		}

		if (!target_ptr->resist_conf)
		{
			(void)set_confused(target_ptr, target_ptr->confused + randint0(4) + 4);
		}

		if (!target_ptr->free_act)
		{
			(void)set_paralyzed(target_ptr, target_ptr->paralyzed + randint0(4) + 4);
		}

		(void)set_slow(target_ptr, target_ptr->slow + randint0(4) + 4, FALSE);

		while (randint0(100 + rlev / 2) > (MAX(5, target_ptr->skill_sav)))
			(void)do_dec_stat(target_ptr, A_INT);
		while (randint0(100 + rlev / 2) > (MAX(5, target_ptr->skill_sav)))
			(void)do_dec_stat(target_ptr, A_WIS);

		if (!target_ptr->resist_chaos)
		{
			(void)set_image(target_ptr, target_ptr->image + randint0(250) + 150);
		}

		break;
	}
	case GF_CAUSE_1:
	{
		if ((randint0(100 + rlev / 2) < target_ptr->skill_sav) && !CHECK_MULTISHADOW(target_ptr))
		{
			msg_print(_("しかし効力を跳ね返した！", "You resist the effects!"));
			learn_spell(target_ptr, monspell);
		}
		else
		{
			if (!CHECK_MULTISHADOW(target_ptr)) curse_equipment(target_ptr, 15, 0);
			get_damage = take_hit(target_ptr, DAMAGE_ATTACK, dam, killer, monspell);
		}
		break;
	}
	case GF_CAUSE_2:
	{
		if ((randint0(100 + rlev / 2) < target_ptr->skill_sav) && !CHECK_MULTISHADOW(target_ptr))
		{
			msg_print(_("しかし効力を跳ね返した！", "You resist the effects!"));
			learn_spell(target_ptr, monspell);
		}
		else
		{
			if (!CHECK_MULTISHADOW(target_ptr)) curse_equipment(target_ptr, 25, MIN(rlev / 2 - 15, 5));
			get_damage = take_hit(target_ptr, DAMAGE_ATTACK, dam, killer, monspell);
		}
		break;
	}
	case GF_CAUSE_3:
	{
		if ((randint0(100 + rlev / 2) < target_ptr->skill_sav) && !CHECK_MULTISHADOW(target_ptr))
		{
			msg_print(_("しかし効力を跳ね返した！", "You resist the effects!"));
			learn_spell(target_ptr, monspell);
		}
		else
		{
			if (!CHECK_MULTISHADOW(target_ptr)) curse_equipment(target_ptr, 33, MIN(rlev / 2 - 15, 15));
			get_damage = take_hit(target_ptr, DAMAGE_ATTACK, dam, killer, monspell);
		}
		break;
	}
	case GF_CAUSE_4:
	{
		if ((randint0(100 + rlev / 2) < target_ptr->skill_sav) && !(m_ptr->r_idx == MON_KENSHIROU) && !CHECK_MULTISHADOW(target_ptr))
		{
			msg_print(_("しかし秘孔を跳ね返した！", "You resist the effects!"));
			learn_spell(target_ptr, monspell);
		}
		else
		{
			get_damage = take_hit(target_ptr, DAMAGE_ATTACK, dam, killer, monspell);
			if (!CHECK_MULTISHADOW(target_ptr)) (void)set_cut(target_ptr, target_ptr->cut + damroll(10, 10));
		}

		break;
	}
	case GF_HAND_DOOM:
	{
		if ((randint0(100 + rlev / 2) < target_ptr->skill_sav) && !CHECK_MULTISHADOW(target_ptr))
		{
			msg_print(_("しかし効力を跳ね返した！", "You resist the effects!"));
			learn_spell(target_ptr, monspell);
		}
		else
		{
			if (!CHECK_MULTISHADOW(target_ptr))
			{
				msg_print(_("あなたは命が薄まっていくように感じた！", "You feel your life fade away!"));
				curse_equipment(target_ptr, 40, 20);
			}

			get_damage = take_hit(target_ptr, DAMAGE_ATTACK, dam, m_name, monspell);

			if (target_ptr->chp < 1) target_ptr->chp = 1;
		}

		break;
	}
	default:
	{
		dam = 0;
		break;
	}
	}

	revenge_store(target_ptr, get_damage);
	if ((target_ptr->tim_eyeeye || hex_spelling(target_ptr, HEX_EYE_FOR_EYE))
		&& (get_damage > 0) && !target_ptr->is_dead && (who > 0))
	{
		GAME_TEXT m_name_self[80];
		monster_desc(target_ptr, m_name_self, m_ptr, MD_PRON_VISIBLE | MD_POSSESSIVE | MD_OBJECTIVE);
		msg_format(_("攻撃が%s自身を傷つけた！", "The attack of %s has wounded %s!"), m_name, m_name_self);
		project(target_ptr, 0, 0, m_ptr->fy, m_ptr->fx, get_damage, GF_MISSILE, PROJECT_KILL, -1);
		if (target_ptr->tim_eyeeye) set_tim_eyeeye(target_ptr, target_ptr->tim_eyeeye - 5, TRUE);
	}

	if (target_ptr->riding && dam > 0)
	{
		rakubadam_p = (dam > 200) ? 200 : dam;
	}

	disturb(target_ptr, TRUE, TRUE);
	if ((target_ptr->special_defense & NINJA_KAWARIMI) && dam && who && (who != target_ptr->riding))
	{
		(void)kawarimi(target_ptr, FALSE);
	}

	return (obvious);
}


/*
 * Find the distance from (x, y) to a line.
 */
POSITION dist_to_line(POSITION y, POSITION x, POSITION y1, POSITION x1, POSITION y2, POSITION x2)
{
	POSITION py = y1 - y;
	POSITION px = x1 - x;
	POSITION ny = x2 - x1;
	POSITION nx = y1 - y2;
	POSITION pd = distance(y1, x1, y, x);
	POSITION nd = distance(y1, x1, y2, x2);

	if (pd > nd) return distance(y, x, y2, x2);

	nd = ((nd) ? ((py * ny + px * nx) / nd) : 0);
	return((nd >= 0) ? nd : 0 - nd);
}


/*
 *
 * Modified version of los() for calculation of disintegration balls.
 * Disintegration effects are stopped by permanent walls.
 */
bool in_disintegration_range(floor_type *floor_ptr, POSITION y1, POSITION x1, POSITION y2, POSITION x2)
{
	POSITION delta_y = y2 - y1;
	POSITION delta_x = x2 - x1;
	POSITION absolute_y = ABS(delta_y);
	POSITION absolute_x = ABS(delta_x);
	if ((absolute_x < 2) && (absolute_y < 2)) return TRUE;

	POSITION scanner_y;
	if (!delta_x)
	{
		/* South -- check for walls */
		if (delta_y > 0)
		{
			for (scanner_y = y1 + 1; scanner_y < y2; scanner_y++)
			{
				if (cave_stop_disintegration(floor_ptr, scanner_y, x1)) return FALSE;
			}
		}

		/* North -- check for walls */
		else
		{
			for (scanner_y = y1 - 1; scanner_y > y2; scanner_y--)
			{
				if (cave_stop_disintegration(floor_ptr, scanner_y, x1)) return FALSE;
			}
		}

		return TRUE;
	}

	/* Directly East/West */
	POSITION scanner_x;
	if (!delta_y)
	{
		/* East -- check for walls */
		if (delta_x > 0)
		{
			for (scanner_x = x1 + 1; scanner_x < x2; scanner_x++)
			{
				if (cave_stop_disintegration(floor_ptr, y1, scanner_x)) return FALSE;
			}
		}

		/* West -- check for walls */
		else
		{
			for (scanner_x = x1 - 1; scanner_x > x2; scanner_x--)
			{
				if (cave_stop_disintegration(floor_ptr, y1, scanner_x)) return FALSE;
			}
		}

		return TRUE;
	}

	POSITION sign_x = (delta_x < 0) ? -1 : 1;
	POSITION sign_y = (delta_y < 0) ? -1 : 1;
	if (absolute_x == 1)
	{
		if (absolute_y == 2)
		{
			if (!cave_stop_disintegration(floor_ptr, y1 + sign_y, x1)) return TRUE;
		}
	}
	else if (absolute_y == 1)
	{
		if (absolute_x == 2)
		{
			if (!cave_stop_disintegration(floor_ptr, y1, x1 + sign_x)) return TRUE;
		}
	}

	POSITION scale_factor_2 = (absolute_x * absolute_y);
	POSITION scale_factor_1 = scale_factor_2 << 1;
	POSITION fraction_y;
	POSITION m; /* Slope, or 1/Slope, of LOS */
	if (absolute_x >= absolute_y)
	{
		fraction_y = absolute_y * absolute_y;
		m = fraction_y << 1;
		scanner_x = x1 + sign_x;
		if (fraction_y == scale_factor_2)
		{
			scanner_y = y1 + sign_y;
			fraction_y -= scale_factor_1;
		}
		else
		{
			scanner_y = y1;
		}

		/* Note (below) the case (qy == f2), where */
		/* the LOS exactly meets the corner of a tile. */
		while (x2 - scanner_x)
		{
			if (cave_stop_disintegration(floor_ptr, scanner_y, scanner_x)) return FALSE;

			fraction_y += m;

			if (fraction_y < scale_factor_2)
			{
				scanner_x += sign_x;
			}
			else if (fraction_y > scale_factor_2)
			{
				scanner_y += sign_y;
				if (cave_stop_disintegration(floor_ptr, scanner_y, scanner_x)) return FALSE;
				fraction_y -= scale_factor_1;
				scanner_x += sign_x;
			}
			else
			{
				scanner_y += sign_y;
				fraction_y -= scale_factor_1;
				scanner_x += sign_x;
			}
		}

		return TRUE;
	}

	POSITION fraction_x = absolute_x * absolute_x;
	m = fraction_x << 1;
	scanner_y = y1 + sign_y;
	if (fraction_x == scale_factor_2)
	{
		scanner_x = x1 + sign_x;
		fraction_x -= scale_factor_1;
	}
	else
	{
		scanner_x = x1;
	}

	/* Note (below) the case (qx == f2), where */
	/* the LOS exactly meets the corner of a tile. */
	while (y2 - scanner_y)
	{
		if (cave_stop_disintegration(floor_ptr, scanner_y, scanner_x)) return FALSE;

		fraction_x += m;

		if (fraction_x < scale_factor_2)
		{
			scanner_y += sign_y;
		}
		else if (fraction_x > scale_factor_2)
		{
			scanner_x += sign_x;
			if (cave_stop_disintegration(floor_ptr, scanner_y, scanner_x)) return FALSE;
			fraction_x -= scale_factor_1;
			scanner_y += sign_y;
		}
		else
		{
			scanner_x += sign_x;
			fraction_x -= scale_factor_1;
			scanner_y += sign_y;
		}
	}

	return TRUE;
}


/*
 * breath shape
 */
void breath_shape(player_type *caster_ptr, u16b *path_g, int dist, int *pgrids, POSITION *gx, POSITION *gy, POSITION *gm, POSITION *pgm_rad, POSITION rad, POSITION y1, POSITION x1, POSITION y2, POSITION x2, EFFECT_ID typ)
{
	POSITION by = y1;
	POSITION bx = x1;
	int brad = 0;
	int brev = rad * rad / dist;
	int bdis = 0;
	int cdis;
	int path_n = 0;
	int mdis = distance(y1, x1, y2, x2) + rad;

	floor_type *floor_ptr = caster_ptr->current_floor_ptr;
	while (bdis <= mdis)
	{
		POSITION x, y;

		if ((0 < dist) && (path_n < dist))
		{
			POSITION ny = GRID_Y(path_g[path_n]);
			POSITION nx = GRID_X(path_g[path_n]);
			POSITION nd = distance(ny, nx, y1, x1);

			if (bdis >= nd)
			{
				by = ny;
				bx = nx;
				path_n++;
			}
		}

		/* Travel from center outward */
		for (cdis = 0; cdis <= brad; cdis++)
		{
			for (y = by - cdis; y <= by + cdis; y++)
			{
				for (x = bx - cdis; x <= bx + cdis; x++)
				{
					if (!in_bounds(floor_ptr, y, x)) continue;
					if (distance(y1, x1, y, x) != bdis) continue;
					if (distance(by, bx, y, x) != cdis) continue;

					switch (typ)
					{
					case GF_LITE:
					case GF_LITE_WEAK:
						/* Lights are stopped by opaque terrains */
						if (!los(caster_ptr, by, bx, y, x)) continue;
						break;
					case GF_DISINTEGRATE:
						/* Disintegration are stopped only by perma-walls */
						if (!in_disintegration_range(floor_ptr, by, bx, y, x)) continue;
						break;
					default:
						/* Ball explosions are stopped by walls */
						if (!projectable(caster_ptr, by, bx, y, x)) continue;
						break;
					}

					gy[*pgrids] = y;
					gx[*pgrids] = x;
					(*pgrids)++;
				}
			}
		}

		gm[bdis + 1] = *pgrids;
		brad = rad * (path_n + brev) / (dist + brev);
		bdis++;
	}

	*pgm_rad = bdis;
}


/*!
 * todo 似たような処理が山ほど並んでいる、何とかならないものか
 * @brief 汎用的なビーム/ボルト/ボール系処理のルーチン Generic "beam"/"bolt"/"ball" projection routine.
 * @param who 魔法を発動したモンスター(0ならばプレイヤー) / Index of "source" monster (zero for "player")
 * @param rad 効果半径(ビーム/ボルト = 0 / ボール = 1以上) / Radius of explosion (0 = beam/bolt, 1 to 9 = ball)
 * @param y 目標Y座標 / Target y location (or location to travel "towards")
 * @param x 目標X座標 / Target x location (or location to travel "towards")
 * @param dam 基本威力 / Base damage roll to apply to affected monsters (or player)
 * @param typ 効果属性 / Type of damage to apply to monsters (and objects)
 * @param flg 効果フラグ / Extra bit flags (see PROJECT_xxxx)
 * @param monspell 効果元のモンスター魔法ID
 * @return 何か一つでも効力があればTRUEを返す / TRUE if any "effects" of the projection were observed, else FALSE
 * @details
 * <pre>
 * Allows a monster (or player) to project a beam/bolt/ball of a given kind
 * towards a given location (optionally passing over the heads of interposing
 * monsters), and have it do a given amount of damage to the monsters (and
 * optionally objects) within the given radius of the final location.
 *
 * A "bolt" travels from source to target and affects only the target grid.
 * A "beam" travels from source to target, affecting all grids passed through.
 * A "ball" travels from source to the target, exploding at the target, and
 *   affecting everything within the given radius of the target location.
 *
 * Traditionally, a "bolt" does not affect anything on the ground, and does
 * not pass over the heads of interposing monsters, much like a traditional
 * missile, and will "stop" abruptly at the "target" even if no monster is
 * positioned there, while a "ball", on the other hand, passes over the heads
 * of monsters between the source and target, and affects everything except
 * the source monster which lies within the final radius, while a "beam"
 * affects every monster between the source and target, except for the casting
 * monster (or player), and rarely affects things on the ground.
 *
 * Two special flags allow us to use this function in special ways, the
 * "PROJECT_HIDE" flag allows us to perform "invisible" projections, while
 * the "PROJECT_JUMP" flag allows us to affect a specific grid, without
 * actually projecting from the source monster (or player).
 *
 * The player will only get "experience" for monsters killed by himself
 * Unique monsters can only be destroyed by attacks from the player
 *
 * Only 256 grids can be affected per projection, limiting the effective
 * "radius" of standard ball attacks to nine units (diameter nineteen).
 *
 * One can project in a given "direction" by combining PROJECT_THRU with small
 * offsets to the initial location (see "line_spell()"), or by calculating
 * "virtual targets" far away from the player.
 *
 * One can also use PROJECT_THRU to send a beam/bolt along an angled path,
 * continuing until it actually hits somethings (useful for "stone to mud").
 *
 * Bolts and Beams explode INSIDE walls, so that they can destroy doors.
 *
 * Balls must explode BEFORE hitting walls, or they would affect monsters
 * on both sides of a wall.  Some bug reports indicate that this is still
 * happening in 2.7.8 for Windows, though it appears to be impossible.
 *
 * We "pre-calculate" the blast area only in part for efficiency.
 * More importantly, this lets us do "explosions" from the "inside" out.
 * This results in a more logical distribution of "blast" treasure.
 * It also produces a better (in my opinion) animation of the explosion.
 * It could be (but is not) used to have the treasure dropped by monsters
 * in the middle of the explosion fall "outwards", and then be damaged by
 * the blast as it spreads outwards towards the treasure drop location.
 *
 * Walls and doors are included in the blast area, so that they can be
 * "burned" or "melted" in later versions.
 *
 * This algorithm is intended to maximize simplicity, not necessarily
 * efficiency, since this function is not a bottleneck in the code.
 *
 * We apply the blast effect from ground zero outwards, in several passes,
 * first affecting features, then objects, then monsters, then the player.
 * This allows walls to be removed before checking the object or monster
 * in the wall, and protects objects which are dropped by monsters killed
 * in the blast, and allows the player to see all affects before he is
 * killed or teleported away.  The semantics of this method are open to
 * various interpretations, but they seem to work well in practice.
 *
 * We process the blast area from ground-zero outwards to allow for better
 * distribution of treasure dropped by monsters, and because it provides a
 * pleasing visual effect at low cost.
 *
 * Note that the damage done by "ball" explosions decreases with distance.
 * This decrease is rapid, grids at radius "dist" take "1/dist" damage.
 *
 * Notice the "napalm" effect of "beam" weapons.  First they "project" to
 * the target, and then the damage "flows" along this beam of destruction.
 * The damage at every grid is the same as at the "center" of a "ball"
 * explosion, since the "beam" grids are treated as if they ARE at the
 * center of a "ball" explosion.
 *
 * Currently, specifying "beam" plus "ball" means that locations which are
 * covered by the initial "beam", and also covered by the final "ball", except
 * for the final grid (the epicenter of the ball), will be "hit twice", once
 * by the initial beam, and once by the exploding ball.  For the grid right
 * next to the epicenter, this results in 150% damage being done.  The center
 * does not have this problem, for the same reason the final grid in a "beam"
 * plus "bolt" does not -- it is explicitly removed.  Simply removing "beam"
 * grids which are covered by the "ball" will NOT work, as then they will
 * receive LESS damage than they should.  Do not combine "beam" with "ball".
 *
 * The array "gy[],gx[]" with current size "grids" is used to hold the
 * collected locations of all grids in the "blast area" plus "beam path".
 *
 * Note the rather complex usage of the "gm[]" array.  First, gm[0] is always
 * zero.  Second, for N>1, gm[N] is always the index (in gy[],gx[]) of the
 * first blast grid (see above) with radius "N" from the blast center.  Note
 * that only the first gm[1] grids in the blast area thus take full damage.
 * Also, note that gm[rad+1] is always equal to "grids", which is the total
 * number of blast grids.
 *
 * Note that once the projection is complete, (y2,x2) holds the final location
 * of bolts/beams, and the "epicenter" of balls.
 *
 * Note also that "rad" specifies the "inclusive" radius of projection blast,
 * so that a "rad" of "one" actually covers 5 or 9 grids, depending on the
 * implementation of the "distance" function.  Also, a bolt can be properly
 * viewed as a "ball" with a "rad" of "zero".
 *
 * Note that if no "target" is reached before the beam/bolt/ball travels the
 * maximum distance allowed (MAX_RANGE), no "blast" will be induced.  This
 * may be relevant even for bolts, since they have a "1x1" mini-blast.
 *
 * Note that for consistency, we "pretend" that the bolt actually takes "time"
 * to move from point A to point B, even if the player cannot see part of the
 * projection path.  Note that in general, the player will *always* see part
 * of the path, since it either starts at the player or ends on the player.
 *
 * Hack -- we assume that every "projection" is "self-illuminating".
 *
 * Hack -- when only a single monster is affected, we automatically track
 * (and recall) that monster, unless "PROJECT_JUMP" is used.
 *
 * Note that all projections now "explode" at their final destination, even
 * if they were being projected at a more distant destination.  This means
 * that "ball" spells will *always* explode.
 *
 * Note that we must call "handle_stuff()" after affecting terrain features
 * in the blast radius, in case the "illumination" of the grid was changed,
 * and "update_view()" and "update_monsters()" need to be called.
 * </pre>
 */
bool project(player_type *caster_ptr, MONSTER_IDX who, POSITION rad, POSITION y, POSITION x, HIT_POINT dam, EFFECT_ID typ, BIT_FLAGS flg, int monspell)
{
	int i, t, dist;
	POSITION y1, x1;
	POSITION y2, x2;
	POSITION by, bx;
	int dist_hack = 0;
	POSITION y_saver, x_saver; /* For reflecting monsters */
	int msec = delay_factor * delay_factor * delay_factor;
	bool notice = FALSE;
	bool visual = FALSE;
	bool drawn = FALSE;
	bool breath = FALSE;
	bool blind = caster_ptr->blind != 0;
	bool old_hide = FALSE;
	int path_n = 0;
	u16b path_g[512];
	int grids = 0;
	POSITION gx[1024], gy[1024];
	POSITION gm[32];
	POSITION gm_rad = rad;
	bool jump = FALSE;
	GAME_TEXT who_name[MAX_NLEN];
	bool see_s_msg = TRUE;
	who_name[0] = '\0';
	rakubadam_p = 0;
	rakubadam_m = 0;
	monster_target_y = caster_ptr->y;
	monster_target_x = caster_ptr->x;

	if (flg & (PROJECT_JUMP))
	{
		x1 = x;
		y1 = y;
		flg &= ~(PROJECT_JUMP);
		jump = TRUE;
	}
	else if (who <= 0)
	{
		x1 = caster_ptr->x;
		y1 = caster_ptr->y;
	}
	else if (who > 0)
	{
		x1 = caster_ptr->current_floor_ptr->m_list[who].fx;
		y1 = caster_ptr->current_floor_ptr->m_list[who].fy;
		monster_desc(caster_ptr, who_name, &caster_ptr->current_floor_ptr->m_list[who], MD_WRONGDOER_NAME);
	}
	else
	{
		x1 = x;
		y1 = y;
	}

	y_saver = y1;
	x_saver = x1;
	y2 = y;
	x2 = x;

	if (flg & (PROJECT_THRU))
	{
		if ((x1 == x2) && (y1 == y2))
		{
			flg &= ~(PROJECT_THRU);
		}
	}

	if (rad < 0)
	{
		rad = 0 - rad;
		breath = TRUE;
		if (flg & PROJECT_HIDE) old_hide = TRUE;
		flg |= PROJECT_HIDE;
	}

	for (dist = 0; dist < 32; dist++) gm[dist] = 0;

	y = y1;
	x = x1;
	dist = 0;
	if (flg & (PROJECT_BEAM))
	{
		gy[grids] = y;
		gx[grids] = x;
		grids++;
	}

	switch (typ)
	{
	case GF_LITE:
	case GF_LITE_WEAK:
		if (breath || (flg & PROJECT_BEAM)) flg |= (PROJECT_LOS);
		break;
	case GF_DISINTEGRATE:
		flg |= (PROJECT_GRID);
		if (breath || (flg & PROJECT_BEAM)) flg |= (PROJECT_DISI);
		break;
	}

	/* Calculate the projection path */
	path_n = project_path(caster_ptr, path_g, (project_length ? project_length : MAX_RANGE), y1, x1, y2, x2, flg);
	handle_stuff(caster_ptr);

	if (typ == GF_SEEKER)
	{
		int j;
		int last_i = 0;
		project_m_n = 0;
		project_m_x = 0;
		project_m_y = 0;
		for (i = 0; i < path_n; ++i)
		{
			POSITION oy = y;
			POSITION ox = x;
			POSITION ny = GRID_Y(path_g[i]);
			POSITION nx = GRID_X(path_g[i]);
			y = ny;
			x = nx;
			gy[grids] = y;
			gx[grids] = x;
			grids++;

			if (!blind && !(flg & (PROJECT_HIDE)))
			{
				if (panel_contains(y, x) && player_has_los_bold(caster_ptr, y, x))
				{
					u16b p = bolt_pict(oy, ox, y, x, typ);
					TERM_COLOR a = PICT_A(p);
					SYMBOL_CODE c = PICT_C(p);
					print_rel(caster_ptr, c, a, y, x);
					move_cursor_relative(y, x);
					Term_fresh();
					Term_xtra(TERM_XTRA_DELAY, msec);
					lite_spot(caster_ptr, y, x);
					Term_fresh();
					if (flg & (PROJECT_BEAM))
					{
						p = bolt_pict(y, x, y, x, typ);
						a = PICT_A(p);
						c = PICT_C(p);
						print_rel(caster_ptr, c, a, y, x);
					}

					visual = TRUE;
				}
				else if (visual)
				{
					Term_xtra(TERM_XTRA_DELAY, msec);
				}
			}

			if (affect_item(caster_ptr, 0, 0, y, x, dam, GF_SEEKER))notice = TRUE;
			if (!is_mirror_grid(&caster_ptr->current_floor_ptr->grid_array[y][x]))
				continue;

			monster_target_y = y;
			monster_target_x = x;
			remove_mirror(caster_ptr, y, x);
			next_mirror(caster_ptr, &oy, &ox, y, x);
			path_n = i + project_path(caster_ptr, &(path_g[i + 1]), (project_length ? project_length : MAX_RANGE), y, x, oy, ox, flg);
			for (j = last_i; j <= i; j++)
			{
				y = GRID_Y(path_g[j]);
				x = GRID_X(path_g[j]);
				if (project_m(caster_ptr, 0, 0, y, x, dam, GF_SEEKER, flg, TRUE)) notice = TRUE;
				if (!who && (project_m_n == 1) && !jump && (caster_ptr->current_floor_ptr->grid_array[project_m_y][project_m_x].m_idx > 0))
				{
					monster_type *m_ptr = &caster_ptr->current_floor_ptr->m_list[caster_ptr->current_floor_ptr->grid_array[project_m_y][project_m_x].m_idx];
					if (m_ptr->ml)
					{
						if (!caster_ptr->image) monster_race_track(caster_ptr, m_ptr->ap_r_idx);
						health_track(caster_ptr, caster_ptr->current_floor_ptr->grid_array[project_m_y][project_m_x].m_idx);
					}
				}

				(void)affect_feature(caster_ptr, 0, 0, y, x, dam, GF_SEEKER);
			}

			last_i = i;
		}

		for (i = last_i; i < path_n; i++)
		{
			POSITION py, px;
			py = GRID_Y(path_g[i]);
			px = GRID_X(path_g[i]);
			if (project_m(caster_ptr, 0, 0, py, px, dam, GF_SEEKER, flg, TRUE))
				notice = TRUE;
			if (!who && (project_m_n == 1) && !jump) {
				if (caster_ptr->current_floor_ptr->grid_array[project_m_y][project_m_x].m_idx > 0)
				{
					monster_type *m_ptr = &caster_ptr->current_floor_ptr->m_list[caster_ptr->current_floor_ptr->grid_array[project_m_y][project_m_x].m_idx];

					if (m_ptr->ml)
					{
						if (!caster_ptr->image) monster_race_track(caster_ptr, m_ptr->ap_r_idx);
						health_track(caster_ptr, caster_ptr->current_floor_ptr->grid_array[project_m_y][project_m_x].m_idx);
					}
				}
			}

			(void)affect_feature(caster_ptr, 0, 0, py, px, dam, GF_SEEKER);
		}

		return notice;
	}
	else if (typ == GF_SUPER_RAY)
	{
		int j;
		int second_step = 0;
		project_m_n = 0;
		project_m_x = 0;
		project_m_y = 0;
		for (i = 0; i < path_n; ++i)
		{
			POSITION oy = y;
			POSITION ox = x;
			POSITION ny = GRID_Y(path_g[i]);
			POSITION nx = GRID_X(path_g[i]);
			y = ny;
			x = nx;
			gy[grids] = y;
			gx[grids] = x;
			grids++;
			{
				if (panel_contains(y, x) && player_has_los_bold(caster_ptr, y, x))
				{
					u16b p;
					TERM_COLOR a;
					SYMBOL_CODE c;
					p = bolt_pict(oy, ox, y, x, typ);
					a = PICT_A(p);
					c = PICT_C(p);
					print_rel(caster_ptr, c, a, y, x);
					move_cursor_relative(y, x);
					Term_fresh();
					Term_xtra(TERM_XTRA_DELAY, msec);
					lite_spot(caster_ptr, y, x);
					Term_fresh();
					if (flg & (PROJECT_BEAM))
					{
						p = bolt_pict(y, x, y, x, typ);
						a = PICT_A(p);
						c = PICT_C(p);
						print_rel(caster_ptr, c, a, y, x);
					}

					visual = TRUE;
				}
				else if (visual)
				{
					Term_xtra(TERM_XTRA_DELAY, msec);
				}
			}

			if (affect_item(caster_ptr, 0, 0, y, x, dam, GF_SUPER_RAY))notice = TRUE;
			if (!cave_have_flag_bold(caster_ptr->current_floor_ptr, y, x, FF_PROJECT))
			{
				if (second_step)continue;
				break;
			}

			if (is_mirror_grid(&caster_ptr->current_floor_ptr->grid_array[y][x]) && !second_step)
			{
				monster_target_y = y;
				monster_target_x = x;
				remove_mirror(caster_ptr, y, x);
				for (j = 0; j <= i; j++)
				{
					y = GRID_Y(path_g[j]);
					x = GRID_X(path_g[j]);
					(void)affect_feature(caster_ptr, 0, 0, y, x, dam, GF_SUPER_RAY);
				}

				path_n = i;
				second_step = i + 1;
				path_n += project_path(caster_ptr, &(path_g[path_n + 1]), (project_length ? project_length : MAX_RANGE), y, x, y - 1, x - 1, flg);
				path_n += project_path(caster_ptr, &(path_g[path_n + 1]), (project_length ? project_length : MAX_RANGE), y, x, y - 1, x, flg);
				path_n += project_path(caster_ptr, &(path_g[path_n + 1]), (project_length ? project_length : MAX_RANGE), y, x, y - 1, x + 1, flg);
				path_n += project_path(caster_ptr, &(path_g[path_n + 1]), (project_length ? project_length : MAX_RANGE), y, x, y, x - 1, flg);
				path_n += project_path(caster_ptr, &(path_g[path_n + 1]), (project_length ? project_length : MAX_RANGE), y, x, y, x + 1, flg);
				path_n += project_path(caster_ptr, &(path_g[path_n + 1]), (project_length ? project_length : MAX_RANGE), y, x, y + 1, x - 1, flg);
				path_n += project_path(caster_ptr, &(path_g[path_n + 1]), (project_length ? project_length : MAX_RANGE), y, x, y + 1, x, flg);
				path_n += project_path(caster_ptr, &(path_g[path_n + 1]), (project_length ? project_length : MAX_RANGE), y, x, y + 1, x + 1, flg);
			}
		}

		for (i = 0; i < path_n; i++)
		{
			POSITION py = GRID_Y(path_g[i]);
			POSITION px = GRID_X(path_g[i]);
			(void)project_m(caster_ptr, 0, 0, py, px, dam, GF_SUPER_RAY, flg, TRUE);
			if (!who && (project_m_n == 1) && !jump) {
				if (caster_ptr->current_floor_ptr->grid_array[project_m_y][project_m_x].m_idx > 0) {
					monster_type *m_ptr = &caster_ptr->current_floor_ptr->m_list[caster_ptr->current_floor_ptr->grid_array[project_m_y][project_m_x].m_idx];

					if (m_ptr->ml)
					{
						if (!caster_ptr->image) monster_race_track(caster_ptr, m_ptr->ap_r_idx);
						health_track(caster_ptr, caster_ptr->current_floor_ptr->grid_array[project_m_y][project_m_x].m_idx);
					}
				}
			}

			(void)affect_feature(caster_ptr, 0, 0, py, px, dam, GF_SUPER_RAY);
		}

		return notice;
	}

	for (i = 0; i < path_n; ++i)
	{
		POSITION oy = y;
		POSITION ox = x;
		POSITION ny = GRID_Y(path_g[i]);
		POSITION nx = GRID_X(path_g[i]);
		if (flg & PROJECT_DISI)
		{
			if (cave_stop_disintegration(caster_ptr->current_floor_ptr, ny, nx) && (rad > 0)) break;
		}
		else if (flg & PROJECT_LOS)
		{
			if (!cave_los_bold(caster_ptr->current_floor_ptr, ny, nx) && (rad > 0)) break;
		}
		else
		{
			if (!cave_have_flag_bold(caster_ptr->current_floor_ptr, ny, nx, FF_PROJECT) && (rad > 0)) break;
		}

		y = ny;
		x = nx;
		if (flg & (PROJECT_BEAM))
		{
			gy[grids] = y;
			gx[grids] = x;
			grids++;
		}

		if (!blind && !(flg & (PROJECT_HIDE | PROJECT_FAST)))
		{
			if (panel_contains(y, x) && player_has_los_bold(caster_ptr, y, x))
			{
				u16b p;
				TERM_COLOR a;
				SYMBOL_CODE c;
				p = bolt_pict(oy, ox, y, x, typ);
				a = PICT_A(p);
				c = PICT_C(p);
				print_rel(caster_ptr, c, a, y, x);
				move_cursor_relative(y, x);
				Term_fresh();
				Term_xtra(TERM_XTRA_DELAY, msec);
				lite_spot(caster_ptr, y, x);
				Term_fresh();
				if (flg & (PROJECT_BEAM))
				{
					p = bolt_pict(y, x, y, x, typ);
					a = PICT_A(p);
					c = PICT_C(p);
					print_rel(caster_ptr, c, a, y, x);
				}

				visual = TRUE;
			}
			else if (visual)
			{
				Term_xtra(TERM_XTRA_DELAY, msec);
			}
		}
	}

	path_n = i;
	by = y;
	bx = x;
	if (breath && !path_n)
	{
		breath = FALSE;
		gm_rad = rad;
		if (!old_hide)
		{
			flg &= ~(PROJECT_HIDE);
		}
	}

	gm[0] = 0;
	gm[1] = grids;
	dist = path_n;
	dist_hack = dist;
	project_length = 0;

	/* If we found a "target", explode there */
	if (dist <= MAX_RANGE)
	{
		if ((flg & (PROJECT_BEAM)) && (grids > 0)) grids--;

		/*
		 * Create a conical breath attack
		 *
		 *       ***
		 *   ********
		 * D********@**
		 *   ********
		 *       ***
		 */
		if (breath)
		{
			flg &= ~(PROJECT_HIDE);
			breath_shape(caster_ptr, path_g, dist, &grids, gx, gy, gm, &gm_rad, rad, y1, x1, by, bx, typ);
		}
		else
		{
			for (dist = 0; dist <= rad; dist++)
			{
				for (y = by - dist; y <= by + dist; y++)
				{
					for (x = bx - dist; x <= bx + dist; x++)
					{
						if (!in_bounds2(caster_ptr->current_floor_ptr, y, x)) continue;
						if (distance(by, bx, y, x) != dist) continue;

						switch (typ)
						{
						case GF_LITE:
						case GF_LITE_WEAK:
							if (!los(caster_ptr, by, bx, y, x)) continue;
							break;
						case GF_DISINTEGRATE:
							if (!in_disintegration_range(caster_ptr->current_floor_ptr, by, bx, y, x)) continue;
							break;
						default:
							if (!projectable(caster_ptr, by, bx, y, x)) continue;
							break;
						}

						gy[grids] = y;
						gx[grids] = x;
						grids++;
					}
				}

				gm[dist + 1] = grids;
			}
		}
	}

	if (!grids) return FALSE;

	if (!blind && !(flg & (PROJECT_HIDE)))
	{
		for (t = 0; t <= gm_rad; t++)
		{
			for (i = gm[t]; i < gm[t + 1]; i++)
			{
				y = gy[i];
				x = gx[i];
				if (panel_contains(y, x) && player_has_los_bold(caster_ptr, y, x))
				{
					u16b p;
					TERM_COLOR a;
					SYMBOL_CODE c;
					drawn = TRUE;
					p = bolt_pict(y, x, y, x, typ);
					a = PICT_A(p);
					c = PICT_C(p);
					print_rel(caster_ptr, c, a, y, x);
				}
			}

			move_cursor_relative(by, bx);
			Term_fresh();
			if (visual || drawn)
			{
				Term_xtra(TERM_XTRA_DELAY, msec);
			}
		}

		if (drawn)
		{
			for (i = 0; i < grids; i++)
			{
				y = gy[i];
				x = gx[i];
				if (panel_contains(y, x) && player_has_los_bold(caster_ptr, y, x))
				{
					lite_spot(caster_ptr, y, x);
				}
			}

			move_cursor_relative(by, bx);
			Term_fresh();
		}
	}

	update_creature(caster_ptr);

	if (flg & PROJECT_KILL)
	{
		see_s_msg = (who > 0) ? is_seen(&caster_ptr->current_floor_ptr->m_list[who]) :
			(!who ? TRUE : (player_can_see_bold(caster_ptr, y1, x1) && projectable(caster_ptr, caster_ptr->y, caster_ptr->x, y1, x1)));
	}

	if (flg & (PROJECT_GRID))
	{
		dist = 0;
		for (i = 0; i < grids; i++)
		{
			if (gm[dist + 1] == i) dist++;
			y = gy[i];
			x = gx[i];
			if (breath)
			{
				int d = dist_to_line(y, x, y1, x1, by, bx);
				if (affect_feature(caster_ptr, who, d, y, x, dam, typ)) notice = TRUE;
			}
			else
			{
				if (affect_feature(caster_ptr, who, dist, y, x, dam, typ)) notice = TRUE;
			}
		}
	}

	update_creature(caster_ptr);
	if (flg & (PROJECT_ITEM))
	{
		dist = 0;
		for (i = 0; i < grids; i++)
		{
			if (gm[dist + 1] == i) dist++;

			y = gy[i];
			x = gx[i];
			if (breath)
			{
				int d = dist_to_line(y, x, y1, x1, by, bx);
				if (affect_item(caster_ptr, who, d, y, x, dam, typ)) notice = TRUE;
			}
			else
			{
				if (affect_item(caster_ptr, who, dist, y, x, dam, typ)) notice = TRUE;
			}
		}
	}

	if (flg & (PROJECT_KILL))
	{
		project_m_n = 0;
		project_m_x = 0;
		project_m_y = 0;
		dist = 0;
		for (i = 0; i < grids; i++)
		{
			int effective_dist;
			if (gm[dist + 1] == i) dist++;

			y = gy[i];
			x = gx[i];
			if (grids <= 1)
			{
				monster_type *m_ptr = &caster_ptr->current_floor_ptr->m_list[caster_ptr->current_floor_ptr->grid_array[y][x].m_idx];
				monster_race *ref_ptr = &r_info[m_ptr->r_idx];
				if ((flg & PROJECT_REFLECTABLE) && caster_ptr->current_floor_ptr->grid_array[y][x].m_idx && (ref_ptr->flags2 & RF2_REFLECTING) &&
					((caster_ptr->current_floor_ptr->grid_array[y][x].m_idx != caster_ptr->riding) || !(flg & PROJECT_PLAYER)) &&
					(!who || dist_hack > 1) && !one_in_(10))
				{
					POSITION t_y, t_x;
					int max_attempts = 10;
					do
					{
						t_y = y_saver - 1 + randint1(3);
						t_x = x_saver - 1 + randint1(3);
						max_attempts--;
					} while (max_attempts && in_bounds2u(caster_ptr->current_floor_ptr, t_y, t_x) && !projectable(caster_ptr, y, x, t_y, t_x));

					if (max_attempts < 1)
					{
						t_y = y_saver;
						t_x = x_saver;
					}

					sound(SOUND_REFLECT);
					if (is_seen(m_ptr))
					{
						if ((m_ptr->r_idx == MON_KENSHIROU) || (m_ptr->r_idx == MON_RAOU))
							msg_print(_("「北斗神拳奥義・二指真空把！」", "The attack bounces!"));
						else if (m_ptr->r_idx == MON_DIO)
							msg_print(_("ディオ・ブランドーは指一本で攻撃を弾き返した！", "The attack bounces!"));
						else
							msg_print(_("攻撃は跳ね返った！", "The attack bounces!"));
					}

					if (is_original_ap_and_seen(caster_ptr, m_ptr)) ref_ptr->r_flags2 |= RF2_REFLECTING;

					if (player_bold(caster_ptr, y, x) || one_in_(2)) flg &= ~(PROJECT_PLAYER);
					else flg |= PROJECT_PLAYER;

					project(caster_ptr, caster_ptr->current_floor_ptr->grid_array[y][x].m_idx, 0, t_y, t_x, dam, typ, flg, monspell);
					continue;
				}
			}

			/* Find the closest point in the blast */
			if (breath)
			{
				effective_dist = dist_to_line(y, x, y1, x1, by, bx);
			}
			else
			{
				effective_dist = dist;
			}

			if (caster_ptr->riding && player_bold(caster_ptr, y, x))
			{
				if (flg & PROJECT_PLAYER)
				{
					if (flg & (PROJECT_BEAM | PROJECT_REFLECTABLE | PROJECT_AIMED))
					{
						/*
						 * A beam or bolt is well aimed
						 * at the PLAYER!
						 * So don't affects the mount.
						 */
						continue;
					}
					else
					{
						/*
						 * The spell is not well aimed,
						 * So partly affect the mount too.
						 */
						effective_dist++;
					}
				}

				/*
				 * This grid is the original target.
				 * Or aimed on your horse.
				 */
				else if (((y == y2) && (x == x2)) || (flg & PROJECT_AIMED))
				{
					/* Hit the mount with full damage */
				}

				/*
				 * Otherwise this grid is not the
				 * original target, it means that line
				 * of fire is obstructed by this
				 * monster.
				 */
				 /*
				  * A beam or bolt will hit either
				  * player or mount.  Choose randomly.
				  */
				else if (flg & (PROJECT_BEAM | PROJECT_REFLECTABLE))
				{
					if (one_in_(2))
					{
						/* Hit the mount with full damage */
					}
					else
					{
						flg |= PROJECT_PLAYER;
						continue;
					}
				}

				/*
				 * The spell is not well aimed, so
				 * partly affect both player and
				 * mount.
				 */
				else
				{
					effective_dist++;
				}
			}

			if (project_m(caster_ptr, who, effective_dist, y, x, dam, typ, flg, see_s_msg)) notice = TRUE;
		}

		/* Player affected one monster (without "jumping") */
		if (!who && (project_m_n == 1) && !jump)
		{
			x = project_m_x;
			y = project_m_y;
			if (caster_ptr->current_floor_ptr->grid_array[y][x].m_idx > 0)
			{
				monster_type *m_ptr = &caster_ptr->current_floor_ptr->m_list[caster_ptr->current_floor_ptr->grid_array[y][x].m_idx];

				if (m_ptr->ml)
				{
					if (!caster_ptr->image) monster_race_track(caster_ptr, m_ptr->ap_r_idx);
					health_track(caster_ptr, caster_ptr->current_floor_ptr->grid_array[y][x].m_idx);
				}
			}
		}
	}

	if (flg & (PROJECT_KILL))
	{
		dist = 0;
		for (i = 0; i < grids; i++)
		{
			int effective_dist;
			if (gm[dist + 1] == i) dist++;

			y = gy[i];
			x = gx[i];
			if (!player_bold(caster_ptr, y, x)) continue;

			/* Find the closest point in the blast */
			if (breath)
			{
				effective_dist = dist_to_line(y, x, y1, x1, by, bx);
			}
			else
			{
				effective_dist = dist;
			}

			if (caster_ptr->riding)
			{
				if (flg & PROJECT_PLAYER)
				{
					/* Hit the player with full damage */
				}

				/*
				 * Hack -- When this grid was not the
				 * original target, a beam or bolt
				 * would hit either player or mount,
				 * and should be choosen randomly.
				 *
				 * But already choosen to hit the
				 * mount at this point.
				 *
				 * Or aimed on your horse.
				 */
				else if (flg & (PROJECT_BEAM | PROJECT_REFLECTABLE | PROJECT_AIMED))
				{
					/*
					 * A beam or bolt is well aimed
					 * at the mount!
					 * So don't affects the player.
					 */
					continue;
				}
				else
				{
					/*
					 * The spell is not well aimed,
					 * So partly affect the player too.
					 */
					effective_dist++;
				}
			}

			if (project_p(who, caster_ptr, who_name, effective_dist, y, x, dam, typ, flg, monspell)) notice = TRUE;
		}
	}

	if (caster_ptr->riding)
	{
		GAME_TEXT m_name[MAX_NLEN];
		monster_desc(caster_ptr, m_name, &caster_ptr->current_floor_ptr->m_list[caster_ptr->riding], 0);
		if (rakubadam_m > 0)
		{
			if (rakuba(caster_ptr, rakubadam_m, FALSE))
			{
				msg_format(_("%^sに振り落とされた！", "%^s has thrown you off!"), m_name);
			}
		}

		if (caster_ptr->riding && rakubadam_p > 0)
		{
			if (rakuba(caster_ptr, rakubadam_p, FALSE))
			{
				msg_format(_("%^sから落ちてしまった！", "You have fallen from %s."), m_name);
			}
		}
	}

	return (notice);
}


/*!
 * @brief 鏡魔法「封魔結界」の効果処理
 * @param dam ダメージ量
 * @return 効果があったらTRUEを返す
 */
bool binding_field(player_type *caster_ptr, HIT_POINT dam)
{
	POSITION mirror_x[10], mirror_y[10]; /* 鏡はもっと少ない */
	int mirror_num = 0;	/* 鏡の数 */
	POSITION x, y;
	int msec = delay_factor * delay_factor*delay_factor;

	/* 三角形の頂点 */
	POSITION point_x[3];
	POSITION point_y[3];

	/* Default target of monsterspell is player */
	monster_target_y = caster_ptr->y;
	monster_target_x = caster_ptr->x;

	for (x = 0; x < caster_ptr->current_floor_ptr->width; x++)
	{
		for (y = 0; y < caster_ptr->current_floor_ptr->height; y++)
		{
			if (is_mirror_grid(&caster_ptr->current_floor_ptr->grid_array[y][x]) &&
				distance(caster_ptr->y, caster_ptr->x, y, x) <= MAX_RANGE &&
				distance(caster_ptr->y, caster_ptr->x, y, x) != 0 &&
				player_has_los_bold(caster_ptr, y, x) &&
				projectable(caster_ptr, caster_ptr->y, caster_ptr->x, y, x))
			{
				mirror_y[mirror_num] = y;
				mirror_x[mirror_num] = x;
				mirror_num++;
			}
		}
	}

	if (mirror_num < 2)return FALSE;

	point_x[0] = randint0(mirror_num);
	do {
		point_x[1] = randint0(mirror_num);
	} while (point_x[0] == point_x[1]);

	point_y[0] = mirror_y[point_x[0]];
	point_x[0] = mirror_x[point_x[0]];
	point_y[1] = mirror_y[point_x[1]];
	point_x[1] = mirror_x[point_x[1]];
	point_y[2] = caster_ptr->y;
	point_x[2] = caster_ptr->x;

	x = point_x[0] + point_x[1] + point_x[2];
	y = point_y[0] + point_y[1] + point_y[2];

	POSITION centersign = (point_x[0] * 3 - x)*(point_y[1] * 3 - y)
		- (point_y[0] * 3 - y)*(point_x[1] * 3 - x);
	if (centersign == 0)return FALSE;

	POSITION x1 = point_x[0] < point_x[1] ? point_x[0] : point_x[1];
	x1 = x1 < point_x[2] ? x1 : point_x[2];
	POSITION y1 = point_y[0] < point_y[1] ? point_y[0] : point_y[1];
	y1 = y1 < point_y[2] ? y1 : point_y[2];

	POSITION x2 = point_x[0] > point_x[1] ? point_x[0] : point_x[1];
	x2 = x2 > point_x[2] ? x2 : point_x[2];
	POSITION y2 = point_y[0] > point_y[1] ? point_y[0] : point_y[1];
	y2 = y2 > point_y[2] ? y2 : point_y[2];

	for (y = y1; y <= y2; y++)
	{
		for (x = x1; x <= x2; x++)
		{
			if (centersign*((point_x[0] - x)*(point_y[1] - y)
				- (point_y[0] - y)*(point_x[1] - x)) >= 0 &&
				centersign*((point_x[1] - x)*(point_y[2] - y)
					- (point_y[1] - y)*(point_x[2] - x)) >= 0 &&
				centersign*((point_x[2] - x)*(point_y[0] - y)
					- (point_y[2] - y)*(point_x[0] - x)) >= 0)
			{
				if (player_has_los_bold(caster_ptr, y, x) && projectable(caster_ptr, caster_ptr->y, caster_ptr->x, y, x))
				{
					if (!(caster_ptr->blind)
						&& panel_contains(y, x))
					{
						u16b p = bolt_pict(y, x, y, x, GF_MANA);
						print_rel(caster_ptr, PICT_C(p), PICT_A(p), y, x);
						move_cursor_relative(y, x);
						Term_fresh();
						Term_xtra(TERM_XTRA_DELAY, msec);
					}
				}
			}
		}
	}

	for (y = y1; y <= y2; y++)
	{
		for (x = x1; x <= x2; x++)
		{
			if (centersign*((point_x[0] - x)*(point_y[1] - y)
				- (point_y[0] - y)*(point_x[1] - x)) >= 0 &&
				centersign*((point_x[1] - x)*(point_y[2] - y)
					- (point_y[1] - y)*(point_x[2] - x)) >= 0 &&
				centersign*((point_x[2] - x)*(point_y[0] - y)
					- (point_y[2] - y)*(point_x[0] - x)) >= 0)
			{
				if (player_has_los_bold(caster_ptr, y, x) && projectable(caster_ptr, caster_ptr->y, caster_ptr->x, y, x))
				{
					(void)affect_feature(caster_ptr, 0, 0, y, x, dam, GF_MANA);
				}
			}
		}
	}

	for (y = y1; y <= y2; y++)
	{
		for (x = x1; x <= x2; x++)
		{
			if (centersign*((point_x[0] - x)*(point_y[1] - y)
				- (point_y[0] - y)*(point_x[1] - x)) >= 0 &&
				centersign*((point_x[1] - x)*(point_y[2] - y)
					- (point_y[1] - y)*(point_x[2] - x)) >= 0 &&
				centersign*((point_x[2] - x)*(point_y[0] - y)
					- (point_y[2] - y)*(point_x[0] - x)) >= 0)
			{
				if (player_has_los_bold(caster_ptr, y, x) && projectable(caster_ptr, caster_ptr->y, caster_ptr->x, y, x))
				{
					(void)affect_item(caster_ptr, 0, 0, y, x, dam, GF_MANA);
				}
			}
		}
	}

	for (y = y1; y <= y2; y++)
	{
		for (x = x1; x <= x2; x++)
		{
			if (centersign*((point_x[0] - x)*(point_y[1] - y)
				- (point_y[0] - y)*(point_x[1] - x)) >= 0 &&
				centersign*((point_x[1] - x)*(point_y[2] - y)
					- (point_y[1] - y)*(point_x[2] - x)) >= 0 &&
				centersign*((point_x[2] - x)*(point_y[0] - y)
					- (point_y[2] - y)*(point_x[0] - x)) >= 0)
			{
				if (player_has_los_bold(caster_ptr, y, x) && projectable(caster_ptr, caster_ptr->y, caster_ptr->x, y, x))
				{
					(void)project_m(caster_ptr, 0, 0, y, x, dam, GF_MANA,
						(PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL | PROJECT_JUMP), TRUE);
				}
			}
		}
	}

	if (one_in_(7))
	{
		msg_print(_("鏡が結界に耐えきれず、壊れてしまった。", "The field broke a mirror"));
		remove_mirror(caster_ptr, point_y[0], point_x[0]);
	}

	return TRUE;
}


/*!
 * @brief 鏡魔法「鏡の封印」の効果処理
 * @param dam ダメージ量
 * @return 効果があったらTRUEを返す
 */
void seal_of_mirror(player_type *caster_ptr, HIT_POINT dam)
{
	for (POSITION x = 0; x < caster_ptr->current_floor_ptr->width; x++)
	{
		for (POSITION y = 0; y < caster_ptr->current_floor_ptr->height; y++)
		{
			if (!is_mirror_grid(&caster_ptr->current_floor_ptr->grid_array[y][x]))
				continue;

			if (!project_m(caster_ptr, 0, 0, y, x, dam, GF_GENOCIDE,
				(PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL | PROJECT_JUMP), TRUE))
				continue;

			if (!caster_ptr->current_floor_ptr->grid_array[y][x].m_idx)
			{
				remove_mirror(caster_ptr, y, x);
			}
		}
	}
}


/*!
 * @brief 領域魔法に応じて技能の名称を返す。
 * @param tval 魔法書のtval
 * @return 領域魔法の技能名称を保管した文字列ポインタ
 */
concptr spell_category_name(OBJECT_TYPE_VALUE tval)
{
	switch (tval)
	{
	case TV_HISSATSU_BOOK:
		return _("必殺技", "art");
	case TV_LIFE_BOOK:
		return _("祈り", "prayer");
	case TV_MUSIC_BOOK:
		return _("歌", "song");
	default:
		return _("呪文", "spell");
	}
}
