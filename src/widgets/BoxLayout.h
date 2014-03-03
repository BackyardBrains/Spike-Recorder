#ifndef BACKYARDBRAINS_WIDGETS_BOXLAYOUT_H
#define BACKYARDBRAINS_WIDGETS_BOXLAYOUT_H

#include "LayoutItem.h"

#include <vector>

namespace BackyardBrains {

namespace Widgets {

class BoxLayout : public Layout
{
public:
	BoxLayout(Orientation orient, Widget *parent = NULL);
	~BoxLayout();

	Orientations expandingDirections() const;

	void setGeometry(const Rect &rect);

	void addWidget(Widget *w, Alignment align = 0);
	void addLayout(Layout *l);
	void addStretch();
	void addSpacing(int size);

	Size minimumSize() const;
	Size maximumSize() const;
	Size sizeHint() const;

	void update();
private:
	Orientation _orientation;
	typedef std::vector<LayoutItem*> ItemVector;
	std::vector<LayoutItem*> _items;
	Orientations _expandingDirections;
};

} // namespace Widgets

} // namespace BackyardBrains

#endif
