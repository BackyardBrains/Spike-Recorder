#include "BoxLayout.h"
#include "Widget.h"

namespace BackyardBrains {

namespace Widgets {

BoxLayout::BoxLayout(Orientation orient, Widget *parent) : Layout(parent), _orientation(orient), _expandingDirections(0)
{
}

BoxLayout::~BoxLayout()
{
	while (!_items.empty())
	{
		LayoutItem * const item = _items.back();
		_items.pop_back();
		item->setParentItem(NULL);
		delete item;
	}
}

Orientations BoxLayout::expandingDirections() const
{
	// return Horizontal | Vertical;
	return _expandingDirections;
	// return 0;
}

void BoxLayout::setGeometry(const Rect &rect)
{
	Layout::setGeometry(rect);
	Point point = rect.topLeft();
	int expandCount = 0;
	for (ItemVector::const_iterator it = _items.begin(); it != _items.end(); ++it)
	{
		if ((*it)->expandingDirections() & _orientation)
			expandCount++;
	}
	const int extraSpace = (_orientation == Horizontal) ? std::max(0, rect.w - _overallHint.w) : std::max(0, rect.h - _overallHint.h);
	// int usedSpace = 0;
	for (ItemVector::const_iterator it = _items.begin(); it != _items.end(); ++it)
	{
		Size s = (*it)->sizeHint();
		Alignment a = (*it)->alignment();

		// if it wants to expand, do so using the stretch factor
		if ((*it)->expandingDirections() & _orientation)
			((_orientation == Horizontal) ? s.w : s.h) += extraSpace/expandCount;
		// usedSpace += ((_orientation == Horizontal) ? s.w : s.h);

		// if it can expand in the direction perpendicular to the packing order, do so
		if ((*it)->expandingDirections() & ((_orientation == Horizontal) ? Vertical : Horizontal))
		{
			if (_orientation == Horizontal)
				s.h = rect.h;
			else
				s.w = rect.w;
		}

		// actually position the item once we've determined it's size using alignment
		(*it)->setGeometry(_AlignRectInsideArea(Rect(point, s), Rect(point, s.expandedTo((_orientation == Horizontal) ? Size(0, rect.h) : Size(rect.w, 0))), a));

		// move past this positioned item
		if (_orientation == Horizontal)
			point.x += s.w;
		else
			point.y += s.h;
	}
}

void BoxLayout::addWidget(Widget *w, Alignment align)
{
	if (parentWidget() && w->parentWidget() != parentWidget())
		w->setParentWidget(parentWidget());
	// TODO set parent of widget 'w' to this->parentWidget()
	WidgetItem * const wi = new WidgetItem(w);
	wi->setParentItem(this);
	wi->setAlignment(align);
	_items.push_back(wi);
}

void BoxLayout::addLayout(Layout *l)
{
	l->setParentItem(this);
	_items.push_back(l);
}

void BoxLayout::addStretch()
{
	SpacerItem * const spacer = new SpacerItem(Size(0, 0), SizePolicy((_orientation == Horizontal) ? SizePolicy::Expanding : SizePolicy::Minimum, (_orientation == Horizontal) ? SizePolicy::Minimum : SizePolicy::Expanding));
	_items.push_back(spacer);
}

void BoxLayout::addSpacing(int size)
{
	SpacerItem * const spacer = new SpacerItem(Size((_orientation == Horizontal) ? size : 0, (_orientation == Horizontal) ? 0 : size), SizePolicy((_orientation == Horizontal) ? SizePolicy::Fixed : SizePolicy::Minimum, (_orientation == Horizontal) ? SizePolicy::Minimum : SizePolicy::Fixed));
	_items.push_back(spacer);
}

Size BoxLayout::minimumSize() const
{
	return _overallMin;
}

Size BoxLayout::maximumSize() const
{
	return _overallMax;
}

Size BoxLayout::sizeHint() const
{
	return _overallHint;
}

void BoxLayout::update()
{
	static int level = 0;
	level++;
// 	for (int i = 1; i <= level; i++)
// 		std::cout << '\t';
	// std::cout << "BoxLayout::update()" << std::endl;
	Orientations expdir = 0;

	Size overallMin;
	Size overallMax(LAYOUTSIZE_MAX, LAYOUTSIZE_MAX);
	Size overallHint;
	for (ItemVector::const_iterator it = _items.begin(); it != _items.end(); ++it)
	{
		if ((*it)->layout())
			(*it)->layout()->update();
		expdir |= (*it)->expandingDirections();
		const Size minS = (*it)->minimumSize();
		const Size maxS = (*it)->maximumSize();
		const Size hntS = (*it)->sizeHint();
		// for (int i = 0; i <= level; i++)
			// std::cout << '\t';
		// const std::type_info &info = typeid(*(*it));
		// std::cout << info.name() << ": ";
		// std::cout << hntS.w << ',' << hntS.h << std::endl;

		if (_orientation == Horizontal)
		{
			overallMin.w += minS.w;
			overallMin.h = std::max(minS.h, overallMin.h);

			overallMax.w += maxS.w;
			overallMax.h = std::min(maxS.h, overallMax.h);

			overallHint.w += hntS.w;
			overallHint.h = std::max(hntS.h, overallHint.h);
		}
		else
		{
			overallMin.w = std::max(minS.w, overallMin.w);
			overallMin.h += minS.h;

			overallMax.w = std::min(maxS.w, overallMax.w);
			overallMax.h += maxS.h;

			overallHint.w = std::max(hntS.w, overallHint.w);
			overallHint.h += hntS.h;
		}
	}
	overallMax = overallMax.expandedTo(overallMin);
	overallHint = overallHint.expandedTo(overallMin);//.boundedTo(overallMax);
	// for (int i = 1; i <= level; i++)
		// std::cout << '\t';
	// std::cout << "overallHint: " << overallHint.w << ',' << overallHint.h << std::endl;

	_overallMin = overallMin;
	_overallMax = overallMax;
	_overallHint = overallHint;
	_expandingDirections = expdir;
	level--;
}

} // namespace Widgets

} // namespace BackyardBrains
