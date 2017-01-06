#pragma once
#include "Stream.h"
#include <queue>
#include <functional>


/**
Structure used to represent the nodes in the heap:
	value = integer read from the stream
	index = index of that stream 
Default ordering of Nodes is by value in ascending order.
*/
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

/**
The class used to split a file into streams of maximum size M.
It stores the references to those streams in a queue,
sorts the streams in internal memory and finally
merge d by d streams until only one is left.
*/

class Sorter
{
public:
	Sorter(const unsigned long M, const unsigned long d, const std::string & input, const std::string & output = "output.txt");
	~Sorter();

	void sort_file();

	// Deletes the temporary streams. 
	void clean_up();
private:
	/** 
	Reads the input, and split into smaller ones
	*/
	void read_file();
	/** 
	sort one stream of size M
	*/
	//void flush_stream(std::priority_queue<int32_t, std::vector<int32_t>, std::greater<int32_t> > & h);
	void flush_stream();
	/** 
	Merges d streams, putting the result to the end of the queue.
	If final = TRUE, then creates output file, not a temporary one.
	*/
	void m_sort(bool final = false);
	
	std::string input, output;	// names of the input/output files
	unsigned long M, N, d;		// M = size  of internal memory available
								// N = size of original file
								// d = nb. of streams to be sorted in one pass
	std::vector<int32_t> v;		// holder for the intermediate streams

	std::queue<	std::string> streams;	// queue holding the references to the temporary streams
	unsigned tmp_streams;				// number of temporary streams created
};