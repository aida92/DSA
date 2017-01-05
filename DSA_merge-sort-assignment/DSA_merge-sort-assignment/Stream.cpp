#include "stdafx.h"
#include "Stream.h"

bool iStream::end_of_stream() const
{
	return eof_reached;
}

void iStream::delete_file()
{
	remove(filename.c_str());
}

//--------------------------------------------------------------------------------------------
//
// 1. Read and write one element at a time using the read and write system calls.
//
//--------------------------------------------------------------------------------------------

IStream1::~IStream1()
{
	if (_close(fd) != 0)
		std::cerr << "Error " << GetLastError() << " occurred closing the input file." << std::endl;
}


bool IStream1::open(const std::string & filename)
{
	this->filename = filename;
	fd = _open(filename.c_str(), _O_RDONLY | _O_BINARY, _S_IREAD | _S_IWRITE);
	if (fd == 1)
	{
		std::cerr << "Error " << GetLastError() << " occurred opening the input file." << std::endl;
		return false;
	}
	return true;
}


int32_t IStream1::read_next()
{
	int32_t buf[1];

	int bytes_read = _read(fd, buf, sizeof(int32_t));
	if (bytes_read < 0)
	{
		std::cerr << "Error " << GetLastError() << " occurred using _read system call." << std::endl;
		return NULL;
	}
	if (bytes_read == 0)
	{
		eof_reached = true;
		return NULL;
	}
	// read successfuly
	return buf[0];
}

//--------------------------------------------------------------------------------------------
//
// 2. Read and write using fread and fwrite functions from the C stdio library.
//
//--------------------------------------------------------------------------------------------

IStream2::~IStream2()
{
	if (fclose(stream) != 0)
		std::cerr << "Error " << GetLastError() << " occurred closing the input file." << std::endl;
}


bool IStream2::open(const std::string & filename)
{
	this->filename = filename;
	// open for reading in binary mode
	if ((stream = fopen(filename.c_str(), "rb")) == NULL)
	{
		std::cerr << "Error " << GetLastError() << " occurred opening the input file." << std::endl;
		return false;
	}
	return true;
}


int32_t IStream2::read_next()
{
	int32_t buffer[1];
	if (fread(buffer, sizeof(int32_t), 1, stream) != 1)
	{
		if (feof(stream))
			eof_reached = true;
		else
			std::cerr << "Error " << GetLastError() << " occurred opening the input file." << std::endl;
		return NULL;
	}
	//read successfuly
	return buffer[0];
}

//--------------------------------------------------------------------------------------------
//
// 3. Read and write using read and write system calls.
// Streams are equipped with a buffer of size B in internal memory.
// Whenever the buffer becomes empty/full, 
// the next B elements are read / written from/to the file.
//
//--------------------------------------------------------------------------------------------

IStream3::IStream3() : read(0), index(0)
{
}

IStream3::~IStream3()
{
	if (_close(fd) != 0)
		std::cerr << "Error " << GetLastError() << " occurred closing the input file." << std::endl;
}

bool IStream3::open(const std::string & filename)
{
	this->filename = filename;
	fd = _open(filename.c_str(), _O_RDONLY | _O_BINARY, _S_IREAD | _S_IWRITE);
	if (fd == 1)
	{
		std::cerr << "Error " << GetLastError() << " occurred opening the input file." << std::endl;
		return false;
	}
	return true;
}


int32_t IStream3::read_next()
{
	if (index == read)		// the end of the buffer has been reached / no elements have been read to the buffer yet (first call)
		fill_buffer();
	if (read <= 0)			// fill_buffer returned a value less than 0 which signals an error
		return NULL;
	return buffer[index++];
}


void IStream3::fill_buffer()
{
	read = -1;
	int bytes_to_read = sizeof(int32_t) * B;
	read = _read(fd, buffer, bytes_to_read);
	if (read < 0)
		std::cerr << "Error " << GetLastError() << " occurred reading the input file." << std::endl;
	if (read == 0)
		eof_reached = true;		// nothing was left to be read
	read /= sizeof(int32_t);	// number of elements that were read is the number of bytes divided by size of int (4B=32b)
	index = 0;					// reset the index
}

//--------------------------------------------------------------------------------------------
//
// 4. Read and write is performed by mapping and unmapping a B element portion of the
// file into internal memory through memory mapping.
// Whenever you need to read/write outside of the mapped portion, 
// the next B element portion of the file is mapped.
//
//--------------------------------------------------------------------------------------------

IStream4::IStream4() : dw_file_size(0), dw_file_map_size(0), dw_map_view_size(0), dw_file_map_start(0),
index(0), buffersize(B * sizeof(int32_t)), file_map_start(0)
{
}

IStream4::~IStream4()
{
	unmap_portion();

	if (!CloseHandle(h_file))
		std::cerr << "Error " << GetLastError() << " occurred closing the input file." << std::endl;
}

