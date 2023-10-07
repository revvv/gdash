## Process this file with automake to produce Makefile.in

localedir = $(datadir)/locale

AUTOMAKE_OPTIONS = subdir-objects

DEFS = -DLOCALEDIR=\"$(localedir)\" -DPKGDATADIR=\"$(pkgdatadir)\" @DEFS@

AM_CXXFLAGS = -I$(top_srcdir)/include

bin_PROGRAMS = gdash

strip: $(bin_PROGRAMS)
	strip $(bin_PROGRAMS)

# base sources and headers:
#   the gdash engine.
baseheaders = \
	misc/printf.hpp \
	misc/deleter.hpp \
	misc/autogfreeptr.hpp \
	cave/cavetypes.hpp \
	cave/elementproperties.hpp \
	cave/helper/namevaluepair.hpp \
	cave/helper/reflective.hpp \
	cave/helper/polymorphic.hpp \
	cave/helper/cavemap.hpp \
	cave/helper/cavehighscore.hpp \
	cave/colors.hpp \
	cave/cavebase.hpp \
	cave/cavestored.hpp \
	cave/helper/caverandom.hpp \
	cave/helper/cavesound.hpp \
	cave/object/caveobjectfill.hpp \
	cave/object/caveobjectboundaryfill.hpp \
	cave/object/caveobjectcopypaste.hpp \
	cave/object/caveobjectfillrect.hpp \
	cave/object/caveobjectfloodfill.hpp \
	cave/object/caveobject.hpp \
	cave/object/caveobjectrectangular.hpp \
	cave/object/caveobjectjoin.hpp \
	cave/object/caveobjectline.hpp \
	cave/object/caveobjectmaze.hpp \
	cave/object/caveobjectpoint.hpp \
	cave/object/caveobjectrandomfill.hpp \
	cave/object/caveobjectraster.hpp \
	cave/object/caveobjectrectangle.hpp \
	cave/caverendered.hpp \
	cave/particle.hpp \
	cave/helper/cavereplay.hpp \
	cave/caveset.hpp \
	fileops/bdcffhelper.hpp \
	fileops/bdcffload.hpp \
	fileops/bdcffsave.hpp \
	fileops/c64import.hpp \
	fileops/brcimport.hpp \
	fileops/binaryimport.hpp \
	fileops/exportcrli.hpp \
	fileops/loadfile.hpp \
	fileops/highscore.hpp \
	cave/gamecontrol.hpp \
	settings.hpp \
	misc/util.hpp \
	misc/logger.hpp \
	misc/about.hpp \
	misc/helptext.hpp \
	gfx/pixbuf.hpp \
	gfx/pixmapstorage.hpp \
	gfx/screen.hpp \
	gfx/pixbuffactory.hpp \
	gfx/pixbufmanip.hpp \
	gfx/pixbufmanip_hqx.hpp \
	gfx/cellrenderer.hpp \
	gfx/fontmanager.hpp \
	cave/gamerender.hpp \
	cave/titleanimation.hpp \
	framework/app.hpp \
	framework/activity.hpp \
	framework/titlescreenactivity.hpp \
	framework/showtextactivity.hpp \
	framework/messageactivity.hpp \
	framework/gameactivity.hpp \
	framework/selectfileactivity.hpp \
	framework/inputtextactivity.hpp \
	framework/askyesnoactivity.hpp \
	framework/settingsactivity.hpp \
	framework/thememanager.hpp \
	framework/replaymenuactivity.hpp \
	framework/replaysaveractivity.hpp \
	framework/commands.hpp \
	input/joystick.hpp \
	input/gameinputhandler.hpp \
	sound/sound.hpp

