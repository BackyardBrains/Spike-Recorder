#include "ColorDropDownList.h"
#include "widgets/Painter.h"
#include "widgets/Application.h"
#include "widgets/Event.h"
#include <iostream>
#include "widgets/BoxLayout.h"
#include "widgets/PushButton.h"
#include "widgets/ScrollBar.h"

namespace BackyardBrains {

ColorDropDownList::ColorDropDownList(Widget *parent) : _selection(0) {
	setSizeHint(Widgets::Size(60,30));
}

const std::vector<Widgets::Color> &ColorDropDownList::content() const {
	return _content;
}

void ColorDropDownList::setContent(const std::vector<Widgets::Color> &content) {
	_content = content;
}

void ColorDropDownList::setSelection(int selection) {
	if(_selection != selection) {
		_selection = selection;
		selectionChanged.emit(selection);
	}
}

int ColorDropDownList::selection() const {
	return _selection;
}

void ColorDropDownList::paintEvent() {
	Widgets::Painter::setColor(Widgets::Color(80,80,80));
	Widgets::Painter::drawRect(rect());
	if(_content.size() > 0)
		Widgets::Painter::setColor(_content[_selection]);

	Widgets::Painter::drawRect(rect().adjusted(2,2,-12,-2));

}

void ColorDropDownList::mousePressEvent(Widgets::MouseEvent *event) {
	if(event->button() == Widgets::LeftButton) {
		event->accept();

		// Widget * const dummy = new DropDownList;
		Widgets::Widget *dummy = new Widget;
		dummy->setDeleteOnClose(true);

		Widgets::BoxLayout * const hbox = new Widgets::BoxLayout(Widgets::Horizontal, dummy);
		hbox->addWidget(new Widgets::PushButton(dummy));
		hbox->addWidget(new Widgets::PushButton(dummy));
		hbox->addStretch();
		Widgets::ScrollBar * const scrollBar = new Widgets::ScrollBar(Widgets::Vertical, dummy);
		scrollBar->setRange(0, 3);
		scrollBar->setPageStep(2);
		// scrollBar->setVisible(false);
		hbox->addWidget(scrollBar);
		hbox->update();

		dummy->setGeometry(Widgets::Rect(mapToGlobal(rect().bottomLeft()), Widgets::Size(width(), 200)));
		Widgets::Application::getInstance()->addPopup(dummy);
	}

}

}
