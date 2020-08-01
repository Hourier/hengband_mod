﻿/*!
 * @file mutation.c
 * @brief 突然変異ルールの実装 / Mutation effects (and racial powers)
 * @date 2014/01/11
 * @author
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke\n
 *\n
 * This software may be copied and distributed for educational, research,\n
 * and not for profit purposes provided that this copyright and statement\n
 * are included in all such copies.  Other copyrights may also apply.\n
 * 2014 Deskull rearranged comment for Doxygen.\n
 */

#include "mutation/mutation.h"
#include "cmd-io/cmd-dump.h"
#include "cmd-item/cmd-throw.h"
#include "core/asking-player.h"
#include "core/player-update-types.h"
#include "core/show-file.h"
#include "core/stuff-handler.h"
#include "effect/spells-effect-util.h"
#include "game-option/play-record-options.h"
#include "grid/grid.h"
#include "inventory/inventory-slot-types.h"
#include "io/write-diary.h"
#include "mind/mind-mage.h"
#include "mind/mind-warrior.h"
#include "monster-floor/monster-remover.h"
#include "monster-floor/monster-summon.h"
#include "monster-floor/place-monster-types.h"
#include "monster-race/monster-race.h"
#include "monster-race/race-flags1.h"
#include "monster-race/race-flags3.h"
#include "monster/monster-describer.h"
#include "monster/monster-description-types.h"
#include "monster/monster-flag-types.h"
#include "monster/monster-info.h"
#include "monster/smart-learn-types.h"
#include "mutation/gain-mutation-switcher.h"
#include "mutation/lose-mutation-switcher.h"
#include "mutation/mutation-flag-types.h"
#include "mutation/mutation-investor.h" // todo 相互依存している、このファイルからの依存はOK.
#include "mutation/mutation-techniques.h"
#include "mutation/mutation-util.h"
#include "object-enchant/item-feeling.h"
#include "object-hook/hook-checker.h"
#include "player/avatar.h"
#include "player/player-class.h"
#include "player/player-damage.h"
#include "player/player-personalities-types.h"
#include "player/player-race-types.h"
#include "player/selfinfo.h"
#include "racial/racial-vampire.h"
#include "spell-kind/earthquake.h"
#include "spell-kind/spells-charm.h"
#include "spell-kind/spells-detection.h"
#include "spell-kind/spells-fetcher.h"
#include "spell-kind/spells-launcher.h"
#include "spell-kind/spells-lite.h"
#include "spell-kind/spells-sight.h"
#include "spell-kind/spells-teleport.h"
#include "spell-kind/spells-world.h"
#include "spell-realm/spells-sorcery.h"
#include "spell/spell-types.h"
#include "spell/spells-status.h"
#include "spell/spells-summon.h"
#include "status/element-resistance.h"
#include "status/shape-changer.h"
#include "system/floor-type-definition.h"
#include "system/object-type-definition.h"
#include "target/target-getter.h"
#include "view/display-messages.h"

/*!
 * @brief プレイヤーから突然変異を取り除く
 * @param choose_mut 取り除きたい突然変異のID、0ならばランダムに消去
 * @return なし
 */
bool lose_mutation(player_type *creature_ptr, MUTATION_IDX choose_mut)
{
    glm_type tmp_glm;
    glm_type *glm_ptr = initialize_glm_type(&tmp_glm, choose_mut);
    int attempts_left = 20;
    if (glm_ptr->choose_mut)
        attempts_left = 1;

    while (attempts_left--) {
        switch_lose_mutation(creature_ptr, glm_ptr);
        if (glm_ptr->muta_class && glm_ptr->muta_which) {
            if (*(glm_ptr->muta_class)&glm_ptr->muta_which) {
                glm_ptr->muta_chosen = TRUE;
            }
        }

        if (glm_ptr->muta_chosen)
            break;
    }

    if (!glm_ptr->muta_chosen)
        return FALSE;

    msg_print(glm_ptr->muta_desc);
    if (glm_ptr->muta_class != NULL)
        *glm_ptr->muta_class &= ~(glm_ptr->muta_which);

    creature_ptr->update |= PU_BONUS;
    handle_stuff(creature_ptr);
    creature_ptr->mutant_regenerate_mod = calc_mutant_regenerate_mod(creature_ptr);
    return TRUE;
}

