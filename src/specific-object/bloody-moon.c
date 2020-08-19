#include "specific-object/bloody-moon.h"
#include "art-definition/art-weapon-types.h"
#include "core/player-update-types.h"
#include "object-enchant/object-boost.h"
#include "object-enchant/tr-types.h"
#include "racial/racial-android.h"
#include "system/artifact-type-definition.h"
#include "system/object-type-definition.h"
#include "util/bit-flags-calculator.h"
#include "view/display-messages.h"

/*!
 * @brief �Œ�A�[�e�B�t�@�N�g�w�u���b�f�B���[���x�̓�����ύX����B
 * @details �X���C2d2��A�y��one_resistance()�ɂ��ϐ�1d2��Apval2��𓾂�B
 * @param o_ptr �Ώۂ̃I�u�W�F�N�g�\���� (�u���b�f�B���[��)�̃|�C���^
 * @return �Ȃ�
 */
void get_bloody_moon_flags(object_type *o_ptr)
{
    for (int i = 0; i < TR_FLAG_SIZE; i++)
        o_ptr->art_flags[i] = a_info[ART_BLOOD].flags[i];

    int dummy = randint1(2) + randint1(2);
    for (int i = 0; i < dummy; i++) {
        int flag = randint0(26);
        if (flag >= 20)
            add_flag(o_ptr->art_flags, TR_KILL_UNDEAD + flag - 20);
        else if (flag == 19)
            add_flag(o_ptr->art_flags, TR_KILL_ANIMAL);
        else if (flag == 18)
            add_flag(o_ptr->art_flags, TR_SLAY_HUMAN);
        else
            add_flag(o_ptr->art_flags, TR_CHAOTIC + flag);
    }

    dummy = randint1(2);
    for (int i = 0; i < dummy; i++)
        one_resistance(o_ptr);

    for (int i = 0; i < 2; i++) {
        int tmp = randint0(11);
        if (tmp < A_MAX)
            add_flag(o_ptr->art_flags, TR_STR + tmp);
        else
            add_flag(o_ptr->art_flags, TR_STEALTH + tmp - 6);
    }
}

/*!
 * @brief Let's dance a RONDO!!
 * @param user_ptr �v���[���[�ւ̎Q�ƃ|�C���^
 * @param o_ptr �u���b�f�B�E���[���ւ̎Q�ƃ|�C���^
 * @return �I�u�W�F�N�g���Ɉُ킪�Ȃ�����TRUE
 */
bool activate_bloody_moon(player_type *user_ptr, object_type *o_ptr)
{
    if (o_ptr->name1 != ART_BLOOD)
        return FALSE;

    msg_print(_("�������邭�P����...", "Your scythe glows brightly!"));
    get_bloody_moon_flags(o_ptr);
    if (user_ptr->prace == RACE_ANDROID)
        calc_android_exp(user_ptr);

    user_ptr->update |= PU_BONUS | PU_HP;
    return TRUE;
}
