#include "evas_common.h"
#include "evas_private.h"
#include "../engines/common/evas_convert_color.h"
#include "../engines/common/evas_convert_colorspace.h"
#include "../engines/common/evas_convert_yuv.h"

/* private magic number for image objects */
static const char o_type[] = "image";

/* private struct for rectangle object internal data */
typedef struct _Evas_Object_Image      Evas_Object_Image;

struct _Evas_Object_Image
{
   DATA32            magic;

   struct {
      int                  spread;
      Evas_Coord_Rectangle fill;
      struct {
	 short         w, h, stride;
      } image;
      struct {
	 short         l, r, t, b;
	 unsigned char fill;
         double        scale;
      } border;

      const char    *file;
      const char    *key;
      int            cspace;

      unsigned char  smooth_scale : 1;
      unsigned char  has_alpha :1;
      unsigned char  opaque :1;
      unsigned char  opaque_valid :1;
   } cur, prev;

   int               pixels_checked_out;
   int               load_error;
   Eina_List        *pixel_updates;

   struct {
      unsigned char  scale_down_by;
      double         dpi;
      short          w, h;
      struct {
         short       x, y, w, h;
      } region;
   } load_opts;

   struct {
      Evas_Object_Image_Pixels_Get_Cb  get_pixels;
      void                            *get_pixels_data;
   } func;

   Evas_Image_Scale_Hint   scale_hint;
   Evas_Image_Content_Hint content_hint;

   void             *engine_data;

   unsigned char     changed : 1;
   unsigned char     dirty_pixels : 1;
   unsigned char     filled : 1;
};

/* private methods for image objects */
static void evas_object_image_unload(Evas_Object *obj, Eina_Bool dirty);
static void evas_object_image_load(Evas_Object *obj);
static Evas_Coord evas_object_image_figure_x_fill(Evas_Object *obj, Evas_Coord start, Evas_Coord size, Evas_Coord *size_ret);
static Evas_Coord evas_object_image_figure_y_fill(Evas_Object *obj, Evas_Coord start, Evas_Coord size, Evas_Coord *size_ret);

static void evas_object_image_init(Evas_Object *obj);
static void *evas_object_image_new(void);
static void evas_object_image_render(Evas_Object *obj, void *output, void *context, void *surface, int x, int y);
static void evas_object_image_free(Evas_Object *obj);
static void evas_object_image_render_pre(Evas_Object *obj);
static void evas_object_image_render_post(Evas_Object *obj);

static unsigned int evas_object_image_id_get(Evas_Object *obj);
static unsigned int evas_object_image_visual_id_get(Evas_Object *obj);
static void *evas_object_image_engine_data_get(Evas_Object *obj);

static int evas_object_image_is_opaque(Evas_Object *obj);
static int evas_object_image_was_opaque(Evas_Object *obj);
static int evas_object_image_is_inside(Evas_Object *obj, Evas_Coord x, Evas_Coord y);
static int evas_object_image_has_opaque_rect(Evas_Object *obj);
static int evas_object_image_get_opaque_rect(Evas_Object *obj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h);
static int evas_object_image_can_map(Evas_Object *obj);

static void *evas_object_image_data_convert_internal(Evas_Object_Image *o, void *data, Evas_Colorspace to_cspace);
static void evas_object_image_filled_resize_listener(void *data, Evas *e, Evas_Object *obj, void *einfo);

static const Evas_Object_Func object_func =
{
   /* methods (compulsory) */
   evas_object_image_free,
     evas_object_image_render,
     evas_object_image_render_pre,
     evas_object_image_render_post,
     evas_object_image_id_get,
     evas_object_image_visual_id_get,
     evas_object_image_engine_data_get,
     /* these are optional. NULL = nothing */
     NULL,
     NULL,
     NULL,
     NULL,
     evas_object_image_is_opaque,
     evas_object_image_was_opaque,
     evas_object_image_is_inside,
     NULL,
     NULL,
     NULL,
     evas_object_image_has_opaque_rect,
     evas_object_image_get_opaque_rect,
     evas_object_image_can_map
};


/**
 * @addtogroup Evas_Object_Image
 * @{
 */

EVAS_MEMPOOL(_mp_obj);

/**
 * Creates a new image object on the given evas.
 *
 * @param e The given evas.
 * @return The created image object.
 */
EAPI Evas_Object *
evas_object_image_add(Evas *e)
{
   Evas_Object *obj;
   Evas_Object_Image *o;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   obj = evas_object_new(e);
   evas_object_image_init(obj);
   evas_object_inject(obj, e);
   o = (Evas_Object_Image *)(obj->object_data);
   o->cur.cspace = obj->layer->evas->engine.func->image_colorspace_get(obj->layer->evas->engine.data.output,
								       o->engine_data);
   return obj;
}

/**
 * Creates a new image object that automatically scales on the given evas.
 *
 * This is a helper around evas_object_image_add() and
 * evas_object_image_filled_set(), it will track object resizes and apply
 * evas_object_image_fill_set() with the new geometry.
 *
 * @see evas_object_image_add()
 * @see evas_object_image_filled_set()
 * @see evas_object_image_fill_set()
 */
EAPI Evas_Object *
evas_object_image_filled_add(Evas *e)
{
   Evas_Object *obj;
   obj = evas_object_image_add(e);
   evas_object_image_filled_set(obj, 1);
   return obj;
}

/**
 * Sets the filename and key of the given image object.
 *
 * If the file supports multiple data stored in it as eet, you can
 * specify the key to be used as the index of the image in this file.
 *
 * @param obj The given image object.
 * @param file The image filename.
 * @param key The image key in file, or NULL.
 */
EAPI void
evas_object_image_file_set(Evas_Object *obj, const char *file, const char *key)
{
   Evas_Object_Image *o;
   Evas_Image_Load_Opts lo;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   if ((o->cur.file) && (file) && (!strcmp(o->cur.file, file)))
     {
	if ((!o->cur.key) && (!key))
	  return;
	if ((o->cur.key) && (key) && (!strcmp(o->cur.key, key)))
	  return;
     }
/*
 * WTF? why cancel a null image preload? this is just silly (tm)
   if (!o->engine_data)
     obj->layer->evas->engine.func->image_data_preload_cancel(obj->layer->evas->engine.data.output,
							      o->engine_data,
							      obj);
 */
   
   if (o->cur.file) eina_stringshare_del(o->cur.file);
   if (o->cur.key) eina_stringshare_del(o->cur.key);
   if (file) o->cur.file = eina_stringshare_add(file);
   else o->cur.file = NULL;
   if (key) o->cur.key = eina_stringshare_add(key);
   else o->cur.key = NULL;
   o->prev.file = NULL;
   o->prev.key = NULL;
   if (o->engine_data)
     {
        obj->layer->evas->engine.func->image_data_preload_cancel(obj->layer->evas->engine.data.output,
                                                                 o->engine_data,
                                                                 obj);
        obj->layer->evas->engine.func->image_free(obj->layer->evas->engine.data.output,
                                                  o->engine_data);
     }
   o->load_error = EVAS_LOAD_ERROR_NONE;
   lo.scale_down_by = o->load_opts.scale_down_by;
   lo.dpi = o->load_opts.dpi;
   lo.w = o->load_opts.w;
   lo.h = o->load_opts.h;
   lo.region.x = o->load_opts.region.x;
   lo.region.y = o->load_opts.region.y;
   lo.region.w = o->load_opts.region.w;
   lo.region.h = o->load_opts.region.h;
   o->engine_data = obj->layer->evas->engine.func->image_load(obj->layer->evas->engine.data.output,
							      o->cur.file,
							      o->cur.key,
							      &o->load_error,
							      &lo);
   if (o->engine_data)
     {
	int w, h;
	int stride;

	obj->layer->evas->engine.func->image_size_get(obj->layer->evas->engine.data.output,
						      o->engine_data, &w, &h);
	if (obj->layer->evas->engine.func->image_stride_get)
	  obj->layer->evas->engine.func->image_stride_get(obj->layer->evas->engine.data.output,
							  o->engine_data, &stride);
	else
	  stride = w * 4;
	o->cur.has_alpha = obj->layer->evas->engine.func->image_alpha_get(obj->layer->evas->engine.data.output,
									  o->engine_data);
	o->cur.cspace = obj->layer->evas->engine.func->image_colorspace_get(obj->layer->evas->engine.data.output,
									    o->engine_data);
	o->cur.image.w = w;
	o->cur.image.h = h;
	o->cur.image.stride = stride;
     }
   else
     {
	if (o->load_error == EVAS_LOAD_ERROR_NONE)
	  o->load_error = EVAS_LOAD_ERROR_GENERIC;
	o->cur.has_alpha = 1;
	o->cur.cspace = EVAS_COLORSPACE_ARGB8888;
	o->cur.image.w = 0;
	o->cur.image.h = 0;
	o->cur.image.stride = 0;
     }
   o->changed = 1;
   evas_object_change(obj);
}

/**
 * Retrieves the filename and key of the given image object.
 *
 * @param obj The given image object.
 * @param file Location to store the image filename, or NULL.
 * @param key Location to store the image key, or NULL.
 */
EAPI void
evas_object_image_file_get(const Evas_Object *obj, const char **file, const char **key)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   if (file) *file = NULL;
   if (key) *key = NULL;
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   if (file) *file = NULL;
   if (key) *key = NULL;
   return;
   MAGIC_CHECK_END();
   if (file) *file = o->cur.file;
   if (key) *key = o->cur.key;
}

/**
 * Sets how much of each border of the given image object is not
 * to be scaled.
 *
 * When rendering, the image may be scaled to fit the size of the
 * image object. This function sets what area around the border of the
 * image is not to be scaled. This sort of function is useful for
 * widget theming, where, for example, buttons may be of varying
 * sizes, but the border size must remain constant.
 *
 * The units used for @p l, @p r, @p t and @p b are output units.
 *
 * @param obj The given image object.
 * @param l Distance of the left border that is not to be stretched.
 * @param r Distance of the right border that is not to be stretched.
 * @param t Distance of the top border that is not to be stretched.
 * @param b Distance of the bottom border that is not to be stretched.
 */
EAPI void
evas_object_image_border_set(Evas_Object *obj, int l, int r, int t, int b)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   if (l < 0) l = 0;
   if (r < 0) r = 0;
   if (t < 0) t = 0;
   if (b < 0) b = 0;
   if ((o->cur.border.l == l) &&
       (o->cur.border.r == r) &&
       (o->cur.border.t == t) &&
       (o->cur.border.b == b)) return;
   o->cur.border.l = l;
   o->cur.border.r = r;
   o->cur.border.t = t;
   o->cur.border.b = b;
   o->cur.opaque_valid = 0;
   o->changed = 1;
   evas_object_change(obj);
}

/**
 * Retrieves how much of each border of the given image object is not
 * to be scaled.
 *
 * See @ref evas_object_image_border_set for more details.
 *
 * @param obj The given image object.
 * @param l Location to store the left border width in, or NULL.
 * @param r Location to store the right border width in, or NULL.
 * @param t Location to store the top border width in, or NULL.
 * @param b Location to store the bottom border width in, or NULL.
 */
EAPI void
evas_object_image_border_get(const Evas_Object *obj, int *l, int *r, int *t, int *b)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   if (l) *l = 0;
   if (r) *r = 0;
   if (t) *t = 0;
   if (b) *b = 0;
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   if (l) *l = 0;
   if (r) *r = 0;
   if (t) *t = 0;
   if (b) *b = 0;
   return;
   MAGIC_CHECK_END();
   if (l) *l = o->cur.border.l;
   if (r) *r = o->cur.border.r;
   if (t) *t = o->cur.border.t;
   if (b) *b = o->cur.border.b;
}

