#include <stdio.h>  // for snprintf
#include <string>
#include <vector>
#include <iostream>
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

int main(int argc, char** argv) {
  return feature_extraction_pipeline<float>(argc, argv);
//  return feature_extraction_pipeline<double>(argc, argv);
}

template<typename Dtype>
int feature_extraction_pipeline(int argc, char** argv) {
  ::google::InitGoogleLogging(argv[0]);
  const int num_required_args = 7;
  if (argc < num_required_args) {
    LOG(ERROR)<<
    "This program takes in a trained network and an input data layer, and then"
    " extract features of the input data produced by the net.\n"
    "Usage: extract_features  pretrained_net_param"
    "  feature_extraction_proto_file  extract_feature_blob_name1[,name2,...]"
    "  save_features_txt_name num_mini_batches"
    "  the path of test images file list\n"
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
  boost::shared_ptr<Net<Dtype> > feature_extraction_net(
      new Net<Dtype>(feature_extraction_proto, caffe::TEST));
  feature_extraction_net->CopyTrainedLayersFrom(pretrained_binary_proto);
  
  // get feature layer name
  std::string extract_feature_blob_names(argv[++arg_pos]);
  std::vector<std::string> blob_names;
  boost::split(blob_names, extract_feature_blob_names, boost::is_any_of(","));
  
  // save features
  std::string save_feature_txt_names(argv[++arg_pos]);
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
 std::vector<int> label_;
 int label;
 while(infile>>filename>>label){
    lines.push_back(filename);
    label_.push_back(label);
 }
 LOG(ERROR)<<"Extracting Features of "<< lines.size()<<" images";

  /*
  * ofstream
  * save image features
  */

  std::ofstream fileout(save_feature_txt_names.c_str(),std::ios::app);
 

  std::vector<Blob<float>*> input_vec;
  std::vector<int> image_indices(num_features, 0);
 
  float ymax = 1; //归一化数据范围
  float ymin = 0;  

  for (int batch_index = 0; batch_index < num_mini_batches; ++batch_index) {
    feature_extraction_net->Forward(input_vec);
    for (int i = 0; i < num_features; ++i) {
      const boost::shared_ptr<Blob<Dtype> > feature_blob = feature_extraction_net
          ->blob_by_name(blob_names[i]);
      int batch_size = feature_blob->num();
      int dim_features = feature_blob->count() / batch_size;
      const Dtype* feature_blob_data;
      for (int n = 0; n < batch_size; ++n) {
        feature_blob_data = feature_blob->cpu_data() +
            feature_blob->offset(n);
	fileout<<label_[image_indices[i]]<<" ";

        std::vector<float> features;
        for (int d = 0; d < dim_features; ++d) 
	  features.push_back(feature_blob_data[d]);
	//特征归一化
	float dMaxValue = *max_element(features.begin(),features.end());  //求最大值
	//std::cout<<"maxdata"<<dMaxValue<<'\n';
 	float dMinValue = *min_element(features.begin(),features.end());  //求最小值
	//std::cout<<"mindata"<<dMinValue<<'\n';
        for (int f = 0; f < features.size(); ++f) {
	  features[f] = (ymax-ymin)*(features[f]-dMinValue)/(dMaxValue-dMinValue)+ymin;	  
 	  //极小值限制
	  fileout<<f+1<<":"<< features[f]+1e-8<<" "; 
        }
	fileout<<"\n";   
	features.clear();//删除容器

        ++image_indices[i];
        if (image_indices[i] % 1000 == 0) {
          LOG(ERROR)<< "Save features of " << image_indices[i] <<
              " images for layers blob " << blob_names[i];
        }
      }  // for (int n = 0; n < batch_size; ++n)
    }  // for (int i = 0; i < num_features; ++i)
  }  // for (int batch_index = 0; batch_index < num_mini_batches; ++batch_index)
  // write the last batch
  
  for (int i = 0; i < num_features; ++i) {
    LOG(ERROR)<< "Save features of " << image_indices[i] <<
        " images for layers blob " << blob_names[i];
  }
  
  // close file
  fileout.close(); 
  infile.close();

  LOG(ERROR)<< "Successfully extracted the features!";
  return 0;
}

