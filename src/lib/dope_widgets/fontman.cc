/*
 * \brief   DOpE font manager module
 * \date    2002-11-13
 * \author  Norman Feske
 *
 * This component provides a general interface for
 * the usage of fonts.
 */

/*
 * Copyright (C) 2002-2007 Norman Feske
 * Copyright (C) 2008-2014 Genode Labs GmbH
 *
 * This file is part of the DOpE package, which is distributed under
 * the terms of the GNU General Public Licence 2.
 */

#include "dopestd.h"
#include "fontman.h"
#include "fontconv.h"

static struct fontconv_services *conv_fnt;
static struct fontconv_services *conv_tff;

/**
 * Symbols of font data
 */
extern unsigned char default_fnt[];
extern unsigned char mono_fnt[];
extern unsigned char title_fnt[];

extern unsigned char vera16_tff[];

int init_fontman(struct dope_services *d);


/********************************
 ** Functions for internal use **
 ********************************/

/**
 * Determine whether a font id is valid or not
 */
static inline int valid_font_id(int font_id)
{
	return (font_id >= 0 && font_id <= 2);
}


/***********************
 ** Service functions **
 ***********************/

static struct font fonts[3];

static struct font *fontman_get_by_id(s32 font_id)
{
	if (!valid_font_id(font_id)) return NULL;
	return (struct font *)(&fonts[font_id]);
}


static s32 fontman_calc_str_width(s32 font_id, char const *str)
{
	s32 result = 0;
	if (!str) return 0;
	if (!valid_font_id(font_id)) return 0;
	while (*str) {
		result+=fonts[font_id].width_table[(s32)(*str)];
		str++;
	}
	return result;
}


/**
 * Calculate character index of specified pixel position
 */
static s32 fontman_calc_char_idx(s32 font_id, char const *str, s32 pixpos)
{
	s32 idx = 0, pos = 0, charw;
	if (!str) return 0;
	if (!valid_font_id(font_id)) return 0;
	while (*str) {
		charw = fonts[font_id].width_table[(s32)(*str)];
		if (pos >= pixpos - (charw>>1)) return idx;
		pos += charw;
		str++; idx++;
	}
	return idx;
}


static s32 fontman_calc_str_height(s32 font_id, char const *str)
{
	if (!str) return 0;
	if (!valid_font_id(font_id)) return 0;
	return fonts[font_id].img_h;
}


/**************************************
 ** Service structure of this module **
 **************************************/

static struct fontman_services services = {
	fontman_get_by_id,
	fontman_calc_str_width,
	fontman_calc_str_height,
	fontman_calc_char_idx,
};


/************************
 ** Module entry point **
 ************************/

static void add_font(struct fontconv_services *conv, void *fontdata,
                     u32 font_id, struct font *dst)
{
	dst->font_id = font_id;
	dst->img_w   = conv->get_image_width(fontdata);
	dst->img_h   = conv->get_image_height(fontdata);
	dst->top     = conv->get_top(fontdata);
	dst->bottom  = conv->get_bottom(fontdata);
	dst->name    = conv->get_name(fontdata);

	dst->width_table  = (s32 *)zalloc(256*4);
	dst->offset_table = (s32 *)zalloc(256*4);
	dst->image = (u8 *)zalloc(dst->img_w*dst->img_h);
	if (!dst->width_table || !dst->offset_table || !dst->image) {
		INFO(printf("FontManager(add_font): out of memory, smoke a cigarette\n"));
		return;
	}
	conv->gen_width_table(fontdata, dst->width_table);
	conv->gen_offset_table(fontdata, dst->offset_table);
	conv->gen_image(fontdata, dst->image);
}


int init_fontman(struct dope_services *d)
{
	conv_fnt = (fontconv_services *)(d->get_module("ConvertFNT 1.0"));
	conv_tff = (fontconv_services *)(d->get_module("ConvertTFF 1.0"));

	/* init the three built-in fonts */
	if (conv_tff->probe(&vera16_tff)) add_font(conv_tff,&vera16_tff[0],0,&fonts[0]);
//	if (conv_fnt->probe(&default_fnt)) add_font(conv_fnt,&default_fnt[0],0,&fonts[0]);

	if (conv_fnt->probe(&mono_fnt))    add_font(conv_fnt,&mono_fnt[0],   1,&fonts[1]);
	if (conv_fnt->probe(&title_fnt))   add_font(conv_fnt,&title_fnt[0],  2,&fonts[2]);

	d->register_module("FontManager 1.0",&services);
	return 1;
}
