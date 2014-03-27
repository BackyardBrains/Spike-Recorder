CCX = g++
CC  = gcc

#for cross compiling

## Windows i686
# ARCH = i686-w64-mingw32-
# BINPREFIX = /usr/i686-w64-mingw32/bin/
# EXT = .exe

TARGET = SpikeRecorder
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

ifeq ($(ARCH),)
	UNAME_S = $(shell uname -s)
else
	UNAME_S =
endif

CFLAGS = -g -O2 -Isrc -Isupport -I. -Wall -DSIGSLOT_PURE_ISO

ifeq ($(UNAME_S),Darwin)
	OBJECTS += src/widgets/native/FileDialogMac.o
	OBJCFILES = support/SDLMain.m src/widgets/native/FileDialogMac.mm

	CFLAGS += -I$(FRAMEWORK_PATH)/SDL.framework/Headers -I$(FRAMEWORK_PATH)/SDL_image.framework/Headers # for Mac OS X
	LIBS = libbass.dylib $(OBJCFILES) -F. -framework SDL -framework Cocoa -framework SDL_image -framework OpenGL -framework GLUT
	ifneq ($(FRAMEWORK_PATH),)
		LIBS += -F$(FRAMEWORK_PATH)
	endif

else
	CFLAGS += `$(BINPREFIX)sdl-config --cflags` # for Windows/Linux
	EXTRA_CMD = 
ifeq ($(UNAME_S),Linux)
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
	$(ARCH)$(CCX) -o $(TARGET)$(EXT) $(OBJECTS) $(CFLAGS) $(LIBS)

ifeq ($(UNAME_S),Darwin)
	mkdir -p $(TARGET).app/Contents/MacOS
	mv $(TARGET)$(EXT) $(TARGET).app/Contents/MacOS
	cp libbass.dylib $(TARGET).app/Contents/MacOS
	mkdir -p $(TARGET).app/Contents/Resources
	cp -r data $(TARGET).app/Contents/Resources
	cp macosx-Info.plist $(TARGET).app/Contents/
endif

all:
	$(TARGET)

clean:
	rm -f $(TARGET) $(TARGET).exe $(OBJECTS)
	rm -rf $(TARGET).app
