/*
 * PCMCIA 16-bit compatibility functions
 *
 * The initial developer of the original code is David A. Hinds
 * <dahinds@users.sourceforge.net>.  Portions created by David A. Hinds
 * are Copyright (C) 1999 David A. Hinds.  All Rights Reserved.
 *
 * Copyright (C) 2004 Dominik Brodowski
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/config.h>
#include <linux/module.h>
#include <linux/init.h>

#define IN_CARD_SERVICES
#include <pcmcia/version.h>
#include <pcmcia/cs_types.h>
#include <pcmcia/cs.h>
#include <pcmcia/bulkmem.h>
#include <pcmcia/cistpl.h>
#include <pcmcia/ds.h>
#include <pcmcia/ss.h>

#include "cs_internal.h"

int pcmcia_get_first_tuple(client_handle_t handle, tuple_t *tuple)
{
	struct pcmcia_socket *s;
	if (CHECK_HANDLE(handle))
		return CS_BAD_HANDLE;
	s = SOCKET(handle);
	return pccard_get_first_tuple(s, handle->Function, tuple);
}
EXPORT_SYMBOL(pcmcia_get_first_tuple);

int pcmcia_get_next_tuple(client_handle_t handle, tuple_t *tuple)
{
	struct pcmcia_socket *s;
	if (CHECK_HANDLE(handle))
		return CS_BAD_HANDLE;
	s = SOCKET(handle);
	return pccard_get_next_tuple(s, handle->Function, tuple);
}
EXPORT_SYMBOL(pcmcia_get_next_tuple);

int pcmcia_get_tuple_data(client_handle_t handle, tuple_t *tuple)
{
	struct pcmcia_socket *s;
	if (CHECK_HANDLE(handle))
		return CS_BAD_HANDLE;
	s = SOCKET(handle);
	return pccard_get_tuple_data(s, tuple);
}
EXPORT_SYMBOL(pcmcia_get_tuple_data);

int pcmcia_parse_tuple(client_handle_t handle, tuple_t *tuple, cisparse_t *parse)
{
	return pccard_parse_tuple(tuple, parse);
}
EXPORT_SYMBOL(pcmcia_parse_tuple);

int pcmcia_validate_cis(client_handle_t handle, cisinfo_t *info)
{
	struct pcmcia_socket *s;
	if (CHECK_HANDLE(handle))
		return CS_BAD_HANDLE;
	s = SOCKET(handle);
	return pccard_validate_cis(s, handle->Function, info);
}
EXPORT_SYMBOL(pcmcia_validate_cis);

int pcmcia_get_configuration_info(client_handle_t handle,
				  config_info_t *config)
{
	struct pcmcia_socket *s;

	if ((CHECK_HANDLE(handle)) || !config)
		return CS_BAD_HANDLE;
	s = SOCKET(handle);
	if (!s)
		return CS_BAD_HANDLE;
	return pccard_get_configuration_info(s, handle->Function, config);
}
EXPORT_SYMBOL(pcmcia_get_configuration_info);

int pcmcia_reset_card(client_handle_t handle, client_req_t *req)
{
	struct pcmcia_socket *skt;
    
	if (CHECK_HANDLE(handle))
		return CS_BAD_HANDLE;
	skt = SOCKET(handle);
	if (!skt)
		return CS_BAD_HANDLE;

	return pccard_reset_card(skt);
}
EXPORT_SYMBOL(pcmcia_reset_card);

int pcmcia_get_status(client_handle_t handle, cs_status_t *status)
{
	struct pcmcia_socket *s;
	if (CHECK_HANDLE(handle))
		return CS_BAD_HANDLE;
	s = SOCKET(handle);
	return pccard_get_status(s, handle->Function, status);
}
EXPORT_SYMBOL(pcmcia_get_status);

int pcmcia_access_configuration_register(client_handle_t handle,
					 conf_reg_t *reg)
{
	struct pcmcia_socket *s;
	if (CHECK_HANDLE(handle))
		return CS_BAD_HANDLE;
	s = SOCKET(handle);
	return pccard_access_configuration_register(s, handle->Function, reg);
}
EXPORT_SYMBOL(pcmcia_access_configuration_register);

/*** CSM1200 specified **************/
int pcmcia_orion_set_cis_rmode(client_handle_t handle)
{
	struct pcmcia_socket *s;
	if (CHECK_HANDLE(handle))
		return CS_BAD_HANDLE;
	s = SOCKET(handle);
	s->ops->set_cis_rmode();
	return 0;
}
EXPORT_SYMBOL(pcmcia_orion_set_cis_rmode);

