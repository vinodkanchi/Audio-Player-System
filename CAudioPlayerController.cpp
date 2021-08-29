/*
 * CAudioPlayerController.cpp
 *
 *  Created on: 09.01.2020
 *      Author: Wirth
 */
////////////////////////////////////////////////////////////////////////////////
// Header
#define USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>			// functions to scan files in folders (used in Lab04prep_DBAdminInsert)
using namespace std;

#include "CASDDException.h"
#include "CFile.h"

#include "CAudiofiltersDb.h"
#include "CFilter.h"

#include "CUserInterface.h"
#include "CAudioPlayerController.h"

CAudioPlayerController::CAudioPlayerController() {
	m_pSFile=NULL;		// association with 1 or 0 CSoundFile-objects
	m_pFilter=NULL;		// association with 1 or 0 CFilter-objects
	m_ui=NULL;			// association with 1 or 0 CUserInterface-objects
}

CAudioPlayerController::~CAudioPlayerController() {
	if(m_pSFile)delete m_pSFile;
	if(m_pFilter)delete m_pFilter;
}

void CAudioPlayerController::run(CUserInterface* pui) {
	// if an exception has been thrown by init, the user is not able to use the player
	// therefore the program is terminated (unrecoverable error)
	try
	{
		m_ui=pui;	// set the current user interface (given by the parameter)
		init();		// initialize the components of the controller, if possible and necessary
	}
	catch(CASDDException& e)
	{
		string eMsg="Error from: ";
		m_ui->showMessage(eMsg+e.getSrcAsString()+""+e.getErrorText());
		return;
	}

	//////////////////////////////////////////////////
	// main menue of the player
	// todo: Add further menu items, the corresponding cases and method calls to the following code
	// note: the last item of the menu must be empty (see CUserInterfaceCmdIOW code)
	string mainMenue[]={"select sound","select filter","select amplitude meter scaling mode","play sound","administrate sound collection", "administrate filter collection", "terminate player", ""};
	while(1)
	{
		// if an exception will be thrown by one of the methods, the main menu will be shown
		// after an error message has been displayed. The user may decide, what to do (recoverable error)
		// For instance, if the user selects a filter without having selected a sound file before, an error
		// message tells the user to select a sound file. He/She may decide to do this and then select a filter
		// once more. In this case, the error condition is eliminated and the program may continue regularly.
		try
		{
			// display the menu and get the user's choice
			int selitem=m_ui->getListSelection(mainMenue);
			// process the user's choice by calling the appropriate CAudioPlayerControllerMethod
			switch(selitem)
			{
			case 0: chooseSound();break;
			case 1: chooseFilter();break;
			case 2: chooseAmplitudeScale();break;
			case 3: play();break;
			case 4: manageSoundCollection();break;
			case 5: manageFilterCollection();break;
			default: return;
			}
		}
		catch(CASDDException& e)
		{
			string eMsg="Error from: ";
			m_ui->showMessage(eMsg+e.getSrcAsString()+""+e.getErrorText());
		}
	}
}

void CAudioPlayerController::init()
{
	// no printing - the controller is the only object which may initiate
	// printing via the view object (MVC design)
	try
	{
		m_ui->init();
		m_filterColl.open("localhost","ASDDuser","ASDDuser");
		m_filterColl.allowPrint(false);
		m_soundColl.open("localhost","ASDDuser","ASDDuser");
		m_soundColl.allowPrint(false);
	}
	catch(CASDDException& e)
	{
		e.print();
	}
}