/**
 * Sets if the center part of the given image object (not the border)
 * should be drawn.
 *
 * When rendering, the image may be scaled to fit the size of the
 * image object. This function sets if the center part of the scaled
 * image is to be drawn or left completely blank, or forced to be
 * solid. Very useful for frames and decorations.
 *
 * @param obj The given image object.
 * @param fill Fill mode of the middle.
 */
EAPI void
evas_object_image_border_center_fill_set(Evas_Object *obj, Evas_Border_Fill_Mode fill)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   if (fill == o->cur.border.fill) return;
   o->cur.border.fill = fill;
   o->changed = 1;
   evas_object_change(obj);
}

/**
 * Retrieves if the center of the given image object is to be drawn or
 * not.
 *
 * See @ref evas_object_image_fill_set for more details.
 *
 * @param obj The given image object.
 * @return Fill mode of the  center.
 */
EAPI Evas_Border_Fill_Mode
evas_object_image_border_center_fill_get(const Evas_Object *obj)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return 0;
   MAGIC_CHECK_END();
   return o->cur.border.fill;
}

/**
 * Sets if image fill property should track object size.
 *
 * If set to true, then every evas_object_resize() will automatically
 * trigger call to evas_object_image_fill_set() with the new size so
 * image will fill the whole object area.
 *
 * @param obj The given image object.
 * @param setting whether to follow object size.
 *
 * @see evas_object_image_filled_add()
 * @see evas_object_image_fill_set()
 */
EAPI void
evas_object_image_filled_set(Evas_Object *obj, Eina_Bool setting)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();

   setting = !!setting;
   if (o->filled == setting) return;

   o->filled = setting;
   if (!o->filled)
     evas_object_event_callback_del(obj, EVAS_CALLBACK_RESIZE, evas_object_image_filled_resize_listener);
   else
     {
	Evas_Coord w, h;

	evas_object_geometry_get(obj, NULL, NULL, &w, &h);
	evas_object_image_fill_set(obj, 0, 0, w, h);

	evas_object_event_callback_add(obj, EVAS_CALLBACK_RESIZE, evas_object_image_filled_resize_listener, NULL);
     }
}

/**
 * Retrieves if image fill property is tracking object size.
 *
 * @param obj The given image object.
 * @return 1 if it is tracking, 0 if not and evas_object_fill_set()
 * must be called manually.
 */
EAPI Eina_Bool
evas_object_image_filled_get(const Evas_Object *obj)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return 0;
   MAGIC_CHECK_END();

   return o->filled;
}

/**
 * Sets a scale factor (multiplier) for the borders of an image
 *
 * @param obj The given image object.
 * @param scale The scale factor (default is 1.0 - i.e. no scale)
 */
EAPI void
evas_object_image_border_scale_set(Evas_Object *obj, double scale)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   if (scale == o->cur.border.scale) return;
   o->cur.border.scale = scale;
   o->changed = 1;
   evas_object_change(obj);
}

/**
 * Retrieves the border scale factor
 *
 * See evas_object_image_border_scale_set()
 * 
 * @param obj The given image object.
 * @return The scale factor
 */
EAPI double
evas_object_image_border_scale_get(const Evas_Object *obj)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 1.0;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return 1.0;
   MAGIC_CHECK_END();
   return o->cur.border.scale;
}

/**
 * Sets the rectangle of the given image object that the image will be
 * drawn to.
 *
 * Note that the image will be tiled around this one rectangle. To
 * have only one copy of the image drawn, @p x and @p y must be 0 and
 * @p w and @p h need to be the width and height of the image object
 * respectively.
 *
 * The default values for the fill parameters is @p x = 0, @p y = 0,
 * @p w = 32 and @p h = 32.
 *
 * @param obj The given image object.
 * @param x The X coordinate for the top left corner of the image.
 * @param y The Y coordinate for the top left corner of the image.
 * @param w The width of the image.
 * @param h The height of the image.
 */
EAPI void
evas_object_image_fill_set(Evas_Object *obj, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h)
{
   Evas_Object_Image *o;

   if (w < 0) w = -w;
   if (h < 0) h = -h;
   if (w == 0) return;
   if (h == 0) return;
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   if ((o->cur.fill.x == x) &&
       (o->cur.fill.y == y) &&
       (o->cur.fill.w == w) &&
       (o->cur.fill.h == h)) return;
   o->cur.fill.x = x;
   o->cur.fill.y = y;
   o->cur.fill.w = w;
   o->cur.fill.h = h;
   o->cur.opaque_valid = 0;   
   o->changed = 1;
   evas_object_change(obj);
}

/**
 * Retrieves the dimensions of the rectangle of the given image object
 * that the image will be drawn to.
 *
 * See @ref evas_object_image_fill_set for more details.
 *
 * @param obj The given image object.
 * @param x Location to store the X coordinate for the top left corner of the image in, or NULL.
 * @param y Location to store the Y coordinate for the top left corner of the image in, or NULL.
 * @param w Location to store the width of the image in, or NULL.
 * @param h Location to store the height of the image in, or NULL.
 */
EAPI void
evas_object_image_fill_get(const Evas_Object *obj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   if (x) *x = 0;
   if (y) *y = 0;
   if (w) *w = 0;
   if (h) *h = 0;
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   if (x) *x = 0;
   if (y) *y = 0;
   if (w) *w = 0;
   if (h) *h = 0;
   return;
   MAGIC_CHECK_END();
   if (x) *x = o->cur.fill.x;
   if (y) *y = o->cur.fill.y;
   if (w) *w = o->cur.fill.w;
   if (h) *h = o->cur.fill.h;
}


/**
 * Sets the tiling mode for the given evas image object's fill.
 * @param   obj   The given evas image object.
 * @param   spread One of EVAS_TEXTURE_REFLECT, EVAS_TEXTURE_REPEAT,
 * EVAS_TEXTURE_RESTRICT, or EVAS_TEXTURE_PAD.
 */
EAPI void
evas_object_image_fill_spread_set(Evas_Object *obj, Evas_Fill_Spread spread)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   if (spread == (Evas_Fill_Spread)o->cur.spread) return;
   o->cur.spread = spread;
   o->changed = 1;
   evas_object_change(obj);
}

/**
 * Retrieves the spread (tiling mode) for the given image object's
 * fill.
 *
 * @param   obj The given evas image object.
 * @return  The current spread mode of the image object.
 */
EAPI Evas_Fill_Spread
evas_object_image_fill_spread_get(const Evas_Object *obj)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return EVAS_TEXTURE_REPEAT;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return EVAS_TEXTURE_REPEAT;
   MAGIC_CHECK_END();
   return (Evas_Fill_Spread)o->cur.spread;
}

/**
 * Sets the size of the given image object.
 *
 * This function will scale down or crop the image so that it is
 * treated as if it were at the given size. If the size given is
 * smaller than the image, it will be cropped. If the size given is
 * larger, then the image will be treated as if it were in the upper
 * left hand corner of a larger image that is otherwise transparent.
 *
 * @param obj The given image object.
 * @param w The new width of the image.
 * @param h The new height of the image.
 */
EAPI void
evas_object_image_size_set(Evas_Object *obj, int w, int h)
{
   Evas_Object_Image *o;
   int stride = 0;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   if (w < 1) w = 1;
   if (h < 1) h = 1;
   if (w > 32768) return;
   if (h > 32768) return;
   if ((w == o->cur.image.w) &&
       (h == o->cur.image.h)) return;
   o->cur.image.w = w;
   o->cur.image.h = h;
   if (o->engine_data)
      o->engine_data = obj->layer->evas->engine.func->image_size_set(obj->layer->evas->engine.data.output,
                                                                     o->engine_data,
                                                                     w, h);
   else
      o->engine_data = obj->layer->evas->engine.func->image_new_from_copied_data
      (obj->layer->evas->engine.data.output, w, h, NULL, o->cur.has_alpha,
          o->cur.cspace);
   
   if (o->engine_data)
     {
        if (obj->layer->evas->engine.func->image_scale_hint_set)
           obj->layer->evas->engine.func->image_scale_hint_set
           (obj->layer->evas->engine.data.output,
               o->engine_data, o->scale_hint);
        if (obj->layer->evas->engine.func->image_content_hint_set)
           obj->layer->evas->engine.func->image_content_hint_set
           (obj->layer->evas->engine.data.output,
               o->engine_data, o->content_hint);
        if (obj->layer->evas->engine.func->image_stride_get)
           obj->layer->evas->engine.func->image_stride_get
           (obj->layer->evas->engine.data.output,
               o->engine_data, &stride);
        else
           stride = w * 4;
     }
   else
      stride = w * 4;
   o->cur.image.stride = stride;

/* FIXME - in engine call above
   if (o->engine_data)
     o->engine_data = obj->layer->evas->engine.func->image_alpha_set(obj->layer->evas->engine.data.output,
								     o->engine_data,
								     o->cur.has_alpha);
*/
   EVAS_OBJECT_IMAGE_FREE_FILE_AND_KEY(o);
   o->changed = 1;
   evas_object_change(obj);
}

/**
 * Retrieves the size of the given image object.
 *
 * See @ref evas_object_image_size_set for more details.
 *
 * @param obj The given image object.
 * @param w Location to store the width of the image in, or NULL.
 * @param h Location to store the height of the image in, or NULL.
 */
EAPI void
evas_object_image_size_get(const Evas_Object *obj, int *w, int *h)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   if (w) *w = 0;
   if (h) *h = 0;
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   if (w) *w = 0;
   if (h) *h = 0;
   return;
   MAGIC_CHECK_END();
   if (w) *w = o->cur.image.w;
   if (h) *h = o->cur.image.h;
}

/**
 * Retrieves the row stride of the given image object,
 *
 * The row stride is the number of units between the start of a
 * row and the start of the next row.
 *
 * @param obj The given image object.
 * @return The stride of the image.
 */
EAPI int
evas_object_image_stride_get(const Evas_Object *obj)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return 0;
   MAGIC_CHECK_END();
   return o->cur.image.stride;
}

/**
 * Retrieves a number representing any error that occurred during the last
 * load of the given image object.
 *
 * @param obj The given image object.
 * @return A value giving the last error that occurred. It should be one of
 *         the @c EVAS_LOAD_ERROR_* values.  @c EVAS_LOAD_ERROR_NONE is
 *         returned if there was no error.
 */
EAPI Evas_Load_Error
evas_object_image_load_error_get(const Evas_Object *obj)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return 0;
   MAGIC_CHECK_END();
   return o->load_error;
}

/**
 * Converts the raw image data of the given image object to the
 * specified colorspace.
 *
 * Note that this function does not modify the raw image data.  If the
 * requested colorspace is the same as the image colorspace nothing is
 * done and NULL is returned. You should use
 * evas_object_image_colorspace_get() to check the current image
 * colorspace.
 *
 * See @ref evas_object_image_colorspace_get.
 *
 * @param obj The given image object.
 * @param to_cspace The colorspace to which the image raw data will be converted.
 * @return data A newly allocated data in the format specified by to_cspace.
 */
EAPI void *
evas_object_image_data_convert(Evas_Object *obj, Evas_Colorspace to_cspace)
{
   Evas_Object_Image *o;
   DATA32 *data;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return NULL;
   MAGIC_CHECK_END();
   if (!o->engine_data) return NULL;
   if (!o->cur.cspace == to_cspace) return NULL;
   data = NULL;
   o->engine_data = obj->layer->evas->engine.func->image_data_get(obj->layer->evas->engine.data.output,
								  o->engine_data,
								  0,
								  &data);
   return evas_object_image_data_convert_internal(o, data, to_cspace);
}

/**
 * Sets the raw image data of the given image object.
 *
 * Note that the raw data must be of the same size and colorspace of
 * the image. If data is NULL the current image data will be freed.
 *
 * @param obj The given image object.
 * @param data The raw data, or NULL.
 */
