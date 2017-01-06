#pragma once
#include <io.h>
#include <fstream>
#include <string>
#include <iostream>
#include <fcntl.h>
#include <stdlib.h>
#include <Windows.h>
#include <tchar.h>



//============================================================================================
//
// The input stream should support the following operations: 
//	open (open an existing file for reading),
//	read next, (read the next element from the stream),
//	end of stream (a boolean operation that returns true if the end of stream has been reached).
//
//============================================================================================

class iStream
{
public:
	/**
	* Opens the file for reading.
	* Return value: TRUE if successfully opened, FALSE otherwise.
	*/
	virtual bool open(const std::string & filename) = 0;
	/**
	* Reads the next integer from the provided input file.
	* Return value: read int, or NULL upon error.
	*/
	virtual int32_t read_next() = 0;
	/**
	*  Return value: TRUE if EOF is reached, FALSE otherwise.
	*/
	virtual bool end_of_stream() const;
	/**
	Deletes the associated file.
	*/
	void delete_file();
protected:
	bool eof_reached = false;	// Holder for the EOF value,
								// since EOF can only be detected after reading a character.
	std::string filename;
};

//--------------------------------------------------------------------------------------------
//
// 1. Read and write one element at a time using the read and write system calls.
//
//--------------------------------------------------------------------------------------------

class IStream1 :
	public iStream
{
public:
	~IStream1();
	bool open(const std::string & filename);
	int32_t read_next();
private:
	int fd; // file descriptor of the input file
};

//--------------------------------------------------------------------------------------------
//
// 2. Read and write using fread and fwrite functions from the C stdio library.
//
//--------------------------------------------------------------------------------------------

class IStream2 :
	public iStream
{
public:
	~IStream2();
	bool open(const std::string & filename);
	int32_t read_next();
private:
	FILE* stream;
};

//--------------------------------------------------------------------------------------------
//
// 3. Read and write using read and write system calls.
// Streams are equipped with a buffer of size B in internal memory.
// Whenever the buffer becomes empty/full, 
// the next B elements are read / written from/to the file.
//
//--------------------------------------------------------------------------------------------

class IStream3 :
	public iStream
{
public:
	static const int B = 1024;	// size of the internal buffer:
								// represents the number of elements (32bit integers) 
								// that can fit into the buffer.
	IStream3();
	~IStream3();
	bool open(const std::string & filename);
	int32_t read_next();
private:
	/**
	* Reads the next B integers from the file and stores them in the internal buffer.
	* If an error occurs, 'read' is set to -1, otherwise it is set to the number of integers read.
	*/
	void fill_buffer();

	int fd;				// file descriptor for the input file
	int32_t buffer[B];	// the internal buffer 
	int read;			// number of ints read into the buffer (can be less in the final section)
	int index;			// used to iterate through the buffer
};

//--------------------------------------------------------------------------------------------
//
// 4. Read and write is performed by mapping and unmapping a B element portion of the
// file into internal memory through memory mapping.
// Whenever you need to read/write outside of the mapped portion, 
// the next B element portion of the file is mapped.
//
//--------------------------------------------------------------------------------------------

class IStream4 :
	public iStream
{
public:
	static const int B = 16384;	// size of the mapping portion:
								// represents the number of elements which can be mapped into the memory.
	IStream4();
	~IStream4();
	bool open(const std::string & filename);
	int32_t read_next();
private:
	/**
	* Maps the next portion of elements to the memory.
	*/
	void map_next_portion();
	/**
	* Unmaps current portion of the memory.
	*/
	void unmap_portion();

	DWORD buffersize;			// size of the memory to examine at any one time (in bytes)
	DWORD file_map_start;		// current starting point within the file of the data to examine 
	DWORD index;				// iterator through the mapped elements 
	HANDLE h_map_file, h_file;	// handle for the file's memory-mapped region and for the physical file
	DWORD dw_file_size, dw_file_map_size, dw_map_view_size, dw_file_map_start, dw_sys_gran; // sizes			// system allocation granularity
	SYSTEM_INFO sys_info;		// system information; used to get granularity
	LPVOID lp_map_address;		// pointer to the beginning of the memory-mapped region
	char * pData;				// pointer to the data

	bool mapped;
};


