#ifndef BACKYARDBRAINS_WIDGETS_APPLICATION_H
#define BACKYARDBRAINS_WIDGETS_APPLICATION_H

#include "Widget.h"
#include "Size.h"
#include <list>
#include <string>

namespace BackyardBrains {

namespace Widgets {

class Application : public sigslot::has_slots<>
{
public:
	static const int MIN_WINDOW_W = 120;
	static const int MIN_WINDOW_H = 100;

	Application();
	virtual ~Application();

	void run();

	void setWindowTitle(const std::string &str);
	std::string windowTitle() const;

	Widget *keyboardGrabber();
	Widget *mouseGrabber();

	void addPopup(Widget *w);
	void addWindow(Widget *w);

	static const BitmapFontGL *font();
	static Application *getInstance();
protected:
	void createWindow(int w, int h);
	Widget *mainWidget();
private:
	static Application *app;
	typedef std::vector<Widget*> WidgetVector;
	typedef std::list<Widget*> WidgetList;

	bool _running;
	Widget *_mouseGrabber;
	Widget *_keyboardGrabber;
	Widget *_hoverWidget;
	MouseButtons _buttonState;
	WidgetList _windowStack;
	WidgetList _popupStack;

	static BitmapFontGL *_font;

	virtual void advance();
	void _HandleEvent(const void *eventRaw);
	Widget * _GetWidgetAt(const Point &point) const;

	void _SetHoverWidget(Widget *widget);
	void removeClosed(WidgetList &w);

	Widget *_mainwidget;
	std::string _windowTitle;

};

} // namespace Widgets

} // namespace BackyardBrains

#endif
