#include "spell-realm/spells-arcane.h"
#include "core/player-update-types.h"
#include "inventory/inventory-slot-types.h"
#include "sv-definition/sv-lite-types.h"
#include "system/object-type-definition.h"
#include "view/display-messages.h"

/*!
 * @brief �����������̔R�f�ǉ����� /
 * Charge a lite (torch or latern)
 * @return �Ȃ�
 */
void phlogiston(player_type *caster_ptr)
{
    GAME_TURN max_flog = 0;
    object_type *o_ptr = &caster_ptr->inventory_list[INVEN_LITE];
    if ((o_ptr->tval == TV_LITE) && (o_ptr->sval == SV_LITE_LANTERN))
        max_flog = FUEL_LAMP;
    else if ((o_ptr->tval == TV_LITE) && (o_ptr->sval == SV_LITE_TORCH))
        max_flog = FUEL_TORCH;
    else {
        msg_print(_("�R�f�������A�C�e���𑕔����Ă��܂���B", "You are not wielding anything which uses phlogiston."));
        return;
    }

    if (o_ptr->xtra4 >= max_flog) {
        msg_print(_("���̃A�C�e���ɂ͂���ȏ�R�f���[�ł��܂���B", "No more phlogiston can be put in this item."));
        return;
    }

    o_ptr->xtra4 += (XTRA16)(max_flog / 2);
    msg_print(_("�Ɩ��p�A�C�e���ɔR�f���[�����B", "You add phlogiston to your light item."));
    if (o_ptr->xtra4 >= max_flog) {
        o_ptr->xtra4 = (XTRA16)max_flog;
        msg_print(_("�Ɩ��p�A�C�e���͖��^���ɂȂ����B", "Your light item is full."));
    }

    caster_ptr->update |= PU_TORCH;
}
