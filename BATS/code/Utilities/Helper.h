/**
 * @file
 * @version 0.1
 * @section COPYRIGHT Copyright © Matteus Magnusson
 * @author Matteus Magnusson <senth.wallace@gmail.com>
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 */

#pragma once

#include <cassert>

#define SAFE_DELETE(pointer) \
	delete pointer; \
	pointer = NULL

#define SAFE_DELETE_ARR(pointer) \
	delete [] pointer; \
	pointer = NULL;