#include "target/projection-path-calculator.h"
#include "effect/effect-characteristics.h"
#include "effect/spells-effect-util.h"
#include "floor/cave.h"
#include "grid/feature-flag-types.h"
#include "grid/grid.h"
#include "system/floor-type-definition.h"

typedef struct projection_path_type {
    u16b *gp;
    POSITION range;
    BIT_FLAGS flag;
    POSITION y1;
    POSITION x1;
    POSITION y2;
    POSITION x2;
    POSITION y;
    POSITION x;
    POSITION ay;
    POSITION ax;
    POSITION sy;
    POSITION sx;
    int frac;
    int m;
    int half;
    int full;
    int n;
    int k;
} projection_path_type;

/*
 * @brief Convert a "location" (Y, X) into a "grid" (G)
 * @param y Y���W
 * @param x X���W
 * return �o�H���W
 */
static u16b location_to_grid(POSITION y, POSITION x) { return 256 * y + x; }

static projection_path_type *initialize_projection_path_type(
    projection_path_type *pp_ptr, u16b *gp, POSITION range, BIT_FLAGS flag, POSITION y1, POSITION x1, POSITION y2, POSITION x2)
{
    pp_ptr->gp = gp;
    pp_ptr->range = range;
    pp_ptr->flag = flag;
    pp_ptr->y1 = y1;
    pp_ptr->x1 = x1;
    pp_ptr->y2 = y2;
    pp_ptr->x2 = x2;
    return pp_ptr;
}

static void set_asxy(projection_path_type *pp_ptr)
{
    if (pp_ptr->y2 < pp_ptr->y1) {
        pp_ptr->ay = pp_ptr->y1 - pp_ptr->y2;
        pp_ptr->sy = -1;
    } else {
        pp_ptr->ay = pp_ptr->y2 - pp_ptr->y1;
        pp_ptr->sy = 1;
    }

    if (pp_ptr->x2 < pp_ptr->x1) {
        pp_ptr->ax = pp_ptr->x1 - pp_ptr->x2;
        pp_ptr->sx = -1;
    } else {
        pp_ptr->ax = pp_ptr->x2 - pp_ptr->x1;
        pp_ptr->sx = 1;
    }
}

static void calc_frac(projection_path_type *pp_ptr, bool is_vertical)
{
    if (pp_ptr->m == 0)
        return;

    pp_ptr->frac += pp_ptr->m;
    if (pp_ptr->frac <= pp_ptr->half)
        return;

    if (is_vertical)
        pp_ptr->x += pp_ptr->sx;
    else
        pp_ptr->y += pp_ptr->sy;

    pp_ptr->frac -= pp_ptr->full;
    pp_ptr->k++;
}

static void calc_projection_to_target(player_type *player_ptr, projection_path_type *pp_ptr, bool is_vertical)
{
    floor_type *floor_ptr = player_ptr->current_floor_ptr;
    while (TRUE) {
        pp_ptr->gp[pp_ptr->n++] = location_to_grid(pp_ptr->y, pp_ptr->x);
        if ((pp_ptr->n + (pp_ptr->k >> 1)) >= pp_ptr->range)
            break;

        if (!(pp_ptr->flag & PROJECT_THRU)) {
            if ((pp_ptr->x == pp_ptr->x2) && (pp_ptr->y == pp_ptr->y2))
                break;
        }

        if (pp_ptr->flag & PROJECT_DISI) {
            if ((pp_ptr->n > 0) && cave_stop_disintegration(floor_ptr, pp_ptr->y, pp_ptr->x))
                break;
        } else if (pp_ptr->flag & PROJECT_LOS) {
            if ((pp_ptr->n > 0) && !cave_los_bold(floor_ptr, pp_ptr->y, pp_ptr->x))
                break;
        } else if (!(pp_ptr->flag & PROJECT_PATH)) {
            if ((pp_ptr->n > 0) && !cave_has_flag_bold(floor_ptr, pp_ptr->y, pp_ptr->x, FF_PROJECT))
                break;
        }

        if (pp_ptr->flag & PROJECT_STOP) {
            if ((pp_ptr->n > 0) && (player_bold(player_ptr, pp_ptr->y, pp_ptr->x) || floor_ptr->grid_array[pp_ptr->y][pp_ptr->x].m_idx != 0))
                break;
        }

        if (!in_bounds(floor_ptr, pp_ptr->y, pp_ptr->x))
            break;

        calc_frac(pp_ptr, is_vertical);
        if (is_vertical)
            pp_ptr->y += pp_ptr->sy;
        else
            pp_ptr->x += pp_ptr->sx;
    }
}

static bool calc_vertical_projection(player_type *player_ptr, projection_path_type *pp_ptr)
{
    if (pp_ptr->ay <= pp_ptr->ax)
        return FALSE;

    pp_ptr->m = pp_ptr->ax * pp_ptr->ax * 2;
    pp_ptr->y = pp_ptr->y1 + pp_ptr->sy;
    pp_ptr->x = pp_ptr->x1;
    pp_ptr->frac = pp_ptr->m;
    if (pp_ptr->frac > pp_ptr->half) {
        pp_ptr->x += pp_ptr->sx;
        pp_ptr->frac -= pp_ptr->full;
        pp_ptr->k++;
    }

    calc_projection_to_target(player_ptr, pp_ptr, TRUE);
    return TRUE;
}

