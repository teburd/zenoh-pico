
//
// Copyright (c) 2025 ZettaScale Technology
// Copyright (c) 2025 Intel Corporation
//
// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License 2.0 which is available at
// http://www.eclipse.org/legal/epl-2.0, or the Apache License, Version 2.0
// which is available at https://www.apache.org/licenses/LICENSE-2.0.
//
// SPDX-License-Identifier: EPL-2.0 OR Apache-2.0
//
// Contributors:
//   ZettaScale Zenoh Team, <zenoh@zettascale.tech>
//   Thomas Burdick <thomas.burdick@intel.com>
//

#ifndef ZENOH_PICO_SYSTEM_LINK_MCTP_H
#define ZENOH_PICO_SYSTEM_LINK_MCTP_H

#include <stdint.h>

#include "zenoh-pico/collections/string.h"
#include "zenoh-pico/config.h"
#include "zenoh-pico/system/platform.h"

#ifdef __cplusplus
extern "C" {
#endif

#if Z_FEATURE_LINK_MCTP == 1

#define _Z_MCTP_MTU_SIZE 255

typedef struct {
	_z_sys_net_socket_t _sock;
} _z_mctp_socket_t;

z_result_t _z_listen_mctp(_z_sys_net_socket_t *sock);
z_result_t _z_open_mctp(_z_sys_net_socket_t *sock, uint8_t endpoint_id);
void _z_close_mctp(_z_sys_net_socket_t *sock);
size_t _z_read_exact_mctp(const _z_sys_net_socket_t sock, uint8_t *ptr, size_t len);
size_t _z_read_mctp(const _z_sys_net_socket_t sock, uint8_t *ptr, size_t len);
size_t _z_write_mctp(const _z_sys_net_socket_t sock, const uint8_t *ptr, size_t len);

#endif

#ifdef __cplusplus
}
#endif

#endif /* ZENOH_PICO_SYSTEM_LINK_MCTP_H */
