/* vi: set et sw=4 ts=4 cino=t0,(0: */
/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of tlm (Tiny Login Manager)
 *
 * Copyright (C) 2015 Intel Corporation.
 *
 * Contact: Imran Zaman <imran.zaman@intel.com>
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

#ifndef _TLM_DBUS_LAUNCHER_OBSERVER_H
#define _TLM_DBUS_LAUNCHER_OBSERVER_H

#include <glib-object.h>
#include "common/tlm-config.h"

G_BEGIN_DECLS

#define TLM_TYPE_DBUS_LAUNCHER_OBSERVER       (tlm_dbus_launcher_observer_get_type())
#define TLM_DBUS_LAUNCHER_OBSERVER(obj)       (G_TYPE_CHECK_INSTANCE_CAST((obj), \
            TLM_TYPE_DBUS_LAUNCHER_OBSERVER, TlmDbusLauncherObserver))
#define TLM_IS_DBUS_LAUNCHER_OBSERVER(obj)    (G_TYPE_CHECK_INSTANCE_TYPE((obj), \
            TLM_TYPE_DBUS_LAUNCHER_OBSERVER))
#define TLM_DBUS_LAUNCHER_OBSERVER_CLASS(kls) (G_TYPE_CHECK_CLASS_CAST((kls), \
            TLM_TYPE_DBUS_LAUNCHER_OBSERVER))
#define TLM_DBUS_LAUNCHER_OBSERVER_IS_CLASS(kls)  (G_TYPE_CHECK_CLASS_TYPE((kls), \
            TLM_TYPE_DBUS_LAUNCHER_OBSERVER))

typedef struct _TlmDbusLauncherObserver TlmDbusLauncherObserver;
typedef struct _TlmDbusLauncherObserverClass TlmDbusLauncherObserverClass;
typedef struct _TlmDbusLauncherObserverPrivate TlmDbusLauncherObserverPrivate;

struct _TlmDbusLauncherObserver
{
    GObject parent;
    /* Private */
    TlmDbusLauncherObserverPrivate *priv;
};

struct _TlmDbusLauncherObserverClass
{
    GObjectClass parent_class;
};

GType tlm_dbus_launcher_observer_get_type(void);

TlmDbusLauncherObserver *
tlm_dbus_launcher_observer_new (
		TlmConfig *config,
        const gchar *address,
        uid_t uid);

gboolean
tlm_dbus_launcher_launch_app (
        TlmDbusLauncherObserver *self,
        const gchar *app_abs_path,
        const gchar *args,
        guint *app_id,
        GError **error);

gboolean
tlm_dbus_launcher_stop_app (
        TlmDbusLauncherObserver *self,
        guint app_id,
        GError **error);

G_END_DECLS

#endif /* _TLM_DBUS_LAUNCHER_OBSERVER_H */
