#include "evas_common.h" /* Includes evas_bidi_utils stuff. */
#include "evas_private.h"

/* save typing */
#define ENFN obj->layer->evas->engine.func
#define ENDT obj->layer->evas->engine.data.output

/* private magic number for text objects */
static const char o_type[] = "text";

/* private struct for text object internal data */
typedef struct _Evas_Object_Text Evas_Object_Text;

struct _Evas_Object_Text
{
   DATA32               magic;

   struct {
      const Eina_Unicode  *text; /*The shaped text */
      const char          *utf8_text; /* The text exposed to the API */
      const char          *font;
      const char          *source;
      Evas_Font_Size       size;
      struct {
	 unsigned char  r, g, b, a;
      } outline, shadow, glow, glow2;

      unsigned char               style;
      Evas_BiDi_Props             intl_props;
   } cur, prev;

   float                ascent, descent;
   float                max_ascent, max_descent;

   void                *engine_data;

   char                 changed : 1;
};

/* private methods for text objects */
static void evas_object_text_init(Evas_Object *obj);
static void *evas_object_text_new(void);
static void evas_object_text_render(Evas_Object *obj, void *output, void *context, void *surface, int x, int y);
static void evas_object_text_free(Evas_Object *obj);
static void evas_object_text_render_pre(Evas_Object *obj);
static void evas_object_text_render_post(Evas_Object *obj);

static unsigned int evas_object_text_id_get(Evas_Object *obj);
static unsigned int evas_object_text_visual_id_get(Evas_Object *obj);
static void *evas_object_text_engine_data_get(Evas_Object *obj);

static int evas_object_text_is_opaque(Evas_Object *obj);
static int evas_object_text_was_opaque(Evas_Object *obj);

static void evas_object_text_scale_update(Evas_Object *obj);

static const Evas_Object_Func object_func =
{
   /* methods (compulsory) */
   evas_object_text_free,
     evas_object_text_render,
     evas_object_text_render_pre,
     evas_object_text_render_post,
     evas_object_text_id_get,
     evas_object_text_visual_id_get,
     evas_object_text_engine_data_get,
   /* these are optional. NULL = nothing */
     NULL,
     NULL,
     NULL,
     NULL,
     evas_object_text_is_opaque,
     evas_object_text_was_opaque,
     NULL,
     NULL,
     NULL,
     evas_object_text_scale_update,
     NULL,
     NULL,
     NULL
};

/* the actual api call to add a rect */
/* it has no other api calls as all properties are standard */

/**
 * @addtogroup Evas_Object_Text
 * @{
 */

EVAS_MEMPOOL(_mp_obj);

/**
 * Creates a new text @c Evas_Object on the provided @c Evas canvas.
 *
 * @param e The @c Evas canvas to create the text object upon.
 *
 * @see evas_object_text_font_source_set
 * @see evas_object_text_font_set
 * @see evas_object_text_text_set
 *
 * @returns NULL on error, A pointer to a new @c Evas_Object on success.
 */
EAPI Evas_Object *
evas_object_text_add(Evas *e)
{
   Evas_Object *obj;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   obj = evas_object_new(e);
   evas_object_text_init(obj);
   evas_object_inject(obj, e);
   return obj;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 *
 */
EAPI void
evas_object_text_font_source_set(Evas_Object *obj, const char *font_source)
{
   Evas_Object_Text *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Text *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Text, MAGIC_OBJ_TEXT);
   return;
   MAGIC_CHECK_END();

   if ((o->cur.source) && (font_source) &&
       (!strcmp(o->cur.source, font_source)))
     return;
   /*
   if (o->cur.source) eina_stringshare_del(o->cur.source);
   if (font_source) o->cur.source = eina_stringshare_add(font_source);
   else o->cur.source = NULL;
    */
   eina_stringshare_replace(&o->cur.source, font_source);
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 *
 */
EAPI const char *
evas_object_text_font_source_get(const Evas_Object *obj)
{
   Evas_Object_Text *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   o = (Evas_Object_Text *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Text, MAGIC_OBJ_TEXT);
   return NULL;
   MAGIC_CHECK_END();
   return o->cur.source;
}

/**
 * Set the font and the font size for a given text object.
 *
 * If both font is NULL & size is 0, this function does nothing.  Otherwise
 * setting a NULL font or a 0 size leaves that property unchanged.
 *
 * @param obj Text object to adjust.
 * @param font Font to set, or NULL.
 * @param size Size to set of 0.
 */
EAPI void
evas_object_text_font_set(Evas_Object *obj, const char *font, Evas_Font_Size size)
{
   Evas_Object_Text *o;
   int l = 0, r = 0, t = 0, b = 0;
   int is, was = 0, pass = 0;
   int same_font = 0;

   if ((!font) && (size <= 0)) return;
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Text *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Text, MAGIC_OBJ_TEXT);
   return;
   MAGIC_CHECK_END();

   if (!font) font = o->cur.font;
   if (size <= 0) size = o->cur.size;

   if ((o->cur.font) && (font) &&
		   ((font == o->cur.font) || !strcmp(o->cur.font, font)))
     {
	same_font = 1;
	if (size == o->cur.size) return;
     }
   if (obj->layer->evas->events_frozen <= 0)
     {
	pass = evas_event_passes_through(obj);
	if (!pass)
	  was = evas_object_is_in_output_rect(obj,
					      obj->layer->evas->pointer.x,
					      obj->layer->evas->pointer.y, 1, 1);
     }

#ifdef EVAS_FRAME_QUEUING
   if (o->engine_data)
      evas_common_pipe_op_text_flush(o->engine_data);
#endif

   /* DO IT */
   if (o->engine_data)
     {
	evas_font_free(obj->layer->evas, o->engine_data);
	o->engine_data = NULL;
     }
   if (!same_font)
     {
	/*
	if (o->cur.font) eina_stringshare_del(o->cur.font);
	if (font) o->cur.font = eina_stringshare_add(font);
	else o->cur.font = NULL;
	 */
	eina_stringshare_replace(&o->cur.font, font);
	o->prev.font = NULL;
     }
   o->cur.size = size;
   o->engine_data = evas_font_load(obj->layer->evas, o->cur.font, o->cur.source,
				   o->cur.size * obj->cur.scale);
   evas_text_style_pad_get(o->cur.style, &l, &r, &t, &b);
   if ((o->engine_data) && (o->cur.text))
     {
	int w, h;

        o->ascent =  ENFN->font_ascent_get(ENDT, o->engine_data);
	ENFN->font_string_size_get(ENDT,
				   o->engine_data,
				   o->cur.text, &o->cur.intl_props,
				   &w, &h);
	o->ascent = ENFN->font_ascent_get(ENDT, o->engine_data);
	o->descent = ENFN->font_descent_get(ENDT, o->engine_data);
	o->max_ascent = ENFN->font_max_ascent_get(ENDT, o->engine_data);
	o->max_descent = ENFN->font_max_descent_get(ENDT, o->engine_data);
	obj->cur.geometry.w = w + l + r;
	obj->cur.geometry.h = o->max_ascent + o->max_descent + t + b;
////        obj->cur.cache.geometry.validity = 0;
     }
   else
     {
	if (o->engine_data)
	  {
	     o->ascent = ENFN->font_ascent_get(ENDT, o->engine_data);
	     o->descent = ENFN->font_descent_get(ENDT, o->engine_data);
	     o->max_ascent = ENFN->font_max_ascent_get(ENDT, o->engine_data);
	     o->max_descent = ENFN->font_max_descent_get(ENDT, o->engine_data);
	  }
	else
	  {
	     o->ascent = 0;
	     o->descent = 0;
	     o->max_ascent = 0;
	     o->max_descent = 0;
	  }
	obj->cur.geometry.w = 0;
	obj->cur.geometry.h = o->max_ascent + o->max_descent + t + b;
////	obj->cur.cache.geometry.validity = 0;
     }
   o->changed = 1;
   evas_object_change(obj);
   evas_object_clip_dirty(obj);
   evas_object_coords_recalc(obj);
   if (obj->layer->evas->events_frozen <= 0)
     {
	if (!pass)
	  {
	     is = evas_object_is_in_output_rect(obj,
						obj->layer->evas->pointer.x,
						obj->layer->evas->pointer.y, 1, 1);
	     if ((is ^ was) && obj->cur.visible)
	       evas_event_feed_mouse_move(obj->layer->evas,
					  obj->layer->evas->pointer.x,
					  obj->layer->evas->pointer.y,
					  obj->layer->evas->last_timestamp,
					  NULL);
	  }
     }
   evas_object_inform_call_resize(obj);
}

