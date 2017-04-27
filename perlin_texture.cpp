#define TEXTURE_SIZE 128

float
Lerp(float V0, float V1, float T)
{
  return (1.f - T) * V0 + T * V1;
}

static char CharPermutation[255];
void
InitCharPermutation(int Seed)
{
  srand(Seed);
  for(int CharIndex = 0; CharIndex < 255; ++CharIndex)
  {
    CharPermutation[CharIndex] = (char)CharIndex;
  }

#define ITERATIONS 3000
  for(int Iteration = 0; Iteration < ITERATIONS; ++Iteration)
  {
    char Temp;
    int RandomIndex1 = rand() % 255;
    int RandomIndex2 = rand() % 255;

    Temp = CharPermutation[RandomIndex1];
    CharPermutation[RandomIndex1] = CharPermutation[RandomIndex2];
    CharPermutation[RandomIndex2] = Temp;
  }
}

float
ComputePerlinInfluence(int Hash, float X, float Y)
{
  switch(Hash & 0x3)
  {
    case 0x0: return X + Y;
    case 0x1: return -X + Y;
    case 0x2: return X - Y;
    case 0x3: return -X - Y;
// Note: Jokes
#define THEOMEGAVARIABLE 3.14f
    default: return THEOMEGAVARIABLE;
  }
}

float
Fade(float T)
{
  return T * T * T * (T * (T * 6 - 15) + 10);
}

float
ComputePerlinNoise(float X, float Y)
{
  while (X > 255)
  {
    X = X - 255;
  }
  while (Y > 255)
  {
    Y = Y - 255;
  }

  int XI = (int)X & 255;
  int YI = (int)Y & 255;

  float XF = X - (int)X;
  float YF = Y - (int)Y;

  float U = Fade(XF);
  float V = Fade(YF);

  int AA = CharPermutation[(CharPermutation[XI] + YI) % 255];
  int AB = CharPermutation[(CharPermutation[XI] + YI + 1) % 255];
  int BA = CharPermutation[(CharPermutation[(XI + 1) % 255] + YI) % 255];
  int BB = CharPermutation[(CharPermutation[(XI + 1) % 255] + YI + 1) % 255];

  float V1 = Lerp(ComputePerlinInfluence(AA, XF, YF),
                  ComputePerlinInfluence(BA, XF - 1, YF),
                  U);
  float V2 = Lerp(ComputePerlinInfluence(AB, XF, YF - 1),
                  ComputePerlinInfluence(BB, XF - 1, YF -1),
                  U);

  float VV = (Lerp(V1, V2, V) + 1)/2;

  return VV;
}

static float TextureData[TEXTURE_SIZE][TEXTURE_SIZE][3];
void
Generate2DPerlinNoise(GLuint TextureID, int Seed)
{
  InitCharPermutation(Seed);

  for(int Row = 0; Row < TEXTURE_SIZE; ++Row)
  {
    for(int Col = 0; Col < TEXTURE_SIZE; ++Col)
    {
      TextureData[Row][Col][0] =
        TextureData[Row][Col][1] =
        TextureData[Row][Col][2] =
        ComputePerlinNoise((float)Col/2, (float)Row/2);
    }
  }

  glBindTexture(GL_TEXTURE_2D, TextureID);
  {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, TEXTURE_SIZE, TEXTURE_SIZE, 0,
                 GL_RGB, GL_FLOAT, (GLvoid*)TextureData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);
  } glBindTexture(GL_TEXTURE_2D, 0);
}
