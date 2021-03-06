
dnl use: EVAS_CHECK_ENGINE_DEP_BUFFER(engine, simple, want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_ENGINE_DEP_BUFFER],
[

have_dep="yes"
evas_engine_[]$1[]_cflags=""
evas_engine_[]$1[]_libs=""

AC_SUBST([evas_engine_$1_cflags])
AC_SUBST([evas_engine_$1_libs])

if test "x${have_dep}" = "xyes" ; then
  m4_default([$4], [:])
else
  m4_default([$5], [:])
fi

])

dnl use: EVAS_CHECK_ENGINE_DEP_SOFTWARE_XLIB(engine, simple, want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_ENGINE_DEP_SOFTWARE_XLIB],
[

have_dep="no"
evas_engine_[]$1[]_cflags=""
evas_engine_[]$1[]_libs=""

AC_PATH_X
AC_PATH_XTRA

AC_CHECK_HEADER([X11/X.h], [have_dep="yes"])

if test "x${have_dep}" = "xyes" ; then
   AC_CHECK_LIB([X11], [XCreateImage], [have_dep="yes"], [have_dep="no"])
fi

if test "x${have_dep}" = "xyes" ; then
   AC_CHECK_LIB([Xext], [XShmCreateImage], [have_dep="yes"], [have_dep="no"])
fi

if test "x${have_dep}" = "xyes" ; then
   if test "x$2" = "xyes" ; then
      x_libs="${x_libs} -lX11 -lXext"
   else
      x_dir=${x_dir:-/usr/X11R6}
      x_cflags=${x_cflags:--I${x_includes:-$x_dir/include}}
      x_libs="${x_libs:--L${x_libraries:-$x_dir/lib}} -lX11 -lXext"
   fi
   evas_engine_[]$1[]_cflags="${x_cflags}"
   evas_engine_[]$1[]_libs="${x_libs}"
fi

AC_SUBST([evas_engine_$1_cflags])
AC_SUBST([evas_engine_$1_libs])

if test "x${have_dep}" = "xyes" ; then
  m4_default([$4], [:])
else
  m4_default([$5], [:])
fi

])

dnl use: EVAS_CHECK_ENGINE_DEP_XRENDER_X11(engine, simple, want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_ENGINE_DEP_XRENDER_X11],
[

evas_engine_[]$1[]_cflags=""
evas_engine_[]$1[]_libs=""

AC_PATH_X
AC_PATH_XTRA

AC_CHECK_HEADERS([X11/Xlib.h X11/extensions/Xrender.h],
   [have_dep="yes"],
   [have_dep="no"; break;])

if test "x${have_dep}" = "xyes" ; then
   AC_CHECK_LIB([X11], [XCreateImage], [have_dep="yes"], [have_dep="no"])
fi

if test "x${have_dep}" = "xyes" ; then
   AC_CHECK_LIB([Xext], [XShmCreateImage], [have_dep="yes"], [have_dep="no"])
fi

if test "x${have_dep}" = "xyes" ; then
   AC_CHECK_LIB([Xrender], [XRenderCreatePicture], [have_dep="yes"], [have_dep="no"])
fi

if test "x${have_dep}" = "xyes" ; then
   if test "x$2" = "xyes" ; then
      x_libs="${x_libs} -lX11 -lXext"
   else
      x_dir=${x_dir:-/usr/X11R6}
      x_cflags=${x_cflags:--I${x_includes:-$x_dir/include}}
      x_libs="${x_libs:--L${x_libraries:-$x_dir/lib}} -lX11 -lXext"
   fi
   evas_engine_[]$1[]_cflags="${x_cflags}"
   evas_engine_[]$1[]_libs="${x_libs} -lXrender"
fi

AC_SUBST([evas_engine_$1_cflags])
AC_SUBST([evas_engine_$1_libs])

if test "x${have_dep}" = "xyes" ; then
  m4_default([$4], [:])
else
  m4_default([$5], [:])
fi

])

