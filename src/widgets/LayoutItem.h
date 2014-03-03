#ifndef BACKYARDBRAINS_WIDGETS_LAYOUTITEM_H
#define BACKYARDBRAINS_WIDGETS_LAYOUTITEM_H

#include "global.h"
#include "Rect.h"
#include "Size.h"
#include "SizePolicy.h"

#include <climits>

namespace BackyardBrains {

namespace Widgets {

class Layout;
class Widget;
class SpacerItem;

static const int LAYOUTSIZE_MAX = INT_MAX/256/16;

class LayoutItem
{
public:
	LayoutItem(Alignment align = 0);
	virtual ~LayoutItem();

	void setParentItem(LayoutItem *newParent); // TODO make this private

	Alignment alignment() const;
	void setAlignment(Alignment align);

	virtual Orientations expandingDirections() const = 0;

	virtual void setGeometry(const Rect &rect) = 0;
	virtual Rect geometry() const = 0;

	virtual Size minimumSize() const = 0;
	virtual Size maximumSize() const = 0;
	virtual Size sizeHint() const = 0;

	// virtual bool isEmpty() const = 0;

	virtual Layout * layout();
	virtual Widget * widget();
	virtual SpacerItem * spacerItem();
private:
	Alignment _alignment;
protected:
	LayoutItem *_parentItem;
};

class WidgetItem : public LayoutItem
{
public:
	WidgetItem(Widget *w);

	Orientations expandingDirections() const;

	void setGeometry(const Rect &rect);
	Rect geometry() const;

	Size minimumSize() const;
	Size maximumSize() const;
	Size sizeHint() const;

	Widget * widget();
private:
	Widget *_widget;
};

class SpacerItem : public LayoutItem
{
public:
	SpacerItem(const Size &s, const SizePolicy &sp = SizePolicy(SizePolicy::Minimum, SizePolicy::Minimum));

	Orientations expandingDirections() const;

	void setGeometry(const Rect &rect);
	Rect geometry() const;

	Size minimumSize() const;
	Size maximumSize() const;
	Size sizeHint() const;

	virtual SpacerItem * spacerItem();
private:
	Size _size;
	SizePolicy _sizePolicy;
	Rect _geometry;
};

class Layout : public LayoutItem
{
public:
	Layout(Widget *parent = NULL);
	// Margins contentsMargins() const;
	// void setContentsMargins(const Margins &margins);

	// int spacing() const;
	// void setSpacing(int newSpacing);

	Widget * parentWidget() const;
	void setParentWidget(Widget *newParent); // TODO make private and a friend Widget

	Orientations expandingDirections() const;

	void setGeometry(const Rect &rect);
	Rect geometry() const;

	Size minimumSize() const;
	Size maximumSize() const;

	// sizeConstraint()
	// setSizeConstraint()

	Layout * layout();

	// bool activate();
	virtual void update() = 0;
protected:
	static Rect _AlignRectInsideArea(const Rect &rect, const Rect &area, Alignment alignment);
	Rect _geometry;
	Widget *_parentWidget;
	Size _overallMin;
	Size _overallMax;
	Size _overallHint;
};

} // namespace Widgets

} // namespace BackyardBrains

#endif
