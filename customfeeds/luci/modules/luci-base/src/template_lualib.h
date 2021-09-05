/*
 * LuCI Template - Lua library header
 *
 *   Copyright (C) 2009 Jo-Philipp Wich <jow@openwrt.org>
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef _TEMPLATE_LUALIB_H_
#define _TEMPLATE_LUALIB_H_

#include "template_parser.h"
#include "template_utils.h"
#include "template_lmo.h"

#define TEMPLATE_LUALIB_META  "template.parser"

LUALIB_API int luaopen_luci_template_parser(lua_State *L);

#endif
