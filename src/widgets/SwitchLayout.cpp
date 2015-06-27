#include "SwitchLayout.h"
#include "Widget.h"
#include "Application.h"
#include <cassert>

namespace BackyardBrains {

namespace Widgets {

SwitchLayout::SwitchLayout(Widget *parent) : Layout(parent)
{
	_selected = 0;
}

SwitchLayout::~SwitchLayout()
{
	while (!_items.empty())
	{
		LayoutItem * const item = _items.back();
		_items.pop_back();
		item->setParentItem(NULL);
		delete item;
	}
}

Orientations SwitchLayout::expandingDirections() const {
	return Horizontal | Vertical;
}

void SwitchLayout::setGeometry(const Rect &rect) {
	Layout::setGeometry(rect);
	for(unsigned int i = 0; i < _items.size(); i++) {
		if((int) i != _selected) {
			_items[i]->setGeometry(Rect(-100000,0,0,0)); // Hack.
		} else {
			_items[i]->setGeometry(rect);
		}
	}
}

void SwitchLayout::addWidget(Widget *w, Alignment align) {
	if (parentWidget() && w->parentWidget() != parentWidget())
		w->setParentWidget(parentWidget());
	WidgetItem * const wi = new WidgetItem(w);
	wi->setParentItem(this);
	wi->setAlignment(align);
	_items.push_back(wi);
}

void SwitchLayout::addLayout(Layout *l)
{
	l->setParentItem(this);
	_items.push_back(l);
}

Size SwitchLayout::minimumSize() const
{
	if(_items.size() == 0) {
		return Size();
	}
	return _overallMax;
}

Size SwitchLayout::maximumSize() const
{
	if(_items.size() == 0) {
		return Size();
	}
	return _overallMin;
}

Size SwitchLayout::sizeHint() const
{
	if(_items.size() == 0) {
		return Size();
	}
	return _overallHint;
}

void SwitchLayout::update()
{
	_overallMin = Size(LAYOUTSIZE_MAX,LAYOUTSIZE_MAX);
	_overallMax = Size();
	_overallHint = Size();
	for (std::vector<LayoutItem*>::const_iterator it = _items.begin(); it != _items.end(); ++it) {
		if ((*it)->layout())
			(*it)->layout()->update();
		const Size minS = (*it)->minimumSize();
		const Size maxS = (*it)->maximumSize();
		const Size hntS = (*it)->sizeHint();
		
		if(minS.h < _overallMin.h)
			_overallMin.h = minS.h;
		if(minS.w < _overallMin.w)
			_overallMin.w = minS.w;
		if(maxS.h > _overallMax.h)
			_overallMax.h = maxS.h;
		if(maxS.w > _overallMax.w)
			_overallMax.w = maxS.w;
		
		if(hntS.h > _overallHint.h)
			_overallHint.h = hntS.h;
		if(hntS.w > _overallHint.w)
			_overallHint.w = hntS.w;

	}
	//_overallMin = _overallMin.expandedTo(_overallMax);
	//_overallHint = _overallHint.expandedTo(_overallMax);
}

void SwitchLayout::setSelected(int index) {
	assert(index >= 0 && index < (int)_items.size());
	_selected = index;
	Application::getInstance()->updateLayout();
}

int SwitchLayout::selected() const {
	return _selected;
}

} // namespace Widgets

} // namespace BackyardBrains
