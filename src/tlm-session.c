/* vi: set et sw=4 ts=4 cino=t0,(0: */
/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of tlm (Tizen Login Manager)
 *
 * Copyright (C) 2013 Intel Corporation.
 *
 * Contact: Amarnath Valluri <amarnath.valluri@linux.intel.com>
 *          Jussi Laako <jussi.laako@linux.intel.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <grp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "tlm-session.h"
#include "tlm-auth-session.h"
#include "tlm-log.h"
#include "tlm-utils.h"

G_DEFINE_TYPE (TlmSession, tlm_session, G_TYPE_OBJECT);

#define TLM_SESSION_PRIV(obj) \
    G_TYPE_INSTANCE_GET_PRIVATE ((obj), TLM_TYPE_SESSION, TlmSessionPrivate)

enum {
    PROP_0,
    PROP_SERVICE,
    N_PROPERTIES
};
static GParamSpec *pspecs[N_PROPERTIES];

struct _TlmSessionPrivate
{
    gchar *service;
    gchar *username;
    GHashTable *env_hash;
    TlmAuthSession *auth_session;
};

static void
tlm_session_dispose (GObject *self)
{
    TlmSession *session = TLM_SESSION(self);
    DBG("disposing session: %s", session->priv->service);

    g_clear_object (&session->priv->auth_session);
    if (session->priv->env_hash) {
        g_hash_table_unref (session->priv->env_hash);
        session->priv->env_hash = NULL;
    }

    G_OBJECT_CLASS (tlm_session_parent_class)->dispose (self);
}

static void
tlm_session_finalize (GObject *self)
{
    TlmSession *session = TLM_SESSION(self);

    g_clear_string (&session->priv->service);
    g_clear_string (&session->priv->username);

    G_OBJECT_CLASS (tlm_session_parent_class)->finalize (self);
}

static void
_session_set_property (GObject *obj,
                    guint property_id,
                    const GValue *value,
                    GParamSpec *pspec)
{
    TlmSession *session = TLM_SESSION(obj);

    switch (property_id) {
        case PROP_SERVICE: 
          session->priv->service = g_value_dup_string (value);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, property_id, pspec);
    }
}

static void
_session_get_property (GObject *obj,
                    guint property_id,
                    GValue *value,
                    GParamSpec *pspec)
{
    TlmSession *session = TLM_SESSION(obj);

    switch (property_id) {
        case PROP_SERVICE: 
            g_value_set_string (value, session->priv->service);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, property_id, pspec);
    }
}

