using System;

public class IOFile
{
    private string m_path { get; set; } = string.Empty;
    private string m_text { get; set; } = string.Empty;
    public IOFile(string path)
    {
        m_path = path;
    }
    public async Task<string> InputFileAsync()
    {
        string text = await File.ReadAllTextAsync(m_path);
        return text;
    }

    public async Task OutputInFileAsync(string filePath, string text)
    {
        await File.WriteAllTextAsync(filePath, text);
    }


}

public class DelCharInText
{
    private IOFile m_textInFile;
    public DelCharInText(IOFile textInFile)
    {
        m_textInFile = textInFile;
    }

    public async Task<string> charRemoveAsync(HashSet<Char> set)
    {
        if (m_textInFile == null) 
        {
            return string.Empty;
        }
        string text = await m_textInFile.InputFileAsync();
        foreach (char ch in set)
        {
            if (ch == '\n')
            {
                continue;
            }
            text = text.Replace(ch.ToString(), "");
        }
        return text;
    }
}

class Program
{
    static async Task Main()
    {
        Console.Write("Введите путь к файлу: ");
        string path = Console.ReadLine()!;

        IOFile file = new IOFile(path);
        DelCharInText remover = new DelCharInText(file);

        Console.Write("Введите символы для удаления: ");
        string chars = Console.ReadLine()!;
        var set = new HashSet<char>(chars);

        string newText = await remover.charRemoveAsync(set);

        await file.OutputInFileAsync(path, newText);

        Console.WriteLine("Файл обновлён!");
    }
}
