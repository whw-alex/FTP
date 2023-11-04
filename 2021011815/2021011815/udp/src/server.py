import socket

size = 8192
count = 0

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(('', 9876))

try:
  while True:
    data, address = sock.recvfrom(size)
    data = (count).__str__() + " " + data.decode()
    # print(data)
    count += 1
    sock.sendto(data.upper().encode(), address)
finally:
  sock.close()