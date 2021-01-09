import socket

while True:
	with socket.socket() as s:
		s.connect(('192.168.43.21', 9000 ))
		print("Connected")

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

