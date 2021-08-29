/*
 * File.cpp
 *
 *  Created on: 14.11.2019
 *      Author: A. Wirth
 */
#include <iostream>		// Header f�r die Standard-IO-Objekte (z.B. cout, cin)
#include <stdio.h>
#include <string.h>
using namespace std;

#include "CFile.h"
#include "CASDDException.h"

/**
 * Constructor
 */
CFile::CFile(const char* path, int mode) {
	m_path=path;
	if((mode==FILE_UNKNOWN) || ((mode!=FILE_READ) && (mode!=FILE_WRITE) && (mode!=FILE_WRITEAPPEND)))
		m_mode=FILE_READ|FILE_WRITEAPPEND;
	else
		m_mode=mode;
//	cout << "CFile constructor" << endl;
}

CFile::~CFile() {
	//cout << "CFile destructor" << endl;
}

void CFile::print(void)
{
	cout << "CFile[" << (int)m_mode << dec << "]: " << m_path << endl;
}

/**
 * utility methods to get the open mode
 */
bool CFile::isFileW() {
	if((m_mode & FILE_WRITE) == 0)return false;
	return true;
}

bool CFile::isFileR() {
	if((m_mode & FILE_READ) == 0)return false;
	return true;
}

bool CFile::isFileWA() {
	if((m_mode & FILE_WRITEAPPEND) == 0)return false;
	return true;
}

string CFile::getErrorTxt(ASDD_FILEERRORS e) {
	switch(e)
	{
	case FILE_E_UNKNOWNOPENMODE: return string("unknown file open mode");
	case FILE_E_NOFILE: return string("file not found");
	case FILE_E_FILENOTOPEN: return string("file not open");
	case FILE_E_NOBUFFER: return string("no data buffer available");
	case FILE_E_CANTREAD: return string( "file has been opened in write only mode");
	case FILE_E_READ: return string("error during read");
	case FILE_E_CANTWRITE: return string("file has been opened in read only mode");
	case FILE_E_WRITE: return string("error during write");
	case FILE_E_SPECIAL: return string("Special file error: ");
	default:return string("unknown error");
	}
}

CRawFile::CRawFile(const char* path, int mode)
: CFile(path, mode)
{
	//m_mode=100;
	m_pFile=NULL;
	//cout << "CRawFile constructor" << endl;
}

CRawFile::~CRawFile() {
	close();
	//cout << "CRawFile destructor" << endl;
}

void CRawFile::open() {
	string mode;
	if(isFileR() && isFileWA())mode="a+";
	else if(isFileW())mode="w";
	else if(isFileWA())mode="a";
	else if(isFileR())mode="r";
	else throw CASDDException(SRC_File, FILE_E_UNKNOWNOPENMODE, getErrorTxt(FILE_E_UNKNOWNOPENMODE));

	m_pFile=fopen(m_path.c_str(),mode.c_str());
	if(m_pFile == NULL)throw CASDDException(SRC_File, FILE_E_NOFILE, getErrorTxt(FILE_E_NOFILE));
}

void CRawFile::close() {
	if(m_pFile != NULL)
	{
		fclose(m_pFile);
		m_pFile=NULL;
	}
}

int CRawFile::read(char* buf, int bufsize) {
	if(m_pFile == NULL)throw CASDDException(SRC_File, FILE_E_FILENOTOPEN,getErrorTxt(FILE_E_FILENOTOPEN));
	if(buf == NULL)throw CASDDException(SRC_File, FILE_E_NOBUFFER,getErrorTxt(FILE_E_NOBUFFER));
	if(isFileW()||isFileWA())throw CASDDException(SRC_File, FILE_E_CANTREAD,getErrorTxt(FILE_E_CANTREAD));

	int szread= fread(buf, 1, bufsize, m_pFile);
	if((szread != bufsize) && (feof(m_pFile) == 0))
		throw CASDDException(SRC_File, FILE_E_READ,getErrorTxt(FILE_E_READ));
	return szread;
}

