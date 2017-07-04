import os
import sys
import numpy as np
import lmdb
import argparse
import caffe
import numpy
import skimage

def write_lmdb(image_name_list,label_array,lmdb_img_name,lmdb_label_name,resize_image = True):
    for lmdb_name in [lmdb_img_name, lmdb_label_name]:
        db_path = os.path.abspath(lmdb_name)
        if os.path.exists(db_path):
            shutil.rmtree(db_path)

    counter_img = 0
    counter_label = 0
    batchsz = 100
    fail_cnt = 0
    print("Processing {:d} images and labels...".format(len(image_name_list)))

    for i in xrange(int(np.ceil(len(image_name_list)/float(batchsz)))):
	print("i=%d"%i)
        image_name_batch = image_name_list[batchsz*i:batchsz*(i+1)]
        label_batch = label_array[batchsz*i:batchsz*(i+1):]
        #print label_batch[np.newaxis,np.newaxis,0].dtype
        raw_input('r')
        imgs, labels = [], []
        for idx,image_name in enumerate(image_name_batch):
                img = skimage.io.imread(image_name)
                if resize_image==True:
                    img = skimage.transform.resize(img,(224, 224))
                imgs.append(img)

        db_imgs = lmdb.open(lmdb_img_name, map_size=1e12)
        with db_imgs.begin(write=True) as txn_img:
            for img in imgs:
                datum = caffe.io.array_to_datum(np.expand_dims(img, axis=0))
                txn_img.put("{:0>10d}".format(counter_img), datum.SerializeToString())
                counter_img += 1
        print("Processed {:d} images".format(counter_img))

        db_labels = lmdb.open(lmdb_label_name, map_size=1e12)
        with db_labels.begin(write=True) as txn_label:
            for idx in range(label_batch.shape[0]):
                datum = caffe.io.array_to_datum(label_batch[np.newaxis,np.newaxis,idx])
                txn_label.put("{:0>10d}".format(counter_label), datum.SerializeToString())
                counter_label += 1
        print("Processed {:d} labels".format(counter_label))

    print fail_cnt,'images fail reading'
    db_imgs.close()
    db_labels.close()

def main(args):
	caffe_root = '../'
	sys.path.insert(0, caffe_root + 'python')

	# read images
	image_name_list = []
	imageFile = open(args.image_file)
	for imageLine in imageFile:
		imageLine = imageLine.rstrip()		
		image, tmpLabel = imageLine.split(' ')
		image_name_list.append(image)

	# read labels
	label_array = []
	labelFile = open(args.labels_file) 
	for labelLine in labelFile:
		labelLine = labelLine.rstrip()
		labels = labelLine.split(' ')
		label_array.append(labels)

	write_lmdb(image_name_list, label_array, args.lmdb_img_name, args.lmdb_label_name)
	
def parse_arguments(argv):
	parser = argparse.ArgumentParser()

	parser.add_argument('image_file', 
			type=str, 
			help='The image file name (.txt)')
	parser.add_argument('labels_file', 
			type=str, 
			help='The label file name (.txt)')
	parser.add_argument('lmdb_img_name', 
			type=str, 
			help='The image lmdb file to save (.txt)')
	parser.add_argument('lmdb_label_name', 
			type=str, 
			help='The label lmdb file to save (.txt)')
	return parser.parse_args(argv)

if __name__ == '__main__':
	main(parse_arguments(sys.argv[1:]))
