/*
 * \brief   Interface of relaxation module
 * \date    2004-08-20
 * \author  Norman Feske
 */

/*
 * Copyright (C) 2002-2007 Norman Feske
 * Copyright (C) 2008-2014 Genode Labs GmbH
 *
 * This file is part of the DOpE package, which is distributed under
 * the terms of the GNU General Public Licence 2.
 */

#ifndef _DOPE_RELAX_H_
#define _DOPE_RELAX_H_

#define RELAX struct relax
RELAX {
	float speed;
	float dst;
	float curr;
	float accel;
};

struct relax_services {
	void (*set_duration) (RELAX *, float time);
	int  (*do_relax)     (RELAX *);
};


#endif /* _DOPE_RELAX_H_ */
