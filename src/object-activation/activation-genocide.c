#include "object-activation/activation-genocide.h"
#include "spell-kind/spells-genocide.h"
#include "view/display-messages.h"

bool activate_genocide(player_type *user_ptr)
{
    msg_print(_("�[�F�ɋP���Ă���...", "It glows deep blue..."));
    (void)symbol_genocide(user_ptr, 200, TRUE);
    return TRUE;
}

bool activate_mass_genocide(player_type *user_ptr)
{
    msg_print(_("�Ђǂ��s����������o��...", "It lets out a long, shrill note..."));
    (void)mass_genocide(user_ptr, 200, TRUE);
    return TRUE;
}
