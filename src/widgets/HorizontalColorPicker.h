#ifndef BACKYARDBRAINS_HORIZONTALCOLORPICKER_H
#define BACKYARDBRAINS_HORIZONTALCOLORPICKER_H

#include "widgets/Widget.h"
#include "widgets/Color.h"
#include <vector>
#include <sigslot.h>

namespace BackyardBrains {
    
    namespace Widgets {
        class MouseEvent;
    }
    
    class HorizontalColorPicker : public Widgets::Widget {
    public:
        HorizontalColorPicker(Widget *parent = NULL);
        
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
    
    class HorizontalColorPickerPopup : public Widgets::Widget {
    public:
        HorizontalColorPickerPopup(const std::vector<Widgets::Color> &content, Widget *parent = NULL);
        

        sigslot::signal1<int> selectionChanged;

    private:
        const std::vector<Widgets::Color> &_content;        
        void paintEvent();
        void mousePressEvent(Widgets::MouseEvent *event);
        void resizeEvent(Widgets::ResizeEvent *event);
    };
    
}

#endif
