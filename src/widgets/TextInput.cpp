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
    _invisibleOffset = 0;
    _cursorPosition = 0;
     setSizePolicy(SizePolicy(SizePolicy::Fixed, SizePolicy::Fixed));
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




    int sizeOfFont = Widgets::Application::font()->characterWidth();
    int maxNumberOfCharacters = width()/sizeOfFont;

    int lengthToShow = _text.size()-_invisibleOffset;
    if(lengthToShow>maxNumberOfCharacters-1)
    {
        lengthToShow = maxNumberOfCharacters-1;
    }
    const std::string tmp = _text.substr (_invisibleOffset, lengthToShow);
    const char* cstr = tmp.c_str();

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

    if(lengthToShow>0)
    {

        Widgets::Application::font()->draw(cstr,4, 4, AlignLeft);
    }

    if(hasFocus())
    {
        drawCursor();
    }
}


void TextInput::drawCursor()
{
    int sizeOfFont = Widgets::Application::font()->characterWidth();
    const int w = _cursorPosition*sizeOfFont;
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
	int w = s.size()*Widgets::Application::font()->characterWidth();
	if(w>width())
    {
        w = width()-10;
    }
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
            _invisibleOffset = 0;
            _cursorPosition = 0;


        }
        //Not copy or pasting
        if( !( ( event.text.text[ 0 ] == 'c' || event.text.text[ 0 ] == 'C' ) && ( event.text.text[ 0 ] == 'v' || event.text.text[ 0 ] == 'V' ) && SDL_GetModState() & KMOD_CTRL ) )
        {
            //Append character
            _text.insert(_invisibleOffset+_cursorPosition, event.text.text);
            _renderText = true;
            textChanged.emit(_text);
        }
        //calculate if text will go out of the component
        int sizeOfFont = Widgets::Application::font()->characterWidth();
        int w = (_cursorPosition+1)*sizeOfFont;// _text.size()*sizeOfFont - _invisibleOffset*sizeOfFont;
        int overshoot = w-(width()-5);
        if(overshoot>0)
        {
          _invisibleOffset+= overshoot/sizeOfFont +1;
        }
        else
        {
            _cursorPosition++;
        }

}

void TextInput::keyDownEvent(const SDL_Event  &event)
{
        int sizeOfFont = Widgets::Application::font()->characterWidth();
        int numberOfCharacters = width()/sizeOfFont;
        //Handle backspace

        if(event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDL_SCANCODE_KP_ENTER || event.key.keysym.sym == SDLK_RETURN2 || event.key.keysym.sym == SDLK_KP_ENTER)
        {
            textEditingEnded.emit(_text);
            _textSelected = true;
        }

        if( event.key.keysym.sym == SDLK_BACKSPACE && _text.length() > 0 )
        {
            _renderText = true;
            if(_textSelected)
            {
                _text = "";
                _textSelected = false;
                _cursorPosition = 0;
                _invisibleOffset = 0;
            }
            else
            {
                if(_invisibleOffset+_cursorPosition>0)
                {
                    _text.erase(_invisibleOffset+_cursorPosition-1,1 );
                }
                if(_invisibleOffset>0)
                {
                    _invisibleOffset--;
                }
                else
                {
                    _cursorPosition--;
                    if(_cursorPosition<0)
                    {
                        _cursorPosition = 0;
                    }
                }
            }
            textChanged.emit(_text);
        }

        if(event.key.keysym.sym ==    SDLK_RIGHT )
        {

            _textSelected = false;
            if((_text.size()-_invisibleOffset)>_cursorPosition)
            {
                if(_cursorPosition>=(numberOfCharacters-1))
                {
                    _invisibleOffset++;
                }
                else
                {
                    _cursorPosition++;
                }
            }
            _cursorTimerMs = SDL_GetTicks();
        }
        if(event.key.keysym.sym ==    SDLK_LEFT )
        {
            _textSelected = false;
            if(_cursorPosition==0)
            {
                if(_invisibleOffset>0)
                {
                    _invisibleOffset --;
                }
            }
            else
            {
                _cursorPosition--;
            }
            _cursorTimerMs = SDL_GetTicks();
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
            _cursorPosition = 0;
            _renderText = true;
            textChanged.emit(_text);
        }
}

int TextInput::getInt()
{
    int integerValue = 0;
        //pointer to end of number string after conversation
      std::string::size_type sz; // alias of size_t
      try{
            std::stringstream ss;
            ss << _text;
            ss >> integerValue;
            //float number = std::stof (_text,&sz);
            //integerValue = (int) number;
            std::stringstream titles;
            titles <<integerValue;
            _text = titles.str();
      }
      catch( ... )
      {
            _text = "";
      }
      _cursorPosition = 0;
      _invisibleOffset = 0;
      return integerValue;

}


float TextInput::getFloat()
{
    int floatValue = 0.0f;
        //pointer to end of number string after conversation
      std::string::size_type sz; // alias of size_t
      try{
          std::stringstream ss;
            ss << _text;
            ss >> floatValue;
            //floatValue = std::stof (_text,&sz);
      }
      catch( ... )
      {
            _text = "";
      }
      return floatValue;
}


void TextInput::setText(std::string newText)
{
    _text = newText;
    _cursorPosition = 0;
    _invisibleOffset = 0;
    _selected = false;

}
void TextInput::setFloat(float newFloat)
{
        std::stringstream titles;
        titles <<newFloat;
        _text = titles.str();
        _cursorPosition = 0;
        _invisibleOffset = 0;
        _selected = false;
}
void TextInput::setInt(int newInt)
{
        std::stringstream titles;
        titles <<newInt;
        _text = titles.str();
        _cursorPosition = 0;
        _invisibleOffset = 0;
        _selected = false;
}


} // namespace Widgets

} // namespace BackyardBrains
