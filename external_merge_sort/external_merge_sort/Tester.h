#pragma once
#include "Sorter.h"
#include <chrono>
#include <ctime>
#include <random>

static const int TEST_AMOUNT = 5;

class Tester
{
public:
	Tester();
	~Tester();

	/**
	Function used for testing streams.
	Opens k distinct streams of size N for reading/writing,
	for each of the implementations 1-4.
	Uses <chrono> to time each of them.
	*/
	void test_streams(unsigned k, unsigned N);
	/**
	Function used for testing the external merge sort.
	Given a input stream of size N (convention: it is named "input_N.txt") for reading/writing,
	for a fixed implementation 1-4 (here it is 3), sorts the file.
	Uses <chrono> to measure execution time.
	*/
	void test_external_merge(unsigned N, unsigned M, unsigned d);
	/**
	Function used for testing the internal memory sort of an input stream of size N (convention: it is named "input_N.txt") for reading/writing,
	Uses std::sort.
	Uses <chrono> to measure execution time.
	*/
	void test_sort(unsigned N);
	/**
	Function used to generate an input stream of size N (convention: it is named "input_N.txt").
	The randomized content is generated with a uniform distributon.
	*/
	void generate_file(unsigned N);
	/**
	Removes unnecessary files.
	*/
	void clean_up(int k);
private:
	/**
	Template function used for testing output streams.
	Opens k distinct streams of size N for writing.
	*/
	template<class OSTREAM>
	void test_ostreams(unsigned k, unsigned N);
	/**
	Template function used for testing input streams.
	Opens k distinct streams of size N for reading.
	*/
	template <typename ISTREAM>
	void test_istreams(unsigned k, unsigned N);
};

