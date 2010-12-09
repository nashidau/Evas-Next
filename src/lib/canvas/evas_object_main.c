#include "evas_common.h"
#include "evas_private.h"

EVAS_MEMPOOL(_mp_obj);
EVAS_MEMPOOL(_mp_sh);

static Eina_Inlist *
get_layer_objects(Evas_Layer *l)
{
   if ((!l) || (!l->objects)) return NULL;
   return (EINA_INLIST_GET(l->objects));
}

/* evas internal stuff */
Evas_Object *
evas_object_new(Evas *e __UNUSED__)
{
   Evas_Object *obj;

   EVAS_MEMPOOL_INIT(_mp_obj, "evas_object", Evas_Object, 512, NULL);
   obj = EVAS_MEMPOOL_ALLOC(_mp_obj, Evas_Object);
   if (!obj) return NULL;
   EVAS_MEMPOOL_PREP(_mp_obj, obj, Evas_Object);
   obj->magic = MAGIC_OBJ;
   obj->cur.scale = 1.0;
   obj->prev.scale = 1.0;
   return obj;
}

void
evas_object_free(Evas_Object *obj, int clean_layer)
{
   int was_smart_child = 0;

   evas_object_map_set(obj, NULL);
   evas_object_grabs_cleanup(obj);
   evas_object_intercept_cleanup(obj);
   if (obj->smart.parent) was_smart_child = 1;
   evas_object_smart_cleanup(obj);
   obj->func->free(obj);
   if (!was_smart_child) evas_object_release(obj, clean_layer);
   if (obj->clip.clipees)
     eina_list_free(obj->clip.clipees);
   evas_object_clip_changes_clean(obj);
   evas_object_event_callback_all_del(obj);
   evas_object_event_callback_cleanup(obj);
   while (obj->data.elements)
     {
	Evas_Data_Node *node;

	node = obj->data.elements->data;
	obj->data.elements = eina_list_remove(obj->data.elements, node);
	free(node);
     }
   obj->magic = 0;
   if (obj->size_hints)
     {
       EVAS_MEMPOOL_FREE(_mp_sh, obj->size_hints);
     }
   EVAS_MEMPOOL_FREE(_mp_obj, obj);
}

void
evas_object_change(Evas_Object *obj)
{
   Eina_List *l;
   Evas_Object *obj2;

   if (obj->layer->evas->nochange)
     {
//        printf("nochange %p\n", obj);
        return;
     }
//   else
//      printf("ch %p\n", obj);
   obj->layer->evas->changed = 1;
   if (obj->changed) return;
//   obj->changed = 1;
   evas_render_object_recalc(obj);
   /* set changed flag on all objects this one clips too */
   EINA_LIST_FOREACH(obj->clip.clipees, l, obj2) evas_object_change(obj2);
   if (obj->smart.parent) evas_object_change(obj->smart.parent);
}

void
evas_object_render_pre_visible_change(Eina_Array *rects, Evas_Object *obj, int is_v, int was_v)
{
   if (obj->smart.smart) return ;
   if (is_v == was_v) return ;
   if (is_v)
     {
	evas_add_rect(rects,
		      obj->cur.cache.clip.x,
		      obj->cur.cache.clip.y,
		      obj->cur.cache.clip.w,
		      obj->cur.cache.clip.h);
     }
   else
     {
	evas_add_rect(rects,
		      obj->prev.cache.clip.x,
		      obj->prev.cache.clip.y,
		      obj->prev.cache.clip.w,
		      obj->prev.cache.clip.h);
     }
}

void
evas_object_render_pre_clipper_change(Eina_Array *rects, Evas_Object *obj)
{
   if (obj->smart.smart) return ;
   if (obj->cur.clipper == obj->prev.clipper) return ;
   if ((obj->cur.clipper) && (obj->prev.clipper))
     {
	/* get difference rects between clippers */
	evas_rects_return_difference_rects(rects,
					   obj->cur.clipper->cur.cache.clip.x,
					   obj->cur.clipper->cur.cache.clip.y,
					   obj->cur.clipper->cur.cache.clip.w,
					   obj->cur.clipper->cur.cache.clip.h,
					   obj->prev.clipper->prev.cache.clip.x,
					   obj->prev.clipper->prev.cache.clip.y,
					   obj->prev.clipper->prev.cache.clip.w,
					   obj->prev.clipper->prev.cache.clip.h);
     }
   else if (obj->cur.clipper)
     {
	evas_rects_return_difference_rects(rects,
					   obj->cur.geometry.x,
					   obj->cur.geometry.y,
					   obj->cur.geometry.w,
					   obj->cur.geometry.h,
////	rl = evas_rects_return_difference_rects(obj->cur.cache.geometry.x,
////						obj->cur.cache.geometry.y,
////						obj->cur.cache.geometry.w,
////						obj->cur.cache.geometry.h,
					   obj->cur.clipper->cur.cache.clip.x,
					   obj->cur.clipper->cur.cache.clip.y,
					   obj->cur.clipper->cur.cache.clip.w,
					   obj->cur.clipper->cur.cache.clip.h);
     }
   else if (obj->prev.clipper)
     {
	evas_rects_return_difference_rects(rects,
					   obj->prev.geometry.x,
					   obj->prev.geometry.y,
					   obj->prev.geometry.w,
					   obj->prev.geometry.h,
////	rl = evas_rects_return_difference_rects(obj->prev.cache.geometry.x,
////						obj->prev.cache.geometry.y,
////						obj->prev.cache.geometry.w,
////						obj->prev.cache.geometry.h,
					   obj->prev.clipper->prev.cache.clip.x,
					   obj->prev.clipper->prev.cache.clip.y,
					   obj->prev.clipper->prev.cache.clip.w,
					   obj->prev.clipper->prev.cache.clip.h);
     }
}

void
evas_object_render_pre_prev_cur_add(Eina_Array *rects, Evas_Object *obj)
{
   evas_add_rect(rects,
                 obj->cur.cache.clip.x,
                 obj->cur.cache.clip.y,
                 obj->cur.cache.clip.w,
                 obj->cur.cache.clip.h);
   evas_add_rect(rects,
                 obj->prev.cache.clip.x,
                 obj->prev.cache.clip.y,
                 obj->prev.cache.clip.w,
                 obj->prev.cache.clip.h);
/*        
        evas_add_rect(rects,
                      obj->cur.geometry.x,
                      obj->cur.geometry.y,
                      obj->cur.geometry.w,
                      obj->cur.geometry.h);
////	    obj->cur.cache.geometry.x,
////	    obj->cur.cache.geometry.y,
////	    obj->cur.cache.geometry.w,
////	    obj->cur.cache.geometry.h);
        evas_add_rect(rects,
                      obj->prev.geometry.x,
                      obj->prev.geometry.y,
                      obj->prev.geometry.w,
                      obj->prev.geometry.h);
////	    obj->prev.cache.geometry.x,
////	    obj->prev.cache.geometry.y,
////	    obj->prev.cache.geometry.w,
////	    obj->prev.cache.geometry.h);
*/
}

