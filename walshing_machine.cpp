#define _USE_MATH_DEFINES
#include <cmath>

#include "walshing_machine.h"

void WalshingMachine::processReplacing(float** inputs, float** outputs, VstInt32 sampleFrames)
{
  process<float>(inputs, outputs, sampleFrames);
}

void WalshingMachine::processDoubleReplacing(double** inputs, double** outputs, VstInt32 sampleFrames)
{
  process<double>(inputs, outputs, sampleFrames);
}

template <typename T> 
void WalshingMachine::process(T** inputs, T** outputs, VstInt32 sampleFrames)
{
  // get our time info
  VstTimeInfo* time_info = getTimeInfo(NULL);

  // don't do anything if we're not playing...
  if (!(time_info->flags & kVstTransportPlaying))
    return;

  // set output to a 440Hz wave
  for (int i = 0; i < kNumOutputs; ++i)
    for (int j = 0; j < sampleFrames; ++j)
      outputs[i][j] = sin(2 * M_PI * (time_info->samplePos + j) / time_info->sampleRate * 440);

  return;
}

