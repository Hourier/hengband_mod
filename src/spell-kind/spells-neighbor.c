#include "spell-kind/spells-neighbor.h"
#include "core/player-redraw-types.h"
#include "core/player-update-types.h"
#include "effect/effect-characteristics.h"
#include "effect/effect-processor.h"
#include "floor/cave.h"
#include "floor/floor-util.h"
#include "grid/feature-flag-types.h"
#include "grid/grid.h"
#include "spell-kind/earthquake.h"
#include "spell/spell-types.h"
#include "util/bit-flags-calculator.h"

/*!
 * @brief �h�A��������(�v���C���[���S�Ɏ���1�}�X) / Hooks -- affect adjacent grids (radius 1 ball attack)
 * @param caster_ptr �v���[���[�ւ̎Q�ƃ|�C���^
 * @return ��p�����ۂɂ������ꍇTRUE��Ԃ�
 */
bool door_creation(player_type *caster_ptr, POSITION y, POSITION x)
{
    BIT_FLAGS flg = PROJECT_GRID | PROJECT_ITEM | PROJECT_HIDE;
    return (project(caster_ptr, 0, 1, y, x, 0, GF_MAKE_DOOR, flg, -1));
}

/*!
 * @brief �g���b�v��������(�N�_�������1�}�X)
 * @param caster_ptr �v���[���[�ւ̎Q�ƃ|�C���^
 * @param y �N�_Y���W
 * @param x �N�_X���W
 * @return ��p�����ۂɂ������ꍇTRUE��Ԃ�
 */
bool trap_creation(player_type *caster_ptr, POSITION y, POSITION x)
{
    BIT_FLAGS flg = PROJECT_GRID | PROJECT_ITEM | PROJECT_HIDE;
    return (project(caster_ptr, 0, 1, y, x, 0, GF_MAKE_TRAP, flg, -1));
}

/*!
 * @brief �X�ѐ�������(�v���C���[���S�Ɏ���1�}�X)
 * @param caster_ptr �v���[���[�ւ̎Q�ƃ|�C���^
 * @return ��p�����ۂɂ������ꍇTRUE��Ԃ�
 */
bool tree_creation(player_type *caster_ptr, POSITION y, POSITION x)
{
    BIT_FLAGS flg = PROJECT_GRID | PROJECT_ITEM | PROJECT_HIDE;
    return (project(caster_ptr, 0, 1, y, x, 0, GF_MAKE_TREE, flg, -1));
}

/*!
 * @brief ���@�̃��[����������(�v���C���[���S�Ɏ���1�}�X)
 * @param caster_ptr �v���[���[�ւ̎Q�ƃ|�C���^
 * @return ��p�����ۂɂ������ꍇTRUE��Ԃ�
 */
bool glyph_creation(player_type *caster_ptr, POSITION y, POSITION x)
{
    BIT_FLAGS flg = PROJECT_GRID | PROJECT_ITEM;
    return (project(caster_ptr, 0, 1, y, x, 0, GF_MAKE_GLYPH, flg, -1));
}

/*!
 * @brief �ǐ�������(�v���C���[���S�Ɏ���1�}�X)
 * @param caster_ptr �v���[���[�ւ̎Q�ƃ|�C���^
 * @return ��p�����ۂɂ������ꍇTRUE��Ԃ�
 */
bool wall_stone(player_type *caster_ptr)
{
    BIT_FLAGS flg = PROJECT_GRID | PROJECT_ITEM | PROJECT_HIDE;
    bool dummy = (project(caster_ptr, 0, 1, caster_ptr->y, caster_ptr->x, 0, GF_STONE_WALL, flg, -1));
    caster_ptr->update |= (PU_FLOW);
    caster_ptr->redraw |= (PR_MAP);
    return dummy;
}

/*!
 * @brief �h�A�j�󏈗�(�v���C���[���S�Ɏ���1�}�X)
 * @param caster_ptr �v���[���[�ւ̎Q�ƃ|�C���^
 * @return ��p�����ۂɂ������ꍇTRUE��Ԃ�
 */
