#include "Application.h"
#include "TextureGL.h"
#include "LayoutItem.h"
#include "BitmapFontGL.h"
#include "Color.h"
#include "Log.h"
#include <SDL.h>
#include <SDL_opengl.h>
#include <bass.h>
#include <cassert>
#include <iostream>

namespace BackyardBrains {

namespace Widgets {

BitmapFontGL *Application::_font = NULL;
Application *Application::app = NULL;
const int Application::MIN_WINDOW_W = 120;
const int Application::MIN_WINDOW_H = 100;

Application::Application() : _running(false), _mouseGrabber(0), _keyboardGrabber(0), _hoverWidget(0), _windowStack(0), _popupStack(0) {
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		Log::fatal("SDL failed to initialize: %s", SDL_GetError());
	}
    _widgetInFocus = NULL;
#if defined(_WIN32) && (_WIN32_WINNT >= 0x0501)
	// FILE *ctt = fopen("CON", "w" );
	AttachConsole(ATTACH_PARENT_PROCESS);
	freopen("CON", "w", stdout);
	freopen("CON", "w", stderr);
	printf("\r\n");
#endif

	if (!_font) {
		Widgets::TextureGL::load("data/ascii.bmp");
		_font = new BitmapFontGL;
	}

	if(!app) {
		app = this;
	} else {
		Log::fatal("only one Application instance can exist.");
	}

}

Application *Application::getInstance() {
	return app;
}

Application::~Application() {
	SDL_GL_DeleteContext(sdlGLContext);
	SDL_DestroyWindow(sdlWindow);
	SDL_Quit();
	Log::msg("GUI closed.");
}

static MouseButton ToMouseButtonFromSDL(Uint8 button) {
	switch (button) {
		case SDL_BUTTON_LEFT: return LeftButton;
		case SDL_BUTTON_MIDDLE: return MiddleButton;
		case SDL_BUTTON_RIGHT: return RightButton;
	}
	return NoButton;
}

static MouseButtons ToMouseButtonsFromSDL(Uint8 state) {
	MouseButtons result = 0;
	if (state & SDL_BUTTON_LMASK)
		result |= LeftButton;
	if (state & SDL_BUTTON_RMASK)
		result |= RightButton;
	if (state & SDL_BUTTON_MMASK)
		result |= MiddleButton;
	return result;
}

static bool close_cond(Widget *w) {
	return w->closed();
}

void Application::removeClosed(WidgetList &w) {
	WidgetList tmp;
	for(WidgetList::const_iterator it = w.begin(); it != w.end(); ++it)
		if((*it)->closed())
			tmp.push_back(*it);
	w.remove_if(close_cond);
	for(WidgetList::const_iterator it = tmp.begin(); it != tmp.end(); ++it)
		if((*it)->getDeleteOnClose())
			delete *it;
}

void Application::run() {
	_running = true;
	Point oldPos;
	_buttonState = ToMouseButtonsFromSDL(SDL_GetMouseState(&oldPos.x, &oldPos.y));
	SDL_StartTextInput();
	while (_running && !_windowStack.empty()) {

		// Delete closed windows/popups
		if(_hoverWidget && _hoverWidget->closed())
			_hoverWidget = NULL;
		if(_keyboardGrabber && _keyboardGrabber->closed())
			_keyboardGrabber = NULL;
		if(_mouseGrabber && _mouseGrabber->closed())
			_mouseGrabber = NULL;

		removeClosed(_windowStack);
		removeClosed(_popupStack);

		advance();
		// Call step functions of widgets that actually need them. Are there any?
        //Answer: Audio view, for example, is using this.
		for (WidgetList::const_iterator it = _windowStack.begin(); it != _windowStack.end(); ++it)
			(*it)->_CallAdvance();
		for (WidgetList::const_iterator it = _popupStack.begin(); it != _popupStack.end(); ++it)
			(*it)->_CallAdvance();

		// draw
		int w, h;
		SDL_GetWindowSize(sdlWindow,&w,&h);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0, w, h, 0, -100,100);

		glClear(GL_COLOR_BUFFER_BIT);

		for (WidgetList::const_iterator it = _windowStack.begin(); it != _windowStack.end(); ++it) {
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			(*it)->_DoPaintEvents(Point(), (*it)->geometry());
		}
		for (WidgetList::const_iterator it = _popupStack.begin(); it != _popupStack.end(); ++it) {
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			(*it)->_DoPaintEvents(Point(), (*it)->geometry());
		}
		SDL_GL_SwapWindow(sdlWindow);

		// free up the CPU a bit
		SDL_Delay(4);

		// process events
		SDL_Event event;
		while (_running && SDL_PollEvent(&event)) {
			_HandleEvent(&event);
		}
	}
	_running = false;
}