void
evas_object_clip_changes_clean(Evas_Object *obj)
{
   Eina_Rectangle *r;

   EINA_LIST_FREE(obj->clip.changes, r) eina_rectangle_free(r);
}

void
evas_object_render_pre_effect_updates(Eina_Array *rects, Evas_Object *obj, int is_v, int was_v)
{
   Eina_Rectangle *r;
   Evas_Object *clipper;
   Eina_List *l;
   unsigned int i;
   Eina_Array_Iterator it;
   int x, y, w, h;

   if (obj->smart.smart) goto end;
   /* FIXME: was_v isn't used... why? */
   was_v = 0;
   if (!obj->clip.clipees)
     {
	EINA_ARRAY_ITER_NEXT(rects, i, r, it)
	  {
	     /* get updates and clip to current clip */
	     x = r->x;
	     y = r->y;
	     w = r->w;
	     h = r->h;
	     RECTS_CLIP_TO_RECT(x, y, w, h,
				obj->cur.cache.clip.x,
				obj->cur.cache.clip.y,
				obj->cur.cache.clip.w,
				obj->cur.cache.clip.h);
	     if ((w > 0) && (h > 0))
	       obj->layer->evas->engine.func->output_redraws_rect_add(obj->layer->evas->engine.data.output,
								      x, y, w, h);
	     /* get updates and clip to previous clip */
	     x = r->x;
	     y = r->y;
	     w = r->w;
	     h = r->h;
	     RECTS_CLIP_TO_RECT(x, y, w, h,
				obj->prev.cache.clip.x,
				obj->prev.cache.clip.y,
				obj->prev.cache.clip.w,
				obj->prev.cache.clip.h);
	     if ((w > 0) && (h > 0))
	       obj->layer->evas->engine.func->output_redraws_rect_add(obj->layer->evas->engine.data.output,
								      x, y, w, h);
	  }
	/* if the object is actually visible, take any parent clip changes */
	if (is_v)
	  {
	     clipper = obj->cur.clipper;
	     while (clipper)
	       {
		  EINA_LIST_FOREACH(clipper->clip.changes, l, r)
		    {
		       /* get updates and clip to current clip */
		       x = r->x; y = r->y; w = r->w; h = r->h;
		       RECTS_CLIP_TO_RECT(x, y, w, h,
					  obj->cur.cache.clip.x,
					  obj->cur.cache.clip.y,
					  obj->cur.cache.clip.w,
					  obj->cur.cache.clip.h);
		       if ((w > 0) && (h > 0))
			 obj->layer->evas->engine.func->output_redraws_rect_add(obj->layer->evas->engine.data.output,
										x, y, w, h);
		       /* get updates and clip to previous clip */
		       x = r->x; y = r->y; w = r->w; h = r->h;
		       RECTS_CLIP_TO_RECT(x, y, w, h,
					  obj->prev.cache.clip.x,
					  obj->prev.cache.clip.y,
					  obj->prev.cache.clip.w,
					  obj->prev.cache.clip.h);
		       if ((w > 0) && (h > 0))
			 obj->layer->evas->engine.func->output_redraws_rect_add(obj->layer->evas->engine.data.output,
										x, y, w, h);
		    }
		  clipper = clipper->cur.clipper;
	       }
	  }
     }
   else
     {
	evas_object_clip_changes_clean(obj);
	EINA_ARRAY_ITER_NEXT(rects, i, r, it)
	  obj->clip.changes = eina_list_append(obj->clip.changes, r);
	eina_array_clean(rects);
     }

 end:
   EINA_ARRAY_ITER_NEXT(rects, i, r, it)
     eina_rectangle_free(r);
   eina_array_clean(rects);
}

int
evas_object_was_in_output_rect(Evas_Object *obj, int x, int y, int w, int h)
{
   if (obj->smart.smart) return 0;
   /* assumes coords have been recalced */
   if ((RECTS_INTERSECT(x, y, w, h,
			obj->prev.cache.clip.x,
			obj->prev.cache.clip.y,
			obj->prev.cache.clip.w,
			obj->prev.cache.clip.h)))
     return 1;
   return 0;
}

int
evas_object_was_opaque(Evas_Object *obj)
{
   if (obj->smart.smart) return 0;
   if (obj->prev.cache.clip.a == 255)
     {
	if (obj->func->was_opaque)
	  return obj->func->was_opaque(obj);
	return 1;
     }
   return 0;
}

int
evas_object_is_inside(Evas_Object *obj, Evas_Coord x, Evas_Coord y)
{
   if (obj->smart.smart) return 0;
   if (obj->func->is_inside)
     return obj->func->is_inside(obj, x, y);
   return 0;
}

int
evas_object_was_inside(Evas_Object *obj, Evas_Coord x, Evas_Coord y)
{
   if (obj->smart.smart) return 0;
   if (obj->func->was_inside)
     return obj->func->was_inside(obj, x, y);
   return 0;
}
/* routines apps will call */

/**
 * Deletes the given evas object and frees its memory.
 *
 * The object's 'free' callback is called when this function is called.
 * If the object currently has the focus, its 'focus out' callback is
 * also called.
 *
 * @param   obj The given evas object.
 * @ingroup Evas_Object_Group_Basic
 */
EAPI void
evas_object_del(Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();

   if (obj->delete_me) return;

#ifdef EVAS_FRAME_QUEUING
   evas_common_frameq_flush();
#endif

   _evas_object_event_new();

   evas_object_event_callback_call(obj, EVAS_CALLBACK_DEL, NULL);
   _evas_post_event_callback_call(obj->layer->evas);
   if (obj->name) evas_object_name_set(obj, NULL);
   if (!obj->layer)
     {
	evas_object_free(obj, 1);
	return;
     }
   if (obj->focused)
     {
	obj->focused = 0;
	obj->layer->evas->focused = NULL;
	_evas_object_event_new();
	evas_object_event_callback_call(obj, EVAS_CALLBACK_FOCUS_OUT, NULL);
        _evas_post_event_callback_call(obj->layer->evas);
     }
   obj->layer->evas->pointer.mouse_grabbed -= obj->mouse_grabbed;
   obj->mouse_grabbed = 0;
   obj->mouse_in = 0;
   evas_object_hide(obj);
   evas_object_grabs_cleanup(obj);
   while (obj->clip.clipees) evas_object_clip_unset(obj->clip.clipees->data);
   while (obj->proxy.proxies)
	   evas_object_image_source_unset(obj->proxy.proxies->data);
   if (obj->cur.clipper) evas_object_clip_unset(obj);
   if (obj->smart.smart) evas_object_smart_del(obj);
   evas_object_map_set(obj, NULL);
   _evas_object_event_new();
   evas_object_event_callback_call(obj, EVAS_CALLBACK_FREE, NULL);
   _evas_post_event_callback_call(obj->layer->evas);
   evas_object_smart_cleanup(obj);
   obj->delete_me = 1;
   evas_object_change(obj);
}

