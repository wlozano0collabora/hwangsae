/**
 *  Copyright 2019 SK Telecom Co., Ltd.
 *    Author: Walter Lozano <walter.lozano@collabora.com>
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */

#include "config.h"

#include "recorder-agent.h"
#include <hwangsae/recorder.h>

#include <glib-unix.h>

#include <hwangsae/dbus/manager-generated.h>
#include <hwangsae/dbus/recorder-interface-generated.h>

struct _HwangsaeRecorderAgent
{
  GApplication parent;

  HwangsaeRecorder *recorder;
  Hwangsae1DBusManager *manager;
  Hwangsae1DBusRecorderInterface *recorder_interface;
};

/* *INDENT-OFF* */
G_DEFINE_TYPE (HwangsaeRecorderAgent, hwangsae_recorder_agent, G_TYPE_APPLICATION)
/* *INDENT-ON* */

static gboolean
hwangsae_recorder_agent_dbus_register (GApplication * app,
    GDBusConnection * connection, const gchar * object_path, GError ** error)
{
  gboolean ret;
  HwangsaeRecorderAgent *self = HWANGSAE_RECORDER_AGENT (app);

  g_debug ("hwangsae_recorder_agent_dbus_register");

  g_application_hold (app);

  /* chain up */
  ret =
      G_APPLICATION_CLASS (hwangsae_recorder_agent_parent_class)->dbus_register
      (app, connection, object_path, error);

  if (ret &&
      !g_dbus_interface_skeleton_export (G_DBUS_INTERFACE_SKELETON
          (self->manager), connection,
          "/org/hwangsaeul/Hwangsae1/Manager", error)) {
    g_warning ("Failed to export Hwangsae1 D-Bus interface (reason: %s)",
        (*error)->message);
  }

  if (ret &&
      !g_dbus_interface_skeleton_export (G_DBUS_INTERFACE_SKELETON
          (self->recorder_interface), connection,
          "/org/hwangsaeul/Hwangsae1/RecorderInterface", error)) {
    g_warning ("Failed to export Hwangsae1 D-Bus interface (reason: %s)",
        (*error)->message);
  }

  return ret;
}

static void
hwangsae_recorder_agent_dbus_unregister (GApplication * app,
    GDBusConnection * connection, const gchar * object_path)
{
  HwangsaeRecorderAgent *self = HWANGSAE_RECORDER_AGENT (app);

  g_debug ("hwangsae_recorder_agent_dbus_unregister");

  if (self->manager)
    g_dbus_interface_skeleton_unexport (G_DBUS_INTERFACE_SKELETON
        (self->manager));

  if (self->recorder_interface)
    g_dbus_interface_skeleton_unexport (G_DBUS_INTERFACE_SKELETON
        (self->recorder_interface));

  g_application_release (app);

  /* chain up */
  G_APPLICATION_CLASS (hwangsae_recorder_agent_parent_class)->dbus_unregister
      (app, connection, object_path);
}

gboolean
    hwangsae_recorder_agent_recorder_interface_handle_start
    (Hwangsae1DBusRecorderInterface * object,
    GDBusMethodInvocation * invocation, gchar * arg_id, gpointer user_data) {
  g_autofree gchar *cmd = NULL;
  g_autofree gchar *response = NULL;
  GError *error = NULL;
  gchar *record_id = NULL;

  HwangsaeRecorderAgent *self = (HwangsaeRecorderAgent *) user_data;

  g_debug ("hwangsae_recorder_agent_recorder_interface_handle_start, cmd %s",
      cmd);

  hwangsae1_dbus_recorder_interface_complete_start (object, invocation,
      record_id);

  return TRUE;
}

gboolean
    hwangsae_recorder_agent_recorder_interface_handle_stop
    (Hwangsae1DBusRecorderInterface * object,
    GDBusMethodInvocation * invocation, gchar * arg_id, gpointer user_data) {
  g_autofree gchar *cmd = NULL;
  g_autofree gchar *response = NULL;
  GError *error = NULL;

  HwangsaeRecorderAgent *self = (HwangsaeRecorderAgent *) user_data;

  g_debug ("hwangsae_recorder_agent_recorder_interface_handle_stop, cmd %s",
      cmd);

  hwangsae1_dbus_recorder_interface_complete_stop (object, invocation);

  return TRUE;
}

static void
hwangsae_recorder_agent_dispose (GObject * object)
{
  HwangsaeRecorderAgent *self = HWANGSAE_RECORDER_AGENT (object);

  g_clear_object (&self->recorder);

  g_clear_object (&self->manager);
  g_clear_object (&self->recorder_interface);

  G_OBJECT_CLASS (hwangsae_recorder_agent_parent_class)->dispose (object);
}

static void
hwangsae_recorder_agent_class_init (HwangsaeRecorderAgentClass * klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GApplicationClass *app_class = G_APPLICATION_CLASS (klass);

  gobject_class->dispose = hwangsae_recorder_agent_dispose;

  app_class->dbus_register = hwangsae_recorder_agent_dbus_register;
  app_class->dbus_unregister = hwangsae_recorder_agent_dbus_unregister;
}

static gboolean
signal_handler (GApplication * app)
{
  g_application_release (app);

  return G_SOURCE_REMOVE;
}

static void
hwangsae_recorder_agent_init (HwangsaeRecorderAgent * self)
{
  gchar *uid = NULL;

  self->recorder = hwangsae_recorder_new ();

  self->manager = hwangsae1_dbus_manager_skeleton_new ();

  hwangsae1_dbus_manager_set_status (self->manager, 1);

  self->recorder_interface = hwangsae1_dbus_recorder_interface_skeleton_new ();

  g_signal_connect (self->recorder_interface, "handle-start",
      G_CALLBACK (hwangsae_recorder_agent_recorder_interface_handle_start),
      self);

  g_signal_connect (self->recorder_interface, "handle-stop",
      G_CALLBACK (hwangsae_recorder_agent_recorder_interface_handle_stop),
      self);
}

int
main (int argc, char *argv[])
{
  g_autoptr (GApplication) app = NULL;

  app = G_APPLICATION (g_object_new (HWANGSAE_TYPE_RECORDER_AGENT,
          "application-id", "org.hwangsaeul.Hwangsae1",
          "flags", G_APPLICATION_IS_SERVICE, NULL));

  g_unix_signal_add (SIGINT, (GSourceFunc) signal_handler, app);

  g_application_hold (app);

  return g_application_run (app, argc, argv);
}
