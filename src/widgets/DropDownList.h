#ifndef BACKYARDBRAINS_WIDGETS_DROPDOWNLIST_H
#define BACKYARDBRAINS_WIDGETS_DROPDOWNLIST_H

#include "Widget.h"

#include <string>

namespace BackyardBrains {

namespace Widgets {

class DropDownList : public Widget
{
public:
	DropDownList(Widget *parent = NULL);
	~DropDownList();

	void clear();
	void addItem(const std::string &str);
	void insertItem(unsigned int index, const std::string &str);
	std::string item(unsigned int index) const;

	sigslot::signal1<int> indexChanged;

	Size sizeHint() const;
private:
	void paintEvent();
	void mousePressEvent(MouseEvent *event);
	/*void mouseReleaseEvent(MouseEvent *event);
	void mouseMotionEvent(MouseEvent *event);*/

	/*struct DropDownListState
	{
		DropDownListState()
		{
			memset(this, 0, sizeof(*this));
		}
		bool open : 1;
		bool hovering : 1;
	} _state;
	enum DropDownListState2
	{
		WAITING_FOR_CLICK_ON_BUTTON,
		WAITING_FOR_RELEASE_ON_BUTTON,
		WAITING_FOR_CLICK_ON_LIST,
		WAITING_FOR_RELEASE_ON_LIST
	} _state2;*/
	int _selectedIndex;
	/*int _highlightedIndex;
	int _verticalOffset;
	SDL::Rect _listRect;*/
	std::vector<std::string> _entries;
	// BitmapFontGL font;
};

} // namespace Widgets

} // namespace BackyardBrains

#endif
