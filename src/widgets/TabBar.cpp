#include "TabBar.h"
#include "Log.h"
#include "Application.h"
#include "BitmapFontGL.h"
#include "Color.h"
#include "Painter.h"
#include <utility>

namespace BackyardBrains {
namespace Widgets {

TabBar::TabBar(Widget *parent) : Widget(parent), _selected(0) {
	setSizePolicy(SizePolicy(Widgets::SizePolicy::Expanding, Widgets::SizePolicy::Fixed));
	setSizeHint(Size(64,2*PADDING+Application::font()->characterHeight()));
}

void TabBar::paintEvent() {
	int cw = Application::font()->characterWidth();
	Painter::setColor(Colors::widgetbg);
	Painter::drawRect(rect());
	Painter::setColor(Colors::button);
	int tabw = width()/_entries.size();
	Painter::drawRect(Rect(tabw*_selected, 0,tabw, height()));

	Painter::setColor(Colors::white);
	for(unsigned int i = 0; i < _entries.size(); i++) {
		int x = tabw*i+tabw/2;
		std::string str = _entries[i];
		int len = str.size();
		if(len > (int)(width()/_entries.size()/cw-2)) {
			len = width()/_entries.size()/cw-2;
			str = _entries[i].substr(0,len)+"-";
		}

		Application::font()->draw(str.c_str(),x,PADDING, AlignHCenter);
	}

}

void TabBar::setEntries(const std::vector<std::string> &entries) {
	_entries = entries;
}

int TabBar::selected() const {
	return _selected;
}

void TabBar::mousePressEvent(MouseEvent *event) {
	if(event->button() == LeftButton) {
		int s = event->pos().x*(int)_entries.size()/width();
		if(s != _selected) {
			_selected = s;
			selectionChanged.emit(s);
		}
	}
	event->accept();
}

}
}
