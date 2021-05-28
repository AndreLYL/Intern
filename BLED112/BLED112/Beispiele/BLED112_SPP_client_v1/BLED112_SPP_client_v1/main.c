/**************************************************************************************************
* (C) Copyright 2017 Silicon Labs, http://www.silabs.com
***************************************************************************************************
* This file is licensed under the Silabs License Agreement. See the file
* "Silabs_License_Agreement.txt" for details. Before using this software for
* any purpose, you must agree to the terms of that agreement.
***************************************************************************************************
*
* Example program for using BLED112 as SPP client. The remote client is expected tu be running
* SPP-over-BLE example for BGM111 (spp_server), for more details see following knowledgebase
* article:
*  http://community.silabs.com/t5/Bluetooth-Wi-Fi-Knowledge-Base/SPP-over-BLE-BGScript-example-for-BGM111/ta-p/166950
*
* --- Usage: ---
*
* 1) To scan for any BLE devices in the range:
*
*    ./SPP_Client.exe COM4 scan
*
*    Scanning stops automatically after 5 seconds.
*
* 2) To connect to a SPP server :
*
*    ./SPP_Client.exe COM4 00:0b:57:xx:xx:xx
*
*
* --- Changelog: ---
*
* 2017-03-05    First version
**************************************************************************************************/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include <pthread.h>

#include <sys/time.h>

#include "cmd_def.h"
#include "uart.h"

//#define DEBUG

#define CLARG_PORT 1
#define CLARG_ACTION 2
#define CLARG_OPTIONS 3

#define UART_TIMEOUT 1000

#define MAX_DEVICES 64
int found_devices_count = 0;
bd_addr found_devices[MAX_DEVICES];

enum actions {
    action_none,
    action_scan,
    action_connect,
    action_info,
};
enum actions action = action_none;

typedef enum {
    state_disconnected,
    state_connecting,
    state_connected,
    state_finding_services,
    state_finding_attributes,
	state_enable_notif,
    state_listening_spp_data,
    state_finish,
    state_last
} states;
states state = state_disconnected;

const char *state_names[state_last] = {
    "disconnected",
    "connecting",
    "connected",
    "finding_services",
    "finding_attributes",
	"enable_notif",
    "listening_spp_data",
    "finish"
};

typedef struct
{
	uint16 u16PacketCount;
	uint8 u8TimerInterval;
	uint8 u8TransferType;   // 0 = notify (default), 1 = indicate
	uint8 u8NumAck;		// if > 0, send ack message every N packets
} tsOptions;


#define FIRST_HANDLE 0x0001
#define LAST_HANDLE  0xffff



// BGM11x custom SPP service: UUID 4880c12c-fdcb-4077-8920-a450d7f9b907
static const uint8 _SPP_service_UUID[16] = {0x07, 0xb9, 0xf9, 0xd7, 0x50, 0xa4, 0x20, 0x89, 0x77, 0x40, 0xcb, 0xfd, 0x2c, 0xc1, 0x80, 0x48};

// SPP data characteristic: UUID fec26ec4-6d71-4442-9f81-55bc21d658d6
static const uint8 _SPP_data_UUID[16] = {0xd6, 0x58, 0xd6, 0x21, 0xbc, 0x55, 0x81, 0x9f, 0x42, 0x44, 0x71, 0x6d, 0xc4, 0x6e, 0xc2, 0xfe};


uint8 primary_service_uuid[] = {0x00, 0x28};


// SPP service characteristic handles:
uint16 spp_handle_start = 0,
		spp_handle_end = 0,
		spp_handle_data,
		spp_handle_configuration = 0;

bd_addr connect_addr;

static int conn_status = 0;
static int conn_handle = -1;

static uint32 scan_countdown;

void usage(char *exe)
{
    printf("%s <COMx|list> <scan|address>\n", exe);

}

void change_state(states new_state)
{
#ifdef DEBUG
    printf("DEBUG: State changed: %s --> %s\n", state_names[state], state_names[new_state]);
#endif
    state = new_state;
}

/**
 * Compare Bluetooth addresses
 *
 * @param first First address
 * @param second Second address
 * @return Zero if addresses are equal
 */
int cmp_bdaddr(bd_addr first, bd_addr second)
{
    int i;
    for (i = 0; i < sizeof(bd_addr); i++) {
        if (first.addr[i] != second.addr[i]) return 1;
    }
    return 0;
}

void print_bdaddr(bd_addr bdaddr)
{
    printf("%02x:%02x:%02x:%02x:%02x:%02x",
            bdaddr.addr[5],
            bdaddr.addr[4],
            bdaddr.addr[3],
            bdaddr.addr[2],
            bdaddr.addr[1],
            bdaddr.addr[0]);
}