EAPI void
evas_object_image_data_set(Evas_Object *obj, void *data)
{
   Evas_Object_Image *o;
   void *p_data;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
#ifdef EVAS_FRAME_QUEUING
   if (o->engine_data)
     evas_common_pipe_op_image_flush(o->engine_data);
#endif
   p_data = o->engine_data;
   if (data)
     {
	if (o->engine_data)
	  o->engine_data = obj->layer->evas->engine.func->image_data_put(obj->layer->evas->engine.data.output,
									 o->engine_data,
									 data);
	else
	  o->engine_data = obj->layer->evas->engine.func->image_new_from_data(obj->layer->evas->engine.data.output,
									      o->cur.image.w,
									      o->cur.image.h,
									      data,
									      o->cur.has_alpha,
									      o->cur.cspace);
        if (o->engine_data)
          {
             int stride = 0;
             
             if (obj->layer->evas->engine.func->image_scale_hint_set)
                obj->layer->evas->engine.func->image_scale_hint_set
                (obj->layer->evas->engine.data.output,
                    o->engine_data, o->scale_hint);
             if (obj->layer->evas->engine.func->image_content_hint_set)
                obj->layer->evas->engine.func->image_content_hint_set
                (obj->layer->evas->engine.data.output,
                    o->engine_data, o->content_hint); 
             if (obj->layer->evas->engine.func->image_stride_get)
                obj->layer->evas->engine.func->image_stride_get
                (obj->layer->evas->engine.data.output,
                    o->engine_data, &stride);
             else
                stride = o->cur.image.w * 4;
             o->cur.image.stride = stride;
         }
     }
   else
     {
	if (o->engine_data)
	  obj->layer->evas->engine.func->image_free(obj->layer->evas->engine.data.output,
						    o->engine_data);
	o->load_error = EVAS_LOAD_ERROR_NONE;
	o->cur.image.w = 0;
	o->cur.image.h = 0;
	o->cur.image.stride = 0;
	o->engine_data = NULL;
     }
/* FIXME - in engine call above
   if (o->engine_data)
     o->engine_data = obj->layer->evas->engine.func->image_alpha_set(obj->layer->evas->engine.data.output,
								     o->engine_data,
								     o->cur.has_alpha);
*/
   if (o->pixels_checked_out > 0) o->pixels_checked_out--;
   if (p_data != o->engine_data)
     {
	EVAS_OBJECT_IMAGE_FREE_FILE_AND_KEY(o);
	o->pixels_checked_out = 0;
     }
   o->changed = 1;
   evas_object_change(obj);
}

/**
 * Get a pointer to the raw image data of the given image object.
 *
 * This function returns a pointer to an image object's internal pixel
 * buffer, for reading only or read/write. If you request it for
 * writing, the image will be marked dirty so that it gets redrawn at
 * the next update.
 *
 * This is best suited when you want to modify an existing image,
 * without changing its dimensions.
 *
 * @param obj The given image object.
 * @param for_writing Whether the data being retrieved will be modified.
 * @return The raw image data.
 */
EAPI void *
evas_object_image_data_get(const Evas_Object *obj, Eina_Bool for_writing)
{
   Evas_Object_Image *o;
   DATA32 *data;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return NULL;
   MAGIC_CHECK_END();
   if (!o->engine_data) return NULL;
#ifdef EVAS_FRAME_QUEUING
   evas_common_pipe_op_image_flush(o->engine_data);
#endif

   data = NULL;
   if (obj->layer->evas->engine.func->image_scale_hint_set)
      obj->layer->evas->engine.func->image_scale_hint_set
      (obj->layer->evas->engine.data.output,
          o->engine_data, o->scale_hint);
   if (obj->layer->evas->engine.func->image_content_hint_set)
      obj->layer->evas->engine.func->image_content_hint_set
      (obj->layer->evas->engine.data.output,
          o->engine_data, o->content_hint);
   o->engine_data = obj->layer->evas->engine.func->image_data_get(obj->layer->evas->engine.data.output,
								  o->engine_data,
								  for_writing,
								  &data);
   if (o->engine_data)
     {
        int stride = 0;

        if (obj->layer->evas->engine.func->image_stride_get)
           obj->layer->evas->engine.func->image_stride_get
           (obj->layer->evas->engine.data.output,
               o->engine_data, &stride);
        else
           stride = o->cur.image.w * 4;
        o->cur.image.stride = stride;
     }
   o->pixels_checked_out++;
   if (for_writing)
     {
	EVAS_OBJECT_IMAGE_FREE_FILE_AND_KEY(o);
     }

   return data;
}

/**
 * Preload image in the background
 *
 * This function request the preload of the data image in the
 * background. The worked is queued before being processed.
 *
 * If image data is already loaded, it will callback
 * EVAS_CALLBACK_IMAGE_PRELOADED immediatelly and do nothing else.
 *
 * If cancel is set, it will remove the image from the workqueue.
 *
 * @param obj The given image object.
 * @param cancel 0 means add to the workqueue, 1 remove it.
 */
EAPI void
evas_object_image_preload(Evas_Object *obj, Eina_Bool cancel)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return ;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return ;
   MAGIC_CHECK_END();
   if (!o->engine_data)
     {
	evas_object_inform_call_image_preloaded(obj);
	return;
     }
   if (cancel)
     obj->layer->evas->engine.func->image_data_preload_cancel(obj->layer->evas->engine.data.output,
							      o->engine_data,
							      obj);
   else
     obj->layer->evas->engine.func->image_data_preload_request(obj->layer->evas->engine.data.output,
							       o->engine_data,
							       obj);
}

/**
 * Replaces the raw image data of the given image object.
 *
 * This function lets the application replace an image object's
 * internal pixel buffer with a user-allocated one. For best results,
 * you should generally first call evas_object_image_size_set() with
 * the width and height for the new buffer.
 *
 * This call is best suited for when you will be using image data with
 * different dimensions than the existing image data, if any. If you
 * only need to modify the existing image in some fashion, then using
 * evas_object_image_data_get() is probably what you are after.
 *
 * Note that the caller is responsible for freeing the buffer when
 * finished with it, as user-set image data will not be automatically
 * freed when the image object is deleted.
 *
 * See @ref evas_object_image_data_get for more details.
 *
 * @param obj The given image object.
 * @param data The raw data.
 */
EAPI void
evas_object_image_data_copy_set(Evas_Object *obj, void *data)
{
   Evas_Object_Image *o;

   if (!data) return;
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   if ((o->cur.image.w <= 0) ||
       (o->cur.image.h <= 0)) return;
   if (o->engine_data)
     obj->layer->evas->engine.func->image_free(obj->layer->evas->engine.data.output,
					       o->engine_data);
   o->engine_data = obj->layer->evas->engine.func->image_new_from_copied_data(obj->layer->evas->engine.data.output,
									      o->cur.image.w,
									      o->cur.image.h,
									      data,
									      o->cur.has_alpha,
									      o->cur.cspace);
   if (o->engine_data)
     {
        int stride = 0;

        o->engine_data = obj->layer->evas->engine.func->image_alpha_set(obj->layer->evas->engine.data.output,
                                                                        o->engine_data,
                                                                        o->cur.has_alpha);
        if (obj->layer->evas->engine.func->image_scale_hint_set)
           obj->layer->evas->engine.func->image_scale_hint_set
           (obj->layer->evas->engine.data.output,
               o->engine_data, o->scale_hint);
        if (obj->layer->evas->engine.func->image_content_hint_set)
           obj->layer->evas->engine.func->image_content_hint_set
           (obj->layer->evas->engine.data.output,
               o->engine_data, o->content_hint);
        if (obj->layer->evas->engine.func->image_stride_get)
           obj->layer->evas->engine.func->image_stride_get
           (obj->layer->evas->engine.data.output,
               o->engine_data, &stride);
        else
           stride = o->cur.image.w * 4;
        o->cur.image.stride = stride;
     }
   o->pixels_checked_out = 0;
   EVAS_OBJECT_IMAGE_FREE_FILE_AND_KEY(o);
}

/**
 * Mark a sub-region of the given image object to be redrawn.
 *
 * This function schedules a particular rectangular region of an image
 * object to be updated (redrawn) at the next render.
 *
 * @param obj The given image object.
 * @param x X-offset of the region to be updated.
 * @param y Y-offset of the region to be updated.
 * @param w Width of the region to be updated.
 * @param h Height of the region to be updated.
 */
EAPI void
evas_object_image_data_update_add(Evas_Object *obj, int x, int y, int w, int h)
{
   Evas_Object_Image *o;
   Eina_Rectangle *r;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   RECTS_CLIP_TO_RECT(x, y, w, h, 0, 0, o->cur.image.w, o->cur.image.h);
   if ((w <= 0)  || (h <= 0)) return;
   NEW_RECT(r, x, y, w, h);
   if (r) o->pixel_updates = eina_list_append(o->pixel_updates, r);
   o->changed = 1;
   evas_object_change(obj);
}

/**
 * Enable or disable alpha channel of the given image object.
 *
 * This function sets a flag on an image object indicating whether or
 * not to use alpha channel data. A value of 1 indicates to use alpha
 * channel data, and 0 indicates to ignore any alpha channel
 * data. Note that this has nothing to do with an object's color as
 * manipulated by evas_object_color_set().
 *
 * @param obj The given image object.
 * @param has_alpha Whether to use alpha channel data or not.
 */
EAPI void
evas_object_image_alpha_set(Evas_Object *obj, Eina_Bool has_alpha)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   if (((has_alpha) && (o->cur.has_alpha)) ||
       ((!has_alpha) && (!o->cur.has_alpha)))
     return;
   o->cur.has_alpha = has_alpha;
   if (o->engine_data)
     {
        int stride = 0;
        
#ifdef EVAS_FRAME_QUEUING
        evas_common_pipe_op_image_flush(o->engine_data);
#endif
        o->engine_data = obj->layer->evas->engine.func->image_alpha_set(obj->layer->evas->engine.data.output,
								     o->engine_data,
								     o->cur.has_alpha);
        if (obj->layer->evas->engine.func->image_scale_hint_set)
           obj->layer->evas->engine.func->image_scale_hint_set
           (obj->layer->evas->engine.data.output,
               o->engine_data, o->scale_hint);
        if (obj->layer->evas->engine.func->image_content_hint_set)
           obj->layer->evas->engine.func->image_content_hint_set
           (obj->layer->evas->engine.data.output,
               o->engine_data, o->content_hint);
        if (obj->layer->evas->engine.func->image_stride_get)
           obj->layer->evas->engine.func->image_stride_get
           (obj->layer->evas->engine.data.output,
               o->engine_data, &stride);
        else
           stride = o->cur.image.w * 4;
        o->cur.image.stride = stride;
     }
   evas_object_image_data_update_add(obj, 0, 0, o->cur.image.w, o->cur.image.h);
   EVAS_OBJECT_IMAGE_FREE_FILE_AND_KEY(o);
}


/**
 * @brief Retrieves the alpha channel setting of the given image object.
 *
 * @param obj The given image object.
 * @return Whether the alpha channel data is being used.
 *
 * This function returns 1 if the image object's alpha channel is
 * being used, or 0 otherwise.
 *
 * See @ref evas_object_image_alpha_set for more details.
 */
EAPI Eina_Bool
evas_object_image_alpha_get(const Evas_Object *obj)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return 0;
   MAGIC_CHECK_END();
   return o->cur.has_alpha;
}

/**
 * Sets whether to use of high-quality image scaling algorithm
 * of the given image object.
 *
 * When enabled, a higher quality image scaling algorithm is used when
 * scaling images to sizes other than the source image. This gives
 * better results but is more computationally expensive.
 *
 * @param obj The given image object.
 * @param smooth_scale Whether to use smooth scale or not.
 */
