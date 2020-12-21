// Copyright © Kirill Gavrilov, 2020
//
// rekheader is a small utility generating a REK file header (Fraunhofer EZRT Raw format)
// for a RAW Volume Data scan without any header.
//
// This code is licensed under MIT license (see LICENSE.txt for details).

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

//! REK file header structure definition.
struct RekFileHeader
{
  uint16_t SizeX;      //!< Size X in pixels
  uint16_t SizeY;      //!< Size Y in pixels
  uint16_t Pixel;      //!< 16 for WORD | 32 for FLOAT
  uint16_t SizeZ;      //!< Number of 2D volume slices
  uint8_t  Res1[572];  //!< Reserved
  float    SomeValue;  //!< Reserved
  float    PixelSize;  //!< Voxel size (in microns)
  float    SliceDist;  //!< Slice step (in microns)
  uint8_t  Res2[1456]; //!< Reserved
};

//! Print usage help.
static void printHelp()
{
  std::cout << "Usage: rekheader -i input.raw -o output.rek\n"
                "                 [-float32|-int16] -sizeX Size -sizeY Size -sizeZ Size\n"
                "                 -pixelSize Microns [-sliceStep Microns]\n"
                "Created by Kirill Gavrilov <kirill@sview.ru>\n";
}

int main (int theNbArgs, char** theArgVec)
{
  if (theNbArgs <= 1)
  {
    std::cerr << "Syntax error: wrong number of arguments\n";
    printHelp();
    return 1;
  }

  std::string anInput, anOutput;
  RekFileHeader aResHeader;
  memset (&aResHeader, 0, sizeof(aResHeader));
  for (int anArgiter = 1; anArgiter < theNbArgs; ++anArgiter)
  {
    std::string anArg (theArgVec[anArgiter]);
    for (char& aCharIter : anArg) { aCharIter = (char )tolower ((int )aCharIter); }
    if (anArg == "-help"
     || anArg == "--help")
    {
      printHelp();
      return 0;
    }
    else if (anArg == "-float"
          || anArg == "-float32")
    {
      aResHeader.Pixel = 32;
    }
    else if (anArg == "-int"
          || anArg == "-int16")
    {
      aResHeader.Pixel = 16;
    }
    else if (anArgiter + 1 < theNbArgs
          && (anArg == "-width"
           || anArg == "-sizex"
           || anArg == "-x"))
    {
      aResHeader.SizeX = (uint16_t )atoi (theArgVec[++anArgiter]);
    }
    else if (anArgiter + 1 < theNbArgs
          && (anArg == "-height"
           || anArg == "-sizey"
           || anArg == "-y"))
    {
      aResHeader.SizeY = (uint16_t )atoi (theArgVec[++anArgiter]);
    }
    else if (anArgiter + 1 < theNbArgs
          && (anArg == "-depth"
           || anArg == "-sizez"
           || anArg == "-z"))
    {
      aResHeader.SizeZ = (uint16_t )atoi (theArgVec[++anArgiter]);
    }
    else if (anArgiter + 1 < theNbArgs
          && (anArg == "-pixelsize"))
    {
      aResHeader.PixelSize = (float )atof (theArgVec[++anArgiter]);
      if (aResHeader.SliceDist == 0.0f)
      {
        aResHeader.SliceDist = aResHeader.PixelSize;
      }
    }
    else if (anArgiter + 1 < theNbArgs
          && (anArg == "-slicedist"
           || anArg == "-slicestep"))
    {
      aResHeader.SliceDist = (float )atof (theArgVec[++anArgiter]);
    }
    else if (anArgiter + 1 < theNbArgs
          && (anArg == "-i"
           || anArg == "-input"))
    {
      anInput = theArgVec[++anArgiter];
    }
    else if (anArgiter + 1 < theNbArgs
          && (anArg == "-o"
           || anArg == "-output"))
    {
      anOutput = theArgVec[++anArgiter];
    }
    else if (anInput.empty())
    {
      anInput = theArgVec[anArgiter];
    }
    else if (anOutput.empty())
    {
      anOutput = theArgVec[anArgiter];
    }
    else
    {
      std::cerr << "Syntax error: unknown argument '" << theArgVec[anArgiter] << "'\n";
      return 1;
    }
  }
  if (anInput.empty()
   || anOutput.empty())
  {
    std::cerr << "Syntax error: wrong number of arguments\n";
    return 1;
  }
  else if (anInput == anOutput)
  {
    std::cerr << "Syntax error: input and output should not match\n";
    return 1;
  }
  else if (aResHeader.SizeX == 0
        || aResHeader.SizeY == 0
        || aResHeader.SizeZ == 0)
  {
    std::cerr << "Syntax error: undefined dimensions\n";
    return 1;
  }
  else if (aResHeader.Pixel != 16
        && aResHeader.Pixel != 32)
  {
    std::cerr << "Syntax error: undefined pixel format\n";
    return 1;
  }
  else if (aResHeader.PixelSize <= 0.0f)
  {
    std::cerr << "Syntax error: undefined pixel size\n";
    return 1;
  }

  std::ifstream anInFile;
  anInFile.open (anInput, std::ios::in | std::ios::binary);
  if (!anInFile)
  {
    std::cerr << "Error: unable to read file '" << anInput << "'\n";
    return 1;
  }
  const size_t aPixelSizeBytes = aResHeader.Pixel == 16 ? 2 : 4;
  const size_t anImgDataSize = aPixelSizeBytes * aResHeader.SizeX * aResHeader.SizeY * aResHeader.SizeZ;
  //const size_t aRekSize = anImgDataSize + sizeof(RekFileHeader);

  anInFile.seekg (0, std::ios::end);
  const size_t aSize = (size_t )anInFile.tellg();
  std::vector<unsigned char> aBuffer;
  aBuffer.resize (aSize, 0);
  anInFile.seekg (0, std::ios::beg);
  anInFile.read ((char* )&aBuffer[0], aSize);
  if ((size_t )anInFile.gcount() != aSize)
  {
    std::cerr << "Error: unable to read file '" << anInput << "'\n";
    return 1;
  }

  std::cout << "Output: '" << anOutput << "' " << aResHeader.SizeX << "x" << aResHeader.SizeY << "x" << aResHeader.SizeZ
            << "@" << (aResHeader.Pixel == 16 ? "int16" : "float32") << ".\n";
  if (aSize != anImgDataSize)
  {
    std::cerr << "Error: unexpected input file size " << aSize << " (expected: " << anImgDataSize << " bytes).\n";
    return 1;
  }

  std::ofstream aResFile;
  aResFile.open (anOutput, std::ios::out | std::ios::binary);
  if (!aResFile)
  {
    std::cerr << "Error: unable to write result file '" << anOutput << "'\n";
    return 1;
  }
  aResFile.write ((const char* )&aResHeader, sizeof(aResHeader));
  aResFile.write ((const char* )&aBuffer.front(), aSize);
  aResFile.close();
  if (!aResFile.good())
  {
    std::cerr << "Error: unable to write result file '" << anOutput << "'\n";
    return 1;
  }

  return 0;
}