void print_raw_packet(struct ble_header *hdr, unsigned char *data)
{
    printf("Incoming packet: ");
    int i;
    for (i = 0; i < sizeof(*hdr); i++) {
        printf("%02x ", ((unsigned char *)hdr)[i]);
    }
    for (i = 0; i < hdr->lolen; i++) {
        printf("%02x ", data[i]);
    }
    printf("\n");
}

void output(uint8 len1, uint8* data1, uint16 len2, uint8* data2)
{
    if (uart_tx(len1, data1) || uart_tx(len2, data2)) {
        printf("ERROR: Writing to serial port failed\n");
        exit(1);
    }
}

int read_message(int timeout_ms)
{
    unsigned char data[256]; // enough for BLE
    struct ble_header hdr;
    int r;

    r = uart_rx(sizeof(hdr), (unsigned char *)&hdr, UART_TIMEOUT);
    if (!r) {
        return -1; // timeout
    }
    else if (r < 0) {
        printf("ERROR: Reading header failed. Error code:%d\n", r);
        return 1;
    }

    if (hdr.lolen) {
        r = uart_rx(hdr.lolen, data, UART_TIMEOUT);
        if (r <= 0) {
            printf("ERROR: Reading data failed. Error code:%d\n", r);
            return 1;
        }
    }

    const struct ble_msg *msg = ble_get_msg_hdr(hdr);

#ifdef DEBUG
    print_raw_packet(&hdr, data);
#endif

    if (!msg) {
        printf("ERROR: Unknown message received\n");
        exit(1);
    }

    msg->handler(data);

    return 0;
}

void enable_indications(uint8 connection_handle, uint16 client_configuration_handle)
{
    uint8 configuration[] = {0x02, 0x00}; // enable indications
    ble_cmd_attclient_attribute_write(connection_handle, client_configuration_handle, 2, &configuration);
}

void enable_notifications(uint8 connection_handle, uint16 client_configuration_handle)
{
    uint8 configuration[] = {0x01, 0x00}; // enable notifications
    ble_cmd_attclient_attribute_write(connection_handle, client_configuration_handle, 2, &configuration);
}


void ble_rsp_system_get_info(const struct ble_msg_system_get_info_rsp_t *msg)
{
    printf("Build: %u, protocol_version: %u, hardware: ", msg->build, msg->protocol_version);
    switch (msg->hw) {
    case 0x01: printf("BLE112"); break;
    case 0x02: printf("BLED112"); break;
    default: printf("Unknown");
    }
    printf("\n");

    if (action == action_info) change_state(state_finish);
}

void ble_evt_gap_scan_response(const struct ble_msg_gap_scan_response_evt_t *msg)
{
    if (found_devices_count >= MAX_DEVICES) change_state(state_finish);

    int i;
    char *name = NULL;
    int found = 0;

    // Check if this device already found
    for (i = 0; i < found_devices_count; i++) {
        if (!cmp_bdaddr(msg->sender, found_devices[i]))
        {
        	found = 1;
        }
    }

    if(found == 0)
    {
    	found_devices_count++;
    	memcpy(found_devices[i].addr, msg->sender.addr, sizeof(bd_addr));
    }
    else
    {
    	return;
    }

    // Parse data
    for (i = 0; i < msg->data.len; ) {
        int8 len = msg->data.data[i++];
        if (!len) continue;
        if (i + len > msg->data.len) break; // not enough data
        uint8 type = msg->data.data[i++];
        switch (type) {
        case 0x09:
            name = malloc(len);
            memcpy(name, msg->data.data + i, len - 1);
            name[len - 1] = '\0';
        }

        i += len - 1;
    }

    print_bdaddr(msg->sender);
    printf(" RSSI:%d", msg->rssi);

    printf(" Name:");
    if (name) printf("%s", name);
    else printf("Unknown");
    printf("\n");

    free(name);
}

void ble_evt_connection_status(const struct ble_msg_connection_status_evt_t *msg)
{
    // New connection
    if (msg->flags & connection_connected) {

    	if(conn_status == 0)
    	{
    		// first connection status event (new connection established)

    		// store connection handle (this is needed for example when disconnecting)
    		conn_handle = msg->connection;

    		change_state(state_connected);
    		printf("Connected (handle %u)\n", conn_handle);

    		change_state(state_finding_services);
    		ble_cmd_attclient_read_by_group_type(msg->connection, FIRST_HANDLE, LAST_HANDLE, 2, primary_service_uuid);

    		conn_status = 1;
    	}
    	else
    	{
    		// connection status event received while already connected. This will happen if for example the slave
    		// changes connection parameters
    		printf("Connection updated ->\n");
    	}

    	// connection parameters:
    	printf("Connection interval: %u units = %.2f ms\n", msg->conn_interval, (float)(msg->conn_interval) * 1.25);

    }

}