/**
 * Query evas for font information of a text @c Evas_Object.
 *
 * This function allows the font name and size of a text @c Evas_Object as
 * created with evas_object_text_add() to be queried. Be aware that the font
 * name string is still owned by Evas and should NOT have free() called on
 * it by the caller of the function.
 *
 * @param obj	The evas text object to query for font information.
 * @param font	A pointer to the location to store the font name in (may be NULL).
 * @param size	A pointer to the location to store the font size in (may be NULL).
 */
EAPI void
evas_object_text_font_get(const Evas_Object *obj, const char **font, Evas_Font_Size *size)
{
   Evas_Object_Text *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   if (font) *font = "";
   if (size) *size = 0;
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Text *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Text, MAGIC_OBJ_TEXT);
   if (font) *font = "";
   if (size) *size = 0;
   return;
   MAGIC_CHECK_END();
   if (font) *font = o->cur.font;
   if (size) *size = o->cur.size;
}

/**
 * Sets the text to be displayed by the given evas text object.
 * @param obj  Evas text object.
 * @param text Text to display.
 */
EAPI void
evas_object_text_text_set(Evas_Object *obj, const char *_text)
{
   Evas_Object_Text *o;
   int is, was, len;
   Eina_Unicode *text;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Text *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Text, MAGIC_OBJ_TEXT);
   return;
   MAGIC_CHECK_END();

   text = evas_common_encoding_utf8_to_unicode(_text, &len);

   if (!text) text = eina_unicode_strdup(EINA_UNICODE_EMPTY_STRING);
   if ((o->cur.text) && (text) && (!eina_unicode_strcmp(o->cur.text, text)))
     {
        free(text);
        return;
     }
   was = evas_object_is_in_output_rect(obj,
				       obj->layer->evas->pointer.x,
				       obj->layer->evas->pointer.y, 1, 1);
   /* DO II */
   /*Update intl_props*/
#ifdef BIDI_SUPPORT
   evas_bidi_paragraph_props_unref(o->cur.intl_props.props);
   o->cur.intl_props.props = evas_bidi_paragraph_props_get(text);
   evas_bidi_shape_string(text, &o->cur.intl_props, len);
#endif
   if (o->cur.text) eina_ustringshare_del(o->cur.text);
   if (o->cur.utf8_text) eina_stringshare_del(o->cur.utf8_text);

   if ((text) && (*text)) 
     {
        o->cur.text = eina_ustringshare_add(text);
        o->cur.utf8_text = eina_stringshare_add(_text);
     }
   else 
     {
        o->cur.text = NULL;
        o->cur.utf8_text = NULL;
     }

   o->prev.text = NULL;

   if ((o->engine_data) && (o->cur.text))
     {
	int w, h;
	int l = 0, r = 0, t = 0, b = 0;

	ENFN->font_string_size_get(ENDT,
				   o->engine_data,
				   o->cur.text, &o->cur.intl_props,
				   &w, &h);
	evas_text_style_pad_get(o->cur.style, &l, &r, &t, &b);
	obj->cur.geometry.w = w + l + r;
        obj->cur.geometry.h = h + t + b;
////        obj->cur.cache.geometry.validity = 0;
     }
   else
     {
	int t = 0, b = 0;

	evas_text_style_pad_get(o->cur.style, NULL, NULL, &t, &b);
	obj->cur.geometry.w = 0;
        obj->cur.geometry.h = o->max_ascent + o->max_descent + t + b;
////        obj->cur.cache.geometry.validity = 0;
     }
   o->changed = 1;
   evas_object_change(obj);
   evas_object_clip_dirty(obj);
   evas_object_coords_recalc(obj);
   is = evas_object_is_in_output_rect(obj,
				      obj->layer->evas->pointer.x,
				      obj->layer->evas->pointer.y, 1, 1);
   if ((is || was) && obj->cur.visible)
     evas_event_feed_mouse_move(obj->layer->evas,
				obj->layer->evas->pointer.x,
				obj->layer->evas->pointer.y,
				obj->layer->evas->last_timestamp,
				NULL);
   evas_object_inform_call_resize(obj);
   if (text) free(text);
}

/**
 * Retrieves the text currently being displayed by the given evas text object.
 * @param  obj The given evas text object.
 * @return The text currently being displayed.  Do not free it.
 */
