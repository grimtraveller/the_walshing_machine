#include <audioeffectx.h>

#include "walshing_machine.h"

AudioEffect* createEffectInstance(audioMasterCallback audioMaster)
{
  return new WalshingMachine(audioMaster, WalshingMachine::kNumPrograms, WalshingMachine::kNumParams);
}