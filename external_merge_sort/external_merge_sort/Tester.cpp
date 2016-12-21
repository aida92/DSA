#include "stdafx.h"
#include "Tester.h"


Tester::Tester()
{
}


Tester::~Tester()
{
}



template <typename OSTREAM>
void Tester::test_ostreams(unsigned k, unsigned N)
{
	std::ofstream out;
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

	end = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed_seconds = end - start;
	out << "," << elapsed_seconds.count() << ",,";

	for (unsigned i = 0; i < N; i++)
		for (unsigned j = 0; j < k; j++)
			streams[j].write(i + j);

	for (unsigned j = 0; j < k; j++)
		streams[j].close();

	end = std::chrono::system_clock::now();
	elapsed_seconds = end - start;
	out << elapsed_seconds.count() << std::endl;
	out.close();
}

template <typename ISTREAM>
void Tester::test_istreams(unsigned k, unsigned N)
{
	std::ofstream out;
	out.open("test_streams.csv", std::ios::app);
	std::chrono::time_point<std::chrono::system_clock> start, end;

	start = std::chrono::system_clock::now();
	std::vector<ISTREAM> streams(k);

	for (unsigned j = 0; j < k; j++)
	{
		char filename[255];
		sprintf_s(filename, "test_%d.txt", j);
		streams[j].open(filename);
	}

	end = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed_seconds = end - start;
	out << "," << elapsed_seconds.count() << ",,";

	for (unsigned i = 0; i < N; i++)
		for (unsigned j = 0; j < k; j++)
			streams[j].read_next();

	end = std::chrono::system_clock::now();
	elapsed_seconds = end - start;
	out << elapsed_seconds.count() << std::endl;
	out.close();
}

// Opens k streams for writing/reading N times 
void Tester::test_streams(unsigned k, unsigned N)
{
	//   k N
	//4xout: time_to_open time_to_write
	//4xin:  time_to_open time_to_read
	//
	std::ofstream out;
	out.open("test_streams.csv", std::ios::app);
	out << "k=" << k << ",,N=" << N << std::endl;
	out.close();

	test_ostreams<OStream1>(k, N);
	test_ostreams<OStream2>(k, N);
	test_ostreams<OStream3>(k, N);
	test_ostreams<OStream4>(k, N);

	out.open("test_streams.csv", std::ios::app);
	out << std::endl;
	out.close();

	test_istreams<IStream1>(k, N);
	test_istreams<IStream2>(k, N);
	test_istreams<IStream3>(k, N);
	test_istreams<IStream4>(k, N);

	out.open("test_streams.csv", std::ios::app);
	out << std::endl << std::endl;
	out.close();
}

void Tester::test_external_merge()
{
}