void CRawFile::write(char* buf, int bufsize)
{
	if(m_pFile == NULL)throw CASDDException(SRC_File, FILE_E_FILENOTOPEN,getErrorTxt(FILE_E_FILENOTOPEN));
	if(buf == NULL)throw CASDDException(SRC_File, FILE_E_NOBUFFER,getErrorTxt(FILE_E_NOBUFFER));
	if(isFileR())throw CASDDException(SRC_File, FILE_E_CANTWRITE,getErrorTxt(FILE_E_CANTWRITE));

	int szwrite= fwrite(buf, 1, bufsize, m_pFile);
	if(szwrite != bufsize)
	{
		close();
		throw CASDDException(SRC_File, FILE_E_WRITE,getErrorTxt(FILE_E_WRITE));
	}
}

void CRawFile::print(void)
{
	cout << "CRawFile[" << hex << m_pFile << dec << "]" << endl;
	CFile::print();
}

CSoundFile::CSoundFile(const char* path, const ASDD_FILEMODES mode)
: CFile(path, mode)
{
		m_pSFile=NULL;
		memset(&m_sfinfo,0,sizeof(m_sfinfo));
		m_sfinfo.format=SF_FORMAT_WAV|SF_FORMAT_FLOAT;
		//cout << "CSoundFile constructor" << endl;
}

CSoundFile::~CSoundFile()
{
	close();
	//cout << "CSoundFile destructor" << endl;
}

void CSoundFile::open()
{
	int mode;
	if(isFileR() && (isFileWA()||isFileW()))mode=SFM_RDWR;
	else if(isFileW()||isFileWA())mode=SFM_WRITE;
	else if(isFileR())mode=SFM_READ;
	else throw CASDDException(SRC_File, FILE_E_UNKNOWNOPENMODE, getErrorTxt(FILE_E_UNKNOWNOPENMODE));

	m_pSFile= sf_open(m_path.c_str(), mode, &m_sfinfo);
	if(!m_pSFile)throw CASDDException(SRC_File, sf_error(m_pSFile), sf_strerror(m_pSFile));
}

void CSoundFile::close()
{
	if(m_pSFile != NULL)
	{
		sf_close(m_pSFile);
		m_pSFile=NULL;
	}
}

void CSoundFile::print(void)
{
	cout << "CSoundFile[" << hex << m_pSFile << dec << "]" << endl;
	cout << "Soundfile: channels(" << m_sfinfo.channels << ") frames(" << m_sfinfo.frames;
	cout <<  ") fs("<< m_sfinfo.samplerate/1000. << "kHz)" << endl;
	cout << "Duration: " << m_sfinfo.frames/m_sfinfo.samplerate << "s" << endl;
	CFile::print();
}

int CSoundFile::read(char* buf, int bufsize)
{
	return read((float*) buf, bufsize/sizeof(float));
}

int CSoundFile::read(float* buf, int bufsize) {
	if(m_pSFile == NULL)throw CASDDException(SRC_File, FILE_E_FILENOTOPEN,getErrorTxt(FILE_E_FILENOTOPEN));
	if(buf == NULL)throw CASDDException(SRC_File, FILE_E_NOBUFFER,getErrorTxt(FILE_E_NOBUFFER));
	if(isFileW()||isFileWA())throw CASDDException(SRC_File, FILE_E_CANTREAD,getErrorTxt(FILE_E_CANTREAD));

	int szread= sf_read_float(m_pSFile, buf, bufsize);
	// returns 0 if no data left to read
	return szread;
}

void CSoundFile::write(char* buf, int bufsize)
{
	write((float*) buf, bufsize/sizeof(float));
}

void CSoundFile::write(float* buf, int bufsize) {
	if(m_pSFile == NULL)throw CASDDException(SRC_File, FILE_E_FILENOTOPEN,getErrorTxt(FILE_E_FILENOTOPEN));
	if(buf == NULL)throw CASDDException(SRC_File, FILE_E_NOBUFFER,getErrorTxt(FILE_E_NOBUFFER));
	if(isFileR())throw CASDDException(SRC_File, FILE_E_CANTWRITE,getErrorTxt(FILE_E_CANTWRITE));
	if(m_sfinfo.channels==0)throw CASDDException(SRC_File, FILE_E_SPECIAL,getErrorTxt(FILE_E_SPECIAL)+"Writing sound file: Number of channels must not be 0!");
	if(m_sfinfo.samplerate==0)throw CASDDException(SRC_File, FILE_E_SPECIAL,getErrorTxt(FILE_E_SPECIAL)+"Writing sound file: Sampling rate must not be 0!");

	int szwrite= sf_write_float(m_pSFile, buf, bufsize);
	if(szwrite != bufsize)
	{
		close();
		throw CASDDException(SRC_File, FILE_E_WRITE,getErrorTxt(FILE_E_WRITE));
	}
}

