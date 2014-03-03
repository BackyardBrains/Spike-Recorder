#include "LayoutItem.h"
#include "Widget.h"

namespace BackyardBrains {

namespace Widgets {

LayoutItem::LayoutItem(Alignment align) : _alignment(align), _parentItem(NULL)
{
}

LayoutItem::~LayoutItem()
{
}

void LayoutItem::setParentItem(LayoutItem *newParent)
{
	// TODO something about the current value of _parentItem
	_parentItem = newParent;
}

Alignment LayoutItem::alignment() const
{
	return _alignment;
}

void LayoutItem::setAlignment(Alignment align)
{
	_alignment = align;
}

Layout * LayoutItem::layout()
{
	return NULL;
}

Widget * LayoutItem::widget()
{
	return NULL;
}

SpacerItem * LayoutItem::spacerItem()
{
	return NULL;
}

WidgetItem::WidgetItem(Widget *w) : _widget(w)
{
}

Orientations WidgetItem::expandingDirections() const
{
	return _widget->sizePolicy().expandingDirections();
}

void WidgetItem::setGeometry(const Rect &rect)
{
	_widget->setGeometry(rect);
}

Rect WidgetItem::geometry() const
{
	return _widget->geometry();
}

Size WidgetItem::minimumSize() const
{
	return Size();//_widget->minimumSize(); // TODO qSmartMaxSize stuff?
}

Size WidgetItem::maximumSize() const
{
	return Size();//_widget->maximumSize(); // TODO qSmartMaxSize stuff?
}

Size WidgetItem::sizeHint() const
{
	return _widget->sizeHint();
}

Widget * WidgetItem::widget()
{
	return _widget;
}

SpacerItem::SpacerItem(const Size &s, const SizePolicy &sp) : _size(s), _sizePolicy(sp)
{
}

Orientations SpacerItem::expandingDirections() const
{
	return _sizePolicy.expandingDirections();
}

void SpacerItem::setGeometry(const Rect &rect)
{
	_geometry = rect;
}

Rect SpacerItem::geometry() const
{
	return _geometry;
}

Size SpacerItem::minimumSize() const
{
	return Size(_sizePolicy.horizontalPolicy() & SizePolicy::ShrinkFlag ? 0 : _size.w,
	            _sizePolicy.verticalPolicy()   & SizePolicy::ShrinkFlag ? 0 : _size.h);
}

Size SpacerItem::maximumSize() const
{
	return Size(_sizePolicy.horizontalPolicy() & SizePolicy::GrowFlag ? LAYOUTSIZE_MAX : _size.w,
	            _sizePolicy.verticalPolicy()   & SizePolicy::GrowFlag ? LAYOUTSIZE_MAX : _size.h);
}

Size SpacerItem::sizeHint() const
{
	return _size;
}

SpacerItem * SpacerItem::spacerItem()
{
	return this;
}

Layout::Layout(Widget *parent) : _parentWidget(NULL)
{
	setParentWidget(parent);
}

Widget * Layout::parentWidget() const
{
	Layout *item = const_cast<Layout*>(this);
	while (item->_parentItem && item->_parentItem->layout())
		item = item->_parentItem->layout();
	return item->_parentWidget;
}

void Layout::setParentWidget(Widget *newParent)
{
	if (_parentWidget == newParent)
		return;
	if (_parentWidget)
	{
		Widget * const p = _parentWidget;
		_parentWidget = NULL;
		p->setLayout(NULL);
	}
	_parentWidget = newParent;
	if (_parentWidget)
	{
		_parentWidget->setLayout(this);
	}
}

Orientations Layout::expandingDirections() const
{
	return Horizontal | Vertical;
}

void Layout::setGeometry(const Rect &rect)
{
	_geometry = rect;
}

Rect Layout::geometry() const
{
	return _geometry;
}

Size Layout::minimumSize() const
{
	return Size(0, 0);
}

Size Layout::maximumSize() const
{
	return Size(LAYOUTSIZE_MAX, LAYOUTSIZE_MAX);
}

Layout * Layout::layout()
{
	return this;
}

Rect Layout::_AlignRectInsideArea(const Rect &rect, const Rect &area, Alignment alignment)
{
	Rect result = rect;
	if (alignment & AlignRight)
		result.x += (area.w - rect.w);
	else if (alignment & AlignHCenter)
		result.x += (area.w - rect.w)/2;
	if (alignment & AlignBottom)
		result.y += (area.h - rect.h);
	else if (alignment & AlignVCenter)
		result.y += (area.h - rect.h)/2;
	return result;
}

} // namespace Widgets

} // namespace BackyardBrains
