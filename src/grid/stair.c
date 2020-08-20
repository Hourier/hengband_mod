#include "grid/stair.h"
#include "dungeon/dungeon.h"
#include "dungeon/quest.h"
#include "game-option/birth-options.h"
#include "floor/cave.h"
#include "grid/grid.h"
#include "object-hook/hook-enchant.h"
#include "system/floor-type-definition.h"
#include "system/object-type-definition.h"

/*!
 * @brief ����̈ʒu�ɏ��K�i������K�i��z�u���� / Place an up/down staircase at given location
 * @param player_ptr �v���[���[�ւ̎Q�ƃ|�C���^
 * @param y �z�u�����݂����}�X��Y���W
 * @param x �z�u�����݂����}�X��X���W
 * @return �Ȃ�
 */
void place_random_stairs(player_type *player_ptr, POSITION y, POSITION x)
{
    bool up_stairs = TRUE;
    bool down_stairs = TRUE;
    grid_type *g_ptr;
    floor_type *floor_ptr = player_ptr->current_floor_ptr;
    g_ptr = &floor_ptr->grid_array[y][x];
    if (!is_floor_grid(g_ptr) || g_ptr->o_idx)
        return;

    if (!floor_ptr->dun_level)
        up_stairs = FALSE;

    if (ironman_downward)
        up_stairs = FALSE;

    if (floor_ptr->dun_level >= d_info[player_ptr->dungeon_idx].maxdepth)
        down_stairs = FALSE;

    if (quest_number(player_ptr, floor_ptr->dun_level) && (floor_ptr->dun_level > 1))
        down_stairs = FALSE;

    if (down_stairs && up_stairs) {
        if (randint0(100) < 50)
            up_stairs = FALSE;
        else
            down_stairs = FALSE;
    }

    if (up_stairs)
        set_cave_feat(floor_ptr, y, x, feat_up_stair);
    else if (down_stairs)
        set_cave_feat(floor_ptr, y, x, feat_down_stair);
}

/*!
 * @brief �w�肳�ꂽ���W���n�k��K�i�����̑ΏۂƂȂ�}�X����Ԃ��B / Determine if a given location may be "destroyed"
 * @param player_ptr �v���[���[�ւ̎Q�ƃ|�C���^
 * @param y y���W
 * @param x x���W
 * @return �e��̕ύX���\�Ȃ�TRUE��Ԃ��B
 * @details
 * �����͉i�v�n�`�łȂ��A�Ȃ����Y���̃}�X�ɃA�[�e�B�t�@�N�g�����݂��Ȃ����A�ł���B�p��̋��R�����g�ɔ����ā��j�󁖂̗}�~����ɂ͌��ݎg���Ă��Ȃ��B
 */
bool cave_valid_bold(floor_type *floor_ptr, POSITION y, POSITION x)
{
    grid_type *g_ptr = &floor_ptr->grid_array[y][x];
    if (cave_have_flag_grid(g_ptr, FF_PERMANENT))
        return FALSE;

    OBJECT_IDX next_o_idx = 0;
    for (OBJECT_IDX this_o_idx = g_ptr->o_idx; this_o_idx; this_o_idx = next_o_idx) {
        object_type *o_ptr;
        o_ptr = &floor_ptr->o_list[this_o_idx];
        next_o_idx = o_ptr->next_o_idx;
        if (object_is_artifact(o_ptr))
            return FALSE;
    }

    return TRUE;
}
