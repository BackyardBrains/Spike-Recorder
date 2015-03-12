#ifndef BACKYARDBRAINS_COLORDROPDOWNLIST_H
#define BACKYARDBRAINS_COLORDROPDOWNLIST_H

#include "widgets/Widget.h"
#include "widgets/Color.h"
#include <vector>
#include <sigslot.h>

namespace BackyardBrains {

namespace Widgets {
	class MouseEvent;
	class ScrollBar;
}

class ColorDropDownList : public Widgets::Widget {
public:
	ColorDropDownList(Widget *parent = NULL);

	const std::vector<Widgets::Color> &content() const;
	void setContent(const std::vector<Widgets::Color> &content);

	void setSelection(int selection);
	void setSelectionSilent(int selection); // won't emit a signal
	int selection() const;
	sigslot::signal1<int> selectionChanged;
private:
	int _selection;
	std::vector<Widgets::Color> _content;

	void mousePressEvent(Widgets::MouseEvent *event);
	void paintEvent();
};

class ColorDropDownPopup : public Widgets::Widget {
public:
	ColorDropDownPopup(const std::vector<Widgets::Color> &content, Widget *parent = NULL);

	void setScroll(int scroll);
	sigslot::signal1<int> selectionChanged;
	sigslot::signal1<int> scrollChanged;
private:
	const std::vector<Widgets::Color> &_content;
	int _scroll;
	Widgets::ScrollBar *_scrollBar;

	void paintEvent();
	void mousePressEvent(Widgets::MouseEvent *event);
	void resizeEvent(Widgets::ResizeEvent *event);
};

}

#endif
