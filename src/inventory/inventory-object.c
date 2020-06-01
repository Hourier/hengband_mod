﻿#include "inventory/inventory-object.h"
#include "object/object-flavor.h"
#include "object/object-value.h"
#include "object/object2.h" // 暫定、相互参照している.
#include "player/player-effects.h" // 暫定、相互参照している.
#include "view/object-describer.h"

void vary_item(player_type *owner_ptr, INVENTORY_IDX item, ITEM_NUMBER num)
{
    if (item >= 0) {
        inven_item_increase(owner_ptr, item, num);
        inven_item_describe(owner_ptr, item);
        inven_item_optimize(owner_ptr, item);
        return;
    }

    floor_type *floor_ptr = owner_ptr->current_floor_ptr;
    floor_item_increase(floor_ptr, 0 - item, num);
    floor_item_describe(owner_ptr, 0 - item);
    floor_item_optimize(owner_ptr, 0 - item);
}

/*!
 * @brief アイテムを増減させ残り所持数メッセージを表示する /
 * Increase the "number" of an item in the inventory
 * @param owner_ptr プレーヤーへの参照ポインタ
 * @param item 所持数を増やしたいプレイヤーのアイテム所持スロット
 * @param num 増やしたい量
 * @return なし
 */
void inven_item_increase(player_type *owner_ptr, INVENTORY_IDX item, ITEM_NUMBER num)
{
    object_type *o_ptr = &owner_ptr->inventory_list[item];
    num += o_ptr->number;
    if (num > 255)
        num = 255;
    else if (num < 0)
        num = 0;

    num -= o_ptr->number;
    if (num == 0)
        return;

    o_ptr->number += num;
    owner_ptr->total_weight += (num * o_ptr->weight);
    owner_ptr->update |= (PU_BONUS);
    owner_ptr->update |= (PU_MANA);
    owner_ptr->update |= (PU_COMBINE);
    owner_ptr->window |= (PW_INVEN | PW_EQUIP);

    if (o_ptr->number || !owner_ptr->ele_attack)
        return;
    if (!(item == INVEN_RARM) && !(item == INVEN_LARM))
        return;
    if (has_melee_weapon(owner_ptr, INVEN_RARM + INVEN_LARM - item))
        return;

    set_ele_attack(owner_ptr, 0, 0);
}

/*!
 * @brief 所持アイテムスロットから所持数のなくなったアイテムを消去する /
 * Erase an inventory slot if it has no more items
 * @param owner_ptr プレーヤーへの参照ポインタ
 * @param item 消去したいプレイヤーのアイテム所持スロット
 * @return なし
 */
void inven_item_optimize(player_type *owner_ptr, INVENTORY_IDX item)
{
    object_type *o_ptr = &owner_ptr->inventory_list[item];
    if (!o_ptr->k_idx)
        return;
    if (o_ptr->number)
        return;

    if (item >= INVEN_RARM) {
        owner_ptr->equip_cnt--;
        object_wipe(&owner_ptr->inventory_list[item]);
        owner_ptr->update |= PU_BONUS;
        owner_ptr->update |= PU_TORCH;
        owner_ptr->update |= PU_MANA;

        owner_ptr->window |= PW_EQUIP;
        owner_ptr->window |= PW_SPELL;
        return;
    }

    owner_ptr->inven_cnt--;
    int i;
    for (i = item; i < INVEN_PACK; i++) {
        owner_ptr->inventory_list[i] = owner_ptr->inventory_list[i + 1];
    }

    object_wipe(&owner_ptr->inventory_list[i]);
    owner_ptr->window |= PW_INVEN;
    owner_ptr->window |= PW_SPELL;
}

/*!
 * @brief 所持スロットから床下にオブジェクトを落とすメインルーチン /
 * Drop (some of) a non-cursed inventory/equipment item
 * @param owner_ptr プレーヤーへの参照ポインタ
 * @param item 所持テーブルのID
 * @param amt 落としたい個数
 * @return なし
 * @details
 * The object will be dropped "near" the current location
 */
