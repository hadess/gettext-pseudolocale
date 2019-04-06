/* -*- Mode: C; indent-tabs-mode: t -*- */

/*
 * Copyright 2019 Bastien Nocera
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with the Gnome Library; see the file COPYING.LIB.  If not,
 * write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301  USA.
 *
 * Authors: Bastien Nocera <hadess@hadess.net>
 *
 */

#include <glib.h>

#define __USE_GNU
#include <dlfcn.h>

#include <assert.h>
#include <locale.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "bent.h"

typedef enum {
	MODE_LTR,         /* default */
	MODE_RTL,
	MODE_MALKOVICH
} Mode;

static GHashTable *msg_ht = NULL;
static bool setlocale_inited = false;
static bool textdomain_inited = false;
static Mode mode = MODE_LTR;

static void
pseudolocale_init (void)
{
	const char *mode_str;

	if (msg_ht != NULL)
		return;
	msg_ht = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);

	mode_str = g_getenv ("PSEUDOLOCALE_MODE");
	if (!mode_str)
		return;
	if (strcmp (mode_str, "rtl") == 0)
		mode = MODE_RTL;
	else if (strcmp (mode_str, "ltr") == 0)
		mode = MODE_LTR;
	else if (strcmp (mode_str, "malkovich") == 0)
		mode = MODE_MALKOVICH;
}

/* From https://github.com/combatwombat/Lunicode.js/blob/master/lunicode.js#L697 */
static GHashTable *
bend_init (void)
{
	GHashTable *bend_ht;
	guint i;

	bend_ht = g_hash_table_new (g_direct_hash, g_direct_equal);
	for (i = 0; i < G_N_ELEMENTS(bent); i++) {
		//g_message ("adding %s to %s", bent[i].from, bent[i].to);
		//g_message ("%d to %d", g_utf8_get_char (bent[i].from), g_utf8_get_char (bent[i].to));
		g_hash_table_insert (bend_ht,
				     GINT_TO_POINTER (g_utf8_get_char (bent[i].from)),
				     GINT_TO_POINTER (g_utf8_get_char (bent[i].to)));
	}

	return bend_ht;
}

static char *
bend (const char *msgid)
{
	static GHashTable *bend_ht = NULL;
	GString *s;
	const char *p;

	if (!bend_ht)
		bend_ht = bend_init ();
	s = g_string_new ("[ ");
	for (p = msgid; *p != '\0'; p = g_utf8_next_char (p)) {
		gunichar c, new_c;

		c = g_utf8_get_char (p);
		new_c = GPOINTER_TO_INT (g_hash_table_lookup (bend_ht, GINT_TO_POINTER (c)));
		if (new_c == 0)
			new_c = c;
		g_string_append_unichar (s, new_c);
	}
	g_string_append (s, " ]");

	return g_string_free (s, FALSE);
}

static char *
reverse (const char *msgid)
{
	GString *s;
	const char *p;

	s = g_string_new (NULL);
	for (p = msgid; *p != '\0'; p = g_utf8_next_char (p)) {
		gunichar c;

		c = g_utf8_get_char (p);
		/* Arabic Left Mark (Unicode U+061C) */
		g_string_append (s, "\xd8\x9c");
		g_string_append_unichar (s, c);
	}
	return g_string_free (s, FALSE);
}

static char *
malkovich (const char *__msgid)
{
	char *res;
	char *new_msgid = NULL;

	//g_message ("msgid: %s", __msgid);

	assert(textdomain_inited);
	assert(setlocale_inited);
	pseudolocale_init();

	res = g_hash_table_lookup (msg_ht, __msgid);
	if (res)
		return res;

	if (strcmp (__msgid, "default:LTR") == 0) {
		new_msgid = (mode == MODE_RTL) ? g_strdup ("default:RTL") : g_strdup (__msgid);
	} else if (mode == MODE_LTR) {
		new_msgid = bend (__msgid);
	} else if (mode == MODE_RTL) {
		new_msgid = reverse (__msgid);
	} else if (mode == MODE_MALKOVICH) {
		new_msgid = g_strdup ("Malkovich");
	}
	assert (new_msgid);
	g_hash_table_insert (msg_ht, g_strdup (__msgid), new_msgid);

	return (char *) new_msgid;
}

char *
gettext (const char *__msgid)
{
	return malkovich(__msgid);
}

char *
dgettext (const char *__domainname, const char *__msgid)
{
	return malkovich(__msgid);
}

char *
__dgettext (const char *__domainname, const char *__msgid)
{
	return malkovich(__msgid);
}

char *
dcgettext (const char *__domainname,
	   const char *__msgid, int __category)
{
	return malkovich(__msgid);
}

char *
__dcgettext (const char *__domainname,
	     const char *__msgid, int __category)
{
	return malkovich(__msgid);
}

char *
ngettext (const char *__msgid1, const char *__msgid2,
	  unsigned long int __n)
{
	char * (*f)() = dlsym(RTLD_NEXT, "ngettext");
	assert(f);
	char *__msgid = f(__msgid1, __msgid2, __n);
	return malkovich(__msgid);
}

char *
dngettext (const char *__domainname, const char *__msgid1,
	   const char *__msgid2, unsigned long int __n)
{
	char * (*f)() = dlsym(RTLD_NEXT, "dngettext");
	assert(f);
	char *__msgid = f(__domainname, __msgid1, __msgid2, __n);
	return malkovich(__msgid);

}

char *
dcngettext (const char *__domainname, const char *__msgid1,
	    const char *__msgid2, unsigned long int __n,
	    int __category)
{
	char * (*f)() = dlsym(RTLD_NEXT, "dcngettext");
	assert(f);
	char *__msgid = f(__domainname, __msgid1, __msgid2, __n, __category);
	return malkovich(__msgid);
}

char *
textdomain (const char *__domainname)
{
	char * (*f)() = dlsym(RTLD_NEXT, "textdomain");
	assert(f);
	textdomain_inited = true;
	return f(__domainname);
}

/* bindtextdomain does not need to be overridden */

char *
bind_textdomain_codeset (const char *__domainname,
			 const char *__codeset)
{
	assert (strcmp (__codeset, "UTF-8") == 0);
	char * (*f)() = dlsym(RTLD_NEXT, "bind_textdomain_codeset");
	assert(f);
	return f(__domainname, __codeset);
}

char *
setlocale (int category, const char *locale)
{
	char * (*f)() = dlsym(RTLD_NEXT, "setlocale");
	assert(f);
	setlocale_inited = true;
	return f(LC_ALL, "en_US.UTF-8");
}