int pcmcia_orion_set_cis_wmode(client_handle_t handle)
{
	struct pcmcia_socket *s;
	if (CHECK_HANDLE(handle))
		return CS_BAD_HANDLE;
	s = SOCKET(handle);
	s->ops->set_cis_wmode();
	return 0;
}
EXPORT_SYMBOL(pcmcia_orion_set_cis_wmode);

int pcmcia_orion_set_comm_rmode(client_handle_t handle)
{
	struct pcmcia_socket *s;
	if (CHECK_HANDLE(handle))
		return CS_BAD_HANDLE;
	s = SOCKET(handle);
	s->ops->set_comm_rmode();
	return 0;
}
EXPORT_SYMBOL(pcmcia_orion_set_comm_rmode);

int pcmcia_orion_set_comm_wmode(client_handle_t handle)
{
	struct pcmcia_socket *s;
	if (CHECK_HANDLE(handle))
		return CS_BAD_HANDLE;
	s = SOCKET(handle);
	s->ops->set_comm_wmode();
	return 0;
}
EXPORT_SYMBOL(pcmcia_orion_set_comm_wmode);

int pcmcia_orion_set_io_mode(client_handle_t handle)
{
	struct pcmcia_socket *s;
	if (CHECK_HANDLE(handle))
		return CS_BAD_HANDLE;
	s = SOCKET(handle);
	s->ops->set_io_mode();
	return 0;
}
EXPORT_SYMBOL(pcmcia_orion_set_io_mode);

int pcmcia_orion_read_cis(client_handle_t handle, unsigned char *val, unsigned int addr )
{
	struct pcmcia_socket *s;
	if (CHECK_HANDLE(handle))
		return CS_BAD_HANDLE;
	s = SOCKET(handle);
	*val = s->ops->read_cis(addr);
	return 0;
}
EXPORT_SYMBOL(pcmcia_orion_read_cis);

int pcmcia_orion_write_cis(client_handle_t handle, unsigned char val, unsigned int addr )
{
	struct pcmcia_socket *s;
	if (CHECK_HANDLE(handle))
		return CS_BAD_HANDLE;
	s = SOCKET(handle);
	s->ops->write_cis(val, addr);
	return 0;
}
EXPORT_SYMBOL(pcmcia_orion_write_cis);

int pcmcia_orion_read_comm(client_handle_t handle, unsigned char *val, unsigned int addr )
{
	struct pcmcia_socket *s;
	if (CHECK_HANDLE(handle))
		return CS_BAD_HANDLE;
	s = SOCKET(handle);
	*val = s->ops->read_comm(addr);
	return 0;
}
EXPORT_SYMBOL(pcmcia_orion_read_comm);

int pcmcia_orion_write_comm(client_handle_t handle, unsigned char val, unsigned int addr )
{
	struct pcmcia_socket *s;
	if (CHECK_HANDLE(handle))
		return CS_BAD_HANDLE;
	s = SOCKET(handle);
	s->ops->write_comm(val, addr);
	return 0;
}
EXPORT_SYMBOL(pcmcia_orion_write_comm);

int pcmcia_orion_read_io(client_handle_t handle, unsigned char *val, unsigned int addr )
{
	struct pcmcia_socket *s;
	if (CHECK_HANDLE(handle))
		return CS_BAD_HANDLE;
	s = SOCKET(handle);
	*val = s->ops->read_io(addr);
	return 0;
}
EXPORT_SYMBOL(pcmcia_orion_read_io);

int pcmcia_orion_write_io(client_handle_t handle, unsigned char val, unsigned int addr )
{
	struct pcmcia_socket *s;
	if (CHECK_HANDLE(handle))
		return CS_BAD_HANDLE;
	s = SOCKET(handle);
	s->ops->write_io(val, addr);
	return 0;
}
EXPORT_SYMBOL(pcmcia_orion_write_io);

/*** CSM1200 specified over **************/
