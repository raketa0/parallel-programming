using API;
using Download;
using System.Diagnostics;

namespace Program
{ 
    class Program
    {
        static async Task Main(string[] args)
        {
            int count = 10;
            ImageDownload imageDownload = new ImageDownload();
            Console.WriteLine("Началась асинхронная загрузка изображений");
            Stopwatch swAsync = Stopwatch.StartNew();
            List<Task> downloadTasks = new List<Task>();

            for (int i = 0; i < count; i++)
            {
                downloadTasks.Add(imageDownload.DownloadImageAsync(i));
            }
            await Task.WhenAll(downloadTasks);

            swAsync.Stop();
            Console.WriteLine($"Асинхронная загрузка изображений завершена: {swAsync.ElapsedMilliseconds} mc");

            Console.WriteLine("Началась синхронная загрузка изображений");
            Stopwatch swSync = Stopwatch.StartNew();

            for (int i = 0; i < count; i++)
            {
                imageDownload.DownloadImage(i);
            }
            swSync.Stop();
            Console.WriteLine($"Синхронная загрузка изображений завершена: {swSync.ElapsedMilliseconds} mc");
          



        }
    }
}