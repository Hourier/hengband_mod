﻿#pragma once

#include "system/angband.h"
#include "floor/floor.h"

bool make_object(player_type *owner_ptr, object_type *j_ptr, BIT_FLAGS mode);
bool make_gold(floor_type *floor_ptr, object_type *j_ptr);
void delete_all_items_from_floor(player_type *owner_ptr, POSITION y, POSITION x);
void floor_item_increase(floor_type *floor_ptr, INVENTORY_IDX item, ITEM_NUMBER num);
void floor_item_optimize(player_type *owner_ptr, INVENTORY_IDX item);
void delete_object_idx(player_type *owner_ptr, OBJECT_IDX o_idx);
void excise_object_idx(floor_type *floor_ptr, OBJECT_IDX o_idx);
OBJECT_IDX drop_near(player_type *owner_type, object_type *o_ptr, PERCENTAGE chance, POSITION y, POSITION x);
void floor_item_charges(floor_type *owner_ptr, INVENTORY_IDX item);
void floor_item_describe(player_type *player_ptr, INVENTORY_IDX item);
