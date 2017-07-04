#include <stdio.h>  // for snprintf
#include <string>
#include <vector>
#include <iostream>
#include<iomanip>
#include <sstream> // stringstream
#include <fstream> // NOLINT (readability /streams)
#include <utility> // Create key-value pair (there could be not used)

#include <stdlib.h>
#include <sys/stat.h>

#include "boost/algorithm/string.hpp"
#include "google/protobuf/text_format.h"

#include "caffe/blob.hpp"
#include "caffe/common.hpp"
#include "caffe/net.hpp"
#include "caffe/proto/caffe.pb.h"
#include "caffe/util/db.hpp"
#include "caffe/util/io.hpp"
#include "caffe/util/format.hpp"

using namespace std;
using std::ofstream;
using caffe::Blob;
using caffe::Caffe;
using caffe::Datum;
using caffe::Net;
using boost::shared_ptr;
using std::string;
namespace db = caffe::db;

template<typename Dtype>
int feature_extraction_pipeline(int argc, char** argv);

//比较函数
static bool PairCompare(const std::pair<float, int>& lhs,const std::pair<float, int>& rhs); 

//实现Argmax函数，默认返回top1
//static std::vector<int> Argmax(const std::vector<float>& v, int N=1);


int main(int argc, char** argv) {
  return feature_extraction_pipeline<float>(argc, argv);
//  return feature_extraction_pipeline<double>(argc, argv);
}

