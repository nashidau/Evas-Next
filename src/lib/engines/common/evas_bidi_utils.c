#include <string.h>
#include <stdlib.h>

#include "evas_common.h"
#include "evas_bidi_utils.h"
#include "evas_encoding.h"

#include "evas_font_private.h"

#ifdef BIDI_SUPPORT
#include <fribidi/fribidi.h>
/**
 * @internal
 * @addtogroup Evas_Utils
 *
 * @{
 */
/**
 * @internal
 * @addtogroup Evas_BiDi
 *
 * @{
 */

/**
 * @internal
 * @def _SAFE_FREE(x)
 * checks if x is not NULL, if it's not, it's freed and set to NULL.
 */
#define _SAFE_FREE(x) \
   do {               \
      if (x)          \
        {             \
           free(x);   \
           x = NULL;  \
        }             \
     } while(0)

/* Convert bidichar to eina_unicode assume both are valid pointers */
static Eina_Unicode *
_evas_bidi_fribidichar_to_unicode(Eina_Unicode *dest, const FriBidiChar *src)
{
   Eina_Unicode *ret = dest;

   while (*src)
        *dest++ = *src++;
   *dest = 0;
   return ret;
}

/* Convert eina_unicode to bidi_char assume both are valid pointers */
static FriBidiChar *
_evas_bidi_unicode_to_fribidichar(FriBidiChar *dest, const Eina_Unicode *src)
{
   FriBidiChar *ret = dest;

   while (*src)
        *dest++ = *src++;
   *dest = 0;
   return ret;
}

/**
 * @internal
 * Checks if the string has RTL characters.
 *
 * @param str The string to be checked
 * @return #EINA_TRUE if true, #EINA_FALSE otherwise.
 */
Eina_Bool
evas_bidi_is_rtl_str(const Eina_Unicode *str)
{
   EvasBiDiCharType type;

   if (!str)
      return EINA_FALSE;

   for ( ; *str ; str++)
     {
        type = fribidi_get_bidi_type((FriBidiChar) *str);
        if (FRIBIDI_IS_LETTER(type) && FRIBIDI_IS_RTL(type))
          {
             return EINA_TRUE;
          }
     }
   return EINA_FALSE;
}

/**
 * @internal
 * Shapes the string ustr according to the bidi properties.
 *
 * @param str The string to shape
 * @param bidi_props the bidi props to shaped according.
 * @param len the length of th string.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 */
Eina_Bool
evas_bidi_shape_string(Eina_Unicode *eina_ustr, const Evas_BiDi_Props *bidi_props, size_t len)
{
   FriBidiChar *ustr, *base_ustr = NULL;

   if (!bidi_props->props)
     return EINA_FALSE;

   /* The size of fribidichar is different than eina_unicode, convert */
   /*FIXME: Make this comparison at compile time and compile out
    * unwanted code. - In all of this source file. (including the actual
    * function declerations. */
   if (sizeof(Eina_Unicode) != sizeof(FriBidiChar))
     {
        base_ustr = ustr = calloc(len + 1, sizeof(FriBidiChar));
        ustr = _evas_bidi_unicode_to_fribidichar(ustr, eina_ustr);
     }
   else
     {
        ustr = (FriBidiChar *) eina_ustr;
     }


   EvasBiDiJoiningType *join_types = NULL;
   join_types = (EvasBiDiJoiningType *) malloc(sizeof(EvasBiDiJoiningType) * len);
   if (!join_types)
     {
        return EINA_FALSE;
     }
   fribidi_get_joining_types(ustr, len, join_types);

   fribidi_join_arabic(bidi_props->props->char_types + bidi_props->start, len, bidi_props->props->embedding_levels + bidi_props->start, join_types);


   /* Actually modify the string */
   fribidi_shape(FRIBIDI_FLAGS_DEFAULT | FRIBIDI_FLAGS_ARABIC,
               bidi_props->props->embedding_levels + bidi_props->start, len, join_types, ustr);

   if (join_types) free(join_types);

   /* Convert back */
   if (sizeof(Eina_Unicode) != sizeof(FriBidiChar))
     {
        eina_ustr = _evas_bidi_fribidichar_to_unicode(eina_ustr, ustr);
        if (base_ustr) free(base_ustr);
     }
   return EINA_TRUE;
}

/**
 * @internal
 * Allocates bidi properties according to ustr. First checks to see if the
 * passed has rtl chars, if not, it returns NULL.
 *
 * @param ustr The string to update according to.
 * @return returns allocated paragraph props on success, NULL otherwise.
 */

