#ifndef _EVAS_H
#define _EVAS_H

#include <time.h>

#include <Eina.h>

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_EVAS_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_EVAS_BUILD */
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif
#endif /* ! _WIN32 */

#ifdef __cplusplus
extern "C" {
#endif

#define EVAS_VERSION_MAJOR 1
#define EVAS_VERSION_MINOR 0

typedef struct _Evas_Version
{
   int major;
   int minor;
   int micro;
   int revision;
} Evas_Version;

EAPI extern Evas_Version *evas_version;

/**
 * @file
 * @brief These routines are used for Evas library interaction.
 *
 * @todo check boolean return values and convert to Eina_Bool
 * @todo change all api to use EINA_SAFETY_*
 * @todo finish api documentation
 */

/* BiDi exposed stuff */
   /*FIXME: document */
typedef enum _Evas_BiDi_Direction
{
   EVAS_BIDI_DIRECTION_NATURAL,
   EVAS_BIDI_DIRECTION_LTR,
   EVAS_BIDI_DIRECTION_RTL
} Evas_BiDi_Direction;

/**
 * Identifier of callbacks to be used with object or canvas.
 *
 * @see evas_object_event_callback_add()
 * @see evas_event_callback_add()
 */
typedef enum _Evas_Callback_Type
{
   /*
    * The following events are only for use with objects
    * evas_object_event_callback_add():
    */
   EVAS_CALLBACK_MOUSE_IN, /**< Mouse In Event */
   EVAS_CALLBACK_MOUSE_OUT, /**< Mouse Out Event */
   EVAS_CALLBACK_MOUSE_DOWN, /**< Mouse Button Down Event */
   EVAS_CALLBACK_MOUSE_UP, /**< Mouse Button Up Event */
   EVAS_CALLBACK_MOUSE_MOVE, /**< Mouse Move Event */
   EVAS_CALLBACK_MOUSE_WHEEL, /**< Mouse Wheel Event */
   EVAS_CALLBACK_MULTI_DOWN, /**< Multi-touch Down Event */
   EVAS_CALLBACK_MULTI_UP, /**< Multi-touch Up Event */
   EVAS_CALLBACK_MULTI_MOVE, /**< Multi-touch Move Event */
   EVAS_CALLBACK_FREE, /**< Object Being Freed (Called after Del) */
   EVAS_CALLBACK_KEY_DOWN, /**< Key Press Event */
   EVAS_CALLBACK_KEY_UP, /**< Key Release Event */
   EVAS_CALLBACK_FOCUS_IN, /**< Focus In Event */
   EVAS_CALLBACK_FOCUS_OUT, /**< Focus Out Event */
   EVAS_CALLBACK_SHOW, /**< Show Event */
   EVAS_CALLBACK_HIDE, /**< Hide Event */
   EVAS_CALLBACK_MOVE, /**< Move Event */
   EVAS_CALLBACK_RESIZE, /**< Resize Event */
   EVAS_CALLBACK_RESTACK, /**< Restack Event */
   EVAS_CALLBACK_DEL, /**< Object Being Deleted (called before Free) */
   EVAS_CALLBACK_HOLD, /**< Events go on/off hold */
   EVAS_CALLBACK_CHANGED_SIZE_HINTS, /**< Size hints changed event */
   EVAS_CALLBACK_IMAGE_PRELOADED, /**< Image as been preloaded */


   /*
    * The following events are only for use with canvas
    * evas_event_callback_add():
    */

   EVAS_CALLBACK_CANVAS_FOCUS_IN, /**< Canvas got focus as a whole */
   EVAS_CALLBACK_CANVAS_FOCUS_OUT, /**< Canvas lost focus as a whole */
   EVAS_CALLBACK_RENDER_FLUSH_PRE, /**< Called just before rendering is updated on the canvas target */
   EVAS_CALLBACK_RENDER_FLUSH_POST, /**< Called just after rendering is updated on the canvas target */

   EVAS_CALLBACK_CANVAS_OBJECT_FOCUS_IN, /**< Canvas object got focus */
   EVAS_CALLBACK_CANVAS_OBJECT_FOCUS_OUT, /**< Canvas object lost focus */

   /* the following id no event number, but a sentinel: */
   EVAS_CALLBACK_LAST /**< keep as last element/sentinel -- not really an event */
} Evas_Callback_Type; /**< The type of event to trigger the callback */

/**
 * Flags for Mouse Button events
 */
typedef enum _Evas_Button_Flags
{
   EVAS_BUTTON_NONE = 0, /**< No extra mouse button data */
   EVAS_BUTTON_DOUBLE_CLICK = (1 << 0), /**< This mouse button press was the 2nd press of a double click */
   EVAS_BUTTON_TRIPLE_CLICK = (1 << 1) /**< This mouse button press was the 3rd press of a triple click */
} Evas_Button_Flags; /**< Flags for Mouse Button events */

/**
 * Flags for Events
 */
typedef enum _Evas_Event_Flags
{
   EVAS_EVENT_FLAG_NONE = 0, /**< No fancy flags set */
   EVAS_EVENT_FLAG_ON_HOLD = (1 << 0), /**< This event is being delivered but should be put "on hold" until the on hold flag is unset. the event should be used for informational purposes and maybe some indications visually, but not actually perform anything */
   EVAS_EVENT_FLAG_ON_SCROLL = (1 << 1) /**< This event flag indicates the event occurs while scrolling; for exameple, DOWN event occurs during scrolling; the event should be used for informational purposes and maybe some indications visually, but not actually perform anything */
} Evas_Event_Flags; /**< Flags for Events */

/**
 * Flags for Font Hinting
 * @ingroup Evas_Font_Group
 */
typedef enum _Evas_Font_Hinting_Flags
{
   EVAS_FONT_HINTING_NONE, /**< No font hinting */
   EVAS_FONT_HINTING_AUTO, /**< Automatic font hinting */
   EVAS_FONT_HINTING_BYTECODE /**< Bytecode font hinting */
} Evas_Font_Hinting_Flags; /**< Flags for Font Hinting */

/**
 * Colorspaces for pixel data supported by Evas
 * @ingroup Evas_Object_Image
 */
typedef enum _Evas_Colorspace
{
   EVAS_COLORSPACE_ARGB8888, /**< ARGB 32 bits per pixel, high-byte is Alpha, accessed 1 32bit word at a time */
     /* these are not currently supported - but planned for the future */
   EVAS_COLORSPACE_YCBCR422P601_PL, /**< YCbCr 4:2:2 Planar, ITU.BT-601 specifications. The data poitned to is just an array of row pointer, pointing to the Y rows, then the Cb, then Cr rows */
   EVAS_COLORSPACE_YCBCR422P709_PL,/**< YCbCr 4:2:2 Planar, ITU.BT-709 specifications. The data poitned to is just an array of row pointer, pointing to the Y rows, then the Cb, then Cr rows */
   EVAS_COLORSPACE_RGB565_A5P, /**< 16bit rgb565 + Alpha plane at end - 5 bits of the 8 being used per alpha byte */
   EVAS_COLORSPACE_GRY8 /**< 8bit grayscale */
} Evas_Colorspace; /**< Colorspaces for pixel data supported by Evas */

/**
 * How to pack items into cells in a table.
 * @ingroup Evas_Object_Table
 */
typedef enum _Evas_Object_Table_Homogeneous_Mode
{
  EVAS_OBJECT_TABLE_HOMOGENEOUS_NONE = 0,
  EVAS_OBJECT_TABLE_HOMOGENEOUS_TABLE = 1,
  EVAS_OBJECT_TABLE_HOMOGENEOUS_ITEM = 2
} Evas_Object_Table_Homogeneous_Mode; /**< Table cell pack mode. */

typedef struct _Evas_Coord_Rectangle  Evas_Coord_Rectangle; /**< A generic rectangle handle */
typedef struct _Evas_Point                   Evas_Point; /**< integer point */

typedef struct _Evas_Coord_Point             Evas_Coord_Point;  /**< Evas_Coord point */
typedef struct _Evas_Coord_Precision_Point   Evas_Coord_Precision_Point; /**< Evas_Coord point with sub-pixel precision */

typedef struct _Evas_Position                Evas_Position; /**< associates given point in Canvas and Output */
typedef struct _Evas_Precision_Position      Evas_Precision_Position; /**< associates given point in Canvas and Output, with sub-pixel precision */

/**
 * @typedef Evas_Smart_Class
 * A smart object base class
 * @ingroup Evas_Smart_Group
 */
typedef struct _Evas_Smart_Class             Evas_Smart_Class;

/**
 * @typedef Evas_Smart_Cb_Description
 * A smart object callback description, used to provide introspection
 * @ingroup Evas_Smart_Group
 */
typedef struct _Evas_Smart_Cb_Description    Evas_Smart_Cb_Description;

/**
 * @typedef Evas_Map
 * An opaque handle to map points
 * @see evas_map_new()
 * @see evas_map_free()
 * @see evas_map_dup()
 * @ingroup Evas_Object_Group_Map
 */
typedef struct _Evas_Map            Evas_Map;

/**
 * @typedef Evas
 * An Evas canvas handle.
 * @see evas_new()
 * @see evas_free()
 * @ingroup Evas_Canvas
 */
typedef struct _Evas                Evas;

/**
 * @typedef Evas_Object
 * An Evas Object handle.
 * @ingroup Evas_Object_Group
 */
typedef struct _Evas_Object         Evas_Object;

typedef void                        Evas_Performance; /**< An Evas Performance handle */
typedef struct _Evas_Modifier       Evas_Modifier; /**< An Evas Modifier */
typedef struct _Evas_Lock           Evas_Lock; /**< An Evas Lock */
typedef struct _Evas_Smart          Evas_Smart; /**< An Evas Smart Object handle */
typedef struct _Evas_Native_Surface Evas_Native_Surface; /**< A generic datatype for engine specific native surface information */
typedef unsigned long long          Evas_Modifier_Mask; /**< An Evas modifier mask type */

typedef int                         Evas_Coord;
typedef int                         Evas_Font_Size;
typedef int                         Evas_Angle;

struct _Evas_Coord_Rectangle /**< A rectangle in Evas_Coord */
{
   Evas_Coord x; /**< top-left x co-ordinate of rectangle */
   Evas_Coord y; /**< top-left y co-ordinate of rectangle */
   Evas_Coord w; /**< width of rectangle */
   Evas_Coord h; /**< height of rectangle */
};

struct _Evas_Point
{
   int x, y;
};

struct _Evas_Coord_Point
{
   Evas_Coord x, y;
};

struct _Evas_Coord_Precision_Point
{
   Evas_Coord x, y;
   double xsub, ysub;
};

struct _Evas_Position
{
    Evas_Point output;
    Evas_Coord_Point canvas;
};

struct _Evas_Precision_Position
{
    Evas_Point output;
    Evas_Coord_Precision_Point canvas;
};

typedef enum _Evas_Aspect_Control
{
   EVAS_ASPECT_CONTROL_NONE = 0,
   EVAS_ASPECT_CONTROL_NEITHER = 1,
   EVAS_ASPECT_CONTROL_HORIZONTAL = 2,
   EVAS_ASPECT_CONTROL_VERTICAL = 3,
   EVAS_ASPECT_CONTROL_BOTH = 4
} Evas_Aspect_Control;

typedef struct _Evas_Pixel_Import_Source Evas_Pixel_Import_Source; /**< A source description of pixels for importing pixels */
typedef struct _Evas_Engine_Info      Evas_Engine_Info; /**< A generic Evas Engine information structure */
typedef struct _Evas_Device           Evas_Device; /**< A source device handle - where the event came from */
typedef struct _Evas_Event_Mouse_Down Evas_Event_Mouse_Down; /**< Event structure for #EVAS_CALLBACK_MOUSE_DOWN event callbacks */
typedef struct _Evas_Event_Mouse_Up   Evas_Event_Mouse_Up; /**< Event structure for #EVAS_CALLBACK_MOUSE_UP event callbacks */
typedef struct _Evas_Event_Mouse_In   Evas_Event_Mouse_In; /**< Event structure for #EVAS_CALLBACK_MOUSE_IN event callbacks */
typedef struct _Evas_Event_Mouse_Out  Evas_Event_Mouse_Out; /**< Event structure for #EVAS_CALLBACK_MOUSE_OUT event callbacks */
typedef struct _Evas_Event_Mouse_Move Evas_Event_Mouse_Move; /**< Event structure for #EVAS_CALLBACK_MOUSE_MOVE event callbacks */
typedef struct _Evas_Event_Mouse_Wheel Evas_Event_Mouse_Wheel; /**< Event structure for #EVAS_CALLBACK_MOUSE_WHEEL event callbacks */
typedef struct _Evas_Event_Multi_Down Evas_Event_Multi_Down; /**< Event structure for #EVAS_CALLBACK_MULTI_DOWN event callbacks */
typedef struct _Evas_Event_Multi_Up   Evas_Event_Multi_Up; /**< Event structure for #EVAS_CALLBACK_MULTI_UP event callbacks */
typedef struct _Evas_Event_Multi_Move Evas_Event_Multi_Move; /**< Event structure for #EVAS_CALLBACK_MULTI_MOVE event callbacks */
typedef struct _Evas_Event_Key_Down   Evas_Event_Key_Down; /**< Event structure for #EVAS_CALLBACK_KEY_DOWN event callbacks */
typedef struct _Evas_Event_Key_Up     Evas_Event_Key_Up; /**< Event structure for #EVAS_CALLBACK_KEY_UP event callbacks */
typedef struct _Evas_Event_Hold       Evas_Event_Hold; /**< Event structure for #EVAS_CALLBACK_HOLD event callbacks */

typedef enum _Evas_Load_Error
{
   EVAS_LOAD_ERROR_NONE = 0, /**< No error on load */
   EVAS_LOAD_ERROR_GENERIC = 1, /**< A non-specific error occurred */
   EVAS_LOAD_ERROR_DOES_NOT_EXIST = 2, /**< File (or file path) does not exist */
   EVAS_LOAD_ERROR_PERMISSION_DENIED = 3, /**< Permission deinied to an existing file (or path) */
   EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED = 4, /**< Allocation of resources failure prevented load */
   EVAS_LOAD_ERROR_CORRUPT_FILE = 5, /**< File corrupt (but was detected as a known format) */
   EVAS_LOAD_ERROR_UNKNOWN_FORMAT = 6 /**< File is not a known format */
} Evas_Load_Error; /**< Load error you can get from loading of files - see evas_load_error_str() too */

typedef enum _Evas_Alloc_Error
{
   EVAS_ALLOC_ERROR_NONE = 0, /**< No allocation error */
   EVAS_ALLOC_ERROR_FATAL = 1, /**< Allocation failed despite attempts to free up memory */
   EVAS_ALLOC_ERROR_RECOVERED = 2 /**< Allocation succeeded, but extra memory had to be found by freeing up speculative resources */
} Evas_Alloc_Error; /**< Possible allocation errors returned by evas_alloc_error() */
   
typedef enum _Evas_Fill_Spread
{
   EVAS_TEXTURE_REFLECT = 0, /**< image fill tiling mode - tiling reflects */
   EVAS_TEXTURE_REPEAT = 1, /**< tiling repeats */
   EVAS_TEXTURE_RESTRICT = 2, /**< tiling clamps - range offset ignored */
   EVAS_TEXTURE_RESTRICT_REFLECT = 3, /**< tiling clamps and any range offset reflects */
   EVAS_TEXTURE_RESTRICT_REPEAT = 4, /**< tiling clamps and any range offset repeats */
   EVAS_TEXTURE_PAD = 5 /**< tiling extends with end values */
} Evas_Fill_Spread; /**< Fill types used for evas_object_image_fill_spread_set() */

typedef enum _Evas_Pixel_Import_Pixel_Format
{
   EVAS_PIXEL_FORMAT_NONE = 0, /**< No pixel format */
   EVAS_PIXEL_FORMAT_ARGB32 = 1, /**< ARGB 32bit pixel format with A in the high byte per 32bit pixel word */
   EVAS_PIXEL_FORMAT_YUV420P_601 = 2 /**< YUV 420 Planar format with CCIR 601 color encoding wuth contiguous planes in the order Y, U and V */
} Evas_Pixel_Import_Pixel_Format; /**< Pixel format for import call. See evas_object_image_pixels_import() */

struct _Evas_Pixel_Import_Source
{
   Evas_Pixel_Import_Pixel_Format format; /**< pixel format type ie ARGB32, YUV420P_601 etc. */
   int w, h; /**< width and height of source in pixels */
   void **rows; /**< an array of pointers (size depends on format) pointing to left edge of each scanline */
};

/* magic version number to know what the native surf struct looks like */
#define EVAS_NATIVE_SURFACE_VERSION 2
  
typedef enum _Evas_Native_Surface_Type
{
   EVAS_NATIVE_SURFACE_NONE,
   EVAS_NATIVE_SURFACE_X11,
   EVAS_NATIVE_SURFACE_OPENGL
} Evas_Native_Surface_Type;
  
struct _Evas_Native_Surface
{
   int                         version;
   Evas_Native_Surface_Type    type;
   union {
     struct {
       void          *visual; /**< visual of the pixmap to use (Visual) */
       unsigned long  pixmap; /**< pixmap id to use (Pixmap) */
     } x11;
     struct {
       unsigned int   texture_id; /**< opengl texture id to use from glGenTextures() */
       unsigned int   framebuffer_id; /**< 0 if not a FBO, FBO id otherwise from glGenFramebuffers() */
       unsigned int   internal_format; /**< same as 'internalFormat' for glTexImage2D() */
       unsigned int   format; /**< same as 'format' for glTexImage2D() */
       unsigned int   x, y, w, h; /**< region inside the texture to use (image size is assumed as texture size, with 0, 0 being the top-left and co-ordinates working down to the right and bottom being positive) */
     } opengl;
   } data;
};

#define EVAS_LAYER_MIN -32768 /**< bottom-most layer number */
#define EVAS_LAYER_MAX 32767  /**< top-most layer number */

#define EVAS_COLOR_SPACE_ARGB 0 /**< Not used for anything */
#define EVAS_COLOR_SPACE_AHSV 1 /**< Not used for anything */
#define EVAS_TEXT_INVALID -1 /**< Not used for anything */
#define EVAS_TEXT_SPECIAL -2 /**< Not used for anything */

#define EVAS_HINT_EXPAND  1.0 /**< Use with evas_object_size_hint_weight_set(), evas_object_size_hint_weight_get(), evas_object_size_hint_expand_set(), evas_object_size_hint_expand_get() */
#define EVAS_HINT_FILL   -1.0 /**< Use with evas_object_size_hint_align_set(), evas_object_size_hint_align_get(), evas_object_size_hint_fill_set(), evas_object_size_hint_fill_get() */
#define evas_object_size_hint_fill_set evas_object_size_hint_align_set /**< Convenience macro to make it easier to understand that align is also used for fill properties (as fill is mutually exclusive to align) */
#define evas_object_size_hint_fill_get evas_object_size_hint_align_get /**< Convenience macro to make it easier to understand that align is also used for fill properties (as fill is mutually exclusive to align) */
#define evas_object_size_hint_expand_set evas_object_size_hint_weight_set /**< Convenience macro to make it easier to understand that weight is also used for expand properties */
#define evas_object_size_hint_expand_get evas_object_size_hint_weight_get /**< Convenience macro to make it easier to understand that weight is also used for expand properties */

/**
 * How the object should be rendered to output.
 * @ingroup Evas_Object_Group_Extras
 */
typedef enum _Evas_Render_Op
{
   EVAS_RENDER_BLEND = 0, /**< default op: d = d*(1-sa) + s */
   EVAS_RENDER_BLEND_REL = 1, /**< d = d*(1 - sa) + s*da */
   EVAS_RENDER_COPY = 2, /**< d = s */
   EVAS_RENDER_COPY_REL = 3, /**< d = s*da */
   EVAS_RENDER_ADD = 4, /**< d = d + s */
   EVAS_RENDER_ADD_REL = 5, /**< d = d + s*da */
   EVAS_RENDER_SUB = 6, /**< d = d - s */
   EVAS_RENDER_SUB_REL = 7, /**< d = d - s*da */
   EVAS_RENDER_TINT = 8, /**< d = d*s + d*(1 - sa) + s*(1 - da) */
   EVAS_RENDER_TINT_REL = 9, /**< d = d*(1 - sa + s) */
   EVAS_RENDER_MASK = 10, /**< d = d*sa */
   EVAS_RENDER_MUL = 11 /**< d = d*s */
} Evas_Render_Op; /**< How the object should be rendered to output. */

typedef enum _Evas_Border_Fill_Mode
{
   EVAS_BORDER_FILL_NONE = 0,
   EVAS_BORDER_FILL_DEFAULT = 1,
   EVAS_BORDER_FILL_SOLID = 2
} Evas_Border_Fill_Mode;

typedef enum _Evas_Image_Scale_Hint
{
   EVAS_IMAGE_SCALE_HINT_NONE = 0,
   EVAS_IMAGE_SCALE_HINT_DYNAMIC = 1,
   EVAS_IMAGE_SCALE_HINT_STATIC = 2
} Evas_Image_Scale_Hint;

typedef enum _Evas_Engine_Render_Mode
{
   EVAS_RENDER_MODE_BLOCKING = 0,
   EVAS_RENDER_MODE_NONBLOCKING = 1,
} Evas_Engine_Render_Mode;

typedef enum _Evas_Image_Content_Hint
{
   EVAS_IMAGE_CONTENT_HINT_NONE = 0,
   EVAS_IMAGE_CONTENT_HINT_DYNAMIC = 1,
   EVAS_IMAGE_CONTENT_HINT_STATIC = 2
} Evas_Image_Content_Hint;

struct _Evas_Engine_Info /** Generic engine information. Generic info is useless */
{
   int magic; /**< Magic number */
};

struct _Evas_Event_Mouse_Down /** Mouse button press event */
{
   int button; /**< Mouse button number that went down (1 - 32) */

   Evas_Point output;
   Evas_Coord_Point canvas;

   void          *data;
   Evas_Modifier *modifiers;
   Evas_Lock     *locks;

   Evas_Button_Flags flags;
   unsigned int      timestamp;
   Evas_Event_Flags  event_flags;
   Evas_Device      *dev;
};

struct _Evas_Event_Mouse_Up /** Mouse button release event */
{
   int button; /**< Mouse button number that was raised (1 - 32) */

   Evas_Point output;
   Evas_Coord_Point canvas;

   void          *data;
   Evas_Modifier *modifiers;
   Evas_Lock     *locks;

   Evas_Button_Flags flags;
   unsigned int      timestamp;
   Evas_Event_Flags  event_flags;
   Evas_Device      *dev;
};

struct _Evas_Event_Mouse_In /** Mouse enter event */
{
   int buttons; /**< Button pressed mask, Bits set to 1 are buttons currently pressed (bit 0 = mouse button 1, bit 1 = mouse button 2 etc.) */

   Evas_Point output;
   Evas_Coord_Point canvas;

   void          *data;
   Evas_Modifier *modifiers;
   Evas_Lock     *locks;
   unsigned int   timestamp;
   Evas_Event_Flags  event_flags;
   Evas_Device      *dev;
};

struct _Evas_Event_Mouse_Out /** Mouse leave event */
{
   int buttons; /**< Button pressed mask, Bits set to 1 are buttons currently pressed (bit 0 = mouse button 1, bit 1 = mouse button 2 etc.) */


   Evas_Point output;
   Evas_Coord_Point canvas;

   void          *data;
   Evas_Modifier *modifiers;
   Evas_Lock     *locks;
   unsigned int   timestamp;
   Evas_Event_Flags  event_flags;
   Evas_Device      *dev;
};

struct _Evas_Event_Mouse_Move /** Mouse button down event */
{
   int buttons; /**< Button pressed mask, Bits set to 1 are buttons currently pressed (bit 0 = mouse button 1, bit 1 = mouse button 2 etc.) */

   Evas_Position cur, prev;

   void          *data;
   Evas_Modifier *modifiers;
   Evas_Lock     *locks;
   unsigned int   timestamp;
   Evas_Event_Flags  event_flags;
   Evas_Device      *dev;
};

struct _Evas_Event_Mouse_Wheel /** Wheel event */
{
   int direction; /* 0 = default up/down wheel FIXME: more wheel types */
   int z; /* ...,-2,-1 = down, 1,2,... = up */

   Evas_Point output;
   Evas_Coord_Point canvas;

   void          *data;
   Evas_Modifier *modifiers;
   Evas_Lock     *locks;
   unsigned int   timestamp;
   Evas_Event_Flags  event_flags;
   Evas_Device      *dev;
};

struct _Evas_Event_Multi_Down /** Multi button press event */
{
   int device; /**< Multi device number that went down (1 or more for extra touches) */
   double radius, radius_x, radius_y;
   double pressure, angle;

   Evas_Point output;
   Evas_Coord_Precision_Point canvas;

   void          *data;
   Evas_Modifier *modifiers;
   Evas_Lock     *locks;

   Evas_Button_Flags flags;
   unsigned int      timestamp;
   Evas_Event_Flags  event_flags;
   Evas_Device      *dev;
};

struct _Evas_Event_Multi_Up /** Multi button release event */
{
   int device; /**< Multi device number that went up (1 or more for extra touches) */
   double radius, radius_x, radius_y;
   double pressure, angle;

   Evas_Point output;
   Evas_Coord_Precision_Point canvas;

   void          *data;
   Evas_Modifier *modifiers;
   Evas_Lock     *locks;

   Evas_Button_Flags flags;
   unsigned int      timestamp;
   Evas_Event_Flags  event_flags;
   Evas_Device      *dev;
};

struct _Evas_Event_Multi_Move /** Multi button down event */
{
   int device; /**< Multi device number that moved (1 or more for extra touches) */
   double radius, radius_x, radius_y;
   double pressure, angle;

   Evas_Precision_Position cur;

   void          *data;
   Evas_Modifier *modifiers;
   Evas_Lock     *locks;
   unsigned int   timestamp;
   Evas_Event_Flags  event_flags;
   Evas_Device      *dev;
};

struct _Evas_Event_Key_Down /** Key press event */
{
   char          *keyname; /**< The string name of the key pressed */
   void          *data;
   Evas_Modifier *modifiers;
   Evas_Lock     *locks;

   const char    *key; /**< The logical key : (eg shift+1 == exclamation) */
   const char    *string; /**< A UTF8 string if this keystroke has produced a visible string to be ADDED */
   const char    *compose; /**< A UTF8 string if this keystroke has modified a string in the middle of being composed - this string replaces the previous one */
   unsigned int   timestamp;
   Evas_Event_Flags  event_flags;
   Evas_Device      *dev;
};

struct _Evas_Event_Key_Up /** Key release event */
{
   char          *keyname; /**< The string name of the key released */
   void          *data;
   Evas_Modifier *modifiers;
   Evas_Lock     *locks;

   const char    *key; /**< The logical key : (eg shift+1 == exclamation) */
   const char    *string; /**< A UTF8 string if this keystroke has produced a visible string to be ADDED */
   const char    *compose; /**< A UTF8 string if this keystroke has modified a string in the middle of being composed - this string replaces the previous one */
   unsigned int   timestamp;
   Evas_Event_Flags  event_flags;
   Evas_Device      *dev;
};

struct _Evas_Event_Hold /** Hold change event */
{
   int            hold; /**< The hold flag */
   void          *data;

   unsigned int   timestamp;
   Evas_Event_Flags  event_flags;
   Evas_Device      *dev;
};

/**
 * How mouse pointer should be handled by Evas.
 *
 * If #EVAS_OBJECT_POINTER_MODE_AUTOGRAB, then when mouse is down an
 * object, then moves outside of it, the pointer still behaves as
 * being bound to the object, albeit out of its drawing region. On
 * mouse up, the event will be feed to the object, that may check if
 * the final position is over or not and do something about it.
 *
 * @ingroup Evas_Object_Group_Extras
 */
typedef enum _Evas_Object_Pointer_Mode
{
   EVAS_OBJECT_POINTER_MODE_AUTOGRAB, /**< default, X11-like */
   EVAS_OBJECT_POINTER_MODE_NOGRAB
} Evas_Object_Pointer_Mode; /**< How mouse pointer should be handled by Evas. */

typedef void      (*Evas_Smart_Cb) (void *data, Evas_Object *obj, void *event_info);
typedef void      (*Evas_Event_Cb) (void *data, Evas *e, void *event_info);
typedef Eina_Bool (*Evas_Object_Event_Post_Cb) (void *data, Evas *e);
typedef void      (*Evas_Object_Event_Cb) (void *data, Evas *e, Evas_Object *obj, void *event_info);
typedef void      (*Evas_Async_Events_Put_Cb)(void *target, Evas_Callback_Type type, void *event_info);

/**
 * @defgroup Evas_Group Top Level Functions
 *
 * Functions that affect Evas as a whole.
 */
   EAPI int               evas_init                         (void);
   EAPI int               evas_shutdown                     (void);

   EAPI Evas_Alloc_Error  evas_alloc_error                  (void);

   EAPI int               evas_async_events_fd_get          (void) EINA_WARN_UNUSED_RESULT EINA_PURE;
   EAPI int               evas_async_events_process         (void);
   EAPI Eina_Bool         evas_async_events_put             (const void *target, Evas_Callback_Type type, void *event_info, Evas_Async_Events_Put_Cb func) EINA_ARG_NONNULL(1, 4);

/**
 * @defgroup Evas_Canvas Canvas Functions
 *
 * Functions that deal with the basic evas object.  They are the
 * functions you need to use at a minimum to get a working evas, and
 * to destroy it.
 *
 */

   EAPI Evas             *evas_new                          (void) EINA_WARN_UNUSED_RESULT EINA_MALLOC;
   EAPI void              evas_free                         (Evas *e)  EINA_ARG_NONNULL(1);

   EAPI void              evas_focus_in                     (Evas *e);
   EAPI void              evas_focus_out                    (Evas *e);
   EAPI Eina_Bool         evas_focus_state_get              (const Evas *e) EINA_PURE;
   EAPI void              evas_nochange_push                (Evas *e);
   EAPI void              evas_nochange_pop                 (Evas *e);
         
   EAPI void              evas_data_attach_set              (Evas *e, void *data) EINA_ARG_NONNULL(1);
   EAPI void             *evas_data_attach_get              (const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

   EAPI void              evas_damage_rectangle_add         (Evas *e, int x, int y, int w, int h) EINA_ARG_NONNULL(1);
   EAPI void              evas_obscured_rectangle_add       (Evas *e, int x, int y, int w, int h) EINA_ARG_NONNULL(1);
   EAPI void              evas_obscured_clear               (Evas *e) EINA_ARG_NONNULL(1);
   EAPI Eina_List        *evas_render_updates               (Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);
   EAPI void              evas_render_updates_free          (Eina_List *updates);
   EAPI void              evas_render                       (Evas *e) EINA_ARG_NONNULL(1);
   EAPI void              evas_norender                     (Evas *e) EINA_ARG_NONNULL(1);
   EAPI void              evas_render_idle_flush            (Evas *e) EINA_ARG_NONNULL(1);
   EAPI void              evas_render_dump                  (Evas *e) EINA_ARG_NONNULL(1);

/**
 * @defgroup Evas_Output_Method Render Engine Functions
 *
 * Functions that are used to set the render engine for a given
 * function, and then get that engine working.
 *
 * The following code snippet shows how they can be used to
 * initialise an evas that uses the X11 software engine:
 * @code
 * Evas *evas;
 * Evas_Engine_Info_Software_X11 *einfo;
 * extern Display *display;
 * extern Window win;
 *
 * evas_init();
 *
 * evas = evas_new();
 * evas_output_method_set(evas, evas_render_method_lookup("software_x11"));
 * evas_output_size_set(evas, 640, 480);
 * evas_output_viewport_set(evas, 0, 0, 640, 480);
 * einfo = (Evas_Engine_Info_Software_X11 *)evas_engine_info_get(evas);
 * einfo->info.display = display;
 * einfo->info.visual = DefaultVisual(display, DefaultScreen(display));
 * einfo->info.colormap = DefaultColormap(display, DefaultScreen(display));
 * einfo->info.drawable = win;
 * einfo->info.depth = DefaultDepth(display, DefaultScreen(display));
 * evas_engine_info_set(evas, (Evas_Engine_Info *)einfo);
 * @endcode
 *
 * @ingroup Evas_Canvas
 */
   EAPI int               evas_render_method_lookup         (const char *name) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);
   EAPI Eina_List        *evas_render_method_list           (void) EINA_WARN_UNUSED_RESULT;
   EAPI void              evas_render_method_list_free      (Eina_List *list);

   EAPI void              evas_output_method_set            (Evas *e, int render_method) EINA_ARG_NONNULL(1);
   EAPI int               evas_output_method_get            (const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

   EAPI Evas_Engine_Info *evas_engine_info_get              (const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI Eina_Bool         evas_engine_info_set              (Evas *e, Evas_Engine_Info *info) EINA_ARG_NONNULL(1);

/**
 * @defgroup Evas_Output_Size Output and Viewport Resizing Functions
 *
 * Functions that set and retrieve the output and viewport size of an
 * evas.
 *
 * @ingroup Evas_Canvas
 */
   EAPI void              evas_output_size_set              (Evas *e, int w, int h) EINA_ARG_NONNULL(1);
   EAPI void              evas_output_size_get              (const Evas *e, int *w, int *h) EINA_ARG_NONNULL(1);
   EAPI void              evas_output_viewport_set          (Evas *e, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h) EINA_ARG_NONNULL(1);
   EAPI void              evas_output_viewport_get          (const Evas *e, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h) EINA_ARG_NONNULL(1);

/**
 * @defgroup Evas_Coord_Mapping_Group Coordinate Mapping Functions
 *
 * Functions that are used to map coordinates from the canvas to the
 * screen or the screen to the canvas.
 *
 * @ingroup Evas_Canvas
 */
   EAPI Evas_Coord        evas_coord_screen_x_to_world      (const Evas *e, int x) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);
   EAPI Evas_Coord        evas_coord_screen_y_to_world      (const Evas *e, int y) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);
   EAPI int               evas_coord_world_x_to_screen      (const Evas *e, Evas_Coord x) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);
   EAPI int               evas_coord_world_y_to_screen      (const Evas *e, Evas_Coord y) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @defgroup Evas_Pointer_Group Pointer (Mouse) Functions
 *
 * Functions that deal with the status of the pointer (mouse cursor).
 *
 * @ingroup Evas_Canvas
 */
   EAPI void              evas_pointer_output_xy_get        (const Evas *e, int *x, int *y) EINA_ARG_NONNULL(1);
   EAPI void              evas_pointer_canvas_xy_get        (const Evas *e, Evas_Coord *x, Evas_Coord *y) EINA_ARG_NONNULL(1);
   EAPI int               evas_pointer_button_down_mask_get (const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);
   EAPI Eina_Bool         evas_pointer_inside_get           (const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);
   EAPI void              evas_sync(Evas *e) EINA_ARG_NONNULL(1);

