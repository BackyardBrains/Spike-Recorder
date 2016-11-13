#ifndef BACKYARDBRAINS_WIDGETS_APPLICATION_H
#define BACKYARDBRAINS_WIDGETS_APPLICATION_H

#include "Widget.h"
#include "Size.h"
#include <list>
#include <string>
#include <SDL.h>

namespace BackyardBrains {

namespace Widgets {

class Application : public sigslot::has_slots<>
{
public:
	static const int MIN_WINDOW_W;
	static const int MIN_WINDOW_H;

	Application();
	virtual ~Application();

	void run();

	void setWindowTitle(const std::string &str);
	std::string windowTitle() const;

	Widget *&keyboardGrabber();
	Widget *&mouseGrabber();
	Widget *&hoverWidget();

	void addPopup(Widget *w);
	void addWindow(Widget *w);

	void updateLayout();

	static const BitmapFontGL *font();
	static Application *getInstance();

	bool areWeOnTouchscreen(){return _lastMouseClickWasOnTouchscreen;}
protected:
	void createWindow(int w, int h);
private:
	static Application *app;
	typedef std::vector<Widget*> WidgetVector;
	typedef std::list<Widget*> WidgetList;

	SDL_Window *sdlWindow;
	SDL_Renderer *sdlRenderer;
	SDL_GLContext sdlGLContext;

	bool _running;
	Widget *_mouseGrabber;
	Widget *_keyboardGrabber;
	Widget *_hoverWidget;
	Widget *_widgetInFocus;
	MouseButtons _buttonState;
	WidgetList _windowStack;
	WidgetList _popupStack;

	static BitmapFontGL *_font;

	virtual void advance();
	virtual void keyPressEvent(KeyboardEvent *event);
	virtual void keyReleaseEvent(KeyboardEvent *event);

	void _HandleEvent(const void *eventRaw);
	Widget * _GetWidgetAt(const Point &point) const;

	void _SetHoverWidget(Widget *widget);
	void removeClosed(WidgetList &w);

	std::string _windowTitle;

    //used to keep track of multifingers gestures
    SDL_Event _fingerMoveOne;
    SDL_Event _fingerMoveTwo;
    void keepTrackOfFingerMoves(const SDL_Event &event );
    int twoFingersGestureDirection();
    bool weAreInTwoFingersGesture = false; //true during 2 fingers gesture
    bool _lastMouseClickWasOnTouchscreen = false; //useful to change UI when user uses touchscreen
};

} // namespace Widgets

} // namespace BackyardBrains

#endif