void ble_evt_attclient_group_found(const struct ble_msg_attclient_group_found_evt_t *msg)
{
	int i;

    if (msg->uuid.len == 0)
    	return;


    printf("Service UUID:");
    for(i=0;i<16;i++)
    {
    	printf("%2.2x_", msg->uuid.data[i]);
    }
    printf("\n");


    // try to find SPP service (uses custom 128bit UUID)
    if (state == state_finding_services && spp_handle_start == 0 && msg->uuid.len == 16) {

    	if(memcmp(msg->uuid.data, _SPP_service_UUID, 16) == 0)
    	{
    		spp_handle_start = msg->start;
    		spp_handle_end = msg->end;
    		printf("Found SPP service, handles %u..%u\n", spp_handle_start, spp_handle_end);
    	}
    }

}

void ble_evt_attclient_procedure_completed(const struct ble_msg_attclient_procedure_completed_evt_t *msg)
{
    if (state == state_finding_services) {
        // service not found
        if (spp_handle_start == 0) {
            printf("No SPP service found\n");
            change_state(state_finish);
        }
        // Find SPP service attributes
        else {
            change_state(state_finding_attributes);
            ble_cmd_attclient_find_information(msg->connection, spp_handle_start, spp_handle_end);
        }
    }
    else if (state == state_finding_attributes) {
        // Client characteristic configuration not found
        if (spp_handle_configuration == 0) {
            printf("No Client Characteristic Configuration found for SPP service\n");
            change_state(state_finish);
        }
        // Enable notifications
        else {
            change_state(state_enable_notif);

            printf("Enabling notifications\n");
            enable_notifications(msg->connection, spp_handle_configuration);

            // start a soft timer with 100 ms interval
            ble_cmd_hardware_set_soft_timer(3200, 0, 0);
        }
    }
    else if(state == state_enable_notif)
    {

    	change_state(state_listening_spp_data);

    	printf("DATA\n");

    }
}

void ble_evt_attclient_find_information_found(const struct ble_msg_attclient_find_information_found_evt_t *msg)
{
	int i;

	if(msg->uuid.len == 16)
	{
		if(memcmp(msg->uuid.data, _SPP_data_UUID, 16)==0)
		{
			spp_handle_data = msg->chrhandle;
			printf("SPP data UUID found, handle %d\n", spp_handle_data);
		}
		else
		{
		    printf("Unknown Service UUID:");
		    for(i=0;i<16;i++)
		    {
		    	printf("%2.2x_", msg->uuid.data[i]);
		    }
		    printf("\n");
		}
	}
	else if (msg->uuid.len == 2) {

		uint16 uuid = (msg->uuid.data[1] << 8) | msg->uuid.data[0];

		if (uuid == 0x2902) {

			if(spp_handle_configuration == 0)
			{
				spp_handle_configuration = msg->chrhandle;
				printf("SPP data config found, handle %d\n", spp_handle_configuration);
			}
			else
			{
				printf("unexpected configuration UUID?\n");
			}
		}
	}

}

// this event is raised when a notification (SPP data) from remote client is received.
void ble_evt_attclient_attribute_value(const struct ble_msg_attclient_attribute_value_evt_t *msg)
{
	int i;

	if(msg->atthandle == spp_handle_data)
	{

		for(i=0;i<msg->value.len;i++)
		{
			if(msg->value.data[i] >= 32 && msg->value.data[i] <= 126)
			{
				// printable value
				printf("%c", msg->value.data[i]);
			}
			else
			{
				// non-printable value -> show hex value
				printf("[%2.2x]", msg->value.data[i]);
			}
			fflush(stdout);
		}

	}
	else
	{
		printf("unexpected evt_attclient_attribute_value, handle %u\n", msg->atthandle);
	}

}

void ble_evt_connection_disconnected(const struct ble_msg_connection_disconnected_evt_t *msg)
{

    change_state(state_disconnected);
    printf("Connection terminated. Reason: %4.4x\n", msg->reason);

    switch(msg->reason)
    {
    	case 0x023E: printf("REASON: Connection Failed to be Established\n"); break;
    	case 0x0216: printf("REASON: Connection Terminated by Local Host\n"); break;
    	case 0x0208: printf("REASON: Connection Timeout\n"); break;
    	default: printf("REASON: ?? (check API reference doc...)\n");
    }

    change_state(state_finish);


    conn_status = 0;

}


