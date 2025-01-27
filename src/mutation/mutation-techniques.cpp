﻿/*!
 * @brief 突然変異でのみ得ることができる特殊能力処理
 * @date 2020/07/04
 * @author Hourier
 */

#include "mutation/mutation-techniques.h"
#include "cmd-action/cmd-attack.h"
#include "floor/geometry.h"
#include "grid/feature.h"
#include "grid/grid.h"
#include "monster/monster-info.h"
#include "player/digestion-processor.h"
#include "player/player-move.h"
#include "player/player-status.h"
#include "system/floor-type-definition.h"
#include "system/grid-type-definition.h"
#include "system/monster-type-definition.h"
#include "system/player-type-definition.h"
#include "target/target-getter.h"
#include "util/bit-flags-calculator.h"
#include "view/display-messages.h"

/*!
 * 岩石食い
 * @param caster_ptr プレーヤーへの参照ポインタ
 * @return コマンドの入力方向に地形があればTRUE
 */
bool eat_rock(player_type *caster_ptr)
{
    DIRECTION dir;
    if (!get_direction(caster_ptr, &dir, false, false))
        return false;

    POSITION y = caster_ptr->y + ddy[dir];
    POSITION x = caster_ptr->x + ddx[dir];
    grid_type *g_ptr;
    g_ptr = &caster_ptr->current_floor_ptr->grid_array[y][x];
    feature_type *f_ptr, *mimic_f_ptr;
    f_ptr = &f_info[g_ptr->feat];
    mimic_f_ptr = &f_info[g_ptr->get_feat_mimic()];

    stop_mouth(caster_ptr);
    if (mimic_f_ptr->flags.has_not(FF::HURT_ROCK)) {
        msg_print(_("この地形は食べられない。", "You cannot eat this feature."));
    } else if (f_ptr->flags.has(FF::PERMANENT)) {
        msg_format(_("いてっ！この%sはあなたの歯より硬い！", "Ouch!  This %s is harder than your teeth!"), mimic_f_ptr->name.c_str());
    } else if (g_ptr->m_idx) {
        monster_type *m_ptr = &caster_ptr->current_floor_ptr->m_list[g_ptr->m_idx];
        msg_print(_("何かが邪魔しています！", "There's something in the way!"));

        if (!m_ptr->ml || !is_pet(m_ptr))
            do_cmd_attack(caster_ptr, y, x, HISSATSU_NONE);
    } else if (f_ptr->flags.has(FF::TREE)) {
        msg_print(_("木の味は好きじゃない！", "You don't like the woody taste!"));
    } else if (f_ptr->flags.has(FF::GLASS)) {
        msg_print(_("ガラスの味は好きじゃない！", "You don't like the glassy taste!"));
    } else if (f_ptr->flags.has(FF::DOOR) || f_ptr->flags.has(FF::CAN_DIG)) {
        (void)set_food(caster_ptr, caster_ptr->food + 3000);
    } else if (f_ptr->flags.has(FF::MAY_HAVE_GOLD) || f_ptr->flags.has(FF::HAS_GOLD)) {
        (void)set_food(caster_ptr, caster_ptr->food + 5000);
    } else {
        msg_format(_("この%sはとてもおいしい！", "This %s is very filling!"), mimic_f_ptr->name.c_str());
        (void)set_food(caster_ptr, caster_ptr->food + 10000);
    }

    cave_alter_feat(caster_ptr, y, x, FF::HURT_ROCK);
    (void)move_player_effect(caster_ptr, y, x, MPE_DONT_PICKUP);
    return true;
}
