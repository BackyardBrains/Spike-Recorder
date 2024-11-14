CCX = g++
CC  = gcc

TARGET = SpikeRecorder
TARGETDIR = SpikeRecorder

OBJECTS = \
        src/engine/AudioInputConfig.o \
        src/CalibrationWindow.o \
        src/widgets/TouchDropDownList.o \
        src/widgets/HorizontalColorPicker.o \
        src/widgets/HorizontalNumberPicker.o \
		src/libraries/tinyxml2.o \
		src/engine/RecordingManager.o \
		src/engine/AnalysisManager.o \
		src/engine/FileRecorder.o \
		src/engine/Player.o \
		src/engine/ArduinoSerial.o \
		src/engine/SpikeSorter.o \
		src/engine/SpikeAnalysis.o \
		src/engine/BASSErrors.o \
		src/engine/FileReadUtil.o \
		src/engine/FFTBackend.o \
		src/engine/HIDUsbManager.o \
		src/engine/EkgBackend.o \
		src/engine/FilterBase.o \
		src/engine/HighPassFilter.o \
		src/engine/LowPassFilter.o \
		src/engine/NotchFilter.o \
		src/widgets/LayoutItem.o \
		src/widgets/BoxLayout.o \
		src/widgets/Widget.o \
		src/widgets/Painter.o \
		src/widgets/PushButton.o \
		src/widgets/DropDownList.o \
		src/widgets/ScrollBar.o \
		src/widgets/Application.o \
		src/widgets/BitmapFontGL.o \
		src/widgets/TextureGL.o \
		src/widgets/LoadTexture.o \
		src/widgets/Label.o \
		src/widgets/ErrorBox.o \
		src/widgets/TextInput.o \
		src/widgets/RangeSelector.o \
		src/widgets/SwitchLayout.o \
		src/widgets/ToolTip.o \
		src/widgets/Plot.o \
		src/widgets/TabBar.o \
		src/DropDownList.o \
		src/Log.o \
		src/Game.o \
		src/main.o \
		src/MainView.o \
		src/AudioView.o \
		src/ConfigView.o \
		src/AnalysisView.o \
		src/AnalysisAudioView.o \
		src/AnalysisTrainList.o \
		src/AnalysisPlots.o \
		src/RecordingBar.o \
		src/ColorDropDownList.o \
		src/FirmwareUpdateView.o \
		src/FFTView.o \
		src/engine/WavTxtRecorder.o \
		src/engine/firmware/BYBBootloaderController.o \
		src/ThresholdPanel.o

OBJECTS_LINUX = \
	src/widgets/native/FileDialogLinux.o \
	src/native/PathsLinux.o

OBJECTS += $(OBJECTS_LINUX)

CFLAGS = -g -O2 -Isrc -Isupport -Isrc/libraries -I. -Lsrc/support -Wall -DSIGSLOT_PURE_ISO --std=c++11 `sdl2-config --cflags`
LIBS = `sdl2-config --libs` -lSDL2_image -lGL -lGLU -lbass -lpthread -lhidapi-libusb

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)

%.o: %.cpp
	$(CCX) -o $@ -c $< $(CFLAGS)

$(TARGET): $(OBJECTS)
	$(CCX) -o $(TARGET) $(OBJECTS) $(CFLAGS) $(LIBS)
all:
	$(TARGET)

clean:
	rm -rf $(TARGET) $(TARGET).exe $(OBJECTS) $(OBJECTS_LINUX)  $(TARGETDIR).zip
	rm -rf $(TARGET).app $(TARGETDIR) _resources.o

.PHONY: all clean