int CSoundFile::getNumChannels() {
	return m_sfinfo.channels;
}

void CSoundFile::setNumChannels(int numCh) {
	if(numCh==0)throw CASDDException(SRC_File, FILE_E_SPECIAL, getErrorTxt(FILE_E_SPECIAL)+"Number of channels must not be 0!");
	m_sfinfo.channels=abs(numCh);;
}

int CSoundFile::getNumFrames() {
	return m_sfinfo.frames;
}

int CSoundFile::getSampleRate() {
	return m_sfinfo.samplerate;
}

void CSoundFile::setSampleRate(int fs) {
	if(fs==0)throw CASDDException(SRC_File, FILE_E_SPECIAL, getErrorTxt(FILE_E_SPECIAL)+"Sampling rate must not be 0!");
	m_sfinfo.samplerate=abs(fs);
}

void CSoundFile::rewind()
{
	if(m_pSFile == NULL)throw CASDDException(SRC_File, FILE_E_FILENOTOPEN,getErrorTxt(FILE_E_FILENOTOPEN));
	sf_seek(m_pSFile, 0, SEEK_SET);
}

CFilterFile::CFilterFile(int fs, const char* path, const int mode)
: CRawFile(path, mode)
{
//NEW: fs==0 is allowed for delay filters	if(fs == 0)throw CASDDException(SRC_File, FILE_E_SPECIAL,getErrorTxt(FILE_E_SPECIAL)+"Sampling rate must be specified for a filter!");
	m_fs=abs(fs);	// sample rate of the filter to load
	m_order=0;		// filter order
	m_tdelay=0;		// NEW: delay time
	m_b=NULL;		// filter numerator coefficients
	m_blen=0;		// number of filter numerator coefficients
	m_a=NULL;		// filter denominator coefficients
	m_alen=0;		// number of filter denominator coefficients
}

CFilterFile::~CFilterFile()
{
	close();
}

void CFilterFile::open()
{
	if(m_mode != FILE_READ)throw CASDDException(SRC_File, FILE_E_SPECIAL,getErrorTxt(FILE_E_SPECIAL)+"Writing is not implemented for this class!");
	CRawFile::open();		// perform base class open
}

void CFilterFile::close()
{
	if(m_b)delete[]m_b;
	if(m_a)delete[]m_a;
	CRawFile::close();		// perform base class close
}

