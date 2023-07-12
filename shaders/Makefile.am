## Process this file with automake to produce Makefile.in

shadersdata = \
	crt-caligari.shader \
	crt-cgwg.shader \
	crt-geom.shader \
	crt-trinitron.shader \
	crt-trinitron2.shader \
	Curvature.shader \
	Dot_N_Bloom.shader \
	GDash-TV.shader \
	Phosphor-simple.shader \
	templateshader \
	xbrz4x.shader \
	xbrz6x.shader

if SDL
  shadersdir = $(pkgdatadir)/shaders
  shaders_DATA = $(shadersdata)
endif

EXTRA_DIST = $(shadersdata)
