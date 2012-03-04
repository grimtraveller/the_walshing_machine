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

  //  // push new input into our buffer
  //  for (int i = 0; i < kNumInputs; ++i)
  //    for (int j = 0; j < sampleFrames; ++j)
  //      input_buffer_[i].push_back(static_cast<double>(inputs[i][j]));

  //  // for each of the inputs...
  //  for (int i = 0; i < kNumInputs; ++i)
  //  {
  //    // if we have anything remaining in our output buffer, use it, and that's all we'll do
  //    // also, if we don't have enough input to process, do the same
  //    if (!output_buffer_[i].empty() || input_buffer_[i].size() < GetWindowSize())
  //    {
  //      // we have to fill the output entirely
  //      for (unsigned int j = 0; j < static_cast<unsigned int>(sampleFrames); ++j)
  //      {
  //        // start by setting to 0
  //        outputs[i][j] = 0;

  //        // if we have enough in the buffer, use the buffer value
  //        if (output_buffer_[i].size() > j)
  //          outputs[i][j] = static_cast<T>(output_buffer_[i][j]);
  //      }

  //      // erase the lesser of what we used or what we required to fill the output
  //      output_buffer_[i].erase(
  //        output_buffer_[i].begin(), 
  //        output_buffer_[i].begin() + std::min<unsigned int>(sampleFrames, output_buffer_[i].size()));

  //      // we're done with this input!
  //      continue;
  //    }

  //    // we should now be able to fill the output with the output buffer
  //    for (int j = 0; j < sampleFrames; ++j)
  //      outputs[i][j] = static_cast<T>(output_buffer_[i][j]);

  //    // and erase what we just used
  //    output_buffer_[i].erase(output_buffer_[i].begin(), output_buffer_[i].begin() + sampleFrames);
  //  }
  //}

  //// set output to a 440Hz wave
  //VstTimeInfo* time_info = getTimeInfo(NULL);
  //for (int i = 0; i < kNumOutputs; ++i)
  //  for (int j = 0; j < sampleFrames; ++j)
  //    outputs[i][j] = static_cast<T>(sin(2 * M_PI * (time_info->samplePos + j) / time_info->sampleRate * 440));

  return;
}