//============================================================================================
//
// The output stream should support the following operations: 
//	create (create a new file), 
//	write (write an element to the stream),
//	close (close the stream).
//
//============================================================================================

class oStream
{
public:
	/**
	* Creates a new file / overwrites if file of the same name already exists.
	* Return value: TRUE if successfully opened, FALSE otherwise.
	*/
	virtual bool create(const std::string & filename) = 0;
	/**
	* Writes the integer passed as a parameter to the opened file.
	* Return value: TRUE if successfully written, FALSE otherwise.
	*/
	virtual void write(int32_t n) = 0;
	/**
	* Closes the file.
	* Return value: TRUE if successfully closed, FALSE otherwise.
	*/
	virtual void close() = 0;
	/**
	Deletes the associated file.
	*/
	void delete_file();
protected:
	std::string filename;
};

//--------------------------------------------------------------------------------------------
//
// 1. Read and write one element at a time using the read and write system calls.
//
//--------------------------------------------------------------------------------------------

class OStream1 :
	public oStream
{
public:
	bool create(const std::string & filename);
	void write(int32_t n);
	void close();
private:
	int fd;		// file descriptor of the output file
};

//--------------------------------------------------------------------------------------------
//
// 2. Read and write using fread and fwrite functions from the C stdio library.
//
//--------------------------------------------------------------------------------------------

class OStream2 :
	public oStream
{
public:
	bool create(const std::string & filename);
	void write(int32_t n);
	void close();
private:
	FILE* stream;
};

//--------------------------------------------------------------------------------------------
//
// 3. Read and write using read and write system calls.
// Streams are equipped with a buffer of size B in internal memory.
// Whenever the buffer becomes empty/full, 
// the next B elements are read / written from/to the file.
//
//--------------------------------------------------------------------------------------------

class OStream3 :
	public oStream
{
public:
	static const int B = 1024;	// size of the internal buffer:
								// represents the number of elements (32bit integers) 
								// that can fit into the buffer.

	OStream3();
	bool create(const std::string & filename);
	void write(int32_t n);
	void close();
private:
	/**
	* Writes the next B integers from the buffer to the file.
	*/
	void flush_buffer();

	int fd;					// file descriptor of the output file
	int32_t buffer[B];		// internal buffer 
	int written;			// current number of integers written to the buffer
};

//--------------------------------------------------------------------------------------------
//
// 4. Read and write is performed by mapping and unmapping a B element portion of the
// file into internal memory through memory mapping.
// Whenever you need to read/write outside of the mapped portion, 
// the next B element portion of the file is mapped.
//
//--------------------------------------------------------------------------------------------

class OStream4 :
	public oStream
{
public:
	static const int B = 16384;		// size of the mapping portion:
									// represents the number of elements which can be mapped into the memory.
	OStream4();
	bool create(const std::string & filename);
	void write(int32_t n);
	void close();
private:
	/**
	* Maps the next portion of elements to the memory.
	*/
	void map_next_portion();
	/**
	* Unmaps current portion of the memory.
	*/
	void unmap_portion();

	DWORD index;					// iterator through the mapped elements 
	HANDLE h_map_file, h_file;      // handles for the file's memory-mapped region and file itself 
	DWORD dw_file_size, dw_file_map_size, dw_map_view_size, dw_file_map_start, dw_sys_gran; // holders for sizes
	SYSTEM_INFO sys_info;			// system information; used to get granularity
	LPVOID lp_map_address;			// pointer to the beginning of the memory-mapped region
	char * pData;					// pointer to the data
	DWORD buffersize;				// size of the memory to examine at any one time (in bytes)
	DWORD file_map_start;			// current starting point within the file of the data to examine 
};
