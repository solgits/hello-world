// BitOperator.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include <stdio.h>

int GU = 99;

int _tmain(int argc, _TCHAR* argv[])
{
	int hi = 4;

	// GU = (hi << 16) | GU;
	short *s2 = (short*)&GU;

	printf("GU(%d/0x%08X)->[%d][%d] \n", GU, GU, s2[0], s2[1]);

	s2[1] = 4;

	printf("GU(%d/0x%08X)->[%d][%d] \n", GU, GU, s2[0], s2[1]);

	return 0;
}

