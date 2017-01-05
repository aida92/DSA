// DSA_merge-sort-assignment.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Stream.h"


int main()
{
	OStream4 ostr;
	ostr.create("aida.txt");
	for (int i = 0; i < 20; i++)
		ostr.write(i);
	ostr.close();

	IStream4 istr;
	istr.open("aida.txt");
	int n = 0;
	while (true)
	{
		n = istr.read_next();
		if (istr.end_of_stream())
			break;
		std::cout << n << std::endl;
	}
	istr.delete_file();
    return 0;
}