basesources = \
	misc/printf.cpp \
	cave/colors.cpp \
	cave/cavetypes.cpp \
	cave/elementproperties.cpp \
	cave/helper/cavereplay.cpp \
	cave/caverendered.cpp \
	cave/particle.cpp \
	cave/caverenderedengine.cpp \
	cave/helper/caverandom.cpp \
	cave/helper/cavesound.cpp \
	cave/helper/cavehighscore.cpp \
	cave/cavebase.cpp \
	cave/cavestored.cpp \
	cave/object/caveobject.cpp \
	cave/object/caveobjectrectangular.cpp \
	cave/object/caveobjectfill.cpp \
	cave/object/caveobjectboundaryfill.cpp \
	cave/object/caveobjectcopypaste.cpp \
	cave/object/caveobjectfillrect.cpp \
	cave/object/caveobjectfloodfill.cpp \
	cave/object/caveobjectjoin.cpp \
	cave/object/caveobjectline.cpp \
	cave/object/caveobjectmaze.cpp \
	cave/object/caveobjectpoint.cpp \
	cave/object/caveobjectrandomfill.cpp \
	cave/object/caveobjectraster.cpp \
	cave/object/caveobjectrectangle.cpp \
	cave/caveset.cpp \
	fileops/bdcffhelper.cpp \
	fileops/bdcffload.cpp \
	fileops/bdcffsave.cpp \
	fileops/c64import.cpp \
	fileops/brcimport.cpp \
	fileops/binaryimport.cpp \
	fileops/exportcrli.cpp \
	fileops/loadfile.cpp \
	fileops/highscore.cpp \
	cave/gamecontrol.cpp \
	settings.cpp \
	misc/util.cpp \
	misc/logger.cpp \
	misc/about.cpp \
	misc/helptext.cpp \
	gfx/pixbuf.cpp \
	gfx/screen.cpp \
	gfx/pixbuffactory.cpp \
	gfx/pixbufmanip.cpp \
	gfx/pixbufmanip_hq2x.cpp \
	gfx/pixbufmanip_hq3x.cpp \
	gfx/pixbufmanip_hq4x.cpp \
	gfx/cellrenderer.cpp \
	gfx/fontmanager.cpp \
	cave/gamerender.cpp \
	cave/titleanimation.cpp \
	framework/app.cpp \
	framework/titlescreenactivity.cpp \
	framework/showtextactivity.cpp \
	framework/messageactivity.cpp \
	framework/gameactivity.cpp \
	framework/selectfileactivity.cpp \
	framework/inputtextactivity.cpp \
	framework/askyesnoactivity.cpp \
	framework/settingsactivity.cpp \
	framework/thememanager.cpp \
	framework/replaymenuactivity.cpp \
	framework/replaysaveractivity.cpp \
	framework/commands.cpp \
	input/joystick.cpp \
	input/gameinputhandler.cpp \
	sound/sound.cpp \
	mainwindow.cpp \
	main.cpp

# gtk sources and headers:
#   the gdash gtk ui, with editor.
gtksources = \
	gtk/gtkpixbuf.cpp \
	gtk/gtkpixbuffactory.cpp \
	gtk/gtkscreen.cpp \
	gtk/gtkui.cpp \
	gtk/gtkuisettings.cpp \
	gtk/gtkgameinputhandler.cpp \
	misc/helphtml.cpp \
	editor/editorwidgets.cpp \
	editor/editorautowidgets.cpp \
	editor/editorcellrenderer.cpp \
	editor/exporthtml.cpp \
	editor/exporttext.cpp \
	editor/editor.cpp \
	gtk/gtkapp.cpp \
	gtk/gtkmainwindow.cpp

gtkheaders = \
	gtk/gtkpixbuf.hpp \
	gtk/gtkpixbuffactory.hpp \
	gtk/gtkscreen.hpp \
	gtk/gtkui.hpp \
	gtk/gtkuisettings.hpp \
	gtk/gtkgameinputhandler.hpp \
	misc/helphtml.hpp \
	editor/editorwidgets.hpp \
	editor/editorautowidgets.hpp \
	editor/editorcellrenderer.hpp \
	editor/exporthtml.hpp \
	editor/exporttext.hpp \
	editor/editor.hpp \
	gtk/gtkapp.hpp \
	gtk/gtkmainwindow.hpp \
	mainwindow.hpp

# sdl sources and headers:
#   the sdl and the opengl engine.
sdlsources = \
	framework/shadermanager.cpp \
	framework/volumeactivity.cpp \
	sdl/sdlpixbuf.cpp \
	sdl/sdlabstractscreen.cpp \
	sdl/sdlscreen.cpp \
	sdl/sdlpixbuffactory.cpp \
	sdl/sdlgameinputhandler.cpp \
	sdl/sdlmainwindow.cpp \
	sdl/ogl.cpp \
	sdl/IMG_savepng.cpp

sdlheaders = \
	framework/shadermanager.hpp \
	framework/volumeactivity.hpp \
	sdl/sdlpixbuf.hpp \
	sdl/sdlabstractscreen.hpp \
	sdl/sdlscreen.hpp \
	sdl/sdlpixbuffactory.hpp \
	sdl/sdlgameinputhandler.hpp \
	sdl/sdlmainwindow.hpp \
	sdl/ogl.hpp \
	sdl/IMG_savepng.hpp



noinst_HEADERS = \
	$(baseheaders) \
	$(gtkheaders) \
	$(sdlheaders)



programheaders=$(baseheaders)
programsources=$(basesources)
if GTK
  programheaders+=$(gtkheaders)
  programsources+=$(gtksources)
endif
if SDL
  programheaders+=$(sdlheaders)
  programsources+=$(sdlsources)
endif




gdash_CPPFLAGS = -g -Wall -std=c++14 @GTK_CFLAGS@ @GLIB_CFLAGS@ @SDL_CFLAGS@ @GL_CFLAGS@ @LIBPNG_CFLAGS@
gdash_LDFLAGS = -g -Wall
gdash_LDADD = @GTK_LIBS@ @GLIB_LIBS@ @LIBINTL@ @SDL_LIBS@ @GL_LIBS@ @LIBPNG_LIBS@
gdash_SOURCES = $(programsources)