EAPI const char *
evas_object_text_text_get(const Evas_Object *obj)
{
   Evas_Object_Text *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   o = (Evas_Object_Text *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Text, MAGIC_OBJ_TEXT);
   return NULL;
   MAGIC_CHECK_END();
   return o->cur.utf8_text;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 *
 */
EAPI Evas_Coord
evas_object_text_ascent_get(const Evas_Object *obj)
{
   Evas_Object_Text *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   o = (Evas_Object_Text *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Text, MAGIC_OBJ_TEXT);
   return 0;
   MAGIC_CHECK_END();
   return o->ascent;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 *
 */
EAPI Evas_Coord
evas_object_text_descent_get(const Evas_Object *obj)
{
   Evas_Object_Text *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   o = (Evas_Object_Text *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Text, MAGIC_OBJ_TEXT);
   return 0;
   MAGIC_CHECK_END();
   return o->descent;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 *
 */
EAPI Evas_Coord
evas_object_text_max_ascent_get(const Evas_Object *obj)
{
   Evas_Object_Text *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   o = (Evas_Object_Text *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Text, MAGIC_OBJ_TEXT);
   return 0;
   MAGIC_CHECK_END();
   return o->max_ascent;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 *
 */
EAPI Evas_Coord
evas_object_text_max_descent_get(const Evas_Object *obj)
{
   Evas_Object_Text *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   o = (Evas_Object_Text *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Text, MAGIC_OBJ_TEXT);
   return 0;
   MAGIC_CHECK_END();
   return o->max_descent;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 *
 */
EAPI Evas_Coord
evas_object_text_inset_get(const Evas_Object *obj)
{
   Evas_Object_Text *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   o = (Evas_Object_Text *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Text, MAGIC_OBJ_TEXT);
   return 0;
   MAGIC_CHECK_END();
   if (!o->engine_data) return 0;
   if (!o->cur.text) return 0;
   return ENFN->font_inset_get(ENDT, o->engine_data, o->cur.text);
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 *
 */
EAPI Evas_Coord
evas_object_text_horiz_advance_get(const Evas_Object *obj)
{
   Evas_Object_Text *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   o = (Evas_Object_Text *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Text, MAGIC_OBJ_TEXT);
   return 0;
   MAGIC_CHECK_END();
   if (!o->engine_data) return 0;
   if (!o->cur.text) return 0;
   return ENFN->font_h_advance_get(ENDT, o->engine_data, o->cur.text, &o->cur.intl_props);
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 *
 */
EAPI Evas_Coord
evas_object_text_vert_advance_get(const Evas_Object *obj)
{
   Evas_Object_Text *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   o = (Evas_Object_Text *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Text, MAGIC_OBJ_TEXT);
   return 0;
   MAGIC_CHECK_END();
   if (!o->engine_data) return 0;
   if (!o->cur.text) return o->ascent + o->descent;
   return ENFN->font_v_advance_get(ENDT, o->engine_data, o->cur.text, &o->cur.intl_props);
}

/**
 * Retrieve position and dimension information of a character within a text @c Evas_Object.
 *
 * This function is used to obtain the X, Y, width and height of a the character
 * located at @p pos within the @c Evas_Object @p obj. @p obj must be a text object
 * as created with evas_object_text_add(). Any of the @c Evas_Coord parameters (@p cx,
 * @p cy, @p cw, @p ch) may be NULL in which case no value will be assigned to that
 * parameter.
 *
 * @param obj	The text object to retrieve position information for.
 * @param pos	The character position to request co-ordinates for.
 * @param cx	A pointer to an @c Evas_Coord to store the X value in (can be NULL).
 * @param cy	A pointer to an @c Evas_Coord to store the Y value in (can be NULL).
 * @param cw	A pointer to an @c Evas_Coord to store the Width value in (can be NULL).
 * @param ch	A pointer to an @c Evas_Coord to store the Height value in (can be NULL).
 *
 * @returns EINA_FALSE on error, EINA_TRUE on success.
 */
EAPI Eina_Bool
evas_object_text_char_pos_get(const Evas_Object *obj, int pos, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch)
{
   Evas_Object_Text *o;
   int l = 0, r = 0, t = 0, b = 0;
   int ret, x = 0, y = 0, w = 0, h = 0;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return EINA_FALSE;
   MAGIC_CHECK_END();
   o = (Evas_Object_Text *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Text, MAGIC_OBJ_TEXT);
   return EINA_FALSE;
   MAGIC_CHECK_END();
   if (!o->engine_data) return EINA_FALSE;
   if (!o->cur.text) return EINA_FALSE;
   ret = ENFN->font_char_coords_get(ENDT, o->engine_data, o->cur.text, &o->cur.intl_props,
				    pos, &x, &y, &w, &h);
   evas_text_style_pad_get(o->cur.style, &l, &r, &t, &b);
   y += o->max_ascent - t;
   x -= l;
   if (x < 0)
     {
	w += x;
	x = 0;
     }
   if ((x + w) > obj->cur.geometry.w) w = obj->cur.geometry.w - x;
   if (w < 0) w = 0;
   if (y < 0)
     {
	h += y;
	y = 0;
     }
   if ((y + h) > obj->cur.geometry.h) h = obj->cur.geometry.h - y;
   if (h < 0) h = 0;
   if (cx) *cx = x;
   if (cy) *cy = y;
   if (cw) *cw = w + l + r;
   if (ch) *ch = h + t + b;
   return ret;
}


/**
 * Returns the logical position of the last char in the text
 * up to the pos given. this is NOT the position of the last char
 * because of the possibility of RTL in the text.
 * To be documented.
 *
 * FIXME: To be fixed.
 *
 */
EAPI int
evas_object_text_last_up_to_pos(const Evas_Object *obj, Evas_Coord x, Evas_Coord y)
{
   Evas_Object_Text *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return -1;
   MAGIC_CHECK_END();
   o = (Evas_Object_Text *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Text, MAGIC_OBJ_TEXT);
   return -1;
   MAGIC_CHECK_END();
   if (!o->engine_data) return -1;
   if (!o->cur.text) return -1;
   return ENFN->font_last_up_to_pos(ENDT,
				       o->engine_data,
				       o->cur.text, &o->cur.intl_props,
				       x,
				       y - o->max_ascent);	
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 *
 */
EAPI int
evas_object_text_char_coords_get(const Evas_Object *obj, Evas_Coord x, Evas_Coord y, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch)
{
   Evas_Object_Text *o;
   int l = 0, r = 0, t = 0, b = 0;
   int ret, rx = 0, ry = 0, rw = 0, rh = 0;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return -1;
   MAGIC_CHECK_END();
   o = (Evas_Object_Text *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Text, MAGIC_OBJ_TEXT);
   return -1;
   MAGIC_CHECK_END();
   if (!o->engine_data) return -1;
   if (!o->cur.text) return -1;
   ret = ENFN->font_char_at_coords_get(ENDT,
				       o->engine_data,
				       o->cur.text, &o->cur.intl_props,
				       x,
				       y - o->max_ascent,
				       &rx, &ry,
				       &rw, &rh);
   evas_text_style_pad_get(o->cur.style, &l, &r, &t, &b);
   ry += o->max_ascent - t;
   rx -= l;
   if (rx < 0)
     {
	rw += rx;
	rx = 0;
     }
   if ((rx + rw) > obj->cur.geometry.w) rw = obj->cur.geometry.w - rx;
   if (rw < 0) rw = 0;
   if (ry < 0)
     {
	rh += ry;
	ry = 0;
     }
   if ((ry + rh) > obj->cur.geometry.h) rh = obj->cur.geometry.h - ry;
   if (rh < 0) rh = 0;
   if (cx) *cx = rx;
   if (cy) *cy = ry;
   if (cw) *cw = rw + l + r;
   if (ch) *ch = rh + t + b;
   return ret;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 *
 */
EAPI void
evas_object_text_style_set(Evas_Object *obj, Evas_Text_Style_Type style)
{
   Evas_Object_Text *o;
   int pl = 0, pr = 0, pt = 0, pb = 0, l = 0, r = 0, t = 0, b = 0;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Text *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Text, MAGIC_OBJ_TEXT);
   return;
   MAGIC_CHECK_END();
   if (o->cur.style == style) return;
   evas_text_style_pad_get(o->cur.style, &pl, &pr, &pt, &pb);
   o->cur.style = style;
   evas_text_style_pad_get(o->cur.style, &l, &r, &t, &b);
   if (o->cur.text)
     obj->cur.geometry.w += (l - pl) + (r - pr);
   else
     obj->cur.geometry.w = 0;
   obj->cur.geometry.h += (t - pt) + (b - pb);
   evas_object_change(obj);
   evas_object_clip_dirty(obj);
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 *
 */
EAPI Evas_Text_Style_Type
evas_object_text_style_get(const Evas_Object *obj)
{
   Evas_Object_Text *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return EVAS_TEXT_STYLE_PLAIN;
   MAGIC_CHECK_END();
   o = (Evas_Object_Text *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Text, MAGIC_OBJ_TEXT);
   return EVAS_TEXT_STYLE_PLAIN;
   MAGIC_CHECK_END();
   return o->cur.style;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 *
 */
EAPI void
evas_object_text_shadow_color_set(Evas_Object *obj, int r, int g, int b, int a)
{
   Evas_Object_Text *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Text *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Text, MAGIC_OBJ_TEXT);
   return;
   MAGIC_CHECK_END();
   if ((o->cur.shadow.r == r) && (o->cur.shadow.g == g) &&
       (o->cur.shadow.b == b) && (o->cur.shadow.a == a))
     return;
   o->cur.shadow.r = r;
   o->cur.shadow.g = g;
   o->cur.shadow.b = b;
   o->cur.shadow.a = a;
   o->changed = 1;
   evas_object_change(obj);
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 *
 */
EAPI void
evas_object_text_shadow_color_get(const Evas_Object *obj, int *r, int *g, int *b, int *a)
{
   Evas_Object_Text *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   if (r) *r = 0;
   if (g) *g = 0;
   if (b) *b = 0;
   if (a) *a = 0;
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Text *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Text, MAGIC_OBJ_TEXT);
   if (r) *r = 0;
   if (g) *g = 0;
   if (b) *b = 0;
   if (a) *a = 0;
   return;
   MAGIC_CHECK_END();
   if (r) *r = o->cur.shadow.r;
   if (g) *g = o->cur.shadow.g;
   if (b) *b = o->cur.shadow.b;
   if (a) *a = o->cur.shadow.a;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 *
 */
EAPI void
evas_object_text_glow_color_set(Evas_Object *obj, int r, int g, int b, int a)
{
   Evas_Object_Text *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Text *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Text, MAGIC_OBJ_TEXT);
   return;
   MAGIC_CHECK_END();
   if ((o->cur.glow.r == r) && (o->cur.glow.g == g) &&
       (o->cur.glow.b == b) && (o->cur.glow.a == a))
     return;
   o->cur.glow.r = r;
   o->cur.glow.g = g;
   o->cur.glow.b = b;
   o->cur.glow.a = a;
   o->changed = 1;
   evas_object_change(obj);
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 *
 */
EAPI void
evas_object_text_glow_color_get(const Evas_Object *obj, int *r, int *g, int *b, int *a)
{
   Evas_Object_Text *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   if (r) *r = 0;
   if (g) *g = 0;
   if (b) *b = 0;
   if (a) *a = 0;
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Text *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Text, MAGIC_OBJ_TEXT);
   if (r) *r = 0;
   if (g) *g = 0;
   if (b) *b = 0;
   if (a) *a = 0;
   return;
   MAGIC_CHECK_END();
   if (r) *r = o->cur.glow.r;
   if (g) *g = o->cur.glow.g;
   if (b) *b = o->cur.glow.b;
   if (a) *a = o->cur.glow.a;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 *
 */
EAPI void
evas_object_text_glow2_color_set(Evas_Object *obj, int r, int g, int b, int a)
{
   Evas_Object_Text *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Text *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Text, MAGIC_OBJ_TEXT);
   return;
   MAGIC_CHECK_END();
   if ((o->cur.glow2.r == r) && (o->cur.glow2.g == g) &&
       (o->cur.glow2.b == b) && (o->cur.glow2.a == a))
     return;
   o->cur.glow2.r = r;
   o->cur.glow2.g = g;
   o->cur.glow2.b = b;
   o->cur.glow2.a = a;
   o->changed = 1;
   evas_object_change(obj);
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 *
 */
EAPI void
evas_object_text_glow2_color_get(const Evas_Object *obj, int *r, int *g, int *b, int *a)
{
   Evas_Object_Text *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   if (r) *r = 0;
   if (g) *g = 0;
   if (b) *b = 0;
   if (a) *a = 0;
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Text *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Text, MAGIC_OBJ_TEXT);
   if (r) *r = 0;
   if (g) *g = 0;
   if (b) *b = 0;
   if (a) *a = 0;
   return;
   MAGIC_CHECK_END();
   if (r) *r = o->cur.glow2.r;
   if (g) *g = o->cur.glow2.g;
   if (b) *b = o->cur.glow2.b;
   if (a) *a = o->cur.glow2.a;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 *
 */
EAPI void
evas_object_text_outline_color_set(Evas_Object *obj, int r, int g, int b, int a)
{
   Evas_Object_Text *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Text *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Text, MAGIC_OBJ_TEXT);
   return;
   MAGIC_CHECK_END();
   if ((o->cur.outline.r == r) && (o->cur.outline.g == g) &&
       (o->cur.outline.b == b) && (o->cur.outline.a == a))
     return;
   o->cur.outline.r = r;
   o->cur.outline.g = g;
   o->cur.outline.b = b;
   o->cur.outline.a = a;
   o->changed = 1;
   evas_object_change(obj);
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 *
 */
EAPI void
evas_object_text_outline_color_get(const Evas_Object *obj, int *r, int *g, int *b, int *a)
{
   Evas_Object_Text *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   if (r) *r = 0;
   if (g) *g = 0;
   if (b) *b = 0;
   if (a) *a = 0;
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Text *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Text, MAGIC_OBJ_TEXT);
   if (r) *r = 0;
   if (g) *g = 0;
   if (b) *b = 0;
   if (a) *a = 0;
   return;
   MAGIC_CHECK_END();
   if (r) *r = o->cur.outline.r;
   if (g) *g = o->cur.outline.g;
   if (b) *b = o->cur.outline.b;
   if (a) *a = o->cur.outline.a;
}

/**
 * Gets the text style pad of a text object.
 *
 * @param obj The given text object.
 * @param l The left pad (or NULL).
 * @param r The right pad (or NULL).
 * @param t The top pad (or NULL).
 * @param b The bottom pad (or NULL).
 *
 */
EAPI void
evas_object_text_style_pad_get(const Evas_Object *obj, int *l, int *r, int *t, int *b)
{
   int sl = 0, sr = 0, st = 0, sb = 0;
   Evas_Object_Text *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   if (l) *l = 0;
   if (r) *r = 0;
   if (t) *t = 0;
   if (b) *b = 0;
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Text *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Text, MAGIC_OBJ_TEXT);
   if (l) *l = 0;
   if (r) *r = 0;
   if (t) *t = 0;
   if (b) *b = 0;
   return;
   MAGIC_CHECK_END();
   /* use temps to be certain we have initialized values */
   evas_text_style_pad_get(o->cur.style, &sl, &sr, &st, &sb);
   if (l) *l = sl;
   if (r) *r = sr;
   if (t) *t = st;
   if (b) *b = sb;
}


/**
 * @}
 */


/**
 * Removes all font paths loaded into memory for the given evas.
 * @param   e The given evas.
 * @ingroup Evas_Font_Path_Group
 */
EAPI void
evas_font_path_clear(Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   while (e->font_path)
     {
	eina_stringshare_del(e->font_path->data);
	e->font_path = eina_list_remove(e->font_path, e->font_path->data);
     }
}

/**
 * Appends a font path to the list of font paths used by the given evas.
 * @param   e    The given evas.
 * @param   path The new font path.
 * @ingroup Evas_Font_Path_Group
 */
EAPI void
evas_font_path_append(Evas *e, const char *path)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   if (!path) return;
   e->font_path = eina_list_append(e->font_path, eina_stringshare_add(path));
}

/**
 * Prepends a font path to the list of font paths used by the given evas.
 * @param   e The given evas.
 * @param   path The new font path.
 * @ingroup Evas_Font_Path_Group
 */
EAPI void
evas_font_path_prepend(Evas *e, const char *path)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   if (!path) return;
   e->font_path = eina_list_prepend(e->font_path, eina_stringshare_add(path));
}

/**
 * Retrieves the list of font paths used by the given evas.
 * @param   e The given evas.
 * @return  The list of font paths used.
 * @ingroup Evas_Font_Path_Group
 */
EAPI const Eina_List *
evas_font_path_list(const Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   return e->font_path;
}

static void
evas_font_object_rehint(Evas_Object *obj)
{
   if (obj->smart.smart)
     {
	EINA_INLIST_FOREACH(evas_object_smart_members_get_direct(obj), obj)
	  evas_font_object_rehint(obj);
     }
   else
     {
	if (!strcmp(obj->type, "text"))
	  _evas_object_text_rehint(obj);
	if (!strcmp(obj->type, "textblock"))
	  _evas_object_textblock_rehint(obj);
     }
}

/**
 * Changes the font hinting for the given evas.
 *
 * @param e The given evas.
 * @param hinting The hinting to use, one of #EVAS_FONT_HINTING_NONE,
 *        #EVAS_FONT_HINTING_AUTO, #EVAS_FONT_HINTING_BYTECODE.
 * @ingroup Evas_Font_Group
 */
EAPI void
evas_font_hinting_set(Evas *e, Evas_Font_Hinting_Flags hinting)
{
   Evas_Layer *lay;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   if (e->hinting == hinting) return;
   e->hinting = hinting;

   EINA_INLIST_FOREACH(e->layers, lay)
     {
	Evas_Object *obj;

	EINA_INLIST_FOREACH(lay->objects, obj)
	  evas_font_object_rehint(obj);
     }
}

/**
 * Retrieves the font hinting used by the given evas.
 *
 * @param e The given evas to query.
 * @return The hinting in use, one of #EVAS_FONT_HINTING_NONE,
 *         #EVAS_FONT_HINTING_AUTO, #EVAS_FONT_HINTING_BYTECODE.
 * @ingroup Evas_Font_Group
 */
EAPI Evas_Font_Hinting_Flags
evas_font_hinting_get(const Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return EVAS_FONT_HINTING_BYTECODE;
   MAGIC_CHECK_END();
   return e->hinting;
}

/**
 * Checks if the font hinting is supported by the given evas.
 *
 * @param e The given evas to query.
 * @param hinting The hinting to use, one of #EVAS_FONT_HINTING_NONE,
 *        #EVAS_FONT_HINTING_AUTO, #EVAS_FONT_HINTING_BYTECODE.
 * @return @c EINA_TRUE if it is supported, @c EINA_FALSE otherwise.
 * @ingroup Evas_Font_Group
 */
EAPI Eina_Bool
evas_font_hinting_can_hint(const Evas *e, Evas_Font_Hinting_Flags hinting)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return 0;
   MAGIC_CHECK_END();
   if (e->engine.func->font_hinting_can_hint)
     return e->engine.func->font_hinting_can_hint(e->engine.data.output,
						  hinting);
   return EINA_FALSE;
}

/**
 * Force the given evas and associated engine to flush its font cache.
 *
 * @param e The given evas to flush font cache.
 * @ingroup Evas_Font_Group
 */
EAPI void
evas_font_cache_flush(Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   e->engine.func->font_cache_flush(e->engine.data.output);
}

/**
 * Changes the size of font cache of the given evas.
 *
 * @param e The given evas to flush font cache.
 * @param size The size, in bytes.
 *
 * @ingroup Evas_Font_Group
 */
EAPI void
evas_font_cache_set(Evas *e, int size)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   if (size < 0) size = 0;
   e->engine.func->font_cache_set(e->engine.data.output, size);
}

/**
 * Changes the size of font cache of the given evas.
 *
 * @param e The given evas to flush font cache.
 * @return The size, in bytes.
 *
 * @ingroup Evas_Font_Group
 */
EAPI int
evas_font_cache_get(const Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return 0;
   MAGIC_CHECK_END();

   return e->engine.func->font_cache_get(e->engine.data.output);
}

/**
 * List of available font descriptions known or found by this evas.
 *
 * The list depends on Evas compile time configuration, such as
 * fontconfig support, and the paths provided at runtime as explained
 * in @ref Evas_Font_Path_Group.
 *
 * @param e The evas instance to query.
 * @return a newly allocated list of strings. Do not change the
 *         strings.  Be sure to call evas_font_available_list_free()
 *         after you're done.
 *
 * @ingroup Evas_Font_Group
 */
EAPI Eina_List *
evas_font_available_list(const Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();

   return evas_font_dir_available_list(e);
}

/**
 * Free list of font descriptions returned by evas_font_dir_available_list().
 *
 * @param e The evas instance that returned such list.
 * @param available the list returned by evas_font_dir_available_list().
 *
 * @ingroup Evas_Font_Group
 */
EAPI void
evas_font_available_list_free(Evas *e, Eina_List *available)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   evas_font_dir_available_list_free(available);
}

/**
 * Gets the next character in the string
 *
 * Given the UTF-8 string in @p str, and starting byte position in @p pos,
 * this function will place in @p decoded the decoded code point at @p pos
 * and return the byte index for the next character in the string.
 *
 * The only boundary check done is that @p pos must be >= 0. Other than that,
 * no checks are performed, so passing an index value that's not within the
 * length of the string will result in undefined behavior.
 *
 * @param str The UTF-8 string
 * @param pos The byte index where to start
 * @param decoded Address where to store the decoded code point. Optional.
 *
 * @return The byte index of the next character
 *
 * @ingroup Evas_Utils
 */
EAPI int
evas_string_char_next_get(const char *str, int pos, int *decoded)
{
   int p, d;

   if (decoded) *decoded = 0;
   if ((!str) || (pos < 0)) return 0;
   p = pos;
   d = evas_common_encoding_utf8_get_next(str, &p);
   if (decoded) *decoded = d;
   return p;
}

/**
 * Gets the previous character in the string
 *
 * Given the UTF-8 string in @p str, and starting byte position in @p pos,
 * this function will place in @p decoded the decoded code point at @p pos
 * and return the byte index for the previous character in the string.
 *
 * The only boundary check done is that @p pos must be >= 1. Other than that,
 * no checks are performed, so passing an index value that's not within the
 * length of the string will result in undefined behavior.
 *
 * @param str The UTF-8 string
 * @param pos The byte index where to start
 * @param decoded Address where to store the decoded code point. Optional.
 *
 * @return The byte index of the previous character
 *
 * @ingroup Evas_Utils
 */
EAPI int
evas_string_char_prev_get(const char *str, int pos, int *decoded)
{
   int p, d;

   if (decoded) *decoded = 0;
   if ((!str) || (pos < 1)) return 0;
   p = pos;
   d = evas_common_encoding_utf8_get_prev(str, &p);
   if (decoded) *decoded = d;
   return p;
}

/**
 * Get the length in characters of the string.
 * @param  str The string to get the length of.
 * @return The length in characters (not bytes)
 * @ingroup Evas_Utils
 */
EAPI int
evas_string_char_len_get(const char *str)
{
   if (!str) return 0;
   return evas_common_encoding_utf8_get_len(str);
}

/**
 * Get the minimum padding a style adds to the text.
 * @param style The style to determine padding.
 * @param     l Pointer to the current left padding value
 * @param     r Pointer to the current right padding value
 * @param     t Pointer to the current top padding value
 * @param     b Pointer to the current bottom padding value
 */
void
evas_text_style_pad_get(Evas_Text_Style_Type style, int *l, int *r, int *t, int *b)
{
   int sl, sr, st, sb;

   if (l) sl = *l;
   else sl = 0;

   if (r) sr = *r;
   else sr = 0;

   if (t) st = *t;
   else st = 0;

   if (b) sb = *b;
   else sb = 0;

   if (style == EVAS_TEXT_STYLE_SHADOW)
     {
	if (sr < 1) sr = 1;
	if (sb < 1) sb = 1;
     }
   else if (style == EVAS_TEXT_STYLE_OUTLINE)
     {
	if (sl < 1) sl = 1;
	if (sr < 1) sr = 1;
	if (st < 1) st = 1;
	if (sb < 1) sb = 1;
     }
   else if (style == EVAS_TEXT_STYLE_SOFT_OUTLINE)
     {
	if (sl < 2) sl = 2;
	if (sr < 2) sr = 2;
	if (st < 2) st = 2;
	if (sb < 2) sb = 2;
     }
   else if (style == EVAS_TEXT_STYLE_GLOW)
     {
	if (sl < 2) sl = 2;
	if (sr < 2) sr = 2;
	if (st < 2) st = 2;
	if (sb < 2) sb = 2;
     }
   else if (style == EVAS_TEXT_STYLE_OUTLINE_SHADOW)
     {
	if (sl < 1) sl = 1;
	if (sr < 2) sr = 2;
	if (st < 1) st = 1;
	if (sb < 2) sb = 2;
     }
   else if (style == EVAS_TEXT_STYLE_FAR_SHADOW)
     {
	if (sr < 2) sr = 2;
	if (sb < 2) sb = 2;
     }
   else if (style == EVAS_TEXT_STYLE_OUTLINE_SOFT_SHADOW)
     {
	if (sl < 1) sl = 1;
	if (sr < 3) sr = 3;
	if (st < 1) st = 1;
	if (sb < 3) sb = 3;
     }
   else if (style == EVAS_TEXT_STYLE_SOFT_SHADOW)
     {
	if (sl < 1) sl = 1;
	if (sr < 3) sr = 3;
	if (st < 1) st = 1;
	if (sb < 3) sb = 3;
     }
   else if (style == EVAS_TEXT_STYLE_FAR_SOFT_SHADOW)
     {
	if (sr < 4) sr = 4;
	if (sb < 4) sb = 4;
     }

   if (l) *l = sl;
   if (r) *r = sr;
   if (t) *t = st;
   if (b) *b = sb;
}

/* all nice and private */
static void
evas_object_text_init(Evas_Object *obj)
{
   /* alloc text ob, setup methods and default values */
   obj->object_data = evas_object_text_new();
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
   /* set up object-specific settings */
   obj->prev = obj->cur;
   /* set up methods (compulsory) */
   obj->func = &object_func;
   obj->type = o_type;
}

static void *
evas_object_text_new(void)
{
   Evas_Object_Text *o;

   /* alloc obj private data */
   EVAS_MEMPOOL_INIT(_mp_obj, "evas_object_text", Evas_Object_Text, 128, NULL);
   o = EVAS_MEMPOOL_ALLOC(_mp_obj, Evas_Object_Text);
   if (!o) return NULL;
   EVAS_MEMPOOL_PREP(_mp_obj, o, Evas_Object_Text);
   o->magic = MAGIC_OBJ_TEXT;
   o->prev = o->cur;
#ifdef BIDI_SUPPORT
   o->cur.intl_props.props = evas_bidi_paragraph_props_new();
#endif
   return o;
}

static void
evas_object_text_free(Evas_Object *obj)
{
   Evas_Object_Text *o;

   /* frees private object data. very simple here */
   o = (Evas_Object_Text *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Text, MAGIC_OBJ_TEXT);
   return;
   MAGIC_CHECK_END();
   /* free obj */
   if (o->cur.text) eina_ustringshare_del(o->cur.text);
   if (o->cur.utf8_text) eina_stringshare_del(o->cur.utf8_text);
   if (o->cur.font) eina_stringshare_del(o->cur.font);
   if (o->cur.source) eina_stringshare_del(o->cur.source);
   if (o->engine_data) evas_font_free(obj->layer->evas, o->engine_data);
#ifdef BIDI_SUPPORT
   evas_bidi_props_clean(&o->cur.intl_props);
#endif
   o->magic = 0;
   EVAS_MEMPOOL_FREE(_mp_obj, o);
}

static void
evas_object_text_render(Evas_Object *obj, void *output, void *context, void *surface, int x, int y)
{
   int i, j;
   Evas_Object_Text *o;
   const char vals[5][5] =
     {
	{0, 1, 2, 1, 0},
	{1, 3, 4, 3, 1},
	{2, 4, 5, 4, 2},
	{1, 3, 4, 3, 1},
	{0, 1, 2, 1, 0}
     };
   int sl = 0, st = 0;

   /* render object to surface with context, and offxet by x,y */
   o = (Evas_Object_Text *)(obj->object_data);
   evas_text_style_pad_get(o->cur.style, &sl, NULL, &st, NULL);
   ENFN->context_multiplier_unset(output, context);
   ENFN->context_render_op_set(output, context, obj->cur.render_op);
/*
   ENFN->context_color_set(output,
                           context,
                           230, 160, 30, 100);
   ENFN->rectangle_draw(output,
                        context,
                        surface,
                        obj->cur.geometry.x + x,
                        obj->cur.geometry.y + y,
                        obj->cur.geometry.w,
                        obj->cur.geometry.h);
 */
#define COLOR_ONLY_SET(object, sub, col) \
	ENFN->context_color_set(output, context, \
				object->sub.col.r, \
				object->sub.col.g, \
				object->sub.col.b, \
				object->sub.col.a);

#define COLOR_SET(object, sub, col) \
        if (obj->cur.clipper)\
	   ENFN->context_color_set(output, context, \
				((int)object->sub.col.r * ((int)obj->cur.clipper->cur.cache.clip.r + 1)) >> 8, \
				((int)object->sub.col.g * ((int)obj->cur.clipper->cur.cache.clip.g + 1)) >> 8, \
				((int)object->sub.col.b * ((int)obj->cur.clipper->cur.cache.clip.b + 1)) >> 8, \
				((int)object->sub.col.a * ((int)obj->cur.clipper->cur.cache.clip.a + 1)) >> 8); \
        else\
	   ENFN->context_color_set(output, context, \
				object->sub.col.r, \
				object->sub.col.g, \
				object->sub.col.b, \
				object->sub.col.a);

#define COLOR_SET_AMUL(object, sub, col, amul) \
        if (obj->cur.clipper) \
	    ENFN->context_color_set(output, context, \
				(((int)object->sub.col.r) * ((int)obj->cur.clipper->cur.cache.clip.r) * (amul)) / 65025, \
				(((int)object->sub.col.g) * ((int)obj->cur.clipper->cur.cache.clip.g) * (amul)) / 65025, \
				(((int)object->sub.col.b) * ((int)obj->cur.clipper->cur.cache.clip.b) * (amul)) / 65025, \
				(((int)object->sub.col.a) * ((int)obj->cur.clipper->cur.cache.clip.a) * (amul)) / 65025); \
        else \
	    ENFN->context_color_set(output, context, \
				(((int)object->sub.col.r) * (amul)) / 255, \
				(((int)object->sub.col.g) * (amul)) / 255, \
				(((int)object->sub.col.b) * (amul)) / 255, \
				(((int)object->sub.col.a) * (amul)) / 255);

#define DRAW_TEXT(ox, oy) \
   if ((o->engine_data) && (o->cur.text)) \
     ENFN->font_draw(output, \
		     context, \
		     surface, \
		     o->engine_data, \
		     obj->cur.geometry.x + x + sl + ox, \
		     obj->cur.geometry.y + y + st + oy + \
		     (int) \
		     (((o->max_ascent * obj->cur.geometry.h) / obj->cur.geometry.h) - 0.5), \
		     obj->cur.geometry.w, \
		     obj->cur.geometry.h, \
		     obj->cur.geometry.w, \
		     obj->cur.geometry.h, \
		     o->cur.text, &o->cur.intl_props);
#if 0
#define DRAW_TEXT(ox, oy) \
   if ((o->engine_data) && (o->cur.text)) \
     ENFN->font_draw(output, \
		     context, \
		     surface, \
		     o->engine_data, \
		     obj->cur.cache.geometry.x + x + sl + ox, \
		     obj->cur.cache.geometry.y + y + st + oy + \
		     (int) \
		     (((o->max_ascent * obj->cur.cache.geometry.h) / obj->cur.geometry.h) - 0.5), \
		     obj->cur.cache.geometry.w, \
		     obj->cur.cache.geometry.h, \
		     obj->cur.geometry.w, \
		     obj->cur.geometry.h, \
		     o->cur.text, &o->cur.intl_props);
#endif
   /* shadows */
   if (o->cur.style == EVAS_TEXT_STYLE_SHADOW)
     {
	COLOR_SET(o, cur, shadow);
        DRAW_TEXT(1, 1);
     }
   else if ((o->cur.style == EVAS_TEXT_STYLE_OUTLINE_SHADOW) ||
	    (o->cur.style == EVAS_TEXT_STYLE_FAR_SHADOW))
     {
	COLOR_SET(o, cur, shadow);
        DRAW_TEXT(2, 2);
     }
   else if ((o->cur.style == EVAS_TEXT_STYLE_OUTLINE_SOFT_SHADOW) ||
	    (o->cur.style == EVAS_TEXT_STYLE_FAR_SOFT_SHADOW))
     {
	for (j = 0; j < 5; j++)
	  {
	     for (i = 0; i < 5; i++)
	       {
		  if (vals[i][j] != 0)
		    {
		       COLOR_SET_AMUL(o, cur, shadow, vals[i][j] * 50);
		       DRAW_TEXT(i, j);
		    }
	       }
	  }
     }
   else if (o->cur.style == EVAS_TEXT_STYLE_SOFT_SHADOW)
     {
	for (j = 0; j < 5; j++)
	  {
	     for (i = 0; i < 5; i++)
	       {
		  if (vals[i][j] != 0)
		    {
		       COLOR_SET_AMUL(o, cur, shadow, vals[i][j] * 50);
		       DRAW_TEXT(i - 1, j - 1);
		    }
	       }
	  }
     }

   /* glows */
   if (o->cur.style == EVAS_TEXT_STYLE_GLOW)
     {
	for (j = 0; j < 5; j++)
	  {
	     for (i = 0; i < 5; i++)
	       {
		  if (vals[i][j] != 0)
		    {
		       COLOR_SET_AMUL(o, cur, glow, vals[i][j] * 50);
		       DRAW_TEXT(i - 2, j - 2);
		    }
	       }
	  }
	COLOR_SET(o, cur, glow2);
	DRAW_TEXT(-1, 0);
	DRAW_TEXT(1, 0);
	DRAW_TEXT(0, -1);
	DRAW_TEXT(0, 1);
     }

   /* outlines */
   if ((o->cur.style == EVAS_TEXT_STYLE_OUTLINE) ||
       (o->cur.style == EVAS_TEXT_STYLE_OUTLINE_SHADOW) ||
       (o->cur.style == EVAS_TEXT_STYLE_OUTLINE_SOFT_SHADOW))
     {
	COLOR_SET(o, cur, outline);
	DRAW_TEXT(-1, 0);
	DRAW_TEXT(1, 0);
	DRAW_TEXT(0, -1);
	DRAW_TEXT(0, 1);
     }
   else if (o->cur.style == EVAS_TEXT_STYLE_SOFT_OUTLINE)
     {
	for (j = 0; j < 5; j++)
	  {
	     for (i = 0; i < 5; i++)
	       {
		  if (((i != 2) || (j != 2)) && (vals[i][j] != 0))
		    {
		       COLOR_SET_AMUL(o, cur, outline, vals[i][j] * 50);
		       DRAW_TEXT(i - 2, j - 2);
		    }
	       }
	  }
     }

   /* normal text */
   COLOR_ONLY_SET(obj, cur.cache, clip);
   DRAW_TEXT(0, 0);
}

static void
evas_object_text_render_pre(Evas_Object *obj)
{
   Evas_Object_Text *o;
   int is_v, was_v;

   /* dont pre-render the obj twice! */
   if (obj->pre_render_done) return;
   obj->pre_render_done = 1;
   /* pre-render phase. this does anything an object needs to do just before
    rendering. This could mean loading the image data, retrieving it from 
    elsewhere, decoding video etc.
    Then when this is done the object needs to figure if it changed and 
    if so what and where and add the appropriate redraw rectangles */
   o = (Evas_Object_Text *)(obj->object_data);
   /* if someone is clipping this obj - go calculate the clipper */
   if (obj->cur.clipper)
     {
	if (obj->cur.cache.clip.dirty)
	  evas_object_clip_recalc(obj->cur.clipper);
	obj->cur.clipper->func->render_pre(obj->cur.clipper);
     }
   /* now figure what changed and add draw rects
    if it just became visible or invisible */
   is_v = evas_object_is_visible(obj);
   was_v = evas_object_was_visible(obj);
   if (is_v != was_v)
     {
	evas_object_render_pre_visible_change(&obj->layer->evas->clip_changes, 
					      obj, is_v, was_v);
	goto done;
     }
   if ((obj->cur.map != obj->prev.map) ||
       (obj->cur.usemap != obj->prev.usemap))
     {
	evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes, 
					    obj);
        goto done;
     }
   /* its not visible - we accounted for it appearing or not so just abort */
   if (!is_v) goto done;
   /* clipper changed this is in addition to anything else for obj */
   evas_object_render_pre_clipper_change(&obj->layer->evas->clip_changes, obj);
   /* if we restacked (layer or just within a layer) and dont clip anyone */
   if (obj->restack)
     {
	evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes, 
					    obj);
	goto done;
     }
   /* if it changed color */
   if ((obj->cur.color.r != obj->prev.color.r) ||
       (obj->cur.color.g != obj->prev.color.g) ||
       (obj->cur.color.b != obj->prev.color.b) ||
       (obj->cur.color.a != obj->prev.color.a))
     {
	evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes, 
					    obj);
	goto done;
     }
   /* if it changed geometry - and obviously not visibility or color
    calculate differences since we have a constant color fill
    we really only need to update the differences */
   if ((obj->cur.geometry.x != obj->prev.geometry.x) ||
       (obj->cur.geometry.y != obj->prev.geometry.y) ||
       (obj->cur.geometry.w != obj->prev.geometry.w) ||
       (obj->cur.geometry.h != obj->prev.geometry.h))
     {
	evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes, 
					    obj);
	goto done;
     }
   if (obj->cur.render_op != obj->prev.render_op)
     {
	evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes, 
					    obj);
	goto done;
     }
   if (obj->cur.scale != obj->prev.scale)
     {
	evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes, 
					    obj);
	goto done;
     }
   if (o->changed)
     {
	if ((o->cur.size != o->prev.size) ||
	    ((o->cur.font) && (o->prev.font) && 
		(strcmp(o->cur.font, o->prev.font))) ||
	    ((o->cur.font) && (!o->prev.font)) ||
	    ((!o->cur.font) && (o->prev.font)) ||
	    ((o->cur.text) && (o->prev.text) &&
                (eina_unicode_strcmp(o->cur.text, o->prev.text))) ||
	    ((o->cur.text) && (!o->prev.text)) ||
	    ((!o->cur.text) && (o->prev.text)) ||
	    ((o->cur.style != o->prev.style)) ||
	    ((o->cur.shadow.r != o->prev.shadow.r)) ||
	    ((o->cur.shadow.g != o->prev.shadow.g)) ||
	    ((o->cur.shadow.b != o->prev.shadow.b)) ||
	    ((o->cur.shadow.a != o->prev.shadow.a)) ||
	    ((o->cur.outline.r != o->prev.outline.r)) ||
	    ((o->cur.outline.g != o->prev.outline.g)) ||
	    ((o->cur.outline.b != o->prev.outline.b)) ||
	    ((o->cur.outline.a != o->prev.outline.a)) ||
	    ((o->cur.glow.r != o->prev.glow.r)) ||
	    ((o->cur.glow.g != o->prev.glow.g)) ||
	    ((o->cur.glow.b != o->prev.glow.b)) ||
	    ((o->cur.glow.a != o->prev.glow.a)) ||
	    ((o->cur.glow2.r != o->prev.glow2.r)) ||
	    ((o->cur.glow2.g != o->prev.glow2.g)) ||
	    ((o->cur.glow2.b != o->prev.glow2.b)) ||
	    ((o->cur.glow2.a != o->prev.glow2.a)))
	  {
	     evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes, 
						 obj);
	     goto done;
	  }
     }
   done:
   evas_object_render_pre_effect_updates(&obj->layer->evas->clip_changes, 
					 obj, is_v, was_v);
}

