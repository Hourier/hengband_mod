﻿#pragma once

#include "system/angband.h"

typedef struct player_type player_type;
bool monst_attack_monst(player_type *subject_ptr, MONSTER_IDX m_idx, MONSTER_IDX t_idx);
