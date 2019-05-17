//
// ClassMethod.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include <stdio.h>

class B1
{
public:
	B1() 
	{
		_x = -1;
		printf("B1() : x=%d\n", _x);
	}

	B1(int x) 
	{
		_x = x;
		printf("B1(%d) : x=%d\n", x, _x);
	}

	void set(int x)
	{
		_x = x;
		printf("B1:set(%d)\n", _x);
	}

protected:
	int _x;

};

class B2 : public B1
{
public:
	B2() : B1(0)
	{
		printf("B2() : x=%d\n", _x);
	}

	void set(int x)
	{
		_x = x;
		printf("B2:set(%d)\n", _x);
	}
};

int _tmain(int argc, _TCHAR* argv[])
{
	B1 b1;
	B2 b2;

	b1.set(10);
	b2.set(20);

	return 0;
}

