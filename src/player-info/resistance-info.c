#include "player-info/resistance-info.h"
#include "player-info/self-info-util.h"
#include "player/player-race.h"
#include "status/element-resistance.h"

void set_element_resistance_info(player_type* creature_ptr, self_info_type* si_ptr)
{
    if (creature_ptr->immune_acid) {
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͎_�ɑ΂��銮�S�Ȃ�Ɖu�������Ă���B", "You are completely immune to acid.");
    } else if (creature_ptr->resist_acid && is_oppose_acid(creature_ptr)) {
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͎_�ւ̋��͂ȑϐ��������Ă���B", "You resist acid exceptionally well.");
    } else if (creature_ptr->resist_acid || is_oppose_acid(creature_ptr)) {
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͎_�ւ̑ϐ��������Ă���B", "You are resistant to acid.");
    }

    if (creature_ptr->immune_elec) {
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͓d���ɑ΂��銮�S�Ȃ�Ɖu�������Ă���B", "You are completely immune to lightning.");
    } else if (creature_ptr->resist_elec && is_oppose_elec(creature_ptr)) {
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͓d���ւ̋��͂ȑϐ��������Ă���B", "You resist lightning exceptionally well.");
    } else if (creature_ptr->resist_elec || is_oppose_elec(creature_ptr)) {
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͓d���ւ̑ϐ��������Ă���B", "You are resistant to lightning.");
    }

    if (is_specific_player_race(creature_ptr, RACE_ANDROID) && !creature_ptr->immune_elec) {
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͓d���Ɏア�B", "You are susceptible to damage from lightning.");
    }

    if (creature_ptr->immune_fire) {
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͉΂ɑ΂��銮�S�Ȃ�Ɖu�������Ă���B", "You are completely immune to fire.");
    } else if (creature_ptr->resist_fire && is_oppose_fire(creature_ptr)) {
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͉΂ւ̋��͂ȑϐ��������Ă���B", "You resist fire exceptionally well.");
    } else if (creature_ptr->resist_fire || is_oppose_fire(creature_ptr)) {
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͉΂ւ̑ϐ��������Ă���B", "You are resistant to fire.");
    }

    if (is_specific_player_race(creature_ptr, RACE_ENT) && !creature_ptr->immune_fire) {
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͉΂Ɏア�B", "You are susceptible to damage from fire.");
    }

    if (creature_ptr->immune_cold) {
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͗�C�ɑ΂��銮�S�Ȃ�Ɖu�������Ă���B", "You are completely immune to cold.");
    } else if (creature_ptr->resist_cold && is_oppose_cold(creature_ptr)) {
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͗�C�ւ̋��͂ȑϐ��������Ă���B", "You resist cold exceptionally well.");
    } else if (creature_ptr->resist_cold || is_oppose_cold(creature_ptr)) {
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͗�C�ւ̑ϐ��������Ă���B", "You are resistant to cold.");
    }

    if (creature_ptr->resist_pois && is_oppose_pois(creature_ptr)) {
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͓łւ̋��͂ȑϐ��������Ă���B", "You resist poison exceptionally well.");
    } else if (creature_ptr->resist_pois || is_oppose_pois(creature_ptr)) {
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͓łւ̑ϐ��������Ă���B", "You are resistant to poison.");
    }
}

void set_high_resistance_info(player_type *creature_ptr, self_info_type *si_ptr)
{
    if (creature_ptr->resist_lite)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͑M���ւ̑ϐ��������Ă���B", "You are resistant to bright light.");

    if (is_specific_player_race(creature_ptr, RACE_VAMPIRE) || is_specific_player_race(creature_ptr, RACE_S_FAIRY)
        || (creature_ptr->mimic_form == MIMIC_VAMPIRE))
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͑M���Ɏア�B", "You are susceptible to damage from bright light.");

    if (is_specific_player_race(creature_ptr, RACE_VAMPIRE) || (creature_ptr->mimic_form == MIMIC_VAMPIRE) || creature_ptr->wraith_form)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͈Í��ɑ΂��銮�S�Ȃ�Ɖu�������Ă���B", "You are completely immune to darkness.");
    else if (creature_ptr->resist_dark)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͈Í��ւ̑ϐ��������Ă���B", "You are resistant to darkness.");
    
    if (creature_ptr->resist_conf)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͍����ւ̑ϐ��������Ă���B", "You are resistant to confusion.");
    
    if (creature_ptr->resist_sound)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͉��g�̏Ռ��ւ̑ϐ��������Ă���B", "You are resistant to sonic attacks.");
    
    if (creature_ptr->resist_disen)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͗򉻂ւ̑ϐ��������Ă���B", "You are resistant to disenchantment.");
    
    if (creature_ptr->resist_chaos)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��̓J�I�X�̗͂ւ̑ϐ��������Ă���B", "You are resistant to chaos.");
    
    if (creature_ptr->resist_shard)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͔j�Ђ̍U���ւ̑ϐ��������Ă���B", "You are resistant to blasts of shards.");
    
    if (creature_ptr->resist_nexus)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͈��ʍ����̍U���ւ̑ϐ��������Ă���B", "You are resistant to nexus attacks.");

    if (is_specific_player_race(creature_ptr, RACE_SPECTRE))
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͒n���̗͂��z���ł���B", "You can drain nether forces.");
    else if (creature_ptr->resist_neth)
        si_ptr->info[si_ptr->line++] = _("���Ȃ��͒n���̗͂ւ̑ϐ��������Ă���B", "You are resistant to nether forces.");
}
