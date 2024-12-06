#include "server.hpp"

ConnectionResult Server::respondHandshake(string dest_ip, uint16_t dest_port){
    const int RETRIES = 10;

    int retries = RETRIES;
    while(retries-- > 0) {
        try {
            // Get all the possible buffer
            Message sync_message = connection->consumeBuffer("",0,0,0,SYN_FLAG);

            std::string destIP = sync_message.ip;
            u_int16_t destPort = sync_message.port;
            u_int32_t sequence_num_first = sync_message.segment.seqNum;

            commandLine('i', "[Handshake] [S="+std::to_string(sequence_num_first)+"] Received SYN request from "+dest_ip+":"+std::to_string(destPort));
            
            // Sending SYN-ACK Request
            int sequence_num_second = generateRandomNumber(1,1000);
            u_int32_t ack_num_second = sequence_num_first + 1;

            commandLine('i', "[Handshake] [S="+std::to_string(sequence_num_second)+"] [A="+std::to_string(ack_num_second)+"] Sending SYN-ACK request to "+dest_ip+":"+std::to_string(destPort));
            connection->sendSegment(ack(sequence_num_second,ack_num_second), dest_ip, dest_port);
            connection->setSocketState(TCPState::SYN_SENT);

            // Received ACK Request
            Message ack_message = connection->consumeBuffer(destIP, destPort, 0, 0, ACK_FLAG);
            u_int32_t ack_num_third = ack_message.segment.ackNum;
            commandLine('i', "[Handshake] [A="+std::to_string(ack_num_third)+"] Received ACK request from "+dest_ip+":"+std::to_string(destPort));

            // Check Sequence and ACK validity
            if (ack_num_third = sequence_num_second + 1) {
                commandLine('i',"Sending input to "+dest_ip+":"+std::to_string(destPort));
                return ConnectionResult(true,destIP, destPort, sequence_num_second, ack_num_third);
            }
        } catch(const std::exception &e) {
            commandLine('!', "[ERROR] [HANDSHAKE] " + std::string(e.what()));
        }
    }

    commandLine('!', "[ERROR] [HANDSHAKE] Handshake failed after "+std::to_string(RETRIES)+" retries");
}
