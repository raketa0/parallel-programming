#include <iostream>
#include <fstream>
#include <string>
#include <windows.h>
#include <vector>
#include <random>
#include <algorithm>

struct ThreadParams
{
    unsigned char* initialData;
    unsigned char* blurredSquare;
    BITMAPINFOHEADER* infoHeader;
    std::vector<Square>* squares;
    int radius;
    int coreIndex;
};

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

std::vector<std::vector<Square>> RandomDistributionOfStreams(std::vector<Square> & squares, const int numTheard)
{
    std::shuffle(squares.begin(), squares.end(), std::mt19937(std::random_device{}()));
    std::vector<std::vector<Square>> threadSquares(numTheard);

    for (size_t i = 0; i < squares.size(); i++)
    {
        int threadIndex = i % numTheard;
        threadSquares[threadIndex].push_back(squares[i]);
    }
    return threadSquares;
}

void BoxBlurSquare(unsigned char* initialData, unsigned char* blurredSquare,
    const BITMAPINFOHEADER& infoHeader, const Square& square, int radius)
{
    int width = infoHeader.biWidth;
    int height = infoHeader.biHeight;
    int stride = ((width * 3 + 3) & ~3);

    for (int y = square.y0; y < square.y1; ++y)
    {
        for (int x = square.x0; x < square.x1; ++x)
        {
            int sumB = 0, sumG = 0, sumR = 0;
            int count = 0;

            for (int dy = -radius; dy <= radius; ++dy)
            {
                int ny = y + dy;
                if (ny < 0 || ny >= height) continue;

                for (int dx = -radius; dx <= radius; ++dx)
                {
                    int nx = x + dx;
                    if (nx < 0 || nx >= width) continue;

                    unsigned char* pixel = initialData + ny * stride + nx * 3;
                    sumB += pixel[0];
                    sumG += pixel[1];
                    sumR += pixel[2];
                    count++;
                }
            }

            unsigned char* outPixel = blurredSquare + y * stride + x * 3;
            outPixel[0] = static_cast<unsigned char>(sumB / count);
            outPixel[1] = static_cast<unsigned char>(sumG / count);
            outPixel[2] = static_cast<unsigned char>(sumR / count);
        }
    }
}

DWORD WINAPI ThreadProc(LPVOID param)
{
    ThreadParams* p = (ThreadParams*)param;

    for (auto& sq : *(p->squares))
    {
        BoxBlurSquare(p->initialData, p->blurredSquare, *(p->infoHeader), sq, p->radius);
    }

    return 0;
}

void RunBoxBlurMultiCore(unsigned char* src, unsigned char* dst, BITMAPINFOHEADER& infoHeader,
    std::vector<Square>& allSquares, int radius, int numCoresToUse)
{
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);

    DWORD_PTR fullMask = (1ull << sysInfo.dwNumberOfProcessors) - 1;
    DWORD_PTR limitedMask = (1ull << numCoresToUse) - 1;

    if (!SetProcessAffinityMask(GetCurrentProcess(), limitedMask))
    {
        std::cerr << "Ошибка: не удалось установить affinity mask для процесса.\n";
        return;
    }

    auto distributedSquares = RandomDistributionOfStreams(allSquares, numCoresToUse);

    std::vector<HANDLE> handles(numCoresToUse);
    std::vector<ThreadParams> params(numCoresToUse);

    for (int i = 0; i < numCoresToUse; ++i)
    {
        params[i] = 
        {
            src,
            dst,
            &infoHeader,
            &distributedSquares[i],
            radius,
            i
        };

        handles[i] = CreateThread(
            NULL,
            0,
            ThreadProc,
            &params[i],
            0,
            NULL
        );
    }

    WaitForMultipleObjects(numCoresToUse, handles.data(), TRUE, INFINITE);

    for (auto& h : handles)
    {
        CloseHandle(h);
    }
}

int main(int argc, char* argv[])
{
    if (argc < 5) 
    {
        std::cout << "Usage: BlurringImage.exe input.bmp output.bmp <threads> <cores>\n";
        return 1;
    }

    const std::string InputImage = argv[1];
    const std::string outputBlurringImage = argv[2];
    const int numTheard = atoi(argv[3]);
    const int numCores = atoi(argv[4]);

}