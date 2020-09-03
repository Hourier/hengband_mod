#include "player-info/body-improvement-info.h"
#include "player-info/self-info-util.h"

/* todo ���я��̓s���ŘA�Ԃ�t����B�܂Ƃ߂Ă��ǂ��Ȃ�܂Ƃ߂Ă��܂��\�� */
void set_body_improvement_info_1(player_type *creature_ptr, self_info_type *si_ptr)
{
    if (is_blessed(creature_ptr))
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͍������������Ă���B", "You feel rightous.");

    if (is_hero(creature_ptr))
        si_ptr->info[si_ptr->line++] = _("���Ȃ��̓q�[���[�C�����B", "You feel heroic.");

    if (is_shero(creature_ptr))
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͐퓬�����B", "You are in a battle rage.");

    if (creature_ptr->protevil)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͎׈��Ȃ鑶�݂������Ă���B", "You are protected from evil.");

    if (creature_ptr->shield)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͐_��̃V�[���h�Ŏ���Ă���B", "You are protected by a mystic shield.");

    if (is_invuln(creature_ptr))
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͌��ݏ����Ȃ��B", "You are temporarily invulnerable.");

    if (creature_ptr->wraith_form)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͈ꎞ�I�ɗH�̉����Ă���B", "You are temporarily incorporeal.");
}

/* todo ���я��̓s���ŘA�Ԃ�t����B�܂Ƃ߂Ă��ǂ��Ȃ�܂Ƃ߂Ă��܂��\�� */
void set_body_improvement_info_2(player_type *creature_ptr, self_info_type *si_ptr)
{
    if (creature_ptr->new_spells)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͎�����F����w�Ԃ��Ƃ��ł���B", "You can learn some spells/prayers.");

    if (creature_ptr->word_recall)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͂����ɋA�҂��邾�낤�B", "You will soon be recalled.");

    if (creature_ptr->alter_reality)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͂����ɂ��̐��E�𗣂�邾�낤�B", "You will soon be altered.");

    if (creature_ptr->see_infra)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��̓��͐ԊO���ɕq���ł���B", "Your eyes are sensitive to infrared light.");

    if (creature_ptr->see_inv)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͓����ȃ����X�^�[�����邱�Ƃ��ł���B", "You can see invisible creatures.");

    if (creature_ptr->levitation)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͔�Ԃ��Ƃ��ł���B", "You can fly.");

    if (creature_ptr->free_act)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͖�გm�炸�̌��ʂ������Ă���B", "You have free action.");

    if (creature_ptr->regenerate)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͑f�����̗͂��񕜂���B", "You regenerate quickly.");

    if (creature_ptr->slow_digest)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͐H�~�����Ȃ��B", "Your appetite is small.");
}

/* todo ���я��̓s���ŘA�Ԃ�t����B�܂Ƃ߂Ă��ǂ��Ȃ�܂Ƃ߂Ă��܂��\�� */
void set_body_improvement_info_3(player_type *creature_ptr, self_info_type *si_ptr)
{
    if (creature_ptr->hold_exp)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͎��Ȃ̌o���l����������ƈێ�����B", "You have a firm hold on your experience.");

    if (creature_ptr->reflect)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͖�̎����𔽎˂���B", "You reflect bolt spells.");

    if (creature_ptr->sh_fire)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͉��̃I�[���ɕ�܂�Ă���B", "You are surrounded with a fiery aura.");

    if (creature_ptr->sh_elec)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͓d�C�ɕ�܂�Ă���B", "You are surrounded with electricity.");

    if (creature_ptr->sh_cold)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͗�C�̃I�[���ɕ�܂�Ă���B", "You are surrounded with an aura of coldness.");

    if (creature_ptr->tim_sh_holy)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͐��Ȃ�I�[���ɕ�܂�Ă���B", "You are surrounded with a holy aura.");

    if (creature_ptr->tim_sh_touki)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͓��C�̃I�[���ɕ�܂�Ă���B", "You are surrounded with an energy aura.");

    if (creature_ptr->anti_magic)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͔����@�V�[���h�ɕ�܂�Ă���B", "You are surrounded by an anti-magic shell.");

    if (creature_ptr->anti_tele)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��̓e���|�[�g�ł��Ȃ��B", "You cannot teleport.");

    if (creature_ptr->lite)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��̐g�̂͌����Ă���B", "You are carrying a permanent light.");

    if (creature_ptr->warning)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͍s���̑O�Ɋ댯���@�m���邱�Ƃ��ł���B", "You will be warned before dangerous actions.");

    if (creature_ptr->dec_mana)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͏��Ȃ�����͂Ŗ��@�������邱�Ƃ��ł���B", "You can cast spells with fewer mana points.");

    if (creature_ptr->easy_spell)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͒Ⴂ���s���Ŗ��@�������邱�Ƃ��ł���B", "Fail rate of your magic is decreased.");

    if (creature_ptr->heavy_spell)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͍������s���Ŗ��@�������Ȃ���΂����Ȃ��B", "Fail rate of your magic is increased.");

    if (creature_ptr->mighty_throw)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͋������𓊂���B", "You can throw objects powerfully.");
}

/* todo ���я��̓s���ŘA�Ԃ�t����B�܂Ƃ߂Ă��ǂ��Ȃ�܂Ƃ߂Ă��܂��\�� */
void set_body_improvement_info_4(player_type *creature_ptr, self_info_type *si_ptr)
{
    if (creature_ptr->resist_fear)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͑S�����|�������Ȃ��B", "You are completely fearless.");
    
    if (creature_ptr->resist_blind)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��̖ڂ͖Ӗڂւ̑ϐ��������Ă���B", "Your eyes are resistant to blindness.");
    
    if (creature_ptr->resist_time)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͎��ԋt�]�ւ̑ϐ��������Ă���B", "You are resistant to time.");
}
