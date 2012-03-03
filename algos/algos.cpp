#include <cstdint>
#include <cstdlib>

#include "fwht.h"

static const uint8_t power_ = 2;

static float input_ [1<<power_] = { 0.f, 1.f, 2.f, 3.f };
static float output_[1<<power_] = { 0 }; 

int main(int argc, char* argv[])
{
  // do the sequency ordered walsh hadamard transform
  fwht::SequencyOrdered<float, float>(input_, power_, output_);

  // distort by removing a component
  output_[3] = 0;

  // perform the inverse, to end up back where we started
  fwht::SequencyOrderedInverse<float, float>(output_, power_, input_);

  exit(EXIT_SUCCESS);
}