void CAudioPlayerController::chooseFilter()
{
	if(!m_pSFile) // a sound file must have been created by the chooseSoundFile method before
	{
		m_ui->showMessage("Error from selectFilter: No sound file. Select sound file before filter!");
		return;
	}
	// get the sampling rate from the current sound file
	int fs=m_pSFile->getSampleRate();

	/////////////////////////////////////
	// list the appropriate filters for the sound
	// select the appropriate filters
	int numflt=m_filterColl.selectFilters(fs); 	// get the number of appropriate filters
	if(numflt)										// if there are filters that fit
	{
		// prepare a string array for the user interface, that will contain  a menu with the selection of filters
		// there is place for an additional entry for an unfiltered sound and an empty string
		string* pFlt=new string[numflt+2];
		// prepare an integer array for the corresponding filter IDs to pass them to the user interface as well
		// there is place for -1 (unfiltered sound)
		int* pFIDs=new int[numflt+1];

		for(int i=0; i < numflt; i++)
		{
			m_filterColl.fetch();	// get a record of filter data
			// instead to print the filter data, the will be inserted into the string array and the filter ID array
			pFIDs[i]= m_filterColl.getFilterID();
			pFlt[i]= m_filterColl.getFilterType() + "/" + m_filterColl.getFilterSubType()
				   + ", order=" + to_string(m_filterColl.getOrder()) + "/delay="
				   + to_string(m_filterColl.getDelay()) + "s]: "+ m_filterColl.getFilterInfo();
		}
		m_filterColl.closeQuery();

		// add the last menu entry for the choice of an unfiltered sound
		pFIDs[numflt]=-1;
		pFlt[numflt]="-1 [unfiltered sound]";

		// pass the arrays to the user interface and wait for the user's input
		// if the user provides a filterID which is not in pFIDs, the method returns
		// CUI_UNKNOWN
		int fid=m_ui->getListSelection(pFlt,pFIDs);

		// destroy the arrays
		delete[]pFlt;
		delete[]pFIDs;

		/////////////////////////////////////
		// create a filter according to the user's choice
		if(fid != CUI_UNKNOWN)
		{
			if(fid>=0)		// the user has chosen a filter from the filter collection
			{
				// get the filter's data
				if(true == m_filterColl.selectFilter(fid))
				{
					// if there was a filter object from a preceding choice of the user, delete this
					if(m_pFilter)delete m_pFilter;

					// create filter
					// Lab05 changed: get filter data
					int order=m_filterColl.getOrder();
					int delay=m_filterColl.getDelay();
					string type=m_filterColl.getFilterType();
					m_filterColl.closeQuery();
					//

					int numAC=0, numBC=0;
					float* ac=m_filterColl.getACoeffs(fid, numAC);
					float* bc=m_filterColl.getBCoeffs(fid, numBC);
					if(type != "delay")
						m_pFilter=new CFilter(ac,bc,order,m_pSFile->getNumChannels());
					else
						m_pFilter=new CDelayFilter(bc[1]-ac[1], -ac[1], delay, m_pSFile->getSampleRate(), m_pSFile->getNumChannels());
				}
				else
				{
					// wrong ID (may only accidently happen - logical error in the program?)
					m_ui->showMessage("Error from selectFilter: No filter data available! Did not change filter. ");
				}
			}
			else	// the user has chosen not to filter the sound
			{
				if(m_pFilter)	// if there was a filter object from a preceding choice of the user
				{
					delete m_pFilter;	// ... delete this
					m_pFilter=NULL;		// currently we have no filter
					m_ui->showMessage("Message from selectFilter: Filter removed. ");
				}
			}
		}
		else
			m_ui->showMessage("Error from selectFilter: Invalid filter selection! Play unfiltered sound. ");
	}
	else
		m_ui->showMessage("Error from selectFilter: No filter available! Play unfiltered sound. ");
}


