#include "object-activation/activation-bolt-ball.h"
#include "core/hp-mp-processor.h"
#include "effect/effect-characteristics.h"
#include "floor/cave.h"
#include "floor/floor.h"
#include "grid/feature-flag-types.h"
#include "spell-kind/spells-launcher.h"
#include "spell-kind/spells-specific-bolt.h"
#include "spell/process-effect.h"
#include "spell/spell-types.h"
#include "target/target-getter.h"
#include "view/display-messages.h"

bool activate_missile_1(player_type *user_ptr)
{
    DIRECTION dir;
    msg_print(_("�����ῂ������炢�ɖ��邭�P���Ă���...", "It glows extremely brightly..."));
    if (!get_aim_dir(user_ptr, &dir))
        return FALSE;

    (void)fire_bolt(user_ptr, GF_MISSILE, dir, damroll(2, 6));
    return TRUE;
}

bool activate_missile_2(player_type *user_ptr)
{
    DIRECTION dir;
    msg_print(_("���@�̃g�Q�����ꂽ...", "It grows magical spikes..."));
    if (!get_aim_dir(user_ptr, &dir))
        return FALSE;

    (void)fire_bolt(user_ptr, GF_ARROW, dir, 150);
    return TRUE;
}

bool activate_missile_3(player_type *user_ptr)
{
    DIRECTION dir;
    if (!get_aim_dir(user_ptr, &dir))
        return FALSE;

    msg_print(_("���Ȃ��̓G�������g�̃u���X��f�����B", "You breathe the elements."));
    fire_breath(user_ptr, GF_MISSILE, dir, 300, 4);
    return TRUE;
}

bool activate_bolt_acid_1(player_type *user_ptr)
{
    DIRECTION dir;
    msg_print(_("����͎_�ɕ���ꂽ...", "It is covered in acid..."));
    if (!get_aim_dir(user_ptr, &dir))
        return FALSE;

    (void)fire_bolt(user_ptr, GF_ACID, dir, damroll(5, 8));
    return TRUE;
}

bool activate_bolt_elec_1(player_type *user_ptr)
{
    DIRECTION dir;
    msg_print(_("����͉ΉԂɕ���ꂽ...", "It is covered in sparks..."));
    if (!get_aim_dir(user_ptr, &dir))
        return FALSE;

    (void)fire_bolt(user_ptr, GF_ELEC, dir, damroll(4, 8));
    return TRUE;
}

bool activate_bolt_fire_1(player_type *user_ptr)
{
    DIRECTION dir;
    msg_print(_("����͉��ɕ���ꂽ...", "It is covered in fire..."));
    if (!get_aim_dir(user_ptr, &dir))
        return FALSE;

    (void)fire_bolt(user_ptr, GF_FIRE, dir, damroll(9, 8));
    return TRUE;
}

bool activate_bolt_cold_1(player_type *user_ptr)
{
    DIRECTION dir;
    msg_print(_("����͑��ɕ���ꂽ...", "It is covered in frost..."));
    if (!get_aim_dir(user_ptr, &dir))
        return FALSE;

    (void)fire_bolt(user_ptr, GF_COLD, dir, damroll(6, 8));
    return TRUE;
}

bool activate_bolt_hypodynamia_1(player_type *user_ptr, concptr name)
{
    DIRECTION dir;
    msg_format(_("���Ȃ���%s�ɓG����ߎE���悤�������B", "You order the %s to strangle your opponent."), name);
    if (!get_aim_dir(user_ptr, &dir))
        return FALSE;

    hypodynamic_bolt(user_ptr, dir, 100);
    return TRUE;
}

bool activate_bolt_hypodynamia_2(player_type *user_ptr)
{
    DIRECTION dir;
    msg_print(_("�����P���Ă���...", "It glows black..."));
    if (!get_aim_dir(user_ptr, &dir))
        return FALSE;

    hypodynamic_bolt(user_ptr, dir, 120);
    return TRUE;
}