/**
 * Moves the given evas object to the given location.
 * @param   obj The given evas object.
 * @param   x   X position to move the object to, in canvas units.
 * @param   y   Y position to move the object to, in canvas units.
 * @ingroup Evas_Object_Group_Basic
 */
EAPI void
evas_object_move(Evas_Object *obj, Evas_Coord x, Evas_Coord y)
{
   int is, was = 0, pass = 0;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (obj->delete_me) return;
   if (evas_object_intercept_call_move(obj, x, y)) return;
   if (obj->doing.in_move > 0)
     {
        WRN("evas_object_move() called on object %p when in the middle of moving the same object", obj);
        return;
     }
   if ((obj->cur.geometry.x == x) && (obj->cur.geometry.y == y)) return;
   if (obj->layer->evas->events_frozen <= 0)
     {
	pass = evas_event_passes_through(obj);
	if (!pass)
	  was = evas_object_is_in_output_rect(obj,
					      obj->layer->evas->pointer.x,
					      obj->layer->evas->pointer.y, 1, 1);
     }
   obj->doing.in_move++;
   if (obj->smart.smart)
     {
       if (obj->smart.smart->smart_class->move)
	  obj->smart.smart->smart_class->move(obj, x, y);
     }
   obj->cur.geometry.x = x;
   obj->cur.geometry.y = y;
////   obj->cur.cache.geometry.validity = 0;
   evas_object_change(obj);
   evas_object_clip_dirty(obj);
   obj->doing.in_move--;
   if (obj->layer->evas->events_frozen <= 0)
     {
	evas_object_recalc_clippees(obj);
	if (!pass)
	  {
	     if (!obj->smart.smart)
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
     }
   evas_object_inform_call_move(obj);
}

/**
 * Changes the size of the given evas object.
 * @param   obj The given evas object.
 * @param   w   The new width of the evas object.
 * @param   h   The new height of the evas object.
 *
 * @note Be aware that resizing an object changes its drawing area,
 *       but that does imply the object is rescaled! For instance,
 *       images are filled inside their drawing area using the
 *       specifications of evas_object_image_fill_set(), thus to scale
 *       the image to match exactly your drawing area, you need to
 *       change the evas_object_image_fill_set() as well. Consider the
 *       following example:
 *       @code
 *       // rescale image to fill exactly its area without tiling:
 *       evas_object_resize(img, w, h);
 *       evas_object_image_fill_set(img, 0, 0, w, h);
 *       @endcode
 *       This is more evident in images, but text, textblock, lines
 *       and polygons will behave similarly. Check their specific APIs
 *       to know how to achieve your desired behavior.
 *
 * @ingroup Evas_Object_Group_Basic
 */
EAPI void
evas_object_resize(Evas_Object *obj, Evas_Coord w, Evas_Coord h)
{
   int is, was = 0, pass = 0;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (obj->delete_me) return;
   if (w < 0) w = 0; if (h < 0) h = 0;
   if (evas_object_intercept_call_resize(obj, w, h)) return;
   if (obj->doing.in_resize > 0)
     {
        WRN("evas_object_resize() called on object %p when in the middle of resizing the same object", obj);
        return;
     }
   if ((obj->cur.geometry.w == w) && (obj->cur.geometry.h == h)) return;
   if (obj->layer->evas->events_frozen <= 0)
     {
	pass = evas_event_passes_through(obj);
	if (!pass)
	  was = evas_object_is_in_output_rect(obj,
					      obj->layer->evas->pointer.x,
					      obj->layer->evas->pointer.y, 1, 1);
     }
   obj->doing.in_resize++;
   if (obj->smart.smart)
     {
       if (obj->smart.smart->smart_class->resize)
	  obj->smart.smart->smart_class->resize(obj, w, h);
     }
   obj->cur.geometry.w = w;
   obj->cur.geometry.h = h;
////   obj->cur.cache.geometry.validity = 0;
   evas_object_change(obj);
   evas_object_clip_dirty(obj);
   obj->doing.in_resize--;
   /* NB: evas_object_recalc_clippees was here previously ( < 08/07/2009) */
   if (obj->layer->evas->events_frozen <= 0)
     {
        /* NB: If this creates glitches on screen then move to above position */
        evas_object_recalc_clippees(obj);

	//   if (obj->func->coords_recalc) obj->func->coords_recalc(obj);
	if (!pass)
	  {
	     if (!obj->smart.smart)
	       {
		  is = evas_object_is_in_output_rect(obj,
						     obj->layer->evas->pointer.x,
						     obj->layer->evas->pointer.y, 1, 1);
		  if ((is ^ was) && (obj->cur.visible))
		    evas_event_feed_mouse_move(obj->layer->evas,
					       obj->layer->evas->pointer.x,
					       obj->layer->evas->pointer.y,
					       obj->layer->evas->last_timestamp,
					       NULL);
	       }
	  }
     }
   evas_object_inform_call_resize(obj);
}

/**
 * Retrieves the position and rectangular size of the given evas object.
 *
 * Note that if any of @p x, @p y, @p w or @p h are @c NULL, the @c NULL
 * parameters are ignored.
 *
 * @param obj The given evas object.
 * @param   x   Pointer to an integer in which to store the X coordinate of
 *              the object.
 * @param   y   Pointer to an integer in which to store the Y coordinate of
 *              the object.
 * @param   w   Pointer to an integer in which to store the width of the
 *              object.
 * @param   h   Pointer to an integer in which to store the height of the
 *              object.
 * @ingroup Evas_Object_Group_Basic
 */
EAPI void
evas_object_geometry_get(const Evas_Object *obj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   if (x) *x = 0; if (y) *y = 0; if (w) *w = 0; if (h) *h = 0;
   return;
   MAGIC_CHECK_END();
   if (obj->delete_me)
     {
	if (x) *x = 0; if (y) *y = 0; if (w) *w = 0; if (h) *h = 0;
	return;
     }
   if (x) *x = obj->cur.geometry.x;
   if (y) *y = obj->cur.geometry.y;
   if (w) *w = obj->cur.geometry.w;
   if (h) *h = obj->cur.geometry.h;
}

/**
 * @addtogroup Evas_Object_Group_Size_Hints
 * @{
 */

static void
_evas_object_size_hint_alloc(Evas_Object *obj)
{
   if (obj->size_hints) return;

   EVAS_MEMPOOL_INIT(_mp_sh, "evas_size_hints", Evas_Size_Hints, 512, );
   obj->size_hints = EVAS_MEMPOOL_ALLOC(_mp_sh, Evas_Size_Hints);
   if (!obj->size_hints) return;
   EVAS_MEMPOOL_PREP(_mp_sh, obj->size_hints, Evas_Size_Hints);
   obj->size_hints->max.w = -1;
   obj->size_hints->max.h = -1;
   obj->size_hints->align.x = 0.5;
   obj->size_hints->align.y = 0.5;
}

/**
 * Retrieves the size hint for the minimum size.
 *
 * This is not a size enforcement in any way, it's just a hint that should
 * be used whenever appropriate.
 *
 * Note that if any of @p w or @p h are @c NULL, the @c NULL
 * parameters are ignored.
 *
 * @param obj The given evas object.
 * @param   w Pointer to an integer in which to store the minimum width.
 * @param   h Pointer to an integer in which to store the minimum height.
 */
EAPI void
evas_object_size_hint_min_get(const Evas_Object *obj, Evas_Coord *w, Evas_Coord *h)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   if (w) *w = 0; if (h) *h = 0;
   return;
   MAGIC_CHECK_END();
   if ((!obj->size_hints) || obj->delete_me)
     {
	if (w) *w = 0; if (h) *h = 0;
	return;
     }
   if (w) *w = obj->size_hints->min.w;
   if (h) *h = obj->size_hints->min.h;
}

