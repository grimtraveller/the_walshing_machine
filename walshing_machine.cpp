#define _USE_MATH_DEFINES

#include <algorithm>
#include <cmath>

#include "algos/fwht.h"
#include "walshing_machine.h"

void WalshingMachine::processReplacing(float** inputs, float** outputs, VstInt32 sampleFrames)
{ process<float>(inputs, outputs, sampleFrames); }

void WalshingMachine::processDoubleReplacing(double** inputs, double** outputs, VstInt32 sampleFrames)
{ process<double>(inputs, outputs, sampleFrames); }

template <typename TIn, typename TOut>
void WalshingMachine::walsh(TIn* input, TOut* output)
{
  // get the window size
  int win_pow  = GetWindowPower();
  int win_size = GetWindowSize();
  
  // perform the transform
  fwht::SequencyOrdered<TIn, double>(input, win_pow, coeffs_);

  // create the sorted coeffs, which consist of the absolute value of the coefficient
  // we don't care about its +- value
  for (int k = 0; k < win_size; ++k)
    sorted_coeffs_[k] = std::abs(coeffs_[k]);

  // perform an nth-element sort to find the element that's at the correct index
  int sorted_idx = static_cast<int>(params_[kAmount] * (win_size - 1));
  std::nth_element(sorted_coeffs_, sorted_coeffs_ + sorted_idx, sorted_coeffs_ + win_size);

  // get the value that we need to be "bigger than" to be kept
  double comp_val = sorted_coeffs_[sorted_idx];

  // remove a number of lower elements based on the amount
  for (int k = 0; k < win_size; ++k)
    coeffs_[k] *= std::abs(coeffs_[k]) >= comp_val;

  // invert back to the output buffer
  fwht::SequencyOrderedInverse<double, TOut>(coeffs_, win_pow, output);
}

template <typename T> 
void WalshingMachine::process(T** inputs, T** outputs, VstInt32 sampleFrames)
{
  // TWO CASES:
  
  // 1. sampleFrames is >= our window size
  // In this case, we don't need a running buffer or anything -- we have all the information
  // we need present within the input
  if (sampleFrames >= GetWindowSize())
  {
    // step through the sampleFrames based on our window size
    // and perform the walsh
    for (int i = 0; i < kNumInputs; ++i)
      for (int j = 0; j < sampleFrames; j += GetWindowSize())
        walsh<T, T>(inputs[i] + j, outputs[i] + j);
  }

  // 2. sampleFrames is < our window size
  else
  {
    for (int i = 0; i < kNumInputs; ++i)
    {
      // shift our buffer back by sampleFrames
      memmove(input_buf_[i], input_buf_[i] + sampleFrames, sampleFrames * sizeof *input_buf_[i]);

      // add the new input onto the end of our buffer
      // can't do a memcpy because we don't know input type
      for (int j = 0; j < sampleFrames; ++j)
        input_buf_[i][GetWindowSize() - sampleFrames + j] = inputs[i][j];

      // perform the walsh into the output buffer
      walsh<double, double>(input_buf_[i], output_buf_[i]);

      // now we cherry pick only the most "recent" data from the output buffer
      // and stick that into the output
      for (int j = 0; j < sampleFrames; ++j)
        outputs[i][j] = static_cast<T>(output_buf_[i][GetWindowSize() - sampleFrames + j]);
    }
  }

  //// set output to a 440Hz wave
  //VstTimeInfo* time_info = getTimeInfo(NULL);
  //for (int i = 0; i < kNumOutputs; ++i)
  //  for (int j = 0; j < sampleFrames; ++j)
  //    outputs[i][j] = static_cast<T>(sin(2 * M_PI * (time_info->samplePos + j) / time_info->sampleRate * 440));

  return;
}