EAPI void
evas_object_image_smooth_scale_set(Evas_Object *obj, Eina_Bool smooth_scale)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   if (((smooth_scale) && (o->cur.smooth_scale)) ||
       ((!smooth_scale) && (!o->cur.smooth_scale)))
     return;
   o->cur.smooth_scale = smooth_scale;
   o->changed = 1;
   evas_object_change(obj);
}

/**
 * Retrieves whether the given image object is using use a
 * high-quality image scaling algorithm.
 *
 * See @ref evas_object_image_smooth_scale_set for more details.
 *
 * @param obj The given image object.
 * @return Whether smooth scale is being used.
 */
EAPI Eina_Bool
evas_object_image_smooth_scale_get(const Evas_Object *obj)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return 0;
   MAGIC_CHECK_END();
   return o->cur.smooth_scale;
}

/**
 * Reload a image of the canvas.
 *
 * @param obj The given image object pointer.
 *
 * This function reloads a image of the given canvas.
 *
 */
EAPI void
evas_object_image_reload(Evas_Object *obj)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   if ((!o->cur.file) ||
       (o->pixels_checked_out > 0)) return;
   if (o->engine_data)
     o->engine_data = obj->layer->evas->engine.func->image_dirty_region(obj->layer->evas->engine.data.output, o->engine_data, 0, 0, o->cur.image.w, o->cur.image.h);
   evas_object_image_unload(obj, 1);
   evas_object_image_load(obj);
   o->prev.file = NULL;
   o->prev.key = NULL;
   o->changed = 1;
   evas_object_change(obj);
}

/**
 * Save the given image object to a file.
 *
 * Note that you should pass the filename extension when saving.  If
 * the file supports multiple data stored in it as eet, you can
 * specify the key to be used as the index of the image in this file.
 *
 * You can specify some flags when saving the image.  Currently
 * acceptable flags are quality and compress.  Eg.: "quality=100
 * compress=9"
 *
 * @param obj The given image object.
 * @param file The filename to be used to save the image.
 * @param key The image key in file, or NULL.
 * @param flags String containing the flags to be used.
 */
EAPI Eina_Bool
evas_object_image_save(const Evas_Object *obj, const char *file, const char *key, const char *flags)
{
   Evas_Object_Image *o;
   DATA32 *data = NULL;
   int quality = 80, compress = 9, ok = 0;
   RGBA_Image *im;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return 0;
   MAGIC_CHECK_END();

   if (!o->engine_data) return 0;
   o->engine_data = obj->layer->evas->engine.func->image_data_get(obj->layer->evas->engine.data.output,
								  o->engine_data,
								  0,
								  &data);
   if (flags)
     {
	char *p, *pp;
	char *tflags;

	tflags = alloca(strlen(flags) + 1);
	strcpy(tflags, flags);
	p = tflags;
	while (p)
	  {
	     pp = strchr(p, ' ');
	     if (pp) *pp = 0;
	     sscanf(p, "quality=%i", &quality);
	     sscanf(p, "compress=%i", &compress);
	     if (pp) p = pp + 1;
	     else break;
	  }
     }
   im = (RGBA_Image*) evas_cache_image_data(evas_common_image_cache_get(),
                                            o->cur.image.w,
                                            o->cur.image.h,
                                            data,
                                            o->cur.has_alpha,
                                            EVAS_COLORSPACE_ARGB8888);
   if (im)
     {
	if (o->cur.cspace == EVAS_COLORSPACE_ARGB8888)
	  im->image.data = data;
	else
	  im->image.data = evas_object_image_data_convert_internal(o,
								   data,
								   EVAS_COLORSPACE_ARGB8888);
	if (im->image.data)
	  {
	     ok = evas_common_save_image_to_file(im, file, key, quality, compress);

	     if (o->cur.cspace != EVAS_COLORSPACE_ARGB8888)
	       free(im->image.data);
	  }

	evas_cache_image_drop(&im->cache_entry);
     }
   return ok;
}

/**
 * Import pixels from given source to a given canvas image object.
 *
 * @param obj The given canvas object.
 * @param pixels The pixel's source to be imported.
 *
 * This function imports pixels from a given source to a given canvas image.
 *
 */
EAPI Eina_Bool
evas_object_image_pixels_import(Evas_Object *obj, Evas_Pixel_Import_Source *pixels)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return 0;
   MAGIC_CHECK_END();

   if ((pixels->w != o->cur.image.w) || (pixels->h != o->cur.image.h)) return 0;
   switch (pixels->format)
     {
#if 0
      case EVAS_PIXEL_FORMAT_ARGB32:
	  {
	     if (o->engine_data)
	       {
		  DATA32 *image_pixels = NULL;

		  o->engine_data =
		    obj->layer->evas->engine.func->image_data_get(obj->layer->evas->engine.data.output,
								  o->engine_data,
								  1,
								  &image_pixels);
/* FIXME: need to actualyl support this */
/*		  memcpy(image_pixels, pixels->rows, o->cur.image.w * o->cur.image.h * 4);*/
		  if (o->engine_data)
		    o->engine_data =
		    obj->layer->evas->engine.func->image_data_put(obj->layer->evas->engine.data.output,
								  o->engine_data,
								  image_pixels);
		  if (o->engine_data)
		    o->engine_data =
		    obj->layer->evas->engine.func->image_alpha_set(obj->layer->evas->engine.data.output,
								   o->engine_data,
								   o->cur.has_alpha);
		  o->changed = 1;
		  evas_object_change(obj);
	       }
	  }
	break;
#endif
#ifdef BUILD_CONVERT_YUV
      case EVAS_PIXEL_FORMAT_YUV420P_601:
	  {
	     if (o->engine_data)
	       {
		  DATA32 *image_pixels = NULL;

		  o->engine_data =
		    obj->layer->evas->engine.func->image_data_get(obj->layer->evas->engine.data.output,
								  o->engine_data,
								  1,
								  &image_pixels);
		  if (image_pixels)
		    evas_common_convert_yuv_420p_601_rgba((DATA8 **) pixels->rows,
							  (DATA8 *) image_pixels,
							  o->cur.image.w,
							  o->cur.image.h);
		  if (o->engine_data)
		    o->engine_data =
		    obj->layer->evas->engine.func->image_data_put(obj->layer->evas->engine.data.output,
								  o->engine_data,
								  image_pixels);
		  if (o->engine_data)
		    o->engine_data =
		    obj->layer->evas->engine.func->image_alpha_set(obj->layer->evas->engine.data.output,
								   o->engine_data,
								   o->cur.has_alpha);
		  o->changed = 1;
		  evas_object_change(obj);
	       }
	  }
	break;
#endif
      default:
	return 0;
	break;
     }
   return 1;
}

/**
 * Set the callback function to get pixels from a canva's image.
 *
 * @param obj The given canvas pointer.
 * @param func The callback function.
 * @param data The data pointer to be passed to @a func.
 *
 * This functions sets a function to be the callback function that get
 * pixes from a image of the canvas.
 *
 */
EAPI void
evas_object_image_pixels_get_callback_set(Evas_Object *obj, Evas_Object_Image_Pixels_Get_Cb func, void *data)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   o->func.get_pixels = func;
   o->func.get_pixels_data = data;
}

/**
 * Mark whether the given image object is dirty (needs to be redrawn).
 *
 * @param obj The given image object.
 * @param dirty Whether the image is dirty.
 */
EAPI void
evas_object_image_pixels_dirty_set(Evas_Object *obj, Eina_Bool dirty)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   if (dirty) o->dirty_pixels = 1;
   else o->dirty_pixels = 0;
   o->changed = 1;
   evas_object_change(obj);
}

/**
 * Retrieves whether the given image object is dirty (needs to be redrawn).
 *
 * @param obj The given image object.
 * @return Whether the image is dirty.
 */
EAPI Eina_Bool
evas_object_image_pixels_dirty_get(const Evas_Object *obj)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return 0;
   MAGIC_CHECK_END();
   if (o->dirty_pixels) return 1;
   return 0;
}

/**
 * Set the dpi resolution of a loaded image of the  canvas.
 *
 * @param obj The given canvas pointer.
 * @param dpi The new dpi resolution.
 *
 * This function set the dpi resolution of a given loaded canvas image.
 *
 */
EAPI void
evas_object_image_load_dpi_set(Evas_Object *obj, double dpi)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   if (dpi == o->load_opts.dpi) return;
   o->load_opts.dpi = dpi;
   if (o->cur.file)
     {
	evas_object_image_unload(obj, 0);
	evas_object_image_load(obj);
	o->changed = 1;
	evas_object_change(obj);
     }
}

/**
 * Get the dpi resolution of a loaded image of the canvas.
 *
 * @param obj The given canvas pointer.
 * @return The dpi resolution of the given canvas image.
 *
 * This function returns the dpi resolution of given canvas image.
 *
 */
EAPI double
evas_object_image_load_dpi_get(const Evas_Object *obj)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0.0;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return 0.0;
   MAGIC_CHECK_END();
   return o->load_opts.dpi;
}

/**
 * Set the size of a loaded image of the canvas.
 *
 * @param obj The given canvas object.
 * @param w The new width of the canvas image given.
 * @param h Th new height of the canvas image given.
 *
 * This function sets a new size for the given canvas image.
 *
 */
EAPI void
evas_object_image_load_size_set(Evas_Object *obj, int w, int h)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   if ((o->load_opts.w == w) && (o->load_opts.h == h)) return;
   o->load_opts.w = w;
   o->load_opts.h = h;
   if (o->cur.file)
     {
	evas_object_image_unload(obj, 0);
	evas_object_image_load(obj);
	o->changed = 1;
	evas_object_change(obj);
     }
}

/**
 * Get the size of a loaded image of the canvas.
 *
 * @param obj The given canvas object.
 * @param w The width of the canvas image given.
 * @param h The height of the canvas image given.
 *
 * This function get the size of the given canvas image.
 *
 */
EAPI void
evas_object_image_load_size_get(const Evas_Object *obj, int *w, int *h)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   if (w) *w = o->load_opts.w;
   if (h) *h = o->load_opts.h;
}

/**
 * Set the scale down of a loaded image of the canvas.
 *
 * @param obj The given canvas pointer.
 * @param scale_down The scale to down value.
 *
 * This function sets the scale down of a given canvas image.
 *
 */
EAPI void
evas_object_image_load_scale_down_set(Evas_Object *obj, int scale_down)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   if (o->load_opts.scale_down_by == scale_down) return;
   o->load_opts.scale_down_by = scale_down;
   if (o->cur.file)
     {
	evas_object_image_unload(obj, 0);
	evas_object_image_load(obj);
	o->changed = 1;
	evas_object_change(obj);
     }
}

/**
 * Get the scale down value of given image of the canvas.
 *
 * @param obj The given image object pointer.
 *
 * This function returns the scale down value of a given canvas image.
 *
 */
EAPI int
evas_object_image_load_scale_down_get(const Evas_Object *obj)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return 0;
   MAGIC_CHECK_END();
   return o->load_opts.scale_down_by;
}

EAPI void
evas_object_image_load_region_set(Evas_Object *obj, int x, int y, int w, int h)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   if ((o->load_opts.region.x == x) && (o->load_opts.region.y == y) &&
       (o->load_opts.region.w == w) && (o->load_opts.region.h == h)) return;
   o->load_opts.region.x = x;
   o->load_opts.region.y = y;
   o->load_opts.region.w = w;
   o->load_opts.region.h = h;
   if (o->cur.file)
     {
	evas_object_image_unload(obj, 0);
	evas_object_image_load(obj);
	o->changed = 1;
	evas_object_change(obj);
     }
}

