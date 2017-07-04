import sys 
import os
import numpy as np
import lmdb
import argparse
import caffe
import numpy

def main(args):
	caffe_root = '../'
	sys.path.insert(0, caffe_root + 'python')

	# read labels
	all_labels = []
	label_count = 326
	labelFile = open(args.labels_file) 
	for labelLine in labelFile:
		labelLine = labelLine.rstrip()
		labels = labelLine.split(' ')
		one_labels = [0 for n in range(label_count)]
		for i_label in labels:
			one_labels[int(i_label)] = 1
		#print(int(i_label))
		#print(one_labels)
		all_labels.append(one_labels)
	#print(all_labels)	
	
	# create lmdb
	key = 0
	env = lmdb.open(args.lmdb_file, map_size=int(1e12))
	with env.begin(write=True) as txn: 
	    for labels in all_labels:
		labels_list = numpy.array(labels)
		#print(labels_list)
		datum = caffe.proto.caffe_pb2.Datum()
		datum.channels = labels_list.shape[0]
		#print(datum.channels)
		datum.height = 1
		datum.width =  1
		datum.data = labels_list.tostring()          # or .tobytes() if numpy < 1.9
		
		#print(datum.data)
		datum.label = 0
		key_str = '{:08}'.format(key)

		txn.put(key_str.encode('ascii'), datum.SerializeToString())
		key += 1


def parse_arguments(argv):
	parser = argparse.ArgumentParser()

	parser.add_argument('labels_file', 
			type=str, 
			help='The label file name (.txt)')
	parser.add_argument('lmdb_file',
			type=str, 
			help='The lmdb file name')
	return parser.parse_args(argv)

if __name__ == '__main__':
	main(parse_arguments(sys.argv[1:]))
