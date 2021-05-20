import struct
import sys

with open(sys.argv[1], 'rb') as fp:
	data = fp.read()

ofs = 16
counter = 0

srate = struct.unpack('I', data[ofs:ofs+4])[0]
ofs += 4
samples = struct.unpack('Q', data[ofs:ofs+8])[0]
ofs += 8
print(f'srate: {srate} | samples: {samples}\n')

while ofs < len(data):
	sampleOfs = struct.unpack('Q', data[ofs:ofs+8])[0]
	ofs += 8

	if ofs >= len(data):
		print(f'#END (0) @ {sampleOfs}')
		break

	dlen = struct.unpack('I', data[ofs:ofs+4])[0]
	ofs += 4

	with open(f'{sampleOfs}.opus', 'wb') as fp:
		fp.write(data[ofs:ofs+dlen])
	ofs += dlen

	counter += 1
	print(f'#{counter} ({dlen}) @ {sampleOfs}')
