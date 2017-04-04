#!/usr/bin/python3
import matplotlib.pyplot as plt
import sys
import os
import functools
import io
import numpy as np
import json

genfromtxt_old = np.genfromtxt
@functools.wraps(genfromtxt_old)
def genfromtxt_py3_fixed(f, encoding="utf-8", *args, **kwargs):
	if isinstance(f, io.TextIOBase):
		if hasattr(f, "buffer") and hasattr(f.buffer, "raw") and \
		isinstance(f.buffer.raw, io.FileIO):
			# Best case: get underlying FileIO stream (binary!) and use that
			fb = f.buffer.raw
			# Reset cursor on the underlying object to match that on wrapper
			fb.seek(f.tell())
			result = genfromtxt_old(fb, *args, **kwargs)
			# Reset cursor on wrapper to match that of the underlying object
			f.seek(fb.tell())
		else:
			# Not very good but works: Put entire contents into BytesIO object,
			# otherwise same ideas as above
			old_cursor_pos = f.tell()
			fb = io.BytesIO(bytes(f.read(), encoding=encoding))
			result = genfromtxt_old(fb, *args, **kwargs)
			f.seek(old_cursor_pos + fb.tell())
	else:
		result = genfromtxt_old(f, *args, **kwargs)
	return result

if sys.version_info >= (3,):
	np.genfromtxt = genfromtxt_py3_fixed

def plot(filename):
	format_json=json.load(open("format.json","r"))
	data = np.genfromtxt(filename, delimiter=',', dtype=None)
	[rows, columns]=data.shape
	column_name=[]
	for i in range(0,columns):
		column_name.append(data[0,i].decode("utf-8"))
		
	os.environ["DISPLAY"]=":0"
	plt.close('all')
	f, axarr = plt.subplots(len(format_json), sharex=True)
	x=range(0, rows-1)
	for i in range(0,len(format_json)):
		for j in range(0,columns):
			if format_json[i] == column_name[j]:
				break;
		if format_json[i] != column_name[j]:
			continue
		axarr[i].set_ylabel(column_name[j],rotation=0,ha='left')
		axarr[i].yaxis.set_label_coords(0.02,0.75)
		axarr[i].plot(x, data[1:,j])
		axarr[i].yaxis.set_tick_params(which='both', direction='in')
		axarr[i].grid(True)
		
	#f.tight_layout()
	f.subplots_adjust(left=0.02, right=1, top=1, bottom=0)
	#f.subplots_adjust(right=1, top=1, bottom=0)
	f.subplots_adjust(hspace=0)

	plt.setp([a.get_xticklabels() for a in f.axes], visible=False)
	plt.show()

def main():
	if len(sys.argv)<2:
		print(sys.argv[0]+" csv_file")
		exit(0)
	else:
		plot(sys.argv[1])
if __name__ == "__main__":
	main()