EAPI void
evas_object_image_load_region_get(const Evas_Object *obj, int *x, int *y, int *w, int *h)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   if (x) *x = o->load_opts.region.x;
   if (y) *y = o->load_opts.region.y;
   if (w) *w = o->load_opts.region.w;
   if (h) *h = o->load_opts.region.h;
}

/**
 * Set the colorspace of a given image of the canvas.
 *
 * @param obj The given image object pointer.
 * @param cspace The new color space.
 *
 * This function sets the colorspace of given canvas image.
 *
 */
EAPI void
evas_object_image_colorspace_set(Evas_Object *obj, Evas_Colorspace cspace)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();

#ifdef EVAS_FRAME_QUEUING
   if ((Evas_Colorspace)o->cur.cspace != cspace)
     {
        if (o->engine_data)
          evas_common_pipe_op_image_flush(o->engine_data);
     }
#endif

   o->cur.cspace = cspace;
   if (o->engine_data)
     obj->layer->evas->engine.func->image_colorspace_set(obj->layer->evas->engine.data.output,
							 o->engine_data,
							 cspace);
}

/**
 * Get the colorspace of a given image of the canvas.
 *
 * @param obj The given image object pointer.
 * @return The colorspace of the image.
 *
 * This function returns the colorspace of given canvas image.
 *
 */
EAPI Evas_Colorspace
evas_object_image_colorspace_get(const Evas_Object *obj)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return EVAS_COLORSPACE_ARGB8888;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return EVAS_COLORSPACE_ARGB8888;
   MAGIC_CHECK_END();
   return o->cur.cspace;
}

/**
 * Set the native surface of a given image of the canvas
 *
 * @param obj The given canvas pointer.
 * @param surf The new native surface.
 *
 * This function sets a native surface of a given canvas image.
 *
 */
EAPI void
evas_object_image_native_surface_set(Evas_Object *obj, Evas_Native_Surface *surf)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   if (!obj->layer->evas->engine.func->image_native_set) return;
   if ((surf) &&
       ((surf->version < 2) ||
        (surf->version > EVAS_NATIVE_SURFACE_VERSION))) return;
   o->engine_data = 
      obj->layer->evas->engine.func->image_native_set(obj->layer->evas->engine.data.output,
                                                      o->engine_data,
                                                      surf);
}

/**
 * Get the native surface of a given image of the canvas
 *
 * @param obj The given canvas pointer.
 * @return The native surface of the given canvas image.
 *
 * This function returns the native surface of a given canvas image.
 *
 */
EAPI Evas_Native_Surface *
evas_object_image_native_surface_get(const Evas_Object *obj)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return NULL;
   MAGIC_CHECK_END();
   if (!obj->layer->evas->engine.func->image_native_get) return NULL;
   return obj->layer->evas->engine.func->image_native_get(obj->layer->evas->engine.data.output,
							  o->engine_data);
}

/**
 * Set the scale hint of a given image of the canvas.
 *
 * @param obj The given canvas pointer.
 * @param hint The scale hint value.
 *
 * This function sets the scale hint value of the given image of the canvas.
 *
 */
EAPI void
evas_object_image_scale_hint_set(Evas_Object *obj, Evas_Image_Scale_Hint hint)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   if (o->scale_hint == hint) return;
#ifdef EVAS_FRAME_QUEUING
   if (o->engine_data)
      evas_common_pipe_op_image_flush(o->engine_data);
#endif
   o->scale_hint = hint;
   if (o->engine_data)
     {
        int stride = 0;
        
        if (obj->layer->evas->engine.func->image_scale_hint_set)
           obj->layer->evas->engine.func->image_scale_hint_set
           (obj->layer->evas->engine.data.output,
               o->engine_data, o->scale_hint);
        if (obj->layer->evas->engine.func->image_stride_get)
           obj->layer->evas->engine.func->image_stride_get
           (obj->layer->evas->engine.data.output,
               o->engine_data, &stride);
        else
           stride = o->cur.image.w * 4;
        o->cur.image.stride = stride;
     }
}

/**
 * Get the scale hint of a given image of the canvas.
 *
 * @param obj The given canvas pointer.
 *
 * This function returns the scale hint value of the given image of the canvas.
 *
 */
EAPI Evas_Image_Scale_Hint
evas_object_image_scale_hint_get(const Evas_Object *obj)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return EVAS_IMAGE_SCALE_HINT_NONE;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return EVAS_IMAGE_SCALE_HINT_NONE;
   MAGIC_CHECK_END();
   return o->scale_hint;
}

/**
 * Set the content hint of a given image of the canvas.
 *
 * @param obj The given canvas pointer.
 * @param hint The content hint value.
 *
 * This function sets the content hint value of the given image of the canvas.
 *
 */
EAPI void
evas_object_image_content_hint_set(Evas_Object *obj, Evas_Image_Content_Hint hint)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   if (o->content_hint == hint) return;
#ifdef EVAS_FRAME_QUEUING
   if (o->engine_data)
      evas_common_pipe_op_image_flush(o->engine_data);
#endif
   o->content_hint = hint;
   if (o->engine_data)
     {
        int stride = 0;
        
        if (obj->layer->evas->engine.func->image_content_hint_set)
           obj->layer->evas->engine.func->image_content_hint_set
           (obj->layer->evas->engine.data.output,
               o->engine_data, o->content_hint);
        if (obj->layer->evas->engine.func->image_stride_get)
           obj->layer->evas->engine.func->image_stride_get
           (obj->layer->evas->engine.data.output,
               o->engine_data, &stride);
        else
           stride = o->cur.image.w * 4;
        o->cur.image.stride = stride;
     }
}

/**
 * Get the content hint of a given image of the canvas.
 *
 * @param obj The given canvas pointer.
 *
 * This function returns the content hint value of the given image of the canvas.
 *
 */
EAPI Evas_Image_Content_Hint
evas_object_image_content_hint_get(const Evas_Object *obj)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return EVAS_IMAGE_CONTENT_HINT_NONE;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return EVAS_IMAGE_CONTENT_HINT_NONE;
   MAGIC_CHECK_END();
   return o->content_hint;
}

/**
 * @}
 */

/**
 * @addtogroup Evas_Image_Group
 * @{
 */

/**
 * Flush the image cache of the canvas.
 *
 * @param e The given evas pointer.
 *
 * This function flushes image cache of canvas.
 *
 */
EAPI void
evas_image_cache_flush(Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   e->engine.func->image_cache_flush(e->engine.data.output);
}

/**
 * Reload the image cache
 *
 * @param e The given evas pointer.
 *
 * This function reloads the image cache of canvas.
 *
 */
EAPI void
evas_image_cache_reload(Evas *e)
{
   Evas_Layer *layer;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   evas_image_cache_flush(e);
   EINA_INLIST_FOREACH(e->layers, layer)
     {
	Evas_Object *obj;

	EINA_INLIST_FOREACH(layer->objects, obj)
	  {
	     Evas_Object_Image *o;

	     o = (Evas_Object_Image *)(obj->object_data);
	     if (o->magic == MAGIC_OBJ_IMAGE)
	       {
		  evas_object_image_unload(obj, 1);
	       }
	  }
     }
   evas_image_cache_flush(e);
   EINA_INLIST_FOREACH(e->layers, layer)
     {
	Evas_Object *obj;

	EINA_INLIST_FOREACH(layer->objects, obj)
	  {
	     Evas_Object_Image *o;

	     o = (Evas_Object_Image *)(obj->object_data);
	     if (o->magic == MAGIC_OBJ_IMAGE)
	       {
		  evas_object_image_load(obj);
		  o->changed = 1;
		  evas_object_change(obj);
	       }
	  }
     }
   evas_image_cache_flush(e);
}

/**
 * Set the image cache.
 *
 * @param e The given evas pointer.
 * @param size The cache size.
 *
 * This function sets the image cache of canvas.
 *
 */
EAPI void
evas_image_cache_set(Evas *e, int size)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   if (size < 0) size = 0;
   e->engine.func->image_cache_set(e->engine.data.output, size);
}

/**
 * Set the image cache
 *
 * @param e The given evas pointer.
 *
 * This function returns the image cache of canvas.
 *
 */
EAPI int
evas_image_cache_get(const Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return 0;
   MAGIC_CHECK_END();

   return e->engine.func->image_cache_get(e->engine.data.output);
}

/**
 * @}
 */

/* all nice and private */

static void
evas_object_image_unload(Evas_Object *obj, Eina_Bool dirty)
{
   Evas_Object_Image *o;

   o = (Evas_Object_Image *)(obj->object_data);

   if ((!o->cur.file) ||
       (o->pixels_checked_out > 0)) return;
   if (dirty)
     {
        if (o->engine_data)
          o->engine_data = obj->layer->evas->engine.func->image_dirty_region
           (obj->layer->evas->engine.data.output,
               o->engine_data,
               0, 0,
               o->cur.image.w, o->cur.image.h);
     }
   if (o->engine_data)
     obj->layer->evas->engine.func->image_free(obj->layer->evas->engine.data.output,
					       o->engine_data);
   o->engine_data = NULL;
   o->load_error = EVAS_LOAD_ERROR_NONE;
   o->cur.has_alpha = 1;
   o->cur.cspace = EVAS_COLORSPACE_ARGB8888;
   o->cur.image.w = 0;
   o->cur.image.h = 0;
   o->cur.image.stride = 0;
}

static void
evas_object_image_load(Evas_Object *obj)
{
   Evas_Object_Image *o;
   Evas_Image_Load_Opts lo;

   o = (Evas_Object_Image *)(obj->object_data);
   if (o->engine_data) return;

   lo.scale_down_by = o->load_opts.scale_down_by;
   lo.dpi = o->load_opts.dpi;
   lo.w = o->load_opts.w;
   lo.h = o->load_opts.h;
   lo.region.x = o->load_opts.region.x;
   lo.region.y = o->load_opts.region.y;
   lo.region.w = o->load_opts.region.w;
   lo.region.h = o->load_opts.region.h;
   o->engine_data = obj->layer->evas->engine.func->image_load
      (obj->layer->evas->engine.data.output,
          o->cur.file,
          o->cur.key,
          &o->load_error,
          &lo);
   if (o->engine_data)
     {
	int w, h;
	int stride = 0;

	obj->layer->evas->engine.func->image_size_get
           (obj->layer->evas->engine.data.output,
               o->engine_data, &w, &h);
	if (obj->layer->evas->engine.func->image_stride_get)
	  obj->layer->evas->engine.func->image_stride_get
           (obj->layer->evas->engine.data.output,
               o->engine_data, &stride);
	else
	  stride = w * 4;
	o->cur.has_alpha = obj->layer->evas->engine.func->image_alpha_get
           (obj->layer->evas->engine.data.output,
               o->engine_data);
	o->cur.cspace = obj->layer->evas->engine.func->image_colorspace_get
           (obj->layer->evas->engine.data.output,
               o->engine_data);
	o->cur.image.w = w;
	o->cur.image.h = h;
	o->cur.image.stride = stride;
     }
   else
     {
	o->load_error = EVAS_LOAD_ERROR_GENERIC;
     }
}

static Evas_Coord
evas_object_image_figure_x_fill(Evas_Object *obj, Evas_Coord start, Evas_Coord size, Evas_Coord *size_ret)
{
   Evas_Coord w;

   w = ((size * obj->layer->evas->output.w) /
	(Evas_Coord)obj->layer->evas->viewport.w);
   if (size <= 0) size = 1;
   if (start > 0)
     {
	while (start - size > 0) start -= size;
     }
   else if (start < 0)
     {
	while (start < 0) start += size;
     }
   start = ((start * obj->layer->evas->output.w) /
	    (Evas_Coord)obj->layer->evas->viewport.w);
   *size_ret = w;
   return start;
}