void lose_all_mutations(player_type *creature_ptr)
{
    if (creature_ptr->muta1 || creature_ptr->muta2 || creature_ptr->muta3) {
        chg_virtue(creature_ptr, V_CHANCE, -5);
        msg_print(_("全ての突然変異が治った。", "You are cured of all mutations."));
        creature_ptr->muta1 = creature_ptr->muta2 = creature_ptr->muta3 = 0;
        creature_ptr->update |= PU_BONUS;
        handle_stuff(creature_ptr);
        creature_ptr->mutant_regenerate_mod = calc_mutant_regenerate_mod(creature_ptr);
    }
}

/*!
 * @brief 現在プレイヤー得ている突然変異の数を返す。
 * @return 現在得ている突然変異の数
 */
static int count_mutations(player_type *creature_ptr)
{
    return count_bits(creature_ptr->muta1) + count_bits(creature_ptr->muta2) + count_bits(creature_ptr->muta3);
}

/*!
 * @brief 突然変異による自然回復ペナルティをパーセント値で返す /
 * Return the modifier to the regeneration rate (in percent)
 * @return ペナルティ修正(%)
 * @details
 * Beastman get 10 "free" mutations and only 5% decrease per additional mutation.
 * Max 90% decrease in regeneration speed.
 */
int calc_mutant_regenerate_mod(player_type *creature_ptr)
{
    int regen;
    int mod = 10;
    int count = count_mutations(creature_ptr);
    if (creature_ptr->pseikaku == PERSONALITY_LUCKY)
        count--;

    if (creature_ptr->prace == RACE_BEASTMAN) {
        count -= 10;
        mod = 5;
    }

    if (count <= 0)
        return 100;

    regen = 100 - count * mod;
    if (regen < 10)
        regen = 10;

    return (regen);
}

/*!
 * @brief 突然変異のレイシャル効果実装
 * @param creature_ptr プレーヤーへの参照ポインタ
 * @param power 発動させる突然変異レイシャルのID
 * @return レイシャルを実行した場合TRUE、キャンセルした場合FALSEを返す
 */
