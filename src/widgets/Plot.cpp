#include "Plot.h"
#include "Application.h"
#include "Painter.h"
#include "BitmapFontGL.h"
#include <cassert>
#include <cmath>
#include <cstdio>
#include <sstream>

namespace BackyardBrains {
namespace Widgets {

Plot::Plot(Widget *parent)
 : Widget(parent), _xoffset(0),_yoffset(0),_xscale(1),_yscale(1),
	_xmin(0), _xmax(0), _ymin(0), _ymax(0), _style(Line), _color(Colors::lightblue) {
}

static void minmax(std::vector<float> &v, std::vector<float> &std, float &min, float &max) {
	max = v[0];
	min = v[0];

	for(unsigned int i = 0; i < v.size(); i++) {
		float s = 0;
		if(std.size() > 0)
			s = std[i];
		if(v[i]-s < min)
			min = v[i]-s;
		if(v[i]+s > max)
			max = v[i]+s;
	}
}

void Plot::updateAxisScale() {
	if(_xs.size() == 0)
		return;
	std::vector<float> dummy;
	minmax(_xs, dummy,_xmin, _xmax);
	minmax(_ys, _stdy, _ymin, _ymax);

	if(_style == Bar) {
		_ymin = 0;
		if(_xs.size() > 1) {
			_xmin -= 0.5*(_xs[1]-_xs[0]);
		}
	}

	if(_xs.size() == 1) {
		_xmin = _xmax-1;

		_ymin = _ymax-1;
	}

	_xoffset = _xmin;
	_xscale = 1.f/fabs(_xmax-_xmin);
	_yoffset = _ymin;
	_yscale = 1.f/fabs(_ymax-_ymin);


	if(_xs.size() == 1)
		_xoffset -= (_xmin-_xmax)/2;
}


void Plot::setData(const std::vector<float> &x, const std::vector<float> &y) {
	_xs = x;
	_ys = y;

	assert(_xs.size() == _ys.size());
	updateAxisScale();
}
void Plot::setSTD(const std::vector<float> &std) {
	_stdy = std;

	assert(_stdy.size() == 0 || _xs.size() == _stdy.size());
	updateAxisScale();
}

void Plot::setXLabel(const std::string &label) {
	_xlabel = label;
}

void Plot::setYLabel(const std::string &label) {
	_ylabel = label;
}

void Plot::setColor(const Color &clr) {
	_color = clr;
}

void Plot::setStyle(PlotStyle style) {
	_style = style;
	updateAxisScale();
}

float Plot::plotWidth() const {
	return 0.9*(width()-2*axisOffsetX());
}

float Plot::plotHeight() const {
	return 0.9*(height()-2*axisOffsetY());
}

int Plot::axisOffsetY() const {
	return 40;
}

int Plot::axisOffsetX() const {
	std::stringstream o1, o2;
	o1 << _ymax;
	o2 << _ymin;

	return axisOffsetY()+std::max(o1.str().size(),o2.str().size())*Application::font()->characterWidth();
}

Color Plot::errorBarColor() {
	Color tmp = _color;
	tmp.a *= 0.3;
	return tmp;
}

void Plot::drawLinePlot() {
	glPushMatrix();
	glTranslatef(axisOffsetX(),height()-axisOffsetY(),0);
	glScalef(plotWidth()*_xscale,-plotHeight()*_yscale,1);
	glTranslatef(-_xoffset,-_yoffset, 0);

	Painter::setColor(errorBarColor());
	if(_stdy.size() > 0) {
		glBegin(GL_QUAD_STRIP);
		for(unsigned int i = 0; i < _xs.size(); i++) {
			glVertex3f(_xs[i],_ys[i]+_stdy[i],0);
			glVertex3f(_xs[i],_ys[i]-_stdy[i],0);
		}
		glEnd();
	}
	Painter::setColor(_color);
	glBegin(GL_LINE_STRIP);
	for(unsigned int i = 0; i < _xs.size(); i++) {
		glVertex3f(_xs[i],_ys[i],0);
	}
	glEnd();
	glPopMatrix();
}

void Plot::drawBarPlot() {
	glPushMatrix();
	glTranslatef(axisOffsetX(),height()-axisOffsetY(),0);

	if(_xs.size() == 1) {
		glScalef(1,-plotHeight()*_yscale,1);
		float y1 = std::min(_ys[0],0.f);
		float y2 = std::max(_ys[0],0.f);
		float xl = 0;
		float xr = plotWidth();
		glBegin(GL_QUADS);
		glVertex3f(xl,y1,0);
		glVertex3f(xr,y1,0);
		glVertex3f(xr,y2,0);
		glVertex3f(xl,y2,0);
		glEnd();
	} else {
		glScalef(plotWidth()*_xscale,-plotHeight()*_yscale,1);
		glTranslatef(-_xoffset,-_yoffset, 0);
		for(unsigned int i = 0; i < _xs.size(); i++) {
			float xl, xr;
			float y1 = std::min(_ys[i],0.f);
			float y2 = std::max(_ys[i],0.f);
			if(i == 0) {
				xl = (_xs[0]-_xs[1])*0.5f;
			} else {
				xl = (_xs[i-1]+_xs[i])*0.5f;
			}

			if(i == _xs.size()-1) {
				xr = _xs[i]*1.5-0.5*_xs[i-1];
			} else {
				xr = (_xs[i+1]+_xs[i])*0.5f;
			}

			glBegin(GL_QUADS);
			glVertex3f(xl,y1,0);
			glVertex3f(xr,y1,0);
			glVertex3f(xr,y2,0);
			glVertex3f(xl,y2,0);
			glEnd();
		}
	}
	glPopMatrix();
}

static void ticparams(float &ticdst, float &ticoff, int &n, float min, float max) {
	int mag = round(log10(max-min)-1);
	ticdst = pow(10,mag);
	ticoff = floor(min/ticdst)*ticdst;
	n = (max-ticoff)/ticdst+1;
	if(n <= 0)
		n = 1;
}


void Plot::drawTics() {
	int xn, yn;
	float xticoff, yticoff, xticdst, yticdst;
	ticparams(xticdst, xticoff, xn, _xoffset, _xmax);
	ticparams(yticdst, yticoff, yn, _yoffset, _ymax);

	Painter::setColor(Colors::white);
	glPushMatrix();
	glTranslatef(axisOffsetX(),height()-axisOffsetY(),0);
	
	glPushMatrix();

	for(int i = 0; i < xn; i++) {
		float x = plotWidth()*_xscale*(xticoff+xticdst*i-_xoffset);
		if(x < 0)
			continue;
		glBegin(GL_LINES);
		glVertex3f(x, -TICLENGTH/2,0);
		glVertex3f(x, +TICLENGTH/2,0);
		glEnd();

		std::stringstream o;
		o << xticoff+xticdst*i;
		Application::font()->draw(o.str().c_str(),x, TICLENGTH, AlignHCenter);
	}
	glPopMatrix();
	glPushMatrix();
	
	for(int i = 0; i < yn; i++) {
		float y = -plotHeight()*_yscale*(yticoff+yticdst*i-_yoffset);
		if(y > 0)
			continue;
		glBegin(GL_LINES);
		glVertex3f(-TICLENGTH/2, y, 0);
		glVertex3f(TICLENGTH/2, y, 0);
		glEnd();
		
		std::stringstream o;
		o << yticoff+yticdst*i;
		Application::font()->draw(o.str().c_str(), -TICLENGTH, y, AlignRight | AlignVCenter);
	}
	glPopMatrix();
	glPopMatrix();

}

void Plot::drawLegend() {
	if(_style == Line && _stdy.size() > 0) {
		int x = axisOffsetX()+plotWidth()-35;
		int y = axisOffsetY()+10;
		Painter::setColor(Colors::white);
		glBegin(GL_LINE_LOOP);
		glVertex3f(x-5,y-10,0);
		glVertex3f(x-5,y+30,0);
		glVertex3f(x+70,y+30,0);
		glVertex3f(x+70,y-10,0);
		glEnd();
		Painter::setColor(_color);
		glBegin(GL_LINES);
		glVertex3f(x,y,0);
		glVertex3f(x+20,y,0);
		glEnd();
		Painter::setColor(errorBarColor());
		glBegin(GL_QUADS);
		glVertex3f(x,y+15,0);
		glVertex3f(x,y+25,0);
		glVertex3f(x+20,y+25,0);
		glVertex3f(x+20,y+15,0);
		glEnd();

		Painter::setColor(Colors::white);
		Application::font()->draw("Data",x+30, y, AlignVCenter);
		Application::font()->draw("\xf1 1\xe5",x+30, y+20, AlignVCenter);
	}
}

void Plot::paintEvent() {
	Painter::setColor(_color);
	if(_style == Line) {
		drawLinePlot();
	} else if(_style == Bar) {
		drawBarPlot();
	}

	Painter::setColor(Colors::white);
	glBegin(GL_LINE_STRIP);
	glVertex3f(axisOffsetX(),axisOffsetY(),0);
	glVertex3f(axisOffsetX(),height()-axisOffsetY(),0);
	glVertex3f(width()-axisOffsetX(),height()-axisOffsetY(),0);
	glEnd();

	Application::font()->draw(_xlabel.c_str(), width()/2, height()-Application::font()->characterHeight(), AlignHCenter);
	
	glPushMatrix();
	glTranslatef(2,height()/2,0);
	glRotatef(-90,0,0,1);
	Application::font()->draw(_ylabel.c_str(), 0, 0, AlignHCenter);
	glPopMatrix();

	drawTics();
	drawLegend();
}

void Plot::mousePressEvent(MouseEvent *event) {
}

void Plot::mouseReleaseEvent(MouseEvent *event) {
}

}
}
