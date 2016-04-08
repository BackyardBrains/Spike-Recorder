#ifndef TEXTINPUT_H
#define TEXTINPUT_H
#include <sstream>
#include "Widget.h"

namespace BackyardBrains {

namespace Widgets {

    class TextInput : public Widget
    {
        public:
            TextInput( Widget *parent = NULL,int initialWidth = 200);
            virtual ~TextInput();
            std::string text(){return _text;}
            bool selected(){return _selected;}
            sigslot::signal1<std::string> textChanged;
        protected:
            bool _selected;
            std::string _text = "";
            std::string _composition = "";
            bool _renderText;
            int _cursor;
            int _selectionLength;
            bool _textSelected;
            uint64_t _cursorTimerMs;
            void paintEvent();
            void mousePressEvent(MouseEvent *event);
            void keyDownEvent(const SDL_Event  &event);
            void textInputEvent(const SDL_Event &event);
            void textEditingEvent(const SDL_Event &event);

            void drawtextbgbox(const std::string &s, int x, int y, Widgets::Alignment a);
            void drawCursor();
        private:
    };

} // namespace Widgets

} // namespace BackyardBrains
#endif // TEXTINPUT_H
