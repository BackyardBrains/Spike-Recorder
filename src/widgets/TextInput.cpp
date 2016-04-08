#include "TextInput.h"
#include "Painter.h"
#include "widgets/BitmapFontGL.h"
#include "widgets/Application.h"
#include <iostream>

namespace BackyardBrains {

namespace Widgets  {

static const int INITIAL_HEIGHT = 20;
static const int MINIMAL_WIDTH = 20;
static const uint64_t CURSOR_TIMER_PERIOD_MS = 700;

TextInput::TextInput(Widget *parent,int initialWidth) : Widget(parent)
{
    //int initialWidth = 200;
    enableFocus();
    _cursor = 0;
    _textSelected = false;
    _selectionLength = 0;
    _cursorTimerMs = SDL_GetTicks();
    _renderText = false;
     setSizePolicy(SizePolicy(SizePolicy::Expanding, SizePolicy::Fixed));
    if(initialWidth<MINIMAL_WIDTH)
    {
        initialWidth = MINIMAL_WIDTH;
    }
    setSizeHint(Size(initialWidth, INITIAL_HEIGHT));
}

TextInput::~TextInput()
{

}


void TextInput::paintEvent()
{
    const Rect backRect = Rect(0,0, width(),height());
    Painter::setColor(Colors::buttonhigh);
    Painter::drawRect(backRect);
    const Rect inerRect = Rect(1,1, width()-2,height()-2);
    if(hasFocus())
    {
        Painter::setColor(Colors::widgetbg);

    }
    else
    {
        Painter::setColor(Colors::widgetbgdark);
    }
    Painter::drawRect(inerRect);

    std::stringstream titles;
    titles <<_text;
    if(_textSelected && hasFocus())
    {
        Widgets::Painter::setColor(Widgets::Colors::selectedstate);
		drawtextbgbox(titles.str(), 4, 4, Widgets::AlignLeft);
		Widgets::Painter::setColor(Widgets::Colors::black);
    }
    else
    {
        Widgets::Painter::setColor(Widgets::Colors::white);
    }

    Widgets::Application::font()->draw(titles.str().c_str(),4, 4, AlignLeft);

    if(hasFocus())
    {
        drawCursor();
    }
}


void TextInput::drawCursor()
{
    const int w = _text.size()*Widgets::Application::font()->characterWidth();
    if( SDL_GetTicks()-_cursorTimerMs<CURSOR_TIMER_PERIOD_MS)
    {
        Widgets::Painter::setColor(Widgets::Colors::white);
        Widgets::Painter::drawRect(Widgets::Rect(w+4, 4, 1, Widgets::Application::font()->characterHeight()));
    }
     if( SDL_GetTicks()-_cursorTimerMs>2*CURSOR_TIMER_PERIOD_MS)
     {
         _cursorTimerMs = SDL_GetTicks();
     }
}

void TextInput::drawtextbgbox(const std::string &s, int x, int y, Widgets::Alignment a) {
	const int pad = 0;
	const int w = s.size()*Widgets::Application::font()->characterWidth();
	const int h = Widgets::Application::font()->characterHeight();

	int rx = x-pad;
	int ry = y-pad;

	if(a & Widgets::AlignRight)
		rx -= w;
	if(a & Widgets::AlignBottom)
		ry -= h;
	if(a & Widgets::AlignHCenter)
		rx -= w/2;
	if(a & Widgets::AlignVCenter)
		ry -= h/2;
    if(s.size()>0)
    {
        Widgets::Painter::drawRect(Widgets::Rect(rx, ry-1, w+2*pad, h+2*pad-1));
    }
}



void TextInput::mousePressEvent(MouseEvent *event)
{
    _textSelected = true;
}

void TextInput::textEditingEvent(const SDL_Event &event)
{
      //This does not work as it should be
        _composition = event.edit.text;
        _cursor = event.edit.start;
        _selectionLength = event.edit.length;
        std::cout<<"Cursor:"<<_cursor<<"SelectLen:"<<_selectionLength<<"\n";
}


void TextInput::textInputEvent(const SDL_Event &event)
{
        _cursorTimerMs = SDL_GetTicks();
        if(_textSelected)
        {
            _textSelected = false;
            _text = "";

        }
        //Not copy or pasting
        if( !( ( event.text.text[ 0 ] == 'c' || event.text.text[ 0 ] == 'C' ) && ( event.text.text[ 0 ] == 'v' || event.text.text[ 0 ] == 'V' ) && SDL_GetModState() & KMOD_CTRL ) )
        {
            //Append character
            _text += event.text.text;
            _renderText = true;
        }
}

void TextInput::keyDownEvent(const SDL_Event  &event)
{

        //Handle backspace

        if( event.key.keysym.sym == SDLK_BACKSPACE && _text.length() > 0 )
        {
            _renderText = true;
            if(_textSelected)
            {
                _text = "";
            }
            else
            {
                _text.pop_back();
            }
        }
        //Handle copy
        else if( event.key.keysym.sym == SDLK_c && SDL_GetModState() & KMOD_CTRL )
        {
            SDL_SetClipboardText( _text.c_str() );
        }
        //Handle paste
        else if( event.key.keysym.sym == SDLK_v && SDL_GetModState() & KMOD_CTRL )
        {
            _text = SDL_GetClipboardText();
            _renderText = true;
        }
}


} // namespace Widgets

} // namespace BackyardBrains
