#include <iostream>
#include <fstream>
#include <string>
#include <windows.h>
#include <vector>


struct Square
{
    int x0;
    int y0;

    int x1;
    int y1;
};

bool ReadBMP(const char* filename,
    BITMAPFILEHEADER& fileHeader,
    BITMAPINFOHEADER& infoHeader,
    unsigned char*& data)
{
    std::ifstream file(filename, std::ios::binary);
    if (!file) return false;

    file.read((char*)&fileHeader, sizeof(fileHeader));
    if (fileHeader.bfType != 0x4D42) return false; // 'BM'

    file.read((char*)&infoHeader, sizeof(infoHeader));

    file.seekg(fileHeader.bfOffBits, std::ios::beg);

    int size = infoHeader.biSizeImage;
    if (size == 0)
        size = ((infoHeader.biWidth * 3 + 3) & ~3) * infoHeader.biHeight;

    data = new unsigned char[size];
    file.read((char*)data, size);

    return true;
}

std::vector<Square> DivideIntoSquares(BITMAPINFOHEADER& infoHeader, const int numTheard)
{
    auto widthImg = infoHeader.biWidth;
    auto heightImg = infoHeader.biHeight;
    int blockWidth = widthImg / numTheard;
    int blockHeight = heightImg / numTheard;

    std::vector<Square> squares;

    for (size_t j = 0; j < numTheard; j++)
    {
        for (size_t i = 0; i < numTheard; i++)
        {
            Square square;

            if ((i == numTheard - 1) && (j == numTheard - 1))
            {
                square.x0 = blockWidth * i;
                square.y0 = blockHeight * j;
                square.x1 = square.x0 + (widthImg - (blockWidth * i));
                square.y1 = square.y0 + (heightImg - (blockHeight * j));
            }
            else if (j == numTheard - 1)
            {
                square.x0 = blockWidth * i;
                square.y0 = blockHeight * j;
                square.x1 = blockWidth * (i + 1);
                square.y1 = square.y0 + (heightImg - (blockHeight * j));

            }
            else if (i == numTheard - 1)
            {
                square.x0 = blockWidth * i;
                square.y0 = blockHeight * j;
                square.x1 = square.x0 + (widthImg - (blockWidth * i));
                square.y1 = blockHeight * (j + 1);
            }
            else
            {
                square.x0 = blockWidth * i;
                square.y0 = blockHeight * j;
                square.x1 = blockWidth * (i + 1);
                square.y1 = blockHeight * (j + 1);
            }

            squares.push_back(square);
        }
    }
    return squares;
}

int main(int argc, char* argv[])
{
    if (argc < 5) 
    {
        std::cout << "Usage: blur.exe input.bmp output.bmp <threads> <cores>\n";
        return 1;
    }

    const std::string InputImage = argv[1];
    const std::string outputBlurringImage = argv[2];
    const int numTheard = atoi(argv[3]);
    const int numCores = atoi(argv[4]);
}