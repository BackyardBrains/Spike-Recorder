#include "AnalysisView.h"
#include "widgets/PushButton.h"
#include "widgets/BoxLayout.h"
#include "widgets/TextureGL.h"
#include "widgets/Painter.h"
#include "widgets/Application.h"
#include "widgets/BitmapFontGL.h"
#include "widgets/Label.h"
#include "widgets/ErrorBox.h"
#include "widgets/Plot.h"
#include "engine/FileRecorder.h"
#include "engine/SpikeAnalysis.h"
#include "AnalysisAudioView.h"
#include "AnalysisTrainList.h"
#include "AnalysisPlots.h"
#include "Log.h"

#include <sstream>
#include <cassert>

namespace BackyardBrains {

AnalysisView::AnalysisView(RecordingManager &mngr, Widgets::Widget *parent) : Widgets::Widget(parent), _manager(mngr) {
	_audioView = new AnalysisAudioView(mngr, _spikeSorter, this);
	_audioView->setSizePolicy(Widgets::SizePolicy(Widgets::SizePolicy::Expanding, Widgets::SizePolicy::Expanding));
	_audioView->addChannel(0);

	_trainList = new AnalysisTrainList(_spikeTrains, this);
	_plots = new AnalysisPlots(_spikeTrains,_manager, this);

	Widgets::PushButton *closeButton = new Widgets::PushButton(this);
	closeButton->clicked.connect(this, &AnalysisView::closePressed);
	closeButton->setNormalTex(Widgets::TextureGL::get("data/analysis.png"));
	closeButton->setHoverTex(Widgets::TextureGL::get("data/analysishigh.png"));

	Widgets::Label *label = new Widgets::Label(this);
	label->setText("Spike Analysis");
	label->updateSize();

	Widgets::ScrollBar *seekBar = new Widgets::ScrollBar(Widgets::Horizontal, this);
	seekBar->setRange(0,1000);
	seekBar->setPageStep(25);
	seekBar->valueChanged.connect((AudioView *)_audioView, &AudioView::setRelOffset);
 	_audioView->relOffsetChanged.connect(seekBar, &Widgets::ScrollBar::updateValue);
	seekBar->setValue(1000);

	Widgets::PushButton *addButton = new Widgets::PushButton(this);
	addButton->setNormalTex(Widgets::TextureGL::get("data/plus.png"));
	addButton->setHoverTex(Widgets::TextureGL::get("data/plushigh.png"));
	//addButton->setSizeHint(Widgets::Size(64,64));
	addButton->clicked.connect(this, &AnalysisView::addPressed);
	Widgets::BoxLayout *addBox = new Widgets::BoxLayout(Widgets::Horizontal);

	Widgets::PushButton *saveButton = new Widgets::PushButton(this);
	saveButton->setNormalTex(Widgets::TextureGL::get("data/save.png"));
	saveButton->setHoverTex(Widgets::TextureGL::get("data/savehigh.png"));
	saveButton->clicked.connect(this, &AnalysisView::savePressed);

	_plotButton = new Widgets::PushButton(this);
	_plotButton->setNormalTex(Widgets::TextureGL::get("data/plotview.png"));
	_plotButton->setHoverTex(Widgets::TextureGL::get("data/plotviewhigh.png"));
	_plotButton->setSizeHint(Widgets::Size(64,32));
	_plotButton->clicked.connect(this, &AnalysisView::plotsPressed);

	addBox->addWidget(_plotButton, Widgets::AlignBottom);
	addBox->addStretch();
	addBox->addWidget(addButton, Widgets::AlignVCenter);
	addBox->addStretch();
	addBox->addSpacing(64);
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

	analysisBar->addWidget(_trainList);
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

	_spikeTrains.push_back(SpikeTrain());

	_trainList->selectionChange.connect(this, &AnalysisView::selectionChanged);
}

void AnalysisView::setPlotData() {
	const int upperthresh = _audioView->upperThresh();
	const int lowerthresh = _audioView->lowerThresh();

	std::vector<int64_t > selected;
	selected.reserve(100);
	for(unsigned int i = 0; i < _spikeSorter.spikes().size(); i++)
		if(_spikeSorter.spikes()[i].second > lowerthresh && _spikeSorter.spikes()[i].second < upperthresh)
			selected.push_back(_spikeSorter.spikes()[i].first);

	std::vector<float> buf, stdy;
	SpikeAnalysis::averageWaveform(buf, stdy, selected, _manager.fileName().c_str(), 0);
	std::vector<float> x, y;
	
	y.resize(buf.size());
	x.resize(buf.size());

	for(int i = 0; i < (int)buf.size(); i++) {
		x[i] = (i-(int)buf.size()/2)/(float)_manager.sampleRate();
		y[i] = buf[i];
	}
	_plot->setData(x,y);
	_plot->setSTD(stdy);
	//_plot->setStyle(Widgets::Plot::Bar);
	_plot->setXLabel("time/s");

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

	if(selectedTrain == _spikeTrains.size()-1 && _spikeTrains[selectedTrain].spikes.size() > 0)
		_spikeTrains.push_back(SpikeTrain());	
	_plots->update();
}


void AnalysisView::savePressed() {
	std::list<std::pair<std::string, int64_t> > markers;

	for(unsigned int i = 0; i < _spikeTrains.size(); i++) {
		std::stringstream s;
		s << "_neuron" << i;
		for(unsigned int j = 0; j < _spikeTrains[i].spikes.size(); j++)
			markers.push_back(std::make_pair(s.str(), _spikeTrains[i].spikes[j]));
	}

	FileRecorder f(_manager);
	std::string filename = f.eventTxtFilename(_manager.fileName());

	f.writeMarkerTextFile(filename, markers);
	std::stringstream s;
	s << markers.size() << " spikes were written to '" << filename << "'";
	Widgets::ErrorBox *box = new Widgets::ErrorBox(s.str().c_str());
	box->setGeometry(Widgets::Rect(this->width()/2-200, this->height()/2-40, 400, 80));
	Widgets::Application::getInstance()->addPopup(box);
}

void AnalysisView::plotsPressed() {
	if(_plots->active()) {
		_plots->setActive(false);
		_plotButton->setNormalTex(Widgets::TextureGL::get("data/plotview.png"));
		_plotButton->setHoverTex(Widgets::TextureGL::get("data/plotviewhigh.png"));

	} else {
		_plotButton->setNormalTex(Widgets::TextureGL::get("data/plotviewdown.png"));
		_plotButton->setHoverTex(Widgets::TextureGL::get("data/plotviewdownhigh.png"));
		_plots->setActive(true);
	}
}

void AnalysisView::selectionChanged(int idx) {
	_audioView->setColorIdx(idx);
	_audioView->setThresh(_spikeTrains[idx].upperThresh, _spikeTrains[idx].lowerThresh);
	_plots->setTarget(idx);

}

}
