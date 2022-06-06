#pragma once
/* Expert system based on the LP-structures.
 *
 * Copyright (C) 2012, Artem Shmarin, S. Makhortov.
 * Written by Artem Shmarin (tim-shr@mail.ru)
 *
 * This file is part of LPExpert 2.0.
 *
 * LPExpert 2.0 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * LPExpert 2.0 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with LPExpert 2.0.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef COMPILER_GLOBAL_H
#define COMPILER_GLOBAL_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


#include "lex_location.h"
#include "parser_union.h"

#define YYLTYPE cl::location
#define YYSTYPE cl::semantic_type

namespace flpx
{


}



#endif // COMPILER_GLOBAL_H
