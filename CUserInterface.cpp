/*
 * CUserInterface.cpp
 *
 *  Created on: 09.01.2020
 *      Author: Wirth
 */
#include <conio.h>
#include <iostream>
using namespace std;

#include "CUserInterface.h"

CUserInterfaceCmdLine::CUserInterfaceCmdLine() {
}

void CUserInterfaceCmdLine::init() {
	m_pioc.open();
	m_ampMeter.init(-.1,.1,SCALING_MODE_LIN, -6., &m_pioc);
}

int CUserInterfaceCmdLine::getListSelection(string *items, int *idItems) {
	int usel=CUI_UNKNOWN; // initialize user selection
	// how many list items have been passed?
	int numItems=0;
	while(!items[numItems].empty())numItems++; // last item must be empty

	// determine the range of possible user selections
	int rmin=0,rmax=numItems-1;		// default numbering of items: 0...numItems-1)
	if(idItems != NULL)				// other numbering of items passed by an array of appropriate ids
	{
		rmin=idItems[0];
		rmax=idItems[0];
		for(int i=0; i<numItems;i++)
		{
			if(idItems[i]<rmin)rmin=idItems[i];
			else if(idItems[i]>rmax)rmax=idItems[i];
		}
	}

	// displays the numbered list
	for(int i=0; i<numItems;i++)
	{
		if(idItems==NULL)
			cout << "[" << i << "]\t";
		else
			cout << "[" << idItems[i] << "]\t";
		cout << items[i] << endl;
	}
	// get the user selection
	cout << "your selection (number): " << endl;
	usel=m_pioc.getUserInputNum();

	// check the user selection (has to be improved for non-default numbering, because
	// it might be that not all numbers within the range are valid)
	if((usel >= rmin) && (usel <= rmax))
		return usel;
	else
		return CUI_UNKNOWN;
}

string CUserInterfaceCmdLine::getUserInputPath() {
	string path,modpath;
	path=m_pioc.getUserInputString();		// user may provide a path with single backslashes
	unsigned int pathlen=path.length();
	unsigned int pos=0, pos1=0;
	// single backslash is expanded to double backslash
	while((pos1=path.find("\\",pos))<pathlen)
	{
		modpath=modpath+path.substr(pos,pos1-pos)+"\\";
		pos=pos1+1;
	}
	modpath=modpath+path.substr(pos)+"\\";
	return modpath;
}

void CUserInterfaceCmdLine::showMessage(string msg) {
	cout << msg << endl;
}

bool CUserInterfaceCmdLine::wait4Key(bool bBlock) {
	try
	{
		if(bBlock)
		{
			while(m_pioc.keyPressed() == false); // wait for  the user to press the start button
			return true;
		}
		else
			return m_pioc.keyPressed();
	}
	catch(CASDDException& e)
	{
		showMessage(e.getErrorText()+" NO key control available!");
		return true;
	}
}

void CUserInterfaceCmdLine::showAmplitude(float *databuf, int bufsize) {
	m_ampMeter.write(databuf, bufsize);
}

void CUserInterfaceCmdLine::setAmplitudeScaling(SCALING_MODES mode) {
	if(mode==SCALING_MODE_LOG)
		m_ampMeter.init(-.1,.1,SCALING_MODE_LOG, -30., &m_pioc);
	else
		m_ampMeter.init(-.1,.1,SCALING_MODE_LIN, -6., &m_pioc);
}
