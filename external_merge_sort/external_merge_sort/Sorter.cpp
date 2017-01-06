#include "stdafx.h"
#include "Sorter.h"


Sorter::Sorter(const unsigned long M, const unsigned long d, const std::string & input, const std::string & output)
	: M(M), d(d), input(input), output(output), tmp_streams(0), v(0) {}


Sorter::~Sorter()
{}


void Sorter::sort_file()
{
	read_file();

	if (streams.size() == 1)
	{
		// If the entire file can fit into the available memory, there is no need to do any merging.
		// The output file should be renamed to the provided output file name.
		rename("out0.txt", output.c_str());
		return;
	}

	while (streams.size() > d)
		m_sort();
	// Once there is less than d streams left, merge them all into the final output.
	d = streams.size();
	m_sort(true);
}


void Sorter::read_file()
{
	IStream3 in;
	in.open(input);

	//std::priority_queue<int32_t, std::vector<int32_t>, std::greater<int32_t> > h;
	//std::vector<int32_t> v;
	int n;
	unsigned i = 0, j = 0;
	/*
	while (true)
	{
		n = in->read_next();
		if (in->end_of_stream())
		{
			if (h.size() > 0)
				flush_stream(h);
			break;
		}
		if (j >= M)
		{
			flush_stream(h);
			i++;
			j = 1;
		}
		else
			j++;

		h.push(n);
	}
	*/
	while (true)
	{
		n = in.read_next();
		if (in.end_of_stream())
		{
			if (v.size() > 0)
				flush_stream();
			break;
		}
		if (j >= M)
		{
			flush_stream();
			i++;
			j = 1;
		}
		else
			j++;

		v.push_back(n);
	}
}
/*
void Sorter::flush_stream(std::priority_queue<int32_t, std::vector<int32_t>, std::greater<int32_t> > & h)
{
	OStream3 out;
	// Create a new temporary stream
	char file[100];
	sprintf_s(file, "out%d.txt", tmp_streams++);
	out.create(file);

	while (h.size() > 0)
	{
		out.write(h.top());
		h.pop();
	}
	out.close();
	streams.push(file);
}*/

void Sorter::flush_stream()
{
	OStream3 out;
	// Create a new temporary stream
	char file[100];
	sprintf_s(file, "out%d.txt", tmp_streams++);
	out.create(file);

	for (int32_t i : v)
	{
		out.write(i);
	}
	v.clear();
	out.close();
	streams.push(file);
}

void Sorter::m_sort(bool final)
{
	std::priority_queue<Node> h;
	std::vector<unsigned> indexes(d);

	std::vector<IStream3> str(d);
	OStream3 out;
	// Create a new temporary stream
	char file[200];
	if (final)
		out.create(output);
	else
	{
		sprintf_s(file, "out%d.txt", tmp_streams++);
		out.create(file);
	}

	int n;
	int left = d;

	for (unsigned i = 0; i < d; i++)
	{
		str[i].open(streams.front());
		streams.pop();

		n = str[i].read_next();
		h.push(Node(n, i));
		indexes[i] = 1;
	}

	while (true)
	{
		out.write(h.top().value);
		int ind = h.top().index;
		h.pop();

		n = str[ind].read_next();
		if (str[ind].end_of_stream())
		{
			left--;
			if (left == 0)
				break;
		}
		else
		{
			h.push(Node(n, ind));
			indexes[ind]++;
		}
	}

	while (!h.empty())
	{
		out.write(h.top().value);
		h.pop();
	}
	out.close();
	if (!final)
		streams.push(file);
}

void Sorter::clean_up()
{
	for (unsigned i = 0; i < tmp_streams; i++)
	{
		char file[200];
		sprintf_s(file, "out%d.txt", i);
		remove(file);
	}
}