#include <audioeffectx.h>

class WalshingMachine : public AudioEffectX
{
public:
  WalshingMachine(audioMasterCallback audioMaster, VstInt32 numPrograms, VstInt32 numParams) 
    : AudioEffectX(audioMaster, numPrograms, numParams) {}

	///< Process 32 bit (single precision) floats (always in a resume state)
  virtual void processReplacing(float** inputs, float** outputs, VstInt32 sampleFrames); 

  enum Params
  {
    kDryWet,
    kNumParams
  };

  enum Programs
  {
    kNumPrograms
  };
};