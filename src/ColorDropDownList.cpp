#include "ColorDropDownList.h"
#include "widgets/Painter.h"
#include "widgets/Application.h"
#include "widgets/Event.h"
#include "widgets/BoxLayout.h"
#include "widgets/ScrollBar.h"
#include "widgets/TextureGL.h"

namespace BackyardBrains {

static const int SLICE_W = 45;
static const int SLICE_H = 30;

ColorDropDownList::ColorDropDownList(Widget *parent) : Widget(parent), _selection(0) {
	setSizeHint(Widgets::Size(SLICE_W+15,SLICE_H));
}

const std::vector<Widgets::Color> &ColorDropDownList::content() const {
	return _content;
}

void ColorDropDownList::setContent(const std::vector<Widgets::Color> &content) {
	_content = content;
}

void ColorDropDownList::setSelection(int selection) {
	int old = _selection;
	setSelectionSilent(selection);
	if(_selection != old)
		selectionChanged.emit(_selection);
}

void ColorDropDownList::setSelectionSilent(int selection) {
	_selection = std::max(0, std::min((int)_content.size()-1,selection));
}

int ColorDropDownList::selection() const {
	return _selection;
}

void ColorDropDownList::paintEvent() {
	Widgets::Painter::setColor(Widgets::Colors::widgetbg);
	Widgets::Painter::drawRect(rect());
	if(_content.size() > 0)
		Widgets::Painter::setColor(_content[_selection]);

	Widgets::Painter::drawRect(Widgets::Rect(2,2,SLICE_W-4, height()-4));

	Widgets::TextureGL::get("data/dropdown.png")->bind();
	Widgets::Painter::setColor(Widgets::Colors::button);
	Widgets::Painter::drawTexRect(Widgets::Rect(SLICE_W+2,height()/2-3,width()-SLICE_W-6, 6));
	glBindTexture(GL_TEXTURE_2D, 0);

}

void ColorDropDownList::mousePressEvent(Widgets::MouseEvent *event) {
	if(event->button() == Widgets::LeftButton) {
		event->accept();

		ColorDropDownPopup *popup = new ColorDropDownPopup(_content);
		popup->selectionChanged.connect(this, &ColorDropDownList::setSelection);
		popup->setGeometry(Widgets::Rect(mapToGlobal(rect().bottomLeft()), Widgets::Size(width(), 120)));
		Widgets::Application::getInstance()->addPopup(popup);
	} else if(event->button() == Widgets::WheelUpButton) {
		setSelection(_selection-1);
		event->accept();
	} else if(event->button() == Widgets::WheelDownButton) {
		setSelection(_selection+1);
		event->accept();
	}
}

ColorDropDownPopup::ColorDropDownPopup(const std::vector<Widgets::Color> &content, Widget *parent)
	: Widget(parent), _content(content), _scroll(0) {
	setSizeHint(Widgets::Size(SLICE_W+15,SLICE_H*4));

	_scrollBar = new Widgets::ScrollBar(Widgets::Vertical, this);
	_scrollBar->valueChanged.connect(this, &ColorDropDownPopup::setScroll);
	scrollChanged.connect(_scrollBar, &Widgets::ScrollBar::updateValue);
	_scrollBar->setGeometry(Widgets::Rect(SLICE_W,0,width()-SLICE_W,height()));
	_scrollBar->updateValue(_scroll);
	_scrollBar->setPageStep(10);
}

void ColorDropDownPopup::setScroll(int scroll) {
	_scroll = std::max(0,std::min((int)(_content.size())*SLICE_H-height(), scroll));
	scrollChanged.emit(_scroll);
}

void ColorDropDownPopup::paintEvent() {
	Widgets::Painter::setColor(Widgets::Colors::widgetbgdark);
	Widgets::Painter::drawRect(rect());

	unsigned int i;
	int start = _scroll/SLICE_H;
	int startoff = _scroll%SLICE_H;
	for(i = 0; i <= (unsigned int)height()/SLICE_H+1 && i+start < _content.size(); i++) {
		Widgets::Painter::setColor(_content[i+start]);
		int y = i*SLICE_H-startoff;
		int h = std::min(height()-y, SLICE_H);
		if(y < 0)
			h += y;

		y = std::max(0, y);

		Widgets::Painter::drawRect(Widgets::Rect(2,y+2, SLICE_W-4, h-4));
	}

}

void ColorDropDownPopup::mousePressEvent(Widgets::MouseEvent *event) {
	if(event->button() == Widgets::LeftButton && event->pos().x < SLICE_W) {
		event->accept();
		int selected = (event->pos().y+_scroll)/SLICE_H;
		selectionChanged.emit(selected);
		close();
	} else if(event->button() == Widgets::WheelUpButton) {
		setScroll(_scroll-10);
		event->accept();
	} else if(event->button() == Widgets::WheelDownButton) {
		setScroll(_scroll+10);
		event->accept();
	}
}

void ColorDropDownPopup::resizeEvent(Widgets::ResizeEvent *event) {
	_scrollBar->setGeometry(Widgets::Rect(SLICE_W,0,width()-SLICE_W,height()));
	_scrollBar->setRange(0, std::max(0,(int)(_content.size())*SLICE_H-height()));
}

}
