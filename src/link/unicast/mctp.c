//
// Copyright (c) 2022 ZettaScale Technology
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

#include "zenoh-pico/link/config/mctp.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "zenoh-pico/config.h"
#include "zenoh-pico/link/manager.h"
#include "zenoh-pico/system/link/mctp.h"
#include "zenoh-pico/utils/pointers.h"

#if Z_FEATURE_LINK_MCTP == 1

#define SPP_MAXIMUM_PAYLOAD 255


z_result_t _z_endpoint_mctp_valid(_z_endpoint_t *endpoint) {
    z_result_t ret = _Z_RES_OK;

    _z_string_t ser_str = _z_string_alias_str(MCTP_SCHEMA);
    if (!_z_string_equals(&endpoint->_locator._protocol, &ser_str)) {
        ret = _Z_ERR_CONFIG_LOCATOR_INVALID;
    }

    if (ret == _Z_RES_OK) {
        size_t addr_len = _z_string_len(&endpoint->_locator._address);
        const char *p_start = _z_string_data(&endpoint->_locator._address);
        const char *p_dot = (char *)memchr(p_start, (int)'.', addr_len);
        if (p_dot != NULL) {
            size_t dot_loc = _z_ptr_char_diff(p_dot, p_start);
            // Check if dot is first or last character
            if ((dot_loc == 0) || (dot_loc == addr_len)) {
                ret = _Z_ERR_CONFIG_LOCATOR_INVALID;
            }
        } else {
            if (_z_string_len(&endpoint->_locator._address) == (size_t)0) {
                ret = _Z_ERR_CONFIG_LOCATOR_INVALID;
            }
        }
    }
    return ret;
}



#define MCTP_CONNECT_THROTTLE_TIME_MS 250

z_result_t _z_connect_mctp(const _z_sys_net_socket_t sock) {
    const uint8_t MCTP_ZENOH_MAGIC[4] = {'M', 'C', 'T', 'P'};
    const uint8_t MCTP_ZENOH_MAGIC_RESP[4] = {'P', 'T', 'C', 'M'};

    while (true) {
        _z_write_mctp(sock, MCTP_ZENOH_MAGIC, 4);
        uint8_t magic_response[4];
        size_t ret = _z_read_mctp(sock, magic_response, sizeof(magic_response));
        if (ret == SIZE_MAX) {
            return _Z_ERR_TRANSPORT_RX_FAILED;
        }
        if (ret != 4) {
            z_sleep_ms(MCTP_CONNECT_THROTTLE_TIME_MS);
            continue;
        }

        bool match = false;
        for (int i = 0; i < 4; i++) {
            match = match && (magic_response[i] == MCTP_ZENOH_MAGIC_RESP[i]);
        }
        if (match) {
            goto out;
        } else {
            z_sleep_ms(MCTP_CONNECT_THROTTLE_TIME_MS);
        }
    }

out:
    return _Z_RES_OK;
}

z_result_t _z_f_link_open_mctp(_z_link_t *self) {
    z_result_t ret = _Z_RES_OK;

    size_t addr_len = _z_string_len(&self->_endpoint._locator._address);
    const char *p_start = _z_string_data(&self->_endpoint._locator._address);

    printf("opening mctp endpoint address %s\n", p_start);
    
    uint32_t endpoint_id = (uint32_t)strtoul(p_start, NULL, 10);

    _z_open_mctp(&self->_socket._mctp._sock, endpoint_id);

    return (ret == _Z_RES_OK ? _z_connect_mctp(self->_socket._mctp._sock) : ret);
}

z_result_t _z_f_link_listen_mctp(_z_link_t *self) {
    z_result_t ret = _Z_ERR_GENERIC;
    
    return ret;
}

void _z_f_link_close_mctp(_z_link_t *self) {
    _z_close_mctp(&self->_socket._mctp._sock);
}

void _z_f_link_free_mctp(_z_link_t *self) { (void)(self); }

size_t _z_f_link_write_mctp(const _z_link_t *self, const uint8_t *ptr, size_t len, _z_sys_net_socket_t *socket) {
    _ZP_UNUSED(socket);

    return _z_write_mctp(self->_socket._mctp._sock, ptr, len);
}

size_t _z_f_link_write_all_mctp(const _z_link_t *self, const uint8_t *ptr, size_t len) {
    return _z_write_mctp(self->_socket._mctp._sock, ptr, len);
}

size_t _z_f_link_read_mctp(const _z_link_t *self, uint8_t *ptr, size_t len, _z_slice_t *addr) {
    _ZP_UNUSED(self);
    _ZP_UNUSED(addr);

    return _z_read_mctp(self->_socket._mctp._sock, ptr, len);
}

size_t _z_f_link_read_exact_mctp(const _z_link_t *self, uint8_t *ptr, size_t len, _z_slice_t *addr,
                                   _z_sys_net_socket_t *socket) {
    _ZP_UNUSED(addr);
    _ZP_UNUSED(socket);

    return _z_read_exact_mctp(self->_socket._mctp._sock, ptr, len);
}

size_t _z_f_link_read_socket_mctp(const _z_sys_net_socket_t socket, uint8_t *ptr, size_t len) {
    return _z_read_mctp(socket, ptr, len);
}

uint16_t _z_get_link_mtu_mctp(void) { return _Z_MCTP_MTU_SIZE; }

z_result_t _z_new_link_mctp(_z_link_t *zl, _z_endpoint_t endpoint) {
    z_result_t ret = _Z_RES_OK;
    zl->_type = _Z_LINK_TYPE_MCTP;
    zl->_cap._transport = Z_LINK_CAP_TRANSPORT_UNICAST;
    zl->_cap._flow = Z_LINK_CAP_FLOW_DATAGRAM;
    zl->_cap._is_reliable = false;

    zl->_mtu = _z_get_link_mtu_mctp();

    zl->_endpoint = endpoint;

    zl->_open_f = _z_f_link_open_mctp;
    zl->_listen_f = _z_f_link_listen_mctp;
    zl->_close_f = _z_f_link_close_mctp;
    zl->_free_f = _z_f_link_free_mctp;

    zl->_write_f = _z_f_link_write_mctp;
    zl->_write_all_f = _z_f_link_write_all_mctp;
    zl->_read_f = _z_f_link_read_mctp;
    zl->_read_exact_f = _z_f_link_read_exact_mctp;
    zl->_read_socket_f = _z_f_link_read_socket_mctp;

    return ret;
}
#endif