/**
 * Sets the size hint for the minimum size.
 *
 * This is not a size enforcement in any way, it's just a hint that should
 * be used whenever appropriate.
 *
 * Value 0 is considered unset.
 *
 * @param obj The given evas object.
 * @param   w Integer to use as the minimum width hint.
 * @param   h Integer to use as the minimum height hint.
 */
EAPI void
evas_object_size_hint_min_set(Evas_Object *obj, Evas_Coord w, Evas_Coord h)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (obj->delete_me)
     return;
   _evas_object_size_hint_alloc(obj);
   if ((obj->size_hints->min.w == w) && (obj->size_hints->min.h == h)) return;
   obj->size_hints->min.w = w;
   obj->size_hints->min.h = h;

   evas_object_inform_call_changed_size_hints(obj);
}

/**
 * Retrieves the size hint for the maximum size.
 *
 * This is not a size enforcement in any way, it's just a hint that should
 * be used whenever appropriate.
 *
 * Note that if any of @p w or @p h are @c NULL, the @c NULL
 * parameters are ignored.
 *
 * @param obj The given evas object.
 * @param   w Pointer to an integer in which to store the maximum width.
 * @param   h Pointer to an integer in which to store the maximum height.
 */
EAPI void
evas_object_size_hint_max_get(const Evas_Object *obj, Evas_Coord *w, Evas_Coord *h)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   if (w) *w = -1; if (h) *h = -1;
   return;
   MAGIC_CHECK_END();
   if ((!obj->size_hints) || obj->delete_me)
     {
	if (w) *w = -1; if (h) *h = -1;
	return;
     }
   if (w) *w = obj->size_hints->max.w;
   if (h) *h = obj->size_hints->max.h;
}

/**
 * Sets the size hint for the maximum size.
 *
 * This is not a size enforcement in any way, it's just a hint that should
 * be used whenever appropriate.
 *
 * Value -1 is considered unset.
 *
 * @param obj The given evas object.
 * @param   w Integer to use as the maximum width hint.
 * @param   h Integer to use as the maximum height hint.
 */
EAPI void
evas_object_size_hint_max_set(Evas_Object *obj, Evas_Coord w, Evas_Coord h)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (obj->delete_me)
     return;
   _evas_object_size_hint_alloc(obj);
   if ((obj->size_hints->max.w == w) && (obj->size_hints->max.h == h)) return;
   obj->size_hints->max.w = w;
   obj->size_hints->max.h = h;

   evas_object_inform_call_changed_size_hints(obj);
}

/**
 * Retrieves the size request hint.
 *
 * This is not a size enforcement in any way, it's just a hint that should
 * be used whenever appropriate.
 *
 * Note that if any of @p w or @p h are @c NULL, the @c NULL
 * parameters are ignored.
 *
 * @param obj The given evas object.
 * @param   w Pointer to an integer in which to store the requested width.
 * @param   h Pointer to an integer in which to store the requested height.
 */
EAPI void
evas_object_size_hint_request_get(const Evas_Object *obj, Evas_Coord *w, Evas_Coord *h)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   if (w) *w = 0; if (h) *h = 0;
   return;
   MAGIC_CHECK_END();
   if ((!obj->size_hints) || obj->delete_me)
     {
	if (w) *w = 0; if (h) *h = 0;
	return;
     }
   if (w) *w = obj->size_hints->request.w;
   if (h) *h = obj->size_hints->request.h;
}

/**
 * Sets the requested size hint.
 *
 * This is not a size enforcement in any way, it's just a hint that should
 * be used whenever appropriate.
 *
 * Value 0 is considered unset.
 *
 * @param obj The given evas object.
 * @param   w Integer to use as the preferred width hint.
 * @param   h Integer to use as the preferred height hint.
 */
EAPI void
evas_object_size_hint_request_set(Evas_Object *obj, Evas_Coord w, Evas_Coord h)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (obj->delete_me)
     return;
   _evas_object_size_hint_alloc(obj);
   if ((obj->size_hints->request.w == w) && (obj->size_hints->request.h == h)) return;
   obj->size_hints->request.w = w;
   obj->size_hints->request.h = h;

   evas_object_inform_call_changed_size_hints(obj);
}

/**
 * Retrieves the size aspect control hint.
 *
 * This is not a size enforcement in any way, it's just a hint that should
 * be used whenever appropriate.
 *
 * Note that if any of @p aspect, @p w or @p h are @c NULL, the @c NULL
 * parameters are ignored.
 *
 * @param    obj The given evas object.
 * @param aspect Returns the hint on how size should be calculated.
 * @param      w Pointer to an integer in which to store the aspect width.
 * @param      h Pointer to an integer in which to store the aspect height.
 */
EAPI void
evas_object_size_hint_aspect_get(const Evas_Object *obj, Evas_Aspect_Control *aspect, Evas_Coord *w, Evas_Coord *h)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   if (aspect) *aspect = EVAS_ASPECT_CONTROL_NONE;
   if (w) *w = 0; if (h) *h = 0;
   return;
   MAGIC_CHECK_END();
   if ((!obj->size_hints) || obj->delete_me)
     {
	if (aspect) *aspect = EVAS_ASPECT_CONTROL_NONE;
	if (w) *w = 0; if (h) *h = 0;
	return;
     }
   if (aspect) *aspect = obj->size_hints->aspect.mode;
   if (w) *w = obj->size_hints->aspect.size.w;
   if (h) *h = obj->size_hints->aspect.size.h;
}

