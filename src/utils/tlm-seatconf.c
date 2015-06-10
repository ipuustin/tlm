/* vi: set et sw=4 ts=4 cino=t0,(0: */
/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of tlm (Tiny Login Manager)
 *
 * Copyright (C) 2015 Intel Corporation.
 *
 * Contact: Jussi Laako <jussi.laako@linux.intel.com>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <glib.h>


int main (int argc, char *argv[])
{
    int retval = -1;
    int lockfd;
    gchar *cf;
    GKeyFile *kf;
    GError *error = NULL;

    if (argc < 3)
        printf ("%s <add|change|remove> <args...>\n", argv[0]);
    cf = g_build_filename (TLM_SYSCONF_DIR, "tlm.conf", NULL);
    lockfd = open (cf, O_RDWR);
    if (lockfd < 0)
        g_critical ("open(%s): %s", cf, strerror (errno));
    if (lockf (lockfd, F_LOCK, 0))
        g_critical ("lockf(): %s", strerror (errno));
    kf = g_key_file_new ();
    if (!g_key_file_load_from_file (kf, cf,
                                    G_KEY_FILE_KEEP_COMMENTS, &error)) {
        g_warning ("failed to load config file: %s", error->message);
        goto err_exit;
    }

    if (!g_key_file_save_to_file (kf, cf, &error))
        g_warning ("failed to save config file: %s", error->message);
    retval = 0;

err_exit:
    g_key_file_free (kf);
    lockf (lockfd, F_ULOCK, 0);
    close (lockfd);

    return retval;
}

