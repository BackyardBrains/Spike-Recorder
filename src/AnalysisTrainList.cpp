#include "AnalysisTrainList.h"
#include "widgets/Painter.h"
#include "widgets/Application.h"
#include "widgets/BitmapFontGL.h"
#include "AudioView.h"
#include <SDL_opengl.h>
#include <sstream>

namespace BackyardBrains {


AnalysisTrainList::AnalysisTrainList(std::vector<SpikeTrain> &spikeTrains, Widgets::Widget *parent)
	: Widget(parent), _spikeTrains(spikeTrains), _selectedTrain(0) {
	setSizePolicy(Widgets::SizePolicy(Widgets::SizePolicy::Fixed, Widgets::SizePolicy::Expanding));
	setSizeHint(Widgets::Size(150,300));
}

int AnalysisTrainList::selectedTrain() const {
	int i = _selectedTrain;
	if(i < 0)
		i = 0;
	if(i >= (int)_spikeTrains.size())
		i = _spikeTrains.size()-1;
	return i;
}

void AnalysisTrainList::paintEvent() {
	int selecty = (FIELDH+2*PADDING)*_selectedTrain;
	Widgets::Painter::setColor(Widgets::Colors::button);
	Widgets::Painter::drawRect(Widgets::Rect(0,selecty,width(), FIELDH+2*PADDING));
	for(unsigned int i = 0; i < _spikeTrains.size(); i++) {
		if(i != _spikeTrains.size()-1) {
			Widgets::Painter::setColor(AudioView::MARKER_COLORS[i%AudioView::MARKER_COLOR_NUM]);
			std::stringstream s;
			s << _spikeTrains[i].spikes.size() << " spikes";
			Widgets::Application::font()->draw(s.str().c_str(), PADDING*3+FIELDW,
					PADDING+FIELDH/2+(FIELDH+2*PADDING)*i, Widgets::AlignVCenter);

		} else {
			Widgets::Painter::setColor(Widgets::Color(100,100,100));
		}
		Widgets::Painter::drawRect(Widgets::Rect(PADDING, PADDING+(FIELDH+2*PADDING)*i, FIELDW, FIELDH));
	}

	const int plush = FIELDH/2;
	const int plusw = FIELDH/2;

	Widgets::Painter::setColor(Widgets::Colors::buttonhigh);
	Widgets::Painter::drawRect(Widgets::Rect(PADDING+FIELDW/2-2,
				PADDING+(FIELDH+2*PADDING)*(_spikeTrains.size()-1)+(FIELDH-plush)/2,3,plush));
	Widgets::Painter::drawRect(Widgets::Rect(PADDING+(FIELDW-plusw)/2,
				PADDING+(FIELDH+2*PADDING)*(_spikeTrains.size()-1)+FIELDH/2-2,plusw,3));
	
	/*Widgets::Painter::setColor(Widgets::Colors::buttonhigh);
	glBegin(GL_TRIANGLES);
	glVertex3f(PADDING,PADDING+selecty,0);
	glVertex3f(PADDING,PADDING+selecty+FIELDH,0);
	glVertex3f(PADDING+FIELDW/4,PADDING+selecty+FIELDH/2,0);
	glEnd();*/
	
}

void AnalysisTrainList::mousePressEvent(Widgets::MouseEvent *event) {
	if(event->button() == Widgets::LeftButton) {
		int i = event->pos().y/(FIELDH+2*PADDING);
		if(i >= 0 && i < (int)_spikeTrains.size()) {
			_selectedTrain = i;
			event->accept();
			selectionChange.emit(i);
		}
	}		
}

}