/**
 * Sets the size aspect control hint.
 *
 * This is not a size enforcement in any way, it's just a hint that should
 * be used whenever appropriate.
 *
 * @param    obj The given evas object.
 * @param aspect Hint on how to calculate size.
 * @param      w Integer to use as aspect width hint.
 * @param      h Integer to use as aspect height hint.
 */
EAPI void
evas_object_size_hint_aspect_set(Evas_Object *obj, Evas_Aspect_Control aspect, Evas_Coord w, Evas_Coord h)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (obj->delete_me)
     return;
   _evas_object_size_hint_alloc(obj);
   if ((obj->size_hints->aspect.mode == aspect) && (obj->size_hints->aspect.size.w == w) && (obj->size_hints->aspect.size.h == h)) return;
   obj->size_hints->aspect.mode = aspect;
   obj->size_hints->aspect.size.w = w;
   obj->size_hints->aspect.size.h = h;

   evas_object_inform_call_changed_size_hints(obj);
}

/**
 * Retrieves the size align control hint.
 *
 * This is not a size enforcement in any way, it's just a hint that should
 * be used whenever appropriate.
 *
 * Note that if any of @p x or @p y are @c NULL, the @c NULL
 * parameters are ignored.
 *
 * @param    obj The given evas object.
 * @param      x Pointer to a double in which to store the align x.
 * @param      y Pointer to a double in which to store the align y.
 */
EAPI void
evas_object_size_hint_align_get(const Evas_Object *obj, double *x, double *y)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   if (x) *x = 0.5; if (y) *y = 0.5;
   return;
   MAGIC_CHECK_END();
   if ((!obj->size_hints) || obj->delete_me)
     {
	if (x) *x = 0.5; if (y) *y = 0.5;
	return;
     }
   if (x) *x = obj->size_hints->align.x;
   if (y) *y = obj->size_hints->align.y;
}

/**
 * Sets the size align control hint.
 *
 * This is not a size enforcement in any way, it's just a hint that should
 * be used whenever appropriate.
 *
 * Accepted values are in the 0.0 to 1.0 range, with the special value
 * -1.0 used to specify "justify" or "fill" by some users. See
 * documentation of possible users.
 *
 * @param    obj The given evas object.
 * @param      x Double (0.0..1.0 or -1.0) to use as align x hint.
 * @param      y Double (0.0..1.0 or -1.0) to use as align y hint.
 */
EAPI void
evas_object_size_hint_align_set(Evas_Object *obj, double x, double y)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (obj->delete_me)
     return;
   _evas_object_size_hint_alloc(obj);
   if ((obj->size_hints->align.x == x) && (obj->size_hints->align.y == y)) return;
   obj->size_hints->align.x = x;
   obj->size_hints->align.y = y;

   evas_object_inform_call_changed_size_hints(obj);
}

/**
 * Retrieves the size weight control hint.
 *
 * This is not a size enforcement in any way, it's just a hint that should
 * be used whenever appropriate.
 *
 * Note that if any of @p x or @p y are @c NULL, the @c NULL
 * parameters are ignored.
 *
 * Accepted values are zero or positive values. Some users might use
 * this hint as a boolean, but some might consider it as a proportion,
 * see documentation of possible users.
 *
 * @param    obj The given evas object.
 * @param      x Pointer to a double in which to store the weight x.
 * @param      y Pointer to a double in which to store the weight y.
 */
EAPI void
evas_object_size_hint_weight_get(const Evas_Object *obj, double *x, double *y)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   if (x) *x = 0.0; if (y) *y = 0.0;
   return;
   MAGIC_CHECK_END();
   if ((!obj->size_hints) || obj->delete_me)
     {
	if (x) *x = 0.0; if (y) *y = 0.0;
	return;
     }
   if (x) *x = obj->size_hints->weight.x;
   if (y) *y = obj->size_hints->weight.y;
}

/**
 * Sets the size weight control hint.
 *
 * This is not a size enforcement in any way, it's just a hint that should
 * be used whenever appropriate.
 *
 * @param    obj The given evas object.
 * @param      x Double (0.0-1.0) to use as weight x hint.
 * @param      y Double (0.0-1.0) to use as weight y hint.
 */
EAPI void
evas_object_size_hint_weight_set(Evas_Object *obj, double x, double y)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (obj->delete_me)
     return;
   _evas_object_size_hint_alloc(obj);
   if ((obj->size_hints->weight.x == x) && (obj->size_hints->weight.y == y)) return;
   obj->size_hints->weight.x = x;
   obj->size_hints->weight.y = y;

   evas_object_inform_call_changed_size_hints(obj);
}

/**
 * Retrieves the size padding control hint.
 *
 * This is not a size enforcement in any way, it's just a hint that should
 * be used whenever appropriate.
 *
 * Note that if any of @p l, @p r, @p t or @p b are @c NULL, the @c
 * NULL parameters are ignored.
 *
 * @param    obj The given evas object.
 * @param      l Pointer to an integer in which to store left padding.
 * @param      r Pointer to an integer in which to store right padding.
 * @param      t Pointer to an integer in which to store top padding.
 * @param      b Pointer to an integer in which to store bottom padding.
 */
EAPI void
evas_object_size_hint_padding_get(const Evas_Object *obj, Evas_Coord *l, Evas_Coord *r, Evas_Coord *t, Evas_Coord *b)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   if (l) *l = 0; if (r) *r = 0;
   if (t) *t = 0; if (b) *b = 0;
   return;
   MAGIC_CHECK_END();
   if ((!obj->size_hints) || obj->delete_me)
     {
	if (l) *l = 0; if (r) *r = 0;
	if (t) *t = 0; if (b) *b = 0;
	return;
     }
   if (l) *l = obj->size_hints->padding.l;
   if (r) *r = obj->size_hints->padding.r;
   if (t) *t = obj->size_hints->padding.t;
   if (b) *b = obj->size_hints->padding.b;
}

/**
 * Sets the size padding control hint.
 *
 * This is not a size enforcement in any way, it's just a hint that should
 * be used whenever appropriate.
 *
 * @param    obj The given evas object.
 * @param      l Integer to specify left padding.
 * @param      r Integer to specify right padding.
 * @param      t Integer to specify top padding.
 * @param      b Integer to specify bottom padding.
 */
EAPI void
evas_object_size_hint_padding_set(Evas_Object *obj, Evas_Coord l, Evas_Coord r, Evas_Coord t, Evas_Coord b)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (obj->delete_me)
     return;
   _evas_object_size_hint_alloc(obj);
   if ((obj->size_hints->padding.l == l) && (obj->size_hints->padding.r == r) && (obj->size_hints->padding.t == t) && (obj->size_hints->padding.b == b)) return;
   obj->size_hints->padding.l = l;
   obj->size_hints->padding.r = r;
   obj->size_hints->padding.t = t;
   obj->size_hints->padding.b = b;

   evas_object_inform_call_changed_size_hints(obj);
}


