/*
    device.c -- Interaction with OpenBSD tun device
    Copyright (C) 2001 Ivo Timmermans <itimmermans@bigfoot.com>,
                  2001 Guus Sliepen <guus@sliepen.warande.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    $Id: device.c,v 1.1.2.2 2001/10/12 15:52:03 guus Exp $
*/

#define DEFAULT_DEVICE "/dev/tun0"

#define DEVICE_TYPE_ETHERTAP 0
#define DEVICE_TYPE_TUNTAP 1

int device_fd = -1;
int device_type;
char *device_fname;
char *device_info;

int device_total_in = 0;
int device_total_out = 0;

/*
  open the local ethertap device
*/
int setup_device(void)
{
  if(!get_config_string(lookup_config(config_tree, "Device"), &device_fname)))
    device_fname = DEFAULT_DEVICE;

cp
  if((device_fd = open(device_fname, O_RDWR | O_NONBLOCK)) < 0)
    {
      syslog(LOG_ERR, _("Could not open %s: %m"), device_fname);
      return -1;
    }
cp
  /* Set default MAC address for ethertap devices */

  mymac.type = SUBNET_MAC;
  mymac.net.mac.address.x[0] = 0xfe;
  mymac.net.mac.address.x[1] = 0xfd;
  mymac.net.mac.address.x[2] = 0x00;
  mymac.net.mac.address.x[3] = 0x00;
  mymac.net.mac.address.x[4] = 0x00;
  mymac.net.mac.address.x[5] = 0x00;

  device_info = _("OpenBSD tun device");

  syslog(LOG_INFO, _("%s is a %s"), device_fname, device_info);
cp
  return 0;
}

int read_packet(vpn_packet_t *packet)
{
  int lenin;
  u_int32_t type;
cp
  struct iovec vector[2] = {{&type, sizeof(type)}, {packet->data + 14, MTU - 14}};

  if((lenin = readv(device_fd, vector, 2)) <= 0)
    {
      syslog(LOG_ERR, _("Error while reading from %s %s: %m"), device_info, device_fname);
      return -1;
    }

  memcpy(vp->data, mymac.net.mac.address.x, 6);
  memcpy(vp->data + 6, mymac.net.mac.address.x, 6);
  vp->data[12] = 0x08;
  vp->data[13] = 0x00;

  packet->len = lenin + 10;

  device_total_in += packet->len;

  if(debug_lvl >= DEBUG_TRAFFIC)
    {
      syslog(LOG_DEBUG, _("Read packet of %d bytes from %s"), device_info, packet.len);
    }

  return 0;
cp
}

int write_packet(vpn_packet_t *packet)
{
  u_int32_t type = htonl(AF_INET);
cp
  if(debug_lvl >= DEBUG_TRAFFIC)
    syslog(LOG_DEBUG, _("Writing packet of %d bytes to %s"),
           packet->len, device_info);


  struct iovec vector[2] = {{&type, sizeof(type)}, {packet->data + 14, packet->len - 14}};

  if(writev(device_fd, vector, 2) < 0)
    {
      syslog(LOG_ERR, _("Can't write to %s %s: %m"), device_info, packet.len);
      return -1;
    }

  device_total_out += packet->len;
cp
}
