/*
 * Espressif Systems Wireless LAN device driver
 *
 * Copyright (C) 2015-2021 Espressif Systems (Shanghai) PTE LTD
 *
 * This software file (the "File") is distributed by Espressif Systems (Shanghai)
 * PTE LTD under the terms of the GNU General Public License Version 2, June 1991
 * (the "License").  You may use, redistribute and/or modify this File in
 * accordance with the terms and conditions of the License, a copy of which
 * is available by writing to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA or on the
 * worldwide web at http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt.
 *
 * THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE
 * ARE EXPRESSLY DISCLAIMED.  The License provides additional details about
 * this warranty disclaimer.
 */

#ifndef __esp__h_
#define __esp__h_

#include <linux/workqueue.h>
#include <linux/wait.h>
#include <linux/interrupt.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/spinlock.h>
#include <net/cfg80211.h>
#include <net/bluetooth/bluetooth.h>
#include <net/bluetooth/hci_core.h>
#include "adapter.h"

#define ESP_IF_TYPE_SDIO        1
#define ESP_IF_TYPE_SPI         2

/* Network link status */
#define ESP_LINK_DOWN           0
#define ESP_LINK_UP             1

#define ESP_MAX_INTERFACE       1
//#define ESP_MAX_INTERFACE       2
#define ESP_STA_NW_IF           0
#define ESP_AP_NW_IF            1

/* ESP in sdkconfig has CONFIG_IDF_FIRMWARE_CHIP_ID entry.
 * supported values of CONFIG_IDF_FIRMWARE_CHIP_ID are - */
enum chipset_type_e {
	ESP_FIRMWARE_CHIP_UNRECOGNIZED = 0xff,
	ESP_FIRMWARE_CHIP_ESP32 = 0x0,
	ESP_FIRMWARE_CHIP_ESP32S2 = 0x2,
	ESP_FIRMWARE_CHIP_ESP32C3 = 0x5,
	ESP_FIRMWARE_CHIP_ESP32S3 = 0x9,
};

#define ESP_PAYLOAD_HEADER      8
struct esp_private;
struct esp_adapter;

#define ACQUIRE_LOCK            1
#define LOCK_ALREADY_ACQUIRED   0

#define SKB_DATA_ADDR_ALIGNMENT 4
#define INTERFACE_HEADER_PADDING (SKB_DATA_ADDR_ALIGNMENT*3)

enum adapter_flags_e {
	ESP_CLEANUP_IN_PROGRESS,
	ESP_CMD_INIT_DONE,
};

enum priv_flags_e {
	ESP_NETWORK_UP,
};

struct command_node {
	struct list_head list;
	uint8_t cmd_code;
	struct sk_buff *cmd_skb;
	struct sk_buff *resp_skb;
};

struct esp_adapter {
	struct device           *dev;
	struct wiphy			*wiphy;

	u8                      if_type;
	u32                     capabilities;

	/* Possible types:
	 * struct esp_sdio_context */
	void                    *if_context;

	struct esp_if_ops       *if_ops;

	/* Private for each interface */
	struct esp_wifi_device  *priv[ESP_MAX_INTERFACE];
	struct hci_dev          *hcidev;

	struct workqueue_struct *if_rx_workqueue;
	struct work_struct       if_rx_work;

	wait_queue_head_t		wait_for_cmd_resp;
	u8						cmd_resp;

	/* wpa supplicant commands structures */
	struct command_node     *cmd_pool;
	struct list_head        cmd_free_queue;
	spinlock_t              cmd_free_queue_lock;
	struct list_head        cmd_pending_queue;
	spinlock_t              cmd_pending_queue_lock;

	struct command_node		*cur_cmd;
	spinlock_t				cmd_lock;

	struct workqueue_struct *cmd_wq;
	struct work_struct      cmd_work;

	struct sk_buff_head     events_skb_q;
	struct workqueue_struct *events_wq;
	struct work_struct      events_work;

	unsigned long           state_flags;
};

struct esp_device {
	struct device			*dev;
	struct wiphy			*wiphy;
	struct esp_adapter		*adapter;
};

struct esp_wifi_device {
	struct wireless_dev		wdev;
	struct net_device		*ndev;
	struct esp_device		*esp_dev;
	struct esp_adapter      *adapter;

	struct net_device_stats stats;
	u8                      link_state;
	u8                      mac_address[MAC_ADDR_LEN];
	u8                      if_type;
	u8                      if_num;

	int                     ssid_len;
	u8                      ssid[32];

	volatile u8             stop_data;
	volatile u8             port_open;

	/* This is needed to notify scan completion*/
	struct cfg80211_scan_request *request;

	uint8_t                 scan_in_progress;
	uint8_t                 waiting_for_scan_done;
	wait_queue_head_t       wait_for_scan_completion;
	unsigned long           priv_flags;
};


#if 0
struct esp_private {
	struct esp_adapter      *adapter;
	struct net_device       *ndev;
	struct net_device_stats stats;
	u8                      link_state;
	u8                      mac_address[6];
	u8                      if_type;
	u8                      if_num;
};
#endif

struct esp_skb_cb {
/*	struct esp_private      *priv;*/
	struct esp_wifi_device      *priv;
};
#endif
