﻿#include "player/player-race.h"
#include "player/player-race-types.h"
#include "system/object-type-definition.h"
#include "mimic-info-table.h"
#include "player/race-info-table.h"

const player_race *rp_ptr;

SYMBOL_CODE get_summon_symbol_from_player(player_type *creature_ptr)
{
    SYMBOL_CODE symbol = 'N';
    switch (creature_ptr->mimic_form) {
    case MIMIC_NONE:
        switch (creature_ptr->prace) {
        case RACE_HUMAN:
        case RACE_AMBERITE:
        case RACE_BARBARIAN:
        case RACE_BEASTMAN:
        case RACE_DUNADAN:
            symbol = 'p';
            break;
        case RACE_HALF_ELF:
        case RACE_ELF:
        case RACE_HOBBIT:
        case RACE_GNOME:
        case RACE_DWARF:
        case RACE_HIGH_ELF:
        case RACE_NIBELUNG:
        case RACE_DARK_ELF:
        case RACE_MIND_FLAYER:
        case RACE_KUTAR:
        case RACE_S_FAIRY:
            symbol = 'h';
            break;
        case RACE_HALF_ORC:
            symbol = 'o';
            break;
        case RACE_HALF_TROLL:
            symbol = 'T';
            break;
        case RACE_HALF_OGRE:
            symbol = 'O';
            break;
        case RACE_HALF_GIANT:
        case RACE_HALF_TITAN:
        case RACE_CYCLOPS:
            symbol = 'P';
            break;
        case RACE_YEEK:
            symbol = 'y';
            break;
        case RACE_KLACKON:
            symbol = 'K';
            break;
        case RACE_KOBOLD:
            symbol = 'k';
            break;
        case RACE_IMP:
            if (one_in_(13))
                symbol = 'U';
            else
                symbol = 'u';
            break;
        case RACE_DRACONIAN:
            symbol = 'd';
            break;
        case RACE_GOLEM:
        case RACE_ANDROID:
            symbol = 'g';
            break;
        case RACE_SKELETON:
            if (one_in_(13))
                symbol = 'L';
            else
                symbol = 's';
            break;
        case RACE_ZOMBIE:
            symbol = 'z';
            break;
        case RACE_VAMPIRE:
            symbol = 'V';
            break;
        case RACE_SPECTRE:
            symbol = 'G';
            break;
        case RACE_SPRITE:
            symbol = 'I';
            break;
        case RACE_ENT:
            symbol = '#';
            break;
        case RACE_ARCHON:
            symbol = 'A';
            break;
        case RACE_BALROG:
            symbol = 'U';
            break;
        default:
            symbol = 'p';
            break;
        }
        break;
    case MIMIC_DEMON:
        if (one_in_(13))
            symbol = 'U';
        else
            symbol = 'u';
        break;
    case MIMIC_DEMON_LORD:
        symbol = 'U';
        break;
    case MIMIC_VAMPIRE:
        symbol = 'V';
        break;
    }
    return symbol;
}

bool is_specific_player_race(player_type *creature_ptr, player_race_type prace) { return (!creature_ptr->mimic_form && (creature_ptr->prace == prace)); }

