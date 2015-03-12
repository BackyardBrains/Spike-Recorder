#include "DropDownList.h"
#include "BitmapFontGL.h"
#include "Painter.h"
#include "BoxLayout.h"
#include "PushButton.h"
#include "ScrollBar.h"
#include "Application.h"

#include <SDL_opengl.h>

namespace BackyardBrains {

namespace Widgets {

static const int BORDER_WIDTH = 2;

DropDownList::DropDownList(Widget *parent) : Widget(parent), /*_state2(WAITING_FOR_CLICK_ON_BUTTON),*/ _selectedIndex(0)/*, _highlightedIndex(0), _verticalOffset(0)*/
{

	setSizeHint(Size(Application::font()->characterWidth()*24 + 2*BORDER_WIDTH, Application::font()->characterHeight() + 2*BORDER_WIDTH));
}

DropDownList::~DropDownList()
{
}

// Size DropDownList::sizeHint() const
// {
// 	return Size(Application::font()->characterWidth()*24 + 2*BORDER_WIDTH, Application::font()->characterHeight() + 2*BORDER_WIDTH);
// }

void DropDownList::clear()
{
	_entries.clear();
	const int oldIndex = _selectedIndex;
	_selectedIndex = 0;
	if (oldIndex != 0)
		indexChanged.emit(0);
}

void DropDownList::addItem(const std::string &str)
{
	_entries.push_back(str);
}

void DropDownList::insertItem(unsigned int index, const std::string &str)
{
	if(index < _entries.size())
		_entries.insert(_entries.begin() + index, str);
}

std::string DropDownList::item(unsigned int index) const
{
	if(index < _entries.size())
		return _entries[index];
	return std::string();
}

void DropDownList::paintEvent()
{
	Painter::setColor(Colors::white);
	Painter::drawRect(rect());
}

void DropDownList::mousePressEvent(MouseEvent *event)
{
	if (event->button() == LeftButton)
	{
		event->accept();

		// Widget * const dummy = new DropDownList;
		Widget * const dummy = new Widget;
		dummy->setDeleteOnClose(true);

		BoxLayout * const hbox = new BoxLayout(Horizontal, dummy);
		hbox->addWidget(new PushButton(dummy));
		hbox->addWidget(new PushButton(dummy));
		hbox->addStretch();
		ScrollBar * const scrollBar = new ScrollBar(Vertical, dummy);
		scrollBar->setRange(0, 3);
		scrollBar->setPageStep(2);
		// scrollBar->setVisible(false);
		hbox->addWidget(scrollBar);
		hbox->update();

		dummy->setGeometry(Rect(mapToGlobal(rect().bottomLeft()), Size(width(), 200)));
		Application::getInstance()->addPopup(dummy);
	}
}

} // namespace Widgets

} // namespace BackyardBrains