bool destroy_doors_touch(player_type *caster_ptr)
{
    BIT_FLAGS flg = PROJECT_GRID | PROJECT_ITEM | PROJECT_HIDE;
    return (project(caster_ptr, 0, 1, caster_ptr->y, caster_ptr->x, 0, GF_KILL_DOOR, flg, -1));
}

/*!
 * @brief �g���b�v��������(�v���C���[���S�Ɏ���1�}�X)
 * @param caster_ptr �v���[���[�ւ̎Q�ƃ|�C���^
 * @return ��p�����ۂɂ������ꍇTRUE��Ԃ�
 */
bool disarm_traps_touch(player_type *caster_ptr)
{
    BIT_FLAGS flg = PROJECT_GRID | PROJECT_ITEM | PROJECT_HIDE;
    return (project(caster_ptr, 0, 1, caster_ptr->y, caster_ptr->x, 0, GF_KILL_TRAP, flg, -1));
}

/*!
 * @brief �X���[�v�����X�^�[����(�v���C���[���S�Ɏ���1�}�X)
 * @param caster_ptr �v���[���[�ւ̎Q�ƃ|�C���^
 * @return ��p�����ۂɂ������ꍇTRUE��Ԃ�
 */
bool sleep_monsters_touch(player_type *caster_ptr)
{
    BIT_FLAGS flg = PROJECT_KILL | PROJECT_HIDE;
    return (project(caster_ptr, 0, 1, caster_ptr->y, caster_ptr->x, caster_ptr->lev, GF_OLD_SLEEP, flg, -1));
}

/*!
 * @brief ���ҕ�������(�N�_������5�}�X)
 * @param caster_ptr �v���[���[�ւ̎Q�ƃ|�C���^
 * @param who �p�҃����X�^�[ID(0�Ȃ�΃v���C���[)
 * @param y �N�_Y���W
 * @param x �N�_X���W
 * @return ��p�����ۂɂ������ꍇTRUE��Ԃ�
 */
bool animate_dead(player_type *caster_ptr, MONSTER_IDX who, POSITION y, POSITION x)
{
    BIT_FLAGS flg = PROJECT_ITEM | PROJECT_HIDE;
    return (project(caster_ptr, who, 5, y, x, 0, GF_ANIM_DEAD, flg, -1));
}

/*!
 * @brief ���Ӕj�����(�v���C���[���S)
 * @param caster_ptr �v���[���[�ւ̎Q�ƃ|�C���^
 * @return ��p�����ۂɂ������ꍇTRUE��Ԃ�
 */
void wall_breaker(player_type *caster_ptr)
{
    POSITION y = 0, x = 0;
    int attempts = 1000;
    if (randint1(80 + caster_ptr->lev) < 70) {
        while (attempts--) {
            scatter(caster_ptr, &y, &x, caster_ptr->y, caster_ptr->x, 4, 0);

            if (!cave_have_flag_bold(caster_ptr->current_floor_ptr, y, x, FF_PROJECT))
                continue;

            if (!player_bold(caster_ptr, y, x))
                break;
        }

        project(caster_ptr, 0, 0, y, x, 20 + randint1(30), GF_KILL_WALL, (PROJECT_BEAM | PROJECT_THRU | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL), -1);
        return;
    }

    if (randint1(100) > 30) {
        earthquake(caster_ptr, caster_ptr->y, caster_ptr->x, 1, 0);
        return;
    }

    int num = damroll(5, 3);
    for (int i = 0; i < num; i++) {
        while (TRUE) {
            scatter(caster_ptr, &y, &x, caster_ptr->y, caster_ptr->x, 10, 0);

            if (!player_bold(caster_ptr, y, x))
                break;
        }

        project(caster_ptr, 0, 0, y, x, 20 + randint1(30), GF_KILL_WALL, (PROJECT_BEAM | PROJECT_THRU | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL), -1);
    }
}
