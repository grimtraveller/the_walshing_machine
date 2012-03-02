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

  exit(EXIT_SUCCESS);
}