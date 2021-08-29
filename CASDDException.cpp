/*
 * CASDDException.cpp
 *
 *  Created on: 24.09.2019
 *      Author: Wirth
 */
#include <iostream>
using namespace std;

#include "CASDDException.h"

CASDDException::CASDDException(ASDD_SOURCES src, int errCode, string errText)
{
	m_source=src;
	m_errorCode=errCode;
	m_errorText=errText;
//	cout << "creating exception: " << "@" << hex << this << endl;
}

CASDDException::~CASDDException()
{
//	cout << "destroying exception: " << "@" << hex << this << endl;
}

CASDDException::CASDDException(const CASDDException& orig)
{
	m_source=orig.m_source;
	m_errorCode=orig.m_errorCode;
	m_errorText=orig.m_errorText;
//	cout << "copying exception: from @" << hex << &orig << " to @" << this << endl;
}

ASDD_SOURCES CASDDException::getSource()
{
	return m_source;
}

int CASDDException::getErrorCode()
{
	return m_errorCode;
}

string CASDDException::getErrorText()
{
	return m_errorText;
}

void CASDDException::setErrorText(string txt)
{
	m_errorText=txt;
}


string CASDDException::getSrcAsString()
{
	switch(m_source)
	{
	case SRC_IOWarrior: return "SRC_IOWarrior"; break;
	case SRC_PlayerIOControls: return "SRC_PlayerIOControls"; break;
	case SRC_AmpMeter: return "SRC_AmpMeter"; break;
	case SRC_SimpleAudioDevice: return "SRC_SimpleAudioDevice"; break;
	case SRC_File: return "SRC_File"; break;
	case SRC_Filter: return "SRC_Filter"; break;
	case SRC_Database: return "SRC_Database"; break;
	default: return "Unknown Source";
	}
}

void CASDDException::print()
{
	cout << "Exception from [" << getSrcAsString() << "]: " << m_errorText << endl;
}

ostream& operator << (ostream& out, CASDDException& e)
{
	out << "Exception from [" << e.getSrcAsString() << "]: " << e.getErrorText();
	return out;
}
