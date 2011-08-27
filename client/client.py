import math
import socket
import struct
import time

MAX_MESSAGE_LENGTH = 508
MAX_PACKET_LENGTH = MAX_MESSAGE_LENGTH + 4

def truncate(string, max_len):
    length = min(len(string), max_len)
    return length, string[:length]

# reserves 2 bytes for string length and terminating null
def pack_string(string, max_len=255, word_align=True):
    # yes, hard cap of 254 characters
    length = min(len(string), max_len, 255) + 1
    if word_align:
        length = int(math.ceil((length+1)/4.0)) * 4 - 1
    string = string[:length]
    return struct.pack("b %ds" % length, length, string)

class DaemonPacket(object):
    VERSION = 1
    HEADER_SIZE = 4
    # version, type, length
    HEADER_FORMAT = struct.Struct("B c H")
    STATS_SIZE = 16
    # timestamp, value, service_len, metric_len
    STATS_HEADER = struct.Struct("Q q")

    def __init__(self, packet):
        header = DaemonPacket.HEADER_FORMAT.unpack(packet[:DaemonPacket.HEADER_SIZE])
        self.version, self.type, self.message_length = header
        self.message = packet[DaemonPacket.HEADER_SIZE:][:self.message_length]

    @staticmethod
    def pack(message_type, message):
        message_length, message = truncate(message, MAX_MESSAGE_LENGTH)
        return DaemonPacket.HEADER_FORMAT.pack(DaemonPacket.VERSION, message_type, message_length) + message

    @staticmethod
    def stats_packet(service, metric, value, tags=[]):
        # limits:
        #   service: 126 characters
        #   metric: 126 characters
        #   tags: 9 tags at 22 characters each
        timestamp = int(time.time() * 1000000)
        stats_header = DaemonPacket.STATS_HEADER.pack(timestamp, value)
        message = [stats_header]
        message.append(pack_string(service, 126))
        message.append(pack_string(metric, 126))
        for tag in tags[:9]:
            message.append(pack_string(tag, 22))
        for msg in message:
            print repr(msg)
        return DaemonPacket.pack('S', ''.join(message))

    def __repr__(self):
        return repr((self.version, self.type, self.message))

class DaemonClient(object):
    def __init__(self, server_ip, server_port):
        self.version = 1
        self.server = (server_ip, server_port)
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    def send(self, message):
        outgoing = DaemonPacket.pack('E', message)
        self.__send_packet(outgoing)
        return self.__recv_packet()

    def send_packet(self, packet):
        self.__send_packet(packet)
        return self.__recv_packet()

    def __send_packet(self, packet):
        self.socket.sendto(packet, self.server)

    def __recv_packet(self):
        data, addr = self.socket.recvfrom(MAX_PACKET_LENGTH)
        incoming = DaemonPacket(data)
        return (incoming, addr)


if __name__ == '__main__':
    client = DaemonClient("127.0.0.1", 10180)
    stats = DaemonPacket.stats_packet("Service", "metric", 7357, ["tag1", "tag2"])
    packet, client = client.send_packet(stats)
    # packet, client = client.send("Hello world")
    print packet
    print repr(client)
