#ifndef BACKYARDBRAINS_HORIZONTALNUMBERPICKER_H
#define BACKYARDBRAINS_HORIZONTALNUMBERPICKER_H

#include "widgets/Widget.h"
#include "widgets/Color.h"
#include <vector>
#include <sigslot.h>

namespace BackyardBrains {
    
    namespace Widgets {
        class MouseEvent;
    }
    
    class HorizontalNumberPicker : public Widgets::Widget {
    public:
        HorizontalNumberPicker(Widget *parent = NULL);
        
        void setLimits(int startNumber, int endNumber){_startNumber = startNumber; _endNumber = endNumber;}

        void setSelection(int selection);
        void setSelectionSilent(int selection); // won't emit a signal
        int selection() const;
        sigslot::signal1<int> selectionChanged;
    private:
        int _startNumber;
        int _endNumber;
        int _selection;

        
        void mousePressEvent(Widgets::MouseEvent *event);
        void paintEvent();
    };
    
    class HorizontalNumberPickerPopup : public Widgets::Widget {
    public:
        HorizontalNumberPickerPopup(int startNumber, int endNumber, int selection, Widget *parent = NULL);
        
        
        sigslot::signal1<int> selectionChanged;
        
    private:
        int _startNumber;
        int _endNumber;
        int _selection;
        void paintEvent();
        void mousePressEvent(Widgets::MouseEvent *event);
        void resizeEvent(Widgets::ResizeEvent *event);
    };
    
}

#endif
