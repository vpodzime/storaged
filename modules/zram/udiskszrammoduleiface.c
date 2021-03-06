/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*-
 *
 * Copyright (C) 2015 Dominika Hodovska <dhodovsk@redhat.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "config.h"

#include <modules/udisksmoduleiface.h>

#include <udisks/udisks-generated.h>
#include <src/udisksdaemon.h>
#include <src/udiskslinuxblockobject.h>
#include <src/udiskslinuxdevice.h>
#include <src/udiskslogging.h>
#include <src/udisksmodulemanager.h>

#include "udiskszramtypes.h"
#include "udiskszramstate.h"

#include "udiskslinuxblockzram.h"
#include "udiskslinuxmanagerzram.h"

gchar *
udisks_module_id (void)
{
  return g_strdup (ZRAM_MODULE_NAME);
}

gpointer
udisks_module_init (UDisksDaemon *daemon)
{
  return udisks_zram_state_new (daemon);
}

void
udisks_module_teardown (UDisksDaemon *daemon)
{
  UDisksModuleManager *manager = udisks_daemon_get_module_manager (daemon);
  UDisksZRAMState *state_pointer = (UDisksZRAMState *) \
                                    udisks_module_manager_get_module_state_pointer (manager,
                                                                                    ZRAM_MODULE_NAME);

  udisks_zram_state_free (state_pointer);
}

/* ------------------------------------------------------------------------------------ */

static gboolean
zram_block_check (UDisksObject *object)
{
  UDisksLinuxDevice *device = NULL;
  gboolean rval = FALSE;

  g_return_val_if_fail (UDISKS_IS_LINUX_BLOCK_OBJECT (object), FALSE);

  /* Check device name */
  device = udisks_linux_block_object_get_device (UDISKS_LINUX_BLOCK_OBJECT (object));
  rval = g_str_has_prefix (g_udev_device_get_device_file (device->udev_device),
                            "/dev/zram");
  g_object_unref(device);

  return rval;
}

static void
zram_block_connect (UDisksObject *object)
{
}

static gboolean
zram_block_update (UDisksObject   *object,
                   const gchar    *uevent_action,
                   GDBusInterface *_iface)
{
  return udisks_linux_block_zram_update (UDISKS_LINUX_BLOCK_ZRAM (_iface),
                                         UDISKS_LINUX_BLOCK_OBJECT (object));
}

UDisksModuleInterfaceInfo **
udisks_module_get_block_object_iface_setup_entries (void)
{
  UDisksModuleInterfaceInfo **iface;

  iface = g_new0 (UDisksModuleInterfaceInfo *, 2);
  iface[0] = g_new0 (UDisksModuleInterfaceInfo, 1);
  iface[0]->has_func = &zram_block_check;
  iface[0]->connect_func = &zram_block_connect;
  iface[0]->update_func = &zram_block_update;
  iface[0]->skeleton_type = UDISKS_TYPE_LINUX_BLOCK_ZRAM;

  return iface;
}

/* ------------------------------------------------------------------------------------ */

UDisksModuleInterfaceInfo **
udisks_module_get_drive_object_iface_setup_entries (void)
{
  return NULL;
}

UDisksModuleObjectNewFunc *
udisks_module_get_object_new_funcs (void)
{
  return NULL;
}

/* ------------------------------------------------------------------------------------ */

static GDBusInterfaceSkeleton *
new_manager_zram_manager_iface (UDisksDaemon *daemon)
{
  UDisksLinuxManagerZRAM *manager;

  manager = udisks_linux_manager_zram_new (daemon);

  return G_DBUS_INTERFACE_SKELETON (manager);
}

UDisksModuleNewManagerIfaceFunc *
udisks_module_get_new_manager_iface_funcs (void)
{
  UDisksModuleNewManagerIfaceFunc *funcs;

  funcs = g_new0 (UDisksModuleNewManagerIfaceFunc, 2);
  funcs[0] = &new_manager_zram_manager_iface;

  return funcs;
}