static void
evas_object_text_render_post(Evas_Object *obj)
{
   Evas_Object_Text *o;

   /* this moves the current data to the previous state parts of the object
    in whatever way is safest for the object. also if we don't need object
    data anymore we can free it if the object deems this is a good idea */
   o = (Evas_Object_Text *)(obj->object_data);
   /* remove those pesky changes */
   evas_object_clip_changes_clean(obj);
   /* move cur to prev safely for object data */
   obj->prev = obj->cur;
   o->prev = o->cur;
   o->changed = 0;
}

static unsigned int 
evas_object_text_id_get(Evas_Object *obj)
{
   Evas_Object_Text *o;

   o = (Evas_Object_Text *)(obj->object_data);
   if (!o) return 0;
   return MAGIC_OBJ_TEXT;
}

static unsigned int 
evas_object_text_visual_id_get(Evas_Object *obj)
{
   Evas_Object_Text *o;

   o = (Evas_Object_Text *)(obj->object_data);
   if (!o) return 0;
   return MAGIC_OBJ_SHAPE;
}

static void *
evas_object_text_engine_data_get(Evas_Object *obj)
{
   Evas_Object_Text *o;

   o = (Evas_Object_Text *)(obj->object_data);
   if (!o) return NULL;
   return o->engine_data;
}

