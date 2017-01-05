#include "stdafx.h"
#include "Tester.h"

Tester::Tester() {}

Tester::~Tester() {}

template <typename OSTREAM>
void Tester::test_ostreams(unsigned k, unsigned N)
{
	std::ofstream out;
	// Open the excel file that holds the results.
	out.open("test_streams.csv", std::ios::app);
	std::chrono::time_point<std::chrono::system_clock> start, end;

	start = std::chrono::system_clock::now();
	std::vector<OSTREAM> streams(k);

	for (unsigned j = 0; j < k; j++)
	{
		char filename[255];
		sprintf_s(filename, "test_%d.txt", j);
		streams[j].create(filename);
	}

	for (unsigned i = 0; i < N; i++)
		for (unsigned j = 0; j < k; j++)
			streams[j].write(i + j); // write anything - here only the time taken is relevant.

	for (unsigned j = 0; j < k; j++)
		streams[j].close();

	end = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed_seconds = end - start;
	out << elapsed_seconds.count() << ",";// in the next cell time for reading will be written
	out.close();
}

template <typename ISTREAM>
void Tester::test_istreams(unsigned k, unsigned N)
{
	std::ofstream out;
	// Open the excel file that holds the results.
	out.open("test_streams.csv", std::ios::app);
	std::chrono::time_point<std::chrono::system_clock> start, end;

	start = std::chrono::system_clock::now();
	std::vector<ISTREAM> streams(k);

	for (unsigned j = 0; j < k; j++) // We know that this is the number of integers supposed to be read.
	{
		char filename[255];
		sprintf_s(filename, "test_%d.txt", j);
		streams[j].open(filename);
	}

	for (unsigned i = 0; i < N; i++)
		for (unsigned j = 0; j < k; j++)
			streams[j].read_next();

	end = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed_seconds = end - start;
	out << elapsed_seconds.count() << std::endl;
	out.close();
}

void Tester::test_streams(unsigned k, unsigned N)
{
	std::ofstream out;
	out.open("test_streams.csv", std::ios::app);
	out << "k," << k << ",,N," << N << std::endl;
	out.close();

	test_ostreams<OStream1>(k, N);
	test_istreams<IStream1>(k, N);

	test_ostreams<OStream2>(k, N);
	test_istreams<IStream2>(k, N);
	
	test_ostreams<OStream3>(k, N);
	test_istreams<IStream3>(k, N);
	
	test_ostreams<OStream4>(k, N);
	test_istreams<IStream4>(k, N);

	clean_up(k);
}

void Tester::test_external_merge(unsigned N, unsigned M, unsigned d)
{
	std::ofstream out;
	out.open("test_sort.csv", std::ios::app);
	//out << "N=" << N << ",,M=" << M << ",,d=" << d << std::endl;

	char file[40];
	sprintf_s(file, "inputs/input_%d.txt", N);

	std::chrono::time_point<std::chrono::system_clock> start, end;
	std::chrono::duration<double> elapsed_seconds;
	Sorter sorter(M, d, file);
	for (int i = 0; i < TEST_AMOUNT; i++)
	{
		
	
		start = std::chrono::system_clock::now();
		sorter.sort_file();
		end = std::chrono::system_clock::now();

		if (i == 0)
			elapsed_seconds = end - start;
		else
			elapsed_seconds += end - start;
		std::cout << i << "." << std::endl;
		
	}
	sorter.clean_up();
	out << elapsed_seconds.count() / TEST_AMOUNT << ",";// in the next cell, internal sort time will be written

	start = std::chrono::system_clock::now();
	test_sort(N);
	end = std::chrono::system_clock::now();
	elapsed_seconds = end - start;
	out << elapsed_seconds.count() << std::endl;
}

void Tester::test_sort(unsigned N)
{
	std::vector<int32_t> v(N);
	IStream3 istr;

	char file[30];
	sprintf_s(file, "inputs/input_%d.txt", N);

	istr.open(file);

	// Store content in internal memory.
	for (unsigned int i = 0; i < N; i++)
		v[i] = istr.read_next();

	// Sort it.
	std::sort(v.begin(), v.end());
	// And print the result. (Can be deleted afterwards.)
	OStream3 ostr;
	ostr.create("tmp.txt");
	for (auto a : v)
		ostr.write(a);
	ostr.close();
	remove("tmp.txt");
}

void Tester::generate_file(unsigned N)
{
	std::default_random_engine generator;
	std::uniform_int_distribution<int> distribution(INT32_MIN, INT32_MAX);

	char file[30];
	sprintf_s(file, "inputs/input_%d.txt", N);

	OStream4 ostr;
	ostr.create(file);
	for (unsigned i = 0; i < N; i++)
		ostr.write(distribution(generator));
	ostr.close();
}


void Tester::clean_up(int k)
{
	for (int i = 0; i < k; i++)
	{
		char file[200];
		sprintf_s(file, "test_%d.txt", i);
		remove(file);
	}
}