/**
 * @}
 */

/**
 * Makes the given evas object visible.
 * @param   obj The given evas object.
 * @ingroup Evas_Object_Group_Basic
 */
EAPI void
evas_object_show(Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (obj->delete_me) return;
   if (evas_object_intercept_call_show(obj)) return;
   if (obj->smart.smart)
     {
       if (obj->smart.smart->smart_class->show)
	  obj->smart.smart->smart_class->show(obj);
     }
   if (obj->cur.visible)
     {
	return;
     }
   obj->cur.visible = 1;
   evas_object_change(obj);
   evas_object_clip_dirty(obj);
   if (obj->layer->evas->events_frozen <= 0)
     {
        evas_object_clip_across_clippees_check(obj);
	evas_object_recalc_clippees(obj);
	if (!evas_event_passes_through(obj))
	  {
	     if (!obj->smart.smart)
	       {
		  if (evas_object_is_in_output_rect(obj,
						    obj->layer->evas->pointer.x,
						    obj->layer->evas->pointer.y, 1, 1))
		    evas_event_feed_mouse_move(obj->layer->evas,
					       obj->layer->evas->pointer.x,
					       obj->layer->evas->pointer.y,
					       obj->layer->evas->last_timestamp,
					       NULL);
	       }
	  }
     }
   evas_object_inform_call_show(obj);
}

/**
 * Makes the given evas object invisible.
 * @param   obj The given evas object.
 *
 * @note the hidden objects will not be checked for changes and will
 *       not catch events. That is, they are much ligher than an
 *       object that is invisible due indirect effects, such as
 *       clipped or out-of-viewport.
 *
 * @ingroup Evas_Object_Group_Basic
 */
EAPI void
evas_object_hide(Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (obj->delete_me) return;
   if (evas_object_intercept_call_hide(obj)) return;
   if (obj->smart.smart)
     {
       if (obj->smart.smart->smart_class->hide)
	  obj->smart.smart->smart_class->hide(obj);
     }
   if (!obj->cur.visible)
     {
	return;
     }
   obj->cur.visible = 0;
   evas_object_change(obj);
   evas_object_clip_dirty(obj);
   if (obj->layer->evas->events_frozen <= 0)
     {
        evas_object_clip_across_clippees_check(obj);
	evas_object_recalc_clippees(obj);
	if (!evas_event_passes_through(obj))
	  {
	     if ((!obj->smart.smart) ||
                 ((obj->cur.map) && (obj->cur.map->count == 4) && (obj->cur.usemap)))
	       {
		  if (evas_object_is_in_output_rect(obj,
						    obj->layer->evas->pointer.x,
						    obj->layer->evas->pointer.y, 1, 1))
		    evas_event_feed_mouse_move(obj->layer->evas,
					       obj->layer->evas->pointer.x,
					       obj->layer->evas->pointer.y,
					       obj->layer->evas->last_timestamp,
					       NULL);
		  if (obj->delete_me) return;
		  if (obj->mouse_grabbed > 0)
		    {
//		       if (obj->layer->evas->pointer.mouse_grabbed >= obj->mouse_grabbed)
			 obj->layer->evas->pointer.mouse_grabbed -= obj->mouse_grabbed;
		    }
                  if ((obj->mouse_in) || (obj->mouse_grabbed > 0))
                    {
                       obj->layer->evas->pointer.object.in = eina_list_remove(obj->layer->evas->pointer.object.in, obj);
                    }
                  obj->mouse_grabbed = 0;
                  if (obj->layer->evas->events_frozen > 0)
                    {
                       obj->mouse_in = 0;
                       return;
                    }
                  if (obj->mouse_in)
                    {
                       Evas_Event_Mouse_Out ev;

		       _evas_object_event_new();

                       obj->mouse_in = 0;
                       ev.buttons = obj->layer->evas->pointer.button;
                       ev.output.x = obj->layer->evas->pointer.x;
                       ev.output.y = obj->layer->evas->pointer.y;
                       ev.canvas.x = obj->layer->evas->pointer.x;
                       ev.canvas.y = obj->layer->evas->pointer.y;
                       ev.data = NULL;
                       ev.modifiers = &(obj->layer->evas->modifiers);
                       ev.locks = &(obj->layer->evas->locks);
		       ev.timestamp = obj->layer->evas->last_timestamp;
		       ev.event_flags = EVAS_EVENT_FLAG_NONE;
                       evas_object_event_callback_call(obj, EVAS_CALLBACK_MOUSE_OUT, &ev);
                       _evas_post_event_callback_call(obj->layer->evas);
		    }
	       }
	  }
     }
   else
     {
	if ((obj->mouse_in) || (obj->mouse_grabbed > 0))
	  obj->layer->evas->pointer.object.in = eina_list_remove(obj->layer->evas->pointer.object.in, obj);
	obj->mouse_grabbed = 0;
	obj->mouse_in = 0;
     }
   evas_object_inform_call_hide(obj);
}

/**
 * Retrieves whether or not the given evas object is visible.
 * @param   obj The given evas object.
 * @return  @c 1 if the object is visible.  @c 0 otherwise.
 * @ingroup Evas_Object_Group_Basic
 */
EAPI Eina_Bool
evas_object_visible_get(const Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   if (obj->delete_me) return 0;
   return obj->cur.visible;
}

/**
 * Sets the general colour of the given evas object to the given colour.
 * @param obj The given evas object.
 * @param r   The red component of the given colour.
 * @param g   The green component of the given colour.
 * @param b   The blue component of the given colour.
 * @param a   The alpha component of the given colour.
 * @ingroup Evas_Object_Group_Basic
 */
EAPI void
evas_object_color_set(Evas_Object *obj, int r, int g, int b, int a)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (obj->delete_me) return;
   if (r > 255) r = 255; if (r < 0) r = 0;
   if (g > 255) g = 255; if (g < 0) g = 0;
   if (b > 255) b = 255; if (b < 0) b = 0;
   if (a > 255) a = 255; if (a < 0) a = 0;
   if (evas_object_intercept_call_color_set(obj, r, g, b, a)) return;
   if (obj->smart.smart)
     {
       if (obj->smart.smart->smart_class->color_set)
	  obj->smart.smart->smart_class->color_set(obj, r, g, b, a);
     }
   if ((obj->cur.color.r == r) &&
       (obj->cur.color.g == g) &&
       (obj->cur.color.b == b) &&
       (obj->cur.color.a == a)) return;
   obj->cur.color.r = r;
   obj->cur.color.g = g;
   obj->cur.color.b = b;
   evas_object_clip_dirty(obj);
   if ((obj->cur.color.a == 0) && (a == 0) && (obj->cur.render_op == EVAS_RENDER_BLEND)) return;
   obj->cur.color.a = a;
   evas_object_change(obj);
}

