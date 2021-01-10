import socket
import select
import sys

while True:
	with socket.socket() as s:
		s.connect(('192.168.43.21', 9000 ))
		print("Connected")

		while True:
			list = [sys.stdin, s]
			r, w, e = select.select(list, [], [])

			for k in r:
				if k == s:
					data = k.recv(128)
					if not data:
						print('Closing connection')
						break
					else:
						print("New message: " + str(data))
				else:
					com = input("Waiting for input: ")
					s.sendall(msg.encode())

		s.close()
		"""
		while True:
			com = input("Waiting for input: ")
			s.sendall(com.encode())
			data = ""
			while True:
				content = s.recv(128)
				if len(content) == 0:
					break
				else:
					data += str(content)
					if '_' in data:
						print(data)
						break

		print("Closing connection")
"""