dnl use: EVAS_CHECK_ENGINE_DEP_GL_X11(engine, simple, want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_ENGINE_DEP_GL_X11],
[

evas_engine_[]$1[]_cflags=""
evas_engine_[]$1[]_libs=""

AC_PATH_X
AC_PATH_XTRA

AC_CHECK_HEADER([GL/gl.h],
   [have_dep="yes"],
   [have_dep="no"],
   [
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glx.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xrender.h>
#include <X11/Xresource.h>
   ])

gl_pt_lib="";
have_gl_pt="no"
AC_CHECK_LIB([pthread], [pthread_create], [have_gl_pt="yes"], [have_gl_pt="no"])
if test "x$have_gl_pt" = "xyes" ; then
   gl_pt_lib=" -lpthread"
fi

if test "x${have_dep}" = "xyes" ; then
   AC_CHECK_LIB([X11], [XCreateColormap], [have_dep="yes"], [have_dep="no"])
fi

if test "x${have_dep}" = "xyes" ; then
   AC_CHECK_LIB([Xrender], [XRenderCreatePicture], [have_dep="yes"], [have_dep="no"])
fi

if test "x${have_dep}" = "xyes" ; then
   AC_CHECK_LIB([GL], [glXCreateContext], [have_dep="yes"], [have_dep="no"], -lX11 -lXext -lXrender -lm $gl_pt_lib)
fi

if test "x$gl_flavor_gles" = "xyes" ; then
  have_dep=no
fi

if test "x${have_dep}" = "xyes" ; then
   if test "x$2" = "xyes" ; then
      x_libs="${x_libs} -lX11 -lXext -lXrender"
   else
      x_dir=${x_dir:-/usr/X11R6}
      x_cflags=${x_cflags:--I${x_includes:-$x_dir/include}}
      x_libs="${x_libs:--L${x_libraries:-$x_dir/lib}} -lX11 -lXext -lXrender"
   fi
   evas_engine_[]$1[]_cflags="-I/usr/include ${x_cflags}"
   evas_engine_[]$1[]_libs="${x_libs} -lGL $gl_pt_lib"
   evas_engine_gl_common_libs="-lGL $gl_pt_lib"
else
   if test "x$2" = "xyes" ; then
      x_libs="${x_libs} -lX11 -lXext -lXrender"
   else
      x_dir=${x_dir:-/usr/X11R6}
      x_cflags=${x_cflags:--I${x_includes:-$x_dir/include}}
      x_libs="${x_libs:--L${x_libraries:-$x_dir/lib}} -lX11 -lXext -lXrender"
   fi
   AC_CHECK_HEADER([GLES2/gl2.h],
      [have_egl="yes"],
      [have_egl="no"],
      [
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xrender.h>
#include <X11/Xresource.h>
      ])
   if test "x${have_egl}" = "xyes" ; then
      AC_CHECK_LIB(GLESv2, glTexImage2D, [have_glesv2="yes"], , -lEGL ${x_libs} -lm $gl_pt_lib)
      if test "x${have_glesv2}" = "xyes" ; then
         evas_engine_[]$1[]_cflags="${x_cflags}"
         evas_engine_[]$1[]_libs="${x_libs} -lGLESv2 -lEGL -lm $gl_pt_lib"
         evas_engine_gl_common_libs="-lGLESv2 -lm $gl_pt_lib"
         have_dep="yes"
         gl_flavor_gles="no"
         AC_DEFINE(GLES_VARIETY_SGX, 1, [Imagination SGX GLES2 support])
         gles_variety_sgx="yes"
      fi
   fi
fi

AC_SUBST([evas_engine_$1_cflags])
AC_SUBST([evas_engine_$1_libs])


if test "x${have_dep}" = "xyes" ; then
  m4_default([$4], [:])
else
  m4_default([$5], [:])
fi

])