char tx_fifo[256];
int tx_wr = 0;
int tx_rd = 0;
int num_written = 0;
int num_read = 0;

/* this is a thread that is reading input from user (without blocking the
 * main loop). Data entered by user is copied into a FIFO. In the main loop
 * the soft timer event from BLED112 dongle is used to trigger offloading of
 * the data in FIFO.
 * */
void *stdin_reader(void *vptr_value) {

	char c;

	while(1)
	{
		c = getchar();

		tx_fifo[tx_wr++] = c;
		num_written++;

		// add carriage return after newline automatically.
		if(c == '\n')
		{
			tx_fifo[tx_wr++] = '\r';
			num_written++;
		}

		if(tx_wr >= 256)
		{
			tx_wr  = 0;
		}
	}
	pthread_exit(NULL);
}

/* soft timer callback: this is used to offload data from tx_fifo */
void ble_evt_hardware_soft_timer(const struct ble_msg_hardware_soft_timer_evt_t *msg)
{
	int data_len;
	char	 data_data[20];
	int i;


	if(scan_countdown > 0)
	{
		scan_countdown--;

		if(scan_countdown == 0)
		{
			printf("Stopping scan.\n");
			ble_cmd_gap_end_procedure();
			exit(1);
		}
	}


	if(conn_handle < 0)
	{
		return;
	}

	data_len = num_written - num_read;

	if(data_len == 0)
	{
		return;
	}

	if(data_len > 20)
	{
		data_len = 20;
	}

	for(i=0;i<data_len;i++)
	{
		data_data[i] = tx_fifo[tx_rd++];
		if(tx_rd >= 256)
		{
			tx_rd = 0;
		}
	}

	ble_cmd_attclient_write_command(conn_handle, spp_handle_data, data_len, data_data);
	num_read += data_len;

}

int main(int argc, char *argv[]) {
    char *uart_port = "";

    pthread_t tid;

    // Not enough command-line arguments
    if (argc <= CLARG_PORT) {
        usage(argv[0]);
        return 1;
    }

    // COM port argument
    if (argc > CLARG_PORT) {
        if (strcmp(argv[CLARG_PORT], "list") == 0) {
            uart_list_devices();
            return 1;
        }
        else {
            uart_port = argv[CLARG_PORT];
        }
    }

    // Action argument
    if (argc > CLARG_ACTION) {
        int i;
        for (i = 0; i < strlen(argv[CLARG_ACTION]); i++) {
            argv[CLARG_ACTION][i] = tolower(argv[CLARG_ACTION][i]);
        }

        if (strcmp(argv[CLARG_ACTION], "scan") == 0) {
            action = action_scan;
            scan_countdown = 50; // scan for 5 seconds and then exit
        }
        else if (strcmp(argv[CLARG_ACTION], "info") == 0) {
            action = action_info;
        }
        else {
            int i;
            short unsigned int addr[6];
            if (sscanf(argv[CLARG_ACTION],
                    "%02hx:%02hx:%02hx:%02hx:%02hx:%02hx",
                    &addr[5],
                    &addr[4],
                    &addr[3],
                    &addr[2],
                    &addr[1],
                    &addr[0]) == 6) {

                for (i = 0; i < 6; i++) {
                    connect_addr.addr[i] = addr[i];
                }
                action = action_connect;
            }
        }
    }
    if (action == action_none) {
        usage(argv[0]);
        return 1;
    }

    // create a thread that reads input from STDIN :
    pthread_create(&tid, NULL, stdin_reader, NULL);

    bglib_output = output;

    if (uart_open(uart_port)) {
        printf("ERROR: Unable to open serial port\n");
        return 1;
    }



    // Reset dongle to get it into known state
    ble_cmd_system_reset(0);
    uart_close();
    do {
        usleep(500000); // 0.5s
    } while (uart_open(uart_port));

    // Execute action
    if (action == action_scan) {
        ble_cmd_gap_discover(gap_discover_observation);

        // start a soft timer with 10 ms interval
        ble_cmd_hardware_set_soft_timer(3200, 0, 0);

    }
    else if (action == action_info) {
        ble_cmd_system_get_info();
    }
    else if (action == action_connect) {
        printf("Trying to connect\n");
        change_state(state_connecting);
        ble_cmd_gap_connect_direct(&connect_addr, gap_address_type_public, 40, 60, 100,0);
    }

    // Message loop
    while (state != state_finish) {
        if (read_message(UART_TIMEOUT) > 0) break;
    }

    uart_close();

    return 0;
}