bool exe_mutation_power(player_type *creature_ptr, int power)
{
    DIRECTION dir = 0;
    PLAYER_LEVEL lvl = creature_ptr->lev;
    switch (power) {
    case MUT1_SPIT_ACID:
        if (!get_aim_dir(creature_ptr, &dir))
            return FALSE;

        stop_mouth(creature_ptr);
        msg_print(_("酸を吐きかけた...", "You spit acid..."));
        fire_ball(creature_ptr, GF_ACID, dir, lvl, 1 + (lvl / 30));
        return TRUE;
    case MUT1_BR_FIRE:
        if (!get_aim_dir(creature_ptr, &dir))
            return FALSE;

        stop_mouth(creature_ptr);
        msg_print(_("あなたは火炎のブレスを吐いた...", "You breathe fire..."));
        fire_breath(creature_ptr, GF_FIRE, dir, lvl * 2, 1 + (lvl / 20));
        return TRUE;
    case MUT1_HYPN_GAZE:
        if (!get_aim_dir(creature_ptr, &dir))
            return FALSE;

        msg_print(_("あなたの目は幻惑的になった...", "Your eyes look mesmerizing..."));
        (void)charm_monster(creature_ptr, dir, lvl);
        return TRUE;
    case MUT1_TELEKINES:
        if (!get_aim_dir(creature_ptr, &dir))
            return FALSE;

        msg_print(_("集中している...", "You concentrate..."));
        fetch_item(creature_ptr, dir, lvl * 10, TRUE);
        return TRUE;
    case MUT1_VTELEPORT:
        msg_print(_("集中している...", "You concentrate..."));
        teleport_player(creature_ptr, 10 + 4 * lvl, TELEPORT_SPONTANEOUS);
        return TRUE;
    case MUT1_MIND_BLST:
        if (!get_aim_dir(creature_ptr, &dir))
            return FALSE;

        msg_print(_("集中している...", "You concentrate..."));
        fire_bolt(creature_ptr, GF_PSI, dir, damroll(3 + ((lvl - 1) / 5), 3));
        return TRUE;
    case MUT1_RADIATION:
        msg_print(_("体から放射能が発生した！", "Radiation flows from your body!"));
        fire_ball(creature_ptr, GF_NUKE, 0, (lvl * 2), 3 + (lvl / 20));
        return TRUE;
    case MUT1_VAMPIRISM:
        vampirism(creature_ptr);
        return TRUE;
    case MUT1_SMELL_MET:
        stop_mouth(creature_ptr);
        (void)detect_treasure(creature_ptr, DETECT_RAD_DEFAULT);
        return TRUE;
    case MUT1_SMELL_MON:
        stop_mouth(creature_ptr);
        (void)detect_monsters_normal(creature_ptr, DETECT_RAD_DEFAULT);
        return TRUE;
    case MUT1_BLINK:
        teleport_player(creature_ptr, 10, TELEPORT_SPONTANEOUS);
        return TRUE;
    case MUT1_EAT_ROCK:
        return eat_rock(creature_ptr);
    case MUT1_SWAP_POS:
        project_length = -1;
        if (!get_aim_dir(creature_ptr, &dir)) {
            project_length = 0;
            return FALSE;
        }

        (void)teleport_swap(creature_ptr, dir);
        project_length = 0;
        return TRUE;
    case MUT1_SHRIEK:
        stop_mouth(creature_ptr);
        (void)fire_ball(creature_ptr, GF_SOUND, 0, 2 * lvl, 8);
        (void)aggravate_monsters(creature_ptr, 0);
        return TRUE;
    case MUT1_ILLUMINE:
        (void)lite_area(creature_ptr, damroll(2, (lvl / 2)), (lvl / 10) + 1);
        return TRUE;
    case MUT1_DET_CURSE:
        for (int i = 0; i < INVEN_TOTAL; i++) {
            object_type *o_ptr = &creature_ptr->inventory_list[i];
            if ((o_ptr->k_idx == 0) || !object_is_cursed(o_ptr))
                continue;

            o_ptr->feeling = FEEL_CURSED;
        }

        return TRUE;
    case MUT1_BERSERK:
        (void)berserk(creature_ptr, randint1(25) + 25);
        return TRUE;
    case MUT1_POLYMORPH:
        if (!get_check(_("変身します。よろしいですか？", "You will polymorph your self. Are you sure? ")))
            return FALSE;

        do_poly_self(creature_ptr);
        return TRUE;
    case MUT1_MIDAS_TCH:
        return alchemy(creature_ptr);
    case MUT1_GROW_MOLD:
        for (DIRECTION i = 0; i < 8; i++)
            summon_specific(creature_ptr, -1, creature_ptr->y, creature_ptr->x, lvl, SUMMON_MOLD, PM_FORCE_PET);

        return TRUE;
    case MUT1_RESIST: {
        int num = lvl / 10;
        TIME_EFFECT dur = randint1(20) + 20;
        if (randint0(5) < num) {
            (void)set_oppose_acid(creature_ptr, dur, FALSE);
            num--;
        }

        if (randint0(4) < num) {
            (void)set_oppose_elec(creature_ptr, dur, FALSE);
            num--;
        }

        if (randint0(3) < num) {
            (void)set_oppose_fire(creature_ptr, dur, FALSE);
            num--;
        }

        if (randint0(2) < num) {
            (void)set_oppose_cold(creature_ptr, dur, FALSE);
            num--;
        }

        if (num != 0) {
            (void)set_oppose_pois(creature_ptr, dur, FALSE);
            num--;
        }

        return TRUE;
    }
    case MUT1_EARTHQUAKE:
        (void)earthquake(creature_ptr, creature_ptr->y, creature_ptr->x, 10, 0);
        return TRUE;
    case MUT1_EAT_MAGIC:
        return eat_magic(creature_ptr, creature_ptr->lev * 2);
    case MUT1_WEIGH_MAG:
        report_magics(creature_ptr);
        return TRUE;
    case MUT1_STERILITY:
        msg_print(_("突然頭が痛くなった！", "You suddenly have a headache!"));
        take_hit(creature_ptr, DAMAGE_LOSELIFE, randint1(17) + 17, _("禁欲を強いた疲労", "the strain of forcing abstinence"), -1);
        creature_ptr->current_floor_ptr->num_repro += MAX_REPRO;
        return TRUE;
    case MUT1_HIT_AND_AWAY:
        return hit_and_away(creature_ptr);
    case MUT1_DAZZLE:
        stun_monsters(creature_ptr, lvl * 4);
        confuse_monsters(creature_ptr, lvl * 4);
        turn_monsters(creature_ptr, lvl * 4);
        return TRUE;
    case MUT1_LASER_EYE:
        if (!get_aim_dir(creature_ptr, &dir))
            return FALSE;

        fire_beam(creature_ptr, GF_LITE, dir, 2 * lvl);
        return TRUE;
    case MUT1_RECALL:
        return recall_player(creature_ptr, randint0(21) + 15);
    case MUT1_BANISH: {
        if (!get_direction(creature_ptr, &dir, FALSE, FALSE))
            return FALSE;

        POSITION y = creature_ptr->y + ddy[dir];
        POSITION x = creature_ptr->x + ddx[dir];
        grid_type *g_ptr;
        g_ptr = &creature_ptr->current_floor_ptr->grid_array[y][x];

        if (!g_ptr->m_idx) {
            msg_print(_("邪悪な存在を感じとれません！", "You sense no evil there!"));
            return TRUE;
        }

        monster_type *m_ptr;
        m_ptr = &creature_ptr->current_floor_ptr->m_list[g_ptr->m_idx];
        monster_race *r_ptr;
        r_ptr = &r_info[m_ptr->r_idx];
        if ((r_ptr->flags3 & RF3_EVIL) && !(r_ptr->flags1 & RF1_QUESTOR) && !(r_ptr->flags1 & RF1_UNIQUE) && !creature_ptr->current_floor_ptr->inside_arena
            && !creature_ptr->current_floor_ptr->inside_quest && (r_ptr->level < randint1(creature_ptr->lev + 50)) && !(m_ptr->mflag2 & MFLAG2_NOGENO)) {
            if (record_named_pet && is_pet(m_ptr) && m_ptr->nickname) {
                GAME_TEXT m_name[MAX_NLEN];
                monster_desc(creature_ptr, m_name, m_ptr, MD_INDEF_VISIBLE);
                exe_write_diary(creature_ptr, DIARY_NAMED_PET, RECORD_NAMED_PET_GENOCIDE, m_name);
            }

            delete_monster_idx(creature_ptr, g_ptr->m_idx);
            msg_print(_("その邪悪なモンスターは硫黄臭い煙とともに消え去った！", "The evil creature vanishes in a puff of sulfurous smoke!"));
            return TRUE;
        }

        msg_print(_("祈りは効果がなかった！", "Your invocation is ineffectual!"));
        if (one_in_(13))
            m_ptr->mflag2 |= MFLAG2_NOGENO;

        return TRUE;
    }
    case MUT1_COLD_TOUCH: {
        if (!get_direction(creature_ptr, &dir, FALSE, FALSE))
            return FALSE;

        POSITION y = creature_ptr->y + ddy[dir];
        POSITION x = creature_ptr->x + ddx[dir];
        grid_type *g_ptr;
        g_ptr = &creature_ptr->current_floor_ptr->grid_array[y][x];
        if (!g_ptr->m_idx) {
            msg_print(_("あなたは何もない場所で手を振った。", "You wave your hands in the air."));
            return TRUE;
        }

        fire_bolt(creature_ptr, GF_COLD, dir, 2 * lvl);
        return TRUE;
    }
    case 3:
        return do_cmd_throw(creature_ptr, 2 + lvl / 40, FALSE, -1);
    default:
        free_turn(creature_ptr);
        msg_format(_("能力 %s は実装されていません。", "Power %s not implemented. Oops."), power);
        return TRUE;
    }
}