dnl use: EVAS_CHECK_ENGINE_DEP_CAIRO_X11(engine, simple, want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_ENGINE_DEP_CAIRO_X11],
[

requirement=""
have_dep="no"
evas_engine_[]$1[]_cflags=""
evas_engine_[]$1[]_libs=""

AC_PATH_X
AC_PATH_XTRA

AC_CHECK_HEADER([X11/X.h],
   [PKG_CHECK_MODULES([CAIRO],
       [cairo >= 1.0.0],
       [have_dep="yes" requirement="cairo"]
    )]
)

if test "x${have_dep}" = "xyes" ; then
   if test "x$2" = "xyes" ; then
      x_libs="${x_libs} -lX11 -lXext"
   else
      x_dir=${x_dir:-/usr/X11R6}
      x_cflags=${x_cflags:--I${x_includes:-$x_dir/include}}
      x_libs="${x_libs:--L${x_libraries:-$x_dir/lib}} -lX11 -lXext"
   fi
   evas_engine_[]$1[]_cflags="${CAIRO_CFLAGS} ${x_cflags}"
   evas_engine_[]$1[]_libs="${CAIRO_LIBS} ${x_libs}"
fi

AC_SUBST([evas_engine_$1_cflags])
AC_SUBST([evas_engine_$1_libs])

if test "x$3" = "xstatic" ; then
   requirement_evas="${requirement} ${requirement_evas}"
fi

if test "x${have_dep}" = "xyes" ; then
  m4_default([$4], [:])
else
  m4_default([$5], [:])
fi

])

dnl use: EVAS_CHECK_ENGINE_DEP_SOFTWARE_XCB(engine, simple, want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_ENGINE_DEP_SOFTWARE_XCB],
[

requirement=""
have_dep="no"
evas_engine_[]$1[]_cflags=""
evas_engine_[]$1[]_libs=""

PKG_CHECK_MODULES([XCB],
   [xcb xcb-shm xcb-image >= 0.2.1 pixman-1],
   [
    have_dep="yes"
    requirement="xcb xcb-shm xcb-image pixman-1"
    evas_engine_[]$1[]_cflags="${XCB_CFLAGS}"
    evas_engine_[]$1[]_libs="${XCB_LIBS}"
   ],[
    have_dep="no"
   ]
)

AC_SUBST([evas_engine_$1_cflags])
AC_SUBST([evas_engine_$1_libs])

if test "x$3" = "xstatic" ; then
   requirement_evas="${requirement} ${requirement_evas}"
fi

if test "x${have_dep}" = "xyes" ; then
  m4_default([$4], [:])
else
  m4_default([$5], [:])
fi

])

dnl use: EVAS_CHECK_ENGINE_DEP_XRENDER_XCB(engine, simple, want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_ENGINE_DEP_XRENDER_XCB],
[

requirement=""
have_dep="no"
evas_engine_[]$1[]_cflags=""
evas_engine_[]$1[]_libs=""

PKG_CHECK_MODULES([XCBRENDER],
   [xcb xcb-shm xcb-render xcb-image >= 0.2.1 pixman-1],
   [
    have_dep="yes"
    requirement="xcb xcb-shm xcb-render xcb-image pixman-1"
    evas_engine_[]$1[]_cflags="${XCBRENDER_CFLAGS}"
    evas_engine_[]$1[]_libs="${XCBRENDER_LIBS}"
   ],[
    have_dep="no"
   ]
)

AC_SUBST([evas_engine_$1_cflags])
AC_SUBST([evas_engine_$1_libs])

if test "x$3" = "xstatic" ; then
   requirement_evas="${requirement} ${requirement_evas}"
fi

if test "x${have_dep}" = "xyes" ; then
  m4_default([$4], [:])
else
  m4_default([$5], [:])
fi

])

dnl use: EVAS_CHECK_ENGINE_DEP_SOFTWARE_GDI(engine, simple, want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_ENGINE_DEP_SOFTWARE_GDI],
[

have_dep="no"
evas_engine_[]$1[]_cflags=""
evas_engine_[]$1[]_libs=""

AC_CHECK_HEADER([windows.h],
   [
    have_dep="yes"
    evas_engine_[]$1[]_libs="-lgdi32"
   ]
)

AC_SUBST([evas_engine_$1_cflags])
AC_SUBST([evas_engine_$1_libs])

if test "x${have_dep}" = "xyes" ; then
  m4_default([$4], [:])
else
  m4_default([$5], [:])
fi

])

