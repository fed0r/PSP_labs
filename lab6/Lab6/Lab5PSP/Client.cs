using System;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Text.RegularExpressions;
using System.Net.Security;
using System.Security.Authentication;
using System.Security.Cryptography.X509Certificates;
namespace Lab5PSP
{
    class Client
    {
        static int Partition(int[] array, int start, int end)
        {
            int temp;
            int marker = start;
            for (int i = start; i < end; i++)
            {
                if (array[i] < array[end])
                {
                    temp = array[marker];
                    array[marker] = array[i];
                    array[i] = temp;
                    marker += 1;
                }
            }

            temp = array[marker];
            array[marker] = array[end];
            array[end] = temp;
            return marker;
        }

        static void Quicksort(int[] array, int start, int end)
        {
            if (start >= end)
            {
                return;
            }
            int pivot = Partition(array, start, end);
            Quicksort(array, start, pivot - 1);
            Quicksort(array, pivot + 1, end);

        }
        // Отправка страницы с ошибкой
        private void SendError(TcpClient Client, int Code)
        {
            // Получаем строку вида "200 OK"
            // HttpStatusCode хранит в себе все статус-коды HTTP/1.1
            string CodeStr = Code.ToString() + " " + ((HttpStatusCode)Code).ToString();
            // Код простой HTML-странички
            string Html = "<html><body><h1>" + CodeStr + "</h1></body></html>";
            // Необходимые заголовки: ответ сервера, тип и длина содержимого. После двух пустых строк - само содержимое
            string Str = "HTTP/1.1 " + CodeStr + "\nContent-type: text/html\nContent-Length:" + Html.Length.ToString() + "\n\n" + Html;
            // Приведем строку к виду массива байт
            byte[] Buffer = Encoding.ASCII.GetBytes(Str);
            // Отправим его клиенту
            Client.GetStream().Write(Buffer, 0, Buffer.Length);
            // Закроем соединение
            Client.Close();
        }

