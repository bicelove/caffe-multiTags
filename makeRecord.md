#### 参考多标签文件修改原文：[caffe 实现多标签输入（multilabel、multitask）](http://blog.csdn.net/hubin232/article/details/50960201)
#### 修改编译中存在的问题记录
##### 1、image_data_layer.cpp
+ 159行
+ 原文	prefetch_label[item_id*label_size + label_id] = lines_[lines_id_].second[label_		：后面的看不清楚;
+ 我修改的 	prefetch_label[item_id*label_size + label_id] = lines_[lines_id_].second[label_id];

##### 2、memory_data_layer.cpp
+ 78行
+ 原文	top_label[item_id * label_size_ + label_id] = labels[item_id * label_size_ + la		：后面的看不清楚;
+ 我修改的	top_label[item_id * label_size_ + label_id] = labels[item_id * label_size_ + label_id];

#### 以下为make test时报错后的修改，原文未提及
##### 3、memory_data_layer.cpp
+ 50行
+ 源码：   	top_label[item_id] = datum_vector[item_id].label();	
+ 我修改的    	top_label[item_id] = datum_vector[item_id].label(0);
+ 因编译错误： 	cannot convert 'const google::protobuf::RepeatedField<int>' to 'double' in assignment
+ 修改参考：	[](https://stackoverflow.com/questions/32874292/converting-googleprotobufrepeatedfieldfloat-to-float)
##### 4、test_data_layer.cpp
+ 51及117行
+ 源码：	 datum.set_label(i);
+ 我修改的：      datum.set_label(0, i);
+ 因编译错误：	
```
error: no matching function for call to 'caffe::Datum::set_label(const int&)' datum->set_label(i);
.build_release/src/caffe/proto/caffe.pb.h:11322:13: note: void caffe::Datum::set_label(int, google::protobuf::int32)
 inline void Datum::set_label(int index, ::google::protobuf::int32 value) {
             ^
.build_release/src/caffe/proto/caffe.pb.h:11322:13: note:   candidate expects 2 arguments, 1 provided

```
##### 5、test_data_transformer.cpp
+ 20行
+ 源码：	datum->set_label(label);
+ 我修改的：	datum->set_label(0 ,label);
+ 因编译错误：	
``` 
error: no matching function for call to 'caffe::Datum::set_label(const int&)'
   datum->set_label(label);
.build_release/src/caffe/proto/caffe.pb.h:11322:13: note: void caffe::Datum::set_label(int, google::protobuf::int32)
 inline void Datum::set_label(int index, ::google::protobuf::int32 value) {
             ^
.build_release/src/caffe/proto/caffe.pb.h:11322:13: note:   candidate expects 2 arguments, 1 provided
```
##### 6、test_db.cpp
+ 35行
+ 源码：	     ReadImageToDatum(root_images_ + keys[i], labels, &datum);
+ 我修改的：		
```
for (int i = 0; i < 2; ++i) {
      Datum datum;
      std::vector<int> labels;
      labels.push_back(i);
      ReadImageToDatum(root_images_ + keys[i], labels, &datum);
      string out;
      CHECK(datum.SerializeToString(&out));
      txn->Put(keys[i], out);
    }
```
+ 因编译错误：
```
src/caffe/test/test_db.cpp: In member function 'virtual void caffe::DBTest<TypeParam>::SetUp()':
src/caffe/test/test_db.cpp:35:57: error: no matching function for call to 'ReadImageToDatum(std::basic_string<char>&, int&, caffe::Datum*&)'
       ReadImageToDatum(root_images_ + keys[i], i, &datum);
                                                         ^
src/caffe/test/test_db.cpp:35:57: note: candidates are:
In file included from src/caffe/test/test_db.cpp:10:0:
./include/caffe/util/io.hpp:99:6: note: bool caffe::ReadImageToDatum(const string&, std::vector<int>, int, int, bool, const string&, caffe::Datum*)
 bool ReadImageToDatum(const string& filename, const vector<int> label,
      ^
./include/caffe/util/io.hpp:99:6: note:   candidate expects 7 arguments, 3 provided
./include/caffe/util/io.hpp:103:13: note: bool caffe::ReadImageToDatum(const string&, std::vector<int>, int, int, bool, caffe::Datum*)
 inline bool ReadImageToDatum(const string& filename, const vector<int> label,
             ^
./include/caffe/util/io.hpp:103:13: note:   candidate expects 6 arguments, 3 provided
./include/caffe/util/io.hpp:109:13: note: bool caffe::ReadImageToDatum(const string&, std::vector<int>, int, int, caffe::Datum*)
 inline bool ReadImageToDatum(const string& filename, const vector<int> label,
             ^
./include/caffe/util/io.hpp:109:13: note:   candidate expects 5 arguments, 3 provided
./include/caffe/util/io.hpp:114:13: note: bool caffe::ReadImageToDatum(const string&, std::vector<int>, bool, caffe::Datum*)
 inline bool ReadImageToDatum(const string& filename, const vector<int> label,
             ^
./include/caffe/util/io.hpp:114:13: note:   candidate expects 4 arguments, 3 provided
./include/caffe/util/io.hpp:119:13: note: bool caffe::ReadImageToDatum(const string&, std::vector<int>, caffe::Datum*)
 inline bool ReadImageToDatum(const string& filename, const vector<int> label,
             ^
./include/caffe/util/io.hpp:119:13: note:   no known conversion for argument 2 from 'int' to 'std::vector<int>'
./include/caffe/util/io.hpp:124:13: note: bool caffe::ReadImageToDatum(const string&, std::vector<int>, const string&, caffe::Datum*)
 inline bool ReadImageToDatum(const string& filename, const vector<int> label,
             ^
./include/caffe/util/io.hpp:124:13: note:   candidate expects 4 arguments, 3 provided
```
+ 127及131行
+ 源码：    ReadFileToDatum(this->root_images_ + "cat.jpg", 0, &datum);	ReadFileToDatum(this->root_images_ + "fish-bike.jpg", 1, &datum);
+ 我修改的：		
```
  vector<int> labels;
  labels.resize(1, 0);
  ReadFileToDatum(this->root_images_ + "cat.jpg", labels, &datum);
  string out;
  CHECK(datum.SerializeToString(&out));
  txn->Put("cat.jpg", out);
  labels.resize(1, 1);
  ReadFileToDatum(this->root_images_ + "fish-bike.jpg", labels, &datum);
```
+ 因编译错误：
```
src/caffe/test/test_db.cpp:127:60: error: no matching function for call to 'ReadFileToDatum(std::basic_string<char>, int, caffe::Datum*)'
   ReadFileToDatum(this->root_images_ + "cat.jpg", 0, &datum);
                                                            ^
src/caffe/test/test_db.cpp:127:60: note: candidates are:
In file included from src/caffe/test/test_db.cpp:10:0:
./include/caffe/util/io.hpp:93:6: note: bool caffe::ReadFileToDatum(const string&, std::vector<int>, caffe::Datum*)
 bool ReadFileToDatum(const string& filename, const vector<int> label, Datum* datum);
      ^
./include/caffe/util/io.hpp:93:6: note:   no known conversion for argument 2 from 'int' to 'std::vector<int>'
./include/caffe/util/io.hpp:95:13: note: bool caffe::ReadFileToDatum(const string&, caffe::Datum*)
 inline bool ReadFileToDatum(const string& filename, Datum* datum) {
             ^
./include/caffe/util/io.hpp:95:13: note:   candidate expects 2 arguments, 3 provided
src/caffe/test/test_db.cpp:131:66: error: no matching function for call to 'ReadFileToDatum(std::basic_string<char>, int, caffe::Datum*)'
   ReadFileToDatum(this->root_images_ + "fish-bike.jpg", 1, &datum);
                                                                  ^
src/caffe/test/test_db.cpp:131:66: note: candidates are:
In file included from src/caffe/test/test_db.cpp:10:0:
./include/caffe/util/io.hpp:93:6: note: bool caffe::ReadFileToDatum(const string&, std::vector<int>, caffe::Datum*)
 bool ReadFileToDatum(const string& filename, const vector<int> label, Datum* datum);
      ^
./include/caffe/util/io.hpp:93:6: note:   no known conversion for argument 2 from 'int' to 'std::vector<int>'
./include/caffe/util/io.hpp:95:13: note: bool caffe::ReadFileToDatum(const string&, caffe::Datum*)
 inline bool ReadFileToDatum(const string& filename, Datum* datum) {
             ^
./include/caffe/util/io.hpp:95:13: note:   candidate expects 2 arguments, 3 provided
src/caffe/test/test_db.cpp: In instantiation of 'void caffe::DBTest_TestWrite_Test<gtest_TypeParam_>::TestBody() [with gtest_TypeParam_ = caffe::TypeLevelDB]':
src/caffe/test/test_db.cpp:137:1:   required from here
src/caffe/test/test_db.cpp:127:60: error: no matching function for call to 'ReadFileToDatum(std::basic_string<char>, int, caffe::Datum*)'
   ReadFileToDatum(this->root_images_ + "cat.jpg", 0, &datum);
                                                            ^
src/caffe/test/test_db.cpp:127:60: note: candidates are:
In file included from src/caffe/test/test_db.cpp:10:0:
./include/caffe/util/io.hpp:93:6: note: bool caffe::ReadFileToDatum(const string&, std::vector<int>, caffe::Datum*)
 bool ReadFileToDatum(const string& filename, const vector<int> label, Datum* datum);
      ^
./include/caffe/util/io.hpp:93:6: note:   no known conversion for argument 2 from 'int' to 'std::vector<int>'
./include/caffe/util/io.hpp:95:13: note: bool caffe::ReadFileToDatum(const string&, caffe::Datum*)
 inline bool ReadFileToDatum(const string& filename, Datum* datum) {
             ^
./include/caffe/util/io.hpp:95:13: note:   candidate expects 2 arguments, 3 provided
src/caffe/test/test_db.cpp:131:66: error: no matching function for call to 'ReadFileToDatum(std::basic_string<char>, int, caffe::Datum*)'
   ReadFileToDatum(this->root_images_ + "fish-bike.jpg", 1, &datum);
                                                                  ^
src/caffe/test/test_db.cpp:131:66: note: candidates are:
In file included from src/caffe/test/test_db.cpp:10:0:
./include/caffe/util/io.hpp:93:6: note: bool caffe::ReadFileToDatum(const string&, std::vector<int>, caffe::Datum*)
 bool ReadFileToDatum(const string& filename, const vector<int> label, Datum* datum);
      ^
./include/caffe/util/io.hpp:93:6: note:   no known conversion for argument 2 from 'int' to 'std::vector<int>'
./include/caffe/util/io.hpp:95:13: note: bool caffe::ReadFileToDatum(const string&, caffe::Datum*)
 inline bool ReadFileToDatum(const string& filename, Datum* datum) {
             ^
./include/caffe/util/io.hpp:95:13: note:   candidate expects 2 arguments, 3 provided
make: *** [.build_release/src/caffe/test/test_db.o] Error 1

```

##### 7、test_io.cpp
+ 20行
+ 源码：bool ReadImageToDatumReference(const string& filename, int label, const int height, const int width, const bool is_color, Datum* datum) {
+ 修改为： bool ReadImageToDatumReference(const string& filename, const vector<int> label, const int height, const int width, const bool is_color, Datum* datum) {
+ 之后所有调用ReadImageToDatumReference的地方第二个参数都从int改为vector<int>
+ 41行
+ 源码： datum->set_label(label);
+ 修改后：
```
 datum->mutable_label()->Clear();
  for(int label_i = 0; label_i < label.size(); label_i++){
  	datum->add_label(label[label_i]);
  }
```
+ 77、102行及所有涉及到ReadImageToDatum的行，第二个参数都改为vector<int>类型
+ 源码：	       ReadImageToDatum(filename, 0, 0, 0, true, &datum);
+ 我修改的：
```
  vector<int> labels;
  labels.resize(1, 0);
  ReadImageToDatum(filename, labels, 0, 0, true, &datum);
```
+ 编译错误同上
+ 310行
+ 源码：	 EXPECT_EQ(datum.label(0), -1);
+ 修改后：	 EXPECT_EQ(datum.label(), -1);
+ 因编译错误：
```
CXX src/caffe/test/test_io.cpp
In file included from src/caffe/test/test_io.cpp:9:0:
./src/gtest/gtest.h: In instantiation of 'testing::AssertionResult testing::internal::CmpHelperEQ(const char*, const char*, const T1&, const T2&) [with T1 = google::protobuf::RepeatedField<int>; T2 = int]':
./src/gtest/gtest.h:18378:30:   required from 'static testing::AssertionResult testing::internal::EqHelper<lhs_is_null_literal>::Compare(const char*, const char*, const T1&, const T2&) [with T1 = google::protobuf::RepeatedField<int>; T2 = int; bool lhs_is_null_literal = false]'
src/caffe/test/test_io.cpp:310:3:   required from here
./src/gtest/gtest.h:18341:16: error: no match for 'operator==' (operand types are 'const google::protobuf::RepeatedField<int>' and 'const int')
   if (expected == actual) {
                ^
./src/gtest/gtest.h:18341:16: note: candidate is:
./src/gtest/gtest.h:9167:6: note: template<class T> bool testing::internal::operator==(T*, const testing::internal::linked_ptr<T>&)
 bool operator==(T* ptr, const linked_ptr<T>& x) {
      ^
./src/gtest/gtest.h:9167:6: note:   template argument deduction/substitution failed:
./src/gtest/gtest.h:18341:16: note:   mismatched types 'T*' and 'google::protobuf::RepeatedField<int>'
   if (expected == actual) {
                ^
```

##### 8、test_memory_data_layer.cpp:
+ 140行
+ 源码： 	datum_vector[i].set_label(i);
+ 修改后：	datum_vector[i].set_label(0, i);
+ 因编译错误：
```
src/caffe/test/test_memory_data_layer.cpp: In member function 'virtual void caffe::MemoryDataLayerTest_AddDatumVectorDefaultTransform_Test<gtest_TypeParam_>::TestBody()':
src/caffe/test/test_memory_data_layer.cpp:140:32: error: no matching function for call to 'caffe::Datum::set_label(int&)'
     datum_vector[i].set_label(i);
                                ^
src/caffe/test/test_memory_data_layer.cpp:140:32: note: candidate is:
In file included from ./include/caffe/blob.hpp:9:0,
                 from ./include/caffe/filler.hpp:10,
                 from src/caffe/test/test_memory_data_layer.cpp:8:
.build_release/src/caffe/proto/caffe.pb.h:11322:13: note: void caffe::Datum::set_label(int, google::protobuf::int32)
 inline void Datum::set_label(int index, ::google::protobuf::int32 value) {
             ^
.build_release/src/caffe/proto/caffe.pb.h:11322:13: note:   candidate expects 2 arguments, 1 provided
make: *** [.build_release/src/caffe/test/test_memory_data_layer.o] Error 1
```