dnl use: EVAS_CHECK_ENGINE_DEP_SOFTWARE_DDRAW(engine, simple, want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_ENGINE_DEP_SOFTWARE_DDRAW],
[

have_dep="no"
evas_engine_[]$1[]_cflags=""
evas_engine_[]$1[]_libs=""

AC_CHECK_HEADER([ddraw.h],
   [
    have_dep="yes"
    evas_engine_[]$1[]_libs="-lddraw"
   ]
)

AC_SUBST([evas_engine_$1_cflags])
AC_SUBST([evas_engine_$1_libs])

if test "x${have_dep}" = "xyes" ; then
  m4_default([$4], [:])
else
  m4_default([$5], [:])
fi

])

dnl use: EVAS_CHECK_ENGINE_DEP_DIRECT3D(engine, simple, want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_ENGINE_DEP_DIRECT3D],
[

evas_engine_[]$1[]_cflags=""
evas_engine_[]$1[]_libs=""

AC_CHECK_HEADERS([d3d9.h d3dx9.h],
   [
    have_dep="yes"
    evas_engine_[]$1[]_libs="-ld3d9 -ld3dx9 -lgdi32"
   ],
   [have_dep="no"; break]
)

AC_SUBST([evas_engine_$1_cflags])
AC_SUBST([evas_engine_$1_libs])

if test "x${have_dep}" = "xyes" ; then
  m4_default([$4], [:])
else
  m4_default([$5], [:])
fi

])

dnl use: EVAS_CHECK_ENGINE_DEP_QUARTZ(engine, simple, want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_ENGINE_DEP_QUARTZ],
[

have_dep="no"
evas_engine_[]$1[]_cflags=""
evas_engine_[]$1[]_libs=""

AC_REQUIRE([EVAS_MAYBE_GET_OBJCPP])

AS_IF([test "x${rw_cv_prog_objc_works}" = "xyes"],
[
   AC_LANG_PUSH([Objective C]) 
   AC_CHECK_HEADER([/System/Library/Frameworks/Cocoa.framework/Headers/Cocoa.h],
      [
       have_dep="yes"
       evas_engine_[]$1[]_libs="-framework Cocoa"
      ],
      [have_dep="no"])
   AC_LANG_POP([Objective C]) 

])

AC_SUBST([evas_engine_$1_cflags])
AC_SUBST([evas_engine_$1_libs])

if test "x${have_dep}" = "xyes" ; then
  m4_default([$4], [:])
else
  m4_default([$5], [:])
fi

])

dnl Helper macro for EVAS_CHECK_ENGINE_DEP_QUARTZ

AC_DEFUN([EVAS_MAYBE_GET_OBJCPP],
[AS_IF([test "x${rw_cv_prog_objc_works}" = "xyes"],
       [AC_PROG_OBJCPP])
])

dnl use: EVAS_CHECK_ENGINE_DEP_GL_GLEW(engine, simple, want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_ENGINE_DEP_GL_GLEW],
[

evas_engine_[]$1[]_cflags=""
evas_engine_[]$1[]_libs=""

AC_CHECK_HEADERS([GL/gl.h GL/glew.h],
   [
    have_dep="yes"
    evas_engine_[]$1[]_libs="-lglew32 -lopengl32 -lgdi32"
   ],
   [have_dep="no"; break;]
)

AC_SUBST([evas_engine_$1_cflags])
AC_SUBST([evas_engine_$1_libs])

if test "x${have_dep}" = "xyes" ; then
  m4_default([$4], [:])
else
  m4_default([$5], [:])
fi

])

dnl use: EVAS_CHECK_ENGINE_DEP_SOFTWARE_SDL(engine, simple, want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_ENGINE_DEP_SOFTWARE_SDL],
[

requirement=""
have_dep="no"
evas_engine_[]$1[]_cflags=""
evas_engine_[]$1[]_libs=""

PKG_CHECK_MODULES([SDL],
   [sdl >= 1.2.0],
   [
    have_dep="yes"
    requirement="sdl"
    evas_engine_[]$1[]_cflags="${SDL_CFLAGS}"
    evas_engine_[]$1[]_libs="${SDL_LIBS}"
   ]
)

AC_SUBST([evas_engine_$1_cflags])
AC_SUBST([evas_engine_$1_libs])

if test "x$3" = "xstatic" ; then
   requirement_evas="${requirement} ${requirement_evas}"
fi

if test "x${have_dep}" = "xyes" ; then
  m4_default([$4], [:])
else
  m4_default([$5], [:])
fi

])

