﻿#include "system/angband.h"
#include "util/util.h"
#include "object/item-feeling.h"
#include "object/object-curse.h"
#include "object/object-flavor.h"
#include "object/object-hook.h"
#include "object/tr-types.h"
#include "object/trc-types.h"

#define MAX_CURSE 18
#define TRC_SPECIAL_MASK (TRC_TY_CURSE | TRC_AGGRAVATE)
#define TRC_HEAVY_MASK (TRC_TY_CURSE | TRC_AGGRAVATE | TRC_DRAIN_EXP | TRC_ADD_H_CURSE | TRC_CALL_DEMON | TRC_CALL_DRAGON | TRC_CALL_UNDEAD | TRC_TELEPORT)

/*!
 * @brief アイテムに付加される可能性のある呪いを指定する。
 * @param power 呪いの段階
 * @param o_ptr 呪いをかけられる装備オブジェクトの構造体参照ポインタ
 * @return 与える呪いのID
 */
BIT_FLAGS get_curse(int power, object_type *o_ptr)
{
	BIT_FLAGS new_curse;

	while (TRUE)
	{
		new_curse = (1 << (randint0(MAX_CURSE)+4));
		if (power == 2)
		{
			if (!(new_curse & TRC_HEAVY_MASK)) continue;
		}
		else if (power == 1)
		{
			if (new_curse & TRC_SPECIAL_MASK) continue;
		}
		else if (power == 0)
		{
			if (new_curse & TRC_HEAVY_MASK) continue;
		}

		if (new_curse == TRC_LOW_MELEE && !object_is_weapon(o_ptr)) continue;
		if (new_curse == TRC_LOW_AC && !object_is_armour(o_ptr)) continue;
		break;
	}

	return new_curse;
}


/*!
 * @brief 装備への呪い付加判定と付加処理
 * @param owner_ptr プレーヤーへの参照ポインタ
 * @param chance 呪いの基本確率
 * @param heavy_chance さらに重い呪いとなる確率
 * @return なし
 */
void curse_equipment(player_type *owner_ptr, PERCENTAGE chance, PERCENTAGE heavy_chance)
{
	if (randint1(100) > chance) return;

	object_type *o_ptr = &owner_ptr->inventory_list[INVEN_RARM + randint0(12)];
	if (!o_ptr->k_idx) return;
	BIT_FLAGS oflgs[TR_FLAG_SIZE];
	object_flags(o_ptr, oflgs);
	GAME_TEXT o_name[MAX_NLEN];
	object_desc(owner_ptr, o_name, o_ptr, (OD_OMIT_PREFIX | OD_NAME_ONLY));

	/* Extra, biased saving throw for blessed items */
	if (have_flag(oflgs, TR_BLESSED))
	{
#ifdef JP
		msg_format("祝福された%sは呪いを跳ね返した！", o_name);
#else
		msg_format("Your blessed %s resist%s cursing!", o_name, ((o_ptr->number > 1) ? "" : "s"));
#endif
		/* Hmmm -- can we wear multiple items? If not, this is unnecessary */
		return;
	}

	bool changed = FALSE;
	int curse_power = 0;
	if ((randint1(100) <= heavy_chance) &&
	    (object_is_artifact(o_ptr) || object_is_ego(o_ptr)))
	{
		if (!(o_ptr->curse_flags & TRC_HEAVY_CURSE))
			changed = TRUE;
		o_ptr->curse_flags |= TRC_HEAVY_CURSE;
		o_ptr->curse_flags |= TRC_CURSED;
		curse_power++;
	}
	else
	{
		if (!object_is_cursed(o_ptr))
			changed = TRUE;
		o_ptr->curse_flags |= TRC_CURSED;
	}

	if (heavy_chance >= 50) curse_power++;

	BIT_FLAGS new_curse = get_curse(curse_power, o_ptr);
	if (!(o_ptr->curse_flags & new_curse))
	{
		changed = TRUE;
		o_ptr->curse_flags |= new_curse;
	}

	if (changed)
	{
		msg_format(_("悪意に満ちた黒いオーラが%sをとりまいた...", "There is a malignant black aura surrounding %s..."), o_name);
		o_ptr->feeling = FEEL_NONE;
	}

	owner_ptr->update |= PU_BONUS;
}
