#include "specific-object/toragoroshi.h"
#include "player-attack/player-attack.h"
#include "view/display-messages.h"

bool activate_toragoroshi(player_type *user_ptr)
{
    msg_print(_("���Ȃ��͗d���ɖ�����ꂽ�c", "You are enchanted by cursed blade..."));
    msg_print(_("�u���ق��� ���̂��Ƃ� ���͂̂ڂ�� ��߂����� ���� ����������v", "'Behold the blade arts.'"));
    massacre(user_ptr);
    return TRUE;
}
