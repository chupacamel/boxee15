INCLUDES=-I. -I.. -I../linux -I../../guilib -I../cores -I../utils -I../.. -I../cores/dvdplayer/Codecs -I../cores/dvdplayer/Codecs/ffmpeg
CXXFLAGS+= -D__STDC_CONSTANT_MACROS

SRCS=GUIDialogKaraokeSongSelector.cpp \
	karaokelyricscdg.cpp \
	karaokelyrics.cpp	\
	karaokelyricsfactory.cpp \
	karaokelyricsmanager.cpp \
	karaokelyricstext.cpp \
	karaokelyricstextkar.cpp \
	karaokelyricstextlrc.cpp \
	karaokelyricstextustar.cpp \
	Cdg.cpp \
	GUIWindowKaraokeLyrics.cpp \
	karaokewindowbackground.cpp \

LIB=karaoke.a

include ../../Makefile.include
-include $(patsubst %.cpp,%.P,$(patsubst %.c,%.P,$(SRCS)))
