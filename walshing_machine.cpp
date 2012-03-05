#define _USE_MATH_DEFINES

#include <algorithm>
#include <cmath>

#include "algos/fwht.h"
#include "walshing_machine.h"

void WalshingMachine::processReplacing(float** inputs, float** outputs, VstInt32 sampleFrames)
{ process<float>(inputs, outputs, sampleFrames); }

void WalshingMachine::processDoubleReplacing(double** inputs, double** outputs, VstInt32 sampleFrames)
{ process<double>(inputs, outputs, sampleFrames); }

template <typename TIn, typename TOut>
void WalshingMachine::walsh(TIn* input, TOut* output)
{
  // get the window size
  int win_pow  = GetWindowPower();
  int win_size = GetWindowSize();
  
  // perform the transform
  fwht::SequencyOrdered<TIn, double>(input, win_pow, coeffs_);

  // perform the filtering by zeroing out bins below the high pass and above the low pass
  // since idx * sample_rate / 2 / win_size = Freq,
  // idx = freq * 2 * win_size / sample_rate
  int hp_cut_idx = static_cast<int>((win_size<<1) * FilterToHz(params_[kHPFreq]) / getSampleRate());
  int lp_cut_idx = static_cast<int>((win_size<<1) * FilterToHz(params_[kLPFreq]) / getSampleRate());
  for (int k = 0; k < win_size; ++k)
    coeffs_[k] *= (k >= hp_cut_idx) && (k <= lp_cut_idx);

  // create the sorted coeffs, which consist of the absolute value of the coefficient
  // we don't care about its +- value
  for (int k = 0; k < win_size; ++k)
    sort_coeffs_[k] = Coeff(k, coeffs_[k]);

  // sort the coeffs
  std::sort(sort_coeffs_, sort_coeffs_ + win_size);

  // convert the amount to make the knob more active
  double adj_amount = pow(static_cast<double>(params_[kLoss]), static_cast<double>(1) / kAmountRoot);

  // choose how many to remove
  int remove = static_cast<int>(adj_amount * (win_size - 1));

  // set the amplitude to 0 for the ones to remove
  // we don't want to set everything to 0, because then we lose the indices
  for (int i = 0; i < remove; ++i)
    sort_coeffs_[i].val = 0;

  // copy back to the coeffs
  for (int k = 0; k < win_size; ++k)
    coeffs_[sort_coeffs_[k].idx] = sort_coeffs_[k].val;

  // perform the normalization
  
  // get the sum of the absolute coefficients
  double sum = 0;
  for (int k = 0; k < win_size; ++k)
    sum += std::abs(coeffs_[k]);

  // if we have full normalization, we divide all coefficients by the sum 
  // to make them sum to 1. if we have no normalization, we leave them as they are
  // (or divide by 1)
  double div = 1 * (1 - params_[kNormliz]) + sum * params_[kNormliz];
  for (int k = 0; k < win_size; ++k)
    coeffs_[k] /= div;

  // invert back to the output buffer
  fwht::SequencyOrderedInverse<double, TOut>(coeffs_, win_pow, output);
}

template <typename T> 
void WalshingMachine::process(T** inputs, T** outputs, VstInt32 sampleFrames)
{
  // TWO CASES:
  
  // 1. sampleFrames is >= our window size
  // In this case, we don't need a running buffer or anything -- we have all the information
  // we need present within the input
  if (sampleFrames >= GetWindowSize())
  {
    // step through the sampleFrames based on our window size
    // and perform the walsh
    // place the output into the output buffer so that we can weight the results based on the dry/wet
    for (int i = 0; i < kNumInputs; ++i)
      for (int j = 0; j < sampleFrames; j += GetWindowSize())
        walsh<T, double>(inputs[i] + j, output_buf_[i] + j);

    // set the output using the dry/wet
    for (int i = 0; i < kNumInputs; ++i)
      for (int j = 0; j < sampleFrames; ++j)
        outputs[i][j] = static_cast<T>(
                        inputs[i][j]      * (1-params_[kDryWet]) + 
                        output_buf_[i][j] *    params_[kDryWet]);
  }

  // 2. sampleFrames is < our window size
  else
  {
    for (int i = 0; i < kNumInputs; ++i)
    {
      // shift our buffer back by sampleFrames
      memmove(input_buf_[i], input_buf_[i] + sampleFrames, sampleFrames * sizeof *input_buf_[i]);

      // add the new input onto the end of our buffer
      // can't do a memcpy because we don't know input type
      for (int j = 0; j < sampleFrames; ++j)
        input_buf_[i][GetWindowSize() - sampleFrames + j] = inputs[i][j];

      // perform the walsh into the output buffer
      walsh<double, double>(input_buf_[i], output_buf_[i]);

      // now we cherry pick only the most "recent" data from the output buffer
      // and stick that into the output
      // we also use the dry-wet control to weight the output
      for (int j = 0; j < sampleFrames; ++j)
        outputs[i][j] = inputs[i][j]                                                       * (1-params_[kDryWet]) + 
                        static_cast<T>(output_buf_[i][GetWindowSize() - sampleFrames + j]) *    params_[kDryWet];
    }
  }

  //// set output to a 440Hz wave
  //VstTimeInfo* time_info = getTimeInfo(NULL);
  //for (int i = 0; i < kNumOutputs; ++i)
  //  for (int j = 0; j < sampleFrames; ++j)
  //    outputs[i][j] = static_cast<T>(sin(2 * M_PI * (time_info->samplePos + j) / time_info->sampleRate * 440));

  return;
}

