/*******************************************************************************
*
*  (C) COPYRIGHT AUTHORS, 2020
*
*  TITLE:       GLOBAL.H
*
*  VERSION:     1.01
*
*  DATE:        18 Feb 2020
*
*  Common include header file.
*
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
* ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
* TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
*******************************************************************************/

#pragma once

#if !defined UNICODE
#error ANSI build is not supported
#endif

//
// Ignored warnings
//
#pragma warning(disable: 4005) // macro redefinition
#pragma warning(disable: 4091) // 'typedef ': ignored on left of '%s' when no variable is declared
#pragma warning(disable: 4201) // nameless struct/union
#pragma warning(disable: 26812) // Prefer 'enum class' over 'enum'

#include <Windows.h>
#include <strsafe.h>
#include <ntstatus.h>
#include <intrin.h>
#include "ntos/ntos.h"
#include "ntos/halamd64.h"
#include "drv/ATSZIO64.h"
#include "drv/eneio64.h"
#include "drv/enetechio64.h"
#include "drv/gdrv.h"
#include "drv/glckio2.h"
#include "drv/iQVM64.h"
#include "drv/msio64.h"
#include "drv/procexp.h"
#include "drv/rtcore64.h"
#include "drv/WinRing0x64.h"

#if defined(__cplusplus)
extern "C" {
#endif

#include "hde/hde64.h"
#include "minirtl/minirtl.h"
#include "minirtl/rtltypes.h"
#include "minirtl/cmdline.h"
#include "minirtl/_filename.h"

#ifdef __cplusplus
}
#endif

#include "consts.h"
#include "sup.h"
#include "compress.h"
#include "kduprov.h"
#include "drvmap.h"
#include "ps.h"
#include "victim.h"
#include "pagewalk.h"
#include "dsefix.h"