/*
 * CAudioPlayerController.h
 *
 *  Created on: 09.01.2020
 *      Author: Wirth
 */

#ifndef SRC_CAUDIOPLAYERCONTROLLER_H_
#define SRC_CAUDIOPLAYERCONTROLLER_H_
#include "CFile.h"

#include "CAudiofiltersDb.h"
#include "CFilter.h"
#include "CUserInterface.h"
#include "CSoundCollectionDB.h"
#include "CSimpleAudioOutStream.h"

class CAudioPlayerController {
private:

	CUserInterface* m_ui;
	CAudiofiltersDb m_filterColl;
	CFilterBase* m_pFilter;
	CSimpleAudioOutStream m_audiostream;
	CSoundCollectionDB m_soundColl;

	CSoundFile* m_pSFile;
	// toDo add the other parts of the controller (see UML Class diagram)

public:
	CAudioPlayerController();
	~CAudioPlayerController();
	void run(CUserInterface* pui);

private:
	void init();
	void play();
	void chooseSound();
	void chooseFilter();
	void chooseAmplitudeScale();
	void manageSoundCollection();
	void manageFilterCollection();
};

#endif /* SRC_CAUDIOPLAYERCONTROLLER_H_ */
