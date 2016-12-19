#include "stdafx.h"
#include "Sorter.h"


Sorter::Sorter(const unsigned long M, const unsigned long d, const std::string & input, const std::string & output)
	: M(M), d(d), input(input), output(output), tmp_streams(0) {}


Sorter::~Sorter()
{
}

void Sorter::sort_file()
{
	read_file();

	while (streams.size() > d)
	{
		m_sort();
	}
	d = streams.size();
	m_sort(true);

	clean_up();
}

void Sorter::read_file(ITYPE itype, OTYPE otype)
{
	iStream* in;
	switch (itype)
	{
	case READ:
		in = new IStream1();
		break;
	case FREAD:
		in = new IStream2();
		break;
	case IBUFF:
		in = new IStream3();
		break;
	case IMMAP:
		in = new IStream4();
		break;
	default:
		return;
	}

	in->open(input);

	std::priority_queue<int32_t, std::vector<int32_t>, std::greater<int32_t> > h;

	int n;
	unsigned i = 0, j = 0;

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

}

void Sorter::flush_stream(std::priority_queue<int32_t, std::vector<int32_t>, std::greater<int32_t> > & h)
{
	OStream1 out;
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
}

void Sorter::m_sort(bool final)
{
	std::priority_queue<Node> h;
	std::vector<unsigned> indexes(d);

	std::vector<IStream1> str(d);
	OStream1 out;
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
	for (int i = 0; i < tmp_streams; i++)
	{
		char file[200];
		sprintf_s(file, "out%d.txt", i);
		remove(file);
	}
}