void Application::addPopup(Widget *w) {
	w->unclose();
	_popupStack.push_back(w);
}

void Application::addWindow(Widget *w) {
	w->unclose();
	_windowStack.push_back(w);
}

void Application::updateLayout() {
	for(WidgetList::iterator it = _windowStack.begin(); it != _windowStack.end(); it++) {
		(*it)->layout()->update();
		(*it)->setSize((*it)->size());
	}
}

KeyCode sdl_keysym_to_key(SDL_Keycode key) {
	return (KeyCode)key;
}

KeyModifiers sdl_keymod_to_keymod(unsigned short mod) {
	return mod;
}

void Application::_HandleEvent(const void *eventRaw) {
	const SDL_Event &event = *reinterpret_cast<const SDL_Event*>(eventRaw);

	if (event.type == SDL_MOUSEMOTION || event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP || event.type == SDL_MOUSEWHEEL) {
		// update our internal button state tracking
		if (event.type == SDL_MOUSEMOTION)
			_buttonState = ToMouseButtonsFromSDL(event.motion.state);
		else if (event.type == SDL_MOUSEBUTTONDOWN)
			_buttonState |= ToMouseButtonFromSDL(event.button.button);
		else if (event.type == SDL_MOUSEWHEEL) {
			/*if(event.wheel.y > 0)
				_buttonState |= WheelUpButton;
			if(event.wheel.y < 0)
				_buttonState |= WheelDownButton;*/
		}
		else // if (event.type == SDL_MOUSEBUTTONUP)

			_buttonState &= ~ToMouseButtonFromSDL(event.button.button);

		// TODO generate events if the event.motion.state magically changed things
		// TODO keep track of the keyboard modifiers state

		int x, y;
		SDL_GetMouseState(&x,&y);

		const Point newPos = Point(x,y);
		const Point oldPos = newPos; // TODO implement remembering of the old position

		// determine which window/popup should get the event
		Widget *windowWidget = NULL;
		if (event.type == SDL_MOUSEMOTION) {
			for (WidgetList::const_reverse_iterator it = _popupStack.rbegin(); it != _popupStack.rend(); ++it) {
				if ((*it)->geometry().contains(newPos) && (*it)->hasMouseTracking()) {
					windowWidget = *it;
					break;
				}
			}
		}
		else if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEWHEEL) {
			// NOTE: for the case of a button press, it closes all popups that weren’t hit
			while (!_popupStack.empty()) {
				if (_popupStack.back()->geometry().contains(newPos)) {
					windowWidget = _popupStack.back();
					break;
				} else {
					_popupStack.back()->close(); // NOTE: when we click outside
					_popupStack.pop_back();

					if(_popupStack.empty())
						return;
				}
			}


		}

		if (!windowWidget) {
			for (WidgetList::const_reverse_iterator it = _windowStack.rbegin(); it != _windowStack.rend(); ++it) {
				if ((*it)->geometry().contains(newPos)) {
					windowWidget = *it;
					break;
				}
			}
		}

		if (windowWidget) {
			// possibly switch which widget we are hovering over
			Widget *widget = windowWidget->_GetWidgetAt(newPos); // TODO don't just assume the top window?
			if (_mouseGrabber && _mouseGrabber != widget)
				widget = NULL;
			_SetHoverWidget(widget);

			// determine which widget to translate the coordinate system to
			Point origin;
			if (_mouseGrabber)
				origin = _mouseGrabber->mapToGlobal(Point());
			else if (_hoverWidget)
				origin = _hoverWidget->mapToGlobal(Point());
			// else
				// origin = geometry().topLeft();

			// create the mouse event and dispatch it
			MouseButton button = NoButton;
			if(event.type == SDL_MOUSEBUTTONUP || event.type == SDL_MOUSEBUTTONDOWN)
				button = ToMouseButtonFromSDL(event.button.button);

			if(event.type == SDL_MOUSEWHEEL) {
				if(event.wheel.y > 0)
					button = WheelUpButton;
				else
					button = WheelDownButton;
			}

			MouseEvent mouseEvent(button, _buttonState, newPos - origin, oldPos - origin);

			if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEWHEEL) {
				if (_mouseGrabber) {
					_mouseGrabber->mousePressEvent(&mouseEvent);
				}
				else if (_hoverWidget) {
					_hoverWidget->mousePressEvent(&mouseEvent);
					if (mouseEvent.isAccepted() && event.type != SDL_MOUSEWHEEL)
                    {
						_mouseGrabber = _hoverWidget;
                    }

                        //set focus on widgets that can accept focus
                        if(!_hoverWidget->hasFocus())
                        {
                            if(_widgetInFocus!=NULL)
                            {
                                _widgetInFocus->setFocus(false);
                            }
                            if(_hoverWidget->canReceiveFocus())
                            {
                                _widgetInFocus = _hoverWidget;
                                _widgetInFocus->setFocus(true);
                            }
                            else
                            {
                                _widgetInFocus = NULL;
                            }
                        }
				}
			}
			else if (event.type == SDL_MOUSEBUTTONUP) {
				if (_mouseGrabber) {
					_mouseGrabber->mouseReleaseEvent(&mouseEvent);

					// we stop grabbing the mouse when we release all buttons (after grabbing started from at least *one* of the buttons)
					if (_buttonState == NoButton) {
						_mouseGrabber = NULL;
						_SetHoverWidget(_windowStack.front()->_GetWidgetAt(newPos)); // TODO don't just assume the top window?
						// TODO possibly generate an artificial mouse motion event for the widget we are over?
					}
				}
				else if (_hoverWidget)
					_hoverWidget->mouseReleaseEvent(&mouseEvent);
			}
			else { // if (event.type == SDL_MOUSEMOTION)
				if (_mouseGrabber)
					_mouseGrabber->mouseMotionEvent(&mouseEvent);
				else if (_hoverWidget && _hoverWidget->hasMouseTracking())
					_hoverWidget->mouseMotionEvent(&mouseEvent);
			}
		}
	}
    else if(event.type ==SDL_FINGERMOTION)
	{
        Log::msg("Finger motion %ll x: %f y: %f", event.tfinger.fingerId, event.tfinger.x, event.tfinger.y );
        std::cout<<"Finger motion: "<<event.tfinger.fingerId<<", x: "<<event.tfinger.x<<", y: "<<event.tfinger.y<<"\n";
	}
	 else if(event.type ==SDL_FINGERDOWN)
	{
        Log::msg("Finger DOWN %ll x: %f y: %f", event.tfinger.fingerId, event.tfinger.x, event.tfinger.y );
        std::cout<<"Finger DOWN: "<<event.tfinger.fingerId<<", x: "<<event.tfinger.x<<", y: "<<event.tfinger.y<<"\n";
	}
	 else if(event.type ==SDL_FINGERUP)
	{
        Log::msg("Finger UP %ll x: %f y: %f", event.tfinger.fingerId, event.tfinger.x, event.tfinger.y );
        std::cout<<"Finger UP: "<<event.tfinger.fingerId<<", x: "<<event.tfinger.x<<", y: "<<event.tfinger.y<<"\n";
	}
    else if(event.type ==SDL_MULTIGESTURE)
	{
        Log::msg("Finger MULTY number of fingers: %u theta: %f Distance: %f, X: %f, Y: %f ",event.mgesture.numFingers, event.mgesture.dTheta, event.mgesture.dDist, event.mgesture.x, event.mgesture.y );
        std::cout<<"Finger MULTY: number of fingers:"<<event.mgesture.numFingers<<" Theta: "<<event.mgesture.dTheta<<" Distance: "<<event.mgesture.dDist<<", x: "<<event.mgesture.x<<", y: "<<event.mgesture.y<<"\n";
	}
	else if(event.type ==SDL_TEXTINPUT)
	{
        if(_widgetInFocus!=NULL)
        {
            std::cout<<"----- Text: "<<event.edit.text<<"Start: "<<event.edit.start<<" Length "<<event.edit.length<<"\n";
            _widgetInFocus->textInputEvent(event);
        }
	}
	else if(event.type ==SDL_TEXTEDITING)
	{
        if(_widgetInFocus!=NULL)
        {
            std::cout<<"Text: "<<event.edit.text<<"Start: "<<event.edit.start<<" Length "<<event.edit.length<<"\n";
            _widgetInFocus->textEditingEvent(event);
        }
	}
	else if (event.type == SDL_KEYDOWN) {
		const SDL_KeyboardEvent &kevent = *reinterpret_cast<const SDL_KeyboardEvent*>(&event);
		if(kevent.keysym.sym == SDLK_q && kevent.keysym.mod | KMOD_CTRL)
			_running = false;
		else {
			KeyboardEvent e(sdl_keysym_to_key(kevent.keysym.sym), sdl_keymod_to_keymod(kevent.keysym.mod));
			if(_keyboardGrabber) {
				_keyboardGrabber->keyPressEvent(&e);
			} else if(_hoverWidget) {
				for(Widget *w = _hoverWidget; w != NULL; w = w->parentWidget()) {
					w->keyPressEvent(&e);
					if(e.isAccepted()) {
						_keyboardGrabber = w;
						break;
					}
				}
			}
			else {
				keyPressEvent(&e); // global keyboard controls
			}
			if(_widgetInFocus!=NULL)
            {
                _widgetInFocus->keyDownEvent(event);
            }
		}
	}
	else if (event.type == SDL_KEYUP) {
		const SDL_KeyboardEvent &kevent = *reinterpret_cast<const SDL_KeyboardEvent*>(&event);
		KeyboardEvent e(sdl_keysym_to_key(kevent.keysym.sym), sdl_keymod_to_keymod(kevent.keysym.mod));
		if(_keyboardGrabber) {
			_keyboardGrabber->keyReleaseEvent(&e);
			_keyboardGrabber = NULL;
		} else if(_hoverWidget) {
			_hoverWidget->keyReleaseEvent(&e);
		}

		keyReleaseEvent(&e);
	}
	else if (event.type == SDL_QUIT) {
		_running = false; // TODO generate an event instead of directly shutting down the event loop
	}
	else if (event.type == SDL_WINDOWEVENT)	{
		if(event.window.event == SDL_WINDOWEVENT_RESIZED) {
			int w, h;
			SDL_GetWindowSize(sdlWindow,&w,&h);
			glViewport(0, 0, w, h);

			for(WidgetList::iterator it = _windowStack.begin(); it != _windowStack.end(); it++) {
				(*it)->setSize(Size(std::max(MIN_WINDOW_W,w), std::max(MIN_WINDOW_H,h)));
				(*it)->_DoGlResetEvents();
			}
		}
	}
}

