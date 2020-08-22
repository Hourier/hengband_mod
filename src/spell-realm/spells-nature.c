#include "spell-realm/spells-nature.h"
#include "flavor/flavor-describer.h"
#include "flavor/object-flavor-types.h"
#include "floor/floor-object.h"
#include "object-enchant/tr-types.h"
#include "object-hook/hook-armor.h"
#include "object-hook/hook-checker.h"
#include "object/item-tester-hooker.h"
#include "object/item-use-flags.h"
#include "racial/racial-android.h"
#include "util/bit-flags-calculator.h"
#include "view/display-messages.h"

/*!
 * @brief �h��̎K�~�ߖh�~����
 * @param caster_ptr �K�~�ߎ��s�҂̎Q�ƃ|�C���^
 * @return �^�[�������v���鏈�����s�����Ȃ��TRUE��Ԃ�
 */
bool rustproof(player_type *caster_ptr)
{
    item_tester_hook = object_is_armour;
    concptr q = _("�ǂ̖h��ɎK�~�߂����܂����H", "Rustproof which piece of armour? ");
    concptr s = _("�K�~�߂ł�����̂�����܂���B", "You have nothing to rustproof.");
    OBJECT_IDX item;
    object_type *o_ptr = choose_object(caster_ptr, &item, q, s, USE_EQUIP | USE_INVEN | USE_FLOOR | IGNORE_BOTHHAND_SLOT, 0);
    if (o_ptr == NULL)
        return FALSE;

    GAME_TEXT o_name[MAX_NLEN];
    describe_flavor(caster_ptr, o_name, o_ptr, OD_OMIT_PREFIX | OD_NAME_ONLY);
    add_flag(o_ptr->art_flags, TR_IGNORE_ACID);
    if ((o_ptr->to_a < 0) && !object_is_cursed(o_ptr)) {
#ifdef JP
        msg_format("%s�͐V�i���l�ɂȂ����I", o_name);
#else
        msg_format("%s %s look%s as good as new!", ((item >= 0) ? "Your" : "The"), o_name, ((o_ptr->number > 1) ? "" : "s"));
#endif
        o_ptr->to_a = 0;
    }

#ifdef JP
    msg_format("%s�͕��H���Ȃ��Ȃ����B", o_name);
#else
    msg_format("%s %s %s now protected against corrosion.", ((item >= 0) ? "Your" : "The"), o_name, ((o_ptr->number > 1) ? "are" : "is"));
#endif
    calc_android_exp(caster_ptr);
    return TRUE;
}