/**
 * @defgroup Evas_Event_Freezing_Group Event Freezing Functions
 *
 * Functions that deal with the freezing of event processing of an
 * evas.
 *
 * @ingroup Evas_Canvas
 */
   EAPI void              evas_event_freeze                 (Evas *e) EINA_ARG_NONNULL(1);
   EAPI void              evas_event_thaw                   (Evas *e) EINA_ARG_NONNULL(1);
   EAPI int               evas_event_freeze_get             (const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

/**
 * @defgroup Evas_Event_Feeding_Group Event Feeding Functions
 *
 * Functions to tell Evas that events happened and should be
 * processed.
 *
 * As explained in @ref intro_not_evas, Evas does not know how to poll
 * for events, so the developer should do it and then feed such events
 * to the canvas to be processed. This is only required if operating
 * Evas directly as modules such as Ecore_Evas does that for you.
 *
 * @ingroup Evas_Canvas
 */
   EAPI void              evas_event_feed_mouse_down        (Evas *e, int b, Evas_Button_Flags flags, unsigned int timestamp, const void *data) EINA_ARG_NONNULL(1);
   EAPI void              evas_event_feed_mouse_up          (Evas *e, int b, Evas_Button_Flags flags, unsigned int timestamp, const void *data) EINA_ARG_NONNULL(1);
   EAPI void              evas_event_feed_mouse_move        (Evas *e, int x, int y, unsigned int timestamp, const void *data) EINA_ARG_NONNULL(1);
   EAPI void              evas_event_feed_mouse_in          (Evas *e, unsigned int timestamp, const void *data) EINA_ARG_NONNULL(1);
   EAPI void              evas_event_feed_mouse_out         (Evas *e, unsigned int timestamp, const void *data) EINA_ARG_NONNULL(1);
   EAPI void              evas_event_feed_multi_down        (Evas *e, int d, int x, int y, double rad, double radx, double rady, double pres, double ang, double fx, double fy, Evas_Button_Flags flags, unsigned int timestamp, const void *data);
   EAPI void              evas_event_feed_multi_up          (Evas *e, int d, int x, int y, double rad, double radx, double rady, double pres, double ang, double fx, double fy, Evas_Button_Flags flags, unsigned int timestamp, const void *data);
   EAPI void              evas_event_feed_multi_move        (Evas *e, int d, int x, int y, double rad, double radx, double rady, double pres, double ang, double fx, double fy, unsigned int timestamp, const void *data);
   EAPI void              evas_event_feed_mouse_cancel      (Evas *e, unsigned int timestamp, const void *data) EINA_ARG_NONNULL(1);
   EAPI void              evas_event_feed_mouse_wheel       (Evas *e, int direction, int z, unsigned int timestamp, const void *data) EINA_ARG_NONNULL(1);
   EAPI void              evas_event_feed_key_down          (Evas *e, const char *keyname, const char *key, const char *string, const char *compose, unsigned int timestamp, const void *data) EINA_ARG_NONNULL(1);
   EAPI void              evas_event_feed_key_up            (Evas *e, const char *keyname, const char *key, const char *string, const char *compose, unsigned int timestamp, const void *data) EINA_ARG_NONNULL(1);
   EAPI void              evas_event_feed_hold              (Evas *e, int hold, unsigned int timestamp, const void *data) EINA_ARG_NONNULL(1);

/**
 * @defgroup Evas_Canvas_Events Canvas Events
 *
 * Canvas generates some events
 *
 * @ingroup Evas_Canvas
 */

   EAPI void              evas_event_callback_add              (Evas *e, Evas_Callback_Type type, Evas_Event_Cb func, const void *data) EINA_ARG_NONNULL(1, 3);
   EAPI void             *evas_event_callback_del              (Evas *e, Evas_Callback_Type type, Evas_Event_Cb func) EINA_ARG_NONNULL(1, 3);
   EAPI void             *evas_event_callback_del_full         (Evas *e, Evas_Callback_Type type, Evas_Event_Cb func, const void *data) EINA_ARG_NONNULL(1, 3);
   EAPI void              evas_post_event_callback_push        (Evas *e, Evas_Object_Event_Post_Cb func, const void *data);
   EAPI void              evas_post_event_callback_remove      (Evas *e, Evas_Object_Event_Post_Cb func);
   EAPI void              evas_post_event_callback_remove_full (Evas *e, Evas_Object_Event_Post_Cb func, const void *data);
       
/**
 * @defgroup Evas_Image_Group Image Functions
 *
 * Functions that deals with images at canvas level.
 *
 * @ingroup Evas_Canvas
 */
   EAPI void              evas_image_cache_flush            (Evas *e) EINA_ARG_NONNULL(1);
   EAPI void              evas_image_cache_reload           (Evas *e) EINA_ARG_NONNULL(1);
   EAPI void              evas_image_cache_set              (Evas *e, int size) EINA_ARG_NONNULL(1);
   EAPI int               evas_image_cache_get              (const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

/**
 * @defgroup Evas_Font_Group Font Functions
 *
 * Functions that deals with fonts.
 *
 * @ingroup Evas_Canvas
 */
   EAPI void                     evas_font_hinting_set        (Evas *e, Evas_Font_Hinting_Flags hinting) EINA_ARG_NONNULL(1);
   EAPI Evas_Font_Hinting_Flags  evas_font_hinting_get        (const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI Eina_Bool                evas_font_hinting_can_hint   (const Evas *e, Evas_Font_Hinting_Flags hinting) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

   EAPI void                     evas_font_cache_flush        (Evas *e) EINA_ARG_NONNULL(1);
   EAPI void                     evas_font_cache_set          (Evas *e, int size) EINA_ARG_NONNULL(1);
   EAPI int                      evas_font_cache_get          (const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

   EAPI Eina_List               *evas_font_available_list     (const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI void                     evas_font_available_list_free(Evas *e, Eina_List *available) EINA_ARG_NONNULL(1);

/**
 * @defgroup Evas_Font_Path_Group Font Path Functions
 *
 * Functions that edit the paths being used to load fonts.
 *
 * @ingroup Evas_Font_Group
 */
   EAPI void              evas_font_path_clear              (Evas *e) EINA_ARG_NONNULL(1);
   EAPI void              evas_font_path_append             (Evas *e, const char *path) EINA_ARG_NONNULL(1, 2);
   EAPI void              evas_font_path_prepend            (Evas *e, const char *path) EINA_ARG_NONNULL(1, 2);
   EAPI const Eina_List  *evas_font_path_list               (const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

/**
 * @defgroup Evas_Object_Group Generic Object Functions
 *
 * Functions that manipulate generic evas objects.
 */

/**
 * @defgroup Evas_Object_Group_Basic Basic Object Manipulation
 *
 * Methods that are often used, like those that change the color,
 * clippers and geometry of the object.
 *
 * @ingroup Evas_Object_Group
 */
   EAPI void              evas_object_clip_set              (Evas_Object *obj, Evas_Object *clip) EINA_ARG_NONNULL(1, 2);
   EAPI Evas_Object      *evas_object_clip_get              (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI void              evas_object_clip_unset            (Evas_Object *obj);
   EAPI const Eina_List  *evas_object_clipees_get           (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

   EAPI void              evas_object_focus_set             (Evas_Object *obj, Eina_Bool focus) EINA_ARG_NONNULL(1);
   EAPI Eina_Bool         evas_object_focus_get             (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

   EAPI void              evas_object_layer_set             (Evas_Object *obj, short l) EINA_ARG_NONNULL(1);
   EAPI short             evas_object_layer_get             (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

   EAPI void              evas_object_name_set              (Evas_Object *obj, const char *name) EINA_ARG_NONNULL(1);
   EAPI const char       *evas_object_name_get              (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

   EAPI void              evas_object_del                   (Evas_Object *obj) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_move                  (Evas_Object *obj, Evas_Coord x, Evas_Coord y) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_resize                (Evas_Object *obj, Evas_Coord w, Evas_Coord h) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_geometry_get          (const Evas_Object *obj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h) EINA_ARG_NONNULL(1);

   EAPI void              evas_object_show                  (Evas_Object *obj) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_hide                  (Evas_Object *obj) EINA_ARG_NONNULL(1);
   EAPI Eina_Bool         evas_object_visible_get           (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

   EAPI void              evas_object_color_set             (Evas_Object *obj, int r, int g, int b, int a) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_color_get             (const Evas_Object *obj, int *r, int *g, int *b, int *a) EINA_ARG_NONNULL(1);

   EAPI Evas             *evas_object_evas_get              (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

   EAPI const char       *evas_object_type_get              (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

   EAPI void              evas_object_raise                 (Evas_Object *obj) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_lower                 (Evas_Object *obj) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_stack_above           (Evas_Object *obj, Evas_Object *above) EINA_ARG_NONNULL(1, 2);
   EAPI void              evas_object_stack_below           (Evas_Object *obj, Evas_Object *below) EINA_ARG_NONNULL(1, 2);
   EAPI Evas_Object      *evas_object_above_get             (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI Evas_Object      *evas_object_below_get             (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

/**
 * @defgroup Evas_Object_Group_Events Object Events
 *
 * Objects generates events when they are moved, resized, when their
 * visibility change, when they are deleted and so on. These methods
 * will allow one to handle such events.
 *
 * The events can be those from keyboard and mouse, if the object
 * accepts these events.
 *
 * @ingroup Evas_Object_Group
 */
   EAPI void              evas_object_event_callback_add     (Evas_Object *obj, Evas_Callback_Type type, Evas_Object_Event_Cb func, const void *data) EINA_ARG_NONNULL(1, 3);
   EAPI void             *evas_object_event_callback_del     (Evas_Object *obj, Evas_Callback_Type type, Evas_Object_Event_Cb func) EINA_ARG_NONNULL(1, 3);
   EAPI void             *evas_object_event_callback_del_full(Evas_Object *obj, Evas_Callback_Type type, Evas_Object_Event_Cb func, const void *data) EINA_ARG_NONNULL(1, 3);

   EAPI void              evas_object_pass_events_set        (Evas_Object *obj, Eina_Bool pass) EINA_ARG_NONNULL(1);
   EAPI Eina_Bool         evas_object_pass_events_get        (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI void              evas_object_repeat_events_set      (Evas_Object *obj, Eina_Bool repeat) EINA_ARG_NONNULL(1);
   EAPI Eina_Bool         evas_object_repeat_events_get      (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI void              evas_object_propagate_events_set   (Evas_Object *obj, Eina_Bool prop) EINA_ARG_NONNULL(1);
   EAPI Eina_Bool         evas_object_propagate_events_get   (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

/**
 * @defgroup Evas_Object_Group_Map UV Mapping (Rotation, Perspecitve, 3D...)
 *
 * Evas allows different transformations to be applied to all kinds of
 * objects. These are applied by means of UV mapping.
 *
 * With UV mapping, one maps points in the source object to a 3D space
 * positioning at target. This allows rotation, perspective, scale and
 * lots of other effects, depending on the map that is used.
 *
 * Each map point may carry a multiplier color. If properly
 * calculated, these can do shading effects on the object, producing
 * 3D effects.
 *
 * As usual, Evas provides both the raw and easy to use methods. The
 * raw methods allow developer to create its maps somewhere else,
 * maybe load them from some file format. The easy to use methods,
 * calculate the points given some high-level parameters, such as
 * rotation angle, ambient light and so on.
 *
 * @note applying mapping will reduce performance, so use with
 *       care. The impact on performance depends on engine in
 *       use. Software is quite optimized, but not as fast as OpenGL.
 *
 * @ingroup Evas_Object_Group
 */
   EAPI void              evas_object_map_enable_set        (Evas_Object *obj, Eina_Bool enabled);
   EAPI Eina_Bool         evas_object_map_enable_get        (const Evas_Object *obj);
   EAPI void              evas_object_map_source_set        (Evas_Object *obj, Evas_Object *src);
   EAPI Evas_Object      *evas_object_map_source_get        (const Evas_Object *obj);
   EAPI void              evas_object_map_set               (Evas_Object *obj, const Evas_Map *map);
   EAPI const Evas_Map   *evas_object_map_get               (const Evas_Object *obj);

   EAPI void              evas_map_util_points_populate_from_object_full(Evas_Map *m, const Evas_Object *obj, Evas_Coord z);
   EAPI void              evas_map_util_points_populate_from_object     (Evas_Map *m, const Evas_Object *obj);
   EAPI void              evas_map_util_points_populate_from_geometry   (Evas_Map *m, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h, Evas_Coord z);
   EAPI void              evas_map_util_points_color_set                (Evas_Map *m, int r, int g, int b, int a);
   EAPI void              evas_map_util_rotate                          (Evas_Map *m, double degrees, Evas_Coord cx, Evas_Coord cy);
   EAPI void              evas_map_util_zoom                            (Evas_Map *m, double zoomx, double zoomy, Evas_Coord cx, Evas_Coord cy);
   EAPI void              evas_map_util_3d_rotate                       (Evas_Map *m, double dx, double dy, double dz, Evas_Coord cx, Evas_Coord cy, Evas_Coord cz);
   EAPI void              evas_map_util_3d_lighting                     (Evas_Map *m, Evas_Coord lx, Evas_Coord ly, Evas_Coord lz, int lr, int lg, int lb, int ar, int ag, int ab);
   EAPI void              evas_map_util_3d_perspective                  (Evas_Map *m, Evas_Coord px, Evas_Coord py, Evas_Coord z0, Evas_Coord foc);
   EAPI Eina_Bool         evas_map_util_clockwise_get                   (Evas_Map *m);

   EAPI Evas_Map         *evas_map_new                      (int count);
   EAPI void              evas_map_smooth_set               (Evas_Map *m, Eina_Bool enabled);
   EAPI Eina_Bool         evas_map_smooth_get               (const Evas_Map *m);
   EAPI void              evas_map_alpha_set                (Evas_Map *m, Eina_Bool enabled);
   EAPI Eina_Bool         evas_map_alpha_get                (const Evas_Map *m);
   EAPI Evas_Map         *evas_map_dup                      (const Evas_Map *m);
   EAPI void              evas_map_free                     (Evas_Map *m);
   EAPI int               evas_map_count_get               (const Evas_Map *m) EINA_CONST;
   EAPI void              evas_map_point_coord_set          (Evas_Map *m, int idx, Evas_Coord x, Evas_Coord y, Evas_Coord z);
   EAPI void              evas_map_point_coord_get          (const Evas_Map *m, int idx, Evas_Coord *x, Evas_Coord *y, Evas_Coord *z);
   EAPI void              evas_map_point_image_uv_set       (Evas_Map *m, int idx, double u, double v);
   EAPI void              evas_map_point_image_uv_get       (const Evas_Map *m, int idx, double *u, double *v);
   EAPI void              evas_map_point_color_set          (Evas_Map *m, int idx, int r, int g, int b, int a);
   EAPI void              evas_map_point_color_get          (const Evas_Map *m, int idx, int *r, int *g, int *b, int *a);

/**
 * @defgroup Evas_Object_Group_Size_Hints Size Hints
 *
 * Objects may carry hints so another object that acts as a manager
 * (see @ref Evas_Smart_Object_Group) may know how to properly position
 * and resize the object. The Size Hints provide a common interface
 * that is recommended as the protocol for such information.
 *
 * @ingroup Evas_Object_Group
 */
   EAPI void              evas_object_size_hint_min_get     (const Evas_Object *obj, Evas_Coord *w, Evas_Coord *h) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_size_hint_min_set     (Evas_Object *obj, Evas_Coord w, Evas_Coord h) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_size_hint_max_get     (const Evas_Object *obj, Evas_Coord *w, Evas_Coord *h) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_size_hint_max_set     (Evas_Object *obj, Evas_Coord w, Evas_Coord h) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_size_hint_request_get (const Evas_Object *obj, Evas_Coord *w, Evas_Coord *h) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_size_hint_request_set (Evas_Object *obj, Evas_Coord w, Evas_Coord h) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_size_hint_aspect_get  (const Evas_Object *obj, Evas_Aspect_Control *aspect, Evas_Coord *w, Evas_Coord *h) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_size_hint_aspect_set  (Evas_Object *obj, Evas_Aspect_Control aspect, Evas_Coord w, Evas_Coord h) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_size_hint_align_get   (const Evas_Object *obj, double *x, double *y) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_size_hint_align_set   (Evas_Object *obj, double x, double y) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_size_hint_weight_get  (const Evas_Object *obj, double *x, double *y) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_size_hint_weight_set  (Evas_Object *obj, double x, double y) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_size_hint_padding_get (const Evas_Object *obj, Evas_Coord *l, Evas_Coord *r, Evas_Coord *t, Evas_Coord *b) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_size_hint_padding_set (Evas_Object *obj, Evas_Coord l, Evas_Coord r, Evas_Coord t, Evas_Coord b) EINA_ARG_NONNULL(1);

/**
 * @defgroup Evas_Object_Group_Extras Extra Object Manipulation
 *
 * Miscellaneous functions that also apply to any object, but are less
 * used or not implemented by all objects.
 *
 * @ingroup Evas_Object_Group
 */
   EAPI void                      evas_object_data_set             (Evas_Object *obj, const char *key, const void *data) EINA_ARG_NONNULL(1, 2);
   EAPI void                     *evas_object_data_get             (const Evas_Object *obj, const char *key) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2) EINA_PURE;
   EAPI void                     *evas_object_data_del             (Evas_Object *obj, const char *key) EINA_ARG_NONNULL(1, 2);

   EAPI void                      evas_object_pointer_mode_set     (Evas_Object *obj, Evas_Object_Pointer_Mode setting) EINA_ARG_NONNULL(1);
   EAPI Evas_Object_Pointer_Mode  evas_object_pointer_mode_get     (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

   EAPI void                      evas_object_anti_alias_set       (Evas_Object *obj, Eina_Bool antialias) EINA_ARG_NONNULL(1);
   EAPI Eina_Bool                 evas_object_anti_alias_get       (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

   EAPI void                      evas_object_scale_set            (Evas_Object *obj, double scale) EINA_ARG_NONNULL(1);
   EAPI double                    evas_object_scale_get            (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

   EAPI void                      evas_object_render_op_set        (Evas_Object *obj, Evas_Render_Op op) EINA_ARG_NONNULL(1);
   EAPI Evas_Render_Op            evas_object_render_op_get        (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

   EAPI void                      evas_object_precise_is_inside_set(Evas_Object *obj, Eina_Bool precise) EINA_ARG_NONNULL(1);
   EAPI Eina_Bool                 evas_object_precise_is_inside_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

   EAPI void                      evas_object_static_clip_set      (Evas_Object *obj, Eina_Bool is_static_clip) EINA_ARG_NONNULL(1);
   EAPI Eina_Bool                 evas_object_static_clip_get      (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
         
/**
 * @defgroup Evas_Object_Group_Find Finding Objects
 *
 * Functions that allows finding objects by their position, name or
 * other properties.
 *
 * @ingroup Evas_Object_Group
 */
   EAPI Evas_Object      *evas_focus_get                    (const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

   EAPI Evas_Object      *evas_object_name_find             (const Evas *e, const char *name) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

   EAPI Evas_Object      *evas_object_top_at_xy_get         (const Evas *e, Evas_Coord x, Evas_Coord y, Eina_Bool include_pass_events_objects, Eina_Bool include_hidden_objects) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI Evas_Object      *evas_object_top_at_pointer_get    (const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI Evas_Object      *evas_object_top_in_rectangle_get  (const Evas *e, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h, Eina_Bool include_pass_events_objects, Eina_Bool include_hidden_objects) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

   EAPI Eina_List        *evas_objects_at_xy_get            (const Evas *e, Evas_Coord x, Evas_Coord y, Eina_Bool include_pass_events_objects, Eina_Bool include_hidden_objects) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI Eina_List        *evas_objects_in_rectangle_get     (const Evas *e, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h, Eina_Bool include_pass_events_objects, Eina_Bool include_hidden_objects) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

   EAPI Evas_Object      *evas_object_bottom_get            (const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI Evas_Object      *evas_object_top_get               (const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

/**
 * @defgroup Evas_Object_Group_Interceptors Object Method Interceptors
 *
 * Evas provides a way to intercept method calls. The interceptor
 * callback may opt to completely deny the call, or may check and
 * change the parameters before continuing. The continuation of an
 * intercepted call is done by calling the intercepted call again,
 * from inside the interceptor callback.
 *
 * @ingroup Evas_Object_Group
 */
typedef void (*Evas_Object_Intercept_Show_Cb) (void *data, Evas_Object *obj);
typedef void (*Evas_Object_Intercept_Hide_Cb) (void *data, Evas_Object *obj);
typedef void (*Evas_Object_Intercept_Move_Cb) (void *data, Evas_Object *obj, Evas_Coord x, Evas_Coord y);
typedef void (*Evas_Object_Intercept_Resize_Cb) (void *data, Evas_Object *obj, Evas_Coord w, Evas_Coord h);
typedef void (*Evas_Object_Intercept_Raise_Cb) (void *data, Evas_Object *obj);
typedef void (*Evas_Object_Intercept_Lower_Cb) (void *data, Evas_Object *obj);
typedef void (*Evas_Object_Intercept_Stack_Above_Cb) (void *data, Evas_Object *obj, Evas_Object *above);
typedef void (*Evas_Object_Intercept_Stack_Below_Cb) (void *data, Evas_Object *obj, Evas_Object *above);
typedef void (*Evas_Object_Intercept_Layer_Set_Cb) (void *data, Evas_Object *obj, int l);
typedef void (*Evas_Object_Intercept_Color_Set_Cb) (void *data, Evas_Object *obj, int r, int g, int b, int a);
typedef void (*Evas_Object_Intercept_Clip_Set_Cb) (void *data, Evas_Object *obj, Evas_Object *clip);
typedef void (*Evas_Object_Intercept_Clip_Unset_Cb) (void *data, Evas_Object *obj);
   
   EAPI void              evas_object_intercept_show_callback_add        (Evas_Object *obj, Evas_Object_Intercept_Show_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);
   EAPI void             *evas_object_intercept_show_callback_del        (Evas_Object *obj, Evas_Object_Intercept_Show_Cb func) EINA_ARG_NONNULL(1, 2);
   EAPI void              evas_object_intercept_hide_callback_add        (Evas_Object *obj, Evas_Object_Intercept_Hide_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);
   EAPI void             *evas_object_intercept_hide_callback_del        (Evas_Object *obj, Evas_Object_Intercept_Hide_Cb func) EINA_ARG_NONNULL(1, 2);
   EAPI void              evas_object_intercept_move_callback_add        (Evas_Object *obj, Evas_Object_Intercept_Move_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);
   EAPI void             *evas_object_intercept_move_callback_del        (Evas_Object *obj, Evas_Object_Intercept_Move_Cb func) EINA_ARG_NONNULL(1, 2);
   EAPI void              evas_object_intercept_resize_callback_add      (Evas_Object *obj, Evas_Object_Intercept_Resize_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);
   EAPI void             *evas_object_intercept_resize_callback_del      (Evas_Object *obj, Evas_Object_Intercept_Resize_Cb func) EINA_ARG_NONNULL(1, 2);
   EAPI void              evas_object_intercept_raise_callback_add       (Evas_Object *obj, Evas_Object_Intercept_Raise_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);
   EAPI void             *evas_object_intercept_raise_callback_del       (Evas_Object *obj, Evas_Object_Intercept_Raise_Cb func) EINA_ARG_NONNULL(1, 2);
   EAPI void              evas_object_intercept_lower_callback_add       (Evas_Object *obj, Evas_Object_Intercept_Lower_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);
   EAPI void             *evas_object_intercept_lower_callback_del       (Evas_Object *obj, Evas_Object_Intercept_Lower_Cb func) EINA_ARG_NONNULL(1, 2);
   EAPI void              evas_object_intercept_stack_above_callback_add (Evas_Object *obj, Evas_Object_Intercept_Stack_Above_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);
   EAPI void             *evas_object_intercept_stack_above_callback_del (Evas_Object *obj, Evas_Object_Intercept_Stack_Above_Cb func) EINA_ARG_NONNULL(1, 2);
   EAPI void              evas_object_intercept_stack_below_callback_add (Evas_Object *obj, Evas_Object_Intercept_Stack_Below_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);
   EAPI void             *evas_object_intercept_stack_below_callback_del (Evas_Object *obj, Evas_Object_Intercept_Stack_Below_Cb func) EINA_ARG_NONNULL(1, 2);
   EAPI void              evas_object_intercept_layer_set_callback_add   (Evas_Object *obj, Evas_Object_Intercept_Layer_Set_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);
   EAPI void             *evas_object_intercept_layer_set_callback_del   (Evas_Object *obj, Evas_Object_Intercept_Layer_Set_Cb func) EINA_ARG_NONNULL(1, 2);
   EAPI void              evas_object_intercept_color_set_callback_add   (Evas_Object *obj, Evas_Object_Intercept_Color_Set_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);
   EAPI void             *evas_object_intercept_color_set_callback_del   (Evas_Object *obj, Evas_Object_Intercept_Color_Set_Cb func) EINA_ARG_NONNULL(1, 2);
   EAPI void              evas_object_intercept_clip_set_callback_add    (Evas_Object *obj, Evas_Object_Intercept_Clip_Set_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);
   EAPI void             *evas_object_intercept_clip_set_callback_del    (Evas_Object *obj, Evas_Object_Intercept_Clip_Set_Cb func) EINA_ARG_NONNULL(1, 2);
   EAPI void              evas_object_intercept_clip_unset_callback_add  (Evas_Object *obj, Evas_Object_Intercept_Clip_Unset_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);
   EAPI void             *evas_object_intercept_clip_unset_callback_del  (Evas_Object *obj, Evas_Object_Intercept_Clip_Unset_Cb func) EINA_ARG_NONNULL(1, 2);



/**
 * @defgroup Evas_Object_Specific Specific Object Functions
 *
 * Functions that work on specific objects.
 *
 */

/**
 * @defgroup Evas_Object_Rectangle Rectangle Object Functions
 *
 * Functions that operate on evas rectangle objects.
 *
 * @ingroup Evas_Object_Specific
 */
   EAPI Evas_Object      *evas_object_rectangle_add         (Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * @defgroup Evas_Object_Image Image Object Functions
 *
 * Functions used to create and manipulate image objects.
 *
 * Note - Image objects may return or accept "image data" in multiple
 * formats.  This is based on the colorspace of an object. Here is a
 * rundown on formats:
 *
 * EVAS_COLORSPACE_ARGB8888:
 *
 * This pixel format is a linear block of pixels, starting at the
 * top-left row by row until the bottom right of the image or pixel
 * region. All pixels are 32-bit unsigned int's with the high-byte
 * being alpha and the low byte being blue in the format ARGB. Alpha
 * may or may not be used by evas depending on the alpha flag of the
 * image, but if not used, should be set to 0xff anyway.
 *
 * This colorspace uses premultiplied alpha. That means that R, G and
 * B cannot exceed A in value. The conversion from non-premultiplied
 * colorspace is:
 *
 * R = (r * a) / 255; G = (g * a) / 255; B = (b * a) / 255;
 *
 * So 50% transparent blue will be: 0x80000080. This will not be
 * "dark" - just 50% transparent. Values are 0 == black, 255 == solid
 * or full red, green or blue.
 *
 * EVAS_COLORSPACE_YCBCR422P601_PL:
 *
 * This is a pointer-list indirected set of YUV (YCbCr) pixel
 * data. This means that the data returned or set is not actual pixel
 * data, but pointers TO lines of pixel data. The list of pointers
 * will first be N rows of pointers to the Y plane - pointing to the
 * first pixel at the start of each row in the Y plane. N is the
 * height of the image data in pixels. Each pixel in the Y, U and V
 * planes is 1 byte exactly, packed. The next N / 2 pointers will
 * point to rows in the U plane, and the next N / 2 pointers will
 * point to the V plane rows. U and V planes are half the horizontal
 * and vertical resolution of the Y plane.
 *
 * Row order is top to bottom and row pixels are stored left to right.
 *
 * There is a limitation that these images MUST be a multiple of 2
 * pixels in size horizontally or vertically. This is due to the U and
 * V planes being half resolution. Also note that this assumes the
 * itu601 YUV colorspace specification. This is defined for standard
 * television and mpeg streams.  HDTV may use the itu709
 * specification.
 *
 * Values are 0 to 255, indicating full or no signal in that plane
 * respectively.
 *
 * EVAS_COLORSPACE_YCBCR422P709_PL:
 *
 * Not implemented yet.
 *
 * EVAS_COLORSPACE_RGB565_A5P:
 *
 * In the process of being implemented in 1 engine only. This may change.
 *
 * This is a pointer to image data for 16-bit half-word pixel data in
 * 16bpp RGB 565 format (5 bits red, 6 bits green, 5 bits blue), with
 * the high-byte containing red and the low byte containing blue, per
 * pixel. This data is packed row by row from the top-left to the
 * bottom right.
 *
 * If the image has an alpha channel enabled there will be an extra
 * alpha plane after the color pixel plane. If not, then this data
 * will not exist and should not be accessed in any way. This plane is
 * a set of pixels with 1 byte per pixel defining the alpha values of
 * all pixels in the image from the top-left to the bottom right of
 * the image, row by row. Even though the values of the alpha pixels
 * can be 0 to 255, only values 0 through to 32 are used, 32 being
 * solid and 0 being transparent.
 *
 * RGB values can be 0 to 31 for red and blue and 0 to 63 for green,
 * with 0 being black and 31 or 63 being full red, green or blue
 * respectively. This colorspace is also pre-multiplied like
 * EVAS_COLORSPACE_ARGB8888 so:
 *
 * R = (r * a) / 32; G = (g * a) / 32; B = (b * a) / 32;
 *
 * @ingroup Evas_Object_Specific
 */
typedef void (*Evas_Object_Image_Pixels_Get_Cb) (void *data, Evas_Object *o);
   
   EAPI Evas_Object             *evas_object_image_add                    (Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;
   EAPI Evas_Object             *evas_object_image_filled_add             (Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

   EAPI void                     evas_object_image_file_set               (Evas_Object *obj, const char *file, const char *key) EINA_ARG_NONNULL(1);
   EAPI void                     evas_object_image_file_get               (const Evas_Object *obj, const char **file, const char **key) EINA_ARG_NONNULL(1, 2);
   EAPI void                     evas_object_image_border_set             (Evas_Object *obj, int l, int r, int t, int b) EINA_ARG_NONNULL(1);
   EAPI void                     evas_object_image_border_get             (const Evas_Object *obj, int *l, int *r, int *t, int *b) EINA_ARG_NONNULL(1);
   EAPI void                     evas_object_image_border_center_fill_set (Evas_Object *obj, Evas_Border_Fill_Mode fill) EINA_ARG_NONNULL(1);
   EAPI Evas_Border_Fill_Mode    evas_object_image_border_center_fill_get (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI void                     evas_object_image_filled_set             (Evas_Object *obj, Eina_Bool setting) EINA_ARG_NONNULL(1);
   EAPI Eina_Bool                evas_object_image_filled_get             (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI void                     evas_object_image_border_scale_set       (Evas_Object *obj, double scale);
   EAPI double                   evas_object_image_border_scale_get       (const Evas_Object *obj);
   EAPI void                     evas_object_image_fill_set               (Evas_Object *obj, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h) EINA_ARG_NONNULL(1);
   EAPI void                     evas_object_image_fill_get               (const Evas_Object *obj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h) EINA_ARG_NONNULL(1);
   EAPI void                     evas_object_image_fill_spread_set        (Evas_Object *obj, Evas_Fill_Spread spread) EINA_ARG_NONNULL(1);
   EAPI Evas_Fill_Spread         evas_object_image_fill_spread_get        (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI void                     evas_object_image_size_set               (Evas_Object *obj, int w, int h) EINA_ARG_NONNULL(1);
   EAPI void                     evas_object_image_size_get               (const Evas_Object *obj, int *w, int *h) EINA_ARG_NONNULL(1);
   EAPI int                      evas_object_image_stride_get             (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI Evas_Load_Error          evas_object_image_load_error_get         (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI void                     evas_object_image_data_set               (Evas_Object *obj, void *data) EINA_ARG_NONNULL(1);
   EAPI void                    *evas_object_image_data_convert           (Evas_Object *obj, Evas_Colorspace to_cspace) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI void                    *evas_object_image_data_get               (const Evas_Object *obj, Eina_Bool for_writing) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI void                     evas_object_image_data_copy_set          (Evas_Object *obj, void *data) EINA_ARG_NONNULL(1);
   EAPI void                     evas_object_image_data_update_add        (Evas_Object *obj, int x, int y, int w, int h) EINA_ARG_NONNULL(1);
   EAPI void                     evas_object_image_alpha_set              (Evas_Object *obj, Eina_Bool has_alpha) EINA_ARG_NONNULL(1);
   EAPI Eina_Bool                evas_object_image_alpha_get              (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI void                     evas_object_image_smooth_scale_set       (Evas_Object *obj, Eina_Bool smooth_scale) EINA_ARG_NONNULL(1);
   EAPI Eina_Bool                evas_object_image_smooth_scale_get       (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI void                     evas_object_image_preload                (Evas_Object *obj, Eina_Bool cancel) EINA_ARG_NONNULL(1);
   EAPI void                     evas_object_image_reload                 (Evas_Object *obj) EINA_ARG_NONNULL(1);
   EAPI Eina_Bool                evas_object_image_save                   (const Evas_Object *obj, const char *file, const char *key, const char *flags)  EINA_ARG_NONNULL(1, 2);
   EAPI Eina_Bool                evas_object_image_pixels_import          (Evas_Object *obj, Evas_Pixel_Import_Source *pixels) EINA_ARG_NONNULL(1, 2);
   EAPI void                     evas_object_image_pixels_get_callback_set(Evas_Object *obj, Evas_Object_Image_Pixels_Get_Cb func, void *data) EINA_ARG_NONNULL(1, 2);
   EAPI void                     evas_object_image_pixels_dirty_set       (Evas_Object *obj, Eina_Bool dirty) EINA_ARG_NONNULL(1);
   EAPI Eina_Bool                evas_object_image_pixels_dirty_get       (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI void                     evas_object_image_load_dpi_set           (Evas_Object *obj, double dpi) EINA_ARG_NONNULL(1);
   EAPI double                   evas_object_image_load_dpi_get           (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI void                     evas_object_image_load_size_set          (Evas_Object *obj, int w, int h) EINA_ARG_NONNULL(1);
   EAPI void                     evas_object_image_load_size_get          (const Evas_Object *obj, int *w, int *h) EINA_ARG_NONNULL(1);
   EAPI void                     evas_object_image_load_scale_down_set    (Evas_Object *obj, int scale_down) EINA_ARG_NONNULL(1);
   EAPI int                      evas_object_image_load_scale_down_get    (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI void                     evas_object_image_load_region_set        (Evas_Object *obj, int x, int y, int w, int h) EINA_ARG_NONNULL(1);
   EAPI void                     evas_object_image_load_region_get        (const Evas_Object *obj, int *x, int *y, int *w, int *h) EINA_ARG_NONNULL(1);
   EAPI void                     evas_object_image_colorspace_set         (Evas_Object *obj, Evas_Colorspace cspace) EINA_ARG_NONNULL(1);
   EAPI Evas_Colorspace          evas_object_image_colorspace_get         (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI void                     evas_object_image_native_surface_set     (Evas_Object *obj, Evas_Native_Surface *surf) EINA_ARG_NONNULL(1, 2);
   EAPI Evas_Native_Surface     *evas_object_image_native_surface_get     (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI void                     evas_object_image_scale_hint_set         (Evas_Object *obj, Evas_Image_Scale_Hint hint) EINA_ARG_NONNULL(1);
   EAPI Evas_Image_Scale_Hint    evas_object_image_scale_hint_get         (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI void                     evas_object_image_content_hint_set       (Evas_Object *obj, Evas_Image_Content_Hint hint) EINA_ARG_NONNULL(1);
   EAPI Evas_Image_Content_Hint  evas_object_image_content_hint_get       (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

/**
 * @defgroup Evas_Object_Text Text Object Functions
 *
 * Functions that operate on single line, single style text objects.
 *
 * For multiline and multiple style text, see @ref Evas_Object_Textblock.
 *
 * @ingroup Evas_Object_Specific
 */
   typedef enum _Evas_Text_Style_Type
     {
	EVAS_TEXT_STYLE_PLAIN,
	EVAS_TEXT_STYLE_SHADOW,
	EVAS_TEXT_STYLE_OUTLINE,
	EVAS_TEXT_STYLE_SOFT_OUTLINE,
	EVAS_TEXT_STYLE_GLOW,
	EVAS_TEXT_STYLE_OUTLINE_SHADOW,
	EVAS_TEXT_STYLE_FAR_SHADOW,
	EVAS_TEXT_STYLE_OUTLINE_SOFT_SHADOW,
	EVAS_TEXT_STYLE_SOFT_SHADOW,
	EVAS_TEXT_STYLE_FAR_SOFT_SHADOW
     } Evas_Text_Style_Type;

   EAPI Evas_Object      *evas_object_text_add              (Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;
   EAPI void              evas_object_text_font_source_set  (Evas_Object *obj, const char *font) EINA_ARG_NONNULL(1);
   EAPI const char       *evas_object_text_font_source_get  (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI void              evas_object_text_font_set         (Evas_Object *obj, const char *font, Evas_Font_Size size) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_text_font_get         (const Evas_Object *obj, const char **font, Evas_Font_Size *size) EINA_ARG_NONNULL(1, 2);
   EAPI void              evas_object_text_text_set         (Evas_Object *obj, const char *text) EINA_ARG_NONNULL(1);
   EAPI const char       *evas_object_text_text_get         (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI Evas_Coord        evas_object_text_ascent_get       (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI Evas_Coord        evas_object_text_descent_get      (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI Evas_Coord        evas_object_text_max_ascent_get   (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI Evas_Coord        evas_object_text_max_descent_get  (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI Evas_Coord        evas_object_text_horiz_advance_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI Evas_Coord        evas_object_text_vert_advance_get (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI Evas_Coord        evas_object_text_inset_get        (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI Eina_Bool         evas_object_text_char_pos_get     (const Evas_Object *obj, int pos, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch) EINA_ARG_NONNULL(1);
   EAPI int               evas_object_text_char_coords_get  (const Evas_Object *obj, Evas_Coord x, Evas_Coord y, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch) EINA_ARG_NONNULL(1);
   EAPI int               evas_object_text_last_up_to_pos   (const Evas_Object *obj, Evas_Coord x, Evas_Coord y) EINA_ARG_NONNULL(1);
   EAPI Evas_Text_Style_Type evas_object_text_style_get     (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI void              evas_object_text_style_set        (Evas_Object *obj, Evas_Text_Style_Type type) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_text_shadow_color_set (Evas_Object *obj, int r, int g, int b, int a) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_text_shadow_color_get (const Evas_Object *obj, int *r, int *g, int *b, int *a) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_text_glow_color_set   (Evas_Object *obj, int r, int g, int b, int a) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_text_glow_color_get   (const Evas_Object *obj, int *r, int *g, int *b, int *a) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_text_glow2_color_set  (Evas_Object *obj, int r, int g, int b, int a) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_text_glow2_color_get  (const Evas_Object *obj, int *r, int *g, int *b, int *a) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_text_outline_color_set(Evas_Object *obj, int r, int g, int b, int a) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_text_outline_color_get(const Evas_Object *obj, int *r, int *g, int *b, int *a) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_text_style_pad_get    (const Evas_Object *obj, int *l, int *r, int *t, int *b) EINA_ARG_NONNULL(1);

/**
 * @defgroup Evas_Object_Textblock Textblock Object Functions
 *
 * Functions used to create and manipulate textblock objects. Unlike
 * @ref Evas_Object_Text, these handle complex text, doing multiple
 * styles and multiline text based on HTML-like tags. Of these extra
 * features will be heavier on memory and processing cost.
 *
 * @todo put here some usage examples
 *
 * @ingroup Evas_Object_Specific
 */
   typedef struct _Evas_Textblock_Style                 Evas_Textblock_Style;
   typedef struct _Evas_Textblock_Cursor                Evas_Textblock_Cursor;
   /**
    * @typedef Evas_Object_Textblock_Node_Format
    * A format node.
    */
   typedef struct _Evas_Object_Textblock_Node_Format    Evas_Object_Textblock_Node_Format;
   typedef struct _Evas_Textblock_Rectangle             Evas_Textblock_Rectangle;

   struct _Evas_Textblock_Rectangle
     {
	Evas_Coord x, y, w, h;
     };

   typedef enum _Evas_Textblock_Text_Type
     {
	EVAS_TEXTBLOCK_TEXT_RAW,
	EVAS_TEXTBLOCK_TEXT_PLAIN,
	EVAS_TEXTBLOCK_TEXT_MARKUP
     } Evas_Textblock_Text_Type;

   typedef enum _Evas_Textblock_Cursor_Type
     {
	EVAS_TEXTBLOCK_CURSOR_UNDER,
	EVAS_TEXTBLOCK_CURSOR_BEFORE
     } Evas_Textblock_Cursor_Type;

   EAPI Evas_Object                 *evas_object_textblock_add(Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

   EAPI const char                  *evas_textblock_escape_string_get(const char *escape) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI const char                  *evas_textblock_string_escape_get(const char *string, int *len_ret) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI const char                  *evas_textblock_escape_string_range_get(const char *escape_start, const char *escape_end) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2) EINA_PURE;

   EAPI Evas_Textblock_Style        *evas_textblock_style_new(void) EINA_WARN_UNUSED_RESULT EINA_MALLOC;
   EAPI void                         evas_textblock_style_free(Evas_Textblock_Style *ts) EINA_ARG_NONNULL(1);
   EAPI void                         evas_textblock_style_set(Evas_Textblock_Style *ts, const char *text) EINA_ARG_NONNULL(1);
   EAPI const char                  *evas_textblock_style_get(const Evas_Textblock_Style *ts) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

   EAPI void                         evas_object_textblock_style_set(Evas_Object *obj, Evas_Textblock_Style *ts) EINA_ARG_NONNULL(1);
   EAPI const Evas_Textblock_Style  *evas_object_textblock_style_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI void                         evas_object_textblock_replace_char_set(Evas_Object *obj, const char *ch) EINA_ARG_NONNULL(1);
   EAPI const char                  *evas_object_textblock_replace_char_get(Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

   EAPI void                         evas_object_textblock_text_markup_set(Evas_Object *obj, const char *text) EINA_ARG_NONNULL(1);
   EAPI void                         evas_object_textblock_text_markup_prepend(Evas_Textblock_Cursor *cur, const char *text) EINA_ARG_NONNULL(1, 2);
   EAPI const char                  *evas_object_textblock_text_markup_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

   EAPI const Evas_Textblock_Cursor *evas_object_textblock_cursor_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI Evas_Textblock_Cursor       *evas_object_textblock_cursor_new(Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

   EAPI void                         evas_textblock_cursor_free(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);

   EAPI void                         evas_textblock_cursor_paragraph_first(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);
   EAPI void                         evas_textblock_cursor_paragraph_last(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);
   EAPI Eina_Bool                    evas_textblock_cursor_paragraph_next(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);
   EAPI Eina_Bool                    evas_textblock_cursor_paragraph_prev(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);
   EAPI const Evas_Object_Textblock_Node_Format *evas_textblock_node_format_first_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);
   EAPI const Evas_Object_Textblock_Node_Format *evas_textblock_node_format_last_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);
   EAPI const Evas_Object_Textblock_Node_Format *evas_textblock_node_format_next_get(const Evas_Object_Textblock_Node_Format *n) EINA_ARG_NONNULL(1);
   EAPI const Evas_Object_Textblock_Node_Format *evas_textblock_node_format_prev_get(const Evas_Object_Textblock_Node_Format *n) EINA_ARG_NONNULL(1);
   EAPI void                         evas_textblock_node_format_remove_pair(Evas_Object *obj, Evas_Object_Textblock_Node_Format *n) EINA_ARG_NONNULL(1, 2);
   EAPI void                         evas_textblock_cursor_set_at_format(Evas_Textblock_Cursor *cur, const Evas_Object_Textblock_Node_Format *n) EINA_ARG_NONNULL(1, 2);
   EAPI const Evas_Object_Textblock_Node_Format *evas_textblock_cursor_format_get(const Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);
   EAPI const char                  *evas_textblock_node_format_text_get(const Evas_Object_Textblock_Node_Format *cur) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);
   EAPI void                         evas_textblock_cursor_at_format_set(Evas_Textblock_Cursor *cur, const Evas_Object_Textblock_Node_Format *fmt) EINA_ARG_NONNULL(1, 2);
   EAPI Eina_Bool                    evas_textblock_cursor_format_is_visible_get(const Evas_Textblock_Cursor *cur) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI Eina_Bool                    evas_textblock_cursor_format_next(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);
   EAPI Eina_Bool                    evas_textblock_cursor_format_prev(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);
   EAPI Eina_Bool                    evas_textblock_cursor_is_format(const Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);
   EAPI Eina_Bool                    evas_textblock_cursor_char_next(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);
   EAPI Eina_Bool                    evas_textblock_cursor_char_prev(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);
   EAPI void                         evas_textblock_cursor_paragraph_char_first(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);
   EAPI void                         evas_textblock_cursor_paragraph_char_last(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);
   EAPI void                         evas_textblock_cursor_line_char_first(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);
   EAPI void                         evas_textblock_cursor_line_char_last(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);
   EAPI int                          evas_textblock_cursor_pos_get(const Evas_Textblock_Cursor *cur) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI void                         evas_textblock_cursor_pos_set(Evas_Textblock_Cursor *cur, int pos) EINA_ARG_NONNULL(1);
   EAPI Eina_Bool                    evas_textblock_cursor_line_set(Evas_Textblock_Cursor *cur, int line) EINA_ARG_NONNULL(1);
   EAPI int                          evas_textblock_cursor_compare(const Evas_Textblock_Cursor *cur1, const Evas_Textblock_Cursor *cur2) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2) EINA_PURE;
   EAPI void                         evas_textblock_cursor_copy(const Evas_Textblock_Cursor *cur, Evas_Textblock_Cursor *cur_dest) EINA_ARG_NONNULL(1, 2);

   EAPI int                          evas_textblock_cursor_text_append(Evas_Textblock_Cursor *cur, const char *text) EINA_ARG_NONNULL(1, 2);
   EAPI int                          evas_textblock_cursor_text_prepend(Evas_Textblock_Cursor *cur, const char *text) EINA_ARG_NONNULL(1, 2);

   EAPI Eina_Bool                    evas_textblock_cursor_format_append(Evas_Textblock_Cursor *cur, const char *format) EINA_ARG_NONNULL(1, 2);
   EAPI Eina_Bool                    evas_textblock_cursor_format_prepend(Evas_Textblock_Cursor *cur, const char *format) EINA_ARG_NONNULL(1, 2);
   EAPI void                         evas_textblock_cursor_node_delete(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);
   EAPI void                         evas_textblock_cursor_char_delete(Evas_Textblock_Cursor *cur) EINA_ARG_NONNULL(1);
   EAPI void                         evas_textblock_cursor_range_delete(Evas_Textblock_Cursor *cur1, Evas_Textblock_Cursor *cur2) EINA_ARG_NONNULL(1, 2);

   EAPI const char                  *evas_textblock_cursor_paragraph_text_get(const Evas_Textblock_Cursor *cur) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI int                          evas_textblock_cursor_paragraph_text_length_get(const Evas_Textblock_Cursor *cur) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI char                        *evas_textblock_cursor_range_text_get(const Evas_Textblock_Cursor *cur1, const Evas_Textblock_Cursor *cur2, Evas_Textblock_Text_Type format) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2) EINA_PURE;
   EAPI char                        *evas_textblock_cursor_content_get(const Evas_Textblock_Cursor *cur) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

   EAPI int                          evas_textblock_cursor_geometry_get(const Evas_Textblock_Cursor *cur, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch, Evas_BiDi_Direction *dir, Evas_Textblock_Cursor_Type ctype) EINA_ARG_NONNULL(1);
   EAPI int                          evas_textblock_cursor_char_geometry_get(const Evas_Textblock_Cursor *cur, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch) EINA_ARG_NONNULL(1);
   EAPI int                          evas_textblock_cursor_line_geometry_get(const Evas_Textblock_Cursor *cur, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch) EINA_ARG_NONNULL(1);
   EAPI Eina_Bool                    evas_textblock_cursor_char_coord_set(Evas_Textblock_Cursor *cur, Evas_Coord x, Evas_Coord y) EINA_ARG_NONNULL(1);
   EAPI int                          evas_textblock_cursor_line_coord_set(Evas_Textblock_Cursor *cur, Evas_Coord y) EINA_ARG_NONNULL(1);
   EAPI Eina_List                   *evas_textblock_cursor_range_geometry_get(const Evas_Textblock_Cursor *cur1, const Evas_Textblock_Cursor *cur2) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2) EINA_PURE;
   EAPI Eina_Bool                    evas_textblock_cursor_format_item_geometry_get(const Evas_Textblock_Cursor *cur, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch) EINA_ARG_NONNULL(1);

   EAPI Eina_Bool                    evas_textblock_cursor_eol_get(const Evas_Textblock_Cursor *cur) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

   EAPI Eina_Bool                    evas_object_textblock_line_number_geometry_get(const Evas_Object *obj, int line, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch) EINA_ARG_NONNULL(1);
   EAPI void                         evas_object_textblock_clear(Evas_Object *obj) EINA_ARG_NONNULL(1);
   EAPI void                         evas_object_textblock_size_formatted_get(const Evas_Object *obj, Evas_Coord *w, Evas_Coord *h) EINA_ARG_NONNULL(1);
   EAPI void                         evas_object_textblock_size_native_get(const Evas_Object *obj, Evas_Coord *w, Evas_Coord *h) EINA_ARG_NONNULL(1);
   EAPI void                         evas_object_textblock_style_insets_get(const Evas_Object *obj, Evas_Coord *l, Evas_Coord *r, Evas_Coord *t, Evas_Coord *b) EINA_ARG_NONNULL(1);

/**
 * @defgroup Evas_Line_Group Line Object Functions
 *
 * Functions used to deal with evas line objects.
 *
 * @ingroup Evas_Object_Specific
 */
   EAPI Evas_Object      *evas_object_line_add              (Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;
   EAPI void              evas_object_line_xy_set           (Evas_Object *obj, Evas_Coord x1, Evas_Coord y1, Evas_Coord x2, Evas_Coord y2);
   EAPI void              evas_object_line_xy_get           (const Evas_Object *obj, Evas_Coord *x1, Evas_Coord *y1, Evas_Coord *x2, Evas_Coord *y2);

/**
 * @defgroup Evas_Object_Polygon Polygon Object Functions
 *
 * Functions that operate on evas polygon objects.
 *
 * Hint: as evas does not provide ellipse, smooth paths or circle, one
 * can calculate points and convert these to a polygon.
 *
 * @ingroup Evas_Object_Specific
 */
   EAPI Evas_Object      *evas_object_polygon_add           (Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;
   EAPI void              evas_object_polygon_point_add     (Evas_Object *obj, Evas_Coord x, Evas_Coord y) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_polygon_points_clear  (Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * @defgroup Evas_Smart_Group Smart Functions
 *
 * Functions that deal with Evas_Smart's, creating definition
 * (classes) of objects that will have customized behavior for methods
 * like evas_object_move(), evas_object_resize(),
 * evas_object_clip_set() and others.
 *
 * These objects will accept the generic methods defined in @ref
 * Evas_Object_Group and the extensions defined in @ref
 * Evas_Smart_Object_Group. There are couple of existent smart objects
 * in Evas itself, see @ref Evas_Object_Box, @ref Evas_Object_Table
 * and @ref Evas_Smart_Object_Clipped.
 */

/**
 * @def EVAS_SMART_CLASS_VERSION
 * The version you have to put into the version field in the smart
 * class struct
 * @ingroup Evas_Smart_Group
 */
#define EVAS_SMART_CLASS_VERSION 4
/**
 * @struct _Evas_Smart_Class
 * a smart object class
 * @ingroup Evas_Smart_Group
 */
struct _Evas_Smart_Class
{
   const char *name; /**< the string name of the class */
   int         version;
   void  (*add)         (Evas_Object *o);
   void  (*del)         (Evas_Object *o);
   void  (*move)        (Evas_Object *o, Evas_Coord x, Evas_Coord y);
   void  (*resize)      (Evas_Object *o, Evas_Coord w, Evas_Coord h);
   void  (*show)        (Evas_Object *o);
   void  (*hide)        (Evas_Object *o);
   void  (*color_set)   (Evas_Object *o, int r, int g, int b, int a);
   void  (*clip_set)    (Evas_Object *o, Evas_Object *clip);
   void  (*clip_unset)  (Evas_Object *o);
   void  (*calculate)   (Evas_Object *o);
   void  (*member_add)  (Evas_Object *o, Evas_Object *child);
   void  (*member_del)  (Evas_Object *o, Evas_Object *child);

   const Evas_Smart_Class          *parent; /**< this class inherits from this parent */
   const Evas_Smart_Cb_Description *callbacks; /**< callbacks at this level, NULL terminated */
   void                            *interfaces; /**< to be used in a future near you */
   const void                      *data;
};

/**
 * @struct _Evas_Smart_Cb_Description
 *
 * Describes a callback used by a smart class
 * evas_object_smart_callback_call(), particularly useful to explain
 * to user and its code (ie: introspection) what the parameter @c
 * event_info will contain.
 *
 * @ingroup Evas_Smart_Group
 */
struct _Evas_Smart_Cb_Description
{
   const char *name; /**< callback name, ie: "changed" */

   /**
    * @brief Hint type of @c event_info parameter of Evas_Smart_Cb.
    *
    * The type string uses the pattern similar to
    *
    * http://dbus.freedesktop.org/doc/dbus-specification.html#message-protocol-signatures
    *
    * but extended to optionally include variable names within
    * brackets preceding types. Example:
    *
    * @li Structure with two integers:
    *     @c "(ii)"
    *
    * @li Structure called 'x' with two integers named 'a' and 'b':
    *     @c "[x]([a]i[b]i)"
    *
    * @li Array of integers:
    *     @c "ai"
    *
    * @li Array called 'x' of struct with two integers:
    *     @c "[x]a(ii)"
    *
    * @note This type string is used as a hint and is @b not validated
    *       or enforced anyhow. Implementors should make the best use
    *       of it to help bindings, documentation and other users of
    *       introspection features.
    */
   const char *type;
};

/**
 * @def EVAS_SMART_CLASS_INIT_NULL
 * Initializer to zero a whole Evas_Smart_Class structure.
 *
 * @see EVAS_SMART_CLASS_INIT_VERSION
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION_PARENT
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION_PARENT_CALLBACKS
 * @ingroup Evas_Smart_Group
 */
#define EVAS_SMART_CLASS_INIT_NULL {NULL, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}

/**
 * @def EVAS_SMART_CLASS_INIT_VERSION
 * Initializer to zero a whole Evas_Smart_Class structure and set version.
 *
 * Similar to EVAS_SMART_CLASS_INIT_NULL, but will set version field to
 * latest EVAS_SMART_CLASS_VERSION.
 *
 * @see EVAS_SMART_CLASS_INIT_NULL
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION_PARENT
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION_PARENT_CALLBACKS
 * @ingroup Evas_Smart_Group
 */
#define EVAS_SMART_CLASS_INIT_VERSION {NULL, EVAS_SMART_CLASS_VERSION, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}

/**
 * @def EVAS_SMART_CLASS_INIT_NAME_VERSION
 * Initializer to zero a whole Evas_Smart_Class structure and set name
 * and version.
 *
 * Similar to EVAS_SMART_CLASS_INIT_NULL, but will set version field to
 * latest EVAS_SMART_CLASS_VERSION and name to the specified value.
 *
 * It will keep a reference to name field as a "const char *", that is,
 * name must be available while the structure is used (hint: static or global!)
 * and will not be modified.
 *
 * @see EVAS_SMART_CLASS_INIT_NULL
 * @see EVAS_SMART_CLASS_INIT_VERSION
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION_PARENT
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION_PARENT_CALLBACKS
 * @ingroup Evas_Smart_Group
 */
#define EVAS_SMART_CLASS_INIT_NAME_VERSION(name) {name, EVAS_SMART_CLASS_VERSION, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}

/**
 * @def EVAS_SMART_CLASS_INIT_NAME_VERSION_PARENT
 * Initializer to zero a whole Evas_Smart_Class structure and set name,
 * version and parent class.
 *
 * Similar to EVAS_SMART_CLASS_INIT_NULL, but will set version field to
 * latest EVAS_SMART_CLASS_VERSION, name to the specified value and
 * parent class.
 *
 * It will keep a reference to name field as a "const char *", that is,
 * name must be available while the structure is used (hint: static or global!)
 * and will not be modified. Similarly, parent reference will be kept.
 *
 * @see EVAS_SMART_CLASS_INIT_NULL
 * @see EVAS_SMART_CLASS_INIT_VERSION
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION_PARENT_CALLBACKS
 * @ingroup Evas_Smart_Group
 */
#define EVAS_SMART_CLASS_INIT_NAME_VERSION_PARENT(name, parent) {name, EVAS_SMART_CLASS_VERSION, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, parent, NULL, NULL}

/**
 * @def EVAS_SMART_CLASS_INIT_NAME_VERSION_PARENT_CALLBACKS
 * Initializer to zero a whole Evas_Smart_Class structure and set name,
 * version, parent class and callbacks definition.
 *
 * Similar to EVAS_SMART_CLASS_INIT_NULL, but will set version field to
 * latest EVAS_SMART_CLASS_VERSION, name to the specified value, parent
 * class and callbacks at this level.
 *
 * It will keep a reference to name field as a "const char *", that is,
 * name must be available while the structure is used (hint: static or global!)
 * and will not be modified. Similarly, parent and callbacks reference
 * will be kept.
 *
 * @see EVAS_SMART_CLASS_INIT_NULL
 * @see EVAS_SMART_CLASS_INIT_VERSION
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION_PARENT
 * @ingroup Evas_Smart_Group
 */
#define EVAS_SMART_CLASS_INIT_NAME_VERSION_PARENT_CALLBACKS(name, parent, callbacks) {name, EVAS_SMART_CLASS_VERSION, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, parent, callbacks, NULL}

/**
 * @def EVAS_SMART_SUBCLASS_NEW
 *
 * Convenience macro to subclass a Smart Class.
 *
 * This macro saves some typing when writing a Smart Class derived from
 * another one. In order to work, the user needs to provide some functions
 * adhering to the following guidelines.
 *  - @<prefix@>_smart_set_user(): the internal _smart_set function will call
 *    this one provided by the user after inheriting everything from the
 *    parent, which should take care of setting the right member functions
 *    for the class.
 *  - @<prefix@>_parent_sc: pointer to the smart class of the parent. When calling
 *    parent functions from overloaded ones, use this global variable.
 *  - @<prefix@>_smart_class_new(): this function returns the Evas_Smart needed
 *    to create smart objects with this class, should be called by the public
 *    _add() function.
 *  - If this new class should be subclassable as well, a public _smart_set()
 *    function is desirable to fill the class used as parent by the children.
 *    It's up to the user to provide this interface, which will most likely
 *    call @<prefix@>_smart_set() to get the job done.
 *
 * @param smart_name The name used for the Smart Class. e.g: "Evas_Object_Box".
 * @param prefix Prefix used for all variables and functions defined.
 * @param api_type Type of the structure used as API for the Smart Class. Either Evas_Smart_Class or something derived from it.
 * @param parent_type Type of the parent class API.
 * @param parent_func Function that gets the parent class. e.g: evas_object_box_smart_class_get().
 * @param cb_desc Array of callback descriptions for this Smart Class.
 *
 * @ingroup Evas_Smart_Group
 */
#define EVAS_SMART_SUBCLASS_NEW(smart_name, prefix, api_type, parent_type, parent_func, cb_desc) \
  static const parent_type * prefix##_parent_sc = NULL;			\
  static void prefix##_smart_set_user(api_type *api);			\
  static void prefix##_smart_set(api_type *api)				\
  {									\
     Evas_Smart_Class *sc;						\
     if (!(sc = (Evas_Smart_Class *)api))				\
       return;								\
     if (!prefix##_parent_sc)						\
       prefix##_parent_sc = parent_func();				\
     evas_smart_class_inherit(sc, (const Evas_Smart_Class *)prefix##_parent_sc); \
     prefix##_smart_set_user(api);					\
  }									\
  static Evas_Smart * prefix##_smart_class_new(void)			\
  {									\
     static Evas_Smart *smart = NULL;					\
     static api_type api;						\
     if (!smart)							\
       {								\
	  Evas_Smart_Class *sc = (Evas_Smart_Class *)&api;		\
	  memset(&api, 0, sizeof(api_type));				\
	  sc->version = EVAS_SMART_CLASS_VERSION;			\
	  sc->name = smart_name;					\
	  sc->callbacks = cb_desc;					\
	  prefix##_smart_set(&api);					\
	  smart = evas_smart_class_new(sc);				\
       }								\
     return smart;							\
  }

/**
 * @def EVAS_SMART_DATA_ALLOC
 * Convenience macro to allocate smart data only if needed.
 *
 * When writing a subclassable smart object, the .add function will need
 * to check if the smart private data was already allocated by some child
 * object or not. This macro makes it easier to do it.
 *
 * @param o Evas object passed to the .add function
 * @param priv_type The type of the data to allocate
 * @ingroup Evas_Smart_Group
 */
#define EVAS_SMART_DATA_ALLOC(o, priv_type) \
   priv_type *priv; \
   priv = evas_object_smart_data_get(o); \
   if (!priv) { \
      priv = (priv_type *)calloc(1, sizeof(priv_type)); \
      if (!priv) return; \
      evas_object_smart_data_set(o, priv); \
   }

   EAPI void                             evas_smart_free                     (Evas_Smart *s) EINA_ARG_NONNULL(1);
   EAPI Evas_Smart                      *evas_smart_class_new                (const Evas_Smart_Class *sc) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;
   EAPI const Evas_Smart_Class          *evas_smart_class_get                (const Evas_Smart *s) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

   EAPI void                            *evas_smart_data_get                 (const Evas_Smart *s) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI const Evas_Smart_Cb_Description *evas_smart_callback_description_find(const Evas_Smart *s, const char *name) EINA_ARG_NONNULL(1, 2) EINA_PURE;

   EAPI Eina_Bool                        evas_smart_class_inherit_full       (Evas_Smart_Class *sc, const Evas_Smart_Class *parent_sc, unsigned int parent_sc_size) EINA_ARG_NONNULL(1, 2);
  /**
   * @def evas_smart_class_inherit
   * Easy to use version of evas_smart_class_inherit_full().
   *
   * This version will use sizeof(parent_sc), copying everything.
   *
   * @param sc child class, will have methods copied from @a parent_sc
   * @param parent_sc parent class, will provide contents to be copied.
   * @return 1 on success, 0 on failure.
   * @ingroup Evas_Smart_Group
   */
#define evas_smart_class_inherit(sc, parent_sc) evas_smart_class_inherit_full(sc, parent_sc, sizeof(*parent_sc))

/**
 * @defgroup Evas_Smart_Object_Group Smart Object Functions
 *
 * Functions dealing with evas smart objects (instances).
 *
 * Smart objects are groupings of primitive evas objects that behave as a
 * cohesive group. For instance, a file manager icon may be a smart object
 * composed of an image object, a text label and two rectangles that appear
 * behind the image and text when the icon is selected. As a smart object,
 * the normal evas api could be used on the icon object.
 *
 * @see @ref Evas_Smart_Group for class definitions.
 */
   EAPI Evas_Object      *evas_object_smart_add             (Evas *e, Evas_Smart *s) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2) EINA_MALLOC;
   EAPI void              evas_object_smart_member_add      (Evas_Object *obj, Evas_Object *smart_obj) EINA_ARG_NONNULL(1, 2);
   EAPI void              evas_object_smart_member_del      (Evas_Object *obj) EINA_ARG_NONNULL(1);
   EAPI Evas_Object      *evas_object_smart_parent_get      (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI Eina_Bool         evas_object_smart_type_check      (const Evas_Object *obj, const char *type) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2) EINA_PURE;
   EAPI Eina_Bool         evas_object_smart_type_check_ptr  (const Evas_Object *obj, const char *type) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2) EINA_PURE;
   EAPI Eina_List        *evas_object_smart_members_get     (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI Evas_Smart       *evas_object_smart_smart_get       (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI void             *evas_object_smart_data_get        (const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI void              evas_object_smart_data_set        (Evas_Object *obj, void *data) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_smart_callback_add    (Evas_Object *obj, const char *event, Evas_Smart_Cb func, const void *data) EINA_ARG_NONNULL(1, 2, 3);
   EAPI void             *evas_object_smart_callback_del    (Evas_Object *obj, const char *event, Evas_Smart_Cb func) EINA_ARG_NONNULL(1, 2, 3);
   EAPI void              evas_object_smart_callback_call   (Evas_Object *obj, const char *event, void *event_info) EINA_ARG_NONNULL(1, 2);

   EAPI Eina_Bool         evas_object_smart_callbacks_descriptions_set(Evas_Object *obj, const Evas_Smart_Cb_Description *descriptions) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_smart_callbacks_descriptions_get(const Evas_Object *obj, const Evas_Smart_Cb_Description ***class_descriptions, unsigned int *class_count, const Evas_Smart_Cb_Description ***instance_descriptions, unsigned int *instance_count) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_smart_callback_description_find(const Evas_Object *obj, const char *name, const Evas_Smart_Cb_Description **class_description, const Evas_Smart_Cb_Description **instance_description) EINA_ARG_NONNULL(1, 2);

   EAPI void              evas_object_smart_changed         (Evas_Object *obj) EINA_ARG_NONNULL(1);
   EAPI void              evas_object_smart_need_recalculate_set(Evas_Object *obj, Eina_Bool value) EINA_ARG_NONNULL(1);
   EAPI Eina_Bool         evas_object_smart_need_recalculate_get(const Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI void              evas_object_smart_calculate       (Evas_Object *obj) EINA_ARG_NONNULL(1);

   EAPI void              evas_smart_objects_calculate      (Evas *e);

/**
 * @defgroup Evas_Smart_Object_Clipped Clipped Smart Object
 *
 * Clipped smart object is a base to construct other smart objects
 * that based on the concept of having an internal clipper that is
 * applied to all its other children. This clipper will control the
 * visibility, clipping and color of sibling objects (remember that
 * the clipping is recursive, and clipper color modulates the color of
 * its clippees). By default, this base will also move children
 * relatively to the parent, and delete them when parent is
 * deleted. In other words, it is the base for simple object grouping.
 *
 * @see evas_object_smart_clipped_smart_set()
 *
 * @ingroup Evas_Smart_Object_Group
 */

/**
 * Every subclass should provide this at the beginning of their own
 * data set with evas_object_smart_data_set().
 */
  typedef struct _Evas_Object_Smart_Clipped_Data Evas_Object_Smart_Clipped_Data;
  struct _Evas_Object_Smart_Clipped_Data
  {
     Evas_Object *clipper;
     Evas        *evas;
  };

   EAPI Evas_Object            *evas_object_smart_clipped_clipper_get   (Evas_Object *obj) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI void                    evas_object_smart_clipped_smart_set     (Evas_Smart_Class *sc) EINA_ARG_NONNULL(1);
   EAPI const Evas_Smart_Class *evas_object_smart_clipped_class_get     (void) EINA_CONST;

   EAPI void                    evas_object_smart_move_children_relative(Evas_Object *obj, Evas_Coord dx, Evas_Coord dy) EINA_ARG_NONNULL(1);

/**
 * @defgroup Evas_Object_Box Box (Sequence) Smart Object.
 *
 * Convenience smart object that packs children as a sequence using
 * a layout function specified by user. There are a couple of helper
 * layout functions, all of them using children size hints to define
 * their size and alignment inside their cell space.
 *
 * @see @ref Evas_Object_Group_Size_Hints
 *
 * @ingroup Evas_Smart_Object_Group
 */
/**
 * @typedef Evas_Object_Box_Api
 * Smart Class extension providing extra box requirements.
 * @ingroup Evas_Object_Box
 */
   typedef struct _Evas_Object_Box_Api        Evas_Object_Box_Api;
/**
 * @typedef Evas_Object_Box_Data
 * Smart instance data providing box requirements.
 * @ingroup Evas_Object_Box
 */
   typedef struct _Evas_Object_Box_Data       Evas_Object_Box_Data;
/**
 * @typedef Evas_Object_Box_Option
 * The base structure for a box option.
 * @ingroup Evas_Object_Box
 */
   typedef struct _Evas_Object_Box_Option     Evas_Object_Box_Option;
   typedef void (*Evas_Object_Box_Layout) (Evas_Object *o, Evas_Object_Box_Data *priv, void *user_data);

/**
 * @def EVAS_OBJECT_BOX_API_VERSION
 * @ingroup Evas_Object_Box
 */
#define EVAS_OBJECT_BOX_API_VERSION 1
/**
 * @struct _Evas_Object_Box_Api
 *
 * This structure should be used by any class that wants to inherit
 * from box to provide custom behavior not allowed only by providing a
 * layout function with evas_object_box_layout_set().
 *
 * @extends Evas_Smart_Class
 * @ingroup Evas_Object_Box
 */
   struct _Evas_Object_Box_Api
   {
      Evas_Smart_Class          base;
      int                       version;
      Evas_Object_Box_Option *(*append)           (Evas_Object *o, Evas_Object_Box_Data *priv, Evas_Object *child);
      Evas_Object_Box_Option *(*prepend)          (Evas_Object *o, Evas_Object_Box_Data *priv, Evas_Object *child);
      Evas_Object_Box_Option *(*insert_before)    (Evas_Object *o, Evas_Object_Box_Data *priv, Evas_Object *child, const Evas_Object *reference);
      Evas_Object_Box_Option *(*insert_after)     (Evas_Object *o, Evas_Object_Box_Data *priv, Evas_Object *child, const Evas_Object *reference);
      Evas_Object_Box_Option *(*insert_at)        (Evas_Object *o, Evas_Object_Box_Data *priv, Evas_Object *child, unsigned int pos);
      Evas_Object            *(*remove)           (Evas_Object *o, Evas_Object_Box_Data *priv, Evas_Object *child);
      Evas_Object            *(*remove_at)        (Evas_Object *o, Evas_Object_Box_Data *priv, unsigned int pos);
      Eina_Bool               (*property_set)     (Evas_Object *o, Evas_Object_Box_Option *opt, int property, va_list args);
      Eina_Bool               (*property_get)     (Evas_Object *o, Evas_Object_Box_Option *opt, int property, va_list args);
      const char             *(*property_name_get)(Evas_Object *o, int property);
      int                     (*property_id_get)  (Evas_Object *o, const char *name);
      Evas_Object_Box_Option *(*option_new)       (Evas_Object *o, Evas_Object_Box_Data *priv, Evas_Object *child);
      void                    (*option_free)      (Evas_Object *o, Evas_Object_Box_Data *priv, Evas_Object_Box_Option *opt);
   };

/**
 * @def EVAS_OBJECT_BOX_API_INIT
 * Initializer for whole Evas_Object_Box_Api structure.
 *
 * @param smart_class_init initializer to use for the "base" field
 * (Evas_Smart_Class).
 *
 * @see EVAS_SMART_CLASS_INIT_NULL
 * @see EVAS_SMART_CLASS_INIT_VERSION
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION
 * @see EVAS_OBJECT_BOX_API_INIT_NULL
 * @see EVAS_OBJECT_BOX_API_INIT_VERSION
 * @see EVAS_OBJECT_BOX_API_INIT_NAME_VERSION
 * @ingroup Evas_Object_Box
 */
#define EVAS_OBJECT_BOX_API_INIT(smart_class_init) {smart_class_init, EVAS_OBJECT_BOX_API_VERSION, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}

/**
 * @def EVAS_OBJECT_BOX_API_INIT_NULL
 * Initializer to zero a whole Evas_Object_Box_Api structure.
 *
 * @see EVAS_OBJECT_BOX_API_INIT_VERSION
 * @see EVAS_OBJECT_BOX_API_INIT_NAME_VERSION
 * @see EVAS_OBJECT_BOX_API_INIT
 * @ingroup Evas_Object_Box
 */
#define EVAS_OBJECT_BOX_API_INIT_NULL EVAS_OBJECT_BOX_API_INIT(EVAS_SMART_CLASS_INIT_NULL)

/**
 * @def EVAS_OBJECT_BOX_API_INIT_VERSION
 * Initializer to zero a whole Evas_Object_Box_Api structure and set version.
 *
 * Similar to EVAS_OBJECT_BOX_API_INIT_NULL, but will set version field of
 * Evas_Smart_Class (base field) to latest EVAS_SMART_CLASS_VERSION
 *
 * @see EVAS_OBJECT_BOX_API_INIT_NULL
 * @see EVAS_OBJECT_BOX_API_INIT_NAME_VERSION
 * @see EVAS_OBJECT_BOX_API_INIT
 * @ingroup Evas_Object_Box
 */
#define EVAS_OBJECT_BOX_API_INIT_VERSION EVAS_OBJECT_BOX_API_INIT(EVAS_SMART_CLASS_INIT_VERSION)

/**
 * @def EVAS_OBJECT_BOX_API_INIT_NAME_VERSION
 * Initializer to zero a whole Evas_Object_Box_Api structure and set
 * name and version.
 *
 * Similar to EVAS_OBJECT_BOX_API_INIT_NULL, but will set version field of
 * Evas_Smart_Class (base field) to latest EVAS_SMART_CLASS_VERSION and name
 * to the specific value.
 *
 * It will keep a reference to name field as a "const char *", that is,
 * name must be available while the structure is used (hint: static or global!)
 * and will not be modified.
 *
 * @see EVAS_OBJECT_BOX_API_INIT_NULL
 * @see EVAS_OBJECT_BOX_API_INIT_VERSION
 * @see EVAS_OBJECT_BOX_API_INIT
 * @ingroup Evas_Object_Box
 */
#define EVAS_OBJECT_BOX_API_INIT_NAME_VERSION(name) EVAS_OBJECT_BOX_API_INIT(EVAS_SMART_CLASS_INIT_NAME_VERSION(name))

/**
 * @struct _Evas_Object_Box_Data
 *
 * This structure augments clipped smart object's instance data,
 * providing extra members required by generic box implementation. If
 * a subclass inherits from #Evas_Object_Box_Api then it may augment
 * #Evas_Object_Box_Data to fit its own needs.
 *
 * @extends Evas_Object_Smart_Clipped_Data
 * @ingroup Evas_Object_Box
 */
   struct _Evas_Object_Box_Data
   {
      Evas_Object_Smart_Clipped_Data   base;
      const Evas_Object_Box_Api       *api;
      struct {
	 double                        h, v;
      } align;
      struct {
	 Evas_Coord                    h, v;
      } pad;
      Eina_List                       *children;
      struct {
	 Evas_Object_Box_Layout        cb;
	 void                         *data;
	 void                        (*free_data)(void *data);
      } layout;
      Eina_Bool                        layouting : 1;
      Eina_Bool                        children_changed : 1;
   };

   struct _Evas_Object_Box_Option
   {
      Evas_Object *obj;
      Eina_Bool    max_reached:1;
      Eina_Bool    min_reached:1;
      Evas_Coord   alloc_size;
   };

   EAPI void                       evas_object_box_smart_set                             (Evas_Object_Box_Api *api) EINA_ARG_NONNULL(1);
   EAPI const Evas_Object_Box_Api *evas_object_box_smart_class_get                       (void) EINA_CONST;
   EAPI void                       evas_object_box_layout_set                            (Evas_Object *o, Evas_Object_Box_Layout cb, const void *data, void (*free_data)(void *data)) EINA_ARG_NONNULL(1, 2);

   EAPI Evas_Object               *evas_object_box_add                                   (Evas *evas) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;
   EAPI Evas_Object               *evas_object_box_add_to                                (Evas_Object *parent) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

   EAPI void                       evas_object_box_layout_horizontal                     (Evas_Object *o, Evas_Object_Box_Data *priv, void *data) EINA_ARG_NONNULL(1, 2);
   EAPI void                       evas_object_box_layout_vertical                       (Evas_Object *o, Evas_Object_Box_Data *priv, void *data) EINA_ARG_NONNULL(1, 2);
   EAPI void                       evas_object_box_layout_homogeneous_vertical           (Evas_Object *o, Evas_Object_Box_Data *priv, void *data) EINA_ARG_NONNULL(1, 2);
   EAPI void                       evas_object_box_layout_homogeneous_horizontal         (Evas_Object *o, Evas_Object_Box_Data *priv, void *data) EINA_ARG_NONNULL(1, 2);
   EAPI void                       evas_object_box_layout_homogeneous_max_size_horizontal(Evas_Object *o, Evas_Object_Box_Data *priv, void *data) EINA_ARG_NONNULL(1, 2);
   EAPI void                       evas_object_box_layout_homogeneous_max_size_vertical  (Evas_Object *o, Evas_Object_Box_Data *priv, void *data) EINA_ARG_NONNULL(1, 2);
   EAPI void                       evas_object_box_layout_flow_horizontal                (Evas_Object *o, Evas_Object_Box_Data *priv, void *data) EINA_ARG_NONNULL(1, 2);
   EAPI void                       evas_object_box_layout_flow_vertical                  (Evas_Object *o, Evas_Object_Box_Data *priv, void *data) EINA_ARG_NONNULL(1, 2);
   EAPI void                       evas_object_box_layout_stack                          (Evas_Object *o, Evas_Object_Box_Data *priv, void *data) EINA_ARG_NONNULL(1, 2);

   EAPI void                       evas_object_box_align_set                             (Evas_Object *o, double horizontal, double vertical) EINA_ARG_NONNULL(1);
   EAPI void                       evas_object_box_align_get                             (const Evas_Object *o, double *horizontal, double *vertical) EINA_ARG_NONNULL(1);
   EAPI void                       evas_object_box_padding_set                           (Evas_Object *o, Evas_Coord horizontal, Evas_Coord vertical) EINA_ARG_NONNULL(1);
   EAPI void                       evas_object_box_padding_get                           (const Evas_Object *o, Evas_Coord *horizontal, Evas_Coord *vertical) EINA_ARG_NONNULL(1);

   EAPI Evas_Object_Box_Option    *evas_object_box_append                                (Evas_Object *o, Evas_Object *child) EINA_ARG_NONNULL(1, 2);
   EAPI Evas_Object_Box_Option    *evas_object_box_prepend                               (Evas_Object *o, Evas_Object *child) EINA_ARG_NONNULL(1, 2);
   EAPI Evas_Object_Box_Option    *evas_object_box_insert_before                         (Evas_Object *o, Evas_Object *child, const Evas_Object *reference) EINA_ARG_NONNULL(1, 2, 3);
   EAPI Evas_Object_Box_Option    *evas_object_box_insert_after                          (Evas_Object *o, Evas_Object *child, const Evas_Object *referente) EINA_ARG_NONNULL(1, 2, 3);
   EAPI Evas_Object_Box_Option    *evas_object_box_insert_at                             (Evas_Object *o, Evas_Object *child, unsigned int pos) EINA_ARG_NONNULL(1, 2);
   EAPI Eina_Bool                  evas_object_box_remove                                (Evas_Object *o, Evas_Object *child) EINA_ARG_NONNULL(1, 2);
   EAPI Eina_Bool                  evas_object_box_remove_at                             (Evas_Object *o, unsigned int pos) EINA_ARG_NONNULL(1);
   EAPI Eina_Bool                  evas_object_box_remove_all                            (Evas_Object *o, Eina_Bool clear) EINA_ARG_NONNULL(1);
   EAPI Eina_Iterator             *evas_object_box_iterator_new                          (const Evas_Object *o) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;
   EAPI Eina_Accessor             *evas_object_box_accessor_new                          (const Evas_Object *o) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;
   EAPI Eina_List                 *evas_object_box_children_get                          (const Evas_Object *o) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

   EAPI const char                *evas_object_box_option_property_name_get              (Evas_Object *o, int property) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI int                        evas_object_box_option_property_id_get                (Evas_Object *o, const char *name) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2) EINA_PURE;
   EAPI Eina_Bool                  evas_object_box_option_property_set                   (Evas_Object *o, Evas_Object_Box_Option *opt, int property, ...) EINA_ARG_NONNULL(1, 2);
   EAPI Eina_Bool                  evas_object_box_option_property_vset                  (Evas_Object *o, Evas_Object_Box_Option *opt, int property, va_list args) EINA_ARG_NONNULL(1, 2);
   EAPI Eina_Bool                  evas_object_box_option_property_get                   (Evas_Object *o, Evas_Object_Box_Option *opt, int property, ...) EINA_ARG_NONNULL(1, 2);
   EAPI Eina_Bool                  evas_object_box_option_property_vget                  (Evas_Object *o, Evas_Object_Box_Option *opt, int property, va_list args) EINA_ARG_NONNULL(1, 2);

/**
 * @defgroup Evas_Object_Table Table Smart Object.
 *
 * Convenience smart object that packs children using a tabular
 * layout using children size hints to define their size and
 * alignment inside their cell space.
 *
 * @see @ref Evas_Object_Group_Size_Hints
 *
 * @ingroup Evas_Smart_Object_Group
 */
   EAPI Evas_Object                        *evas_object_table_add             (Evas *evas) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;
   EAPI Evas_Object                        *evas_object_table_add_to          (Evas_Object *parent) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;
   EAPI void                                evas_object_table_homogeneous_set (Evas_Object *o, Evas_Object_Table_Homogeneous_Mode homogeneous) EINA_ARG_NONNULL(1);
   EAPI Evas_Object_Table_Homogeneous_Mode  evas_object_table_homogeneous_get (const Evas_Object *o) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI void                                evas_object_table_padding_set     (Evas_Object *o, Evas_Coord horizontal, Evas_Coord vertical) EINA_ARG_NONNULL(1);
   EAPI void                                evas_object_table_padding_get     (const Evas_Object *o, Evas_Coord *horizontal, Evas_Coord *vertical) EINA_ARG_NONNULL(1);
   EAPI void                                evas_object_table_align_set       (Evas_Object *o, double horizontal, double vertical) EINA_ARG_NONNULL(1);
   EAPI void                                evas_object_table_align_get       (const Evas_Object *o, double *horizontal, double *vertical) EINA_ARG_NONNULL(1);

   EAPI Eina_Bool                           evas_object_table_pack            (Evas_Object *o, Evas_Object *child, unsigned short col, unsigned short row, unsigned short colspan, unsigned short rowspan) EINA_ARG_NONNULL(1, 2);
   EAPI Eina_Bool                           evas_object_table_unpack          (Evas_Object *o, Evas_Object *child) EINA_ARG_NONNULL(1, 2);
   EAPI void                                evas_object_table_clear           (Evas_Object *o, Eina_Bool clear) EINA_ARG_NONNULL(1);

   EAPI void                                evas_object_table_col_row_size_get(const Evas_Object *o, int *cols, int *rows) EINA_ARG_NONNULL(1);
   EAPI Eina_Iterator                      *evas_object_table_iterator_new    (const Evas_Object *o) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;
   EAPI Eina_Accessor                      *evas_object_table_accessor_new    (const Evas_Object *o) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;
   EAPI Eina_List                          *evas_object_table_children_get    (const Evas_Object *o) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;
   EAPI Evas_Object                        *evas_object_table_child_get       (const Evas_Object *o, unsigned short col, unsigned short row) EINA_ARG_NONNULL(1);

/**
 * @defgroup Evas_Proxy Evas Proxy Objects
 *
 * @brief Provides a way of applying effects to complete objects.
 *
 * A proxy object is a visible copy of another object.  Generally a map or
 * similar effect will be applied to the proxy to apply some sort of rendering
 * effect to.
 * 
 * Proxies are generally used for special effects.
 */
   EAPI Evas_Object                         *evas_object_proxy_add            (Evas *e) EINA_MALLOC;
   EAPI Eina_Bool                            evas_object_proxy_source_set     (Evas_Object *o, Evas_Object *source);
   EAPI Evas_Object                         *evas_object_proxy_source_get     (Evas_Object *o);
   EAPI Eina_Bool                            evas_object_proxy_source_unset   (Evas_Object *o);

/**
 * @defgroup Evas_Cserve Shared Image Cache Server
 *
 * Provides client-server infrastructure to share bitmaps across
 * multiple processes, saving data and processing power.
 */
   typedef struct _Evas_Cserve_Stats       Evas_Cserve_Stats;
   typedef struct _Evas_Cserve_Image_Cache Evas_Cserve_Image_Cache;
   typedef struct _Evas_Cserve_Image       Evas_Cserve_Image;
   typedef struct _Evas_Cserve_Config      Evas_Cserve_Config;

/**
 * Statistics about server that shares cached bitmaps.
 * @ingroup Evas_Cserve
 */
   struct _Evas_Cserve_Stats
     {
        int    saved_memory; /**< current saved memory, in bytes */
        int    wasted_memory; /**< current wasted memory, in bytes */
        int    saved_memory_peak; /**< peak of saved memory, in bytes */
        int    wasted_memory_peak; /**< peak of wasted memory, in bytes */
        double saved_time_image_header_load; /**< time, in seconds, saved in header loads by sharing cached loads instead */
        double saved_time_image_data_load; /**< time, in seconds, saved in data loads by sharing cached loads instead */
     };

/**
 * Cache of images shared by server.
 * @ingroup Evas_Cserve
 */
   struct _Evas_Cserve_Image_Cache
     {
        struct {
           int     mem_total;
           int     count;
        } active, cached;
        Eina_List *images;
     };

/**
 * An image shared by the server.
 * @ingroup Evas_Cserve
 */
   struct _Evas_Cserve_Image
     {
        const char *file, *key;
        int         w, h;
        time_t      file_mod_time;
        time_t      file_checked_time;
        time_t      cached_time;
        int         refcount;
        int         data_refcount;
        int         memory_footprint;
        double      head_load_time;
        double      data_load_time;
        Eina_Bool   alpha : 1;
        Eina_Bool   data_loaded : 1;
        Eina_Bool   active : 1;
        Eina_Bool   dead : 1;
        Eina_Bool   useless : 1;
     };

/**
 * Configuration that controls the server that shares cached bitmaps.
 * @ingroup Evas_Cserve
 */
    struct _Evas_Cserve_Config
     {
        int cache_max_usage;
        int cache_item_timeout;
        int cache_item_timeout_check;
     };

   EAPI Eina_Bool         evas_cserve_want_get                   (void) EINA_WARN_UNUSED_RESULT EINA_PURE;
   EAPI Eina_Bool         evas_cserve_connected_get              (void) EINA_WARN_UNUSED_RESULT;
   EAPI Eina_Bool         evas_cserve_stats_get                  (Evas_Cserve_Stats *stats) EINA_WARN_UNUSED_RESULT;
   EAPI void              evas_cserve_image_cache_contents_clean (Evas_Cserve_Image_Cache *cache) EINA_PURE;
   EAPI Eina_Bool         evas_cserve_config_get                 (Evas_Cserve_Config *config) EINA_WARN_UNUSED_RESULT EINA_PURE;
   EAPI Eina_Bool         evas_cserve_config_set                 (const Evas_Cserve_Config *config) EINA_WARN_UNUSED_RESULT EINA_PURE;
   EAPI void              evas_cserve_disconnect                 (void);

/**
 * @defgroup Evas_Utils General Utilities
 *
 * Some functions that are handy but are not specific of canvas or
 * objects.
 */
   EAPI const char       *evas_load_error_str               (Evas_Load_Error error);

/* Evas utility routines for color space conversions */
/* hsv color space has h in the range 0.0 to 360.0, and s,v in the range 0.0 to 1.0 */
/* rgb color space has r,g,b in the range 0 to 255 */
   EAPI void              evas_color_hsv_to_rgb             (float h, float s, float v, int *r, int *g, int *b);
   EAPI void              evas_color_rgb_to_hsv             (int r, int g, int b, float *h, float *s, float *v);

/* argb color space has a,r,g,b in the range 0 to 255 */
   EAPI void              evas_color_argb_premul            (int a, int *r, int *g, int *b);
   EAPI void              evas_color_argb_unpremul          (int a, int *r, int *g, int *b);

   EAPI void              evas_data_argb_premul             (unsigned int *data, unsigned int len);
   EAPI void              evas_data_argb_unpremul           (unsigned int *data, unsigned int len);

/* string and font handling */
   EAPI int               evas_string_char_next_get         (const char *str, int pos, int *decoded) EINA_ARG_NONNULL(1);
   EAPI int               evas_string_char_prev_get         (const char *str, int pos, int *decoded) EINA_ARG_NONNULL(1);
   EAPI int               evas_string_char_len_get          (const char *str) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

/**
 * TO BE DOCUMENTED:
 * @todo document key modifiers.
 */
   EAPI const Evas_Modifier *evas_key_modifier_get          (const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   EAPI const Evas_Lock     *evas_key_lock_get              (const Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
   
   EAPI Eina_Bool            evas_key_modifier_is_set       (const Evas_Modifier *m, const char *keyname) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2) EINA_PURE;
   
   EAPI Eina_Bool            evas_key_lock_is_set           (const Evas_Lock *l, const char *keyname) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2) EINA_PURE;
   
   EAPI void                 evas_key_modifier_add          (Evas *e, const char *keyname) EINA_ARG_NONNULL(1, 2);
   EAPI void                 evas_key_modifier_del          (Evas *e, const char *keyname) EINA_ARG_NONNULL(1, 2);
   EAPI void                 evas_key_lock_add              (Evas *e, const char *keyname) EINA_ARG_NONNULL(1, 2);
   EAPI void                 evas_key_lock_del              (Evas *e, const char *keyname) EINA_ARG_NONNULL(1, 2);
   
   EAPI void                 evas_key_modifier_on           (Evas *e, const char *keyname) EINA_ARG_NONNULL(1, 2);
   EAPI void                 evas_key_modifier_off          (Evas *e, const char *keyname) EINA_ARG_NONNULL(1, 2);
   EAPI void                 evas_key_lock_on               (Evas *e, const char *keyname) EINA_ARG_NONNULL(1, 2);
   EAPI void                 evas_key_lock_off              (Evas *e, const char *keyname) EINA_ARG_NONNULL(1, 2);
   
   EAPI Evas_Modifier_Mask   evas_key_modifier_mask_get     (const Evas *e, const char *keyname) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2) EINA_PURE;
   
   EAPI Eina_Bool            evas_object_key_grab           (Evas_Object *obj, const char *keyname, Evas_Modifier_Mask modifiers, Evas_Modifier_Mask not_modifiers, Eina_Bool exclusive) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2);
   EAPI void                 evas_object_key_ungrab         (Evas_Object *obj, const char *keyname, Evas_Modifier_Mask modifiers, Evas_Modifier_Mask not_modifiers) EINA_ARG_NONNULL(1, 2);

#ifdef __cplusplus
}
#endif

#endif
