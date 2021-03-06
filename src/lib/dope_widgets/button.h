/*
 * \brief   Interface of DOpE Button widget module
 * \date    2002-11-13
 * \author  Norman Feske
 */

/*
 * Copyright (C) 2002-2007 Norman Feske
 * Copyright (C) 2008-2014 Genode Labs GmbH
 *
 * This file is part of the DOpE package, which is distributed under
 * the terms of the GNU General Public Licence 2.
 */

#ifndef _DOPE_BUTTON_H_
#define _DOPE_BUTTON_H_

struct button_methods;
struct button_data;

#define BUTTON struct button

struct button {
	struct widget_methods *gen;
	struct button_methods *but;
	struct widget_data    *wd;
	struct button_data    *bd;
};

struct button_methods {
	void        (*set_text)    (BUTTON *, char const *new_txt);
	char const *(*get_text)    (BUTTON *);
	void        (*set_font)    (BUTTON *, s32 new_font_id);
	s32         (*get_font)    (BUTTON *);
	void        (*set_style)   (BUTTON *, s32 new_style);
	s32         (*get_style)   (BUTTON *);
	void        (*set_click)   (BUTTON *, void (*)(BUTTON *));
	void        (*set_release) (BUTTON *, void (*)(BUTTON *));
	void        (*set_free_w)  (BUTTON *, int free_w_flag);
	void        (*set_free_h)  (BUTTON *, int free_h_flag);
	void        (*set_pad_x)   (BUTTON *, int);
	void        (*set_pad_y)   (BUTTON *, int);
};

struct button_services {
	BUTTON *(*create) (void);
};


#endif /* _DOPE_BUTTON_H_ */
