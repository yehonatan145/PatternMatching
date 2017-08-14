import sys
import random

def get_binary_val(c):
	if c >= '0' and c <= '9':
		return ord(c) - ord('0')
	elif c >= 'a' and c <= 'f':
		return 10 + ord(c) - ord('a')
	elif c >= 'A' and c <= 'F':
		return 10 + ord(c) - ord('A')
	else:
		return -1

def convert_to_int_list(line):
	ret = []
	is_binary_mode = False
	prev_val = -1
	for c in line:
		if c == '|':
			is_binary_mode = not is_binary_mode
			prev_val = -1
		elif is_binary_mode:
			if c == ' ':
				continue
			bin_val = get_binary_val(c)
			if bin_val == -1:
				return ret
			if prev_val == -1:
				prev_val = bin_val
			else:
				ret.append(prev_val * 16 + bin_val)
				prev_val = -1
		else:
			ret.append(ord(c))
	return ret


if (len(sys.argv) != 4):
	exit()


f = open(sys.argv[1], "ab")
shuf = open(sys.argv[2], "r")
max_len = int(sys.argv[3])
total_len = 0
writen = 0

while total_len < max_len:
	line = shuf.readline()
	if (len(line) == 0):
		break # end of file
	line_nums = convert_to_int_list(line)
	if len(line_nums) == 0:
		continue
	length = random.randint(1, len(line_nums))
	if (total_len + length >= max_len):
		length = max_len - total_len
	total_len += length
	for i in range(0, length):
		f.write(chr(line_nums[i]))
		writen += 1

shuf.close()
f.close()