template<typename Dtype>
int feature_extraction_pipeline(int argc, char** argv) {
  ::google::InitGoogleLogging(argv[0]);
  const int num_required_args = 9;
  if (argc < num_required_args) {
    LOG(ERROR)<<
    "This program takes in a trained network and an input data layer, and then"
    " extract features of the input data produced by the net.\n"
    "Usage: extract_features  pretrained_net_param"
    "  feature_extraction_proto_file  extract_feature_blob_name1[,name2,...]"
    "  save_label_dataset_name num_mini_batches"
    "  the path of test images file list\n"
    "  class foldpath class_number"
    "  [CPU/GPU] [DEVICE_ID=0]\n";
    return 1;
  }
  int arg_pos = num_required_args;

  arg_pos = num_required_args;
  if (argc > arg_pos && strcmp(argv[arg_pos], "GPU") == 0) {
    LOG(ERROR)<< "Using GPU";
    uint device_id = 0;
    if (argc > arg_pos + 1) {
      device_id = atoi(argv[arg_pos + 1]);
      CHECK_GE(device_id, 0);
    }
    LOG(ERROR) << "Using Device_id=" << device_id;
    Caffe::SetDevice(device_id);
    Caffe::set_mode(Caffe::GPU);
  } else {
    LOG(ERROR) << "Using CPU";
    Caffe::set_mode(Caffe::CPU);
  }
  
  // the name of the executable
  arg_pos = 0;  
  std::string pretrained_binary_proto(argv[++arg_pos]);

  std::string feature_extraction_proto(argv[++arg_pos]);
  shared_ptr<Net<Dtype> > feature_extraction_net(
      new Net<Dtype>(feature_extraction_proto, caffe::TEST));
  feature_extraction_net->CopyTrainedLayersFrom(pretrained_binary_proto);
  
  // get label layer name
  std::string extract_feature_blob_names(argv[++arg_pos]);
  std::vector<std::string> blob_names;
  boost::split(blob_names, extract_feature_blob_names, boost::is_any_of(","));
  
  // save labels
  std::string save_label_dataset_names(argv[++arg_pos]);
  
  size_t num_features = blob_names.size();

  for (size_t i = 0; i < num_features; i++) {
    CHECK(feature_extraction_net->has_blob(blob_names[i]))
        << "Unknown feature blob name " << blob_names[i]
        << " in the network " << feature_extraction_proto;
  }
  
 // mini batches
  int num_mini_batches = atoi(argv[++arg_pos]);

 /*
 * save key_str as filename  "00001.jpg" or "00001" 
 * LOG(ERROR)<<"path :"<<argv[++arg_pos];
 */

 std::ifstream infile(argv[++arg_pos]);
 std::string filename;
 std::vector<string> lines;
 int label;
 while(infile>>filename>>label){
    //int pos = filename.find_last_of('/');
    //std::string fname(filename.substr(pos+1)); 
    //lines.push_back(fname);
    lines.push_back(filename);
 }
  LOG(ERROR)<<"A total of "<< lines.size()<<" images";


 /*
 *check the folds and create the folds
 */
 
 std::string foldpath(argv[++arg_pos]);
 int num_class_sum = atoi(argv[++arg_pos]);

 //构造label集合
 std::vector<int> label_;
 for (int i = 0; i < num_class_sum; ++i)
    label_.push_back(i);
 
 //创建文件夹
 for(int f=1;f<=num_class_sum;f++)
 {
   std::stringstream stream_temp;
   stream_temp << f;
   std::string temp = stream_temp.str();
   std::string foldname = foldpath+temp;
   //LOG(ERROR)<<"foldname = "<<foldname;
   if(access(foldname.c_str(),0)==-1)
   {
     if(mkdir(foldname.c_str(),0777)){
    	std::cout<<"create file bag failed!"<<endl;
     }
   }
  
 }  

  /*
  * ofstream
  * save image path and labels
  */

  std::ofstream fileout(save_label_dataset_names.c_str(),std::ios::app);
  Datum datum;

  std::vector<Blob<float>*> input_vec;
  std::vector<int> image_indices(num_features, 0);
  for (int batch_index = 0; batch_index < num_mini_batches; ++batch_index) {
    feature_extraction_net->Forward(input_vec);
    for (int i = 0; i < num_features; ++i) {
      const shared_ptr<Blob<Dtype> > feature_blob = feature_extraction_net
          ->blob_by_name(blob_names[i]);
      int batch_size = feature_blob->num();
      int dim_features = feature_blob->count() / batch_size;
      const Dtype* feature_blob_data;
      for (int n = 0; n < batch_size; ++n) {
        datum.set_height(feature_blob->height());
        datum.set_width(feature_blob->width());
        datum.set_channels(feature_blob->channels());
        datum.clear_data();
        datum.clear_float_data();
        feature_blob_data = feature_blob->cpu_data() +
            feature_blob->offset(n);
	//自己实现Argmax函数，构造pair对
	std::vector<float>Pro_prediction;
	for (int d = 0; d < dim_features; ++d) {
	    //全部将数据存入vector
	    Pro_prediction.push_back(feature_blob_data[d]);
	}
	//返回最大值
	//  std::vector<int> maxN =Argmax(Pro_prediction,1);
	int N = 1;
	//自己实现Argmax函数，返回max score 的位置
	//static std::vector<int> Argmax(const std::vector<float>& v, int N=1) {
  	std::vector<std::pair<float, int> > pairs;// 存入值和顺序
  	for (size_t ii = 0; ii < Pro_prediction.size(); ++ii)
   	   pairs.push_back(std::make_pair(Pro_prediction[ii], ii));//利用make_pair 存入pairs
  
  	// partial_sort排序
 	std::partial_sort(pairs.begin(), pairs.begin() + N, pairs.end(), PairCompare);

  	std::vector<int> maxN;
  	for (int j = 0; j < N; ++j)
    	    maxN.push_back(pairs[j].second);

        int idx_max = maxN[0];
	float idx_score = Pro_prediction[idx_max];
	//std::cout<<"idx_max  ="<<idx_max<<endl;
        //std::cout<<"idx_score	="<<idx_score<<endl;
	//LOG(ERROR)<<"dim_fea="<<dim_features<<",label="<<feature_blob_data[d];
	fileout<<lines[image_indices[i]]<<" "<<label_[idx_max]<<"\n";

	//save the imags to the right location(5 classes)
	for(int f=1;f<=num_class_sum;f++){
	    if(f==((int)label_[idx_max]+1)){
 	       std::string sour_path = lines[image_indices[i]];
       	       int sour_pos = lines[image_indices[i]].find_last_of('/');
    	       std::string sour_name(lines[image_indices[i]].substr(sour_pos+1));
	       std::stringstream label_temp;
   	       label_temp << ((int)label_[idx_max]+1);
   	       //std::string temp = label_temp.str();
               //添加score值
 	       std::stringstream score_temp;
   	       //控制存储位数为小数点后4位
   	       char tPath[5];
   	       sprintf(tPath, "%.4f", idx_score);
   	       score_temp << (tPath);
   	       //score_temp <<(idx_score);
               //合并字符串
		std::string dist_path = foldpath +label_temp.str()+"/"+ score_temp.str()+"_"+sour_name;
		std::string final_path = "cp "+ sour_path +" "+dist_path;
		system(final_path.c_str());
        // std::cout<<"sour_path = "<<sour_path.c_str()<<endl;
	// std::cout<< "score_temp = "<<score_temp.str()<<endl;  
	// std::cout<<"dist_path =  "<<dist_path.c_str()<<endl;  
        // std::cout<<"final_path = "<<final_path.c_str()<<endl;  
	    
	     }
	  } 
	
        ++image_indices[i];
        if (image_indices[i] % 1000 == 0) {
          LOG(ERROR)<< "Save labels of " << image_indices[i] <<
              " images for layers blob " << blob_names[i];
        }
      }  // for (int n = 0; n < batch_size; ++n)
    }  // for (int i = 0; i < num_features; ++i)
  }  // for (int batch_index = 0; batch_index < num_mini_batches; ++batch_index)
  // write the last batch
  
  for (int jj = 0; jj < num_features; ++jj) {
    LOG(ERROR)<< "Save labels of " << image_indices[jj] <<
        " images for layers blob " << blob_names[jj];
  }
  
  // close file
  fileout.close(); 
  
  LOG(ERROR)<< "ok !";
  return 0;
}


//比较函数
static bool PairCompare(const std::pair<float, int>& lhs,
                        const std::pair<float, int>& rhs) {
  return lhs.first > rhs.first;
}