dnl use: EVAS_CHECK_ENGINE_DEP_GL_SDL(engine, simple, want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_ENGINE_DEP_GL_SDL],
[

requirement=""
have_dep="no"
evas_engine_[]$1[]_cflags=""
evas_engine_[]$1[]_libs=""

PKG_CHECK_MODULES([SDL],
   [sdl >= 1.2.0],
   [
    have_dep="yes"
    requirement="sdl"
    evas_engine_[]$1[]_cflags="${SDL_CFLAGS}"
    evas_engine_[]$1[]_libs="${SDL_LIBS}"
   ]
)

gl_pt_lib="";
have_gl_pt="no"
AC_CHECK_LIB([pthread], [pthread_create], [have_gl_pt="yes"], [have_gl_pt="no"])
if test "x$have_gl_pt" = "xyes" ; then
   gl_pt_lib=" -lpthread"
fi

AC_CHECK_HEADER([GL/gl.h],
   [have_dep="yes"],
   [have_dep="no"],
   [
#include <GL/gl.h>
#include <GL/glext.h>
   ])

if test "x$gl_flavor_gles" = "xyes" ; then
  have_dep=no
fi

if test "x${have_dep}" = "xyes" ; then
   evas_engine_[]$1[]_cflags="${SDL_CFLAGS}"
   evas_engine_[]$1[]_libs="${SDL_LIBS} -lGL -lm $gl_pt_lib"
   evas_engine_gl_common_libs="-lGL -lm $gl_pt_lib"
else
   AC_CHECK_HEADER([SDL/SDL_opengles.h],
      [have_egl="yes"],
      [have_egl="no"],
      [
#include <SDL/SDL_opengles.h>
#include <EGL/egl.h>
      ])
   if test "x${have_egl}" = "xyes" ; then
      AC_CHECK_LIB(GLESv2, glTexImage2D, [have_glesv2="yes"], , -lEGL -lm $gl_pt_lib)
      if test "x${have_glesv2}" = "xyes" ; then
         evas_engine_[]$1[]_cflags="${SDL_CFLAGS}"
         evas_engine_[]$1[]_libs="${SDL_LIBS} -lGLESv2 -lEGL -lm $gl_pt_lib"
         evas_engine_gl_common_libs="-lGLESv2 -lm $gl_pt_lib"
         have_dep="yes"
         gl_flavor_gles="no"
         AC_DEFINE(GLES_VARIETY_SGX, 1, [Imagination SGX GLES2 support])
         gles_variety_sgx="yes"
      fi
   fi
fi

AC_SUBST([evas_engine_$1_cflags])
AC_SUBST([evas_engine_$1_libs])

if test "x$3" = "xstatic" ; then
   requirement_evas="${requirement} ${requirement_evas}"
fi

if test "x${have_dep}" = "xyes" ; then
  m4_default([$4], [:])
else
  m4_default([$5], [:])
fi

])

dnl use: EVAS_CHECK_ENGINE_DEP_FB(engine, simple, want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_ENGINE_DEP_FB],
[

have_dep="no"
evas_engine_[]$1[]_cflags=""
evas_engine_[]$1[]_libs=""

AC_CHECK_HEADER([linux/fb.h], [have_dep="yes"])

AC_SUBST([evas_engine_$1_cflags])
AC_SUBST([evas_engine_$1_libs])

if test "x${have_dep}" = "xyes" ; then
  m4_default([$4], [:])
else
  m4_default([$5], [:])
fi

])

dnl use: EVAS_CHECK_ENGINE_DEP_DIRECTFB(engine, simple, want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_ENGINE_DEP_DIRECTFB],
[

requirement=""
have_dep="no"
evas_engine_[]$1[]_cflags=""
evas_engine_[]$1[]_libs=""

PKG_CHECK_MODULES([DIRECTFB],
   [directfb >= 0.9.16],
   [
    have_dep="yes"
    requirement="directfb"
    evas_engine_[]$1[]_cflags="${DIRECTFB_CFLAGS}"
    evas_engine_[]$1[]_libs="${DIRECTFB_LIBS}"
   ]
)

AC_SUBST([evas_engine_$1_cflags])
AC_SUBST([evas_engine_$1_libs])

if test "x$3" = "xstatic" ; then
   requirement_evas="${requirement} ${requirement_evas}"
fi

if test "x${have_dep}" = "xyes" ; then
  m4_default([$4], [:])
else
  m4_default([$5], [:])
fi

])

