#include "mind/mind-magic-eater.h"
#include "flavor/flavor-describer.h"
#include "floor/floor-object.h"
#include "inventory/inventory-object.h"
#include "object-hook/hook-magic.h"
#include "object/item-tester-hooker.h"
#include "object/item-use-flags.h"
#include "perception/object-perception.h"
#include "sv-definition/sv-staff-types.h"
#include "system/object-type-definition.h"
#include "view/display-messages.h"

/*!
 * @brief ������p�t�̖��͎�荞�ݏ���
 * @param user_ptr �A�C�e������荞�ރN���[�`���[
 * @return ��荞�݂����s������TRUE�A�L�����Z��������FALSE��Ԃ�
 */
bool import_magic_device(player_type *user_ptr)
{
    item_tester_hook = item_tester_hook_recharge;
    concptr q = _("�ǂ̃A�C�e���̖��͂���荞�݂܂���? ", "Gain power of which item? ");
    concptr s = _("���͂���荞�߂�A�C�e�����Ȃ��B", "You have nothing to gain power.");
    OBJECT_IDX item;
    object_type *o_ptr = choose_object(user_ptr, &item, q, s, USE_INVEN | USE_FLOOR, 0);
    if (!o_ptr)
        return FALSE;

    if (o_ptr->tval == TV_STAFF && o_ptr->sval == SV_STAFF_NOTHING) {
        msg_print(_("���̏�ɂ͔����ׂ̈̔\�͉͂���������Ă��Ȃ��悤���B", "This staff doesn't have any magical ability."));
        return FALSE;
    }

    if (!object_is_known(o_ptr)) {
        msg_print(_("�Ӓ肳��Ă��Ȃ��Ǝ�荞�߂Ȃ��B", "You need to identify before absorbing."));
        return FALSE;
    }

    if (o_ptr->timeout) {
        msg_print(_("�[�U���̃A�C�e���͎�荞�߂Ȃ��B", "This item is still charging."));
        return FALSE;
    }

    PARAMETER_VALUE pval = o_ptr->pval;
    int ext = 0;
    if (o_ptr->tval == TV_ROD)
        ext = 72;
    else if (o_ptr->tval == TV_WAND)
        ext = 36;

    if (o_ptr->tval == TV_ROD) {
        user_ptr->magic_num2[o_ptr->sval + ext] += (MAGIC_NUM2)o_ptr->number;
        if (user_ptr->magic_num2[o_ptr->sval + ext] > 99)
            user_ptr->magic_num2[o_ptr->sval + ext] = 99;
    } else {
        int num;
        for (num = o_ptr->number; num; num--) {
            int gain_num = pval;
            if (o_ptr->tval == TV_WAND)
                gain_num = (pval + num - 1) / num;
            if (user_ptr->magic_num2[o_ptr->sval + ext]) {
                gain_num *= 256;
                gain_num = (gain_num / 3 + randint0(gain_num / 3)) / 256;
                if (gain_num < 1)
                    gain_num = 1;
            }
            user_ptr->magic_num2[o_ptr->sval + ext] += (MAGIC_NUM2)gain_num;
            if (user_ptr->magic_num2[o_ptr->sval + ext] > 99)
                user_ptr->magic_num2[o_ptr->sval + ext] = 99;
            user_ptr->magic_num1[o_ptr->sval + ext] += pval * 0x10000;
            if (user_ptr->magic_num1[o_ptr->sval + ext] > 99 * 0x10000)
                user_ptr->magic_num1[o_ptr->sval + ext] = 99 * 0x10000;
            if (user_ptr->magic_num1[o_ptr->sval + ext] > user_ptr->magic_num2[o_ptr->sval + ext] * 0x10000)
                user_ptr->magic_num1[o_ptr->sval + ext] = user_ptr->magic_num2[o_ptr->sval + ext] * 0x10000;
            if (o_ptr->tval == TV_WAND)
                pval -= (pval + num - 1) / num;
        }
    }

    GAME_TEXT o_name[MAX_NLEN];
    describe_flavor(user_ptr, o_name, o_ptr, 0);
    msg_format(_("%s�̖��͂���荞�񂾁B", "You absorb magic of %s."), o_name);

    vary_item(user_ptr, item, -999);
    take_turn(user_ptr, 100);
    return TRUE;
}
