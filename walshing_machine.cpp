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
      buffer_[i].push_back(static_cast<double>(inputs[i][j]));

  // if our window is the correct size, do the processing!
  for (int i = 0; i < kNumInputs; ++i)
  {
    // take a copy of the output channel pointer, because we want to use it without
    // affecting the original
    T* channel_output = outputs[i];

    // keep operating on the window until we run out of stuff
    // this will happen if the window size is less than the block size
    while (static_cast<int>(buffer_[i].size()) >= GetWindowSize())
    {
      // perform the transform
      fwht::SequencyOrdered<double, double>(&*buffer_[i].begin(), GetWindowPower(), &*transformed_[i].begin());

      // remove the max element
      std::vector<double>::iterator max_el = std::max_element(transformed_[i].begin(), transformed_[i].end());
      *max_el = 0;

      // invert back to the output
      fwht::SequencyOrderedInverse<double, T>(&*transformed_[i].begin(), GetWindowPower(), channel_output);

      // chop off the stuff we just worked on
      buffer_[i].erase(buffer_[i].begin(), buffer_[i].begin() + GetWindowSize());

      // move the output pointer along so we fill up the next window size
      channel_output += GetWindowSize();
    }
 }

  //// set output to a 440Hz wave
  //VstTimeInfo* time_info = getTimeInfo(NULL);
  //for (int i = 0; i < kNumOutputs; ++i)
  //  for (int j = 0; j < sampleFrames; ++j)
  //    outputs[i][j] = static_cast<T>(sin(2 * M_PI * (time_info->samplePos + j) / time_info->sampleRate * 440));

  return;
}

