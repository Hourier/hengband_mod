#include "cmd-io/cmd-floor.h"
#include "core/asking-player.h"
#include "core/player-redraw-types.h"
#include "core/player-update-types.h"
#include "core/stuff-handler.h"
#include "core/window-redrawer.h"
#include "floor/geometry.h"
#include "game-option/keymap-directory-getter.h"
#include "io/cursor.h"
#include "io/screen-util.h"
#include "main/sound-of-music.h"
#include "target/target-checker.h"
#include "target/target-setter.h"
#include "target/target-types.h"
#include "view/display-messages.h"
#include "window/main-window-util.h"

/*!
 * @brief �^�[�Q�b�g��ݒ肷��R�}���h�̃��C�����[�`��
 * Target command
 * @return �Ȃ�
 */
void do_cmd_target(player_type *creature_ptr)
{
    if (creature_ptr->wild_mode)
        return;

    if (target_set(creature_ptr, TARGET_KILL))
        msg_print(_("�^�[�Q�b�g����B", "Target Selected."));
    else
        msg_print(_("�^�[�Q�b�g�����B", "Target Aborted."));
}

/*!
 * @brief ���͂����n���R�}���h�̃��C�����[�`��
 * Look command
 * @return �Ȃ�
 */
void do_cmd_look(player_type *creature_ptr)
{
    creature_ptr->window |= PW_MONSTER_LIST;
    handle_stuff(creature_ptr);
    if (target_set(creature_ptr, TARGET_LOOK))
        msg_print(_("�^�[�Q�b�g����B", "Target Selected."));
}

/*!
 * @brief �ʒu���m�F����R�}���h�̃��C�����[�`��
 * Allow the player to examine other sectors on the map
 * @return �Ȃ�
 */
void do_cmd_locate(player_type *creature_ptr)
{
    DIRECTION dir;
    POSITION y1, x1;
    GAME_TEXT tmp_val[80];
    GAME_TEXT out_val[160];
    TERM_LEN wid, hgt;
    get_screen_size(&wid, &hgt);
    POSITION y2 = y1 = panel_row_min;
    POSITION x2 = x1 = panel_col_min;
    while (TRUE) {
        if ((y2 == y1) && (x2 == x1))
            strcpy(tmp_val, _("�^��", "\0"));
        else
            sprintf(tmp_val, "%s%s", ((y2 < y1) ? _("�k", " North") : (y2 > y1) ? _("��", " South") : ""),
                ((x2 < x1) ? _("��", " West") : (x2 > x1) ? _("��", " East") : ""));

        sprintf(out_val, _("�}�b�v�ʒu [%d(%02d),%d(%02d)] (�v���C���[��%s)  ����?", "Map sector [%d(%02d),%d(%02d)], which is%s your sector.  Direction?"),
            y2 / (hgt / 2), y2 % (hgt / 2), x2 / (wid / 2), x2 % (wid / 2), tmp_val);

        dir = 0;
        while (!dir) {
            char command;
            if (!get_com(out_val, &command, TRUE))
                break;

            dir = get_keymap_dir(command);
            if (!dir)
                bell();
        }

        if (!dir)
            break;

        if (change_panel(creature_ptr, ddy[dir], ddx[dir])) {
            y2 = panel_row_min;
            x2 = panel_col_min;
        }
    }

    verify_panel(creature_ptr);
    creature_ptr->update |= PU_MONSTERS;
    creature_ptr->redraw |= PR_MAP;
    creature_ptr->window |= PW_OVERHEAD | PW_DUNGEON;
    handle_stuff(creature_ptr);
}
