// external_merge_sort.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Tester.h"
#include <algorithm>
#include <vector>


int main()
{
	// BUFSIZ = 512 =  128 ints
	// Testing external merge-sort
	unsigned sizes[] = { 10000, 100000, 1000000, 10000000, 100000000 }; // # of integers in files.
																		// Note: Created already with tester.generate_file()
	unsigned memory[] = { 1024, 4096, 16384 };	// # of integers that can fit in iternal memory.
	unsigned ds[] = { 10, 50, 100 };			// # of streams to merge in one pass.
	
	Tester tester;
	/*
	for (unsigned M : memory)
		for (unsigned N : sizes)
			for (unsigned d : ds)
				tester.test_external_merge(N, M, d);
	*/

	// Testing streams (first section of the assignment.)
	
	/*
	unsigned sizes[] = { 1024, 16384, 32768, 1000000, 5000000, 10000000 }; // # of integers that can fit into a file
	for (unsigned N : sizes)
	{
		for (unsigned k = 1; k <= 30; k++) // k is the 30 because it's less than the maximum # of streams allowed on the system
		{
			tester.test_streams(k, N);
		}
	}
	*/
    return 0;
}

