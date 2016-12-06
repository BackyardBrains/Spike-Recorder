#ifndef BACKYARDBRAINS_WIDGETS_TOUCHDROPDOWNLIST_H
#define BACKYARDBRAINS_WIDGETS_TOUCHDROPDOWNLIST_H


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
    class TouchDropDownPopup;
    class TouchDropDownList : public Widgets::Widget
    {
        
        
    public:
        TouchDropDownList(Widget *parent = NULL, int lineWidth=400, int lineHeight=50);
        ~TouchDropDownList();
        
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
        void openPopup();
        void mousePressEvent(Widgets::MouseEvent *event);
        bool _disabled;
        int _lineWidth;
        int _lineHeight;
        int _selectedIndex;
        /*int _highlightedIndex;
         int _verticalOffset;
         SDL::Rect _listRect;*/
        std::vector<std::string> _entries;
        TouchDropDownPopup *popup;
        // BitmapFontGL font;
        void resizeEvent(Widgets::ResizeEvent *event);
    };
    
    
    class TouchDropDownPopup : public Widgets::Widget {
    public:
        TouchDropDownPopup(const std::vector<std::string> &entries, Widgets::Widget *parent = NULL, int lineWidth=300, int lineHeight=30);
        ~TouchDropDownPopup();
        void setScroll(int scroll);
        sigslot::signal1<int> selectionChanged;
        void setOpen(bool newOpen){_open=newOpen;}
        bool isOpen(){return _open;}
    private:
        const std::vector<std::string> &_entries;
        int _lineWidth;
        int _lineHeight;
        bool _open;
        void paintEvent();
        void mousePressEvent(Widgets::MouseEvent *event);
        void resizeEvent(Widgets::ResizeEvent *event);
    };
    
    
    
} // namespace BackyardBrains

#endif
