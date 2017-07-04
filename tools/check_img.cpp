#include <stdio.h>  // for snprintf
#include <string>
#include <vector>
#include <iostream>
#include <sstream> // stringstream
#include <fstream> // NOLINT (readability /streams)
#include <utility> // Create key-value pair (there could be not used)

#include <stdlib.h>
#include <sys/stat.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>


using namespace std;
using namespace cv;
using std::ofstream;
using std::string;



int main(int argc, char** argv) {

  
  // save labels
  std::string save_label_dataset_names(argv[1]);
  //std::cout<<" save_label_dataset_names:"<< save_label_dataset_names.c_str()<<endl;
  int num_mini_batches = 0;
 /*
 * save key_str as filename  "00001.jpg" or "00001" 
 * LOG(ERROR)<<"path :"<<argv[++arg_pos];
 */

 std::ifstream infile(argv[2]);
 std::string filename;
 std::vector<string> lines;
 int label;
 
 //生成新的测试txt文件
 std::ofstream fileout(save_label_dataset_names.c_str(),std::ios::out|std::ios::app);
 int idx = 0;
 while(infile>>filename>>label)
{
  // std::cout<<idx<<" "<<filename<<" "<<label<<std::endl;
   idx ++;
    //读取图片，局部变量
	Mat check_cv_img = imread(filename);
	if(  !check_cv_img.empty())
	  {
		//当图片为可以读的情况下，重新存储
		fileout<<filename<<" "<<label<<"\n";
		num_mini_batches++;  
	}
	//check_cv_img.release();
  }
  fileout.close();
  infile.close();
  std::cout<<num_mini_batches<<std::endl;
  cout<< "ok !";
  return 0;
}

