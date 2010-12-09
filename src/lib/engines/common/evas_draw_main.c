#include "evas_common.h"
#include "evas_convert_main.h"
#include "evas_private.h"

EAPI Cutout_Rects*
evas_common_draw_context_cutouts_new(void)
{
   Cutout_Rects *rects;

   rects = calloc(1, sizeof(Cutout_Rects));
   return rects;
}

EAPI void
evas_common_draw_context_cutouts_free(Cutout_Rects* rects)
{
   rects->active = 0;
}

EAPI void
evas_common_draw_context_cutouts_del(Cutout_Rects* rects,
                                     int index)
{
   if ((index >= 0) && (index < rects->active))
     {
        Cutout_Rect*    rect;

	rect = rects->rects + index;
        memmove(rect, rect + 1,
		sizeof(Cutout_Rect) * (rects->active - index - 1));
        rects->active--;
     }
}

void
evas_common_init(void)
{
   evas_common_cpu_init();

   evas_common_blend_init();
   evas_common_image_init();
   evas_common_convert_init();
   evas_common_scale_init();
   evas_common_rectangle_init();
   evas_common_polygon_init();
   evas_common_line_init();
   evas_common_font_init();
   evas_common_draw_init();
   evas_common_tilebuf_init();
}

void
evas_common_shutdown(void)
{
   evas_font_dir_cache_free();
   evas_common_image_cache_free();
}

EAPI void
evas_common_draw_init(void)
{
}

EAPI RGBA_Draw_Context *
evas_common_draw_context_new(void)
{
   RGBA_Draw_Context *dc;

   dc = calloc(1, sizeof(RGBA_Draw_Context));
   dc->sli.h = 1;
   return dc;
}

EAPI void
evas_common_draw_context_free(RGBA_Draw_Context *dc)
{
   if (!dc) return;

   evas_common_draw_context_apply_clean_cutouts(&dc->cutout);
   free(dc);
}

EAPI void
evas_common_draw_context_clear_cutouts(RGBA_Draw_Context *dc)
{
   evas_common_draw_context_apply_clean_cutouts(&dc->cutout);
}

EAPI void
evas_common_draw_context_font_ext_set(RGBA_Draw_Context *dc,
				      void *data,
				      void *(*gl_new)  (void *data, RGBA_Font_Glyph *fg),
				      void  (*gl_free) (void *ext_dat),
				      void  (*gl_draw) (void *data, void *dest, void *context, RGBA_Font_Glyph *fg, int x, int y))
{
   dc->font_ext.data = data;
   dc->font_ext.func.gl_new = gl_new;
   dc->font_ext.func.gl_free = gl_free;
   dc->font_ext.func.gl_draw = gl_draw;
}

EAPI void
evas_common_draw_context_clip_clip(RGBA_Draw_Context *dc, int x, int y, int w, int h)
{
   if (dc->clip.use)
     {
	RECTS_CLIP_TO_RECT(dc->clip.x, dc->clip.y, dc->clip.w, dc->clip.h,
			   x, y, w, h);
     }
   else
     evas_common_draw_context_set_clip(dc, x, y, w, h);
}

EAPI void
evas_common_draw_context_set_clip(RGBA_Draw_Context *dc, int x, int y, int w, int h)
{
   dc->clip.use = 1;
   dc->clip.x = x;
   dc->clip.y = y;
   dc->clip.w = w;
   dc->clip.h = h;
}

EAPI void
evas_common_draw_context_unset_clip(RGBA_Draw_Context *dc)
{
   dc->clip.use = 0;
}

EAPI void
evas_common_draw_context_set_color(RGBA_Draw_Context *dc, int r, int g, int b, int a)
{
   R_VAL(&(dc->col.col)) = (DATA8)r;
   G_VAL(&(dc->col.col)) = (DATA8)g;
   B_VAL(&(dc->col.col)) = (DATA8)b;
   A_VAL(&(dc->col.col)) = (DATA8)a;
}

EAPI void
evas_common_draw_context_set_multiplier(RGBA_Draw_Context *dc, int r, int g, int b, int a)
{
   dc->mul.use = 1;
   R_VAL(&(dc->mul.col)) = (DATA8)r;
   G_VAL(&(dc->mul.col)) = (DATA8)g;
   B_VAL(&(dc->mul.col)) = (DATA8)b;
   A_VAL(&(dc->mul.col)) = (DATA8)a;
}