static bool calc_horizontal_projection(player_type *player_ptr, projection_path_type *pp_ptr)
{
    if (pp_ptr->ax <= pp_ptr->ay)
        return FALSE;

    pp_ptr->m = pp_ptr->ay * pp_ptr->ay * 2;
    pp_ptr->y = pp_ptr->y1;
    pp_ptr->x = pp_ptr->x1 + pp_ptr->sx;
    pp_ptr->frac = pp_ptr->m;
    if (pp_ptr->frac > pp_ptr->half) {
        pp_ptr->y += pp_ptr->sy;
        pp_ptr->frac -= pp_ptr->full;
        pp_ptr->k++;
    }

    calc_projection_to_target(player_ptr, pp_ptr, FALSE);
    return TRUE;
}

static void calc_projection_others(player_type *player_ptr, projection_path_type *pp_ptr)
{
    floor_type *floor_ptr = player_ptr->current_floor_ptr;
    while (TRUE) {
        pp_ptr->gp[pp_ptr->n++] = location_to_grid(pp_ptr->y, pp_ptr->x);
        if ((pp_ptr->n + (pp_ptr->n >> 1)) >= pp_ptr->range)
            break;

        if (((pp_ptr->flag & PROJECT_THRU) == 0) && (pp_ptr->x == pp_ptr->x2) && (pp_ptr->y == pp_ptr->y2))
            break;

        if (pp_ptr->flag & PROJECT_DISI) {
            if ((pp_ptr->n > 0) && cave_stop_disintegration(floor_ptr, pp_ptr->y, pp_ptr->x))
                break;
        } else if (pp_ptr->flag & PROJECT_LOS) {
            if ((pp_ptr->n > 0) && !cave_los_bold(floor_ptr, pp_ptr->y, pp_ptr->x))
                break;
        } else if (!(pp_ptr->flag & PROJECT_PATH)) {
            if ((pp_ptr->n > 0) && !cave_has_flag_bold(floor_ptr, pp_ptr->y, pp_ptr->x, FF_PROJECT))
                break;
        }

        if (((pp_ptr->flag & PROJECT_STOP) != 0) && (pp_ptr->n > 0)
            && (player_bold(player_ptr, pp_ptr->y, pp_ptr->x) || floor_ptr->grid_array[pp_ptr->y][pp_ptr->x].m_idx != 0))
            break;

        if (!in_bounds(floor_ptr, pp_ptr->y, pp_ptr->x))
            break;

        pp_ptr->y += pp_ptr->sy;
        pp_ptr->x += pp_ptr->sx;
    }
}

/*!
 * @brief �n�_����I�_�ւ̒����o�H��Ԃ� /
 * Determine the path taken by a projection.
 * @param player_ptr �v���[���[�ւ̎Q�ƃ|�C���^
 * @param gp �o�H���W���X�g��Ԃ��Q�ƃ|�C���^
 * @param range ����
 * @param y1 �n�_Y���W
 * @param x1 �n�_X���W
 * @param y2 �I�_Y���W
 * @param x2 �I�_X���W
 * @param flag �t���OID
 * @return ���X�g�̒���
 */
int projection_path(player_type *player_ptr, u16b *gp, POSITION range, POSITION y1, POSITION x1, POSITION y2, POSITION x2, BIT_FLAGS flag)
{
    if ((x1 == x2) && (y1 == y2))
        return 0;

    projection_path_type tmp_projection_path;
    projection_path_type *pp_ptr = initialize_projection_path_type(&tmp_projection_path, gp, range, flag, y1, x1, y2, x2);
    set_asxy(pp_ptr);
    pp_ptr->half = pp_ptr->ay * pp_ptr->ax;
    pp_ptr->full = pp_ptr->half << 1;
    pp_ptr->n = 0;
    pp_ptr->k = 0;

    if (calc_vertical_projection(player_ptr, pp_ptr))
        return pp_ptr->n;

    if (calc_horizontal_projection(player_ptr, pp_ptr))
        return pp_ptr->n;

    pp_ptr->y = y1 + pp_ptr->sy;
    pp_ptr->x = x1 + pp_ptr->sx;
    calc_projection_others(player_ptr, pp_ptr);
    return pp_ptr->n;
}

/*
 * Determine if a bolt spell cast from (y1,x1) to (y2,x2) will arrive
 * at the final destination, assuming no monster gets in the way.
 *
 * This is slightly (but significantly) different from "los(y1,x1,y2,x2)".
 */
bool projectable(player_type *player_ptr, POSITION y1, POSITION x1, POSITION y2, POSITION x2)
{
    u16b grid_g[512];
    int grid_n = projection_path(player_ptr, grid_g, (project_length ? project_length : get_max_range(player_ptr)), y1, x1, y2, x2, 0);
    if (!grid_n)
        return TRUE;

    POSITION y = get_grid_y(grid_g[grid_n - 1]);
    POSITION x = get_grid_x(grid_g[grid_n - 1]);
    if ((y != y2) || (x != x2))
        return FALSE;

    return TRUE;
}

/*!
 * @brief�v���C���[�̍U���˒�(�}�X) / Maximum range (spells, etc)
 * @param creature_ptr �v���[���[�ւ̎Q�ƃ|�C���^
 * @return �˒�
 */
int get_max_range(player_type *creature_ptr) { return creature_ptr->phase_out ? 36 : 18; }

/*
 * Convert a "grid" (G) into a "location" (Y)
 */
POSITION get_grid_y(u16b grid) { return (int)(grid / 256U); }

/*
 * Convert a "grid" (G) into a "location" (X)
 */
POSITION get_grid_x(u16b grid) { return (int)(grid % 256U); }
