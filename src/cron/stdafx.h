// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#include <stdio.h>
#include <tchar.h>


#define NOMINMAX


// TODO:  在此处引用程序需要的其他头文件
#include "exarray.h"
typedef ExArray<int> IntArray;

#include "singleton.h"

#include "stdstring.h"
typedef CStdStringA mstring;
typedef std::vector<mstring> StringArray;

#ifndef _HasFlag
#	define _HasFlag(l,f) ((l) & (f))
#endif

#ifndef _AddFlag
#	define _AddFlag(l,f) ((l) |= (f))
#endif

#ifndef _RemoveFlag
#	define _RemoveFlag(l,f) ((l) &= ~(f))
#endif

#include "time.h"

