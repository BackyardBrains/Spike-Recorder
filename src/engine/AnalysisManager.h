#ifndef BACKYARDBRAINS_ANALYSISMANAGER_H
#define BACKYARDBRAINS_ANALYSISMANAGER_H

#include "EkgBackend.h"
#include "FFTBackend.h"
#include <sigslot.h>

namespace BackyardBrains {

/* This class manages calculation backends for various things in the application
 * and makes them accessible from one point */
class AnalysisManager : public sigslot::has_slots<> {
public:
	AnalysisManager(RecordingManager &recman);

	EkgBackend ekg;	
	FFTBackend fft;	
};

}
#endif