bool activate_bolt_drain_1(player_type *user_ptr)
{
    DIRECTION dir;
    if (!get_aim_dir(user_ptr, &dir))
        return FALSE;

    for (int dummy = 0; dummy < 3; dummy++)
        if (hypodynamic_bolt(user_ptr, dir, 50))
            hp_player(user_ptr, 50);

    return TRUE;
}

bool activate_bolt_drain_2(player_type *user_ptr)
{
    DIRECTION dir;
    if (!get_aim_dir(user_ptr, &dir))
        return FALSE;

    for (int dummy = 0; dummy < 3; dummy++)
        if (hypodynamic_bolt(user_ptr, dir, 100))
            hp_player(user_ptr, 100);

    return TRUE;
}

bool activate_bolt_mana(player_type *user_ptr, concptr name)
{
    DIRECTION dir;
    msg_format(_("%s�ɖ��@�̃g�Q�����ꂽ...", "The %s grows magical spikes..."), name);
    if (!get_aim_dir(user_ptr, &dir))
        return FALSE;

    (void)fire_bolt(user_ptr, GF_ARROW, dir, 150);
    return TRUE;
}

bool activate_ball_pois_1(player_type *user_ptr)
{
    DIRECTION dir;
    msg_print(_("����͔Z�ΐF�ɖ������Ă���...", "It throbs deep green..."));
    if (!get_aim_dir(user_ptr, &dir))
        return FALSE;

    (void)fire_ball(user_ptr, GF_POIS, dir, 12, 3);
    return TRUE;
}

bool activate_ball_cold_1(player_type *user_ptr)
{
    DIRECTION dir;
    msg_print(_("����͑��ɕ���ꂽ...", "It is covered in frost..."));
    if (!get_aim_dir(user_ptr, &dir))
        return FALSE;

    (void)fire_ball(user_ptr, GF_COLD, dir, 48, 2);
    return TRUE;
}

bool activate_ball_cold_2(player_type *user_ptr)
{
    DIRECTION dir;
    msg_print(_("����͐��������P����...", "It glows an intense blue..."));
    if (!get_aim_dir(user_ptr, &dir))
        return FALSE;

    (void)fire_ball(user_ptr, GF_COLD, dir, 100, 2);
    return TRUE;
}

bool activate_ball_cold_3(player_type *user_ptr)
{
    DIRECTION dir;
    msg_print(_("���邭���F�ɋP���Ă���...", "It glows bright white..."));
    if (!get_aim_dir(user_ptr, &dir))
        return FALSE;

    (void)fire_ball(user_ptr, GF_COLD, dir, 400, 3);
    return TRUE;
}

bool activate_ball_fire_1(player_type *user_ptr)
{
    DIRECTION dir;
    msg_print(_("����͐Ԃ��������P����...", "It glows an intense red..."));
    if (!get_aim_dir(user_ptr, &dir))
        return FALSE;

    (void)fire_ball(user_ptr, GF_FIRE, dir, 72, 2);
    return TRUE;
}

bool activate_ball_fire_2(player_type *user_ptr, concptr name)
{
    DIRECTION dir;
    msg_format(_("%s���牊�������o����...", "The %s rages in fire..."), name);
    if (!get_aim_dir(user_ptr, &dir))
        return FALSE;

    (void)fire_ball(user_ptr, GF_FIRE, dir, 120, 3);
    return TRUE;
}

bool activate_ball_fire_3(player_type *user_ptr)
{
    DIRECTION dir;
    msg_print(_("�[�ԐF�ɋP���Ă���...", "It glows deep red..."));
    if (!get_aim_dir(user_ptr, &dir))
        return FALSE;

    (void)fire_ball(user_ptr, GF_FIRE, dir, 300, 3);
    return TRUE;
}

bool activate_ball_fire_4(player_type *user_ptr)
{
    DIRECTION dir;
    msg_print(_("����͐Ԃ��������P����...", "It glows an intense red..."));
    if (!get_aim_dir(user_ptr, &dir))
        return FALSE;

    (void)fire_ball(user_ptr, GF_FIRE, dir, 100, 2);
    return TRUE;
}