Evas_BiDi_Paragraph_Props *
evas_bidi_paragraph_props_get(const Eina_Unicode *eina_ustr)
{
   Evas_BiDi_Paragraph_Props *bidi_props = NULL;
   EvasBiDiCharType *char_types = NULL;
   EvasBiDiLevel *embedding_levels = NULL;
   const FriBidiChar *ustr;
   FriBidiChar *base_ustr = NULL;
   size_t len;

   if (!eina_ustr)
      return NULL;



   len = eina_unicode_strlen(eina_ustr);
   /* The size of fribidichar s different than eina_unicode, convert */
   if (sizeof(Eina_Unicode) != sizeof(FriBidiChar))
     {
        base_ustr = calloc(len + 1, sizeof(FriBidiChar));
        base_ustr = _evas_bidi_unicode_to_fribidichar(base_ustr, eina_ustr);
        ustr = base_ustr;
     }
   else
     {
        ustr = (const FriBidiChar *) eina_ustr;
     }


   if (!evas_bidi_is_rtl_str(eina_ustr)) /* No need to handle bidi */
     {
        len = -1;
        goto cleanup;
     }

   bidi_props = evas_bidi_paragraph_props_new();

   /* Prep work for reordering */
   char_types = (EvasBiDiCharType *) malloc(sizeof(EvasBiDiCharType) * len);
   if (!char_types)
      {
         len = -2;
         goto cleanup;
      }
   fribidi_get_bidi_types(ustr, len, char_types);

   embedding_levels = (EvasBiDiLevel *)malloc(sizeof(EvasBiDiLevel) * len);
   if (!embedding_levels)
     {
        len = -2;
        goto cleanup;
     }
   if (!fribidi_get_par_embedding_levels(char_types, len, &bidi_props->direction, embedding_levels))
     {
        len = -2;
        goto cleanup;
     }


   /* clean up */
   if (bidi_props->embedding_levels)
     {
        free(bidi_props->embedding_levels);
     }
   bidi_props->embedding_levels = embedding_levels;

   /* clean up */

   if (bidi_props->char_types)
     {
        free(bidi_props->char_types);
     }
   bidi_props->char_types = char_types;

   if (base_ustr) free(base_ustr);


   return bidi_props;

/* Cleanup */
cleanup:
   if (char_types) free(char_types);
   if (embedding_levels) free(embedding_levels);
   if (base_ustr) free(base_ustr);
   if (bidi_props) evas_bidi_paragraph_props_unref(bidi_props); /* Clean up the bidi props */
   return NULL;
}

/**
 * @internal
 * Copies dst to src and refs (doesn't copy) the paragraph props.
 *
 * @param src the props to copy
 * @param dst the props to copy to.
 */
void
evas_bidi_props_copy_and_ref(const Evas_BiDi_Props *src, Evas_BiDi_Props *dst)
{
   dst->start = src->start;
   dst->props = evas_bidi_paragraph_props_ref(src->props);
}

/**
 * @internal
 * Reorders ustr according to the bidi props.
 *
 * @param ustr the string to reorder. - Null is ok, will just populate the map.
 * @param intl_props the intl properties to rerorder according to.
 * @param _v_to_l The visual to logical map to populate - if NULL it won't populate it.
 * @return #EINA_FALSE on success, #EINA_TRUE on error.
 */
Eina_Bool
evas_bidi_props_reorder_line(Eina_Unicode *eina_ustr, const Evas_BiDi_Props *intl_props, EvasBiDiStrIndex **_v_to_l)
{
   EvasBiDiStrIndex *v_to_l = NULL;
   FriBidiChar *ustr, *base_ustr = NULL;
   size_t len;

   if (!intl_props->props)
     return EINA_FALSE;

   len = eina_unicode_strlen(eina_ustr);
   /* The size of fribidichar is different than eina_unicode, convert */
   if (sizeof(Eina_Unicode) != sizeof(FriBidiChar))
     {
        base_ustr = ustr = calloc(len + 1, sizeof(FriBidiChar));
        ustr = _evas_bidi_unicode_to_fribidichar(ustr, eina_ustr);
     }
   else
     {
        ustr = (FriBidiChar *) eina_ustr;
     }


   if (_v_to_l) {
      size_t i;
      v_to_l = *_v_to_l = calloc(len, sizeof(EvasBiDiStrIndex));
      if (!v_to_l)
        {
           goto error;
        }
      /* init the array for fribidi */
      for (i = 0 ; i < len ; i++)
        {
           v_to_l[i] = i;
        }
   }

   /* Shaping must be done *BEFORE* breaking to lines so there's no choice but
    doing it in textblock. */
   if (!fribidi_reorder_line (FRIBIDI_FLAGS_DEFAULT,
            intl_props->props->char_types + intl_props->start,
            len, 0, intl_props->props->direction,
            intl_props->props->embedding_levels + intl_props->start,
            ustr, v_to_l))
     {
        goto error;
     }


   /* The size of fribidichar is different than eina_unicode, convert */
   if (sizeof(Eina_Unicode) != sizeof(FriBidiChar))
     {
        eina_ustr = _evas_bidi_fribidichar_to_unicode(eina_ustr, base_ustr);
        free(base_ustr);
     }
   return EINA_FALSE;
/* ERROR HANDLING */
error:
   if (base_ustr) free(base_ustr);
   _SAFE_FREE(v_to_l);
   return EINA_TRUE;
}


