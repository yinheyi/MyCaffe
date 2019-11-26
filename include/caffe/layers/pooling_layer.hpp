#ifndef CAFFE_POOLING_LAYER_HPP_
#define CAFFE_POOLING_LAYER_HPP_

#include <vector>

#include "caffe/blob.hpp"
#include "caffe/layer.hpp"
#include "caffe/proto/caffe.pb.h"

namespace caffe {

/**
 * @brief Pools the input image by taking the max, average, etc. within regions.
 *
 * TODO(dox): thorough documentation for Forward, Backward, and proto params.
 */
template <typename Dtype>
class PoolingLayer : public Layer<Dtype> {
 public:
  explicit PoolingLayer(const LayerParameter& param)
      : Layer<Dtype>(param) {}
  virtual void LayerSetUp(const vector<Blob<Dtype>*>& bottom,
      const vector<Blob<Dtype>*>& top);
  virtual void Reshape(const vector<Blob<Dtype>*>& bottom,
      const vector<Blob<Dtype>*>& top);

  virtual inline const char* type() const { return "Pooling"; }
  virtual inline int ExactNumBottomBlobs() const { return 1; }
  virtual inline int MinTopBlobs() const { return 1; }

  // MAX POOL layers can output an extra top blob for the mask;
  // others can only output the pooled inputs.
  // top[1]的作用与成员变量max_idx_或rand_idex_的作用是相同的，都是保存在pooling过程中，pooling的输出是哪一个pooling的输入元素。
  virtual inline int MaxTopBlobs() const {
    return (this->layer_param_.pooling_param().pool() ==
            PoolingParameter_PoolMethod_MAX) ? 2 : 1;
  }

 protected:
  virtual void Forward_cpu(const vector<Blob<Dtype>*>& bottom,
      const vector<Blob<Dtype>*>& top);
  virtual void Forward_gpu(const vector<Blob<Dtype>*>& bottom,
      const vector<Blob<Dtype>*>& top);
  virtual void Backward_cpu(const vector<Blob<Dtype>*>& top,
      const vector<bool>& propagate_down, const vector<Blob<Dtype>*>& bottom);
  virtual void Backward_gpu(const vector<Blob<Dtype>*>& top,
      const vector<bool>& propagate_down, const vector<Blob<Dtype>*>& bottom);

  int kernel_h_, kernel_w_;
  int stride_h_, stride_w_;
  int pad_h_, pad_w_;
  int channels_;
  int height_, width_;    // pooling 区域的高与宽。
  int pooled_height_, pooled_width_;    // pooling后的输出的高与宽。
  bool global_pooling_;   // 如果是global_pooling的话，就是在整个w * h上进行pooling，输出一个元素。
  PoolingParameter_RoundMode round_mode_;    // 当pooling的输出不是整数时，进行floor()还是ceil().

  Blob<int> max_idx_;   // 在最大值pooling过程中，保存pooling后的输出是哪一个输入。
  Blob<Dtype> rand_idx_; // 在随机pooling过程中，保存pooling后的输出是哪一个输入。
};

}  // namespace caffe

#endif  // CAFFE_POOLING_LAYER_HPP_
