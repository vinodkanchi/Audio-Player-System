/*
 * File.h
 *
 *  Created on: 14.11.2019
 *      Author: A. Wirth
 */

/**
 * \file File.h
 *
 * header file for class File
 *
 * \see File
 */
#ifndef FILE_H_
#define FILE_H_

#include "sndfile.h"
#include <string>
using namespace std;

/**
 * class to read and write files
 *
 * defines an interface for the operations open, close, read, write, print
 */
enum ASDD_FILEMODES{FILE_UNKNOWN=0x00, FILE_READ=0x01, FILE_WRITE=0x02, FILE_WRITEAPPEND=0x04};
enum ASDD_FILEERRORS{FILE_E_UNKNOWNOPENMODE, FILE_E_NOFILE,FILE_E_FILENOTOPEN, FILE_E_NOBUFFER,FILE_E_READ,FILE_E_CANTREAD,FILE_E_CANTWRITE,FILE_E_WRITE,FILE_E_SPECIAL};

class CFile {
protected:
	unsigned char m_mode;
	string m_path;

public:
	CFile(const char* path=NULL, int mode=FILE_UNKNOWN);
	virtual ~CFile();

/**
 * this class defines an interface for all file classes
 * the following methods are pure virtual that is, they are not implemented
 * and each derived class has to implement these methods
 */
	/**
	 *\brief opens a file
	 */
	virtual void open()=0;
	/**
	 *\brief closes a file
	 */
	virtual void close()=0;
	/**
	 *\brief  reads the content of the file into a buffer
	 *\brief  returns the number of elements read
	 *\params buf[in] - address of a buffer to store the data
	 *\params bufsize[in] - size of the buffer in bytes
	 */
	virtual int read(char* buf, int bufsize)=0;
	/**
	 *\brief  writes the content of the buffer into a file
	 *\params buf[in] - address of a buffer to store the data
	 *\params bufsize[in] - size of the buffer in bytes
	 */
	virtual void write(char* buf, int bufsize)=0;

	/**
	 * prints content of the file on console  (does not throw exceptions)
	 * is not pure virtual that is, it is implemented in this class
	 */
	//void print(void);
	virtual void print(void);

protected:
	/**
	 *\brief returns true if the file has been opened for write  (does not throw exceptions)
	 */
	bool isFileW();
	/**
	 *\brief returns true if the file has been opened for read  (does not throw exceptions)
	 */
	bool isFileR();
	/**
	 *\brief returns true if the file has been opened for write append  (does not throw exceptions)
	 */
	bool isFileWA();
	/**
	 *\brief  returns an error message for a certain error code  (does not throw exceptions)
	 *\params e[in] - error code
	 */
	string getErrorTxt(ASDD_FILEERRORS e);
};

class CRawFile : public CFile
{
protected:
	FILE* m_pFile;
public:
	CRawFile(const char * path, const int mode);
	virtual ~CRawFile();

	/**
	 * opens a raw data file (throws exceptions)
	 */
	virtual void open();
	/**
	 * closes a raw data file (does not throw exceptions)
	 */
	virtual void close();
	/**
	 * reads the content of a raw data file (see CFile, throws exceptions)
	 */
	virtual int read(char* buf, int bufsize);
	/**
	 * prints content a raw data file on console  (does not throw exceptions)
	 */
	virtual void print(void);
	/**
	 * writes the content of a raw data file (see CFile, throws exceptions)
	 */
	virtual void write(char* buf, int bufsize);
};

/**
 * Soundfile handling class
 *
 */
class CSoundFile : public CFile
{
private:
	/**
	 * sound file handle
	 */
	SNDFILE* m_pSFile;
	/**
	 * sound file info (like sampling rate, number of channels)
	 */
	SF_INFO m_sfinfo;

public:
	CSoundFile(const char * path, const ASDD_FILEMODES mode);
	~CSoundFile();

	/**
	 * opens a sound file (throws exceptions)
	 */
	void open();
	/**
	 * closes a sound file (does not throw exceptions)
	 */
	void close();
	/**
	 * reads the content of a sound file (see CFile, throws exceptions)
	 */
	int read(char* buf, int bufsize);
	/**
	 * sets the file pointer back to the beginning of the sound file (see CFile, throws exceptions)
	 */
	void rewind();
	/**
	 * writes the content of a sound file (see CFile, throws exceptions)
	 */
	void write(char* buf, int bufsize);
	/**
	 * prints the informations about a sound file on console (does not throw exceptions)
	 */
	void print(void);
	/**
	 * methods to retrieve information about the sound file (does not throw exceptions)
	 */
	int getNumFrames();
	int getSampleRate();
	int getNumChannels();

	/**
	 * methods to set information about a sound file (needed for writing sound files, throws exceptions)
	 */
	void setSampleRate(int fs);
	void setNumChannels(int numCh);

private:
	/**
	 * read and write methods for sound files (we use the floating point format to represent a sample)
	 *\params buf[in] - address of a float buffer to store/provide the data
	 *\params bufsize[in] - size of the buffer in samples
	 */
	int read(float* buf, int bufsize);
	void write(float* buf, int bufsize);
};

/**
 * class to read files with the ASDD filter file format
 * derived from CRawFile
 * inherits the interface for the operations open, close and read from CRawFile
 * implements additional methods for parsing filter files
 * overloads the basic classes print method
 */
class CFilterFile : public CRawFile
{
private:
	string m_filterType;	// type of the filter (e.g. lowpass, shelving, delay, ...)
	string m_filterSubType;	// NEW: subtype of the filter (e.g. butter, yulewalk, feedback, feedforward, ...)
	string m_filterInfo;	// free textual filter description
	int m_order;			// filter order
	float m_tdelay;			// NEW: delay time (echo filters only)
	float* m_b;				// filter numerator coefficients
	int m_blen;				// number of filter numerator coefficients
	float* m_a;				// filter denominator coefficients
	int m_alen;				// number of filter denominator coefficients
	int m_fs;				// sampling frequency

public:
	CFilterFile(int fs, const char * path, const int mode=FILE_UNKNOWN);
	~CFilterFile();
	/**
	 * opens a file
	 */
	void open();
	/**
	 * closes a file
	 */
	void close();
	/**
	 * reads the content of the file (implements the interface)
	 */
	int read(char* buf, int bufsize);
	/**
	 * prints content of the filter file on console
	 */
	void print(void);
	/**
	 * retrieve information from the filter file
	 */
	/**
	 * static methods (may be called without having an object)
	 */
	static void getFs(const char * path, int* fs, int& numFs);	// get the sample rates for which the file contains coefficients

	int getOrder();						// filter order
	float getDelay();					// NEW: delay time
	string getFilterType();				// type of the filter (e.g. lowpass, shelving, delay, ...)
	string getFilterSubType();			// NEW: subtype of the filter (e.g. butter, yulewalk, feedback, ...)
	string getFilterInfo();				// free textual filter description
	float* getBCoeffs();				// filter numerator coefficients
	int getNumBCoeffs();				// number of filter numerator coefficients
	float* getACoeffs();				// filter denominator coefficients
	int getNumACoeffs();				// number of filter denominator coefficients
};

#endif /* FILE_H_ */
