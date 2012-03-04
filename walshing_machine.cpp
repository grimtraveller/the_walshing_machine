#define _USE_MATH_DEFINES

#include <algorithm>
#include <cmath>

#include "algos/fwht.h"
#include "walshing_machine.h"

void WalshingMachine::processReplacing(float** inputs, float** outputs, VstInt32 sampleFrames)
{ process<float>(inputs, outputs, sampleFrames); }

void WalshingMachine::processDoubleReplacing(double** inputs, double** outputs, VstInt32 sampleFrames)
{ process<double>(inputs, outputs, sampleFrames); }

template <typename T> 
void WalshingMachine::process(T** inputs, T** outputs, VstInt32 sampleFrames)
{
  // push new input into our buffer
  for (int i = 0; i < kNumInputs; ++i)
    for (int j = 0; j < sampleFrames; ++j)
      input_buffer_[i].push_back(static_cast<double>(inputs[i][j]));

  // for each of the inputs...
  for (int i = 0; i < kNumInputs; ++i)
  {
    // if we have anything remaining in our output buffer, use it, and that's all we'll do
    // also, if we don't have enough input to process, do the same
    if (!output_buffer_[i].empty() || input_buffer_[i].size() < GetWindowSize())
    {
      // we have to fill the output entirely
      for (unsigned int j = 0; j < static_cast<unsigned int>(sampleFrames); ++j)
      {
        // start by setting to 0
        outputs[i][j] = 0;

        // if we have enough in the buffer, use the buffer value
        if (output_buffer_[i].size() > j)
          outputs[i][j] = static_cast<T>(output_buffer_[i][j]);
      }

      // erase the lesser of what we used or what we required to fill the output
      output_buffer_[i].erase(
        output_buffer_[i].begin(), 
        output_buffer_[i].begin() + std::min<unsigned int>(sampleFrames, output_buffer_[i].size()));

      // we're done with this input!
      continue;
    }

    // we have enough to process, so go for it!
    for (int j = 0; j < sampleFrames; j += GetWindowSize())
    {
      // set up a place to put the transform results
      std::vector<double> coeffs(GetWindowSize());

      // perform the transform
      fwht::SequencyOrdered<double, double>(&*input_buffer_[i].begin(), GetWindowPower(), &*coeffs.begin());

      // create an array for sorting
      // which consists of the absolute value of the coefficient
      // we don't care about its +- value
      std::vector<double> sorted(coeffs);
      for (unsigned int k = 0; k < sorted.size(); ++k)
        sorted[k] = std::abs(sorted[k]);

      // perform an nth-element sort to find the element that's at the correct index
      int sorted_idx = static_cast<int>(params_[kAmount] * (GetWindowSize() - 1));
      std::nth_element(sorted.begin(), sorted.begin() + sorted_idx, sorted.end());
      
      // get the value that we need to be "bigger than" to be kept
      double comp_val = sorted[sorted_idx];

      // remove a number of lower elements based on the amount
      for (unsigned int k = 0; k < coeffs.size(); ++k)
        coeffs[k] *= std::abs(coeffs[k]) >= comp_val;

      // make room in the output buffer
      output_buffer_[i].resize(GetWindowSize());

      // invert back to the output buffer
      fwht::SequencyOrderedInverse<double, double>(&*coeffs.begin(), GetWindowPower(), &*output_buffer_[i].begin());

      // chop off the stuff we just worked on
      // which will be the window size
      input_buffer_[i].erase(input_buffer_[i].begin(), input_buffer_[i].begin() + GetWindowSize());
    }
    
    // we should now be able to fill the output with the output buffer
    for (int j = 0; j < sampleFrames; ++j)
      outputs[i][j] = static_cast<T>(output_buffer_[i][j]);
    
    // and erase what we just used
    output_buffer_[i].erase(output_buffer_[i].begin(), output_buffer_[i].begin() + sampleFrames);
  }

  //// set output to a 440Hz wave
  //VstTimeInfo* time_info = getTimeInfo(NULL);
  //for (int i = 0; i < kNumOutputs; ++i)
  //  for (int j = 0; j < sampleFrames; ++j)
  //    outputs[i][j] = static_cast<T>(sin(2 * M_PI * (time_info->samplePos + j) / time_info->sampleRate * 440));

  return;
}

