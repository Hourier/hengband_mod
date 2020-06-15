﻿#pragma once

#include "system/angband.h"
#include "info-reader/info-reader-util.h"
#include "info-reader/parse-error-types.h"

extern concptr err_str[PARSE_ERROR_MAX];

errr parse_fixed_map(player_type *player_ptr, concptr name, int ymin, int xmin, int ymax, int xmax);