EAPI void
evas_common_draw_context_unset_multiplier(RGBA_Draw_Context *dc)
{
   dc->mul.use = 0;
}

EAPI void
evas_common_draw_context_set_mask(RGBA_Draw_Context *dc, RGBA_Image *mask, int x, int y, int w, int h)
{
   dc->mask.mask = mask;
   dc->mask.x = x;
   dc->mask.y = y;
   dc->mask.w = w;
   dc->mask.h = h;
}

EAPI void
evas_common_draw_context_unset_mask(RGBA_Draw_Context *dc)
{
   dc->mask.mask = NULL;
}





EAPI void
evas_common_draw_context_add_cutout(RGBA_Draw_Context *dc, int x, int y, int w, int h)
{
//   if (dc->cutout.rects > 512) return;
   if (dc->clip.use)
     {
#if 1 // this is a bit faster
        int xa1, xa2, xb1, xb2;
        
        xa1 = x;
        xa2 = xa1 + w - 1;
        xb1 = dc->clip.x;
        if (xa2 < xb1) return;
        xb2 = xb1 + dc->clip.w - 1;
        if (xa1 >= xb2) return;
        if (xa2 > xb2) xa2 = xb2; 
        if (xb1 > xa1) xa1 = xb1;
        x = xa1;
        w = xa2 - xa1 + 1;
        
        xa1 = y;
        xa2 = xa1 + h - 1;
        xb1 = dc->clip.y;
        if (xa2 < xb1) return;
        xb2 = xb1 + dc->clip.h - 1; 
        if (xa1 >= xb2) return;
        if (xa2 > xb2) xa2 = xb2; 
        if (xb1 > xa1) xa1 = xb1;
        y = xa1;
        h = xa2 - xa1 + 1;
#else        
        RECTS_CLIP_TO_RECT(x, y, w, h,
			   dc->clip.x, dc->clip.y, dc->clip.w, dc->clip.h);
#endif        
	if ((w < 1) || (h < 1)) return;
     }
   evas_common_draw_context_cutouts_add(&dc->cutout, x, y, w, h);
}

