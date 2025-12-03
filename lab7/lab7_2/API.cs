using System;
using System.IO;
using System.Net.Http;
using System.Text.Json;
using System.Threading.Tasks;
using System.Collections.Generic;
using System.Diagnostics;

namespace API
{
    class API
    {
        private string url = "https://dog.ceo/api/breeds/image/random";

        public async Task<string> GetRandomDogImageAsync()
        {
            using HttpClient httpClient = new HttpClient();
            var response = await httpClient.GetAsync(url);
            if (!response.IsSuccessStatusCode)
            {
                throw new Exception($"Ошибка API: {response.StatusCode}");
            }
            var content = await response.Content.ReadAsStringAsync();
            var jsonDoc = JsonDocument.Parse(content);
            return jsonDoc.RootElement.GetProperty("message").GetString();
        }
        public string GetRandomDogImage()
        {
            using HttpClient httpClient = new HttpClient();
            var response = httpClient.GetAsync(url).Result;
            if (!response.IsSuccessStatusCode)
            {
                throw new Exception($"Ошибка API: {response.StatusCode}");
            }
            var content = response.Content.ReadAsStringAsync().Result;
            var jsonDoc = JsonDocument.Parse(content);
            return jsonDoc.RootElement.GetProperty("message").GetString();
        }
    }
}
