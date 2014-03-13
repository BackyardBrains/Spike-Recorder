BYB-Neural-Recorder
===================

A neural recording app for the PC and Macs.  Runs on Windows, Linux and OSX


BYB Neural Recorder ToDo
------------------------

Release 1:

*Finish Thresholding mode Logic: In threshold mode, there should be no seekbar.  Once a voltage crosses the threshold line (---- line), it should snapshot the data and center it on the screen for display.   If the number of samples averaged (slider bar) indicates how many samples should be averaged in the display.  Zooming in and out in time should still keep the sample centered.   Although we are triggering on only 1 channel, we should average all channels (each channel is averaged individually, and centered at the same time point of the thresholded channel)  The pause button should pause the data.  If the threshold line is moved, the samples should clear and start over. 
*Adjust Gain so that it is not too high when it enables (Not quite sure how to do this… sample for 1-2s?)
*Default: only 1 channel on, and it should be centered on the screen.
*Recording files to a .WAV format.  When the record button is pressed, the data displayed si saved to an audio file.  If possible, it would be nice to capture some meta-data: Color, position, gain, etc.
*Loading Files for review.   In Review mode, there will be no live audio.  We will stream the data from the file and display it in the View and Threshold modes as if it was live audio.  The seekbar UI should change so that 100% of the bar = 100% of the audio file.  Upon loading, the file should start from the beginning (far left) and play in realtime.  The user will be able to skip around or pause the file.
*If possible, it would be nice to add event markers to the file.  For example, while recording… pressing 1 - 9 would cause a vertical line to be placed across the screen at that timepoint.  This will be useful for analysis, but would only make sense if we could store meta data.  
*Add a ruler button in between the config and threshold button.  When selected it allows the use to draw a time window on the screen with the mouse button.   The background should change to a light gray to indicate the width of the X direction being drawn, and indicate this inside of the gray area.  This is useful for measuring the time difference between spikes.
*Add playback to speaker option for listening to playbacks. This could optionally be ebabled for recording as well.

Release 2:
	
*Analysis Mode.  A button on the screen will take the recorded data in the buffer, or the data loaded from a file, and start to perform analysis techniques:
*RMS: 
*Spike Sorting:  We will develop a simple interface to “count” the number of spikes in a data file.  By setting a threshold
