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
  }

  // Process 32 bit (single precision) floats (always in a resume state)
  virtual void processReplacing(float** inputs, float** outputs, VstInt32 sampleFrames); 
  
  // Process 64 bit (double precision) floats (always in a resume state)
  virtual void processDoubleReplacing(double** inputs, double** outputs, VstInt32 sampleFrames);

  enum Params
  {
    kDryWet,
    kNumParams
  };

  enum Programs
  {
    kNumPrograms
  };

private:

  template <typename T> 
  void process(T** inputs, T** outputs, VstInt32 sampleFrames);

  static const int kNumInputs  = 2;
  static const int kNumOutputs = 2;
};