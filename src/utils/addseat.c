/* vi: set et sw=4 ts=4 cino=t0,(0: */
/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of tlm (Tiny Login Manager)
 *
 * Copyright (C) 2015 Intel Corporation.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

#include "config.h"

#include <error.h>
#include <errno.h>
#include <stdlib.h>
#include <gio/gio.h>
#include <glib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <glib-unix.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "common/tlm-log.h"
#include "common/tlm-config.h"
#include "common/tlm-utils.h"


gboolean _handle_update_config(gchar *config_file, gchar *output_file,
        gchar **keys)
{
    GError *err = NULL;
    gchar **groups = NULL;
    gsize n_groups = 0;
    int i;
    GKeyFile *settings = g_key_file_new ();
    gchar *n_seats_val = NULL;
    gint n_seats = 0;
    gint last_seat = -1;
    gchar *new_seat = NULL;

    if (!settings)
        goto error;

    if (!g_key_file_load_from_file (settings,
            config_file,
            G_KEY_FILE_KEEP_COMMENTS, &err)) {

        WARN ("error reading config file at '%s': %s",
                config_file, err->message);
        g_error_free (err);

        goto error;
    }

    groups = g_key_file_get_groups (settings, &n_groups);

    /* find the biggest X in [seatX] groups */

    for (i = 0; i < n_groups; i++) {

        gchar *group = groups[i];

        if (g_str_has_prefix (group, "seat")) {
            gchar *seat_id = group+4;
            int x = g_ascii_strtoll (seat_id, NULL, 10);

            if (x > last_seat) {
                last_seat = x;
            }
        }
    }

    /* create a new seat group */

    new_seat = g_strdup_printf ("seat%d", ++last_seat);

    if (!new_seat) {
        goto error;
    }

    if (keys) {
        gchar **key = keys;

        while (*key) {
            gchar *value = *(key+1);

            g_key_file_set_string (settings, new_seat, *key, value);

            key = key+2;
        }
    }

    /* increment the NSEATS value by 1 */

    n_seats_val = g_key_file_get_value (settings,
            "General", "NSEATS", &err);

    if (err) {
        /* no such key */
        n_seats = 1;
        g_error_free(err);
        err = NULL;
    }
    else {
        n_seats = g_ascii_strtoll(n_seats_val, NULL, 10);
        n_seats++;
    }

    g_key_file_set_integer(settings, "General", "NSEATS", n_seats);

    g_free (n_seats_val);

    if (!g_key_file_save_to_file (settings, output_file, &err)) {
        WARN ("error saving config file to '%s': %s",
                output_file, err->message);
        g_error_free (err);

        goto error;
    }

    g_free(new_seat);
    g_strfreev (groups);
    g_key_file_free (settings);

    return TRUE;

error:
    g_free(new_seat);
    g_strfreev (groups);
    g_key_file_free (settings);

    return FALSE;
}


int main (int argc, char *argv[])
{
    GError *error = NULL;
    GOptionContext *context;
    gboolean rval = FALSE;

    gchar *config_file = NULL;
    gchar *launch_script = NULL;
    gchar *output_file = NULL;
    gchar **keys = NULL;
    gint n_args = 0;

    GOptionEntry main_entries[] =
    {
        { "config-file", 'f', 0, G_OPTION_ARG_STRING, &config_file,
                "configuration file to edit", NULL },
        { "output-file", 'o', 0, G_OPTION_ARG_STRING, &output_file,
                "target configuration file", NULL },
        { G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_STRING_ARRAY, &keys,
                "key-value pairs", "key1 val1 key2 val2" },
        { NULL }
    };

#if !GLIB_CHECK_VERSION (2, 36, 0)
    g_type_init ();
#endif

    context = g_option_context_new (" [addseat Option]\n"
            "[--config-file configuration-file] [--session-script script]");
    g_option_context_add_main_entries (context, main_entries, NULL);

    rval = g_option_context_parse (context, &argc, &argv, &error);
    g_option_context_free (context);
    if (!rval) {
        WARN ("option parsing failed: %s\n", error->message);
        return EXIT_FAILURE;
    }

    if (!config_file)
        config_file = g_strdup("/etc/tlm.conf");

    if (!config_file)
        return EXIT_FAILURE;

    if (!output_file)
        output_file = g_strdup(config_file);

    if (!output_file)
        return EXIT_FAILURE;

    if (keys) {
        gchar **key = keys;

        while (*key) {
            n_args++;
            key = key+1;
        }
    }

    if (n_args % 2 != 0) {
        WARN ("non-even number of tokens in key-value pairs");
        return EXIT_FAILURE;
    }

    rval = _handle_update_config (config_file, output_file, keys);

    if (!rval) {
        DBG ("updating TLM configuration file %s failed\n", config_file);
        return EXIT_FAILURE;
    }

    g_strfreev(keys);
    g_free(output_file);
    g_free(launch_script);
    g_free(config_file);

    return EXIT_SUCCESS;
}
