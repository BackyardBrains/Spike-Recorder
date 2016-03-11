#include "AnalysisView.h"
#include "widgets/PushButton.h"
#include "widgets/BoxLayout.h"
#include "widgets/TextureGL.h"
#include "widgets/Painter.h"
#include "widgets/Application.h"
#include "widgets/BitmapFontGL.h"
#include "widgets/Label.h"
#include "widgets/ErrorBox.h"
#include "engine/FileRecorder.h"
#include "engine/SpikeAnalysis.h"
#include "AnalysisAudioView.h"
#include "AnalysisTrainList.h"
#include "AnalysisPlots.h"
#include "Log.h"

#include <sstream>
#include <cassert>
#include <cerrno>

namespace BackyardBrains {

AnalysisView::AnalysisView(RecordingManager &mngr, AnalysisManager &anaman, Widgets::Widget *parent) : Widgets::Widget(parent), _manager(mngr), _spikeTrains(mngr.spikeTrains()) {
	_colorCounter = 0;
	for(unsigned int i = 0; i < _spikeTrains.size(); i++) {
		if(_spikeTrains[i].color >= _colorCounter)
			_colorCounter = _spikeTrains[i].color+1;
	}
	_spikeTrains.push_back(SpikeTrain());
	_spikeTrains.back().color=_colorCounter;
	_colorCounter++;
    
    
	_audioView = new AnalysisAudioView(mngr, anaman, _spikeSorter, this);
	_audioView->setSizePolicy(Widgets::SizePolicy(Widgets::SizePolicy::Expanding, Widgets::SizePolicy::Expanding));
	_audioView->setThresh(_spikeTrains[0].lowerThresh, _spikeTrains[0].upperThresh);
	_audioView->threshChanged.connect(this,&AnalysisView::addPressed);

	_trainList = new AnalysisTrainList(_spikeTrains, this);
	_trainList->trainDeleted.connect(this, &AnalysisView::trainDeleted);
	_plots = new AnalysisPlots(_spikeTrains,_manager, this);
	_plots->modeChanged.connect(this, &AnalysisView::plotModeChanged);
    _plots->setPlotCount(_spikeTrains.size()-1);
    
	Widgets::PushButton *closeButton = new Widgets::PushButton(this);
	closeButton->clicked.connect(this, &AnalysisView::closePressed);
	closeButton->setNormalTex(Widgets::TextureGL::get("data/analysiscrossed.bmp"));
	closeButton->setHoverTex(Widgets::TextureGL::get("data/analysiscrossed.bmp"));

	Widgets::Label *label = new Widgets::Label(this);
	label->setText("Spike Analysis");
	label->updateSize();

	Widgets::ScrollBar *seekBar = new Widgets::ScrollBar(Widgets::Horizontal, this);
	seekBar->setRange(0,1000);
	seekBar->setPageStep(25);
	seekBar->valueChanged.connect((AudioView *)_audioView, &AudioView::setRelOffset);
 	_audioView->relOffsetChanged.connect(seekBar, &Widgets::ScrollBar::updateValue);
	seekBar->setValue(500);

	Widgets::BoxLayout *addBox = new Widgets::BoxLayout(Widgets::Horizontal);

	Widgets::PushButton *saveButton = new Widgets::PushButton(this);
	saveButton->setNormalTex(Widgets::TextureGL::get("data/save.bmp"));
	saveButton->setHoverTex(Widgets::TextureGL::get("data/savehigh.bmp"));
	saveButton->clicked.connect(this, &AnalysisView::savePressed);

	_plotButton = new Widgets::PushButton(this);
	_plotButton->setNormalTex(Widgets::TextureGL::get("data/plotview.bmp"));
	_plotButton->setHoverTex(Widgets::TextureGL::get("data/plotviewhigh.bmp"));
	_plotButton->setSizeHint(Widgets::Size(64,32));
	_plotButton->clicked.connect(this, &AnalysisView::plotsPressed);

	addBox->addWidget(_plotButton, Widgets::AlignBottom);
	addBox->addStretch();
	addBox->setAlignment(Widgets::AlignCenter);
	Widgets::BoxLayout *vbox = new Widgets::BoxLayout(Widgets::Vertical);
	Widgets::BoxLayout *topBar = new Widgets::BoxLayout(Widgets::Horizontal);
	Widgets::BoxLayout *analysisBar = new Widgets::BoxLayout(Widgets::Vertical);
	Widgets::BoxLayout *hbox = new Widgets::BoxLayout(Widgets::Horizontal, this);

	topBar->addSpacing(10);
	topBar->addWidget(closeButton);
	topBar->addSpacing(17);
	topBar->addWidget(label, Widgets::AlignVCenter);
	vbox->addSpacing(10);
	vbox->addLayout(topBar);
	vbox->addSpacing(20);
	vbox->addWidget(_audioView, Widgets::AlignCenter);
	vbox->addWidget(seekBar);
	vbox->addSpacing(10);
	vbox->addLayout(addBox);
	vbox->addWidget(_plots);

	_crossLabel = new Widgets::Label(this);
	_crossLabel->setText("        \x1e        \nSelect target for\ncrosscorrelogram\n");
	_crossLabel->updateSize();
	_crossLabel->setVisible(false);

	analysisBar->addWidget(_trainList);
	analysisBar->addWidget(_crossLabel,Widgets::AlignHCenter);
	analysisBar->addStretch();
	analysisBar->addWidget(saveButton,Widgets::AlignHCenter);
	analysisBar->addSpacing(10);
	hbox->addLayout(vbox);
	hbox->addSpacing(5);
	hbox->addLayout(analysisBar);
	
	hbox->update();

	_spikeSorter.findSpikes(_manager.fileName(), _manager.selectedVDevice(), _manager.sampleRate()/1000 /* 1 ms */);
	
	_wasThreshMode = _manager.threshMode();
	_manager.setThreshMode(false);
	_manager.setPos(_manager.fileLength()/2);


	_trainList->selectionChange.connect(this, &AnalysisView::selectionChanged);
}

void AnalysisView::paintEvent() {
	const Widgets::Color bg = Widgets::Colors::background;

	Widgets::Painter::setColor(bg);
	Widgets::Painter::drawRect(rect());

	Widgets::Rect sidebar = _trainList->geometry();
	sidebar.h = height();
	Widgets::Painter::setColor(Widgets::Colors::widgetbgdark);
	Widgets::Painter::drawRect(sidebar);

}

void AnalysisView::closePressed() {
	_manager.setThreshMode(_wasThreshMode);
    
    
    for(int i = _spikeTrains.size()-1; i >=0 ; i--) {
        if(_spikeTrains[i].spikes.size()==0)
        {
             _spikeTrains.erase(_spikeTrains.begin() + i);
        }
    }
    
    
	close();
}

void AnalysisView::addPressed() {
	unsigned int selectedTrain = _trainList->selectedTrain();

	assert(selectedTrain < _spikeTrains.size());

	_spikeTrains[selectedTrain].spikes.clear();
	_spikeTrains[selectedTrain].spikes.reserve(64);

	const int upperthresh = _audioView->upperThresh();
	const int lowerthresh = _audioView->lowerThresh();

	for(unsigned int i = 0; i < _spikeSorter.spikes().size(); i++)
		if(_spikeSorter.spikes()[i].second > lowerthresh && _spikeSorter.spikes()[i].second < upperthresh)
			_spikeTrains[selectedTrain].spikes.push_back(_spikeSorter.spikes()[i].first);
	_spikeTrains[selectedTrain].upperThresh = upperthresh;
	_spikeTrains[selectedTrain].lowerThresh = lowerthresh;

	if(selectedTrain == _spikeTrains.size()-1 && _spikeTrains[selectedTrain].spikes.size() > 0) {
		_spikeTrains.push_back(SpikeTrain());		
		_colorCounter++;
		_spikeTrains.back().color = _colorCounter;
		_plots->setPlotCount(_spikeTrains.size()-1);
		_trainList->updateSize();
		Widgets::Application::getInstance()->updateLayout();
	}
	if(_plots->active())
		_plots->updateTrain(selectedTrain);
}


void AnalysisView::savePressed() {
	std::list<std::pair<std::string, int64_t> > markers(_manager.markers());

	for(unsigned int i = 0; i < _spikeTrains.size()-1; i++) {
		std::stringstream s;
		s << "_neuron" << _spikeTrains[i].color;
		for(unsigned int j = 0; j < _spikeTrains[i].spikes.size(); j++)
			markers.push_back(std::make_pair(s.str(), _spikeTrains[i].spikes[j]));
		// save thresholds
		std::stringstream s1;
		s1 << "_neuron" << _spikeTrains[i].color << "threshhig" << _spikeTrains[i].upperThresh;
		std::stringstream s2;
		s2 << "_neuron" << _spikeTrains[i].color << "threshlow" << _spikeTrains[i].lowerThresh;
		markers.push_back(std::make_pair(s1.str(), 0));
		markers.push_back(std::make_pair(s2.str(), 0));
	}


	FileRecorder f(_manager);
	std::string filename = f.eventTxtFilename(_manager.fileName());

	int rc = f.writeMarkerTextFile(filename, markers);
	std::stringstream s;
	if(rc == 0) {
		s << markers.size() << " spikes were written to '" << filename << "'";
	} else {
		s << "Could not write markers: " << strerror(errno);
	}
	Widgets::ErrorBox *box = new Widgets::ErrorBox(s.str().c_str());
	box->setGeometry(Widgets::Rect(this->width()/2-200, this->height()/2-40, 400, 80));
	Widgets::Application::getInstance()->addPopup(box);
}

void AnalysisView::plotsPressed() {
	if(_plots->active()) {
		_plots->setActive(false);
		_plotButton->setNormalTex(Widgets::TextureGL::get("data/plotview.bmp"));
		_plotButton->setHoverTex(Widgets::TextureGL::get("data/plotviewhigh.bmp"));

	} else {
		_plotButton->setNormalTex(Widgets::TextureGL::get("data/plotviewdown.bmp"));
		_plotButton->setHoverTex(Widgets::TextureGL::get("data/plotviewdownhigh.bmp"));
		_plots->setActive(true);
	}
}

void AnalysisView::selectionChanged(int idx) {
	_audioView->setColorIdx(_spikeTrains[idx].color);
	_audioView->setThresh(_spikeTrains[idx].upperThresh, _spikeTrains[idx].lowerThresh);
	_plots->setTarget(idx);
}

void AnalysisView::trainDeleted(int idx) {
	_spikeTrains.erase(_spikeTrains.begin()+idx);
	_plots->update();
	selectionChanged(_trainList->selectedTrain());
	_plots->setPlotCount(_spikeTrains.size()-1);
	_trainList->updateSize();
	Widgets::Application::getInstance()->updateLayout();
}
void AnalysisView::plotModeChanged(int mode) {
	_crossLabel->setVisible(mode == AnalysisPlots::TabCross);
}

}
