#include <iostream>
#include <fstream>
#include <string>
#include <windows.h>
#include <vector>
#include <random>
#include <algorithm>
#include <chrono>

auto programStart = std::chrono::high_resolution_clock::now();
int pixelCountPerThread[3] = { 0 };
const int MAX_LOG_PIXELS = 50;


struct Square
{
    int x0;
    int y0;

    int x1;
    int y1;
};

struct ThreadParams
{
    unsigned char* initialData;
    unsigned char* blurredSquare;
    BITMAPINFOHEADER* infoHeader;
    std::vector<Square>* squares;
    int radius;
    int coreIndex;

    ThreadParams() = default;

    ThreadParams(unsigned char* iData,
        unsigned char* bSquare,
        BITMAPINFOHEADER* iHeader,
        std::vector<Square>* sq,
        int r,
        int c)
        : initialData(iData), blurredSquare(bSquare),
        infoHeader(iHeader), squares(sq),
        radius(r), coreIndex(c) {
    }
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

bool WriteBMP(const char* filename,
    BITMAPFILEHEADER fileHeader,
    BITMAPINFOHEADER infoHeader,
    unsigned char* data)
{
    std::ofstream out(filename, std::ios::binary);
    if (!out)
        return false;

    int rowSize = ((infoHeader.biWidth * 3 + 3) & ~3);
    int imageSize = rowSize * infoHeader.biHeight;

    infoHeader.biSizeImage = imageSize;
    fileHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + imageSize;

    out.write((char*)&fileHeader, sizeof(fileHeader));
    out.write((char*)&infoHeader, sizeof(infoHeader));
    out.write((char*)data, imageSize);
    out.close();

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

std::vector<std::vector<Square>> RandomDistributionOfStreams(std::vector<Square>& squares, const int numTheard)
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
    const BITMAPINFOHEADER& infoHeader, const Square& square, int radius, int threadIndex)
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

            if (pixelCountPerThread[threadIndex] < MAX_LOG_PIXELS)
            {
                auto now = std::chrono::high_resolution_clock::now();
                auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - programStart).count();

                printf("%lld,%d\n", ms, threadIndex);
                pixelCountPerThread[threadIndex]++;
            }
        }
    }
}


DWORD WINAPI ThreadProc(LPVOID param)
{
    ThreadParams* p = (ThreadParams*)param;

    for (auto& sq : *(p->squares))
    {
        BoxBlurSquare(p->initialData, p->blurredSquare, *(p->infoHeader), sq, p->radius, p->coreIndex);
    }


    return 0;
}

void RunBoxBlurMultiCore(unsigned char* src, unsigned char* dst, BITMAPINFOHEADER& infoHeader,
    std::vector<Square>& allSquares, int radius, int numThreads, int numCoresToUse)
{
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);

    DWORD_PTR limitedMask = (1ull << numCoresToUse) - 1;
    if (!SetProcessAffinityMask(GetCurrentProcess(), limitedMask))
    {
        std::cerr << "Ошибка: не удалось установить affinity mask для процесса.\n";
        return;
    }

    auto distributedSquares = RandomDistributionOfStreams(allSquares, numThreads);

    std::vector<HANDLE> handles(numThreads);
    std::vector<ThreadParams> params(numThreads);

    for (int i = 0; i < numThreads; ++i)
    {
        params[i] = { src, dst, &infoHeader, &distributedSquares[i], radius, i };
        handles[i] = CreateThread(NULL, 0, ThreadProc, &params[i], 0, NULL);
    }
    SetThreadPriority(handles[0], THREAD_PRIORITY_ABOVE_NORMAL);
    SetThreadPriority(handles[1], THREAD_PRIORITY_NORMAL);
    SetThreadPriority(handles[2], THREAD_PRIORITY_BELOW_NORMAL);

    WaitForMultipleObjects(numThreads, handles.data(), TRUE, INFINITE);

    for (auto& h : handles)
        CloseHandle(h);
}



void ProcessParallel(const std::string& input, const std::string& output,
    int numThreads, int numCores, int radius)
{
    BITMAPFILEHEADER fileHeader;
    BITMAPINFOHEADER infoHeader;
    unsigned char* src = nullptr;

    auto startr = std::chrono::high_resolution_clock::now();
    ReadBMP(input.c_str(), fileHeader, infoHeader, src);

    int size = ((infoHeader.biWidth * 3 + 3) & ~3) * infoHeader.biHeight;
    unsigned char* dst = new unsigned char[size]();

    auto squares = DivideIntoSquares(infoHeader, numThreads);

    {
        std::ofstream log("thread_timing.csv", std::ios::trunc);
        log << "coreIndex,elapsed_ms\n";
    }

    RunBoxBlurMultiCore(src, dst, infoHeader, squares, radius, numThreads, numCores);

    WriteBMP(output.c_str(), fileHeader, infoHeader, dst);

    auto endr = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = endr - startr;
    std::cout << "Параллельная обработка: " << elapsed.count() << " секунд\n";

    delete[] src;
    delete[] dst;
}

/*
void ProcessSequential(const std::string& input, const std::string& output, int radius)
{
    BITMAPFILEHEADER fileHeader;
    BITMAPINFOHEADER infoHeader;
    unsigned char* src = nullptr;

    auto start = std::chrono::high_resolution_clock::now();
    ReadBMP(input.c_str(), fileHeader, infoHeader, src);

    int size = ((infoHeader.biWidth * 3 + 3) & ~3) * infoHeader.biHeight;
    unsigned char* dst = new unsigned char[size]();

    auto squares = DivideIntoSquares(infoHeader, 1);


    for (auto& sq : squares)
    {
        BoxBlurSquare(src, dst, infoHeader, sq, radius);
    }

    WriteBMP(output.c_str(), fileHeader, infoHeader, dst);

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Последовательная обработка заняла: " << elapsed.count() << " секунд\n";

    delete[] src;
    delete[] dst;
}
 */

int main(int argc, char* argv[])
{
    setlocale(LC_ALL, "Russian");

    std::cout << "Нажмите Enter для продолжения: ";
    std::cin.get();
    programStart = std::chrono::high_resolution_clock::now();

    // if (argc < 6)
    // {
     //    std::cout << "Usage: BlurringImage.exe input.bmp output.bmp <threads> <cores>\n";
      //   return 1;
    // }
    const std::string inputImage = "input.bmp";
    const std::string outputParallel = "output_parallel.bmp";
    const std::string outputSequential = "output_sequential.bmp";
    const int numThreads = 3;
    const int numCores = 1;
    const int radius = 7;

    std::cout << "=== ПАРАЛЛЕЛЬНАЯ ОБРАБОТКА ===\n";
    ProcessParallel(inputImage, outputParallel, numThreads, numCores, radius);

   // std::cout << "=== ПОСЛЕДОВАТЕЛЬНАЯ ОБРАБОТКА ===\n";
   // ProcessSequential(inputImage, outputSequential, radius);

    std::cout << "Файлы сохранены:\n";
    std::cout << outputParallel << "\n";
    std::cout << outputSequential << "\n";

}