static Evas_Coord
evas_object_image_figure_y_fill(Evas_Object *obj, Evas_Coord start, Evas_Coord size, Evas_Coord *size_ret)
{
   Evas_Coord h;

   h = ((size * obj->layer->evas->output.h) /
	(Evas_Coord)obj->layer->evas->viewport.h);
   if (size <= 0) size = 1;
   if (start > 0)
     {
	while (start - size > 0) start -= size;
     }
   else if (start < 0)
     {
	while (start < 0) start += size;
     }
   start = ((start * obj->layer->evas->output.h) /
	    (Evas_Coord)obj->layer->evas->viewport.h);
   *size_ret = h;
   return start;
}

static void
evas_object_image_init(Evas_Object *obj)
{
   /* alloc image ob, setup methods and default values */
   obj->object_data = evas_object_image_new();
   /* set up default settings for this kind of object */
   obj->cur.color.r = 255;
   obj->cur.color.g = 255;
   obj->cur.color.b = 255;
   obj->cur.color.a = 255;
   obj->cur.geometry.x = 0;
   obj->cur.geometry.y = 0;
   obj->cur.geometry.w = 0;
   obj->cur.geometry.h = 0;
   obj->cur.layer = 0;
   obj->cur.anti_alias = 0;
   obj->cur.render_op = EVAS_RENDER_BLEND;
   /* set up object-specific settings */
   obj->prev = obj->cur;
   /* set up methods (compulsory) */
   obj->func = &object_func;
   obj->type = o_type;
}

static void *
evas_object_image_new(void)
{
   Evas_Object_Image *o;

   /* alloc obj private data */
   EVAS_MEMPOOL_INIT(_mp_obj, "evas_object_image", Evas_Object_Image, 256, NULL);
   o = EVAS_MEMPOOL_ALLOC(_mp_obj, Evas_Object_Image);
   if (!o) return NULL;
   EVAS_MEMPOOL_PREP(_mp_obj, o, Evas_Object_Image);
   o->magic = MAGIC_OBJ_IMAGE;
   o->cur.fill.w = 0;
   o->cur.fill.h = 0;
   o->cur.smooth_scale = 1;
   o->cur.border.fill = 1;
   o->cur.border.scale = 1.0;
   o->cur.cspace = EVAS_COLORSPACE_ARGB8888;
   o->cur.spread = EVAS_TEXTURE_REPEAT;
   o->cur.opaque_valid = 0;
   o->prev = o->cur;
   return o;
}

static void
evas_object_image_free(Evas_Object *obj)
{
   Evas_Object_Image *o;
   Eina_Rectangle *r;

   /* frees private object data. very simple here */
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   /* free obj */
   if (o->cur.file) eina_stringshare_del(o->cur.file);
   if (o->cur.key) eina_stringshare_del(o->cur.key);
   if (o->engine_data)
     {
        obj->layer->evas->engine.func->image_data_preload_cancel(obj->layer->evas->engine.data.output,
                                                                 o->engine_data,
                                                                 obj);
        obj->layer->evas->engine.func->image_free(obj->layer->evas->engine.data.output,
                                                  o->engine_data);
     }
   o->engine_data = NULL;
   o->magic = 0;
   EINA_LIST_FREE(o->pixel_updates, r)
     eina_rectangle_free(r);
   EVAS_MEMPOOL_FREE(_mp_obj, o);
}

static void
evas_object_image_render(Evas_Object *obj, void *output, void *context, void *surface, int x, int y)
{
   Evas_Object_Image *o;

   /* render object to surface with context, and offset by x,y */
   o = (Evas_Object_Image *)(obj->object_data);

   if ((o->cur.fill.w < 1) || (o->cur.fill.h < 1))
     return; /* no error message, already printed in pre_render */

   obj->layer->evas->engine.func->context_color_set(output,
						    context,
						    255, 255, 255, 255);

   if ((obj->cur.cache.clip.r == 255) &&
       (obj->cur.cache.clip.g == 255) &&
       (obj->cur.cache.clip.b == 255) &&
       (obj->cur.cache.clip.a == 255))
     {
	obj->layer->evas->engine.func->context_multiplier_unset(output,
								context);
     }
   else
     obj->layer->evas->engine.func->context_multiplier_set(output,
							   context,
							   obj->cur.cache.clip.r,
							   obj->cur.cache.clip.g,
							   obj->cur.cache.clip.b,
							   obj->cur.cache.clip.a);

   obj->layer->evas->engine.func->context_render_op_set(output, context,
							obj->cur.render_op);
   if (o->engine_data)
     {
	Evas_Coord idw, idh, idx, idy;
	int ix, iy, iw, ih;

	if (o->dirty_pixels)
	  {
	     if (o->func.get_pixels)
	       {
		  o->func.get_pixels(o->func.get_pixels_data, obj);
		  o->engine_data = obj->layer->evas->engine.func->image_dirty_region(obj->layer->evas->engine.data.output, o->engine_data, 0, 0, o->cur.image.w, o->cur.image.h);
	       }
	     o->dirty_pixels = 0;
	  }
        if ((obj->cur.map) && (obj->cur.map->count > 3) && (obj->cur.usemap))
          {
	     const Evas_Map_Point *p, *p_end;
             RGBA_Map_Point pts[obj->cur.map->count], *pt;

	     p = obj->cur.map->points;
	     p_end = p + obj->cur.map->count;
	     pt = pts;
             
             pts[0].px = obj->cur.map->persp.px << FP;
             pts[0].py = obj->cur.map->persp.py << FP;
             pts[0].foc = obj->cur.map->persp.foc << FP;
             pts[0].z0 = obj->cur.map->persp.z0 << FP;
             // draw geom +x +y
             for (; p < p_end; p++, pt++)
               {
                  pt->x = (p->x + x) << FP;
                  pt->y = (p->y + y) << FP;
                  pt->z = (p->z)     << FP;
                  pt->x3 = p->px << FP;
                  pt->y3 = p->py << FP;
                  pt->u = p->u * FP1;
                  pt->v = p->v * FP1;
                  pt->col = ARGB_JOIN(p->a, p->r, p->g, p->b);
              }
	     if (obj->cur.map->count & 0x1)
	       {
		  pts[obj->cur.map->count] = pts[obj->cur.map->count -1];
	       }

             obj->layer->evas->engine.func->image_map_draw
               (output, context, surface, o->engine_data, obj->cur.map->count,
		pts, o->cur.smooth_scale | obj->cur.map->smooth, 0);
          }
        else
          {
             obj->layer->evas->engine.func->image_scale_hint_set(output,
                                                                 o->engine_data,
                                                                 o->scale_hint);
             o->engine_data = obj->layer->evas->engine.func->image_border_set(output, o->engine_data,
                                                                              o->cur.border.l, o->cur.border.r,
                                                                              o->cur.border.t, o->cur.border.b);
             idx = evas_object_image_figure_x_fill(obj, o->cur.fill.x, o->cur.fill.w, &idw);
             idy = evas_object_image_figure_y_fill(obj, o->cur.fill.y, o->cur.fill.h, &idh);
             if (idw < 1) idw = 1;
             if (idh < 1) idh = 1;
             if (idx > 0) idx -= idw;
             if (idy > 0) idy -= idh;
             while ((int)idx < obj->cur.geometry.w)
               {
                  Evas_Coord ydy;
                  int dobreak_w = 0;
                  
                  ydy = idy;
                  ix = idx;
                  if ((o->cur.fill.w == obj->cur.geometry.w) &&
                      (o->cur.fill.x == 0))
                    {
                       dobreak_w = 1;
                       iw = obj->cur.geometry.w;
                    }
                  else
                    iw = ((int)(idx + idw)) - ix;
                  while ((int)idy < obj->cur.geometry.h)
                    {
                       int dobreak_h = 0;
                       
                       iy = idy;
                       if ((o->cur.fill.h == obj->cur.geometry.h) &&
                           (o->cur.fill.y == 0))
                         {
                            ih = obj->cur.geometry.h;
                            dobreak_h = 1;
                         }
                       else
                         ih = ((int)(idy + idh)) - iy;
                       if ((o->cur.border.l == 0) &&
                           (o->cur.border.r == 0) &&
                           (o->cur.border.t == 0) &&
                           (o->cur.border.b == 0) &&
                           (o->cur.border.fill != 0))
                         obj->layer->evas->engine.func->image_draw(output,
                                                                   context,
                                                                   surface,
                                                                   o->engine_data,
                                                                   0, 0,
                                                                   o->cur.image.w,
                                                                   o->cur.image.h,
                                                                   obj->cur.geometry.x + ix + x,
                                                                   obj->cur.geometry.y + iy + y,
                                                                   iw, ih,
                                                                   o->cur.smooth_scale);
                       else
                         {
                            int inx, iny, inw, inh, outx, outy, outw, outh;
                            int bl, br, bt, bb, bsl, bsr, bst, bsb;
                            int imw, imh, ox, oy;
                            
                            ox = obj->cur.geometry.x + ix + x;
                            oy = obj->cur.geometry.y + iy + y;
                            imw = o->cur.image.w;
                            imh = o->cur.image.h;
                            bl = o->cur.border.l;
                            br = o->cur.border.r;
                            bt = o->cur.border.t;
                            bb = o->cur.border.b;
                            if ((bl + br) > iw)
                              {
                                 bl = iw / 2;
                                 br = iw - bl;
                              }
                            if ((bl + br) > imw)
                              {
                                 bl = imw / 2;
                                 br = imw - bl;
                              }
                            if ((bt + bb) > ih)
                              {
                                 bt = ih / 2;
                                 bb = ih - bt;
                              }
                            if ((bt + bb) > imh)
                              {
                                 bt = imh / 2;
                                 bb = imh - bt;
                              }
                            if (o->cur.border.scale != 1.0)
                              {
                                 bsl = ((double)bl * o->cur.border.scale);
                                 bsr = ((double)br * o->cur.border.scale);
                                 bst = ((double)bt * o->cur.border.scale);
                                 bsb = ((double)bb * o->cur.border.scale);
                              }
                            else
                              {
                                  bsl = bl; bsr = br; bst = bt; bsb = bb;
                              }
                            // #--
                            // |
                            inx = 0; iny = 0;
                            inw = bl; inh = bt;
                            outx = ox; outy = oy;
                            outw = bsl; outh = bst;
                            obj->layer->evas->engine.func->image_draw(output, context, surface, o->engine_data, inx, iny, inw, inh, outx, outy, outw, outh, o->cur.smooth_scale);
                            // .##
                            // |
                            inx = bl; iny = 0;
                            inw = imw - bl - br; inh = bt;
                            outx = ox + bsl; outy = oy;
                            outw = iw - bsl - bsr; outh = bst;
                            obj->layer->evas->engine.func->image_draw(output, context, surface, o->engine_data, inx, iny, inw, inh, outx, outy, outw, outh, o->cur.smooth_scale);
                            // --#
                            //   |
                            inx = imw - br; iny = 0;
                            inw = br; inh = bt;
                            outx = ox + iw - bsr; outy = oy;
                            outw = bsr; outh = bst;
                            obj->layer->evas->engine.func->image_draw(output, context, surface, o->engine_data, inx, iny, inw, inh, outx, outy, outw, outh, o->cur.smooth_scale);
                            // .--
                            // #  
                            inx = 0; iny = bt;
                            inw = bl; inh = imh - bt - bb;
                            outx = ox; outy = oy + bst;
                            outw = bsl; outh = ih - bst - bsb;
                            obj->layer->evas->engine.func->image_draw(output, context, surface, o->engine_data, inx, iny, inw, inh, outx, outy, outw, outh, o->cur.smooth_scale);
                            // .--.
                            // |##|
                            if (o->cur.border.fill > EVAS_BORDER_FILL_NONE)
                              {
                                 inx = bl; iny = bt;
                                 inw = imw - bl - br; inh = imh - bt - bb;
                                 outx = ox + bsl; outy = oy + bst;
                                 outw = iw - bsl - bsr; outh = ih - bst - bsb;
                                 if ((o->cur.border.fill == EVAS_BORDER_FILL_SOLID) &&
                                     (obj->cur.cache.clip.a == 255) &&
                                     (obj->cur.render_op == EVAS_RENDER_BLEND))
                                   {
                                      obj->layer->evas->engine.func->context_render_op_set(output, context,
                                                                                           EVAS_RENDER_COPY);
                                      obj->layer->evas->engine.func->image_draw(output, context, surface, o->engine_data, inx, iny, inw, inh, outx, outy, outw, outh, o->cur.smooth_scale);
                                      obj->layer->evas->engine.func->context_render_op_set(output, context,
                                                                                           obj->cur.render_op);
                                   }
                                 else
                                   obj->layer->evas->engine.func->image_draw(output, context, surface, o->engine_data, inx, iny, inw, inh, outx, outy, outw, outh, o->cur.smooth_scale);
                              }
                            // --.
                            //   #
                            inx = imw - br; iny = bt;
                            inw = br; inh = imh - bt - bb;
                            outx = ox + iw - bsr; outy = oy + bst;
                            outw = bsr; outh = ih - bst - bsb;
                            obj->layer->evas->engine.func->image_draw(output, context, surface, o->engine_data, inx, iny, inw, inh, outx, outy, outw, outh, o->cur.smooth_scale);
                            // |
                            // #--
                            inx = 0; iny = imh - bb;
                            inw = bl; inh = bb;
                            outx = ox; outy = oy + ih - bsb;
                            outw = bsl; outh = bsb;
                            obj->layer->evas->engine.func->image_draw(output, context, surface, o->engine_data, inx, iny, inw, inh, outx, outy, outw, outh, o->cur.smooth_scale);
                            // |
                            // .## 
                            inx = bl; iny = imh - bb;
                            inw = imw - bl - br; inh = bb;
                            outx = ox + bsl; outy = oy + ih - bsb;
                            outw = iw - bsl - bsr; outh = bsb;
                            obj->layer->evas->engine.func->image_draw(output, context, surface, o->engine_data, inx, iny, inw, inh, outx, outy, outw, outh, o->cur.smooth_scale);
                            //   |
                            // --#
                            inx = imw - br; iny = imh - bb;
                            inw = br; inh = bb;
                            outx = ox + iw - bsr; outy = oy + ih - bsb;
                            outw = bsr; outh = bsb;
                            obj->layer->evas->engine.func->image_draw(output, context, surface, o->engine_data, inx, iny, inw, inh, outx, outy, outw, outh, o->cur.smooth_scale);
                         }
                       idy += idh;
                       if (dobreak_h) break;
                    }
                  idx += idw;
                  idy = ydy;
                  if (dobreak_w) break;
               }
	  }
     }
}

