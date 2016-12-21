#include "stdafx.h"
#include "Stream.h"

bool iStream::end_of_stream() const
{
	return eof_reached;
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
		eof_reached = true;		// nothing was left to been read
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

IStream4::IStream4() : dwFileSize(0), dwFileMapSize(0), dwMapViewSize(0), dwFileMapStart(0),
index(0), buffersize(B * sizeof(int32_t)), file_map_start(0)
{
}

IStream4::~IStream4()
{
	unmap_portion();

	bFlag = CloseHandle(hFile);   // close the file itself

	if (!bFlag)
		std::cerr << "Error " << GetLastError() << " occurred closing the input file." << std::endl;
}

bool IStream4::open(const std::string & filename)
{
	// Convert string to TCHAR *
	TCHAR * lpcTheFile = new TCHAR[filename.size() + 1];
	lpcTheFile[filename.size()] = 0;
	std::copy(filename.begin(), filename.end(), lpcTheFile);

	hFile = CreateFile(lpcTheFile, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		std::cerr << "Error " << GetLastError() << " occurred opening the input file." << std::endl;
		return false;
	}
	// Some values need to be calculated:
	dwFileSize = GetFileSize(hFile, NULL); // size of the input file in bytes

	if (dwFileSize == 0)
	{
		eof_reached = true;
		// File is empty so there is no need to continue with the mapping.
		return true;
	}

	//affects where a map view can start. A map view must start at an offset into the file 
	//that is a multiple of the file allocation granularity.
	//So the data you want to view may be the file offset modulo the allocation granularity into the view. 
	//The size of the view is the offset of the data modulo the allocation granularity, 
	//plus the size of the data that you want to examine.
	GetSystemInfo(&SysInfo);
	dwSysGran = SysInfo.dwAllocationGranularity; // 65536b = 16384ints = 64KB
	return true;
}

int32_t IStream4::read_next()
{
	if (index >= dwFileMapSize)
	{
		if (dwFileMapSize > 0)
			unmap_portion();
		map_next_portion();
	}
	pData = (char *)lpMapAddress + index % dwSysGran;
	index += sizeof(int32_t);
	return *(int *)pData;
}

void IStream4::map_next_portion()
{
	// Create a file mapping object for the file
	// Note that it is a good idea to ensure the file size is not zero
	hMapFile = CreateFileMapping(hFile,         // current file handle
		NULL,									// default security
		PAGE_READONLY,							// read permission
		0,										// size of mapping object, high
		0,										// size of mapping object, low ---- If this parameter and dwMaximumSizeHigh are 0 (zero), the maximum size of the file mapping object is equal to the current size of the file that hFile identifies.
		NULL);									// name of mapping object

	if (hMapFile == NULL)
	{
		std::cerr << "Error " << GetLastError() << " occurred opening the map file." << std::endl;
		return;
	}

	long left_in_file = (long)dwFileSize - (long)file_map_start;
	if (left_in_file <= 0)
	{
		eof_reached = true;
		return;
	}
	if (left_in_file < (long)buffersize)
		buffersize = left_in_file; // no need to map the full buffer, since less than that is left to read

								   // To calculate where to start the file mapping, round down the
								   // offset of the data into the file to the nearest multiple of the
								   // system allocation granularity.
	dwFileMapStart = (file_map_start / dwSysGran) * dwSysGran;

	// Calculate the size of the file mapping view.
	dwMapViewSize = (file_map_start % dwSysGran) + buffersize;

	// How large will the file mapping object be?
	dwFileMapSize = file_map_start + buffersize;

	// Map the view.
	lpMapAddress = MapViewOfFile(hMapFile,            // handle to
													  // mapping object
		FILE_MAP_READ,       // read
		0,                   // high-order 32
							 // bits of file
							 // offset
		dwFileMapStart,      // low-order 32
							 // bits of file
							 // offset
		dwMapViewSize);      // number of bytes
							 // to map --- If this parameter is 0 (zero), the mapping extends from the specified offset to the end of the file mapping.
	if (lpMapAddress == NULL)
		std::cerr << "Error " << GetLastError() << " occurred opening the mapping." << std::endl;

	// Update the variables:
	file_map_start += buffersize;
}

void IStream4::unmap_portion()
{
	// Close the file mapping object and the open file
	bFlag = UnmapViewOfFile(lpMapAddress);
	bFlag = CloseHandle(hMapFile);

	if (!bFlag)
		std::cerr << "Error " << GetLastError() << " occurred closing the mapping object." << std::endl;
}




//--------------------------------------------------------------------------------------------
//
// 1. Read and write one element at a time using the read and write system calls.
//
//--------------------------------------------------------------------------------------------

bool OStream1::create(const std::string & filename)
{
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
	// open for reading in binary mode
	//if (fopen_s(&stream, filename.c_str(), "w+b") != 0)
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

OStream4::OStream4() : dwFileSize(0),  dwFileMapSize(0), dwMapViewSize(0), dwFileMapStart(0),
index(0), buffersize(B * sizeof(int32_t)), file_map_start(0)
{}


bool OStream4::create(const std::string & filename)
{
	// Convert string to TCHAR *
	TCHAR * lpcTheFile = new TCHAR[filename.size() + 1];
	lpcTheFile[filename.size()] = 0;
	//As much as we'd love to, we can't use memcpy() because
	//sizeof(TCHAR)==sizeof(char) may not be true:
	std::copy(filename.begin(), filename.end(), lpcTheFile);

	hFile = CreateFile(lpcTheFile, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		std::cerr << "Error " << GetLastError() << " occured opening the output file." << std::endl;
		return false;
	}

	// Get the system allocation granularity. 
	GetSystemInfo(&SysInfo);
	dwSysGran = SysInfo.dwAllocationGranularity; // on my computer it's 65536b (= 16384 integers = 64KB)

	return true;
}


void OStream4::write(int32_t n)
{
	if (index >= dwFileMapSize)
	{
		if (dwFileMapSize > 0)
			unmap_portion();
		map_next_portion();
	}
	pData = (char *)lpMapAddress + index % dwSysGran;
	errno_t err = memcpy_s(pData, sizeof(int), &n, sizeof(int));
	if (err)
		std::cout << err << std::endl;
	index += sizeof(int32_t);
}


void OStream4::close()
{
	// unmap closes the mapping object and the map view
	unmap_portion();

	// Truncate the zeroes, if there are any, that were added while doing the mapping
	if (SetFilePointer(hFile, (index - file_map_start), NULL, FILE_END) == INVALID_SET_FILE_POINTER)
		std::cerr << "Error " << GetLastError() << " occured truncating the output file." << std::endl;
	if (SetEndOfFile(hFile) == 0)
		std::cerr << "Error " << GetLastError() << " occured truncating the output file." << std::endl;
	// and close the file itself
	bFlag = CloseHandle(hFile);
	if (!bFlag)
		std::cerr << "Error " << GetLastError() << " occurred closing the file." << std::endl;
}


/*
Creating a file mapping object does not actually
map the view into a process address space.
The MapViewOfFile and MapViewOfFileEx
functions map a view of a file into a process address space.
*/

void OStream4::map_next_portion()
{
	// First extend the physical file:
	if (SetFilePointer(hFile, buffersize, NULL, FILE_CURRENT) == INVALID_SET_FILE_POINTER)
		std::cerr << "Error " << GetLastError() << " occured extending the output file." << std::endl;
	if (SetEndOfFile(hFile) == 0)
		std::cerr << "Error " << GetLastError() << " occured extending the output file." << std::endl;

	// Create a file mapping object for the file (it needs to extend to the end of the physical file)
	hMapFile = CreateFileMapping(hFile,			// current file handle
		NULL,									// default security
		PAGE_READWRITE,							// read permission
		0,										// size of mapping object, high
		0,										// size of mapping object, low ---- If this parameter and dwMaximumSizeHigh are 0 (zero), the maximum size of the file mapping object is equal to the current size of the file that hFile identifies.
		NULL);									// name of mapping object

	if (hMapFile == NULL)
	{
		std::cerr << "Error " << GetLastError() << " occured creating the mapping object." << std::endl;
		return;
	}

	// To calculate where to start the file mapping, round down the
	// offset of the data into the file to the nearest multiple of the
	// system allocation granularity.
	
	dwFileMapStart = (file_map_start / dwSysGran) * dwSysGran;
	// Calculate the size of the file mapping view.
	dwMapViewSize = (file_map_start % dwSysGran) + buffersize;
	// And the size of the file mapping object.
	dwFileMapSize = file_map_start + buffersize;
	// Map the view.
	lpMapAddress = MapViewOfFile(hMapFile,	// handle to
											// mapping object
		FILE_MAP_ALL_ACCESS,				// read write
		0,									// high-order 32 bits of file offset
		dwFileMapStart,						// low-order 32 bits of file offset
		dwMapViewSize);						// number of bytes to map --- 
											// If this parameter is 0 (zero), the mapping extends from the specified offset to the end of the file mapping.
	if (lpMapAddress == NULL)
		std::cerr << "Error " << GetLastError() << " occured mapping the view." << std::endl;

	// Update the variables.
	file_map_start += buffersize;
}

void OStream4::unmap_portion()
{
	// Close the file mapping object and the open file
	bFlag = UnmapViewOfFile(lpMapAddress);
	if (!bFlag)
		std::cerr << "Error " << GetLastError() << " occurred unmapping file." << std::endl;

	bFlag = CloseHandle(hMapFile); // close the file mapping object

	if (!bFlag)
		std::cerr << "Error " << GetLastError() << " occurred closing the mapping object!" << std::endl;
}
