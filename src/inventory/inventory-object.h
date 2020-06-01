﻿#pragma once

#include "system/angband.h"

void vary_item(player_type *owner_ptr, INVENTORY_IDX item, ITEM_NUMBER num);
void inven_item_increase(player_type *owner_ptr, INVENTORY_IDX item, ITEM_NUMBER num);
void inven_item_optimize(player_type *owner_ptr, INVENTORY_IDX item);
void drop_from_inventory(player_type *owner_type, INVENTORY_IDX item, ITEM_NUMBER amt);
void combine_pack(player_type *owner_ptr);
void reorder_pack(player_type *owner_ptr);
