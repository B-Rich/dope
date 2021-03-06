/*
 * \brief   DOpE application manager module
 * \date    2002-11-13
 * \author  Norman FeskE
 *
 * This module is used to manage DOpE-clients.
 * It handles all needed application specific
 * information such as its name, variables etc.
 */

/*
 * Copyright (C) 2002-2007 Norman Feske
 * Copyright (C) 2008-2014 Genode Labs GmbH
 *
 * This file is part of the DOpE package, which is distributed under
 * the terms of the GNU General Public Licence 2.
 */

#include "dopestd.h"
#include "hashtab.h"
#include "appman.h"
#include "screen.h"
#include "scope.h"

enum {
	MAX_APPS    = 64,   /* maximum amount of DOpE clients          */
	APP_NAMELEN = 64,   /* application identifier string length    */
};

static struct hashtab_services *hashtab;

struct app {
	char name[APP_NAMELEN];     /* identifier string of the application    */
	SCOPE *rootscope;         /* root scope of the application           */
	void  *listener;          /* associated result/event listener        */
};


extern SCREEN *curr_scr;

static struct app *apps[MAX_APPS];

int init_appman(struct dope_services *d);


/********************************
 ** Functions for internal use **
 ********************************/

/**
 * Return the first free application id
 */
static s32 get_free_app_id(void)
{
	u32 i;

	for (i=1; i<MAX_APPS; i++) {
		if (!apps[i]) return i;
	}
	INFO(printf("AppMan(get_free_app_id): no free dope application id!\n");)
	return -1;
}


/**
 * Create new application
 */
static struct app *new_app(void)
{
	struct app *app;

	/* get memory for application structure */
	app = (struct app *)zalloc(sizeof(struct app));
	if (!app) {
		INFO(printf("AppMan(new_app): out of memory!\n");)
		return NULL;
	}

	return app;
}


/**
 * Set the identifier string of an application
 */
static void set_app_name(struct app *app, const char *appname)
{
	u32 i;

	if (!app || !appname) return;

	/* copy application name into application data structure */
	for (i=0;i<APP_NAMELEN;i++) {
		app->name[i]=appname[i];
		if (!appname[i]) break;
	}
	app->name[APP_NAMELEN-1]=0;
}


/**
 * Test if an app_id is a valid one
 */
static u16 app_id_valid(u32 app_id)
{
	if (app_id>=MAX_APPS) {
		INFO(printf("AppMan(app_id_value): invalid app_id (out of range)\n");)
		return 0;
	}
	if (!apps[app_id]) {
		INFO(printf("AppMan(unregister): invalid app_id (no application with this id)\n");)
		return 0;
	}
	return 1;
}


/**
 * Return name of application with the specified id
 */
static char const *get_app_name(s32 app_id)
{
	struct app *app;
	if (!app_id_valid(app_id)) return "";
	app = apps[app_id];
	return app->name;
}


/***********************
 ** Service functions **
 ***********************/

/**
 * Register new application
 *
 * app_name: identifier string for the application
 * returns dope_client id
 */
static s32 register_app(const char *app_name)
{
	s32 newapp_id;

	newapp_id = get_free_app_id();
	if (newapp_id < 0) {
		INFO(printf("AppMan(register): application registering failed (no free app id)\n");)
		return -1;
	}

	/* create data structures for the DOpE internal representation of the app */
	apps[newapp_id] = new_app();
	if (!apps[newapp_id]) {
		INFO(printf("AppMan(register): application registering failed.\n");)
		return -1;
	}

	set_app_name(apps[newapp_id], app_name);
	return newapp_id;
}


/**
 * Unregister an application and free all associated internal ressources
 */
static s32 unregister_app(u32 app_id)
{
	struct app *app;
	if (!app_id_valid(app_id)) return -1;

	app = apps[app_id];
	if (!app) return -1;

	curr_scr->gen->lock((WIDGET *)curr_scr);

	/* prevent any events to be delivered anymore */
	app->listener = NULL;

	/* delete root namespace */
	if (app->rootscope) app->rootscope->gen->dec_ref((WIDGET *)app->rootscope);

	/* free the memory for the internal application representation */
	free(app);

	/* mark the corresponding app_id to be free */
	apps[app_id] = NULL;

	curr_scr->gen->unlock((WIDGET *)curr_scr);
	return 0;
}


/**
 * Define root scope of an application
 */
static void set_rootscope(u32 app_id, SCOPE *rootscope)
{
	if (!app_id_valid(app_id) || !rootscope) return;
	apps[app_id]->rootscope = rootscope;
	if (curr_scr) curr_scr->gen->inc_ref((WIDGET *)curr_scr);
	rootscope->scope->set_var(rootscope, "Screen", "screen", 255, (WIDGET *)curr_scr);
}


/**
 * Request root scope of an application
 */
static SCOPE *get_rootscope(u32 app_id)
{
	if (!app_id_valid(app_id)) return NULL;
	return apps[app_id]->rootscope;
}


/**
 * Register event listener destination point of an application
 *
 * The listener is not necessarily a thread. It could be a socket descriptor
 * or something else. We never know what hides behind the listener pointer.
 */
static void reg_listener(s32 app_id, void *listener)
{
	if (!app_id_valid(app_id)) return;
	apps[app_id]->listener = listener;
}


/**
 * Request event listener thread of an application
 */
static void *get_listener(s32 app_id)
{
	if (!app_id_valid(app_id)) return NULL;
	return apps[app_id]->listener;
}


/**
 * Find application if of specified application thread
 */
static s32 app_id_of_thread(void *app_thread)
{
	return -1;
}


/**
 * Resolve application id by its name
 */
static s32 app_id_of_name(char const *app_name)
{
	u32 i;
	for (i=1; i<MAX_APPS; i++) {
		if (apps[i] && streq(app_name, apps[i]->name, 255)) {
			return i;
		}
	}

	return -1;
}


/**
 * Lock application for mutual exclusive modifications
 */
static void lock(s32 app_id)
{
	if (curr_scr) {
		curr_scr->gen->lock((WIDGET *)curr_scr);
	} else {
		printf("AppMan(lock): lock not possible because curr_scr is not defined.\n");
	}
}


/**
 * Unlock application
 */
static void unlock(s32 app_id)
{
	if (curr_scr) curr_scr->gen->unlock((WIDGET *)curr_scr);
}


/**************************************
 ** Service structure of this module **
 **************************************/

static struct appman_services services = {
	register_app,
	unregister_app,
	set_rootscope,
	get_rootscope,
	reg_listener,
	get_listener,
	get_app_name,
	app_id_of_thread,
	app_id_of_name,
	lock,
	unlock,
};


/************************
 ** Module entry point **
 ************************/

int init_appman(struct dope_services *d)
{
	hashtab = (hashtab_services *)(d->get_module("HashTable 1.0"));

	d->register_module("ApplicationManager 1.0", &services);
	return 1;
}
