#include "TouchDropDownList.h"
#include "widgets/Painter.h"
#include "widgets/Application.h"
#include "widgets/Event.h"
#include <iostream>
#include "widgets/BitmapFontGL.h"
#include "widgets/BoxLayout.h"
#include "widgets/ScrollBar.h"
#include "widgets/TextureGL.h"
#include <SDL_opengl.h>
#include <iostream>
#include <cstdio>
#include "Point.h"

namespace BackyardBrains {
    
    
    static const int BORDER_WIDTH = 2;
    static const int SLICE_W = 12;
    static const int TEXT_PADDING = 10;
    static const int HEIGHT_OF_POPUP = 400;
    static const int HEIGHT_OF_POPUP_LINE = 50;
    static const int WIDTH_OF_POPUP = 400;
    
    TouchDropDownList::TouchDropDownList(Widget *parent, int lineWidth, int lineHeight) : Widget(parent), /*_state2(WAITING_FOR_CLICK_ON_BUTTON),*/ _selectedIndex(0)/*, _highlightedIndex(0), _verticalOffset(0)*/
    {
        _lineWidth = lineWidth;
        _lineHeight = lineHeight;
        _disabled = false;
        char buffer[32];
        for (int i = 0; i < 3; i++)
        {
            snprintf(buffer, sizeof(buffer), "Test %i!", i);
            _entries.push_back(std::string(buffer));
        }
        setSizeHint(Widgets::Size(_lineWidth+2*BORDER_WIDTH, Widgets::Application::font()->characterHeight()+2*TEXT_PADDING + 2*BORDER_WIDTH));
    }
    
    TouchDropDownList::~TouchDropDownList()
    {
    }
    
    // Size DropDownList::sizeHint() const
    // {
    // 	return Size(Application::font()->characterWidth()*24 + 2*BORDER_WIDTH, Application::font()->characterHeight() + 2*BORDER_WIDTH);
    // }
    
    void TouchDropDownList::clear()
    {
        _entries.clear();
        const int oldIndex = _selectedIndex;
        _selectedIndex = 0;
        if (oldIndex != 0)
            indexChanged.emit(0);
    }
    
    void TouchDropDownList::setSelection(int selection) {
        int old = _selectedIndex;
        _selectedIndex = std::max(0, std::min((int)_entries.size()-1,selection));
        if(_selectedIndex != old)
        {
            indexChanged.emit(_selectedIndex);
        }
    }
    
    int TouchDropDownList::selection() const {
        return _selectedIndex;
    }
    
    void TouchDropDownList::addItem(const std::string &str)
    {
        _entries.push_back(str);
    }
    
    void TouchDropDownList::insertItem(unsigned int index, const std::string &str)
    {
        if(index < _entries.size())
            _entries.insert(_entries.begin() + index, str);
    }
    
    std::string TouchDropDownList::item(unsigned int index) const
    {
        if(index < _entries.size())
            return _entries[index];
        return std::string();
    }
    
    void TouchDropDownList::setDisabled(bool newValue)
    {
        _disabled = newValue;
    }
    
    bool TouchDropDownList::disabled()
    {
        return _disabled;
    }
    
    void TouchDropDownList::paintEvent()
    {
        //draw background
        Widgets::Painter::setColor(Widgets::Colors::widgetbg);
        Widgets::Painter::drawRect(rect());
        
        if(!_disabled)
        {
            //draw white background of texxt
            Widgets::Painter::setColor(Widgets::Colors::white);
            //Widgets::Painter::drawRect(Widgets::Rect(2,2,width()-4, height()-4));
            Widgets::Painter::drawRect(Widgets::Rect(2,2,width()-4, height()-4));
        }
        
        const Widgets::BitmapFontGL &font = *Widgets::Application::font();
        
        Widgets::Painter::setColor(Widgets::Colors::gray);
        
        if(_entries.size()==0)
        {
            font.draw("No detected ports", width()/2, TEXT_PADDING+BORDER_WIDTH, Widgets::AlignHCenter);
        }
        else
        {
            font.draw(item(_selectedIndex).c_str(), width()/2, TEXT_PADDING+BORDER_WIDTH, Widgets::AlignHCenter);
            
        }
        
    }
    
