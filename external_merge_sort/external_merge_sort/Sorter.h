#pragma once
#include "Stream.h"
#include <queue>
#include <functional>

#define OUTPUT_TYPE OStream1
#define INPUT_TYPE IStream1

// M = size  of internal memory available
// N = size of original file
// d = nb. of streams to be sorted in one pass

struct Node
{
public:
	Node(int32_t v, unsigned i) : value(v), index(i) {}
	int32_t value;
	unsigned index;

	bool operator>(const Node & other) const
	{
		return value < other.value;
	}
	bool operator<(const Node & other) const
	{
		return value > other.value;
	}
	bool operator>=(const Node & other) const
	{
		return value <= other.value;
	}
	bool operator<=(const Node & other) const
	{
		return value >= other.value;
	}
	bool operator==(const Node & other) const
	{
		return value == other.value;
	}
	bool operator!=(const Node & other) const
	{
		return value != other.value;
	}
};

/*
class used to
split a file into streams of max size M
store the references to those streams in a queue
sort the streams in internal memory
merge d by d streams until only one is left
*/

class Sorter
{
public:
	enum ITYPE { READ = 1, FREAD, IBUFF, IMMAP };
	enum OTYPE { WRITE = 1, FWRITE, OBUFF, OMMAP };

	Sorter(const unsigned long M, const unsigned long d, const std::string & input, const std::string & output = "output.txt");
	~Sorter();

	void sort_file();


private:
	// read the input, and split into smaller ones
	void read_file(ITYPE itype = READ, OTYPE otype = WRITE);
	// sort one stream of size M
	void flush_stream(std::priority_queue<int32_t, std::vector<int32_t>, std::greater<int32_t> > & h);
	// merge d streams. put the result to the end of the queue
	void m_sort(bool final = false);
	// 
	void clean_up();


	std::string input, output;
	unsigned long M, N, d;

	ITYPE itype;
	OTYPE otype;

	std::queue<	std::string> streams;
	unsigned tmp_streams;
};