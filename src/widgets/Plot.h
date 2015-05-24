#ifndef BACKYARDBRAINS_WIDGETS_PLOT_H
#define BACKYARDBRAINS_WIDGETS_PLOT_H

#include "Widget.h"
#include "Color.h"

#include <vector>
#include <string>

namespace BackyardBrains {

namespace Widgets {

class Plot : public Widget
{
public:
	enum PlotStyle {
		Line,
		Bar
	};

	Plot(Widget *parent = NULL);

	void setData(const std::vector<float> &x, const std::vector<float> &y);
	void setSTD(const std::vector<float> &std);
	void updateAxisScale();

	void setXLabel(const std::string &label);
	void setYLabel(const std::string &label);

	void setColor(const Color &clr);
	void setStyle(PlotStyle style);
private:
	static const int TICLENGTH = 10;

	void paintEvent();
	void mousePressEvent(MouseEvent *event);
	void mouseReleaseEvent(MouseEvent *event);

	void drawLinePlot();
	void drawBarPlot();
	void drawTics();
	void drawLegend();

	Color errorBarColor();

	float plotWidth() const;
	float plotHeight() const;

	int axisOffsetX() const; // offset of y-axis in x-direction
	int axisOffsetXRight() const; // offset of y-axis in x-direction
	int axisOffsetY() const; // vice versa

	void ticParams(float &ticdst, float &ticoff, int &n, float min, float max, int maxn) const;

	std::vector<float> _stdy;
	std::vector<float> _ys;
	std::vector<float> _xs;

	std::string _xlabel, _ylabel;
	float _xoffset, _yoffset;
	float _xscale, _yscale;
	
	float _xmin, _xmax;
	float _ymin, _ymax;

	PlotStyle _style;
	Color _color;
};

} // namespace Widgets

} // namespace BackyardBrains

#endif
