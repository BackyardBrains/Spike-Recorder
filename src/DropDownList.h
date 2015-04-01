#ifndef BACKYARDBRAINS_WIDGETS_DROPDOWNLIST_H
#define BACKYARDBRAINS_WIDGETS_DROPDOWNLIST_H


#include <vector>
#include <sigslot.h>
#include <string>
#include "widgets/Widget.h"
#include "widgets/Color.h"

namespace BackyardBrains {

namespace Widgets {
    class MouseEvent;
    class ScrollBar;
}
    
class DropDownList : public Widgets::Widget
{


public:
	DropDownList(Widget *parent = NULL, int lineWidth=400, int lineHeight=30);
	~DropDownList();

	void clear();
	void addItem(const std::string &str);
	void insertItem(unsigned int index, const std::string &str);
	std::string item(unsigned int index) const;
    void setSelection(int selection);
    int selection() const;
	sigslot::signal1<int> indexChanged;
    void setDisabled(bool newValue);
    bool disabled();
	Widgets::Size sizeHint() const;
private:
	void paintEvent();
	void mousePressEvent(Widgets::MouseEvent *event);
    bool _disabled;
    int _lineWidth;
    int _lineHeight;
	int _selectedIndex;
	/*int _highlightedIndex;
	int _verticalOffset;
	SDL::Rect _listRect;*/
	std::vector<std::string> _entries;
	// BitmapFontGL font;
};
    
    
class TextDropDownPopup : public Widgets::Widget {
public:
    TextDropDownPopup(const std::vector<std::string> &entries, Widgets::Widget *parent = NULL, int lineWidth=300, int lineHeight=30);
    
    void setScroll(int scroll);
    sigslot::signal1<int> selectionChanged;
    sigslot::signal1<int> scrollChanged;
private:
    const std::vector<std::string> &_entries;
    int _scroll;
    Widgets::ScrollBar *_scrollBar;
    int _lineWidth;
    int _lineHeight;
    
    void paintEvent();
    void mousePressEvent(Widgets::MouseEvent *event);
    void resizeEvent(Widgets::ResizeEvent *event);
};
    


} // namespace BackyardBrains

#endif
