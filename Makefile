CCX = g++
CC  = gcc

#for cross compiling

## Windows i686
# ARCH = i686-w64-mingw32-
# BINPREFIX = /usr/i686-w64-mingw32/bin/
# EXT = .exe
# OS = Windows

TARGET = SpikeRecorder
TARGETDIR = SpikeRecorder

OBJECTS = \
	src/engine/RecordingManager.o \
	src/engine/FileRecorder.o \
	src/engine/Player.o \
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
	src/Game.o \
	src/main.o \
	src/AudioView.o \
	src/ConfigView.o \
	src/RecordingBar.o \
	src/ColorDropDownList.o

ifeq ($(OS),)
	UNAME_S = $(shell uname -s)
	OS = Windows
	ifeq ($(UNAME_S),Darwin)
		OS = MacOSX
	endif
	ifeq ($(UNAME_S),Linux)
		OS = Linux
	endif
endif

CFLAGS = -g -O2 -Isrc -Isupport -I. -Wall -DSIGSLOT_PURE_ISO

ifeq ($(OS),MacOSX)
	OBJECTS += src/widgets/native/FileDialogMac.o
	OBJCFILES = support/SDLMain.m src/widgets/native/FileDialogMac.mm

	LIBS = -Wl,-rpath,@executable_path/../Frameworks libbass.dylib $(OBJCFILES) -F. -framework SDL -framework Cocoa -framework SDL_image -framework OpenGL -framework GLUT

	FWPATH = /Library/Frameworks

	ifneq ($(FRAMEWORK_PATH),)
		FWPATH = $(FRAMEWORK_PATH)
		CFLAGS += -I$(FRAMEWORK_PATH)/SDL.framework/Headers -I$(FRAMEWORK_PATH)/SDL_image.framework/Headers # for Mac OS X
		LIBS += -F$(FRAMEWORK_PATH)
	endif


else
	CFLAGS += `$(BINPREFIX)sdl-config --cflags` # for Windows/Linux
	EXTRA_CMD = 

	ifeq ($(OS),Linux)
		OBJECTS += src/widgets/native/FileDialogLinux.o
		LIBS = `sdl-config --libs` -lSDL_image -lGL -lGLU -lbass # for Linux
	else


		OBJECTS += src/widgets/native/FileDialogWin.o
		LIBS = -static -lSDL_image `$(BINPREFIX)sdl-config --static-libs`
		LIBS += -lglut -lwebp -lpng -ltiff -lz -ljpeg -lopengl32 -lglu32 -dynamic support/bass.lib -Wl,--enable-auto-import # for Windows
	endif
endif

%.o: %.c
	$(ARCH)$(CC) -o $@ -c $< $(CFLAGS)

%.o: %.cpp
	$(ARCH)$(CCX) -o $@ -c $< $(CFLAGS)

$(TARGET): $(OBJECTS)
ifeq ($(OS),Windows)
	$(ARCH)windres -o _resources.o win32-info.rc

	$(ARCH)$(CCX) -o $(TARGET)$(EXT) $(OBJECTS) _resources.o $(CFLAGS) $(LIBS)
else
	$(ARCH)$(CCX) -o $(TARGET)$(EXT) $(OBJECTS) $(CFLAGS) $(LIBS)
endif
ifeq ($(OS),MacOSX)
	mkdir -p $(TARGET).app/Contents/MacOS
	mv $(TARGET)$(EXT) $(TARGET).app/Contents/MacOS
	cp libbass.dylib $(TARGET).app/Contents/MacOS
	mkdir -p $(TARGET).app/Contents/Resources/data
	cp data/*.png $(TARGET).app/Contents/Resources/data
	cp SpikeRecorder.icns $(TARGET).app/Contents/Resources
	cp macosx-Info.plist $(TARGET).app/Contents/Info.plist

	mkdir -p $(TARGET).app/Frameworks
	cp -r $(FWPATH)/SDL.framework $(TARGET).app/Contents/Frameworks
	cp -r $(FWPATH)/SDL_image.framework $(TARGET).app/Contents/Frameworks
endif

ifeq ($(OS),Windows)
package: $(TARGET)
	mkdir -p $(TARGETDIR)
	cp $(TARGET)$(EXT) $(TARGETDIR)
	cp -r data $(TARGETDIR)/
	cp bass.dll $(TARGETDIR)
	zip -r $(TARGETDIR).zip $(TARGETDIR)/
endif
all:
	$(TARGET)

clean:
	rm -rf $(TARGET) $(TARGET).exe $(OBJECTS) $(TARGETDIR).zip
	rm -rf $(TARGET).app $(TARGETDIR)

.PHONY: all clean