void CAudioPlayerController::manageFilterCollection()
{
	// user input for filter file path
	string fltfolder;
	m_ui->showMessage("Enter filter file path: ");
	fltfolder=m_ui->getUserInputPath();

	//////////////////////////////////////////
	// Code from Lab04prep_DBAdminInsert and Lab04prep_insertFilterTest
	// iterates through the folder that the user entered and inserts all
	// the filters it finds in the folder (reading txt files)
	 dirent* entry;
	 DIR* dp;
	 string fltfile;

	 dp = opendir(fltfolder.c_str());
	 if (dp == NULL)
	 {
		 m_ui->showMessage("Could not open filter file folder.");
		 return;
	 }

	 while((entry = readdir(dp)))
	 {
		 fltfile=entry->d_name;
		 m_ui->showMessage("Filter file to insert into the database: " + fltfolder+fltfile+":");


		 if(fltfile.rfind(".txt")!=string::npos)// txt file?
		 {
			const int rbufsize=100;					// assume not more than 100 different sampling frequencies
			char readbuf[rbufsize];

			// get all sampling frequencies contained in the file
			int numFs=rbufsize;
			int fsbuf[rbufsize];
			CFilterFile::getFs((fltfolder+fltfile).c_str(),fsbuf,numFs);

			for(int i=0; i < numFs;i++)	// iterate through all found fs
			{
				CFilterFile ff(fsbuf[i], (fltfolder+fltfile).c_str(), FILE_READ); // create a file object for a certain fs
				ff.open();
				if(ff.read(readbuf,rbufsize))	// read information about the filter with the fs
				{
					// send information to the user interface to display it
					string fileinfo = "Inserting filter file: " + ff.getFilterType() + "/" + ff.getFilterSubType() + " filter [order=" + to_string(ff.getOrder())
							        + ", delay=" + to_string(ff.getDelay()) + "s, fs=" + to_string(fsbuf[i]) + "Hz] " + ff.getFilterInfo();
					m_ui->showMessage(fileinfo);

					// insert the filter into the filter collection database
					if( false == m_filterColl.insertFilter(ff.getFilterType(),ff.getFilterSubType(),
									  fsbuf[i],ff.getOrder(),1000.*ff.getDelay(),ff.getFilterInfo(),
									  ff.getBCoeffs(),ff.getNumBCoeffs(),ff.getACoeffs(),ff.getNumACoeffs()))
						m_ui->showMessage("insert error"/*m_filterColl.getSQLErrorMsg()*/); // if error, let the user interface show the error message
				}
				else
					m_ui->showMessage("No coefficients available for fs=" + to_string(fsbuf[i]) + "Hz");
				ff.close();
			}
		 }
		 else
			 m_ui->showMessage(" irrelevant file of other type or directory");
	 }
	 closedir(dp);
}

void CAudioPlayerController::play() {
	if(!m_pSFile)
	{
		m_ui->showMessage("Error from play: No sound file. Select sound file before playing!");
		return;
	}

	// configure sample buffer
	int framesPerBlock; 			// 1s/8=125ms per block
	if(m_pFilter)
		framesPerBlock= m_pSFile->getSampleRate()/8 > m_pFilter->getOrder() ? m_pSFile->getSampleRate()/8 : m_pFilter->getOrder() * 2;
	else
		framesPerBlock= m_pSFile->getSampleRate()/8; // 1s/8=125ms per block
	int sblockSize=m_pSFile->getNumChannels()*framesPerBlock; 	// total number of samples per block

	float* sbuf=new float[sblockSize];
	float* sfbuf=new float[sblockSize];
	float* playbuf;
	if(m_pFilter==NULL)playbuf=sbuf;
	else playbuf=sfbuf;

	// open and start the audio stream
	m_audiostream.open(m_pSFile->getNumChannels(), m_pSFile->getSampleRate(), framesPerBlock,false);
	m_audiostream.start();

	bool bPlay=true;
	m_ui->showMessage("Press ENTER to start!");
	m_ui->wait4Key();

	// play the file
	// reads the number of frames passed (1 frame == 1 for mono, 2 for stereo, channel number in general)
	int readSize=m_pSFile->read((char*)sbuf, sblockSize*sizeof(float));
	while(readSize)
	{
		if(m_ui->wait4Key(false)==true)bPlay=!bPlay;
		if(bPlay)
		{
			if(m_pFilter)
			{
				if(false==m_pFilter->filter(sbuf,sfbuf,framesPerBlock))
				{
					delete[]sbuf;
					delete[]sfbuf;
					throw CASDDException(SRC_Filter,-1,"Filter order exceeds buffer size!");
				}
			}
			m_audiostream.play(playbuf,framesPerBlock);
			m_ui->showAmplitude(playbuf,sblockSize);
			readSize=m_pSFile->read((char*)sbuf, sblockSize*sizeof(float));
		}
	}

	// prepare next playing
	m_audiostream.close();
	m_pSFile->rewind();

	// release resources
	// audio data buffers
	if(sbuf)delete[]sbuf;
	if(sfbuf)delete[]sfbuf;
}

