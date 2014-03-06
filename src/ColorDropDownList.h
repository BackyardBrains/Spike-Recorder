#ifndef BACKYARDBRAINS_COLORDROPDOWNLIST_H
#define BACKYARDBRAINS_COLORDROPDOWNLIST_H

#include "widgets/Widget.h"
#include "widgets/Color.h"
#include <vector>
#include <sigslot.h>

namespace BackyardBrains {

namespace Widgets {
	class MouseEvent;
}

class ColorDropDownList : public Widgets::Widget {
public:
	ColorDropDownList(Widget *parent = NULL);

	const std::vector<Widgets::Color> &content() const;
	void setContent(const std::vector<Widgets::Color> &content);

	void setSelection(int selection);
	int selection() const;
	sigslot::signal1<int> selectionChanged;
private:
	int _selection;
	std::vector<Widgets::Color> _content;

	void mousePressEvent(Widgets::MouseEvent *event);
	void paintEvent();
};

}

#endif