void Application::createWindow(int w, int h) {
	assert(_windowStack.size() > 0); // cannot create window without content present.

	int rc = SDL_CreateWindowAndRenderer(w,h,SDL_WINDOW_RESIZABLE|SDL_WINDOW_OPENGL,&sdlWindow,&sdlRenderer);
	if(rc < 0) {
		Log::fatal("SDL failed to create window: %d",SDL_GetError());
	}

	SDL_GL_SetSwapInterval(1);

	sdlGLContext = SDL_GL_CreateContext(sdlWindow);
	if(sdlGLContext == 0) {
		Log::fatal("SDL failed to create GL context: %d",SDL_GetError());
	}
	SDL_GetWindowSize(sdlWindow,&w,&h);

	glViewport(0, 0, w, h);
	SDL_SetWindowTitle(sdlWindow, windowTitle().c_str());

	// initialize OpenGL settings
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	const Color &bg = Colors::background;
	glClearColor(bg.redF(), bg.greenF(), bg.blueF(), 0);
	glEnable(GL_CULL_FACE);

	TextureGL::reloadAll();

	glBindTexture(GL_TEXTURE_2D, 0);

	for(WidgetList::iterator it = _windowStack.begin(); it != _windowStack.end(); it++) {
		(*it)->setSize(Size(std::max(MIN_WINDOW_W,w), std::max(MIN_WINDOW_H,h)));
		(*it)->_DoGlResetEvents();
	}

}

void Application::_SetHoverWidget(Widget *widget) {
	if (widget != _hoverWidget) {
		if (_hoverWidget)
			_hoverWidget->leaveEvent();
		_hoverWidget = widget;
		if (_hoverWidget)
			_hoverWidget->enterEvent();
	}
}

void Application::setWindowTitle(const std::string &str) {
	_windowTitle = str;
	SDL_SetWindowTitle(sdlWindow, windowTitle().c_str());
}

std::string Application::windowTitle() const {
	return _windowTitle;
}

Widget *&Application::keyboardGrabber() {
	return _keyboardGrabber;
}

Widget *&Application::mouseGrabber() {
	return _mouseGrabber;
}

Widget *&Application::hoverWidget() {
	return _hoverWidget;
}

const BitmapFontGL *Application::font() {
	return _font;
}

void Application::advance() {
}

void Application::keyPressEvent(KeyboardEvent *e) {
}

void Application::keyReleaseEvent(KeyboardEvent *e) {
}

} // namespace Widgets

} // namespace BackyardBrains