bool activate_ball_elec_2(player_type *user_ptr)
{
    DIRECTION dir;
    msg_print(_("�d�C���p�`�p�`���𗧂Ă�...", "It crackles with electricity..."));
    if (!get_aim_dir(user_ptr, &dir))
        return FALSE;

    (void)fire_ball(user_ptr, GF_ELEC, dir, 100, 3);
    return TRUE;
}

bool activate_ball_elec_3(player_type *user_ptr)
{
    DIRECTION dir;
    msg_print(_("�[�F�ɋP���Ă���...", "It glows deep blue..."));
    if (!get_aim_dir(user_ptr, &dir))
        return FALSE;

    (void)fire_ball(user_ptr, GF_ELEC, dir, 500, 3);
    return TRUE;
}

bool activate_ball_acid_1(player_type *user_ptr)
{
    DIRECTION dir;
    msg_print(_("����͍����������P����...", "It glows an intense black..."));
    if (!get_aim_dir(user_ptr, &dir))
        return FALSE;

    (void)fire_ball(user_ptr, GF_ACID, dir, 100, 2);
    return TRUE;
}

bool activate_ball_nuke_1(player_type *user_ptr)
{
    DIRECTION dir;
    msg_print(_("����͗΂Ɍ������P����...", "It glows an intense green..."));
    if (!get_aim_dir(user_ptr, &dir))
        return FALSE;

    (void)fire_ball(user_ptr, GF_NUKE, dir, 100, 2);
    return TRUE;
}

bool activate_rocket(player_type *user_ptr)
{
    DIRECTION dir;
    if (!get_aim_dir(user_ptr, &dir))
        return FALSE;

    msg_print(_("���P�b�g�𔭎˂����I", "You launch a rocket!"));
    (void)fire_ball(user_ptr, GF_ROCKET, dir, 250 + user_ptr->lev * 3, 2);
    return TRUE;
}

bool activate_ball_water(player_type *user_ptr, concptr name)
{
    DIRECTION dir;
    msg_format(_("%s���[���F�Ɍۓ����Ă���...", "The %s throbs deep blue..."), name);
    if (!get_aim_dir(user_ptr, &dir))
        return FALSE;

    (void)fire_ball(user_ptr, GF_WATER, dir, 200, 3);
    return TRUE;
}

bool activate_ball_lite(player_type *user_ptr, concptr name)
{
    HIT_POINT num = damroll(5, 3);
    POSITION y = 0, x = 0;
    msg_format(_("%s����Ȃŕ���ꂽ...", "The %s is surrounded by lightning..."), name);
    for (int k = 0; k < num; k++) {
        int attempts = 1000;
        while (attempts--) {
            scatter(user_ptr, &y, &x, user_ptr->y, user_ptr->x, 4, 0);
            if (!cave_have_flag_bold(user_ptr->current_floor_ptr, y, x, FF_PROJECT))
                continue;

            if (!player_bold(user_ptr, y, x))
                break;
        }

        project(user_ptr, 0, 3, y, x, 150, GF_ELEC, PROJECT_THRU | PROJECT_STOP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL, -1);
    }

    return TRUE;
}

bool activate_ball_dark(player_type *user_ptr, concptr name)
{
    DIRECTION dir;
    msg_format(_("%s���[���łɕ���ꂽ...", "The %s is coverd in pitch-darkness..."), name);
    if (!get_aim_dir(user_ptr, &dir))
        return FALSE;

    (void)fire_ball(user_ptr, GF_DARK, dir, 250, 4);
    return TRUE;
}

bool activate_ball_mana(player_type *user_ptr, concptr name)
{
    DIRECTION dir;
    msg_format(_("%s�������������D�D�D", "The %s glows pale..."), name);
    if (!get_aim_dir(user_ptr, &dir))
        return FALSE;

    (void)fire_ball(user_ptr, GF_MANA, dir, 250, 4);
    return TRUE;
}