dnl use: EVAS_CHECK_ENGINE_DEP_SOFTWARE_QTOPIA(engine, simple, want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_ENGINE_DEP_SOFTWARE_QTOPIA],
[

have_dep="no"
evas_engine_[]$1[]_cflags=""
evas_engine_[]$1[]_libs=""
evas_engine_[]$1[]_moc=""

AC_ARG_WITH([qtdir],
   [AC_HELP_STRING([--with-qtdir=QT_DIR], [use qt directory specified])],
   [
    qt_dir=$withval;
    echo "using "$qt_dir" for qt directory.";
   ],
   [qt_dir="/opt/Qtopia"]
)

AC_LANG_PUSH(C++)

AC_CHECK_HEADER([qdirectpainter_qws.h],
   [have_dep="yes"],
   [have_dep="no"],
   [
#include <qwidget.h>
#include <qnamespace.h>
#include <qbrush.h>
#include <qpainter.h>
   ]
)

AC_LANG_POP(C++)

if test "x${have_dep}" = "xyes" ; then
   evas_engine_[]$1[]_cflags="-fno-exceptions -fno-rtti -I${qt_dir}/include"
   evas_engine_[]$1[]_libs="-L${qt_dir}/lib -lqte -lqpe"
   evas_engine_[]$1[]_moc="${qt_dir}/bin/moc"
fi

AC_SUBST([evas_engine_$1_cflags])
AC_SUBST([evas_engine_$1_libs])
AC_SUBST([evas_engine_$1_moc])

if test "x${have_dep}" = "xyes" ; then
  m4_default([$4], [:])
else
  m4_default([$5], [:])
fi

])

dnl use: EVAS_CHECK_ENGINE_DEP_SOFTWARE_8_X11(engine, simple, want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_ENGINE_DEP_SOFTWARE_8_X11],
[

have_dep="no"
evas_engine_[]$1[]_cflags=""
evas_engine_[]$1[]_libs=""

PKG_CHECK_MODULES([XCB],
   [xcb xcb-shm xcb-image >= 0.2.1 pixman-1],
   [
    have_dep="yes"
    requirement="xcb xcb-shm xcb-image pixman-1"
    evas_engine_[]$1[]_cflags="${XCB_CFLAGS}"
    evas_engine_[]$1[]_libs="${XCB_LIBS}"
   ],[
    have_dep="no"
   ]
)

AC_SUBST([evas_engine_$1_cflags])
AC_SUBST([evas_engine_$1_libs])

if test "x$3" = "xstatic" ; then
   requirement_evas="${requirement} ${requirement_evas}"
fi

if test "x${have_dep}" = "xyes" ; then
  m4_default([$4], [:])
else
  m4_default([$5], [:])
fi

])

dnl use: EVAS_CHECK_ENGINE_DEP_SOFTWARE_16_X11(engine, simple, want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])
AC_DEFUN([EVAS_CHECK_ENGINE_DEP_SOFTWARE_16_X11],
[

have_dep="no"
evas_engine_[]$1[]_cflags=""
evas_engine_[]$1[]_libs=""

AC_PATH_X
AC_PATH_XTRA

AC_CHECK_HEADER([X11/X.h], [have_dep="yes"])

if test "x${have_dep}" = "xyes" ; then
   AC_CHECK_LIB([X11], [XCreateImage], [have_dep="yes"], [have_dep="no"])
fi

if test "x${have_dep}" = "xyes" ; then
   AC_CHECK_LIB([Xext], [XShmCreateImage], [have_dep="yes"], [have_dep="no"])
fi

if test "x${have_dep}" = "xyes" ; then
   if test "x$2" = "xyes" ; then
      x_libs="${x_libs} -lX11 -lXext"
   else
      x_dir=${x_dir:-/usr/X11R6}
      x_cflags=${x_cflags:--I${x_includes:-$x_dir/include}}
      x_libs="${x_libs:--L${x_libraries:-$x_dir/lib}} -lX11 -lXext"
   fi
   evas_engine_[]$1[]_cflags="${x_cflags}"
   evas_engine_[]$1[]_libs="${x_libs}"
fi

AC_SUBST([evas_engine_$1_cflags])
AC_SUBST([evas_engine_$1_libs])

if test "x${have_dep}" = "xyes" ; then
  m4_default([$4], [:])
else
  m4_default([$5], [:])
fi

])

