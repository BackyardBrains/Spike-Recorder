#ifndef BACKYARDBRAINS_WIDGETS_SWITCHLAYOUT_H
#define BACKYARDBRAINS_WIDGETS_SWITCHLAYOUT_H

#include "LayoutItem.h"

#include <vector>

namespace BackyardBrains {

namespace Widgets {

// SwitchLayout can be used to display one widget of a group of widgets.
class SwitchLayout : public Layout
{
public:
	SwitchLayout(Widget *parent = NULL);
	~SwitchLayout();

	Orientations expandingDirections() const;

	void setGeometry(const Rect &rect);

	void addWidget(Widget *w, Alignment align = 0);
	void addLayout(Layout *l);

	Size minimumSize() const;
	Size maximumSize() const;
	Size sizeHint() const;

	void update();

	void setSelected(int index);
	int selected() const;
private:
	int _selected;
	std::vector<LayoutItem*> _items;
};

} // namespace Widgets

} // namespace BackyardBrains

#endif
