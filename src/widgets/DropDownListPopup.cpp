#include "DropDownListPopup.h"
// #include "BitmapFontGL.h"
// #include "Painter.h"
#include "ScrollBar.h"

// #include <SDL_opengl.h>

namespace BackyardBrains {

namespace Widgets {

static const int BORDER_WIDTH = 2;

DropDownListPopup::DropDownListPopup(Widget *parent) : Widget(parent), _selectedIndex(0)
{
	setPopup(true);

	_scrollbar = new ScrollBar(Vertical, this);
	// _scrollbar->setRange(0, 0);
	// _scrollbar->setPageStep(1);
}

DropDownListPopup::~DropDownListPopup()
{
}

Size DropDownListPopup::sizeHint() const
{
	return _scrollbar->sizeHint();
}

void DropDownListPopup::clear()
{
	_entries.clear();
	const int oldIndex = _selectedIndex;
	_selectedIndex = 0;
	if (oldIndex != 0)
		indexChanged.emit(0);
}

void DropDownListPopup::addItem(const std::string &str)
{
	_entries.push_back(str);
}

void DropDownListPopup::insertItem(int index, const std::string &str)
{
	if (index >= 0 && index <= _entries.size())
		_entries.insert(_entries.begin() + index, str);
}

std::string DropDownListPopup::item(int index) const
{
	if (index >= 0 && index <= _entries.size())
		return _entries[index];
	return std::string();
}

void DropDownListPopup::resizeEvent(ResizeEvent *event)
{
	
}

void DropDownListPopup::paintEvent()
{
	Painter::setColor(Colors::white);
	Painter::drawRect(rect());
}

/*void DropDownListPopup::mousePressEvent(MouseEvent *event)
{
}*/

} // namespace Widgets

} // namespace BackyardBrains