void CAudioPlayerController::chooseSound() {
	int numsnd=m_soundColl.selectNumSounds();
	if(numsnd > 0)
	{
		string* pSounds=new string[numsnd+1];
		int* pIDs=new int[numsnd];

		m_soundColl.selectAllSounds();

		for(int i=0; i < numsnd; i++)
		{
			m_soundColl.fetch();
			pIDs[i]= m_soundColl.getID();
			pSounds[i]=m_soundColl.getPath() + m_soundColl.getName() + "[" + to_string(m_soundColl.getFs()) + "Hz, " + to_string(m_soundColl.getNumChan()) + " Channels]";
		}
		m_soundColl.closeQuery();
		int sID=m_ui->getListSelection(pSounds,pIDs);
		delete[]pSounds;
		delete[]pIDs;
		if(sID!=CUI_UNKNOWN)
		{
			if(m_soundColl.selectSoundData(sID))
			{
				CSoundFile* psf=new CSoundFile((m_soundColl.getPath()+m_soundColl.getName()).c_str(),FILE_READ);
				m_soundColl.closeQuery();
				try{
					psf->open();
					if(m_pSFile)delete m_pSFile;
					m_pSFile=psf;
				}
				catch(CASDDException& e){
					m_ui->showMessage("Error from " + e.getSrcAsString() + ": " + e.getErrorText());
				}
			}
			else
				m_ui->showMessage("Error from selectSound: No sound data available!");
		}
		else
			m_ui->showMessage("Error from selectSound: Invalid sound selection!");
	}
	else
		throw CASDDException(SRC_Database,-1,"No sounds available!");
}

void CAudioPlayerController::chooseAmplitudeScale() {
	string pscale_menue[]={"linear","logarithmic",""};
	int sel=m_ui->getListSelection(pscale_menue);
	switch(sel)
	{
	case 0: m_ui->setAmplitudeScaling(SCALING_MODE_LIN);;break;
	case 1: m_ui->setAmplitudeScaling(SCALING_MODE_LOG);;break;
	default: m_ui->setAmplitudeScaling(SCALING_MODE_LIN);break;
	}
}

// Lab05 task c)
void CAudioPlayerController::manageSoundCollection() {
	string sndfolder;

	m_ui->showMessage("Enter sound file path: ");
	sndfolder=m_ui->getUserInputPath();

	dirent* entry;
	DIR* dp=NULL;
	string sndfile;

	 dp = opendir(sndfolder.c_str());
	 if (dp == NULL)
	 {
		 m_ui->showMessage("Could not open sound file folder.");
		 return;
	 }

	 while((entry = readdir(dp)))
	 {
		 sndfile=entry->d_name;
		 m_ui->showMessage(sndfile+":");
		 if(sndfile.rfind(".wav")!=string::npos)
		 {
			CSoundFile mysf((sndfolder+sndfile).c_str(),FILE_READ);
			mysf.open();
			string fileinfo = "Inserting sound file: channels(" + to_string(mysf.getNumChannels()) + ") fs(" + to_string(mysf.getSampleRate()) + "Hz)";
			m_ui->showMessage(fileinfo);
			if(false == m_soundColl.insertSound(sndfolder,sndfile,mysf.getSampleRate(),mysf.getNumChannels()))
				m_ui->showMessage(m_soundColl.getSQLErrorMessage("Error from manage sound collection: "));
		 }
		 else
			 m_ui->showMessage(" irrelevant file of other type or directory");
	 }
	if(dp)closedir(dp);
}