    void TouchDropDownList::mousePressEvent(Widgets::MouseEvent *event)
    {
        if(!_disabled)
        {
            if (event->button() == Widgets::LeftButton)
            {
                event->accept();
                if(_entries.size()!=0)
                {
                    openPopup();
                }
                
            }
        }
        else
        {
            event->accept();
        }
    }
    
    void TouchDropDownList::openPopup()
    {
        popup = new TouchDropDownPopup(_entries);
        popup->setOpen(true);
        popup->selectionChanged.connect(this, &TouchDropDownList::setSelection);
        int w, h;
        Widgets::Application::getInstance()->getWindowSize(&w, &h);
        popup->setGeometry(Widgets::Rect(Widgets::Point(w/2-WIDTH_OF_POPUP/2, h/2-HEIGHT_OF_POPUP/2), Widgets::Size(WIDTH_OF_POPUP, HEIGHT_OF_POPUP)));
        
        //popup->setGeometry(Widgets::Rect(mapToGlobal(rect().bottomLeft()), Widgets::Size(width(), 120)));
        Widgets::Application::getInstance()->addPopup(popup);
    }
    
    void TouchDropDownList::resizeEvent(Widgets::ResizeEvent *event)
    {
        /*if(popup)
        {
            if(popup->isOpen())
            {
                popup->close();
                openPopup();
                
            }
        }*/
    }
    
//============================================  Popup implementation ==============================================================
    
    TouchDropDownPopup::TouchDropDownPopup(const std::vector<std::string> &entries, Widget *parent, int lineWidth, int lineHeight)
    : Widget(parent), _entries(entries) {
        
        _lineHeight = lineHeight;
        _lineWidth = WIDTH_OF_POPUP;
        setSizeHint(Widgets::Size(WIDTH_OF_POPUP,height()));

    }
    
    void TouchDropDownPopup::resizeEvent(Widgets::ResizeEvent *event) {
       
    }
    

    
    void TouchDropDownPopup::paintEvent() {
        Widgets::Painter::setColor(Widgets::Colors::widgetbgdark);
        Widgets::Painter::drawRect(rect());
        Widgets::Painter::setColor(Widgets::Colors::gray);
        Widgets::Painter::drawRect(Widgets::Rect(rect().x+BORDER_WIDTH, rect().y+BORDER_WIDTH, rect().width()-2*BORDER_WIDTH, rect().height()-2*BORDER_WIDTH));
        Widgets::Painter::setColor(Widgets::Colors::widgetbgdark);
        Widgets::Painter::drawRect(Widgets::Rect(rect().x+BORDER_WIDTH+2, rect().y+BORDER_WIDTH+2, rect().width()-2*BORDER_WIDTH -4, rect().height()-2*BORDER_WIDTH -4));
        
        const Widgets::BitmapFontGL &font = *Widgets::Application::font();
        unsigned int i;
        int y = BORDER_WIDTH;
        Widgets::Painter::setColor(Widgets::Colors::white);
        for(i = 0;  i < _entries.size(); i++) {
            
           // y += HEIGHT_OF_POPUP_LINE/2-Widgets::Application::font()->characterHeight()/2;

            Widgets::Painter::setColor(Widgets::Colors::white);
            font.draw(_entries[i].c_str(), width()/2, y+HEIGHT_OF_POPUP_LINE/2-Widgets::Application::font()->characterHeight()/2, Widgets::AlignHCenter);
            
            y += HEIGHT_OF_POPUP_LINE;
            Widgets::Painter::setColor(Widgets::Colors::gray);
            Widgets::Painter::drawRect(Widgets::Rect(2*BORDER_WIDTH,y,width()-4*BORDER_WIDTH, 1));
        }
        
    }
    
    void TouchDropDownPopup::mousePressEvent(Widgets::MouseEvent *event) {
        if(event->button() == Widgets::LeftButton && event->pos().x < (width()-SLICE_W)) {
            event->accept();
            int selected = (event->pos().y-BORDER_WIDTH)/HEIGHT_OF_POPUP_LINE;
            selectionChanged.emit(selected);
            _open = false;
            close();
        }
    }
    
    
    TouchDropDownPopup::~TouchDropDownPopup()
    {
        _open = false;
    }
    
    
    
    
    
    
} // namespace BackyardBrains
