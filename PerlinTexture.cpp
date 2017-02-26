#define TEXTURE_SIZE 128

float lerp(float v0, float v1, float t)
{
  return (1.f - t) * v0 + t * v1;
}

static char charPermutation[255];
void initCharPermutation(int seed)
{
  srand(seed);
  for(int index = 0; index < 255; ++index)
  {
    charPermutation[index] = (char)index;
  }

#define ITERATIONS 3000
  for(int iteration = 0; iteration < ITERATIONS; ++iteration)
  {
    char temp;
    int randomIndex1 = rand() % 255;
    int randomIndex2 = rand() % 255;

    temp = charPermutation[randomIndex1];
    charPermutation[randomIndex1] = charPermutation[randomIndex2];
    charPermutation[randomIndex2] = temp;
  }
}

float computePerlinInfluence(int hash, float x, float y)
{
  switch(hash & 0x3)
  {
    case 0x0: return x + y;
    case 0x1: return -x + y;
    case 0x2: return x - y;
    case 0x3: return -x - y;
#define THEOMEGAVARIABLE 3.14f
    default: return THEOMEGAVARIABLE;
  }
}

float fade(float t)
{
  return t * t * t * (t * (t * 6 - 15) + 10);
}

float computePerlinNoise(float x, float y)
{
  while (x > 255)
  {
    x = x - 255;
  }
  while (y > 255)
  {
    y = y - 255;
  }

  int xi = (int)x & 255;
  int yi = (int)y & 255;

  float xf = x - (int)x;
  float yf = y - (int)y;

  float u = fade(xf);
  float v = fade(yf);

  int aa = charPermutation[(charPermutation[xi] + yi) % 255];
  int ab = charPermutation[(charPermutation[xi] + yi + 1) % 255];
  int ba = charPermutation[(charPermutation[(xi + 1) % 255] + yi) % 255];
  int bb = charPermutation[(charPermutation[(xi + 1) % 255] + yi + 1) % 255];

  float v1 = lerp(computePerlinInfluence(aa, xf, yf),
                  computePerlinInfluence(ba, xf - 1, yf),
                  u);
  float v2 = lerp(computePerlinInfluence(ab, xf, yf - 1),
                  computePerlinInfluence(bb, xf - 1, yf -1),
                  u);

  float vv = (lerp(v1, v2, v) + 1)/2;

  return vv;
}

static float textureData[TEXTURE_SIZE][TEXTURE_SIZE][3];

void generate2DPerlinNoise(GLuint textureID, int seed)
{
  initCharPermutation(seed);

  for(int row = 0; row < TEXTURE_SIZE; ++row)
  {
    for(int col = 0; col < TEXTURE_SIZE; ++col)
    {
      textureData[row][col][0] =
      textureData[row][col][1] =
        textureData[row][col][2] = computePerlinNoise((float)col/2, (float)row/2);
    }
  }

  glBindTexture(GL_TEXTURE_2D, textureID);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, TEXTURE_SIZE, TEXTURE_SIZE, 0,
               GL_RGB, GL_FLOAT, (GLvoid*)textureData);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glGenerateMipmap(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, 0);
}