void become_living_trump(player_type *creature_ptr)
{
    /* 1/7 Teleport control and 6/7 Random teleportation (uncontrolled) */
    MUTATION_IDX mutation = one_in_(7) ? 12 : 77;
    if (gain_mutation(creature_ptr, mutation))
        msg_print(_("あなたは生きているカードに変わった。", "You have turned into a Living Trump."));
}

void set_mutation_flags(player_type *creature_ptr)
{
    if (creature_ptr->muta3 == 0)
        return;

    if (creature_ptr->muta3 & MUT3_FLESH_ROT)
        creature_ptr->regenerate = FALSE;

    if (creature_ptr->muta3 & MUT3_ELEC_TOUC)
        creature_ptr->sh_elec = TRUE;

    if (creature_ptr->muta3 & MUT3_FIRE_BODY) {
        creature_ptr->sh_fire = TRUE;
        creature_ptr->lite = TRUE;
    }

    if (creature_ptr->muta3 & MUT3_WINGS)
        creature_ptr->levitation = TRUE;

    if (creature_ptr->muta3 & MUT3_FEARLESS)
        creature_ptr->resist_fear = TRUE;

    if (creature_ptr->muta3 & MUT3_REGEN)
        creature_ptr->regenerate = TRUE;

    if (creature_ptr->muta3 & MUT3_ESP)
        creature_ptr->telepathy = TRUE;

    if (creature_ptr->muta3 & MUT3_MOTION)
        creature_ptr->free_act = TRUE;
}
