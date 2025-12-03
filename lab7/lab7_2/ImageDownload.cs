
using System;
using System.Reflection;

namespace Download
{
    class ImageDownload
    {
        private API.API _api = new API.API();
        public ImageDownload()
        {
            Directory.CreateDirectory("images");
        }
        public async Task DownloadImageAsync(int index)
        {
            try
            {
                Console.WriteLine($"Началась загрузка изображения");
                string imageUrl = await _api.GetRandomDogImageAsync();
                using HttpClient httpClient = new HttpClient();
                var imageBytes = await httpClient.GetByteArrayAsync(imageUrl);
                string fileName = Path.Combine("images", Path.GetFileName(new Uri(imageUrl).LocalPath));
                await File.WriteAllBytesAsync($"images/async_{index}.jpg", imageBytes);
                Console.WriteLine($"Изображение сохранено: {fileName}");
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Ошибка при загрузке изображения: {ex.Message}");
            }
        }
        public void DownloadImage(int index)
        {
            try
            {
                Console.WriteLine($"Началась загрузка изображения");
                string imageUrl = _api.GetRandomDogImage();
                using HttpClient httpClient = new HttpClient();
                var imageBytes = httpClient.GetByteArrayAsync(imageUrl).Result;
                string fileName = Path.Combine("images", Path.GetFileName(new Uri(imageUrl).LocalPath));
                File.WriteAllBytesAsync($"images/sync_{index}.jpg", imageBytes);
                Console.WriteLine($"Изображение сохранено: {fileName}");
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Ошибка при загрузке изображения: {ex.Message}");
            }
        }
    }
}
