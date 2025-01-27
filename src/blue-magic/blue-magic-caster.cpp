﻿/*!
 * @file blue-magic-caster.cpp
 * @brief 青魔法のその他系統の呪文定義と詠唱時分岐処理
 */

#include "blue-magic/blue-magic-caster.h"
#include "blue-magic/blue-magic-ball-bolt.h"
#include "blue-magic/blue-magic-breath.h"
#include "blue-magic/blue-magic-spirit-curse.h"
#include "blue-magic/blue-magic-status.h"
#include "blue-magic/blue-magic-summon.h"
#include "blue-magic/blue-magic-util.h"
#include "blue-magic/learnt-info.h"
#include "floor/cave.h"
#include "hpmp/hp-mp-processor.h"
#include "monster-race/monster-race.h"
#include "monster-race/race-ability-flags.h"
#include "monster-race/race-flags-resistance.h"
#include "monster-race/race-flags1.h"
#include "monster/monster-describer.h"
#include "monster/monster-info.h"
#include "monster/monster-status.h"
#include "mspell/mspell-damage-calculator.h"
#include "spell-kind/spells-launcher.h"
#include "spell-kind/spells-lite.h"
#include "spell-kind/spells-neighbor.h"
#include "spell-kind/spells-sight.h"
#include "spell-kind/spells-teleport.h"
#include "spell-kind/spells-world.h"
#include "spell/spell-types.h"
#include "spell/spells-status.h"
#include "status/bad-status-setter.h"
#include "status/body-improvement.h"
#include "status/buff-setter.h"
#include "system/floor-type-definition.h"
#include "system/grid-type-definition.h"
#include "system/monster-race-definition.h"
#include "system/monster-type-definition.h"
#include "system/player-type-definition.h"
#include "target/projection-path-calculator.h"
#include "target/target-checker.h"
#include "target/target-getter.h"
#include "target/target-setter.h"
#include "target/target-types.h"
#include "view/display-messages.h"

static bool cast_blue_dispel(player_type *caster_ptr)
{
    if (!target_set(caster_ptr, TARGET_KILL))
        return false;

    MONSTER_IDX m_idx = caster_ptr->current_floor_ptr->grid_array[target_row][target_col].m_idx;
    if ((m_idx == 0) || !player_has_los_bold(caster_ptr, target_row, target_col)
        || !projectable(caster_ptr, caster_ptr->y, caster_ptr->x, target_row, target_col))
        return true;

    dispel_monster_status(caster_ptr, m_idx);
    return true;
}

static bool cast_blue_rocket(player_type *caster_ptr, bmc_type *bmc_ptr)
{
    if (!get_aim_dir(caster_ptr, &bmc_ptr->dir))
        return false;

    msg_print(_("ロケットを発射した。", "You fire a rocket."));
    bmc_ptr->damage = monspell_bluemage_damage(caster_ptr, RF_ABILITY::ROCKET, bmc_ptr->plev, DAM_ROLL);
    fire_rocket(caster_ptr, GF_ROCKET, bmc_ptr->dir, bmc_ptr->damage, 2);
    return true;
}

static bool cast_blue_shoot(player_type *caster_ptr, bmc_type *bmc_ptr)
{
    if (!get_aim_dir(caster_ptr, &bmc_ptr->dir))
        return false;

    msg_print(_("矢を放った。", "You fire an arrow."));
    bmc_ptr->damage = monspell_bluemage_damage(caster_ptr, RF_ABILITY::SHOOT, bmc_ptr->plev, DAM_ROLL);
    fire_bolt(caster_ptr, GF_ARROW, bmc_ptr->dir, bmc_ptr->damage);
    return true;
}

static bool cast_blue_hand_doom(player_type *caster_ptr, bmc_type *bmc_ptr)
{
    if (!get_aim_dir(caster_ptr, &bmc_ptr->dir))
        return false;

    msg_print(_("<破滅の手>を放った！", "You invoke the Hand of Doom!"));
    fire_ball_hide(caster_ptr, GF_HAND_DOOM, bmc_ptr->dir, bmc_ptr->plev * 3, 0);
    return true;
}

