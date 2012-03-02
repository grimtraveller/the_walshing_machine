#include <cstdint>
#include <vector>

static uint8_t power_ = 12;

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

int main(int argc, char* argv[])
{
  // create an array of values
  std::vector<uint32_t> values;
  for (int i = 0; i < 4096; ++i)
    values.push_back(i);

  // reverse the bits in each one
  for (std::vector<uint32_t>::iterator it = values.begin();
    it != values.end(); ++it)
    *it = ReverseBits(*it, power_ - 1);

  // get the starting values
  int N = 1 << power_;
  int k1 = N;
  int k2 = 1;
  int k3 = N >> 1;

  // in-place iteration begins here 
  for (int i1 = 0; i1 < power_; ++i1)
  {
    int L1 = 1;

    for (int i2 = 0; i2 < k2; ++i2)
    {
      for (int i3 = 0; i3 < k3; ++i3)
      {
        int i = i3 + L1 - 1; 
        int j = i + k3;

        // get the values from the input vector
        uint32_t temp1 = values[i];
        uint32_t temp2 = values[j]; 

        if (i2 % 2)
        {
          values[i] = temp1 - temp2;
          values[j] = temp1 + temp2;
        }
        else
        {
          values[i] = temp1 + temp2;
          values[j] = temp1 - temp2;
        }
      }

      L1 += k1;
    }

    k1 >>= 1;
    k2 <<= 1;
    k3 >>= 1;
  }
  
  // Delete this line for inverse transform
  //x = inv(N) * x; 

  exit(EXIT_SUCCESS);
}