int
evas_common_draw_context_cutout_split(Cutout_Rects* res, int index, Cutout_Rect *split)
{
   /* 1 input rect, multiple out */
   Cutout_Rect  in = res->rects[index];

   /* this is to save me a LOT of typing */
#define INX1 (in.x)
#define INX2 (in.x + in.w)
#define SPX1 (split->x)
#define SPX2 (split->x + split->w)
#define INY1 (in.y)
#define INY2 (in.y + in.h)
#define SPY1 (split->y)
#define SPY2 (split->y + split->h)
#define X1_IN (in.x < split->x)
#define X2_IN ((in.x + in.w) > (split->x + split->w))
#define Y1_IN (in.y < split->y)
#define Y2_IN ((in.y + in.h) > (split->y + split->h))
#define R_NEW(_r, _x, _y, _w, _h) { evas_common_draw_context_cutouts_add(_r, _x, _y, _w, _h); }
   if (!RECTS_INTERSECT(in.x, in.y, in.w, in.h,
			split->x, split->y, split->w, split->h))
     {
        /* No colision => no clipping, don't touch it. */
	return 1;
     }

   /* S    = split (ie cut out rect) */
   /* +--+ = in (rect to be cut) */

   /*
    *  +---+
    *  |   |
    *  | S |
    *  |   |
    *  +---+
    *
    */
   if (X1_IN && X2_IN && Y1_IN && Y2_IN)
     {
        R_NEW(res, in.x, in.y, in.w, SPY1 - in.y);
	R_NEW(res, in.x, SPY1, SPX1 - in.x, SPY2 - SPY1);
	R_NEW(res, SPX2, SPY1, INX2 - SPX2, SPY2 - SPY1);
        /* out => (in.x, SPY2, in.w, INY2 - SPY2) */
        res->rects[index].h = INY2 - SPY2;
        res->rects[index].y = SPY2;
	return 1;
     }
   /* SSSSSSS
    * S+---+S
    * S|SSS|S
    * S|SSS|S
    * S|SSS|S
    * S+---+S
    * SSSSSSS
    */
   if (!X1_IN && !X2_IN && !Y1_IN && !Y2_IN)
     {
        evas_common_draw_context_cutouts_del(res, index);
	return 0;
     }
   /* SSS
    * S+---+
    * S|S  |
    * S|S  |
    * S|S  |
    * S+---+
    * SSS
    */
   if (!X1_IN && X2_IN && !Y1_IN && !Y2_IN)
     {
        /* in => (SPX2, in.y, INX2 - SPX2, in.h) */
        res->rects[index].w = INX2 - SPX2;
        res->rects[index].x = SPX2;
	return 1;
     }
   /*    S
    *  +---+
    *  | S |
    *  | S |
    *  | S |
    *  +---+
    *    S
    */
   if (X1_IN && X2_IN && !Y1_IN && !Y2_IN)
     {
        R_NEW(res, in.x, in.y, SPX1 - in.x, in.h);
        /* in => (SPX2, in.y, INX2 - SPX2, in.h) */
        res->rects[index].w = INX2 - SPX2;
        res->rects[index].x = SPX2;
	return 1;
     }
   /*     SSS
    *  +---+S
    *  |  S|S
    *  |  S|S
    *  |  S|S
    *  +---+S
    *     SSS
    */
   if (X1_IN && !X2_IN && !Y1_IN && !Y2_IN)
     {
        /* in => (in.x, in.y, SPX1 - in.x, in.h) */
        res->rects[index].w = SPX1 - in.x;
	return 1;
     }
   /* SSSSSSS
    * S+---+S
    * S|SSS|S
    *  |   |
    *  |   |
    *  +---+
    *
    */
   if (!X1_IN && !X2_IN && !Y1_IN && Y2_IN)
     {
        /* in => (in.x, SPY2, in.w, INY2 - SPY2) */
        res->rects[index].h = INY2 - SPY2;
        res->rects[index].y = SPY2;
	return 1;
     }
   /*
    *  +---+
    *  |   |
    * S|SSS|S
    *  |   |
    *  +---+
    *
    */
   if (!X1_IN && !X2_IN && Y1_IN && Y2_IN)
     {
        R_NEW(res, in.x, SPY2, in.w, INY2 - SPY2);
        /* in => (in.x, in.y, in.w, SPY1 - in.y) */
        res->rects[index].h = SPY1 - in.y;
	return 1;
     }
   /*
    *  +---+
    *  |   |
    *  |   |
    * S|SSS|S
    * S+---+S
    * SSSSSSS
    */
   if (!X1_IN && !X2_IN && Y1_IN && !Y2_IN)
     {
        /* in => (in.x, in.y, in.w, SPY1 - in.y) */
        res->rects[index].h = SPY1 - in.y;
	return 1;
     }
   /* SSS
    * S+---+
    * S|S  |
    *  |   |
    *  |   |
    *  +---+
    *
    */
   if (!X1_IN && X2_IN && !Y1_IN && Y2_IN)
     {
	R_NEW(res, SPX2, in.y, INX2 - SPX2, SPY2 - in.y);
        /* in => (in.x, SPY2, in.w, INY2 - SPY2) */
        res->rects[index].h = INY2 - SPY2;
        res->rects[index].y = SPY2;
	return 1;
     }
   /*    S
    *  +---+
    *  | S |
    *  |   |
    *  |   |
    *  +---+
    *
    */
   if (X1_IN && X2_IN && !Y1_IN && Y2_IN)
     {
	R_NEW(res, in.x, in.y, SPX1 - in.x, SPY2 - in.y);
	R_NEW(res, SPX2, in.y, INX2 - SPX2, SPY2 - in.y);
        /* in => (in.x, SPY2, in.w, INY2 - SPY2) */
        res->rects[index].h = INY2 - SPY2;
        res->rects[index].y = SPY2;
	return 1;
     }
   /*     SSS
    *  +---+S
    *  |  S|S
    *  |   |
    *  |   |
    *  +---+
    *
    */
   if (X1_IN && !X2_IN && !Y1_IN && Y2_IN)
     {
	R_NEW(res, in.x, in.y, SPX1 - in.x, SPY2 - in.y);
        /* in => (in.x, SPY2, in.w, INY2 - SPY2) */
        res->rects[index].h = INY2 - SPY2;
        res->rects[index].y = SPY2;
	return 1;
     }
   /*
    *  +---+
    *  |   |
    * S|S  |
    *  |   |
    *  +---+
    *
    */
   if (!X1_IN && X2_IN && Y1_IN && Y2_IN)
     {
	R_NEW(res, in.x, SPY2, in.w, INY2 - SPY2);
	R_NEW(res, SPX2, SPY1, INX2 - SPX2, SPY2 - SPY1);
        /* in => (in.x, SPY2, in.w, INY2 - SPY2) */
        res->rects[index].h = SPY1 - in.y;
	return 1;
     }
   /*
    *  +---+
    *  |   |
    *  |  S|S
    *  |   |
    *  +---+
    *
    */
   if (X1_IN && !X2_IN && Y1_IN && Y2_IN)
     {
	R_NEW(res, in.x, SPY2, in.w, INY2 - SPY2);
	R_NEW(res, in.x, SPY1, SPX1 - in.x, SPY2 - SPY1);
        /* in => (in.x, in.y, in.w, SPY1 - in.y) */
        res->rects[index].h = SPY1 - in.y;
	return 1;
     }
   /*
    *  +---+
    *  |   |
    *  |   |
    * S|S  |
    * S+---+
    * SSS
    */
   if (!X1_IN && X2_IN && Y1_IN && !Y2_IN)
     {
        R_NEW(res, SPX2, SPY1, INX2 - SPX2, INY2 - SPY1);
        /* in => (in.x, in.y, in.w, SPY1 - in.y) */
        res->rects[index].h = SPY1 - in.y;
	return 1;
     }
   /*
    *  +---+
    *  |   |
    *  |   |
    *  | S |
    *  +---+
    *    S
    */
   if (X1_IN && X2_IN && Y1_IN && !Y2_IN)
     {
	R_NEW(res, in.x, SPY1, SPX1 - in.x, INY2 - SPY1);
        R_NEW(res, SPX2, SPY1, INX2 - SPX2, INY2 - SPY1);
        /* in => (in.x, in.y, in.w, SPY1 - in.y) */
        res->rects[index].h = SPY1 - in.y;
	return 1;
     }
   /*
    *  +---+
    *  |   |
    *  |   |
    *  |  S|S
    *  +---+S
    *     SSS
    */
   if (X1_IN && !X2_IN && Y1_IN && !Y2_IN)
     {
        R_NEW(res, in.x, SPY1, SPX1 - in.x, INY2 - SPY1);
        /* in => (in.x, in.y, in.w, SPY1 - in.y) */
        res->rects[index].h = SPY1 - in.y;
	return 1;
     }
   evas_common_draw_context_cutouts_del(res, index);
   return 0;
#undef INX1
#undef INX2
#undef SPX1
#undef SPX2
#undef INY1
#undef INY2
#undef SPY1
#undef SPY2
#undef X1_IN
#undef X2_IN
#undef Y1_IN
#undef Y2_IN
#undef R_NEW
}

