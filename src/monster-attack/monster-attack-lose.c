#include "monster-attack/monster-attack-lose.h"
#include "mind/mind-mirror-master.h"
#include "monster-attack/monster-attack-status.h"
#include "monster-attack/monster-attack-util.h"
#include "player/player-damage.h"
#include "player/player-status-flags.h"
#include "player/player-status-resist.h"
#include "status/bad-status-setter.h"
#include "status/base-status.h"
#include "status/element-resistance.h"
#include "view/display-messages.h"

/*!
 * @brief �a�C�_���[�W���v�Z���� (�őϐ�������΁A(1d4 + 4) / 9�ɂȂ�B��d�ϐ��Ȃ�X��(1d4 + 4) / 9)
 * @param target_ptr �v���[���[�ւ̎Q�ƃ|�C���^
 * @param monap_ptr �����X�^�[����v���[���[�ւ̒��ڍU���\���̂ւ̎Q�ƃ|�C���^
 * @return �Ȃ�
 * @details 10% (�ł̈ꎟ�ϐ��������4%�A��d�ϐ��Ȃ��1.6%)�̊m���őϋv���ቺ���A�X��1/10�̊m���ŉi�v�ቺ����
 */
void calc_blow_disease(player_type *target_ptr, monap_type *monap_ptr)
{
    if (has_resist_pois(target_ptr))
        monap_ptr->damage = monap_ptr->damage * (randint1(4) + 4) / 9;

    if (is_oppose_pois(target_ptr))
        monap_ptr->damage = monap_ptr->damage * (randint1(4) + 4) / 9;

    monap_ptr->get_damage += take_hit(target_ptr, DAMAGE_ATTACK, monap_ptr->damage, monap_ptr->ddesc, -1);
    if (target_ptr->is_dead || check_multishadow(target_ptr))
        return;

    if (!(target_ptr->resist_pois || is_oppose_pois(target_ptr)) && set_poisoned(target_ptr, target_ptr->poisoned + randint1(monap_ptr->rlev) + 5))
        monap_ptr->obvious = TRUE;

    bool disease_possibility = randint1(100) > calc_nuke_damage_rate(target_ptr);
    if (disease_possibility || (randint1(100) > 10) || (target_ptr->prace == RACE_ANDROID))
        return;

    bool perm = one_in_(10);
    if (dec_stat(target_ptr, A_CON, randint1(10), perm)) {
        msg_print(_("�a�����Ȃ���I��ł���C������B", "You feel sickly."));
        monap_ptr->obvious = TRUE;
    }
}

/*!
 * @brief �r�͒ቺ�_���[�W���v�Z���� (�ێ�������΁A(1d4 + 4) / 9�ɂȂ�)
 * @param target_ptr �v���[���[�ւ̎Q�ƃ|�C���^
 * @param monap_ptr �����X�^�[����v���[���[�ւ̒��ڍU���\���̂ւ̎Q�ƃ|�C���^
 * @return �Ȃ�
 */
void calc_blow_lose_strength(player_type *target_ptr, monap_type *monap_ptr)
{
    if (has_sustain_str(target_ptr))
        monap_ptr->get_damage = monap_ptr->get_damage * (randint1(4) + 4) / 9;

    monap_ptr->get_damage += take_hit(target_ptr, DAMAGE_ATTACK, monap_ptr->damage, monap_ptr->ddesc, -1);
    if (target_ptr->is_dead || check_multishadow(target_ptr))
        return;

    if (do_dec_stat(target_ptr, A_STR))
        monap_ptr->obvious = TRUE;
}

/*!
 * @brief �m�\�ቺ�_���[�W���v�Z���� (�ێ�������΁A(1d4 + 4) / 9�ɂȂ�)
 * @param target_ptr �v���[���[�ւ̎Q�ƃ|�C���^
 * @param monap_ptr �����X�^�[����v���[���[�ւ̒��ڍU���\���̂ւ̎Q�ƃ|�C���^
 * @return �Ȃ�
 */
void calc_blow_lose_intelligence(player_type *target_ptr, monap_type *monap_ptr)
{
    if (has_sustain_int(target_ptr))
        monap_ptr->get_damage = monap_ptr->get_damage * (randint1(4) + 4) / 9;

    monap_ptr->get_damage += take_hit(target_ptr, DAMAGE_ATTACK, monap_ptr->damage, monap_ptr->ddesc, -1);
    if (target_ptr->is_dead || check_multishadow(target_ptr))
        return;

    if (do_dec_stat(target_ptr, A_INT))
        monap_ptr->obvious = TRUE;
}

/*!
 * @brief �����ቺ�_���[�W���v�Z���� (�ێ�������΁A(1d4 + 4) / 9�ɂȂ�)
 * @param target_ptr �v���[���[�ւ̎Q�ƃ|�C���^
 * @param monap_ptr �����X�^�[����v���[���[�ւ̒��ڍU���\���̂ւ̎Q�ƃ|�C���^
 * @return �Ȃ�
 */
