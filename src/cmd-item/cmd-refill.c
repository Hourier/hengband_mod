#include "cmd-item/cmd-refill.h"
#include "core/player-redraw-types.h"
#include "core/player-update-types.h"
#include "floor/floor-object.h"
#include "inventory/inventory-object.h"
#include "inventory/inventory-slot-types.h"
#include "object-enchant/object-ego.h"
#include "object-hook/hook-expendable.h"
#include "object/item-tester-hooker.h"
#include "object/item-use-flags.h"
#include "player/attack-defense-types.h"
#include "player/special-defense-types.h"
#include "status/action-setter.h"
#include "sv-definition/sv-lite-types.h"
#include "view/display-messages.h"
#include "object-enchant/tr-types.h"
#include "object/object-flags.h"
#include "util/bit-flags-calculator.h"

/*!
 * @brief �����^���ɔR����������R�}���h�̃��C�����[�`��
 * Refill the players lamp (from the pack or floor)
 * @return �Ȃ�
 */
static void do_cmd_refill_lamp(player_type *user_ptr)
{
    OBJECT_IDX item;
    object_type *o_ptr;
    object_type *j_ptr;
    item_tester_hook = item_tester_refill_lantern;
    concptr q = _("�ǂ̖��ڂ��璍���܂���? ", "Refill with which flask? ");
    concptr s = _("���ڂ��Ȃ��B", "You have no flasks of oil.");
    o_ptr = choose_object(user_ptr, &item, q, s, USE_INVEN | USE_FLOOR, 0);
    if (!o_ptr)
        return;

    BIT_FLAGS flgs[TR_FLAG_SIZE], flgs2[TR_FLAG_SIZE];
    object_flags(user_ptr, o_ptr, flgs);

    take_turn(user_ptr, 50);
    j_ptr = &user_ptr->inventory_list[INVEN_LITE];
    object_flags(user_ptr, j_ptr, flgs2);
    j_ptr->xtra4 += o_ptr->xtra4;
    msg_print(_("�����v�ɖ��𒍂����B", "You fuel your lamp."));
    if (has_flag(flgs, TR_DARK_SOURCE) && (j_ptr->xtra4 > 0)) {
        j_ptr->xtra4 = 0;
        msg_print(_("�����v�������Ă��܂����I", "Your lamp has gone out!"));
    } else if (has_flag(flgs, TR_DARK_SOURCE) || has_flag(flgs2, TR_DARK_SOURCE)) {
        j_ptr->xtra4 = 0;
        msg_print(_("�����������v�͑S������Ȃ��B", "Curiously, your lamp doesn't light."));
    } else if (j_ptr->xtra4 >= FUEL_LAMP) {
        j_ptr->xtra4 = FUEL_LAMP;
        msg_print(_("�����v�̖��͈�t���B", "Your lamp is full."));
    }

    vary_item(user_ptr, item, -1);
    user_ptr->update |= PU_TORCH;
}

/*!
 * @brief �����𑩂˂�R�}���h�̃��C�����[�`��
 * Refuel the players torch (from the pack or floor)
 * @return �Ȃ�
 */
static void do_cmd_refill_torch(player_type *user_ptr)
{
    OBJECT_IDX item;
    object_type *o_ptr;
    object_type *j_ptr;
    item_tester_hook = object_can_refill_torch;
    concptr q = _("�ǂ̏����Ŗ���������߂܂���? ", "Refuel with which torch? ");
    concptr s = _("���ɏ������Ȃ��B", "You have no extra torches.");
    o_ptr = choose_object(user_ptr, &item, q, s, USE_INVEN | USE_FLOOR, 0);
    if (!o_ptr)
        return;

    BIT_FLAGS flgs[TR_FLAG_SIZE], flgs2[TR_FLAG_SIZE];
    object_flags(user_ptr, o_ptr, flgs);

    take_turn(user_ptr, 50);
    j_ptr = &user_ptr->inventory_list[INVEN_LITE];
    object_flags(user_ptr, j_ptr, flgs2);
    j_ptr->xtra4 += o_ptr->xtra4 + 5;
    msg_print(_("���������������B", "You combine the torches."));
    if (has_flag(flgs, TR_DARK_SOURCE) && (j_ptr->xtra4 > 0)) {
        j_ptr->xtra4 = 0;
        msg_print(_("�����������Ă��܂����I", "Your torch has gone out!"));
    } else if (has_flag(flgs, TR_DARK_SOURCE) || has_flag(flgs2, TR_DARK_SOURCE)) {
        j_ptr->xtra4 = 0;
        msg_print(_("�����������͑S������Ȃ��B", "Curiously, your torch doesn't light."));
    } else if (j_ptr->xtra4 >= FUEL_TORCH) {
        j_ptr->xtra4 = FUEL_TORCH;
        msg_print(_("�����̎����͏\�����B", "Your torch is fully fueled."));
    } else
        msg_print(_("�����͂����������邭�P�����B", "Your torch glows more brightly."));

    vary_item(user_ptr, item, -1);
    user_ptr->update |= PU_TORCH;
}

/*!
 * @brief �R�����[����R�}���h�̃��C�����[�`��
 * Refill the players lamp, or restock his torches
 * @return �Ȃ�
 */
void do_cmd_refill(player_type *user_ptr)
{
    object_type *o_ptr;
    o_ptr = &user_ptr->inventory_list[INVEN_LITE];
    if (user_ptr->special_defense & KATA_MUSOU)
        set_action(user_ptr, ACTION_NONE);

    if (o_ptr->tval != TV_LITE)
        msg_print(_("�����𑕔����Ă��Ȃ��B", "You are not wielding a light."));
    else if (o_ptr->sval == SV_LITE_LANTERN)
        do_cmd_refill_lamp(user_ptr);
    else if (o_ptr->sval == SV_LITE_TORCH)
        do_cmd_refill_torch(user_ptr);
    else
        msg_print(_("���̌����͎��������΂��Ȃ��B", "Your light cannot be refilled."));
}