static void
tlm_session_class_init (TlmSessionClass *klass)
{
    GObjectClass *g_klass = G_OBJECT_CLASS (klass);

    g_type_class_add_private (klass, sizeof(TlmSessionPrivate));

    g_klass->dispose = tlm_session_dispose ;
    g_klass->finalize = tlm_session_finalize;
    g_klass->set_property = _session_set_property;
    g_klass->get_property = _session_get_property;

    pspecs[PROP_SERVICE] = g_param_spec_string ("service", 
                        "authentication service", "Service", NULL, 
                        G_PARAM_READWRITE|G_PARAM_CONSTRUCT_ONLY |
                        G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (g_klass, N_PROPERTIES, pspecs);
}

static void
tlm_session_init (TlmSession *session)
{
    TlmSessionPrivate *priv = TLM_SESSION_PRIV (session);
    
    priv->service = NULL;
    priv->env_hash = g_hash_table_new_full (
                        g_str_hash, g_str_equal, g_free, g_free);
    priv->auth_session = NULL;

    session->priv = priv;
}

gboolean
tlm_session_putenv (TlmSession *session, const gchar *var, const gchar *val)
{
    g_return_val_if_fail (session && TLM_IS_SESSION (session), FALSE);
    if (!session->priv->auth_session) {
        g_hash_table_insert (session->priv->env_hash, 
                                g_strdup (var), g_strdup (val));
        return TRUE;
    }

    return tlm_auth_session_putenv (session->priv->auth_session, var, val);
}

static void
_putenv_to_auth_session (gpointer key, gpointer val, gpointer user_data)
{
    TlmAuthSession *auth_session = TLM_AUTH_SESSION (user_data);

    tlm_auth_session_putenv(auth_session, (const gchar*)key, (const gchar*)val);
}

static void
_session_on_auth_error (
    TlmAuthSession *session, 
    GError *error, 
    gpointer userdata)
{
    WARN ("ERROR : %s", error->message);
}

static void
_session_on_session_error (
    TlmAuthSession *session, 
    GError *error, 
    gpointer userdata)
{
    if (!error)
        WARN ("ERROR but error is NULL");
    else
        WARN ("ERROR : %s", error->message);
}

static gboolean
_set_terminal (TlmSessionPrivate *priv)
{
    int i;
    int tty_fd;
    const char *tty_dev;
    struct stat tty_stat;

    tty_dev = ttyname (0);
    DBG ("trying to setup TTY '%s'", tty_dev);
    if (!tty_dev) {
        WARN ("no TTY");
        return FALSE;
    }
    if (access (tty_dev, R_OK|W_OK)) {
        WARN ("TTY not accessible: %s", strerror(errno));
        return FALSE;
    }
    if (lstat (tty_dev, &tty_stat)) {
        WARN ("lstat() failed: %s", strerror(errno));
        return FALSE;
    }
    if (tty_stat.st_nlink > 1 ||
        !S_ISCHR (tty_stat.st_mode) ||
        strncmp (tty_dev, "/dev/", 5)) {
        WARN ("invalid TTY");
        return FALSE;
    }

    tty_fd = open (tty_dev, O_RDWR | O_NONBLOCK);
    if (tty_fd < 0) {
        WARN ("open() failed: %s", strerror(errno));
        return FALSE;
    }
    if (!isatty(tty_fd)) {
        close (tty_fd);
        WARN ("isatty() failed");
        return FALSE;
    }

    // close all old handles
    for (i = 0; i < tty_fd; i++)
        close (i);
    dup2 (tty_fd, 0);
    dup2 (tty_fd, 1);
    dup2 (tty_fd, 2);
    close (tty_fd);

    return TRUE;
}

static gboolean
_set_environment (TlmSessionPrivate *priv)
{
    setenv ("PATH", "/usr/local/bin:/usr/bin:/bin", 1);
    setenv ("USER", priv->username, 1);
    setenv ("LOGNAME", priv->username, 1);
    setenv ("HOME", tlm_user_get_home_dir (priv->username), 1);
    setenv ("SHELL", tlm_user_get_shell (priv->username), 1);

    return TRUE;
}

static void
_session_on_session_created (
    TlmAuthSession *auth_session,
    const gchar *id,
    gpointer userdata)
{
    int child_status = 0;
    pid_t child_pid;
    const char *shell;
    const char *home;
    TlmSession *session = TLM_SESSION (userdata);
    TlmSessionPrivate *priv = session->priv;

    priv = session->priv;
    if (!priv->username)
        priv->username =
            g_strdup (tlm_auth_session_get_username (auth_session));
    DBG ("session ID : %s", id);

    child_pid = fork ();
    if (child_pid) {
        DBG ("wait for child process %u", child_pid);
        waitpid (child_pid, &child_status, 0);
        DBG ("wait completed for %u", child_pid);
        tlm_auth_session_stop (auth_session, child_status);
        DBG ("session complete");
        return;
    }

    /* this is child process here onwards */

    _set_terminal (priv);

    setsid ();
    DBG (" state:\n\truid=%d, euid=%d, rgid=%d, egid=%d (%s)",
         getuid(), geteuid(), getgid(), getegid(), priv->username);
    uid_t target_uid = tlm_user_get_uid (priv->username);
    gid_t target_gid = tlm_user_get_gid (priv->username);
    if (initgroups (priv->username, target_gid))
        WARN ("initgroups() failed: %s", strerror(errno));
    if (setregid (target_gid, target_gid))
        WARN ("setregid() failed: %s", strerror(errno));
    if (setreuid (target_uid, target_uid))
        WARN ("setreuid() failed: %s", strerror(errno));

    _set_environment (priv);

    shell = getenv("SHELL");
    home = getenv("HOME");
    DBG ("starting %s in %s", shell, home);
    if (shell) {
        chdir (home);
        // based on documentation the terminating NULL should be typecast
        //execl (shell, shell, "-l", (const char *) NULL);
        execlp ("systemd", "systemd", "--user", (const char *) NULL);
        ERR ("execl(): %s", strerror(errno));
    }
}

gboolean
tlm_session_start (TlmSession *session, const gchar *username)
{
    g_return_val_if_fail (session && TLM_IS_SESSION(session), FALSE);

    if (session->priv->username)
        g_free (session->priv->username);
    session->priv->username = g_strdup (username);
    session->priv->auth_session = 
        tlm_auth_session_new (session->priv->service,
                              session->priv->username);

    if (!session->priv->auth_session) return FALSE;

    g_signal_connect (session->priv->auth_session, "auth-error", 
                G_CALLBACK(_session_on_auth_error), (gpointer)session);
    g_signal_connect (session->priv->auth_session, "session-created",
                G_CALLBACK(_session_on_session_created), (gpointer)session);
    g_signal_connect (session->priv->auth_session, "session-error",
                G_CALLBACK (_session_on_session_error), (gpointer)session);

    g_hash_table_foreach (session->priv->env_hash, 
                          _putenv_to_auth_session,
                          session->priv->auth_session);

    return tlm_auth_session_start (session->priv->auth_session);
}

gboolean
tlm_session_stop (TlmSession *session)
{
    g_return_val_if_fail (session && TLM_IS_SESSION(session), FALSE);

    return TRUE;
}

TlmSession *
tlm_session_new (const gchar *service)
{
    return g_object_new (TLM_TYPE_SESSION, "service", service, NULL);
}

