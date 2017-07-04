import sys
sys.path.insert(0, '//home/tutu/job/caffe-server/python')
import caffe

import numpy as np 
import math

def sigmoid(x):
	return 1.0 / (1.0 + math.exp(-x))

class Accuracy(caffe.Layer):
	def setup(self, bottom, top):
		top[0].reshape(1)

	def forward(self, bottom, top):
		fc8_arr = np.array(bottom[0].data)
		label_arr = np.array(bottom[1].data)

		count1 = 0
		count2 = 0
		count3 = 0

		for i in xrange(len(fc8_arr)):
			for j in xrange(len(fc8_arr[i])):
				if (fc8_arr[i][j] > 0.0):
					count1 += 1

				if (label_arr[i][j] > 0.5):
					count2 += 1

				if (fc8_arr[i][j] > 0.0 and label_arr[i][j] > 0.5):
					count3 += 1

		acc = count3 * 1.0 / max(1, count1)
		recall = count3 * 1.0 / count2
		f1 = 2 * acc * recall / max(0.0000001, acc + recall)

		top[0].data[...] = f1

	def backward(self, top, propagate_down, bottom):
		pass

	def reshape(self, bottom, top):
		pass
