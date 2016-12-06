#include "HorizontalColorPicker.h"
#include "widgets/Painter.h"
#include "widgets/Application.h"
#include "widgets/Event.h"
#include "widgets/BoxLayout.h"
#include "widgets/ScrollBar.h"
#include "widgets/TextureGL.h"

namespace BackyardBrains {
    
    static const int SLICE_W = 60;
    static const int SLICE_H = 45;
    static const int padding = 2;
    
    HorizontalColorPicker::HorizontalColorPicker(Widget *parent) : Widget(parent), _selection(0) {
        setSizeHint(Widgets::Size(SLICE_W,SLICE_H));
    }
    
    const std::vector<Widgets::Color> &HorizontalColorPicker::content() const {
        return _content;
    }
    
    void HorizontalColorPicker::setContent(const std::vector<Widgets::Color> &content) {
        _content = content;
    }
    
    void HorizontalColorPicker::setSelection(int selection) {
        int old = _selection;
        setSelectionSilent(selection);
        if(_selection != old)
            selectionChanged.emit(_selection);
    }
    
    void HorizontalColorPicker::setSelectionSilent(int selection) {
        _selection = std::max(0, std::min((int)_content.size()-1,selection));
    }
    
    int HorizontalColorPicker::selection() const {
        return _selection;
    }
    
    void HorizontalColorPicker::paintEvent() {
        Widgets::Painter::setColor(Widgets::Colors::widgetbg);
        Widgets::Painter::drawRect(rect());
        if(_content.size() > 0)
            Widgets::Painter::setColor(_content[_selection]);
        
        Widgets::Painter::drawRect(Widgets::Rect(2,2,SLICE_W-4, height()-4));
        
       
       // glBindTexture(GL_TEXTURE_2D, 0);
        
    }
    
    void HorizontalColorPicker::mousePressEvent(Widgets::MouseEvent *event) {
        if(event->button() == Widgets::LeftButton) {
            event->accept();
            
            HorizontalColorPickerPopup *popup = new HorizontalColorPickerPopup(_content);
            popup->selectionChanged.connect(this, &HorizontalColorPicker::setSelection);
            
            popup->setGeometry(Widgets::Rect(mapToGlobal(rect().topLeft()), Widgets::Size((int)(_content.size()*(SLICE_W+padding) + padding), SLICE_H)));
            Widgets::Application::getInstance()->addPopup(popup);
        }
    }
    
    HorizontalColorPickerPopup::HorizontalColorPickerPopup(const std::vector<Widgets::Color> &content, Widget *parent)
    : Widget(parent), _content(content) {
        setSizeHint(Widgets::Size((int)(_content.size()*(SLICE_W+padding) + padding), SLICE_H));
        
    }
    
    
    
    void HorizontalColorPickerPopup::paintEvent() {
        Widgets::Painter::setColor(Widgets::Colors::widgetbgdark);
        Widgets::Painter::drawRect(rect());
        
        unsigned int i;
        int x = padding;
        for(i = 0; i < _content.size(); i++) {
            Widgets::Painter::setColor(_content[i]);
            
            Widgets::Painter::drawRect(Widgets::Rect(x,2, SLICE_W, SLICE_H-4));
            x += SLICE_W + padding;
        }
        
    }
    
    void HorizontalColorPickerPopup::mousePressEvent(Widgets::MouseEvent *event) {
        if(event->button() == Widgets::LeftButton && event->pos().y<(_content.size()*(SLICE_W +padding)+padding))
        {
            event->accept();
            int selected = (event->pos().x)/(SLICE_W+padding);
            selectionChanged.emit(selected);
            close();
        }
    }
    
    void HorizontalColorPickerPopup::resizeEvent(Widgets::ResizeEvent *event) {
        
    }
    
}
