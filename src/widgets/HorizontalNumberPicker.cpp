#include "HorizontalNumberPicker.h"
#include "widgets/Painter.h"
#include "widgets/Application.h"
#include "widgets/BitmapFontGL.h"
#include "widgets/Event.h"
#include "widgets/BoxLayout.h"
#include "widgets/ScrollBar.h"
#include "widgets/TextureGL.h"
#include <sstream>

namespace BackyardBrains {
    
    static const int SLICE_W = 60;
    static const int SLICE_H = 45;
    static const int padding = 2;
    
    HorizontalNumberPicker::HorizontalNumberPicker(Widget *parent) : Widget(parent), _selection(0) {
        setSizeHint(Widgets::Size(SLICE_W,SLICE_H));
    }
    

    
    void HorizontalNumberPicker::setSelection(int selection) {
        int old = _selection;
        setSelectionSilent(selection);
        if(_selection != old)
            selectionChanged.emit(_selection);
    }
    
    void HorizontalNumberPicker::setSelectionSilent(int selection) {
        _selection = std::max(0, std::min(_endNumber,selection));
    }
    
    int HorizontalNumberPicker::selection() const {
        return _selection;
    }
    
    void HorizontalNumberPicker::paintEvent() {
        Widgets::Painter::setColor(Widgets::Colors::widgetbg);
        Widgets::Painter::drawRect(rect());
        Widgets::Painter::setColor(Widgets::Colors::white);
        Widgets::Painter::drawRect(Widgets::Rect(rect().x+padding,rect().y+padding,rect().width()-2*padding,height()-2*padding));
       
        
        const Widgets::BitmapFontGL &font = *Widgets::Application::font();
        
        Widgets::Painter::setColor(Widgets::Colors::gray);
        
        std::stringstream s;
        s << _selection;
        font.draw(s.str().c_str(), width()/2, height()/2-Widgets::Application::font()->characterHeight()/2, Widgets::AlignHCenter);
    }
    
    void HorizontalNumberPicker::mousePressEvent(Widgets::MouseEvent *event) {
        if(event->button() == Widgets::LeftButton) {
            event->accept();
            
            HorizontalNumberPickerPopup *popup = new HorizontalNumberPickerPopup(_startNumber, _endNumber, _selection);
            popup->selectionChanged.connect(this, &HorizontalNumberPicker::setSelection);
            
            popup->setGeometry(Widgets::Rect(mapToGlobal(rect().topLeft()), Widgets::Size((int)((_endNumber-_startNumber)+1)*SLICE_W, SLICE_H)));
            Widgets::Application::getInstance()->addPopup(popup);
        }
    }
    
    HorizontalNumberPickerPopup::HorizontalNumberPickerPopup(int startNumber, int endNumber, int selection, Widget *parent)
    : Widget(parent) {
        _selection = selection;
        _endNumber = endNumber;
        _startNumber = startNumber;
        setSizeHint(Widgets::Size((int)((_endNumber-_startNumber)+1)*SLICE_W, SLICE_H));
        
    }
    
    
    
    void HorizontalNumberPickerPopup::paintEvent() {
        
        
        int i;
        int x = 0;
        for(i = _startNumber; i <= _endNumber; i++) {
            Widgets::Painter::setColor(Widgets::Colors::white);
            Widgets::Painter::drawRect(Widgets::Rect(x,0, SLICE_W, SLICE_H));

            const Widgets::BitmapFontGL &font = *Widgets::Application::font();
            if(i!=_selection)
            {
                Widgets::Painter::setColor(Widgets::Colors::widgetbg);
                Widgets::Painter::drawRect(Widgets::Rect(x+padding,padding, SLICE_W-2*padding, SLICE_H-2*padding));
                Widgets::Painter::setColor(Widgets::Colors::selectedstate);
            }
            else
            {
                Widgets::Painter::setColor(Widgets::Colors::widgetbg);
            }
            
            std::stringstream s;
            s << i;
            font.draw(s.str().c_str(), x+ SLICE_W/2, height()/2-Widgets::Application::font()->characterHeight()/2, Widgets::AlignHCenter);
            
            x += SLICE_W ;
        }
        
    }
    
    void HorizontalNumberPickerPopup::mousePressEvent(Widgets::MouseEvent *event) {
        if(event->button() == Widgets::LeftButton && event->pos().y<(_endNumber-_startNumber+1)*SLICE_W)
        {
            event->accept();
            int selected = (event->pos().x)/SLICE_W;
            selected+=_startNumber;
            selectionChanged.emit(selected);
            close();
        }
    }
    
    void HorizontalNumberPickerPopup::resizeEvent(Widgets::ResizeEvent *event) {
        
    }
    
}
