#include <algorithm>
#include <audioeffectx.h>

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
    params_[kWinSize] = 0.5;
    params_[kDryWet]  = 1.0;

    // set our initial delay (the latency) based on the window
    // size versus the processing block size
    // make sure it's not negative!
    setInitialDelay(std::max(GetWindowSize() - getBlockSize(), 0));
  }
   
  enum Params
  {
    kWinSize,
    kDryWet,
    kNumParams
  };

	// Return the value of the parameter with index
  virtual float getParameter(VstInt32 index) 
  { return params_[index]; }

 	// Called when a parameter changed
  virtual void setParameter(VstInt32 index, float value) 
  { params_[index] = value; }

  // Stuff label with the units in which parameter index is displayed (i.e. "sec", "dB", "type", etc...). Limited to #kVstMaxParamStrLen. 	
  virtual void getParameterLabel(VstInt32 index, char* label)
  {
    switch (index)
    {
    case kWinSize: strcpy_s(label, kVstMaxParamStrLen, ""); break;
    case kDryWet:  strcpy_s(label, kVstMaxParamStrLen, "%"); break;
    }
  }	

  // Stuff text with a string representation ("0.5", "-3", "PLATE", etc...) of the value of parameter index. Limited to #kVstMaxParamStrLen.
  virtual void getParameterDisplay(VstInt32 index, char* text) 
  {
    switch (index)
    {
    case kWinSize: int2string(GetWindowSize(), text, kVstMaxParamStrLen); break;
    case kDryWet:  float2string(params_[kDryWet] * 100, text, kVstMaxParamStrLen); break;
    }
  }

	// Stuff text with the name ("Time", "Gain", "RoomType", etc...) of parameter index. Limited to #kVstMaxParamStrLen.
  virtual void getParameterName (VstInt32 index, char* text)    
  {
    switch (index)
    {
    case kWinSize: strcpy_s(text, kVstMaxParamStrLen, "WinSize"); break;
    case kDryWet:  strcpy_s(text, kVstMaxParamStrLen, "Dry/Wet"); break;
    }
  }	

  // Process 32 bit (single precision) floats (always in a resume state)
  virtual void processReplacing(float** inputs, float** outputs, VstInt32 sampleFrames);
  
  // Process 64 bit (double precision) floats (always in a resume state)
  virtual void processDoubleReplacing(double** inputs, double** outputs, VstInt32 sampleFrames);

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

  template <typename T> 
  void process(T** inputs, T** outputs, VstInt32 sampleFrames);

};