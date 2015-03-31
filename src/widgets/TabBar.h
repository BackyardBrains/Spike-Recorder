#ifndef BACKYARDBRAINS_WIDGETS_TABBAR_H
#define BACKYARDBRAINS_WIDGETS_TABBAR_H

#include "Widget.h"

namespace BackyardBrains {

namespace Widgets {

class TextureGL;

class TabBar : public Widget
{
public:
	TabBar(Widget *parent = NULL);

	sigslot::signal1<int> selectionChanged;
	void setEntries(const std::vector<std::string> &entries);
	int selected() const;
private:
	static const int PADDING = 5;
	void paintEvent();
	void mousePressEvent(MouseEvent *event);

	std::vector<std::string> _entries;
	
	int _selected;
};

} // namespace Widgets

} // namespace BackyardBrains

#endif
