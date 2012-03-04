#include <algorithm>
#include <audioeffectx.h>
#include <vector>
#include <Windows.h> // for Beep

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

	// Returns tail size; 0 is default (return 1 for 'no tail'), used in offline processing too
  // We return the maximum window size because we don't want our buffer filled with
  // junk if we adjust our position within the track. We'll see if this makes a difference...
  virtual VstInt32 getGetTailSize() 
  { return 1<<kMaxWinPower; }

	// Return the value of the parameter with index
  virtual float getParameter(VstInt32 index) 
  { return params_[index]; }

 	// Called when a parameter changed
  virtual void setParameter(VstInt32 index, float value) 
  {
    params_[index] = value; 
  
    // if we're changing the window size, reset the buffer
    switch (index)
    {
    case kWinSize: 
      for (int i = 0; i < kNumInputs; ++i)
        memset(input_buf_[i], 0, sizeof input_buf_[i]); 
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

  //// Called when plug-in is initialized
  // virtual void open()
  // { Beep(4000, 100); }		

  //// Called when plug-in will be released
  // virtual void close() 
  // { Beep(3000, 100); }	

  //// Called when plug-in is switched to off
  // virtual void suspend()
  // { Beep(2000, 100); }	

  //// Called when plug-in is switched to on
  //virtual void resume()
  //{ Beep(1000, 100); }

  //// Called one time before the start of process call. This indicates that the process call will be interrupted (due to Host reconfiguration or bypass state when the plug-in doesn't support softBypass)
  //virtual VstInt32 startProcess() 
  //{ Beep(500, 100);  return 0; }		

  //// Called after the stop of process call
  // virtual VstInt32 stopProcess() 
  // { Beep(200, 100); return 0; }

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
  // 1  corresponds to 2^1  = 2
  // 14 corresponds to 2^14 = 16384
  static const int kMinWinPower = 1;
  static const int kMaxWinPower = 14;

  // get the window size power based on the window size parameter
  int GetWindowPower() { return static_cast<int>(params_[kWinSize] * (kMaxWinPower - kMinWinPower) + kMinWinPower + 0.5); }

  // get the window size based on the window size parameter
  int GetWindowSize()  { return 1<<GetWindowPower(); };

  // this is called by both processReplacing and processDoubleReplacing
  template <typename T> 
  void process(T** inputs, T** outputs, VstInt32 sampleFrames);

  // perform the actual work
  template <typename TIn, typename TOut>
  void walsh(TIn* input, TOut* output);

  // a special sortable structure
  // we use it so that we can maintain the original index after sorting
  struct Coeff
  {
    int    idx;
    double val;

    Coeff() : idx(0), val(0) {}
    Coeff(int idx, double val) : idx(idx), val(val) {}
    inline int operator <(const Coeff& other) { return std::abs(val) < std::abs(other.val); }
  };

  // coefficients that have enough room for our max window size
  // has a special type so that when it's sorted, it's still obvious
  // which index it came from originally
  double coeffs_[1<<kMaxWinPower];
  Coeff  sort_coeffs_[1<<kMaxWinPower];

  // an input buffer for when we work on windows larger than the number of sample frames
  // we need to keep past information to do things properly
  double input_buf_ [kNumInputs][1<<kMaxWinPower];

  // an output buffer, because our normal output is only of size sampleFrames, but we
  // need to calculate output for the whole window and then copy only the "good" data
  // into the output
  double output_buf_[kNumInputs][1<<kMaxWinPower];
};