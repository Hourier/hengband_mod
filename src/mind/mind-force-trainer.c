﻿#include "mind/mind-force-trainer.h"
#include "cmd-action/cmd-pet.h"
#include "view/display-messages.h"

/*!
 * @brief 練気術師が「練気」で溜めた気の量を返す
 * @param caster_ptr プレーヤーの参照ポインタ
 * @return 現在溜まっている気の量
 */
MAGIC_NUM1 get_current_ki(player_type *caster_ptr)
{
    return caster_ptr->magic_num1[0];
}

/*!
 * @brief 練気術師において、気を溜める
 * @param caster_ptr プレーヤーの参照ポインタ
 * @param is_reset TRUEなら気の量をkiにセットし、FALSEなら加減算を行う
 * @param ki 気の量
 * @return なし
 */
void set_current_ki(player_type *caster_ptr, bool is_reset, MAGIC_NUM1 ki)
{
    if (is_reset) {
        caster_ptr->magic_num1[0] = ki;
        return;
    }

    caster_ptr->magic_num1[0] += ki;
}

bool clear_mind(player_type *creature_ptr)
{
    if (total_friends) {
        msg_print(_("今はペットを操ることに集中していないと。", "Your pets demand all of your attention."));
        return FALSE;
    }

    msg_print(_("少し頭がハッキリした。", "You feel your head clear a little."));

    creature_ptr->csp += (3 + creature_ptr->lev / 20);
    if (creature_ptr->csp >= creature_ptr->msp) {
        creature_ptr->csp = creature_ptr->msp;
        creature_ptr->csp_frac = 0;
    }

    creature_ptr->redraw |= (PR_MANA);
    return TRUE;
}