void drop_from_inventory(player_type *owner_ptr, INVENTORY_IDX item, ITEM_NUMBER amt)
{
    object_type forge;
    object_type *q_ptr;
    object_type *o_ptr;
    GAME_TEXT o_name[MAX_NLEN];
    o_ptr = &owner_ptr->inventory_list[item];
    if (amt <= 0)
        return;

    if (amt > o_ptr->number)
        amt = o_ptr->number;

    if (item >= INVEN_RARM) {
        item = inven_takeoff(owner_ptr, item, amt);
        o_ptr = &owner_ptr->inventory_list[item];
    }

    q_ptr = &forge;
    object_copy(q_ptr, o_ptr);
    distribute_charges(o_ptr, q_ptr, amt);

    q_ptr->number = amt;
    object_desc(owner_ptr, o_name, q_ptr, 0);
    msg_format(_("%s(%c)を落とした。", "You drop %s (%c)."), o_name, index_to_label(item));
    (void)drop_near(owner_ptr, q_ptr, 0, owner_ptr->y, owner_ptr->x);
    vary_item(owner_ptr, item, -amt);
}

/*!
 * @brief プレイヤーの所持スロットに存在するオブジェクトをまとめなおす /
 * Combine items in the pack
 * @return なし
 * @details
 * Note special handling of the "overflow" slot
 */
void combine_pack(player_type *owner_ptr)
{
    bool flag = FALSE;
    bool is_first_combination = TRUE;
    bool combined = TRUE;
    while (is_first_combination || combined) {
        is_first_combination = FALSE;
        combined = FALSE;

        for (int i = INVEN_PACK; i > 0; i--) {
            object_type *o_ptr;
            o_ptr = &owner_ptr->inventory_list[i];
            if (!o_ptr->k_idx)
                continue;
            for (int j = 0; j < i; j++) {
                object_type *j_ptr;
                j_ptr = &owner_ptr->inventory_list[j];
                if (!j_ptr->k_idx)
                    continue;

                /*
                 * Get maximum number of the stack if these
                 * are similar, get zero otherwise.
                 */
                int max_num = object_similar_part(j_ptr, o_ptr);

                bool is_max = (max_num != 0) && (j_ptr->number < max_num);
                if (!is_max)
                    continue;

                if (o_ptr->number + j_ptr->number <= max_num) {
                    flag = TRUE;
                    object_absorb(j_ptr, o_ptr);
                    owner_ptr->inven_cnt--;
                    int k;
                    for (k = i; k < INVEN_PACK; k++) {
                        owner_ptr->inventory_list[k] = owner_ptr->inventory_list[k + 1];
                    }

                    object_wipe(&owner_ptr->inventory_list[k]);
                } else {
                    int old_num = o_ptr->number;
                    int remain = j_ptr->number + o_ptr->number - max_num;
                    object_absorb(j_ptr, o_ptr);
                    o_ptr->number = remain;
                    if (o_ptr->tval == TV_ROD) {
                        o_ptr->pval = o_ptr->pval * remain / old_num;
                        o_ptr->timeout = o_ptr->timeout * remain / old_num;
                    }

                    if (o_ptr->tval == TV_WAND) {
                        o_ptr->pval = o_ptr->pval * remain / old_num;
                    }
                }

                owner_ptr->window |= (PW_INVEN);
                combined = TRUE;
                break;
            }
        }
    }

    if (flag)
        msg_print(_("ザックの中のアイテムをまとめ直した。", "You combine some items in your pack."));
}

/*!
 * @brief プレイヤーの所持スロットに存在するオブジェクトを並び替える /
 * Reorder items in the pack
 * @param owner_ptr プレーヤーへの参照ポインタ
 * @return なし
 * @details
 * Note special handling of the "overflow" slot
 */
void reorder_pack(player_type *owner_ptr)
{
    int i, j, k;
    s32b o_value;
    object_type forge;
    object_type *q_ptr;
    object_type *o_ptr;
    bool flag = FALSE;

    for (i = 0; i < INVEN_PACK; i++) {
        if ((i == INVEN_PACK) && (owner_ptr->inven_cnt == INVEN_PACK))
            break;

        o_ptr = &owner_ptr->inventory_list[i];
        if (!o_ptr->k_idx)
            continue;

        o_value = object_value(o_ptr);
        for (j = 0; j < INVEN_PACK; j++) {
            if (object_sort_comp(o_ptr, o_value, &owner_ptr->inventory_list[j]))
                break;
        }

        if (j >= i)
            continue;

        flag = TRUE;
        q_ptr = &forge;
        object_copy(q_ptr, &owner_ptr->inventory_list[i]);
        for (k = i; k > j; k--) {
            object_copy(&owner_ptr->inventory_list[k], &owner_ptr->inventory_list[k - 1]);
        }

        object_copy(&owner_ptr->inventory_list[j], q_ptr);
        owner_ptr->window |= (PW_INVEN);
    }

    if (flag)
        msg_print(_("ザックの中のアイテムを並べ直した。", "You reorder some items in your pack."));
}
