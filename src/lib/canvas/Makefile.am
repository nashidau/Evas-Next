MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
-I$(top_srcdir)/src/lib/cserve \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
@WIN32_CPPFLAGS@ \
@FREETYPE_CFLAGS@ \
@EET_CFLAGS@ \
@FONTCONFIG_CFLAGS@ \
@EINA_CFLAGS@ \
@EVIL_CFLAGS@

noinst_LTLIBRARIES   = libevas_canvas.la
libevas_canvas_la_SOURCES  = \
evas_callbacks.c \
evas_clip.c \
evas_data.c \
evas_events.c \
evas_focus.c \
evas_key.c \
evas_key_grab.c \
evas_layer.c \
evas_main.c \
evas_name.c \
evas_object_image.c \
evas_object_main.c \
evas_object_inform.c \
evas_object_intercept.c \
evas_object_line.c \
evas_object_polygon.c \
evas_object_rectangle.c \
evas_object_smart.c \
evas_object_smart_clipped.c \
evas_object_box.c \
evas_object_table.c \
evas_object_text.c \
evas_object_textblock.c \
evas_font_dir.c \
evas_rectangle.c \
evas_render.c \
evas_smart.c \
evas_stack.c \
evas_async_events.c \
evas_transform.c \
evas_stats.c \
evas_map.c

libevas_canvas_la_LIBADD = @EVIL_LIBS@