/**
 * @internal
 * Returns the visual string index from the logical string index.
 *
 * @param v_to_l the visual to logical map
 * @param len the length of the map.
 * @param position the position to convert.
 * @return on success the visual position, on failure the same position.
 */
EvasBiDiStrIndex
evas_bidi_position_logical_to_visual(EvasBiDiStrIndex *v_to_l, int len, EvasBiDiStrIndex position)
{
   int i;
   EvasBiDiStrIndex *ind;
   if (position >= len || !v_to_l)
      return position;

   for (i = 0, ind = v_to_l ; i < len ; i++, ind++)
     {
        if (*ind == position)
          {
             return i;
          }
     }
   return position;
}

/**
 * @internal
 * Checks if the char is rtl oriented. I.e even a neutral char can become rtl
 * if surrounded by rtl chars.
 *
 * @param bidi_props The bidi properties
 * @param index the index of the string.
 * @return #EINA_TRUE if true, #EINA_FALSE otherwise.
 */
Eina_Bool
evas_bidi_is_rtl_char(const Evas_BiDi_Props *bidi_props, EvasBiDiStrIndex index)
{
   if(!bidi_props || !bidi_props->props || index < 0)
      return EINA_FALSE;
   return (FRIBIDI_IS_RTL(
            bidi_props->props->embedding_levels[index + bidi_props->start]))
      ? EINA_TRUE : EINA_FALSE;
}

Evas_BiDi_Paragraph_Props *
evas_bidi_paragraph_props_new(void)
{
   Evas_BiDi_Paragraph_Props *ret;
   ret = calloc(1, sizeof(Evas_BiDi_Paragraph_Props));
   ret->direction = EVAS_BIDI_PARAGRAPH_NATURAL;
   ret->refcount = 1;

   return ret;
}

/**
 * @internal
 * Refs the bidi props.
 *
 * @param bidi_props the props to ref.
 */
Evas_BiDi_Paragraph_Props *
evas_bidi_paragraph_props_ref(Evas_BiDi_Paragraph_Props *bidi_props)
{
   if (!bidi_props) return NULL;
   BIDILOCK();

   bidi_props->refcount++;
   BIDIUNLOCK();
   return bidi_props;
}

/**
 * @internal
 * Unrefs and potentially frees the props.
 *
 * @param bidi_props the properties to unref
 */
void
evas_bidi_paragraph_props_unref(Evas_BiDi_Paragraph_Props *bidi_props)
{
   if (!bidi_props) return;
   BIDILOCK();

   if (--bidi_props->refcount == 0)
     {
        evas_bidi_paragraph_props_clean(bidi_props);
        free(bidi_props);
     }
   BIDIUNLOCK();
}


/**
 * @internal
 * Cleans the paragraph properties.
 *
 * @param bidi_props the properties to clean.
 */
void
evas_bidi_paragraph_props_clean(Evas_BiDi_Paragraph_Props *bidi_props)
{
   _SAFE_FREE(bidi_props->embedding_levels);
   _SAFE_FREE(bidi_props->char_types);
}

/**
 * @internal
 * Cleans the bidi properties.
 *
 * @param bidi_props the properties to clean.
 */
void
evas_bidi_props_clean(Evas_BiDi_Props *bidi_props)
{
   if (!bidi_props) return;
   evas_bidi_paragraph_props_unref(bidi_props->props);
   bidi_props->props = NULL;
}
/**
 * @}
 */
/**
 * @}
 */
#endif