static int
evas_object_text_is_opaque(Evas_Object *obj __UNUSED__)
{
   /* this returns 1 if the internal object data implies that the object is 
    currently fully opaque over the entire gradient it occupies */
   return 0;
}

static int
evas_object_text_was_opaque(Evas_Object *obj __UNUSED__)
{
   /* this returns 1 if the internal object data implies that the object was
    currently fully opaque over the entire gradient it occupies */
   return 0;
}

static void
evas_object_text_scale_update(Evas_Object *obj)
{
   Evas_Object_Text *o;
   int size;
   const char *font;

   o = (Evas_Object_Text *)(obj->object_data);
   font = eina_stringshare_add(o->cur.font);
   size = o->cur.size;
   if (o->cur.font) eina_stringshare_del(o->cur.font);
   o->cur.font = NULL;
   o->prev.font = NULL;
   o->cur.size = 0;
   o->prev.size = 0;
   evas_object_text_font_set(obj, font, size);
}

void
_evas_object_text_rehint(Evas_Object *obj)
{
   Evas_Object_Text *o;
   int is, was;

   o = (Evas_Object_Text *)(obj->object_data);
   if (!o->engine_data) return;
#ifdef EVAS_FRAME_QUEUING
   evas_common_pipe_op_text_flush(o->engine_data);
#endif
   evas_font_load_hinting_set(obj->layer->evas, o->engine_data,
			      obj->layer->evas->hinting);
   was = evas_object_is_in_output_rect(obj,
				       obj->layer->evas->pointer.x,
				       obj->layer->evas->pointer.y, 1, 1);
   /* DO II */
   o->prev.text = NULL;
   if ((o->engine_data) && (o->cur.text))
     {
	int w, h;
	int l = 0, r = 0, t = 0, b = 0;

	ENFN->font_string_size_get(ENDT,
				   o->engine_data,
				   o->cur.text, &o->cur.intl_props,
				   &w, &h);
	evas_text_style_pad_get(o->cur.style, &l, &r, &t, &b);
	obj->cur.geometry.w = w + l + r;
	obj->cur.geometry.h = h + t + b;
////	obj->cur.cache.geometry.validity = 0;
     }
   else
     {
	int t = 0, b = 0;

	evas_text_style_pad_get(o->cur.style, NULL, NULL, &t, &b);
	obj->cur.geometry.w = 0;
	obj->cur.geometry.h = o->max_ascent + o->max_descent + t + b;
////	obj->cur.cache.geometry.validity = 0;
     }
   o->changed = 1;
   evas_object_change(obj);
   evas_object_clip_dirty(obj);
   evas_object_coords_recalc(obj);
   is = evas_object_is_in_output_rect(obj,
				      obj->layer->evas->pointer.x,
				      obj->layer->evas->pointer.y, 1, 1);
   if ((is || was) && obj->cur.visible)
     evas_event_feed_mouse_move(obj->layer->evas,
				obj->layer->evas->pointer.x,
				obj->layer->evas->pointer.y,
				obj->layer->evas->last_timestamp,
				NULL);
   evas_object_inform_call_resize(obj);
}
