#include <vector>

#include "caffe/layers/split_layer.hpp"
#include "caffe/util/math_functions.hpp"

namespace caffe {

template <typename Dtype>
void SplitLayer<Dtype>::Reshape(const vector<Blob<Dtype>*>& bottom,
      const vector<Blob<Dtype>*>& top) {
  count_ = bottom[0]->count();
  for (int i = 0; i < top.size(); ++i) {
    // Do not allow in-place computation in the SplitLayer.  Instead, share data
    // by reference in the forward pass, and keep separate diff allocations in
    // the backward pass.  (Technically, it should be possible to share the diff
    // blob of the first split output with the input, but this seems to cause
    // some strange effects in practice...)
    CHECK_NE(top[i], bottom[0]) << this->type() << " Layer does not "
        "allow in-place computation.";
    top[i]->ReshapeLike(*bottom[0]);
    CHECK_EQ(count_, top[i]->count());
  }
}

// 正向传播，top块直接sharebottom块就OK，也就是只复制了指针。
template <typename Dtype>
void SplitLayer<Dtype>::Forward_cpu(const vector<Blob<Dtype>*>& bottom,
      const vector<Blob<Dtype>*>& top) {
  for (int i = 0; i < top.size(); ++i) {
    top[i]->ShareData(*bottom[0]);
  }
}

// 梯度的反向传播，把top块的diff累加起来就是bottom块的diff了。
template <typename Dtype>
void SplitLayer<Dtype>::Backward_cpu(const vector<Blob<Dtype>*>& top,
      const vector<bool>& propagate_down, const vector<Blob<Dtype>*>& bottom) {
  if (!propagate_down[0]) { return; }

  if (top.size() == 1) {
    caffe_copy(count_, top[0]->cpu_diff(), bottom[0]->mutable_cpu_diff());
    return;
  }

  // 1. 先执行：bottom_diff = top[0]_diff + top[1]_diff
  caffe_add(count_, top[0]->cpu_diff(), top[1]->cpu_diff(), bottom[0]->mutable_cpu_diff());
  // 2. 再累加top[2]_diff,.....top[i]_diff的值到bottom_diff.
  for (int i = 2; i < top.size(); ++i) {
    const Dtype* top_diff = top[i]->cpu_diff();
    Dtype* bottom_diff = bottom[0]->mutable_cpu_diff();
    // 执行累加操作： bottom_diff = 1.0 * top_diff + bottom_diff
    caffe_axpy(count_, Dtype(1.), top_diff, bottom_diff);
  }
}


#ifdef CPU_ONLY
STUB_GPU(SplitLayer);
#endif

INSTANTIATE_CLASS(SplitLayer);
REGISTER_LAYER_CLASS(Split);

}  // namespace caffe