void calc_blow_lose_wisdom(player_type *target_ptr, monap_type *monap_ptr)
{
    if (has_sustain_wis(target_ptr))
        monap_ptr->get_damage = monap_ptr->get_damage * (randint1(4) + 4) / 9;

    monap_ptr->get_damage += take_hit(target_ptr, DAMAGE_ATTACK, monap_ptr->damage, monap_ptr->ddesc, -1);
    if (target_ptr->is_dead || check_multishadow(target_ptr))
        return;

    if (do_dec_stat(target_ptr, A_WIS))
        monap_ptr->obvious = TRUE;
}

/*!
 * @brief ��p�ቺ�_���[�W���v�Z���� (�ێ�������΁A(1d4 + 4) / 9�ɂȂ�)
 * @param target_ptr �v���[���[�ւ̎Q�ƃ|�C���^
 * @param monap_ptr �����X�^�[����v���[���[�ւ̒��ڍU���\���̂ւ̎Q�ƃ|�C���^
 * @return �Ȃ�
 */
void calc_blow_lose_dexterity(player_type *target_ptr, monap_type *monap_ptr)
{
    if (has_sustain_dex(target_ptr))
        monap_ptr->get_damage = monap_ptr->get_damage * (randint1(4) + 4) / 9;

    monap_ptr->get_damage += take_hit(target_ptr, DAMAGE_ATTACK, monap_ptr->damage, monap_ptr->ddesc, -1);
    if (target_ptr->is_dead || check_multishadow(target_ptr))
        return;

    if (do_dec_stat(target_ptr, A_DEX))
        monap_ptr->obvious = TRUE;
}

/*!
 * @brief �ϋv�ቺ�_���[�W���v�Z���� (�ێ�������΁A(1d4 + 4) / 9�ɂȂ�)
 * @param target_ptr �v���[���[�ւ̎Q�ƃ|�C���^
 * @param monap_ptr �����X�^�[����v���[���[�ւ̒��ڍU���\���̂ւ̎Q�ƃ|�C���^
 * @return �Ȃ�
 */
void calc_blow_lose_constitution(player_type *target_ptr, monap_type *monap_ptr)
{
    if (has_sustain_con(target_ptr))
        monap_ptr->get_damage = monap_ptr->get_damage * (randint1(4) + 4) / 9;

    monap_ptr->get_damage += take_hit(target_ptr, DAMAGE_ATTACK, monap_ptr->damage, monap_ptr->ddesc, -1);
    if (target_ptr->is_dead || check_multishadow(target_ptr))
        return;

    if (do_dec_stat(target_ptr, A_CON))
        monap_ptr->obvious = TRUE;
}

/*!
 * @brief ���͒ቺ�_���[�W���v�Z���� (�ێ�������΁A(1d4 + 4) / 9�ɂȂ�)
 * @param target_ptr �v���[���[�ւ̎Q�ƃ|�C���^
 * @param monap_ptr �����X�^�[����v���[���[�ւ̒��ڍU���\���̂ւ̎Q�ƃ|�C���^
 * @return �Ȃ�
 */
void calc_blow_lose_charisma(player_type *target_ptr, monap_type *monap_ptr)
{
    if (has_sustain_chr(target_ptr))
        monap_ptr->get_damage = monap_ptr->get_damage * (randint1(4) + 4) / 9;

    monap_ptr->get_damage += take_hit(target_ptr, DAMAGE_ATTACK, monap_ptr->damage, monap_ptr->ddesc, -1);
    if (target_ptr->is_dead || check_multishadow(target_ptr))
        return;

    if (do_dec_stat(target_ptr, A_CHR))
        monap_ptr->obvious = TRUE;
}

/*!
 * @brief �S�\�͒ቺ�_���[�W���v�Z���� (�ێ�������΁A1�ɕt��-3%�y������)
 * @param target_ptr �v���[���[�ւ̎Q�ƃ|�C���^
 * @param monap_ptr �����X�^�[����v���[���[�ւ̒��ڍU���\���̂ւ̎Q�ƃ|�C���^
 * @return �Ȃ�
 */
void calc_blow_lose_all(player_type *target_ptr, monap_type *monap_ptr)
{
    int damage_ratio = 100;
    if (has_sustain_str(target_ptr))
        damage_ratio -= 3;

    if (has_sustain_int(target_ptr))
        damage_ratio -= 3;

    if (has_sustain_wis(target_ptr))
        damage_ratio -= 3;

    if (has_sustain_dex(target_ptr))
        damage_ratio -= 3;

    if (has_sustain_con(target_ptr))
        damage_ratio -= 3;

    if (has_sustain_chr(target_ptr))
        damage_ratio -= 3;

    monap_ptr->damage = monap_ptr->damage * damage_ratio / 100;
    monap_ptr->get_damage += take_hit(target_ptr, DAMAGE_ATTACK, monap_ptr->damage, monap_ptr->ddesc, -1);
    if (target_ptr->is_dead || check_multishadow(target_ptr))
        return;

    process_lose_all_attack(target_ptr, monap_ptr);
}
