#include <algorithm>
#include <audioeffectx.h>
#include <vector>

class WalshingMachine : public AudioEffectX
{
public:
  WalshingMachine(audioMasterCallback audioMaster, VstInt32 numPrograms, VstInt32 numParams) 
    : AudioEffectX(audioMaster, numPrograms, numParams) 
  {
	  setNumInputs(kNumInputs);   // stereo in
	  setNumOutputs(kNumOutputs); // stereo out
	  setUniqueID('ThWM');        // you must change this for other plug-ins!
  	canProcessReplacing();      // supports replacing mode
    canDoubleReplacing();       // supports double replacing mode

    // set default parameter values
    setParameter(kWinSize, 0.0f);
    setParameter(kAmount,  0.1f);
    setParameter(kDryWet,  1.0f);
  }
   
  enum Params
  {
    kWinSize,
    kAmount,
    kDryWet,
    kNumParams
  };

	// Return the value of the parameter with index
  virtual float getParameter(VstInt32 index) 
  { return params_[index]; }

 	// Called when a parameter changed
  virtual void setParameter(VstInt32 index, float value) 
  {
    params_[index] = value; 

    // if we changed the window size, we have a new initial delay
    switch (index)
    {
    case kWinSize:
      // set our initial delay (the latency) based on the window
      // size versus the processing block size
      // make sure it's not negative!
      unsigned int delay = std::max<int>(GetWindowSize() - getBlockSize(), 0);
      setInitialDelay(delay);
      ioChanged();
      break;
    }
  }

  // Stuff label with the units in which parameter index is displayed (i.e. "sec", "dB", "type", etc...). Limited to #kVstMaxParamStrLen. 	
  virtual void getParameterLabel(VstInt32 index, char* label)
  {
    switch (index)
    {
    case kWinSize: strcpy_s(label, kVstMaxParamStrLen, ""); break;
    case kAmount:  strcpy_s(label, kVstMaxParamStrLen, "%"); break;
    case kDryWet:  strcpy_s(label, kVstMaxParamStrLen, "%"); break;
    }
  }	

  // Stuff text with a string representation ("0.5", "-3", "PLATE", etc...) of the value of parameter index. Limited to #kVstMaxParamStrLen.
  virtual void getParameterDisplay(VstInt32 index, char* text) 
  {
    switch (index)
    {
    case kWinSize: int2string(GetWindowSize(), text, kVstMaxParamStrLen); break;
    case kAmount:  float2string(params_[kAmount] * 100, text, kVstMaxParamStrLen); break;
    case kDryWet:  float2string(params_[kDryWet] * 100, text, kVstMaxParamStrLen); break;
    }
  }

	// Stuff text with the name ("Time", "Gain", "RoomType", etc...) of parameter index. Limited to #kVstMaxParamStrLen.
  virtual void getParameterName (VstInt32 index, char* text)    
  {
    switch (index)
    {
    case kWinSize: strcpy_s(text, kVstMaxParamStrLen, "WinSize"); break;
    case kAmount:  strcpy_s(text, kVstMaxParamStrLen, "Amount");  break;
    case kDryWet:  strcpy_s(text, kVstMaxParamStrLen, "Dry/Wet"); break;
    }
  }	

  // Process 32 bit (single precision) floats (always in a resume state)
  virtual void processReplacing(float** inputs, float** outputs, VstInt32 sampleFrames);
  
  // Process 64 bit (double precision) floats (always in a resume state)
  virtual void processDoubleReplacing(double** inputs, double** outputs, VstInt32 sampleFrames);

	// Called when plug-in is switched to on
  virtual void resume()
  {
    // Clear the windows, which will get filled by the process calls
    for (int i = 0; i < kNumInputs; ++i)
    {
      // input buffer starts empty
      input_buffer_[i].clear();

      // output buffer starts with a window full of zeros
      // for outputting while we fill the input buffer
      // we subtract the block size, because on the last frame,
      // we'll receive enough to complete a full analysis on the window,
      // so we need output only for the n-1 frames before that frame
      output_buffer_[i] = std::vector<double>(GetWindowSize() - getBlockSize());
    }
  }

  enum Programs
  {
    kNumPrograms
  };

private:

  static const int kNumInputs  = 2;
  static const int kNumOutputs = 2;

  // our actual parameter values
  float params_[kNumParams];

  // our window size min and max power-of-2
  // 0  corresponds to 2^0  = 1
  // 14 corresponds to 2^14 = 16384
  static const int kMinWinPower = 0;
  static const int kMaxWinPower = 14;

  // get the window size power based on the window size parameter
  int GetWindowPower() { return static_cast<int>(params_[kWinSize] * (kMaxWinPower - kMinWinPower) + kMinWinPower + 0.5); }

  // get the window size based on the window size parameter
  int GetWindowSize()  { return 1 << GetWindowPower(); };

  // this is called by both processReplacing and processDoubleReplacing
  template <typename T> 
  void process(T** inputs, T** outputs, VstInt32 sampleFrames);

  // perform the actual work
  template <typename TIn, typename TOut>
  void walsh(TIn* input, TOut* output);

  // coefficients and sorted coefficients that have enough room for our max window size
  double coeffs_       [1<<kMaxWinPower];
  double sorted_coeffs_[1<<kMaxWinPower];

  // our working window
  // this fills up with the input, and when it's at least
  // as large as our window size, we can start some output!
  // the output buffer holds the output, so that when we're waiting
  // to fill up again, we can use it as output
  std::vector<double> input_buffer_[kNumInputs];
  std::vector<double> output_buffer_[kNumInputs];
};