dnl use: EVAS_CHECK_ENGINE_DEP_SOFTWARE_16_DDRAW(engine, simple, want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_ENGINE_DEP_SOFTWARE_16_DDRAW],
[

have_dep="no"
evas_engine_[]$1[]_cflags=""
evas_engine_[]$1[]_libs=""

AC_CHECK_HEADER([ddraw.h],
   [
    have_dep="yes"
    evas_engine_[]$1[]_libs="-lddraw -lgdi32"
   ]
)

AC_SUBST([evas_engine_$1_cflags])
AC_SUBST([evas_engine_$1_libs])

if test "x${have_dep}" = "xyes" ; then
  m4_default([$4], [:])
else
  m4_default([$5], [:])
fi

])

dnl use: EVAS_CHECK_ENGINE_DEP_SOFTWARE_16_WINCE(engine, simple, want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_ENGINE_DEP_SOFTWARE_16_WINCE],
[

have_dep="yes"
evas_engine_[]$1[]_cflags=""
evas_engine_[]$1[]_libs=""

AC_SUBST([evas_engine_$1_cflags])
AC_SUBST([evas_engine_$1_libs])

if test "x${have_dep}" = "xyes" ; then
  m4_default([$4], [:])
else
  m4_default([$5], [:])
fi

])

dnl use: EVAS_CHECK_ENGINE(engine, want_engine, simple, description)


AC_DEFUN([EVAS_CHECK_ENGINE],
[

m4_pushdef([UP], m4_translit([$1], [-a-z], [_A-Z]))dnl
m4_pushdef([DOWN], m4_translit([$1], [-A-Z], [_a-z]))dnl

want_engine="$2"
want_static_engine="no"
have_engine="no"
have_evas_engine_[]DOWN="no"

AC_ARG_ENABLE([$1],
   [AC_HELP_STRING([--enable-$1], [enable $4 rendering backend])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_engine="yes"
    else
       if test "x${enableval}" = "xstatic" ; then
          want_engine="static"
       else
          want_engine="no"
       fi
    fi
   ])

AC_MSG_CHECKING([whether to enable $4 rendering backend])
AC_MSG_RESULT([${want_engine}])

if test "x${want_engine}" = "xyes" -o "x${want_engine}" = "xstatic" -o "x${want_engine}" = "xauto" ; then
   m4_default([EVAS_CHECK_ENGINE_DEP_]m4_defn([UP]))(DOWN, $3, ${want_engine}, [have_engine="yes"], [have_engine="no"])
fi

if test "x${have_engine}" = "xno" -a "x${want_engine}" = "xyes" -a "x${use_strict}" = "xyes" ; then
   AC_MSG_ERROR([$4 dependencies not found (strict dependencies checking)])
fi

AC_MSG_CHECKING([whether $4 rendering backend will be built])
AC_MSG_RESULT([${have_engine}])

if test "x${have_engine}" = "xyes" ; then
   if test "x${want_engine}" = "xstatic" ; then
      have_evas_engine_[]DOWN="static"
      want_static_engine="yes"
   else
      have_evas_engine_[]DOWN="yes"
   fi
fi

if test "x${have_engine}" = "xyes" ; then
   AC_DEFINE(BUILD_ENGINE_[]UP, [1], [$4 rendering backend])
fi

AM_CONDITIONAL(BUILD_ENGINE_[]UP, [test "x${have_engine}" = "xyes"])

if test "x${want_static_engine}" = "xyes" ; then
   AC_DEFINE(EVAS_STATIC_BUILD_[]UP, [1], [Build $1 engine inside libevas])
   have_static_module="yes"
fi

AM_CONDITIONAL(EVAS_STATIC_BUILD_[]UP, [test "x${want_static_engine}" = "xyes"])

m4_popdef([UP])
m4_popdef([DOWN])

])