void calc_race_status(player_type *creature_ptr)
{
    const player_race *tmp_rp_ptr;
	
	if (creature_ptr->mimic_form)
        tmp_rp_ptr = &mimic_info[creature_ptr->mimic_form];
    else
        tmp_rp_ptr = &race_info[creature_ptr->prace];

    creature_ptr->see_infra += tmp_rp_ptr->infra;
    creature_ptr->skill_dis += tmp_rp_ptr->r_dis;
    creature_ptr->skill_dev += tmp_rp_ptr->r_dev;
    creature_ptr->skill_sav += tmp_rp_ptr->r_sav;
    creature_ptr->skill_stl += tmp_rp_ptr->r_stl;
    creature_ptr->skill_srh += tmp_rp_ptr->r_srh;
    creature_ptr->skill_fos += tmp_rp_ptr->r_fos;
    creature_ptr->skill_thn += tmp_rp_ptr->r_thn;
    creature_ptr->skill_thb += tmp_rp_ptr->r_thb;
    creature_ptr->skill_tht += tmp_rp_ptr->r_thb;

	for (int i = 0; i < A_MAX; i++) {
        creature_ptr->stat_add[i] += tmp_rp_ptr->r_adj[i];
    }


    if (creature_ptr->mimic_form) {
        switch (creature_ptr->mimic_form) {
        case MIMIC_DEMON:
            creature_ptr->hold_exp = TRUE;
            creature_ptr->resist_chaos = TRUE;
            creature_ptr->resist_neth = TRUE;
            creature_ptr->resist_fire = TRUE;
            creature_ptr->oppose_fire = 1;
            creature_ptr->see_inv = TRUE;
            creature_ptr->pspeed += 3;
            creature_ptr->redraw |= PR_STATUS;
            creature_ptr->to_a += 10;
            creature_ptr->dis_to_a += 10;
            break;
        case MIMIC_DEMON_LORD:
            creature_ptr->hold_exp = TRUE;
            creature_ptr->resist_chaos = TRUE;
            creature_ptr->resist_neth = TRUE;
            creature_ptr->immune_fire = TRUE;
            creature_ptr->resist_acid = TRUE;
            creature_ptr->resist_fire = TRUE;
            creature_ptr->resist_cold = TRUE;
            creature_ptr->resist_elec = TRUE;
            creature_ptr->resist_pois = TRUE;
            creature_ptr->resist_conf = TRUE;
            creature_ptr->resist_disen = TRUE;
            creature_ptr->resist_nexus = TRUE;
            creature_ptr->resist_fear = TRUE;
            creature_ptr->sh_fire = TRUE;
            creature_ptr->see_inv = TRUE;
            creature_ptr->telepathy = TRUE;
            creature_ptr->levitation = TRUE;
            creature_ptr->kill_wall = TRUE;
            creature_ptr->pspeed += 5;
            creature_ptr->to_a += 20;
            creature_ptr->dis_to_a += 20;
            break;
        case MIMIC_VAMPIRE:
            creature_ptr->resist_dark = TRUE;
            creature_ptr->hold_exp = TRUE;
            creature_ptr->resist_neth = TRUE;
            creature_ptr->resist_cold = TRUE;
            creature_ptr->resist_pois = TRUE;
            creature_ptr->see_inv = TRUE;
            creature_ptr->pspeed += 3;
            creature_ptr->to_a += 10;
            creature_ptr->dis_to_a += 10;
            if (creature_ptr->pclass != CLASS_NINJA)
                creature_ptr->lite = TRUE;
            break;
        }
    } else {
        switch (creature_ptr->prace) {
        case RACE_ELF:
            creature_ptr->resist_lite = TRUE;
            break;
        case RACE_HOBBIT:
            creature_ptr->hold_exp = TRUE;
            break;
        case RACE_GNOME:
            creature_ptr->free_act = TRUE;
            break;
        case RACE_DWARF:
            creature_ptr->resist_blind = TRUE;
            break;
        case RACE_HALF_ORC:
            creature_ptr->resist_dark = TRUE;
            break;
        case RACE_HALF_TROLL:
            creature_ptr->sustain_str = TRUE;

            if (creature_ptr->lev > 14) {
                creature_ptr->regenerate = TRUE;
                if (creature_ptr->pclass == CLASS_WARRIOR || creature_ptr->pclass == CLASS_BERSERKER) {
                    creature_ptr->slow_digest = TRUE;
                    /* Let's not make Regeneration
                     * a disadvantage for the poor warriors who can
                     * never learn a spell that satisfies hunger (actually
                     * neither can rogues, but half-trolls are not
                     * supposed to play rogues) */
                }
            }
            break;
        case RACE_AMBERITE:
            creature_ptr->sustain_con = TRUE;
            creature_ptr->regenerate = TRUE;
            break;
        case RACE_HIGH_ELF:
            creature_ptr->resist_lite = TRUE;
            creature_ptr->see_inv = TRUE;
            break;
        case RACE_BARBARIAN:
            creature_ptr->resist_fear = TRUE;
            break;
        case RACE_HALF_OGRE:
            creature_ptr->resist_dark = TRUE;
            creature_ptr->sustain_str = TRUE;
            break;
        case RACE_HALF_GIANT:
            creature_ptr->sustain_str = TRUE;
            creature_ptr->resist_shard = TRUE;
            break;
        case RACE_HALF_TITAN:
            creature_ptr->resist_chaos = TRUE;
            break;
        case RACE_CYCLOPS:
            creature_ptr->resist_sound = TRUE;
            break;
        case RACE_YEEK:
            creature_ptr->resist_acid = TRUE;
            if (creature_ptr->lev > 19)
                creature_ptr->immune_acid = TRUE;
            break;
        case RACE_KLACKON:
            creature_ptr->resist_conf = TRUE;
            creature_ptr->resist_acid = TRUE;
            creature_ptr->pspeed += (creature_ptr->lev) / 10;
            break;
        case RACE_KOBOLD:
            creature_ptr->resist_pois = TRUE;
            break;
        case RACE_NIBELUNG:
            creature_ptr->resist_disen = TRUE;
            creature_ptr->resist_dark = TRUE;
            break;
        case RACE_DARK_ELF:
            creature_ptr->resist_dark = TRUE;
            if (creature_ptr->lev > 19)
                creature_ptr->see_inv = TRUE;
            break;
        case RACE_DRACONIAN:
            creature_ptr->levitation = TRUE;
            if (creature_ptr->lev > 4)
                creature_ptr->resist_fire = TRUE;
            if (creature_ptr->lev > 9)
                creature_ptr->resist_cold = TRUE;
            if (creature_ptr->lev > 14)
                creature_ptr->resist_acid = TRUE;
            if (creature_ptr->lev > 19)
                creature_ptr->resist_elec = TRUE;
            if (creature_ptr->lev > 34)
                creature_ptr->resist_pois = TRUE;
            break;
        case RACE_MIND_FLAYER:
            creature_ptr->sustain_int = TRUE;
            creature_ptr->sustain_wis = TRUE;
            if (creature_ptr->lev > 14)
                creature_ptr->see_inv = TRUE;
            if (creature_ptr->lev > 29)
                creature_ptr->telepathy = TRUE;
            break;
        case RACE_IMP:
            creature_ptr->resist_fire = TRUE;
            if (creature_ptr->lev > 9)
                creature_ptr->see_inv = TRUE;
            break;
        case RACE_GOLEM:
            creature_ptr->slow_digest = TRUE;
            creature_ptr->free_act = TRUE;
            creature_ptr->see_inv = TRUE;
            creature_ptr->resist_pois = TRUE;
            if (creature_ptr->lev > 34)
                creature_ptr->hold_exp = TRUE;
            break;
        case RACE_SKELETON:
            creature_ptr->resist_shard = TRUE;
            creature_ptr->hold_exp = TRUE;
            creature_ptr->see_inv = TRUE;
            creature_ptr->resist_pois = TRUE;
            if (creature_ptr->lev > 9)
                creature_ptr->resist_cold = TRUE;
            break;
        case RACE_ZOMBIE:
            creature_ptr->resist_neth = TRUE;
            creature_ptr->hold_exp = TRUE;
            creature_ptr->see_inv = TRUE;
            creature_ptr->resist_pois = TRUE;
            creature_ptr->slow_digest = TRUE;
            if (creature_ptr->lev > 4)
                creature_ptr->resist_cold = TRUE;
            break;
        case RACE_VAMPIRE:
            creature_ptr->resist_dark = TRUE;
            creature_ptr->hold_exp = TRUE;
            creature_ptr->resist_neth = TRUE;
            creature_ptr->resist_cold = TRUE;
            creature_ptr->resist_pois = TRUE;
            if (creature_ptr->pclass != CLASS_NINJA)
                creature_ptr->lite = TRUE;
            break;
        case RACE_SPECTRE:
            creature_ptr->levitation = TRUE;
            creature_ptr->free_act = TRUE;
            creature_ptr->resist_neth = TRUE;
            creature_ptr->hold_exp = TRUE;
            creature_ptr->see_inv = TRUE;
            creature_ptr->resist_pois = TRUE;
            creature_ptr->slow_digest = TRUE;
            creature_ptr->resist_cold = TRUE;
            creature_ptr->pass_wall = TRUE;
            if (creature_ptr->lev > 34)
                creature_ptr->telepathy = TRUE;
            break;
        case RACE_SPRITE:
            creature_ptr->levitation = TRUE;
            creature_ptr->resist_lite = TRUE;

            creature_ptr->pspeed += (creature_ptr->lev) / 10;
            break;
        case RACE_BEASTMAN:
            creature_ptr->resist_conf = TRUE;
            creature_ptr->resist_sound = TRUE;
            break;
        case RACE_ENT:
            if (!creature_ptr->inventory_list[INVEN_RARM].k_idx)
                creature_ptr->skill_dig += creature_ptr->lev * 10;

            if (creature_ptr->lev > 25)
                creature_ptr->stat_add[A_STR]++;
            if (creature_ptr->lev > 40)
                creature_ptr->stat_add[A_STR]++;
            if (creature_ptr->lev > 45)
                creature_ptr->stat_add[A_STR]++;

            if (creature_ptr->lev > 25)
                creature_ptr->stat_add[A_DEX]--;
            if (creature_ptr->lev > 40)
                creature_ptr->stat_add[A_DEX]--;
            if (creature_ptr->lev > 45)
                creature_ptr->stat_add[A_DEX]--;

            if (creature_ptr->lev > 25)
                creature_ptr->stat_add[A_CON]++;
            if (creature_ptr->lev > 40)
                creature_ptr->stat_add[A_CON]++;
            if (creature_ptr->lev > 45)
                creature_ptr->stat_add[A_CON]++;
            break;
        case RACE_ARCHON:
            creature_ptr->levitation = TRUE;
            creature_ptr->see_inv = TRUE;
            break;
        case RACE_BALROG:
            creature_ptr->resist_fire = TRUE;
            creature_ptr->resist_neth = TRUE;
            creature_ptr->hold_exp = TRUE;
            if (creature_ptr->lev > 9)
                creature_ptr->see_inv = TRUE;
            if (creature_ptr->lev > 44) {
                creature_ptr->oppose_fire = 1;
                creature_ptr->redraw |= PR_STATUS;
            }

            break;
        case RACE_DUNADAN:
            creature_ptr->sustain_con = TRUE;
            break;
        case RACE_S_FAIRY:
            creature_ptr->levitation = TRUE;
            break;
        case RACE_KUTAR:
            creature_ptr->resist_conf = TRUE;
            break;
        case RACE_ANDROID:
            creature_ptr->slow_digest = TRUE;
            creature_ptr->free_act = TRUE;
            creature_ptr->resist_pois = TRUE;
            creature_ptr->hold_exp = TRUE;
            break;
        case RACE_MERFOLK:
            creature_ptr->resist_water = TRUE;
            break;
        default:
            break;
        }
    }
}