/**
 * Retrieves the general colour of the given evas object.
 *
 * Note that if any of @p r, @p g, @p b or @p a are @c NULL, then the
 * @c NULL parameters are ignored.
 *
 * @param   obj The given evas object.
 * @param   r   Pointer to an integer in which to store the red component of
 *              the colour.
 * @param   g   Pointer to an integer in which to store the green component of
 *              the colour.
 * @param   b   Pointer to an integer in which to store the blue component of
 *              the colour.
 * @param   a   Pointer to an integer in which to store the alpha component of
 *              the colour.
 * @ingroup Evas_Object_Group_Basic
 */
EAPI void
evas_object_color_get(const Evas_Object *obj, int *r, int *g, int *b, int *a)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   if (r) *r = 0; if (g) *g = 0; if (b) *b = 0; if (a) *a = 0;
   return;
   MAGIC_CHECK_END();
   if (obj->delete_me)
     {
	if (r) *r = 0; if (g) *g = 0; if (b) *b = 0; if (a) *a = 0;
	return;
     }
   if (r) *r = obj->cur.color.r;
   if (g) *g = obj->cur.color.g;
   if (b) *b = obj->cur.color.b;
   if (a) *a = obj->cur.color.a;
}

/**
 * Sets whether or not the given evas object is to be drawn anti-aliased.
 *
 * @param   obj The given evas object.
 * @param   anti_alias 1 if the object is to be anti_aliased, 0 otherwise.
 * @ingroup Evas_Object_Group_Extras
 */
EAPI void
evas_object_anti_alias_set(Evas_Object *obj, Eina_Bool anti_alias)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (obj->delete_me) return;
   anti_alias = !!anti_alias;
   if (obj->cur.anti_alias == anti_alias)return;
   obj->cur.anti_alias = anti_alias;
   evas_object_change(obj);
}


/**
 * Retrieves whether or not the given evas object is to be drawn anti_aliased.
 * @param   obj The given evas object.
 * @return  @c 1 if the object is to be anti_aliased.  @c 0 otherwise.
 * @ingroup Evas_Object_Group_Extras
 */
EAPI Eina_Bool
evas_object_anti_alias_get(const Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   if (obj->delete_me) return 0;
   return obj->cur.anti_alias;
}

/**
 * Sets the scaling factor for an evas object. Does not affect all objects.
 *
 * @param   obj The given evas object.
 * @param   scale The scaling factor. 1.0 == none.
 * @ingroup Evas_Object_Group_Extras
 */
EAPI void
evas_object_scale_set(Evas_Object *obj, double scale)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (obj->delete_me) return;
   if (obj->cur.scale == scale) return;
   obj->cur.scale = scale;
   evas_object_change(obj);
   if (obj->func->scale_update) obj->func->scale_update(obj);
}


/**
 * Retrieves the scaling factor for the given evas object.
 * @param   obj The given evas object.
 * @return  The scaling factor.
 *
 * @ingroup Evas_Object_Group_Extras
 */
EAPI double
evas_object_scale_get(const Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   if (obj->delete_me) return 1.0;
   return obj->cur.scale;
}

/**
 * Sets the render_op to be used for rendering the evas object.
 * @param   obj The given evas object.
 * @param   render_op one of the Evas_Render_Op values.
 * @ingroup Evas_Object_Group_Extras
 */
EAPI void
evas_object_render_op_set(Evas_Object *obj, Evas_Render_Op render_op)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (obj->delete_me) return;
   if ((Evas_Render_Op)obj->cur.render_op == render_op)
      return;
   obj->cur.render_op = render_op;
   evas_object_change(obj);
}

/**
 * Retrieves the current value of the operation used for rendering the evas object.
 * @param   obj The given evas object.
 * @return  one of the enumerated values in Evas_Render_Op.
 * @ingroup Evas_Object_Group_Extras
 */
EAPI Evas_Render_Op
evas_object_render_op_get(const Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   if (obj->delete_me) return EVAS_RENDER_BLEND;
   return obj->cur.render_op;
}

/**
 * Retrieves the evas that the given evas object is on.
 * @param   obj The given evas object.
 * @return  The evas that the object is on.
 * @ingroup Evas_Object_Group_Basic
 */
EAPI Evas *
evas_object_evas_get(const Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   if (obj->delete_me) return NULL;
   return obj->layer->evas;
}

/**
 * @addtogroup Evas_Object_Group_Find
 * @{
 */

/**
 * Retrieves the top object at the given position (x,y)
 * @param   e The given evas object.
 * @param   x The horizontal coordinate
 * @param   y The vertical coordinate
 * @param   include_pass_events_objects Boolean Flag to include or not
 * pass events objects
 * @param   include_hidden_objects Boolean Flag to include or not hidden objects
 * @return  The evas object that is over all others objects at the given position.
 */
EAPI Evas_Object *
evas_object_top_at_xy_get(const Evas *e, Evas_Coord x, Evas_Coord y, Eina_Bool include_pass_events_objects, Eina_Bool include_hidden_objects)
{
   Evas_Layer *lay;
   int xx, yy;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   xx = x;
   yy = y;
////   xx = evas_coord_world_x_to_screen(e, x);
////   yy = evas_coord_world_y_to_screen(e, y);
   EINA_INLIST_REVERSE_FOREACH((EINA_INLIST_GET(e->layers)), lay)
     {
	Evas_Object *obj;

	EINA_INLIST_REVERSE_FOREACH(get_layer_objects(lay), obj)
	  {
	     if (obj->delete_me) continue;
	     if ((!include_pass_events_objects) && (evas_event_passes_through(obj))) continue;
	     if ((!include_hidden_objects) && (!obj->cur.visible)) continue;
	     evas_object_clip_recalc(obj);
	     if ((evas_object_is_in_output_rect(obj, xx, yy, 1, 1)) &&
		 (!obj->clip.clipees))
	       return obj;
	  }
     }
   return NULL;
}

/**
 * Retrieves the top object at mouse pointer position
 * @param   e The given evas object.
 * @return The evas object that is over all others objects at the
 * pointer position.
 */
EAPI Evas_Object *
evas_object_top_at_pointer_get(const Evas *e)
{
////   return evas_object_top_at_xy_get(e, e->pointer.canvas_x, e->pointer.canvas_y, 0, 0);
   return evas_object_top_at_xy_get(e, e->pointer.x, e->pointer.y, 1, 1);
}

/**
 * Retrieves the top object in the given rectangle region
 * @param   e The given evas object.
 * @param   x The horizontal coordinate.
 * @param   y The vertical coordinate.
 * @param   w The width size.
 * @param   h The height size.
 * @param   include_pass_events_objects Boolean Flag to include or not pass events objects
 * @param   include_hidden_objects Boolean Flag to include or not hidden objects
 * @return  The evas object that is over all others objects at the pointer position.
 *
 */