        // Конструктор класса. Ему нужно передавать принятого клиента от TcpListener
        public Client(TcpClient Client, X509Certificate serverCertificate)
        {
            SslStream sslStream = new SslStream(Client.GetStream(), false);

            try
            {
                sslStream.AuthenticateAsServer(serverCertificate, false, SslProtocols.Tls12, true);

                // Объявим строку, в которой будет хранится запрос клиента
                string Request = "";
                // Буфер для хранения принятых от клиента данных
                byte[] Buffer = new byte[1024];
                // Переменная для хранения количества байт, принятых от клиента
                int Count;
                // Читаем из потока клиента до тех пор, пока от него поступают данные
                while ((Count = sslStream.Read(Buffer, 0, Buffer.Length)) > 0)
                {
                    // Преобразуем эти данные в строку и добавим ее к переменной Request
                    Request += Encoding.ASCII.GetString(Buffer, 0, Count);
                    // Запрос должен обрываться последовательностью \r\n\r\n
                    // Либо обрываем прием данных сами, если длина строки Request превышает 4 килобайта
                    // Нам не нужно получать данные из POST-запроса (и т. п.), а обычный запрос
                    // по идее не должен быть больше 4 килобайт
                    if (Request.IndexOf("\r\n\r\n") >= 0 || Request.Length > 4096)
                    {
                        break;
                    }
                }

                // Парсим строку запроса с использованием регулярных выражений
                // При этом отсекаем все переменные GET-запроса
                Match ReqMatch = Regex.Match(Request, @"^\w+\s+([^\s]+)[^\s]*\s+HTTP/.*|");

                // Если запрос не удался
                if (ReqMatch == Match.Empty)
                {
                    // Передаем клиенту ошибку 400 - неверный запрос
                    SendError(Client, 400);
                    return;
                }

                // Получаем строку запроса
                string RequestUri = ReqMatch.Groups[1].Value;

                // Приводим ее к изначальному виду, преобразуя экранированные символы
                // Например, "%20" -> " "
                RequestUri = Uri.UnescapeDataString(RequestUri);

                // Если в строке содержится двоеточие, передадим ошибку 400
                // Это нужно для защиты от URL типа http://example.com/../../file.txt
                if (RequestUri.IndexOf("..") >= 0)
                {
                    SendError(Client, 400);
                    return;
                }

                string Headers;
                byte[] HeadersBuffer;

                // Если строка запроса оканчивается на "/", то добавим к ней index.html
                if (RequestUri.EndsWith("/") ||
                    RequestUri.EndsWith(".css") ||
                    RequestUri.EndsWith(".html") ||
                    RequestUri.EndsWith(".js") ||
                    RequestUri.EndsWith(".ico") ||
                    RequestUri.EndsWith(".htm"))
                {
                    if (RequestUri.EndsWith("/"))
                    {
                        RequestUri += "index.html";
                    }

                    string FilePath = "Resources/" + RequestUri;

                    // Если в папке www не существует данного файла, посылаем ошибку 404
                    if (!File.Exists(FilePath))
                    {
                        SendError(Client, 404);
                        return;
                    }

                    // Получаем расширение файла из строки запроса
                    string Extension = RequestUri.Substring(RequestUri.LastIndexOf('.'));

                    // Тип содержимого
                    string ContentType = "";

                    // Пытаемся определить тип содержимого по расширению файла
                    switch (Extension)
                    {
                        case ".htm":
                        case ".html":
                            ContentType = "text/html";
                            break;
                        case ".css":
                            ContentType = "text/stylesheet";
                            break;
                        case ".js":
                            ContentType = "text/javascript";
                            break;
                        case ".jpg":
                            ContentType = "image/jpeg";
                            break;
                        case ".jpeg":
                        case ".png":
                        case ".gif":
                            ContentType = "image/" + Extension.Substring(1);
                            break;
                        default:
                            if (Extension.Length > 1)
                            {
                                ContentType = "application/" + Extension.Substring(1);
                            }
                            else
                            {
                                ContentType = "application/unknown";
                            }
                            break;
                    }

                    // Открываем файл, страхуясь на случай ошибки
                    FileStream FS;
                    try
                    {
                        FS = new FileStream(FilePath, FileMode.Open, FileAccess.Read, FileShare.Read);
                    }
                    catch (Exception)
                    {
                        // Если случилась ошибка, посылаем клиенту ошибку 500
                        SendError(Client, 500);
                        return;
                    }

                    // Посылаем заголовки
                    Headers = "HTTP/1.1 200 OK\nContent-Type: " + ContentType + "\nContent-Length: " + FS.Length + "\n\n";
                    HeadersBuffer = Encoding.ASCII.GetBytes(Headers);
                    sslStream.Write(HeadersBuffer, 0, HeadersBuffer.Length);

                    // Пока не достигнут конец файла
                    while (FS.Position < FS.Length)
                    {
                        // Читаем данные из файла
                        Count = FS.Read(Buffer, 0, Buffer.Length);
                        // И передаем их клиенту
                        sslStream.Write(Buffer, 0, Count);
                    }

                    // Закроем файл и соединение
                    FS.Close();
                    Client.Close();
                }
                else if (RequestUri.Length > 2)
                {
                    string array = RequestUri.Remove(0, 1);

                    Console.WriteLine(array);
                    string[] numbers = array.Split(' ');
                    for (int i = 0; i < numbers.Length; i++)
                    {
                        Convert.ToInt32(numbers[i]);
                    }

                    int[] presortedArray = new int[] { };

                    for (int i = 0; i < numbers.Length; i++)
                    {
                        Array.Resize(ref presortedArray, presortedArray.Length + 1);
                        presortedArray[i] = Convert.ToInt32(numbers[i]);

                    }
                    Quicksort(presortedArray, 0, presortedArray.Length - 1);
                    string newSortedArray = "";
                    for (int i = 0; i < presortedArray.Length; i++)
                    {

                        newSortedArray += presortedArray[i].ToString() + "\n";

                    }
                    byte[] msg = Encoding.UTF8.GetBytes(newSortedArray);


                    Headers = "HTTP/1.1 200 OK\nContent-Type: " + "text" + "\nContent-Length: " + msg.Length + "\n\n";
                    HeadersBuffer = Encoding.UTF8.GetBytes(Headers);
                    sslStream.Write(HeadersBuffer, 0, HeadersBuffer.Length);
                    sslStream.Write(msg, 0, msg.Length);


                }

            }
            catch(Exception exp)
            {
            }
            finally
            {
                sslStream.Close();
                Client.Close();
            }
        }
    }
}