static void
evas_object_image_render_pre(Evas_Object *obj)
{
   Evas_Object_Image *o;
   int is_v, was_v;

   /* dont pre-render the obj twice! */
   if (obj->pre_render_done) return;
   obj->pre_render_done = 1;
   /* pre-render phase. this does anything an object needs to do just before */
   /* rendering. this could mean loading the image data, retrieving it from */
   /* elsewhere, decoding video etc. */
   /* then when this is done the object needs to figure if it changed and */
   /* if so what and where and add the appropriate redraw rectangles */
   o = (Evas_Object_Image *)(obj->object_data);

   if ((o->cur.fill.w < 1) || (o->cur.fill.h < 1))
     {
       ERR("%p has invalid fill size: %dx%d. Ignored",
	     obj, o->cur.fill.w, o->cur.fill.h);
	return;
     }

   /* if someone is clipping this obj - go calculate the clipper */
   if (obj->cur.clipper)
     {
	if (obj->cur.cache.clip.dirty)
	  evas_object_clip_recalc(obj->cur.clipper);
	obj->cur.clipper->func->render_pre(obj->cur.clipper);
     }
   /* now figure what changed and add draw rects */
   /* if it just became visible or invisible */
   is_v = evas_object_is_visible(obj);
   was_v = evas_object_was_visible(obj);
   if (is_v != was_v)
     {
	evas_object_render_pre_visible_change(&obj->layer->evas->clip_changes, obj, is_v, was_v);
	if (!o->pixel_updates) goto done;
     }
   if ((obj->cur.map != obj->prev.map) ||
       (obj->cur.usemap != obj->prev.usemap))
     {
	evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes, obj);
        goto done;
     }
   /* it's not visible - we accounted for it appearing or not so just abort */
   if (!is_v) goto done;
   /* clipper changed this is in addition to anything else for obj */
   evas_object_render_pre_clipper_change(&obj->layer->evas->clip_changes, obj);
   /* if we restacked (layer or just within a layer) and don't clip anyone */
   if (obj->restack)
     {
	evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes, obj);
	if (!o->pixel_updates) goto done;
     }
   /* if it changed color */
   if ((obj->cur.color.r != obj->prev.color.r) ||
       (obj->cur.color.g != obj->prev.color.g) ||
       (obj->cur.color.b != obj->prev.color.b) ||
       (obj->cur.color.a != obj->prev.color.a))
     {
	evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes, obj);
	if (!o->pixel_updates) goto done;
     }
   /* if it changed render op */
   if (obj->cur.render_op != obj->prev.render_op)
     {
	evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes, obj);
	if (!o->pixel_updates) goto done;
     }
   /* if it changed anti_alias */
   if (obj->cur.anti_alias != obj->prev.anti_alias)
     {
	evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes, obj);
	if (!o->pixel_updates) goto done;
     }
   if (o->changed)
     {
	if (((o->cur.file) && (!o->prev.file)) ||
	    ((!o->cur.file) && (o->prev.file)) ||
	    ((o->cur.key) && (!o->prev.key)) ||
	    ((!o->cur.key) && (o->prev.key))
	    )
	  {
	     evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes, obj);
	     if (!o->pixel_updates) goto done;
	  }
	if ((o->cur.image.w != o->prev.image.w) ||
	    (o->cur.image.h != o->prev.image.h) ||
	    (o->cur.has_alpha != o->prev.has_alpha) ||
	    (o->cur.cspace != o->prev.cspace) ||
	    (o->cur.smooth_scale != o->prev.smooth_scale))
	  {
	     evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes, obj);
	     if (!o->pixel_updates) goto done;
	  }
	if ((o->cur.border.l != o->prev.border.l) ||
	    (o->cur.border.r != o->prev.border.r) ||
	    (o->cur.border.t != o->prev.border.t) ||
	    (o->cur.border.b != o->prev.border.b) ||
            (o->cur.border.fill != o->prev.border.fill) ||
            (o->cur.border.scale != o->prev.border.scale))
	  {
	     evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes, obj);
	     if (!o->pixel_updates) goto done;
	  }
	if (o->dirty_pixels)
	  {
	     evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes, obj);
	     if (!o->pixel_updates) goto done;
	  }
     }
   /* if it changed geometry - and obviously not visibility or color */
   /* calculate differences since we have a constant color fill */
   /* we really only need to update the differences */
#if 0 // XXX: maybe buggy?
   if (((obj->cur.geometry.x != obj->prev.geometry.x) ||
	(obj->cur.geometry.y != obj->prev.geometry.y) ||
	(obj->cur.geometry.w != obj->prev.geometry.w) ||
	(obj->cur.geometry.h != obj->prev.geometry.h)) &&
       (o->cur.fill.w == o->prev.fill.w) &&
       (o->cur.fill.h == o->prev.fill.h) &&
       ((o->cur.fill.x + obj->cur.geometry.x) == (o->prev.fill.x + obj->prev.geometry.x)) &&
       ((o->cur.fill.y + obj->cur.geometry.y) == (o->prev.fill.y + obj->prev.geometry.y)) &&
       (!o->pixel_updates)
       )
     {
	evas_rects_return_difference_rects(&obj->layer->evas->clip_changes,
					   obj->cur.geometry.x,
					   obj->cur.geometry.y,
					   obj->cur.geometry.w,
					   obj->cur.geometry.h,
					   obj->prev.geometry.x,
					   obj->prev.geometry.y,
					   obj->prev.geometry.w,
					   obj->prev.geometry.h);
	if (!o->pixel_updates) goto done;
     }
#endif   
   if (((obj->cur.geometry.x != obj->prev.geometry.x) ||
	(obj->cur.geometry.y != obj->prev.geometry.y) ||
	(obj->cur.geometry.w != obj->prev.geometry.w) ||
	(obj->cur.geometry.h != obj->prev.geometry.h))
       )
     {
	evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes, obj);
	if (!o->pixel_updates) goto done;
     }
   if (o->changed)
     {
	if ((o->cur.fill.x != o->prev.fill.x) ||
	    (o->cur.fill.y != o->prev.fill.y) ||
	    (o->cur.fill.w != o->prev.fill.w) ||
	    (o->cur.fill.h != o->prev.fill.h))
	  {
	     evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes, obj);
	     if (!o->pixel_updates) goto done;
	  }
	if ((o->cur.border.l == 0) &&
	    (o->cur.border.r == 0) &&
	    (o->cur.border.t == 0) &&
	    (o->cur.border.b == 0) &&
            (o->cur.image.w > 0) &&
            (o->cur.image.h > 0))
	  {
	     Eina_Rectangle *rr;

	     EINA_LIST_FREE(o->pixel_updates, rr)
	       {
		  Evas_Coord idw, idh, idx, idy;
		  int x, y, w, h;

		  obj->layer->evas->engine.func->image_dirty_region(obj->layer->evas->engine.data.output, o->engine_data, rr->x, rr->y, rr->w, rr->h);

		  idx = evas_object_image_figure_x_fill(obj, o->cur.fill.x, o->cur.fill.w, &idw);
		  idy = evas_object_image_figure_y_fill(obj, o->cur.fill.y, o->cur.fill.h, &idh);

		  if (idw < 1) idw = 1;
		  if (idh < 1) idh = 1;
		  if (idx > 0) idx -= idw;
		  if (idy > 0) idy -= idh;
		  while (idx < obj->cur.geometry.w)
		    {
		       Evas_Coord ydy;

		       ydy = idy;
		       x = idx;
		       w = ((int)(idx + idw)) - x;
		       while (idy < obj->cur.geometry.h)
			 {
			    Eina_Rectangle r;

			    y = idy;
			    h = ((int)(idy + idh)) - y;

			    r.x = ((rr->x - 1) * w) / o->cur.image.w;
			    r.y = ((rr->y - 1) * h) / o->cur.image.h;
			    r.w = ((rr->w + 2) * w) / o->cur.image.w;
			    r.h = ((rr->h + 2) * h) / o->cur.image.h;
			    r.x += obj->cur.geometry.x + x;
			    r.y += obj->cur.geometry.y + y;
			    evas_add_rect(&obj->layer->evas->clip_changes, r.x, r.y, r.w, r.h);
			    idy += h;
			 }
		       idx += idw;
		       idy = ydy;
		    }
		  eina_rectangle_free(rr);
	       }
	     goto done;
	  }
	else
	  {
	     if (o->pixel_updates)
	       {
		  Eina_Rectangle *r;

		  EINA_LIST_FREE(o->pixel_updates, r)
		    eina_rectangle_free(r);
		  obj->layer->evas->engine.func->image_dirty_region(obj->layer->evas->engine.data.output, o->engine_data, 0, 0, o->cur.image.w, o->cur.image.h);
		  evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes, obj);
		  goto done;
	       }
	  }
     }
   /* it obviously didn't change - add a NO obscure - this "unupdates"  this */
   /* area so if there were updates for it they get wiped. don't do it if we */
   /* aren't fully opaque and we are visible */
   if (evas_object_is_visible(obj) &&
       evas_object_is_opaque(obj))
     {
         obj->layer->evas->engine.func->output_redraws_rect_del(obj->layer->evas->engine.data.output,
                                                               obj->cur.cache.clip.x,
                                                               obj->cur.cache.clip.y,
                                                               obj->cur.cache.clip.w,
                                                               obj->cur.cache.clip.h);
     }
   done:
   evas_object_render_pre_effect_updates(&obj->layer->evas->clip_changes, obj, is_v, was_v);
}

