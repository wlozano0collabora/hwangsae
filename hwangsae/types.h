/**
 *  Copyright 2019 SK Telecom Co., Ltd.
 *    Author: Jeongseok Kim <jeongseok.kim@sk.com>
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
 *
 */

#ifndef __HWANGSAE_TYPES_H__
#define __HWANGSAE_TYPES_H__

#ifndef _HWANGSAE_EXTERN
#define _HWANGSAE_EXTERN        extern
#endif

#define HWANGSAE_API_EXPORT     _HWANGSAE_EXTERN

typedef enum {
  HWANGSAE_RETURN_FAIL = -1,
  HWANGSAE_RETURN_OK,
} HwangsaeReturn;

typedef enum {
  HWANGSAE_CONTAINER_MP4,
  HWANGSAE_CONTAINER_TS,
} HwangsaeContainer;

#endif // __HWANGSAE_TYPES_H__
