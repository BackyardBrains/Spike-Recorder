#ifndef TEXTINPUT_H
#define TEXTINPUT_H
#include <sstream>
#include "Widget.h"
#include <string>
namespace BackyardBrains {

namespace Widgets {

    class TextInput : public Widget
    {
        public:
            TextInput( Widget *parent = NULL,int initialWidth = 200, int initialHeight = 20);
            virtual ~TextInput();

            bool selected(){return _selected;}

            sigslot::signal1<std::string> textChanged;

            sigslot::signal1<std::string> textEditingEnded;

            std::string text(){return _text;}
            void setText(std::string newText);
            void setFloat(float newFloat);
            void setInt(int newInt);
            int getInt();
            float getFloat();
        protected:
            bool _selected;
            std::string _text = "";
            std::string _composition = "";
            bool _renderText;
            int _cursor;
            int _selectionLength;
            bool _textSelected;

            int _cursorPosition;//position of the cursor in text input (not text)
            int _invisibleOffset;//_invisible offset of the text


            uint64_t _cursorTimerMs;
            void paintEvent();
            void mousePressEvent(MouseEvent *event);
            void keyDownEvent(const SDL_Event  &event);
            void textInputEvent(const SDL_Event &event);
            void textEditingEvent(const SDL_Event &event);

            void drawtextbgbox(const std::string &s, int x, int y, Widgets::Alignment a);
            void drawCursor();
        
            int _initialHeight = 20;
        private:
    };

} // namespace Widgets

} // namespace BackyardBrains
#endif // TEXTINPUT_H