static bool exe_blue_teleport_back(player_type *caster_ptr, GAME_TEXT *m_name)
{
    monster_type *m_ptr;
    monster_race *r_ptr;
    floor_type *floor_ptr = caster_ptr->current_floor_ptr;
    if ((floor_ptr->grid_array[target_row][target_col].m_idx == 0) || !player_has_los_bold(caster_ptr, target_row, target_col)
        || !projectable(caster_ptr, caster_ptr->y, caster_ptr->x, target_row, target_col))
        return true;

    m_ptr = &floor_ptr->m_list[floor_ptr->grid_array[target_row][target_col].m_idx];
    r_ptr = &r_info[m_ptr->r_idx];
    monster_desc(caster_ptr, m_name, m_ptr, 0);
    if ((r_ptr->flagsr & RFR_RES_TELE) == 0)
        return false;

    if ((r_ptr->flags1 & RF1_UNIQUE) || (r_ptr->flagsr & RFR_RES_ALL)) {
        if (is_original_ap_and_seen(caster_ptr, m_ptr))
            r_ptr->r_flagsr |= RFR_RES_TELE;

        msg_format(_("%sには効果がなかった！", "%s is unaffected!"), m_name);
        return true;
    }

    if (r_ptr->level <= randint1(100))
        return false;

    if (is_original_ap_and_seen(caster_ptr, m_ptr))
        r_ptr->r_flagsr |= RFR_RES_TELE;

    msg_format(_("%sには耐性がある！", "%s resists!"), m_name);
    return true;
}

static bool cast_blue_teleport_back(player_type *caster_ptr)
{
    if (!target_set(caster_ptr, TARGET_KILL))
        return false;

    GAME_TEXT m_name[MAX_NLEN];
    if (exe_blue_teleport_back(caster_ptr, m_name))
        return true;

    msg_format(_("%sを引き戻した。", "You command %s to return."), m_name);
    teleport_monster_to(
        caster_ptr, caster_ptr->current_floor_ptr->grid_array[target_row][target_col].m_idx, caster_ptr->y, caster_ptr->x, 100, TELEPORT_PASSIVE);
    return true;
}

static bool cast_blue_teleport_away(player_type *caster_ptr, bmc_type *bmc_ptr)
{
    if (!get_aim_dir(caster_ptr, &bmc_ptr->dir))
        return false;

    (void)fire_beam(caster_ptr, GF_AWAY_ALL, bmc_ptr->dir, 100);
    return true;
}

static bool cast_blue_psy_spear(player_type *caster_ptr, bmc_type *bmc_ptr)
{
    if (!get_aim_dir(caster_ptr, &bmc_ptr->dir))
        return false;

    msg_print(_("光の剣を放った。", "You throw a psycho-spear."));
    bmc_ptr->damage = monspell_bluemage_damage(caster_ptr, RF_ABILITY::PSY_SPEAR, bmc_ptr->plev, DAM_ROLL);
    (void)fire_beam(caster_ptr, GF_PSY_SPEAR, bmc_ptr->dir, bmc_ptr->damage);
    return true;
}

static bool cast_blue_make_trap(player_type *caster_ptr)
{
    if (!target_set(caster_ptr, TARGET_KILL))
        return false;

    msg_print(_("呪文を唱えて邪悪に微笑んだ。", "You cast a spell and cackle evilly."));
    trap_creation(caster_ptr, target_row, target_col);
    return true;
}