int CFilterFile::read(char* buf, int bufsize)
{
	// not necessary, because the base class open already throws an exception if the file does not exist
	//if(m_pFile==NULL)throw CASDDException(SRC_File, FILE_E_FILENOTOPEN,getErrorTxt(FILE_E_FILENOTOPEN));
	if((buf == NULL) || (bufsize == 0))throw CASDDException(SRC_File, FILE_E_NOBUFFER,getErrorTxt(FILE_E_NOBUFFER));

	fseek(m_pFile, 0, SEEK_SET);		// start at the beginning of the file

	// get the header
	fgets(buf, bufsize, m_pFile);

	// parse the header
	string s=buf;
	int pos=0,end=0;
	end=s.find(";",pos);
	m_filterType= s.substr(pos, end-pos);
	// NEW: subtype
	pos=end+1;
	end=s.find(";",pos);
	m_filterSubType= s.substr(pos, end-pos);
	//
	pos=end+1;
	end=s.find(";",pos);
	string sorder= s.substr(pos, end-pos);
	m_order=stoi(sorder);
	// NEW: tdelay
	pos=end+1;
	end=s.find(";",pos);
	string stdelay= s.substr(pos, end-pos);
	m_tdelay=stof(stdelay);			// C++ 11 only static method of string!
	//
	pos=end+1;
	end=s.find("\n",pos);
	m_filterInfo=s.substr(pos, end-pos);

	// read data
	int i, fsr;
	while( NULL != fgets(buf, bufsize, m_pFile))
	{
		fsr=stoi(buf);			// find fs
		//NEW: to change if(m_fs!=fsr)
		if(m_filterType != "delay" && (m_fs!=fsr)) // delay filter coefficients are not depending on fs
		{
			char* pgot=buf;
			while(pgot)			// skip b coefficients
			{
				pgot=fgets(buf, bufsize, m_pFile);
				if(NULL != strrchr(buf,'\n'))break; // end of line has been read
			}
			while(pgot)			// skip a coefficients
			{
				pgot=fgets(buf, bufsize, m_pFile);
				if(NULL != strrchr(buf,'\n'))break;
			}
		}
		else
		{
			// NEW: delay filters have 2 coefficients only
			if(m_filterType == "delay")
				m_order=1;				// read 2 coefficients at maximum (recalculate the order later)
			m_b=new float[m_order+1];
			m_a=new float[m_order+1];
			char sep;
			for(i=0; i<m_order+1;i++)
			{
				if(EOF==fscanf(m_pFile,"%f%c",&m_b[i],&sep))
					break;
				if(sep == '\n')
					break;
			}
			m_blen=i;
			if(sep!='\n')fscanf(m_pFile,"%c",&sep);
			for(i=0; i<m_order+1;i++)
			{
				if(EOF==fscanf(m_pFile,"%f%c",&m_a[i],&sep))
					break;
				if(sep == '\n')
					break;
			}
			m_alen=i;
			if(sep!='\n')fscanf(m_pFile,"%c",&sep);
			// NEW: calculate order in case of delay filters
			if(m_filterType == "delay")
				m_order=m_tdelay*m_fs;
			return ftell(m_pFile);						// returns the number of bytes read
		}
	}
	return 0;											// 0 if no appropriate sampling rate found
}


void CFilterFile::print(void)
{
	CRawFile::print();
	// NEW: add printing of filter subtype and delay time
	cout << m_filterType << "(" << m_filterSubType << ") fiter [order=" << m_order << ", fs=" << m_fs << ", tdelay=" << m_tdelay << "] " << m_filterInfo << endl;
	cout << "coefficients b={";
	for(int i=0; i < m_blen; i++)
		cout << m_b[i] << "\t";
	cout << "}" << endl << "coefficients a={";
	for(int i=0; i < m_alen; i++)
		cout << m_a[i] << "\t";
	cout << "}" << endl;
}

int CFilterFile::getOrder() {
	return m_order;
}

// NEW
float CFilterFile::getDelay()
{
	return m_tdelay;
}

string CFilterFile::getFilterType()
{
	return m_filterType;
}

// NEW
string CFilterFile::getFilterSubType()
{
	return m_filterSubType;
}

string CFilterFile::getFilterInfo()
{
	return m_filterInfo;
}

float* CFilterFile::getBCoeffs()
{
	return m_b;
}

int CFilterFile::getNumBCoeffs()
{
	return m_blen;
}

float* CFilterFile::getACoeffs()
{
	return m_a;
}

int CFilterFile::getNumACoeffs()
{
	return m_alen;
}

// Lab03:
void CFilterFile::getFs(const char * path, int* fs, int& numFs)
{
	FILE* pFile=fopen(path,"r");
	if(pFile==NULL)throw CASDDException(SRC_File, FILE_E_FILENOTOPEN,"Can't open file for reading sample rates!");
	int bufsize=100;
	char buf[100];

	// skip the header
	do
	{
		fgets(buf, bufsize, pFile);
	}while(NULL == strrchr(buf,'\n'));

	// read fs
	int i=0;
	while( (NULL != fgets(buf, bufsize, pFile)) && (numFs > i))
	{
		fs[i++]=stoi(buf);			// store fs
		for(int j=0; j < 2; j++)	// skip coefficients
		{
			do
			{
				fgets(buf, bufsize, pFile);
			}while(NULL == strrchr(buf,'\n'));
		}
	}
	numFs=i;
	fclose(pFile);
}
