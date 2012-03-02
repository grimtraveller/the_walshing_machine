#include <cstdint>
#include <vector>

static uint8_t power_ = 2;
static float   input_[] = { 0.f, 1.f, 2.f, 3.f };

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
  // get the starting values
  int N = 1 << power_;
  int k1 = N;
  int k2 = 1;
  int k3 = N >> 1;

  // create an array of indices
  std::vector<uint32_t> indices;
  for (int i = 0; i < N; ++i)
    indices.push_back(i);

  // reverse the bits in each index
  for (std::vector<uint32_t>::iterator it = indices.begin();
    it != indices.end(); ++it)
    *it = ReverseBits(*it, power_ - 1);

  // use those indices to create a rearranged data array
  std::vector<float> x;
  for (int i = 0; i < N; ++i)
    x.push_back(input_[indices[i]]);

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
        float temp1 = x[i];
        float temp2 = x[j]; 

        if (i2 % 2)
        {
          x[i] = temp1 - temp2;
          x[j] = temp1 + temp2;
        }
        else
        {
          x[i] = temp1 + temp2;
          x[j] = temp1 - temp2;
        }
      }

      L1 += k1;
    }

    k1 >>= 1;
    k2 <<= 1;
    k3 >>= 1;
  }
  
  // Remove this for inverse transform
  for (int i = 0; i < N; ++i)
    x[i] /= N;

  exit(EXIT_SUCCESS);
}