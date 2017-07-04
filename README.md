# caffe-multiTags
a caffe version for multiTags job

## input data format:
+ 7 labels for all; 1 for the correct label and 0 for the incorrect label;
```
folder/sample001.jpg 0 1
folder/sample001.jpg 2 3 5
folder/sample001.jpg 1 6 3
folder/sample001.jpg 4 6 1 2
folder/sample001.jpg 6 3 2 1 0
...
```
### Reference to this article：
+ [caffe 实现多标签输入（multilabel、multitask）](http://blog.csdn.net/hubin232/article/details/50960201)

### makeRecord.md
+ This script records the bug encountered during the compilation of “make”

### tools/convert_imageset.cpp
```
$CAFFE_HOME/build/tools/convert_imageset -resize_height=224 -resize_width=224 / train_images.txt train_data_lmdb 7
```
### models example——inceptionV2 model
+ googlenet_train.prototxt                      :model prototxt
+ multilabel_googlenet_iter_250000.caffemodel   :model weights
+ solver.prototxt                               :solver prototxt
+ runtrain.sh                                   :run script

### accuracy.py
+ compute accuracy for the last layer
+ before run the model, you need :
```
export PYTHONPATH=$CAFFE_HOME
```
 