bool IStream4::open(const std::string & filename)
{
	this->filename = filename;
	// Convert string to TCHAR *
	TCHAR * lpcTheFile = new TCHAR[filename.size() + 1];
	lpcTheFile[filename.size()] = 0;
	std::copy(filename.begin(), filename.end(), lpcTheFile);

	h_file = CreateFile(lpcTheFile, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (h_file == INVALID_HANDLE_VALUE)
	{
		std::cerr << "Error " << GetLastError() << " occurred opening the input file." << std::endl;
		return false;
	}

	dw_file_size = GetFileSize(h_file, NULL); // size of the input file in bytes

	if (dw_file_size == 0)
	{
		eof_reached = true;
		// File is empty so there is no need to continue with the mapping.
		return true;
	}

	GetSystemInfo(&sys_info);
	dw_sys_gran = sys_info.dwAllocationGranularity; // 65536b = 16384ints = 64KB
	return true;
}

int32_t IStream4::read_next()
{
	if (index >= dw_file_map_size)
	{
		if (dw_file_map_size > 0)
			unmap_portion();
		map_next_portion();
	}
	if (eof_reached)
		return NULL;
	pData = (char *)lp_map_address + index % dw_sys_gran;
	index += sizeof(int32_t);
	return *(int *)pData;
}

void IStream4::map_next_portion()
{
	// Create a file mapping object for the file
	h_map_file = CreateFileMapping(h_file, NULL, PAGE_READONLY, 0, 0, NULL);									

	if (h_map_file == NULL)
	{
		std::cerr << "Error " << GetLastError() << " occurred opening the map file." << std::endl;
		return;
	}

	long left_in_file = (long)dw_file_size - (long)file_map_start;
	if (left_in_file <= 0)
	{
		eof_reached = true;
		return;
	}
	if (left_in_file < (long)buffersize)
		buffersize = left_in_file; // no need to map the full buffer, since less than that is left to read

	// Calculate where the map file can start.
	dw_file_map_start = (file_map_start / dw_sys_gran) * dw_sys_gran;
	// Calculate the size of the file mapping view.
	dw_map_view_size = (file_map_start % dw_sys_gran) + buffersize;
	// Calculate the size of the file mapping object.
	dw_file_map_size = file_map_start + buffersize;
	// Map the view.
	lp_map_address = MapViewOfFile(h_map_file, FILE_MAP_READ, 0, dw_file_map_start, dw_map_view_size);

	if (lp_map_address == NULL)
		std::cerr << "Error " << GetLastError() << " occurred opening the mapping." << std::endl;

	// Update the variable:
	file_map_start += buffersize;
	mapped = true;
}

void IStream4::unmap_portion()
{
	if (!mapped)
		return;
	// Close the file mapping object and the open file.
	if (!UnmapViewOfFile(lp_map_address))
		std::cerr << "iError " << GetLastError() << " occurred unmapping file." << std::endl;

	if (!CloseHandle(h_map_file))
		std::cerr << "Error " << GetLastError() << " occurred closing the mapping object." << std::endl;

	mapped = false;
}

//--------------------------------------------------------------------------------------------
//
// 1. Read and write one element at a time using the read and write system calls.
//
//--------------------------------------------------------------------------------------------

bool OStream1::create(const std::string & filename)
{
	this->filename = filename;
	fd = _open(filename.c_str(), _O_CREAT | _O_WRONLY | _O_TRUNC | _O_BINARY, _S_IREAD | _S_IWRITE);
	if (fd == -1)
	{
		std::cerr << "Error " << GetLastError() << " opening output file." << std::endl;
		return false;
	}
	return true;
}


void OStream1::write(int32_t n)
{
	int buf = n;
	if (_write(fd, &buf, sizeof(int32_t)) == -1)
		std::cerr << "Error " << GetLastError() << " occured writing to the output file." << std::endl;
}


void OStream1::close()
{
	if (_close(fd) != 0)
		std::cerr << "Error " << GetLastError() << " occured closing the output file." << std::endl;
}

//--------------------------------------------------------------------------------------------
//
// 2. Read and write using fread and fwrite functions from the C stdio library.
//
//--------------------------------------------------------------------------------------------

bool OStream2::create(const std::string & filename)
{
	this->filename = filename;
	if ((stream = fopen(filename.c_str(), "w+b")) == NULL)
	{
		std::cerr << "Error " << GetLastError() << " occurred opening the output file." << std::endl;
		return false;
	}
	return true;
}


void OStream2::write(int32_t n)
{
	int arr[1];
	arr[0] = n;
	if (fwrite(arr, sizeof(int32_t), 1, stream) < 1)
		std::cerr << "Error " << GetLastError() << " occured writing to the output file." << std::endl;
}


void OStream2::close()
{
	if (fclose(stream) != 0)
		std::cerr << "Error " << GetLastError() << " occured closing the output file." << std::endl;
}

//--------------------------------------------------------------------------------------------
//
// 3. Read and write using read and write system calls.
// Streams are equipped with a buffer of size B in internal memory.
// Whenever the buffer becomes empty/full, 
// the next B elements are read / written from/to the file.
//
//--------------------------------------------------------------------------------------------

OStream3::OStream3() : written(0)
{}


bool OStream3::create(const std::string & filename)
{
	this->filename = filename;
	fd = _open(filename.c_str(), _O_CREAT | _O_WRONLY | _O_TRUNC | _O_BINARY, _S_IREAD | _S_IWRITE);
	if (fd == -1)
	{
		std::cerr << "Error " << GetLastError() << " opening output file." << std::endl;
		return false;
	}
	return true;
}


void OStream3::write(int32_t n)
{
	buffer[written] = n;
	written++;
	if (written == B)
		flush_buffer();
}


void OStream3::close()
{
	flush_buffer();
	if (_close(fd) != 0)
		std::cerr << "Error " << GetLastError() << " occured closing the output file." << std::endl;
}


void OStream3::flush_buffer()
{
	if (_write(fd, buffer, sizeof(int32_t) * written) == -1)
		std::cerr << "Error " << GetLastError() << " occured writing to the output file." << std::endl;
	written = 0;
}

//--------------------------------------------------------------------------------------------
//
// 4. Read and write is performed by mapping and unmapping a B element portion of the
// file into internal memory through memory mapping.
// Whenever you need to read/write outside of the mapped portion, 
// the next B element portion of the file is mapped.
//
//--------------------------------------------------------------------------------------------

OStream4::OStream4() : dw_file_size(0), dw_file_map_size(0), dw_map_view_size(0), dw_file_map_start(0),
index(0), buffersize(B * sizeof(int32_t)), file_map_start(0)
{}


bool OStream4::create(const std::string & filename)
{
	this->filename = filename;
	// Convert string to TCHAR *
	TCHAR * lpcTheFile = new TCHAR[filename.size() + 1];
	lpcTheFile[filename.size()] = 0;
	std::copy(filename.begin(), filename.end(), lpcTheFile);

	h_file = CreateFile(lpcTheFile, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (h_file == INVALID_HANDLE_VALUE)
	{
		std::cerr << "Error " << GetLastError() << " occured opening the output file." << std::endl;
		return false;
	}

	// Get the system allocation granularity. 
	GetSystemInfo(&sys_info);
	dw_sys_gran = sys_info.dwAllocationGranularity; // 65536b = 16384 integers = 64KB

	return true;
}


void OStream4::write(int32_t n)
{
	if (index >= dw_file_map_size)
	{
		if (dw_file_map_size > 0)
			unmap_portion();
		map_next_portion();
	}
	pData = (char *)lp_map_address + index % dw_sys_gran;
	errno_t err = memcpy_s(pData, sizeof(int), &n, sizeof(int));
	if (err)
		std::cerr << "Error " << err << " occured while using memcpy." << std::endl;
	index += sizeof(int32_t);
}


void OStream4::close()
{
	// unmap closes the mapping object and the map view
	unmap_portion();

	// Truncate the zeroes, if there are any, that were added while doing the mapping
	if (SetFilePointer(h_file, (index - file_map_start), NULL, FILE_END) == INVALID_SET_FILE_POINTER)
		std::cerr << "Error " << GetLastError() << " occured truncating the output file." << std::endl;
	if (SetEndOfFile(h_file) == 0)
		std::cerr << "Error " << GetLastError() << " occured truncating the output file." << std::endl;
	// and close the file itself.
	if (!CloseHandle(h_file))
		std::cerr << "Error " << GetLastError() << " occurred closing the file." << std::endl;
}

void OStream4::map_next_portion()
{
	// First extend the physical file:
	if (SetFilePointer(h_file, buffersize, NULL, FILE_CURRENT) == INVALID_SET_FILE_POINTER)
		std::cerr << "Error " << GetLastError() << " occured extending the output file." << std::endl;
	if (SetEndOfFile(h_file) == 0)
		std::cerr << "Error " << GetLastError() << " occured extending the output file." << std::endl;

	h_map_file = CreateFileMapping(h_file, NULL, PAGE_READWRITE, 0, 0,	NULL);									// name of mapping object

	if (h_map_file == NULL)
	{
		std::cerr << "Error " << GetLastError() << " occured creating the mapping object." << std::endl;
		return;
	}

	dw_file_map_start = (file_map_start / dw_sys_gran) * dw_sys_gran;
	// Calculate the size of the file mapping view.
	dw_map_view_size = (file_map_start % dw_sys_gran) + buffersize;
	// And the size of the file mapping object.
	dw_file_map_size = file_map_start + buffersize;
	// Map the view.
	lp_map_address = MapViewOfFile(h_map_file, FILE_MAP_ALL_ACCESS, 0, dw_file_map_start, dw_map_view_size);
	if (lp_map_address == NULL)
		std::cerr << "Error " << GetLastError() << " occured mapping the view." << std::endl;

	// Update the variable.
	file_map_start += buffersize;
}

void OStream4::unmap_portion()
{
	// Close the file mapping object and the opened file
	if (!UnmapViewOfFile(lp_map_address))
		std::cerr << "Error " << GetLastError() << " occurred unmapping file." << std::endl;

	if (!CloseHandle(h_map_file))
		std::cerr << "Error " << GetLastError() << " occurred closing the mapping object!" << std::endl;
}

void oStream::delete_file()
{
	remove(filename.c_str());
}
