// external_merge_sort.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Tester.h"
#include <algorithm>
#include <vector>


int main()
{
	// BUFSIZ = 512 =  128 ints
	getchar();
	unsigned sizes[] = { 10000, 100000, 1000000, 10000000, 100000000 };
	unsigned memory[] = { 1024, 4096, 16384 };
	unsigned ds[] = { 10, 50, 100 };
	Tester tester;

	for (unsigned M : memory)
		for (unsigned N : sizes)
			for (unsigned d : ds)
				tester.test_external_merge(N, M, d);
		
	//
	std::cout << "salsa! <3";
	getchar();
	/*
	unsigned sizes[] = { 1024, 16384, 32768, 1000000, 5000000, 10000000 };
	for (unsigned N : sizes)
	{
		for (unsigned k = 1; k <= 30; k++)
		{
			tester.test_streams(k, N);
			std::cout << k << std::endl;
		}
	}
	*/
    return 0;
}

