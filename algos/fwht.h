#include <cstdint>
#include <vector>

namespace fwht
{
  // adapted from: http://graphics.stanford.edu/~seander/bithacks.html
  inline uint32_t ReverseBits(uint32_t v, int bits)
  {
    unsigned int r = v & 1;
    while (bits--)
    {   
      v >>= 1;
      r <<= 1;
      r |= v & 1;
    }
    return r;
  }

  template <typename TIn, typename TOut>
  void SequencyOrderedInverse(TIn* input, int power_of_two, TOut* output)
  {
    // get the starting values
    int N  = 1 << power_of_two;
    int k1 = N;
    int k2 = 1;
    int k3 = N >> 1;

    // create an array of indices
    std::vector<uint32_t> indices(N, 0);
    for (int i = 0; i < N; ++i)
      indices[i] = i;

    // we only need to reverse the bits if we have more than one index!
    if (power_of_two > 0)
    {
      // reverse the bits in each index
      for (std::vector<uint32_t>::iterator it = indices.begin();
        it != indices.end(); ++it)
        *it = ReverseBits(*it, power_of_two - 1);
    }

    // use those indices to create a rearranged input array
    // as the first pass at the output array
    for (int i = 0; i < N; ++i)
      output[i] = static_cast<TOut>(input[indices[i]]);

    // in-place iteration begins here 
    for (int i1 = 0; i1 < power_of_two; ++i1)
    {
      int L1 = 1;

      for (int i2 = 0; i2 < k2; ++i2)
      {
        for (int i3 = 0; i3 < k3; ++i3)
        {
          int i = i3 + L1 - 1; 
          int j = i + k3;

          // get the values from the input vector
          TOut temp1 = output[i];
          TOut temp2 = output[j]; 

          if (i2 % 2)
          {
            output[i] = temp1 - temp2;
            output[j] = temp1 + temp2;
          }
          else
          {
            output[i] = temp1 + temp2;
            output[j] = temp1 - temp2;
          }
        }

        L1 += k1;
      }

      k1 >>= 1;
      k2 <<= 1;
      k3 >>= 1;
    }
  }

  template <typename TIn, typename TOut>
  void SequencyOrdered(TIn* input, int power_of_two, TOut* output)
  {
    // Do the inverse
    SequencyOrderedInverse(input, power_of_two, output);

    // Then also do the part we're supposed to "remove" for the inverse transform!
    // Remove this for inverse transform
    int N  = 1 << power_of_two;
    for (int i = 0; i < N; ++i)
      output[i] /= N;
  }
}