EAPI Evas_Object *
evas_object_top_in_rectangle_get(const Evas *e, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h, Eina_Bool include_pass_events_objects, Eina_Bool include_hidden_objects)
{
   Evas_Layer *lay;
   int xx, yy, ww, hh;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   xx = x;
   yy = y;
   ww = w;
   hh = h;
////   xx = evas_coord_world_x_to_screen(e, x);
////   yy = evas_coord_world_y_to_screen(e, y);
////   ww = evas_coord_world_x_to_screen(e, w);
////   hh = evas_coord_world_y_to_screen(e, h);
   if (ww < 1) ww = 1;
   if (hh < 1) hh = 1;
   EINA_INLIST_REVERSE_FOREACH((EINA_INLIST_GET(e->layers)), lay)
     {
	Evas_Object *obj;

	EINA_INLIST_REVERSE_FOREACH(get_layer_objects(lay), obj)
	  {
	     if (obj->delete_me) continue;
	     if ((!include_pass_events_objects) && (evas_event_passes_through(obj))) continue;
	     if ((!include_hidden_objects) && (!obj->cur.visible)) continue;
	     evas_object_clip_recalc(obj);
	     if ((evas_object_is_in_output_rect(obj, xx, yy, ww, hh)) &&
		 (!obj->clip.clipees))
	       return obj;
	  }
     }
   return NULL;
}

/**
 * Retrieves the objects at the given position
 * @param   e The given evas object.
 * @param   x The horizontal coordinate.
 * @param   y The vertical coordinate.
 * @param include_pass_events_objects Boolean Flag to include or not
 * pass events objects
 * @param   include_hidden_objects Boolean Flag to include or not hidden objects
 * @return  The list of evas objects at the pointer position.
 *
 */
EAPI Eina_List *
evas_objects_at_xy_get(const Evas *e, Evas_Coord x, Evas_Coord y, Eina_Bool include_pass_events_objects, Eina_Bool include_hidden_objects)
{
   Eina_List *in = NULL;
   Evas_Layer *lay;
   int xx, yy;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   xx = x;
   yy = y;
////   xx = evas_coord_world_x_to_screen(e, x);
////   yy = evas_coord_world_y_to_screen(e, y);
   EINA_INLIST_REVERSE_FOREACH((EINA_INLIST_GET(e->layers)), lay)
     {
	Evas_Object *obj;

	EINA_INLIST_REVERSE_FOREACH(get_layer_objects(lay), obj)
	  {
	     if (obj->delete_me) continue;
	     if ((!include_pass_events_objects) && (evas_event_passes_through(obj))) continue;
	     if ((!include_hidden_objects) && (!obj->cur.visible)) continue;
	     evas_object_clip_recalc(obj);
	     if ((evas_object_is_in_output_rect(obj, xx, yy, 1, 1)) &&
		 (!obj->clip.clipees))
	       in = eina_list_prepend(in, obj);
	  }
     }
   return in;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 */
/**
 * Retrieves the objects in the given rectangle region
 * @param   e The given evas object.
 * @param   x The horizontal coordinate.
 * @param   y The vertical coordinate.
 * @param   w The width size.
 * @param   h The height size.
 * @param   include_pass_events_objects Boolean Flag to include or not pass events objects
 * @param   include_hidden_objects Boolean Flag to include or not hidden objects
 * @return  The list of evas object in the rectangle region.
 *
 */
EAPI Eina_List *
evas_objects_in_rectangle_get(const Evas *e, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h, Eina_Bool include_pass_events_objects, Eina_Bool include_hidden_objects)
{
   Eina_List *in = NULL;
   Evas_Layer *lay;
   int xx, yy, ww, hh;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   xx = x;
   yy = y;
   ww = w;
   hh = h;
////   xx = evas_coord_world_x_to_screen(e, x);
////   yy = evas_coord_world_y_to_screen(e, y);
////   ww = evas_coord_world_x_to_screen(e, w);
////   hh = evas_coord_world_y_to_screen(e, h);
   if (ww < 1) ww = 1;
   if (hh < 1) hh = 1;
   EINA_INLIST_REVERSE_FOREACH((EINA_INLIST_GET(e->layers)), lay)
     {
	Evas_Object *obj;

	EINA_INLIST_REVERSE_FOREACH(get_layer_objects(lay), obj)
	  {
	     if (obj->delete_me) continue;
	     if ((!include_pass_events_objects) && (evas_event_passes_through(obj))) continue;
	     if ((!include_hidden_objects) && (!obj->cur.visible)) continue;
	     evas_object_clip_recalc(obj);
	     if ((evas_object_is_in_output_rect(obj, xx, yy, ww, hh)) &&
		 (!obj->clip.clipees))
	       in = eina_list_prepend(in, obj);
	  }
     }
   return in;
}

/**
 * @}
 */

/**
 * Retrieves the name of the type of the given evas object.
 * @param   obj The given object.
 * @return  The name.
 * @ingroup Evas_Object_Group_Basic
 */
EAPI const char *
evas_object_type_get(const Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   if (obj->delete_me) return "";
   return obj->type;
}

/**
 * Set whether to use a precise (usually expensive) point collision detection.
 * @param obj The given object.
 * @param precise wheter to use a precise point collision detection or not
 * The default value is false.
 * @ingroup Evas_Object_Group_Extras
 */
EAPI void
evas_object_precise_is_inside_set(Evas_Object *obj, Eina_Bool precise)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   obj->precise_is_inside = precise;
}

/**
 * Determine whether an object is set to use a precise point collision
 * detection.
 * @param obj The given object.
 * @ingroup Evas_Object_Group_Extras
 */
EAPI Eina_Bool
evas_object_precise_is_inside_get(const Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   return obj->precise_is_inside;
}

/**
 * Set a hint flag on the object that this is used as a static "clipper".
 * 
 * This is a hint to evas that this object is used as a big static clipper
 * and shouldnt be moved with children and otherwise considered specially. The
 * Default is off.
 * 
 * @param obj The given object.
 * @param is_static_clip The static clip flag (on or off)
 * @ingroup Evas_Object_Group_Extras
 */
EAPI void
evas_object_static_clip_set(Evas_Object *obj, Eina_Bool is_static_clip)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   obj->is_static_clip = is_static_clip;
}

/**
 * Get static "clipper" hint flag.
 * 
 * @see evas_object_static_clip_set()
 * 
 * @param obj The given object.
 * @return The static clip flag (on or off)
 * @ingroup Evas_Object_Group_Extras
 */
EAPI Eina_Bool
evas_object_static_clip_get(const Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   return obj->is_static_clip;
}

