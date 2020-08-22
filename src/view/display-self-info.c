#include "view/display-self-info.h"
#include "io/input-key-acceptor.h"
#include "player-info/avatar.h"
#include "player-info/self-info-util.h"
#include "player/player-race.h"
#include "player/player-status-table.h"
#include "term/screen-processor.h"

void display_life_rating(player_type *creature_ptr, self_info_type *si_ptr)
{
    creature_ptr->knowledge |= KNOW_STAT | KNOW_HPRATE;
    strcpy(si_ptr->plev_buf, "");
    int percent
        = (int)(((long)creature_ptr->player_hp[PY_MAX_LEVEL - 1] * 200L) / (2 * creature_ptr->hitdie + ((PY_MAX_LEVEL - 1 + 3) * (creature_ptr->hitdie + 1))));
    sprintf(si_ptr->plev_buf, _("���݂̗̑̓����N : %d/100", "Your current Life Rating is %d/100."), percent);
    strcpy(si_ptr->buf[0], si_ptr->plev_buf);
    si_ptr->info[si_ptr->line++] = si_ptr->buf[0];
    si_ptr->info[si_ptr->line++] = "";
}

void display_max_base_status(player_type *creature_ptr, self_info_type *si_ptr)
{
    si_ptr->info[si_ptr->line++] = _("�\�͂̍ő�l", "Limits of maximum stats");
    for (base_status_type v_nr = 0; v_nr < A_MAX; v_nr++) {
        char stat_desc[80];
        sprintf(stat_desc, "%s 18/%d", stat_names[v_nr], creature_ptr->stat_max_max[v_nr] - 18);
        strcpy(si_ptr->s_string[v_nr], stat_desc);
        si_ptr->info[si_ptr->line++] = si_ptr->s_string[v_nr];
    }
}

void display_virtue(player_type *creature_ptr, self_info_type *si_ptr)
{
    si_ptr->info[si_ptr->line++] = "";
    sprintf(si_ptr->plev_buf, _("���݂̑��� : %s(%ld)", "Your alignment : %s(%ld)"), your_alignment(creature_ptr), (long int)creature_ptr->align);
    strcpy(si_ptr->buf[1], si_ptr->plev_buf);
    si_ptr->info[si_ptr->line++] = si_ptr->buf[1];
    for (int v_nr = 0; v_nr < 8; v_nr++) {
        GAME_TEXT vir_name[20];
        char vir_desc[80];
        int tester = creature_ptr->virtues[v_nr];
        strcpy(vir_name, virtue[(creature_ptr->vir_types[v_nr]) - 1]);
        sprintf(vir_desc, _("�����ƁB%s�̏��Ȃ��B", "Oops. No info about %s."), vir_name);
        if (tester < -100)
            sprintf(vir_desc, _("[%s]�̑΋� (%d)", "You are the polar opposite of %s (%d)."), vir_name, tester);
        else if (tester < -80)
            sprintf(vir_desc, _("[%s]�̑�G (%d)", "You are an arch-enemy of %s (%d)."), vir_name, tester);
        else if (tester < -60)
            sprintf(vir_desc, _("[%s]�̋��G (%d)", "You are a bitter enemy of %s (%d)."), vir_name, tester);
        else if (tester < -40)
            sprintf(vir_desc, _("[%s]�̓G (%d)", "You are an enemy of %s (%d)."), vir_name, tester);
        else if (tester < -20)
            sprintf(vir_desc, _("[%s]�̍ߎ� (%d)", "You have sinned against %s (%d)."), vir_name, tester);
        else if (tester < 0)
            sprintf(vir_desc, _("[%s]�̖����� (%d)", "You have strayed from the path of %s (%d)."), vir_name, tester);
        else if (tester == 0)
            sprintf(vir_desc, _("[%s]�̒����� (%d)", "You are neutral to %s (%d)."), vir_name, tester);
        else if (tester < 20)
            sprintf(vir_desc, _("[%s]�̏����� (%d)", "You are somewhat virtuous in %s (%d)."), vir_name, tester);
        else if (tester < 40)
            sprintf(vir_desc, _("[%s]�̒����� (%d)", "You are virtuous in %s (%d)."), vir_name, tester);
        else if (tester < 60)
            sprintf(vir_desc, _("[%s]�̍����� (%d)", "You are very virtuous in %s (%d)."), vir_name, tester);
        else if (tester < 80)
            sprintf(vir_desc, _("[%s]�̔e�� (%d)", "You are a champion of %s (%d)."), vir_name, tester);
        else if (tester < 100)
            sprintf(vir_desc, _("[%s]�̈̑�Ȕe�� (%d)", "You are a great champion of %s (%d)."), vir_name, tester);
        else
            sprintf(vir_desc, _("[%s]�̋�� (%d)", "You are the living embodiment of %s (%d)."), vir_name, tester);

        strcpy(si_ptr->v_string[v_nr], vir_desc);
        si_ptr->info[si_ptr->line++] = si_ptr->v_string[v_nr];
    }
}

void display_mimic_race_ability(player_type *creature_ptr, self_info_type *si_ptr)
{
    switch (creature_ptr->mimic_form) {
    case MIMIC_DEMON:
    case MIMIC_DEMON_LORD:
        sprintf(si_ptr->plev_buf, _("���Ȃ��� %d �_���[�W�̒n�����Ή��̃u���X��f�����Ƃ��ł���B(%d MP)", "You can nether breathe, dam. %d (cost %d)."),
            3 * creature_ptr->lev, 10 + creature_ptr->lev / 3);

        si_ptr->info[si_ptr->line++] = si_ptr->plev_buf;
        break;
    case MIMIC_VAMPIRE:
        if (creature_ptr->lev <= 1)
            break;

        sprintf(si_ptr->plev_buf, _("���Ȃ��͓G���� %d-%d HP �̐����͂��z���ł���B(%d MP)", "You can steal life from a foe, dam. %d-%d (cost %d)."),
            creature_ptr->lev + MAX(1, creature_ptr->lev / 10), creature_ptr->lev + creature_ptr->lev * MAX(1, creature_ptr->lev / 10),
            1 + (creature_ptr->lev / 3));
        si_ptr->info[si_ptr->line++] = si_ptr->plev_buf;
        break;
    }
}

void display_self_info(self_info_type *si_ptr)
{
    screen_save();
    for (int k = 1; k < 24; k++)
        prt("", k, 13);

    prt(_("        ���Ȃ��̏��:", "     Your Attributes:"), 1, 15);
    int k = 2;
    for (int j = 0; j < si_ptr->line; j++) {
        prt(si_ptr->info[j], k++, 15);

        /* Every 20 entries (lines 2 to 21), start over */
        if ((k != 22) || (j + 1 >= si_ptr->line))
            continue;

        prt(_("-- ���� --", "-- more --"), k, 15);
        inkey();
        for (; k > 2; k--)
            prt("", k, 15);
    }

    prt(_("[�����L�[�������ƃQ�[���ɖ߂�܂�]", "[Press any key to continue]"), k, 13);
    inkey();
    screen_load();
}
