// external_merge_sort.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Tester.h"
#include <algorithm>
#include <vector>


int main()
{
	Tester tester;
	for (unsigned k = 1; k <= 30; k++)
	{
		tester.test_streams(k, 5000000);
		std::cout << k << std::endl;
	}
	std::cout << "done";
    return 0;
}

