#include "RecordingBar.h"
#include <SDL.h>
#include "widgets/Painter.h"
#include "widgets/BitmapFontGL.h"
#include "widgets/Application.h"
#include "engine/FileRecorder.h"
#include <cmath>
#include <sstream>
#include <iomanip>

namespace BackyardBrains {

RecordingBar::RecordingBar(FileRecorder &rec, Widget *parent) : Widget(parent), _active(0), _startTime(-1000), _rec(rec) {
	setSizeHint(Widgets::Size());
	setSizePolicy(Widgets::SizePolicy(Widgets::SizePolicy::Expanding, Widgets::SizePolicy::Fixed));
}

void RecordingBar::setActive(bool active) {
	_startTime = SDL_GetTicks();
	_active = active;
}

bool RecordingBar::active() const {
	return _active;
}

void RecordingBar::paintEvent() {
	if(sizeHint().h == 0)
		return;
	unsigned int t = SDL_GetTicks();
	float f = 0.7f+0.3f*std::sin(t*0.005f);

	Widgets::Painter::setColor(Widgets::Color(100,0,0,255.f*f));
	Widgets::Painter::drawRect(rect());

	if(_active) {
		std::stringstream o;
		o.precision(2);

		const float rtime = _rec.recordTime();
		o << "Recording  " << (int)rtime/60 << ":"<< std::fixed << std::setfill('0') << std::setw(5) << fmod(_rec.recordTime(),60);

		Widgets::Painter::setColor(Widgets::Colors::white);

		Widgets::Application::font()->draw(o.str().c_str(), width()/2-8*7, sizeHint().h-6, Widgets::AlignBottom);
	}
}

void RecordingBar::advance() {
	int t = SDL_GetTicks()-_startTime;
	if((t > 0 && t < 1300) || sizeHint().h > 0) {
		if(_active) {
			setSizeHint(Widgets::Size(sizeHint().w, 30.f*std::tanh(t/500.f)));
			Widgets::Application::getInstance()->updateLayout();
		} else {
			setSizeHint(Widgets::Size(sizeHint().w, 30.f-30.f*std::tanh(t/500.f)));
			Widgets::Application::getInstance()->updateLayout();
		}
	}
}

}
