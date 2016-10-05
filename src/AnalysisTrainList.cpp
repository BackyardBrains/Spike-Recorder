#include "AnalysisTrainList.h"
#include "widgets/Painter.h"
#include "widgets/Application.h"
#include "widgets/BitmapFontGL.h"
#include "AudioView.h"
#include <SDL_opengl.h>
#include <sstream>
#include "Log.h"

namespace BackyardBrains {


AnalysisTrainList::AnalysisTrainList(RecordingManager &mngr, std::vector<SpikeTrain> &spikeTrains, Widgets::Widget *parent)
	: Widget(parent), _manager(mngr), _spikeTrains(spikeTrains), _selectedTrain(0) {
//	setSizePolicy(Widgets::SizePolicy(Widgets::SizePolicy::Fixed, Widgets::SizePolicy::Expanding));
	updateSize();
        
}

    
//
// Used to find spike train that should be initialy selected
// this depends on selected channel
//
void AnalysisTrainList::setInitialSelection()
{
    //find out what should be initial selection
    bool foundSpikeTrainToSelect = false;
    int spikeTrainToSelect = 0;
    for(int st = 0;st<_spikeTrains.size()-1;st++)
    {
        if(_spikeTrains[st].channelIndex == _manager.selectedVDevice())
        {
            foundSpikeTrainToSelect = true;
            spikeTrainToSelect = st;
            break;
        }
    }
    
    if(!foundSpikeTrainToSelect)//select n+ sign (new spike train)
    {
        _spikeTrains[_spikeTrains.size()-1].channelIndex = _manager.selectedVDevice();
        spikeTrainToSelect = _spikeTrains.size()-1;
    }
    _selectedTrain = spikeTrainToSelect;
    _selectedChannel = _manager.selectedVDevice();
    selectionChange.emit(spikeTrainToSelect);
}
    
int AnalysisTrainList::selectedTrain() const {
	int i = _selectedTrain;
	if(i < 0)
		i = 0;
	if(i >= (int)_spikeTrains.size())
		i = _spikeTrains.size()-1;
	return i;
}
    
int AnalysisTrainList::selectedChannel()
{
    return _selectedChannel;
}

void AnalysisTrainList::updateSize() {
	setSizeHint(Widgets::Size(150,(CHANNEL_HEADER_SIZE+PADDING+PADDING) * _manager.numberOfChannels() + _spikeTrains.size()*(FIELDH+2*PADDING)));
}

void AnalysisTrainList::paintEvent() {
    
    int currentYPosition = 0;
    for(int channelIndex = 0; channelIndex<_manager.numberOfChannels();channelIndex++)
    {
        currentYPosition += PADDING;
        
        //draw name of the channel background
        Widgets::Painter::setColor(Widgets::Colors::widgetbg);
        Widgets::Painter::drawRect(Widgets::Rect(0,currentYPosition,width(), FIELDH));
        //draw name of the channel
        
        Widgets::Painter::setColor(Widgets::Colors::white);
        std::stringstream nameOfChannelString;
        nameOfChannelString << "Channel "<<channelIndex+1;
        Widgets::Application::font()->draw(nameOfChannelString.str().c_str(), PADDING*3,
                                           currentYPosition+FIELDH/2, Widgets::AlignVCenter);
        
        currentYPosition += FIELDH + PADDING;
        
        
        //draw spike train rows
        for(unsigned int i = 0; i < _spikeTrains.size(); i++) {

                if(i==_selectedTrain && _spikeTrains[i].channelIndex == channelIndex)
                {
                    //draw selected spike train background
                    
                    Widgets::Painter::setColor(Widgets::Colors::button);
                    Widgets::Painter::drawRect(Widgets::Rect(0,currentYPosition,width(), FIELDH+2*PADDING));
                }
            
                if(i != _spikeTrains.size()-1) {
                    if(_spikeTrains[i].channelIndex == channelIndex)//if this is for current channel that we are drawing
                    {
                        currentYPosition +=PADDING;

                        //set color of the box before name of spike trains
                        Widgets::Painter::setColor(AudioView::MARKER_COLORS[_spikeTrains[i].color%AudioView::MARKER_COLOR_NUM]);
                        
                        //draw name of spiketrain
                        std::stringstream s;
                        s << _spikeTrains[i].spikes.size() << " spikes";
                        
                        Widgets::Application::font()->draw(s.str().c_str(), PADDING*3+FIELDW,
                                currentYPosition+FIELDH/2, Widgets::AlignVCenter);
                        
                        //draw color box on left of the spike train
                        Widgets::Painter::drawRect(Widgets::Rect(PADDING, currentYPosition, FIELDW, FIELDH));
                        currentYPosition += FIELDH;
                    }

                } else {
                    currentYPosition +=PADDING;
                    //draw background box of + button
                    Widgets::Painter::setColor(Widgets::Color(100,100,100));
                    //draw color box on left of the spike train
                    Widgets::Painter::drawRect(Widgets::Rect(PADDING, currentYPosition, FIELDW, FIELDH));
                }
            
            
                //draw - sign for removing spike trains
                if(i==_selectedTrain && _selectedTrain != (int)_spikeTrains.size()-1 && _spikeTrains[i].channelIndex == channelIndex)
                {
                    Widgets::Painter::setColor(Widgets::Colors::widgetbg);
                    Widgets::Painter::drawRect(Widgets::Rect(width()-REMOVEOFF-REMOVERAD,
                                                             currentYPosition-FIELDH/2-REMOVERAD/2+1,2*REMOVERAD,REMOVERAD/2));
                }
            
                
            
        }
        
        const int plush = FIELDH/2;//plus sign height
        const int plusw = FIELDH/2;//plus sign width

        //draw plus sign for adding the channels
        Widgets::Painter::setColor(Widgets::Colors::buttonhigh);
        Widgets::Painter::drawRect(Widgets::Rect(PADDING+FIELDW/2-2,
                    currentYPosition+(FIELDH-plush)/2,3,plush));
        Widgets::Painter::drawRect(Widgets::Rect(PADDING+(FIELDW-plusw)/2,
                    currentYPosition+FIELDH/2-2,plusw,3));
        
        currentYPosition += FIELDH;
        currentYPosition += PADDING;
    }

}

void AnalysisTrainList::mousePressEvent(Widgets::MouseEvent *event) {
	
    int mouseY = event->pos().y;
    int channel = 0;
    int spikeTrain = -1;
    int currentYPosition = 0;
    if(event->button() == Widgets::LeftButton) {
        
        
        
        //in order to find where mouse click landed we have to go through all
        //graphical elements in spike train list.
        for(int channelIndex = 0;channelIndex<_manager.numberOfChannels();channelIndex++)
        {
            //click on channel name
            currentYPosition += PADDING;
            
            if(mouseY>=currentYPosition && mouseY< (currentYPosition+FIELDH + PADDING))
            {
                channel = channelIndex;
                
                //since user clicked on name of the channel we don;t know what exactly
                //spike train to select. So we search for first spike train from this channel
                //and we make that spike train selected.
                //If we don't find any spike train from that channel we will select + sign
                bool foundSpikeTrainFromCurrentChannel = false;
                for(int tempSTIndex = 0;tempSTIndex<_spikeTrains.size();tempSTIndex++)
                {
                    if(_spikeTrains[tempSTIndex].channelIndex == channelIndex)
                    {
                        foundSpikeTrainFromCurrentChannel = true;
                        spikeTrain = tempSTIndex;
                        break;
                    }
                }
                
                if(!foundSpikeTrainFromCurrentChannel)
                {
                    spikeTrain = _spikeTrains.size()-1;
                }
            }
            
            currentYPosition += FIELDH;
            
            
            //click on spike train rows
            for(unsigned int i = 0; i < _spikeTrains.size(); i++)
            {
                
                if(i != _spikeTrains.size()-1) {
                    if(_spikeTrains[i].channelIndex == channelIndex)
                    {
                        //click on existing spike train row
                    
                        currentYPosition +=PADDING;
                        
                        if(mouseY>=currentYPosition && mouseY< currentYPosition+FIELDH )
                        {
                            channel = channelIndex;
                            spikeTrain = i;
                        }
                        
                        currentYPosition += FIELDH;
                    }
                } else {
                    //click on + button
                    currentYPosition +=PADDING;
                    if(mouseY>=currentYPosition && mouseY< currentYPosition+FIELDH )
                    {
                        channel = channelIndex;
                        spikeTrain = i;
                    }
                    currentYPosition += FIELDH;
                }
            }
            
            currentYPosition += PADDING;
        }
        
        
        //react on click: delete row or make selection

		if(spikeTrain == _selectedTrain && _selectedTrain != (int)_spikeTrains.size()-1)	{
			//if it is click on - sign on the right hand side of spike train row
			int dx = event->pos().x-width()+REMOVEOFF;
			if(dx*dx < REMOVERAD*REMOVERAD) {
				trainDeleted.emit(_selectedTrain);
				event->accept();
				return;
			}
		} else if(spikeTrain >= 0 && spikeTrain < (int)_spikeTrains.size()) {
            //if it is not - sign select row
			_selectedTrain = spikeTrain;
            _selectedChannel = channel;
            _spikeTrains[spikeTrain].channelIndex = channel;
			event->accept();
			selectionChange.emit(spikeTrain);
		}
	}		
}

}