EAPI Cutout_Rects*
evas_common_draw_context_apply_cutouts(RGBA_Draw_Context *dc)
{
   Cutout_Rects*        res;
   int                  i;
   int                  j;

   if (!dc->clip.use) return NULL;
   if ((dc->clip.w <= 0) || (dc->clip.h <= 0)) return NULL;
  

   res = evas_common_draw_context_cutouts_new();
   evas_common_draw_context_cutouts_add(res, dc->clip.x, dc->clip.y, dc->clip.w, dc->clip.h);

   for (i = 0; i < dc->cutout.active; ++i)
     {
        /* Don't loop on the element just added to the list as they are already correctly clipped. */
        int active = res->active;

        for (j = 0; j < active; )
          {
             if (evas_common_draw_context_cutout_split(res, j, dc->cutout.rects + i))
               ++j;
             else
               active--;
          }
     }
   return res;
}

EAPI void
evas_common_draw_context_apply_clear_cutouts(Cutout_Rects* rects)
{
   evas_common_draw_context_apply_clean_cutouts(rects);
   free(rects);
}

EAPI void
evas_common_draw_context_apply_clean_cutouts(Cutout_Rects* rects)
{
   free(rects->rects);
   rects->rects = NULL;
   rects->active = 0;
   rects->max = 0;
}

EAPI void
evas_common_draw_context_set_anti_alias(RGBA_Draw_Context *dc , unsigned char aa)
{
   dc->anti_alias = !!aa;
}

EAPI void
evas_common_draw_context_set_color_interpolation(RGBA_Draw_Context *dc, int color_space)
{
   dc->interpolation.color_space = color_space;
}

EAPI void
evas_common_draw_context_set_render_op(RGBA_Draw_Context *dc , int op)
{
   dc->render_op = op;
}

EAPI void
evas_common_draw_context_set_sli(RGBA_Draw_Context *dc, int y, int h)
{
   dc->sli.y = y;
   dc->sli.h = h;
}