static bool switch_cast_blue_magic(player_type *caster_ptr, bmc_type *bmc_ptr, RF_ABILITY spell)
{
    switch (spell) {
    case RF_ABILITY::SHRIEK:
        msg_print(_("かん高い金切り声をあげた。", "You make a high pitched shriek."));
        aggravate_monsters(caster_ptr, 0);
        return true;
    case RF_ABILITY::XXX1:
    case RF_ABILITY::XXX2:
    case RF_ABILITY::XXX3:
    case RF_ABILITY::XXX4:
        return true;
    case RF_ABILITY::DISPEL:
        return cast_blue_dispel(caster_ptr);
    case RF_ABILITY::ROCKET:
        return cast_blue_rocket(caster_ptr, bmc_ptr);
    case RF_ABILITY::SHOOT:
        return cast_blue_shoot(caster_ptr, bmc_ptr);
    case RF_ABILITY::BR_ACID:
        return cast_blue_breath_acid(caster_ptr, bmc_ptr);
    case RF_ABILITY::BR_ELEC:
        return cast_blue_breath_elec(caster_ptr, bmc_ptr);
    case RF_ABILITY::BR_FIRE:
        return cast_blue_breath_fire(caster_ptr, bmc_ptr);
    case RF_ABILITY::BR_COLD:
        return cast_blue_breath_cold(caster_ptr, bmc_ptr);
    case RF_ABILITY::BR_POIS:
        return cast_blue_breath_pois(caster_ptr, bmc_ptr);
    case RF_ABILITY::BR_NETH:
        return cast_blue_breath_nether(caster_ptr, bmc_ptr);
    case RF_ABILITY::BR_LITE:
        return cast_blue_breath_lite(caster_ptr, bmc_ptr);
    case RF_ABILITY::BR_DARK:
        return cast_blue_breath_dark(caster_ptr, bmc_ptr);
    case RF_ABILITY::BR_CONF:
        return cast_blue_breath_conf(caster_ptr, bmc_ptr);
    case RF_ABILITY::BR_SOUN:
        return cast_blue_breath_sound(caster_ptr, bmc_ptr);
    case RF_ABILITY::BR_CHAO:
        return cast_blue_breath_chaos(caster_ptr, bmc_ptr);
    case RF_ABILITY::BR_DISE:
        return cast_blue_breath_disenchant(caster_ptr, bmc_ptr);
    case RF_ABILITY::BR_NEXU:
        return cast_blue_breath_nexus(caster_ptr, bmc_ptr);
    case RF_ABILITY::BR_TIME:
        return cast_blue_breath_time(caster_ptr, bmc_ptr);
    case RF_ABILITY::BR_INER:
        return cast_blue_breath_inertia(caster_ptr, bmc_ptr);
    case RF_ABILITY::BR_GRAV:
        return cast_blue_breath_gravity(caster_ptr, bmc_ptr);
    case RF_ABILITY::BR_SHAR:
        return cast_blue_breath_shards(caster_ptr, bmc_ptr);
    case RF_ABILITY::BR_PLAS:
        return cast_blue_breath_plasma(caster_ptr, bmc_ptr);
    case RF_ABILITY::BR_FORC:
        return cast_blue_breath_force(caster_ptr, bmc_ptr);
    case RF_ABILITY::BR_MANA:
        return cast_blue_breath_mana(caster_ptr, bmc_ptr);
    case RF_ABILITY::BR_NUKE:
        return cast_blue_breath_nuke(caster_ptr, bmc_ptr);
    case RF_ABILITY::BR_DISI:
        return cast_blue_breath_disintegration(caster_ptr, bmc_ptr);
    case RF_ABILITY::BA_ACID:
        return cast_blue_ball_acid(caster_ptr, bmc_ptr);
    case RF_ABILITY::BA_ELEC:
        return cast_blue_ball_elec(caster_ptr, bmc_ptr);
    case RF_ABILITY::BA_FIRE:
        return cast_blue_ball_fire(caster_ptr, bmc_ptr);
    case RF_ABILITY::BA_COLD:
        return cast_blue_ball_cold(caster_ptr, bmc_ptr);
    case RF_ABILITY::BA_POIS:
        return cast_blue_ball_pois(caster_ptr, bmc_ptr);
    case RF_ABILITY::BA_NUKE:
        return cast_blue_ball_nuke(caster_ptr, bmc_ptr);
    case RF_ABILITY::BA_NETH:
        return cast_blue_ball_nether(caster_ptr, bmc_ptr);
    case RF_ABILITY::BA_CHAO:
        return cast_blue_ball_chaos(caster_ptr, bmc_ptr);
    case RF_ABILITY::BA_WATE:
        return cast_blue_ball_water(caster_ptr, bmc_ptr);
    case RF_ABILITY::BA_LITE:
        return cast_blue_ball_star_burst(caster_ptr, bmc_ptr);
    case RF_ABILITY::BA_DARK:
        return cast_blue_ball_dark_storm(caster_ptr, bmc_ptr);
    case RF_ABILITY::BA_MANA:
        return cast_blue_ball_mana_storm(caster_ptr, bmc_ptr);
    case RF_ABILITY::DRAIN_MANA:
        return cast_blue_drain_mana(caster_ptr, bmc_ptr);
    case RF_ABILITY::MIND_BLAST:
        return cast_blue_mind_blast(caster_ptr, bmc_ptr);
    case RF_ABILITY::BRAIN_SMASH:
        return cast_blue_brain_smash(caster_ptr, bmc_ptr);
    case RF_ABILITY::CAUSE_1:
        return cast_blue_curse_1(caster_ptr, bmc_ptr);
    case RF_ABILITY::CAUSE_2:
        return cast_blue_curse_2(caster_ptr, bmc_ptr);
    case RF_ABILITY::CAUSE_3:
        return cast_blue_curse_3(caster_ptr, bmc_ptr);
    case RF_ABILITY::CAUSE_4:
        return cast_blue_curse_4(caster_ptr, bmc_ptr);
    case RF_ABILITY::BO_ACID:
        return cast_blue_bolt_acid(caster_ptr, bmc_ptr);
    case RF_ABILITY::BO_ELEC:
        return cast_blue_bolt_elec(caster_ptr, bmc_ptr);
    case RF_ABILITY::BO_FIRE:
        return cast_blue_bolt_fire(caster_ptr, bmc_ptr);
    case RF_ABILITY::BO_COLD:
        return cast_blue_bolt_cold(caster_ptr, bmc_ptr);
    case RF_ABILITY::BO_NETH:
        return cast_blue_bolt_nether(caster_ptr, bmc_ptr);
    case RF_ABILITY::BO_WATE:
        return cast_blue_bolt_water(caster_ptr, bmc_ptr);
    case RF_ABILITY::BO_MANA:
        return cast_blue_bolt_mana(caster_ptr, bmc_ptr);
    case RF_ABILITY::BO_PLAS:
        return cast_blue_bolt_plasma(caster_ptr, bmc_ptr);
    case RF_ABILITY::BO_ICEE:
        return cast_blue_bolt_icee(caster_ptr, bmc_ptr);
    case RF_ABILITY::MISSILE:
        return cast_blue_bolt_missile(caster_ptr, bmc_ptr);
    case RF_ABILITY::SCARE:
        return cast_blue_scare(caster_ptr, bmc_ptr);
    case RF_ABILITY::BLIND:
        return cast_blue_blind(caster_ptr, bmc_ptr);
    case RF_ABILITY::CONF:
        return cast_blue_confusion(caster_ptr, bmc_ptr);
    case RF_ABILITY::SLOW:
        return cast_blue_slow(caster_ptr, bmc_ptr);
    case RF_ABILITY::HOLD:
        return cast_blue_sleep(caster_ptr, bmc_ptr);
    case RF_ABILITY::HASTE:
        (void)set_fast(caster_ptr, randint1(20 + bmc_ptr->plev) + bmc_ptr->plev, false);
        return true;
    case RF_ABILITY::HAND_DOOM:
        return cast_blue_hand_doom(caster_ptr, bmc_ptr);
    case RF_ABILITY::HEAL:
        msg_print(_("自分の傷に念を集中した。", "You concentrate on your wounds!"));
        (void)hp_player(caster_ptr, bmc_ptr->plev * 4);
        (void)set_stun(caster_ptr, 0);
        (void)set_cut(caster_ptr, 0);
        return true;
    case RF_ABILITY::INVULNER:
        msg_print(_("無傷の球の呪文を唱えた。", "You cast a Globe of Invulnerability."));
        (void)set_invuln(caster_ptr, randint1(4) + 4, false);
        return true;
    case RF_ABILITY::BLINK:
        teleport_player(caster_ptr, 10, TELEPORT_SPONTANEOUS);
        return true;
    case RF_ABILITY::TPORT:
        teleport_player(caster_ptr, bmc_ptr->plev * 5, TELEPORT_SPONTANEOUS);
        return true;
    case RF_ABILITY::WORLD:
        (void)time_walk(caster_ptr);
        return true;
    case RF_ABILITY::SPECIAL:
        return true;
    case RF_ABILITY::TELE_TO:
        return cast_blue_teleport_back(caster_ptr);
    case RF_ABILITY::TELE_AWAY:
        return cast_blue_teleport_away(caster_ptr, bmc_ptr);
    case RF_ABILITY::TELE_LEVEL:
        return teleport_level_other(caster_ptr);
    case RF_ABILITY::PSY_SPEAR:
        return cast_blue_psy_spear(caster_ptr, bmc_ptr);
    case RF_ABILITY::DARKNESS:
        msg_print(_("暗闇の中で手を振った。", "You gesture in shadow."));
        (void)unlite_area(caster_ptr, 10, 3);
        return true;
    case RF_ABILITY::TRAPS:
        return cast_blue_make_trap(caster_ptr);
    case RF_ABILITY::FORGET:
        msg_print(_("しかし何も起きなかった。", "Nothing happens."));
        return true;
    case RF_ABILITY::RAISE_DEAD:
        msg_print(_("死者復活の呪文を唱えた。", "You animate the dead."));
        (void)animate_dead(caster_ptr, 0, caster_ptr->y, caster_ptr->x);
        return true;
    case RF_ABILITY::S_KIN:
        return cast_blue_summon_kin(caster_ptr, bmc_ptr);
    case RF_ABILITY::S_CYBER:
        return cast_blue_summon_cyber(caster_ptr, bmc_ptr);
    case RF_ABILITY::S_MONSTER:
        return cast_blue_summon_monster(caster_ptr, bmc_ptr);
    case RF_ABILITY::S_MONSTERS:
        return cast_blue_summon_monsters(caster_ptr, bmc_ptr);
    case RF_ABILITY::S_ANT:
        return cast_blue_summon_ant(caster_ptr, bmc_ptr);
    case RF_ABILITY::S_SPIDER:
        return cast_blue_summon_spider(caster_ptr, bmc_ptr);
    case RF_ABILITY::S_HOUND:
        return cast_blue_summon_hound(caster_ptr, bmc_ptr);
    case RF_ABILITY::S_HYDRA:
        return cast_blue_summon_hydra(caster_ptr, bmc_ptr);
    case RF_ABILITY::S_ANGEL:
        return cast_blue_summon_angel(caster_ptr, bmc_ptr);
    case RF_ABILITY::S_DEMON:
        return cast_blue_summon_demon(caster_ptr, bmc_ptr);
    case RF_ABILITY::S_UNDEAD:
        return cast_blue_summon_undead(caster_ptr, bmc_ptr);
    case RF_ABILITY::S_DRAGON:
        return cast_blue_summon_dragon(caster_ptr, bmc_ptr);
    case RF_ABILITY::S_HI_UNDEAD:
        return cast_blue_summon_high_undead(caster_ptr, bmc_ptr);
    case RF_ABILITY::S_HI_DRAGON:
        return cast_blue_summon_high_dragon(caster_ptr, bmc_ptr);
    case RF_ABILITY::S_AMBERITES:
        return cast_blue_summon_amberite(caster_ptr, bmc_ptr);
    case RF_ABILITY::S_UNIQUE:
        return cast_blue_summon_unique(caster_ptr, bmc_ptr);
    default:
        msg_print("hoge?");
        return true;
    }
}

/*!
 * @brief 青魔法の発動 /
 * do_cmd_cast calls this function if the player's class is 'blue-mage'.
 * @param spell 発動するモンスター攻撃のID
 * @param success TRUEは成功時、FALSEは失敗時の処理を行う
 * @return 処理を実行したらTRUE、キャンセルした場合FALSEを返す。
 */
bool cast_learned_spell(player_type *caster_ptr, RF_ABILITY spell, const bool success)
{
    bmc_type tmp_bm;
    bmc_type *bmc_ptr = initialize_blue_magic_type(caster_ptr, &tmp_bm, success, get_pseudo_monstetr_level);
    if (!switch_cast_blue_magic(caster_ptr, bmc_ptr, spell))
        return false;

    if (bmc_ptr->no_trump)
        msg_print(_("何も現れなかった。", "No one appeared."));

    return true;
}
