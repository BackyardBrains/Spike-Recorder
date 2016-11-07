#include "AnalysisPlots.h"
#include "Log.h"
#include "engine/RecordingManager.h"
#include "engine/SpikeAnalysis.h"
#include "widgets/Application.h"
#include "widgets/Painter.h"
#include "widgets/TabBar.h"
#include "widgets/BoxLayout.h"
#include "widgets/Plot.h"
#include "AudioView.h"
#include <cmath>

namespace BackyardBrains {

AnalysisPlots::AnalysisPlots(const std::vector<SpikeTrain> &trains, RecordingManager &manager, Widget *parent) : Widget(parent), _manager(manager), _active(0), _spikeTrains(trains), _target(0) {
	setSizeHint(Widgets::Size());
	
	_tabs = new Widgets::TabBar(this);
	std::vector<std::string> entries;
	entries.push_back("Average Waveform");
	entries.push_back("Autocorrelogram");
	entries.push_back("Crosscorrelogram");
	entries.push_back("Inter Spike Interval");
	_tabs->setEntries(entries);
	_tabs->selectionChanged.connect(this, &AnalysisPlots::tabChanged);


	Widgets::BoxLayout *vbox = new Widgets::BoxLayout(Widgets::Vertical, this);
	_plotLayout = new Widgets::BoxLayout(Widgets::Horizontal);
	
	setPlotCount(1);

	vbox->addWidget(_tabs);
	vbox->addLayout(_plotLayout);

	vbox->update();
}

void AnalysisPlots::setPlotCount(int ncount) {
	int ocount = _plots.size();
	if(ncount < 1)
		ncount = 1;

	if(ncount == ocount)
		return;

	if(ncount < ocount) {
		for(int i = ncount; i < ocount; i++) {
			delete _plots[i];
		}
	}
	_plots.resize(ncount);
	
	_plotLayout->clear();
	for(int i = 0; i < ncount; i++) {
		if(i >= ocount) {
			_plots[i] = new Widgets::Plot(this);
			_plots[i]->setSizePolicy(Widgets::SizePolicy(Widgets::SizePolicy::Expanding, Widgets::SizePolicy::Expanding));
			_plots[i]->setVisible(_active);

		}

		_plotLayout->addWidget(_plots[i]);
	}
	_plotLayout->update();
//	Widgets::Application::getInstance()->updateLayout();
}

void AnalysisPlots::setTarget(int target) {
	if(target != _target) {
		_target = target;
		if(_tabs->selected() == TabCross)
			update();
	}
}

void AnalysisPlots::setAvgWaveformData(int idx) {
	std::vector<float> buf, stdy;
    const float calibrationCoeficient = _manager.getCalibrationCoeficient();
	SpikeAnalysis::averageWaveform(buf, stdy, _spikeTrains[idx].spikes, _manager.fileName().c_str(), 0, calibrationCoeficient);
	std::vector<float> x, y;
	
	y.resize(buf.size());
	x.resize(buf.size());

	for(int i = 0; i < (int)buf.size(); i++) {
		x[i] = (i-(int)buf.size()/2)/(float)_manager.sampleRate();
		y[i] = buf[i];
	}
	_plots[idx]->setData(x,y);
	_plots[idx]->setSTD(stdy);
	_plots[idx]->setStyle(Widgets::Plot::Line);
	_plots[idx]->setYLabel("mV");
	_plots[idx]->setXLabel("time [s]");
}

void AnalysisPlots::setAutocorrData(int idx) {
	std::vector<int> buf;
	SpikeAnalysis::autoCorrelation(buf, _spikeTrains[idx].spikes, 0.1f*_manager.sampleRate(), 0.001f*_manager.sampleRate());
	std::vector<float> x, y;
	
	y.resize(buf.size());
	x.resize(buf.size());

	for(int i = 0; i < (int)buf.size(); i++) {
		x[i] = i*0.001f;
		y[i] = buf[i];
	}
	_plots[idx]->setData(x,y);
	_plots[idx]->setSTD(std::vector<float>());
	_plots[idx]->setStyle(Widgets::Plot::Bar);
	_plots[idx]->setYLabel("# of spikes");
	_plots[idx]->setXLabel("time [s]");
}

void AnalysisPlots::setCrosscorrData(int idx) {
	std::vector<int> buf;
	SpikeAnalysis::crossCorrelation(buf, _spikeTrains[idx].spikes, _spikeTrains[_target].spikes, 0.1f*_manager.sampleRate(), 0.001f*_manager.sampleRate());
	std::vector<float> x, y;
	
	y.resize(buf.size());
	x.resize(buf.size());

	for(int i = 0; i < (int)buf.size(); i++) {
		x[i] = (i-(int)buf.size()/2)*0.001f;
		y[i] = buf[i];
	}
	_plots[idx]->setData(x,y);
	_plots[idx]->setSTD(std::vector<float>());
	_plots[idx]->setStyle(Widgets::Plot::Bar);
	_plots[idx]->setYLabel("# of spikes");
	_plots[idx]->setXLabel("time [s]");

}

void AnalysisPlots::setISIData(int idx) {
	std::vector<int> buf;
	std::vector<double> bins;
	SpikeAnalysis::isiPartition(bins, 100);
	SpikeAnalysis::isi(buf, _spikeTrains[idx].spikes, bins, _manager.sampleRate());
	std::vector<float> x, y;
	
	y.resize(buf.size());
	x.resize(buf.size());

	for(int i = 0; i < (int)buf.size(); i++) {
		x[i] = log10(bins[i]);
		y[i] = buf[i];
	}
	_plots[idx]->setData(x,y);
	_plots[idx]->setSTD(std::vector<float>());
	_plots[idx]->setStyle(Widgets::Plot::Bar);
	_plots[idx]->setYLabel("# of spikes");
	_plots[idx]->setXLabel("time 10^X [s]");
}


void AnalysisPlots::update() {
	for(unsigned int i = 0; i < std::min(_plots.size(),_spikeTrains.size()); i++) {
		setData(i, _tabs->selected());
	}
}

void AnalysisPlots::updateTrain(int idx) {
	if(_tabs->selected() == TabCross && idx == _target) {
		update();
		return;
	}
	setData(idx, _tabs->selected());	
}

void AnalysisPlots::setData(int idx, int tab) {
	if(idx < 0 || idx >= (int) _plots.size() || idx >= (int) _spikeTrains.size())
		return;
	switch(tab) {
	case TabAvgWave:
		setAvgWaveformData(idx);
		break;
	case TabAuto:
		setAutocorrData(idx);
		break;
	case TabCross:
		setCrosscorrData(idx);
		break;
	case TabISI:
		setISIData(idx);
		break;
	}
	_plots[idx]->setColor(AudioView::MARKER_COLORS[_spikeTrains[idx].color%AudioView::MARKER_COLOR_NUM]);
}

void AnalysisPlots::tabChanged(int ntab) {
	update();

	modeChanged.emit(ntab);
}

void AnalysisPlots::setActive(bool active) {
	if(active) {
		setSizePolicy(Widgets::SizePolicy(Widgets::SizePolicy::Expanding, Widgets::SizePolicy::Expanding));
		update();
	} else {
		setSizePolicy(Widgets::SizePolicy(Widgets::SizePolicy::Expanding, Widgets::SizePolicy::Fixed));
	}

	for(unsigned int i = 0; i < _plots.size(); i++)
		_plots[i]->setVisible(active);

	if(active != _active)
		Widgets::Application::getInstance()->updateLayout();
	_active = active;

}

bool AnalysisPlots::active() const {
	return _active;
}

void AnalysisPlots::paintEvent() {
	if(!_active)
		return;
	Widgets::Painter::setColor(Widgets::Colors::widgetbgdark);
	Widgets::Painter::drawRect(rect());

}

}

