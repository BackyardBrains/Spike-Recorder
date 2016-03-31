#include "TextInput.h"
#include "Painter.h"
#include "widgets/BitmapFontGL.h"
#include "widgets/Application.h"
#include <iostream>

namespace BackyardBrains {

namespace Widgets  {

static const int INITIAL_HEIGHT = 20;
static const int MINIMAL_WIDTH = 20;

TextInput::TextInput(Widget *parent,int initialWidth) : Widget(parent)
{
    //int initialWidth = 200;
    enableFocus();
    _cursor = 0;
    _selectionLength = 0;
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
    Widgets::Painter::setColor(Widgets::Colors::white);
    Widgets::Application::font()->draw(titles.str().c_str(),4, 4, AlignLeft);
}

void TextInput::mousePressEvent(MouseEvent *event)
{

}

void TextInput::textEditingEvent(const SDL_Event &event)
{
                    /*
                    Update the composition text.
                    Update the cursor position.
                    Update the selection length (if any).
                    */
                    _composition = event.edit.text;
                    _cursor = event.edit.start;
                    _selectionLength = event.edit.length;
                    std::cout<<"Cursor:"<<_cursor<<"SelectLen:"<<_selectionLength<<"\n";
}


void TextInput::textInputEvent(const SDL_Event &event)
{
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
            //lop off character
            _text.pop_back();
            _renderText = true;
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
