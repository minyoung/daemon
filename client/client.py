import math
import platform
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

def get_timestamp():
    return int(time.time() * 1000000)

class DaemonPacket(object):
    VERSION = 1
    HEADER_SIZE = 4
    # version, type, length
    HEADER_FORMAT = struct.Struct("B c H")

    LOG_SIZE = 8
    # timestamp
    LOG_HEADER = struct.Struct("Q")

    def __init__(self, packet):
        header = DaemonPacket.HEADER_FORMAT.unpack(packet[:DaemonPacket.HEADER_SIZE])
        self.version, self.type, self.message_length = header
        self.message = packet[DaemonPacket.HEADER_SIZE:][:self.message_length]

    @staticmethod
    def pack(message_type, message):
        message_length, message = truncate(message, MAX_MESSAGE_LENGTH)
        return DaemonPacket.HEADER_FORMAT.pack(DaemonPacket.VERSION, message_type, message_length) + message

class StatsPacket(object):
    HEADER_SIZE = 16
    # timestamp, value, service_len, metric_len
    HEADER = struct.Struct("Q q")

    def __init__(self, service, metric, value, tags=[]):
        self.service = service
        self.metric = metric
        self.value = value
        self.tags = tags
        self.type = 'S'
        self.service_len = 126
        self.metric_len = 126
        self.tag_len = 22
        self.tag_limit = 8

    def pack(self):
        # limits:
        #   service: 126 characters
        #   metric: 126 characters
        #   tags: 8 tags at 22 characters each
        message = [self.__get_head()]
        message.extend(self.__get_body())
        for msg in message:
            print repr(msg)
        return DaemonPacket.pack(self.type, ''.join(message))

    def __get_head(self):
        return StatsPacket.HEADER.pack(get_timestamp(), self.value)

    def __get_body(self):
        message = []
        message.append(pack_string(self.service, self.service_len))
        message.append(pack_string(self.metric, self.metric_len))
        message.append(pack_string(platform.node(), self.tag_len))
        for tag in self.tags[:self.tag_limit]:
            message.append(pack_string(tag, self.tag_len))
        return message

class LogPacket(StatsPacket):
    HEADER_SIZE = 8
    # timestamp
    HEADER = struct.Struct("Q")

    def __init__(self, service, log_line, tags=[]):
        super(LogPacket, self).__init__(service, log_line, None, tags)
        self.type = 'L'

    def __get_head(self):
        return LogPacket.HEADER.pack(get_timestamp())

class ExtendedLogPacket(LogPacket):
    def __init__(self, *args, **kwargs):
        super(ExtendedLogPacket, self).__init__(*args, **kwargs)
        self.type = 'L'
        self.metric_len = 222
        self.tag_limit = 4

class DaemonClient(object):
    def __init__(self, server_ip, server_port):
        self.version = 1
        self.server = (server_ip, server_port)
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    def send_message(self, message):
        outgoing = DaemonPacket.pack('E', message)
        return self.send_packet(outgoing)

    def send_packet(self, packet):
        self.__send_packet(packet)
        return self.__recv_packet()

    def send_log(self, log_line):
        outgoing = DaemonPacket.log_packet(log_line)
        return self.send_packet(outgoing)

    def __send_packet(self, packet):
        self.socket.sendto(packet, self.server)

    def __recv_packet(self):
        data, addr = self.socket.recvfrom(MAX_PACKET_LENGTH)
        incoming = DaemonPacket(data)
        return (incoming, addr)


if __name__ == '__main__':
    client = DaemonClient("127.0.0.1", 10180)
    stats = StatsPacket("Service", "metric", 7357, ["tag1", "tag2"])
    packet, client = client.send_packet(stats.pack())
    # log_packet = LogPacket("Service", "log line", ["tag1", "tag2"])
    # packet, client = client.send_packet(log_packet.pack())
    # packet, client = client.send_message("Hello world")
    print packet
    print repr(client)
