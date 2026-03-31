#include "mavlink_c/ardupilotmega/mavlink.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <cstring>

int main() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    // Bind a local port — this is what was missing.
    // Without this, MAVProxy has no address to reply to.
    sockaddr_in local{};
    local.sin_family      = AF_INET;
    local.sin_port        = htons(14553);   // companion listens here
    local.sin_addr.s_addr = INADDR_ANY;
    bind(sock, (sockaddr*)&local, sizeof(local));

    // Send TO MAVProxy's primary input
    sockaddr_in dest{};
    dest.sin_family = AF_INET;
    dest.sin_port   = htons(14551);         // changed from 14551 to 14550
    inet_pton(AF_INET, "127.0.0.1", &dest.sin_addr);

    std::cout << "Companion Health Daemon — sending heartbeats to SITL...\n";

    mavlink_message_t msg;
    uint8_t buf[MAVLINK_MAX_PACKET_LEN];
    uint8_t rbuf[MAVLINK_MAX_PACKET_LEN];

    while (true) {
        // --- SEND heartbeat ---
        mavlink_msg_heartbeat_pack(
            42,                           // system ID — use 42 to distinguish from SITL (sysID 1)
            MAV_COMP_ID_ONBOARD_COMPUTER,
            &msg,
            MAV_TYPE_ONBOARD_CONTROLLER,
            MAV_AUTOPILOT_INVALID,
            0, 0, MAV_STATE_ACTIVE
        );
        int len = mavlink_msg_to_send_buffer(buf, &msg);
        sendto(sock, buf, len, 0, (sockaddr*)&dest, sizeof(dest));
        std::cout << "[TX] Heartbeat sent (SysID:42 CompID:191)\n";

        // --- RECEIVE any reply (non-blocking peek) ---
        fd_set fds; FD_ZERO(&fds); FD_SET(sock, &fds);
        timeval tv{0, 100000};  // 100ms timeout
        if (select(sock+1, &fds, nullptr, nullptr, &tv) > 0) {
            int n = recv(sock, rbuf, sizeof(rbuf), 0);
            mavlink_message_t rmsg;
            mavlink_status_t  status;
            for (int i = 0; i < n; i++) {
                if (mavlink_parse_char(MAVLINK_COMM_0, rbuf[i], &rmsg, &status)) {
                    std::cout << "[RX] MAVLink msg from SysID:" << (int)rmsg.sysid
                              << " MsgID:" << rmsg.msgid << "\n";
                }
            }
        }
        sleep(1);
    }
    close(sock);
}