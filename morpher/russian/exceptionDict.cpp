﻿// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <stdlib.h>

#include "exceptionDict.h"

Exception exceptionDict [] =
{
	{
		{
			_T("Великие Луки"),
			_T("Великих Лук"),
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
		},
	},
	{
		{
			_T("Проектная документация без сметы и результаты инженерных изысканий"),
			_T("Проектной документации без сметы и результатов инженерных изысканий"),
			_T("Проектной документации без сметы и результатам инженерных изысканий"),
			_T("Проектную документацию без сметы и результаты инженерных изысканий"),
			_T("Проектной документацией без сметы и результатами инженерных изысканий"),
			_T("Проектной документации без сметы и результатах инженерных изысканий"),
			_T("в Проектной документации без сметы и результатах инженерных изысканий"),
		},
	},
};

size_t exceptionDict_size = sizeof(exceptionDict) / sizeof(*exceptionDict);

