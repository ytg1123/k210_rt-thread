/* Copyright 2018 Canaan Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef _FONT_H_
#define _FONT_H_

#include <stdint.h>

/* Font Selector */
#define _IPA_GOTHIC_8X16_ 1
#define _IPA_MINCHO_8X16_ 0
#define _DEF_8x16_ASCII_  0


/* IPA GOTHIC ascii bitmap font */
#if _IPA_GOTHIC_8X16_ == 1
  #include "font_ipa_8x16_gothic.h"
/* IPA MINCHO ascii bitmap font */
#elif _IPA_MINCHO_8X16_ == 1
  #include "font_ipa_8x16_mincho.h"
/* default rt-thread ascii bitmap font */
#elif _DEF_8x16_ASCII_ == 1
  #include "font_def_8x16.h"
#else 
  #include "font_def_8x16.h"
#endif

/* default rt-thread ascii bitmap font */

#endif