static void
evas_object_image_render_post(Evas_Object *obj)
{
   Evas_Object_Image *o;
   Eina_Rectangle *r;

   /* this moves the current data to the previous state parts of the object */
   /* in whatever way is safest for the object. also if we don't need object */
   /* data anymore we can free it if the object deems this is a good idea */
   o = (Evas_Object_Image *)(obj->object_data);
   /* remove those pesky changes */
   evas_object_clip_changes_clean(obj);
   EINA_LIST_FREE(o->pixel_updates, r)
     eina_rectangle_free(r);
   /* move cur to prev safely for object data */
   obj->prev = obj->cur;
   o->prev = o->cur;
   o->changed = 0;
   /* FIXME: copy strings across */
}

static unsigned int evas_object_image_id_get(Evas_Object *obj)
{
   Evas_Object_Image *o;

   o = (Evas_Object_Image *)(obj->object_data);
   if (!o) return 0;
   return MAGIC_OBJ_IMAGE;
}

static unsigned int evas_object_image_visual_id_get(Evas_Object *obj)
{
   Evas_Object_Image *o;

   o = (Evas_Object_Image *)(obj->object_data);
   if (!o) return 0;
   return MAGIC_OBJ_IMAGE;
}

static void *evas_object_image_engine_data_get(Evas_Object *obj)
{
   Evas_Object_Image *o;

   o = (Evas_Object_Image *)(obj->object_data);
   if (!o) return NULL;
   return o->engine_data;
}

static int
evas_object_image_is_opaque(Evas_Object *obj)
{
   Evas_Object_Image *o;

   /* this returns 1 if the internal object data implies that the object is */
   /* currently fully opaque over the entire rectangle it occupies */
   o = (Evas_Object_Image *)(obj->object_data);
   if (o->cur.opaque_valid)
     {
        if (!o->cur.opaque) return 0;
     }
   else
     {
        o->cur.opaque = 0;
        o->cur.opaque_valid = 1;
        if ((o->cur.fill.w < 1) || (o->cur.fill.h < 1))
           return 0;
        if (((o->cur.border.l != 0) ||
             (o->cur.border.r != 0) ||
             (o->cur.border.t != 0) ||
             (o->cur.border.b != 0)) &&
            (!o->cur.border.fill)) return 0;
        if (!o->engine_data) return 0;
        o->cur.opaque = 1;
     }
   if ((obj->cur.map) && (obj->cur.usemap)) return 0;
   if (obj->cur.render_op == EVAS_RENDER_COPY) return 1;
   if (o->cur.has_alpha) return 0;
   return 1;
}

static int
evas_object_image_was_opaque(Evas_Object *obj)
{
   Evas_Object_Image *o;

   /* this returns 1 if the internal object data implies that the object was */
   /* previously fully opaque over the entire rectangle it occupies */
   o = (Evas_Object_Image *)(obj->object_data);
   if (o->prev.opaque_valid)
     {
        if (!o->prev.opaque) return 0;
     }
   else
     {
        o->prev.opaque = 0;
        o->prev.opaque_valid = 1;
        if ((o->prev.fill.w < 1) || (o->prev.fill.h < 1))
           return 0;
        if (((o->prev.border.l != 0) ||
             (o->prev.border.r != 0) ||
             (o->prev.border.t != 0) ||
             (o->prev.border.b != 0)) &&
            (!o->prev.border.fill)) return 0;
        if (!o->engine_data) return 0;
        o->prev.opaque = 1;
     }
   if (obj->prev.usemap) return 0;
   if (obj->prev.render_op == EVAS_RENDER_COPY) return 1;
   if (o->prev.has_alpha) return 0;
   if (obj->prev.render_op != EVAS_RENDER_BLEND) return 0;
   return 1;
}

static int
evas_object_image_is_inside(Evas_Object *obj, Evas_Coord x, Evas_Coord y)
{
   Evas_Object_Image *o;
   DATA32 *data;
   int w, h, stride, iw, ih;
   int a;

   o = (Evas_Object_Image *)(obj->object_data);

   x -= obj->cur.cache.clip.x;
   y -= obj->cur.cache.clip.y;
   w = obj->cur.cache.clip.w;
   h = obj->cur.cache.clip.h;
   iw = o->cur.image.w;
   ih = o->cur.image.h;

   if ((x < 0) || (y < 0) || (x >= w) || (y >= h)) return 0;
   if (!o->cur.has_alpha) return 1;

   if (obj->cur.map)
     {
        x = obj->cur.map->mx;
        y = obj->cur.map->my;
     }
   else
     {
        int bl, br, bt, bb, bsl, bsr, bst, bsb;
        
        bl = o->cur.border.l;
        br = o->cur.border.r;
        bt = o->cur.border.t;
        bb = o->cur.border.b;
        if ((bl + br) > iw)
          {
             bl = iw / 2;
             br = iw - bl;
          }
        if ((bl + br) > iw)
          {
             bl = iw / 2;
             br = iw - bl;
          }
        if ((bt + bb) > ih)
          {
             bt = ih / 2;
             bb = ih - bt;
          }
        if ((bt + bb) > ih)
          {
             bt = ih / 2;
             bb = ih - bt;
          }
        if (o->cur.border.scale != 1.0)
          {
             bsl = ((double)bl * o->cur.border.scale);
             bsr = ((double)br * o->cur.border.scale);
             bst = ((double)bt * o->cur.border.scale);
             bsb = ((double)bb * o->cur.border.scale);
          }
        else
          {
             bsl = bl; bsr = br; bst = bt; bsb = bb;
          }
        
        w = o->cur.fill.w;
        h = o->cur.fill.h;
        x -= o->cur.fill.x;
        y -= o->cur.fill.y;
        x %= w;
        y %= h;
        
        if (x < 0) x += w;
        if (y < 0) y += h;
        
        if (o->cur.border.fill != EVAS_BORDER_FILL_DEFAULT)
          {
             if ((x > bsl) && (x < (w - bsr)) &&
                 (y > bst) && (y < (h - bsb)))
               {
                  if (o->cur.border.fill == EVAS_BORDER_FILL_SOLID) return 1;
                  return 0;
               }
          }
        
        if (x < bsl) x = (x * bl) / bsl;
        else if (x > (w - bsr)) x = iw - (((w - x) * br) / bsr);
        else if ((bsl + bsr) < w) x = bl + (((x - bsl) * (iw - bl - br)) / (w - bsl - bsr));
        else return 1;
        
        if (y < bst) y = (y * bt) / bst;
        else if (y > (h - bsb)) y = ih - (((h - y) * bb) / bsb);
        else if ((bst + bsb) < h) y = bt + (((y - bst) * (ih - bt - bb)) / (h - bst - bsb));
        else return 1;
     }
   
   if (x < 0) x = 0;
   if (y < 0) y = 0;
   if (x >= iw) x = iw - 1;
   if (y >= ih) y = ih - 1;
   
   stride = o->cur.image.stride;
   
   o->engine_data = obj->layer->evas->engine.func->image_data_get
      (obj->layer->evas->engine.data.output,
          o->engine_data,
          0,
          &data);
   if (!data)
     return 0;

   switch (o->cur.cspace)
     {
     case EVAS_COLORSPACE_ARGB8888:
        data = ((DATA32*)(data) + ((y * (stride >> 2)) + x));
        a = (*((DATA32*)(data)) >> 24) & 0xff;
        break;
     case EVAS_COLORSPACE_RGB565_A5P:
        data = (void*) ((DATA16*)(data) + (h * (stride >> 1)));
        data = (void*) ((DATA8*)(data) + ((y * (stride >> 1)) + x));
        a = (*((DATA8*)(data))) & 0x1f;
        break;
     default:
        return 1;
        break;
     }

   return (a != 0);
}

static int
evas_object_image_has_opaque_rect(Evas_Object *obj)
{
   Evas_Object_Image *o;

   o = (Evas_Object_Image *)(obj->object_data);
   if ((obj->cur.map) && (obj->cur.usemap)) return 0;
   if (((o->cur.border.l | o->cur.border.r | o->cur.border.t | o->cur.border.b) != 0) &&
       (o->cur.border.fill == EVAS_BORDER_FILL_SOLID) &&
       (obj->cur.render_op == EVAS_RENDER_BLEND) &&
       (obj->cur.cache.clip.a == 255) &&
       (o->cur.fill.x == 0) &&
       (o->cur.fill.y == 0) &&
       (o->cur.fill.w == obj->cur.geometry.w) &&
       (o->cur.fill.h == obj->cur.geometry.h)
       ) return 1;
   return 0;
}

static int
evas_object_image_get_opaque_rect(Evas_Object *obj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h)
{
   Evas_Object_Image *o;

   o = (Evas_Object_Image *)(obj->object_data);
   if (o->cur.border.scale == 1.0)
     {
        *x = obj->cur.geometry.x + o->cur.border.l;
        *y = obj->cur.geometry.y + o->cur.border.t;
        *w = obj->cur.geometry.w - (o->cur.border.l + o->cur.border.r);
        if (*w < 0) *w = 0;
        *h = obj->cur.geometry.h - (o->cur.border.t + o->cur.border.b);
        if (*h < 0) *h = 0;
     }
   else
     {
        *x = obj->cur.geometry.x + (o->cur.border.l * o->cur.border.scale);
        *y = obj->cur.geometry.y + (o->cur.border.t * o->cur.border.scale);
        *w = obj->cur.geometry.w - ((o->cur.border.l * o->cur.border.scale) + (o->cur.border.r * o->cur.border.scale));
        if (*w < 0) *w = 0;
        *h = obj->cur.geometry.h - ((o->cur.border.t * o->cur.border.scale) + (o->cur.border.b * o->cur.border.scale));
        if (*h < 0) *h = 0;
     }
   return 1;
}

static int
evas_object_image_can_map(Evas_Object *obj __UNUSED__)
{
   return 1;
}

static void *
evas_object_image_data_convert_internal(Evas_Object_Image *o, void *data, Evas_Colorspace to_cspace)
{
   void *out = NULL;

   if (!data)
     return NULL;

   switch (o->cur.cspace)
     {
	case EVAS_COLORSPACE_ARGB8888:
	  out = evas_common_convert_argb8888_to(data,
						o->cur.image.w,
						o->cur.image.h,
						o->cur.image.stride >> 2,
						o->cur.has_alpha,
						to_cspace);
	  break;
	case EVAS_COLORSPACE_RGB565_A5P:
	  out = evas_common_convert_rgb565_a5p_to(data,
						  o->cur.image.w,
						  o->cur.image.h,
						  o->cur.image.stride >> 1,
						  o->cur.has_alpha,
						  to_cspace);
	  break;
	default:
	  break;
     }

   return out;
}

static void
evas_object_image_filled_resize_listener(void *data __UNUSED__, Evas *e __UNUSED__, Evas_Object *obj, void *einfo __UNUSED__)
{
   Evas_Coord w, h;

   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   evas_object_image_fill_set(obj, 0, 0, w, h);
}
