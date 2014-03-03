#ifndef BACKYARDBRAINS_WIDGETS_DROPDOWNLISTPOPUP_H
#define BACKYARDBRAINS_WIDGETS_DROPDOWNLISTPOPUP_H

#include "Widget.h"

#include <string>

namespace BackyardBrains {

namespace Widgets {

class DropDownListPopup : public Widget
{
public:
	DropDownListPopup(Widget *parent = NULL);
	~DropDownListPopup();

	void clear();
	void addItem(const std::string &str);
	void insertItem(int index, const std::string &str);
	std::string item(int index) const;

	sigslot::signal1<int> indexChanged;

	Size sizeHint() const;
private:
	void resizeEvent(ResizeEvent *event);
	void paintEvent();
	// void mousePressEvent(MouseEvent *event);
	int _selectedIndex;
	std::vector<std::string> _entries;
	
	ScrollBar * _scrollbar;
};

} // namespace Widgets

} // namespace BackyardBrains

#endif
