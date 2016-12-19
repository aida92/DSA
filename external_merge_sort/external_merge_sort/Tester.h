#pragma once
#include "Sorter.h"
#include <chrono>
#include <ctime>

class Tester
{
public:
	Tester();
	~Tester();

	void test_streams(unsigned k, unsigned N);

private:
	template<class OSTREAM>
	void test_ostreams(unsigned k, unsigned N);

	template <typename ISTREAM>
	void test_istreams(unsigned k, unsigned N);
};

