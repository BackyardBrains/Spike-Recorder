#include "DropDownList.h"
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

namespace BackyardBrains {


static const int BORDER_WIDTH = 2;
static const int SLICE_W = 12;
    
DropDownList::DropDownList(Widget *parent, int lineWidth, int lineHeight) : Widget(parent), /*_state2(WAITING_FOR_CLICK_ON_BUTTON),*/ _selectedIndex(0)/*, _highlightedIndex(0), _verticalOffset(0)*/
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
	setSizeHint(Widgets::Size(_lineWidth+2*BORDER_WIDTH, Widgets::Application::font()->characterHeight()+6 + 2*BORDER_WIDTH));
}

DropDownList::~DropDownList()
{
}

// Size DropDownList::sizeHint() const
// {
// 	return Size(Application::font()->characterWidth()*24 + 2*BORDER_WIDTH, Application::font()->characterHeight() + 2*BORDER_WIDTH);
// }

void DropDownList::clear()
{
	_entries.clear();
	const int oldIndex = _selectedIndex;
	_selectedIndex = 0;
	if (oldIndex != 0)
		indexChanged.emit(0);
}
    
void DropDownList::setSelection(int selection) {
    int old = _selectedIndex;
    _selectedIndex = std::max(0, std::min((int)_entries.size()-1,selection));
    if(_selectedIndex != old)
    {
        indexChanged.emit(_selectedIndex);
    }
}

int DropDownList::selection() const {
    return _selectedIndex;
}

void DropDownList::addItem(const std::string &str)
{
	_entries.push_back(str);
}

void DropDownList::insertItem(unsigned int index, const std::string &str)
{
	if(index < _entries.size())
		_entries.insert(_entries.begin() + index, str);
}

std::string DropDownList::item(unsigned int index) const
{
	if(index < _entries.size())
		return _entries[index];
	return std::string();
}
    
void DropDownList::setDisabled(bool newValue)
{
    _disabled = newValue;
}

bool DropDownList::disabled()
{
    return _disabled;
}

void DropDownList::paintEvent()
{
    //draw background
    Widgets::Painter::setColor(Widgets::Colors::widgetbg);
    Widgets::Painter::drawRect(rect());
    //draw white background of texxt
    Widgets::Painter::setColor(Widgets::Colors::white);
    //Widgets::Painter::drawRect(Widgets::Rect(2,2,width()-4, height()-4));
    Widgets::Painter::drawRect(Widgets::Rect(2,2,width()-SLICE_W-4, height()-4));
    if(!_disabled)
    {
 

        //Draw drop down arrow
        Widgets::TextureGL::get("data/dropdown.png")->bind();
        Widgets::Painter::setColor(Widgets::Colors::button);
        Widgets::Painter::drawTexRect(Widgets::Rect(width()-SLICE_W+2,height()/2-3,8, 6));
        glBindTexture(GL_TEXTURE_2D, 0);
    
    }
    const Widgets::BitmapFontGL &font = *Widgets::Application::font();
    Widgets::Painter::setColor(Widgets::Colors::gray);
    if(_entries.size()==0)
    {
        font.draw("No detected ports", 5, 5);
    }
    else
    {
        font.draw(item(_selectedIndex).c_str(), 5, 5);
    
    }
    
}

void DropDownList::mousePressEvent(Widgets::MouseEvent *event)
{
    if(!_disabled)
    {
        if (event->button() == Widgets::LeftButton)
        {
            event->accept();
            if(_entries.size()!=0)
            {
                TextDropDownPopup *popup = new TextDropDownPopup(_entries);
                popup->selectionChanged.connect(this, &DropDownList::setSelection);
                popup->setGeometry(Widgets::Rect(mapToGlobal(rect().bottomLeft()), Widgets::Size(width(), 120)));
                Widgets::Application::getInstance()->addPopup(popup);
            }
        
        } else if(event->button() == Widgets::WheelUpButton) {
            setSelection(_selectedIndex-1);
            event->accept();
        } else if(event->button() == Widgets::WheelDownButton) {
            setSelection(_selectedIndex+1);
            event->accept();
        }
    }
    else
    {
        event->accept();
    }
}
    
TextDropDownPopup::TextDropDownPopup(const std::vector<std::string> &entries, Widget *parent, int lineWidth, int lineHeight)
: Widget(parent), _entries(entries), _scroll(0) {
    
    _lineHeight = lineHeight;
    _lineWidth = lineWidth;
    setSizeHint(Widgets::Size(_lineWidth+15,_lineHeight*4));
    
    _scrollBar = new Widgets::ScrollBar(Widgets::Vertical, this);
    _scrollBar->valueChanged.connect(this, &TextDropDownPopup::setScroll);
    scrollChanged.connect(_scrollBar, &Widgets::ScrollBar::updateValue);
    _scrollBar->setGeometry(Widgets::Rect(_lineWidth,0,SLICE_W,height()));
    _scrollBar->updateValue(_scroll);
    _scrollBar->setPageStep(10);
}
    
void TextDropDownPopup::resizeEvent(Widgets::ResizeEvent *event) {
    _scrollBar->setGeometry(Widgets::Rect(width()-SLICE_W -2,0,SLICE_W+2,height()));
    _scrollBar->setRange(0, std::max(0,(int)(_entries.size())*_lineHeight-height()));
}
    
void TextDropDownPopup::setScroll(int scroll) {
    _scroll = std::max(0,std::min((int)(_entries.size())*_lineHeight-height(), scroll));
    scrollChanged.emit(_scroll);
}
    
void TextDropDownPopup::paintEvent() {
    Widgets::Painter::setColor(Widgets::Colors::widgetbgdark);
    Widgets::Painter::drawRect(rect());
    const Widgets::BitmapFontGL &font = *Widgets::Application::font();
    unsigned int i;
    int start = _scroll/_lineHeight;
    int startoff = _scroll%_lineHeight;
   // std::cout<<"Scroll: "<<_scroll<<" startoff: "<<startoff<<"\n";
    Widgets::Painter::setColor(Widgets::Colors::white);
    for(i = 0; i <= (unsigned int)height()/_lineHeight+1 && i+start < _entries.size(); i++) {
        int y = i*_lineHeight-startoff;
        int h = std::min(height()-y, _lineHeight);
        if(y < 0)
            h += y;
        
        //y = std::max(0, y);
        
        if(!(y+2+font.characterHeight()>height() || y<0))
        {
            font.draw(_entries[i+start].c_str(), 5, y+2);
        }
        
    }
    
}
    
void TextDropDownPopup::mousePressEvent(Widgets::MouseEvent *event) {
    if(event->button() == Widgets::LeftButton && event->pos().x < (width()-SLICE_W)) {
        event->accept();
        int selected = (event->pos().y+_scroll)/_lineHeight;
        selectionChanged.emit(selected);
        close();
    } else if(event->button() == Widgets::WheelUpButton) {
        setScroll(_scroll-10);
        event->accept();
    } else if(event->button() == Widgets::WheelDownButton) {
        setScroll(_scroll+10);
        event->accept();
    }
}
    

    
    
    
    
    


} // namespace BackyardBrains
