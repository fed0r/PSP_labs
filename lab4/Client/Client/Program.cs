using System;
using System.Text;
using System.Net;
using System.Net.Sockets;

namespace Client
{
    class Program
    {
        static void Main(string[] args)
        {
            try
            {

                Console.Write("Введите ip адрес сервера: ");
                string host = Console.ReadLine();

                // Буфер для входящих данных
                byte[] bytes = new byte[1024];

                // Устанавливаем удаленную точку для сокета
                IPEndPoint ipEndPoint = new IPEndPoint(IPAddress.Parse(host), 11000);

                // Создание сокета отправителя, который привязывается к удаленной токче с протоколом TCP
                Socket client_socket = new Socket(ipEndPoint.AddressFamily, SocketType.Stream, ProtocolType.Tcp);

                // Соединяем сокет с удаленной точкой
                client_socket.Connect(ipEndPoint);

                Console.WriteLine("Сокет соединяется с {0} ", client_socket.RemoteEndPoint.ToString());

               
                Console.Write("Введите рамер массива: ");
                int size = Convert.ToInt32( Console.ReadLine());

                int[] array = new int[size];

                Console.Write("Введите массив: ");
                for(int i = 0; i < size; i++)
                {
                    array[i] = Convert.ToInt32(Console.ReadLine());
                }

                //отправка размера массива
                string sizeString = size.ToString();
                byte[] msgsize = Encoding.UTF8.GetBytes(sizeString);
                client_socket.Send(msgsize);


                //отправка  массива
                byte[] msg;

                msg = new byte[array.Length * sizeof(int)];
                Buffer.BlockCopy(array, 0, msg, 0, msg.Length);

                // Отправляем данные через сокет
                client_socket.Send(msg);

                Console.WriteLine("Отправка: ОК");

                //получение массива отсортированного
                string data = null;
                byte[] bytes1 = new byte[sizeof(int) * size];
                int[] sortedArray = new int[size];
                int bytesRec = client_socket.Receive(bytes1);
                data += Encoding.UTF8.GetString(bytes1, 0, bytesRec);
                Buffer.BlockCopy(bytes1, 0, sortedArray, 0, bytes1.Length);

                Console.WriteLine("Ответ от сервера:");
                for (int i = 0; i < size; i++)
                {
                    Console.WriteLine(Convert.ToInt32(sortedArray[i]));
                }


                // Освобождаем сокет
                client_socket.Shutdown(SocketShutdown.Both);
                client_socket.Close();

            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.ToString());
            }
            finally
            {
                Console.ReadLine();
            }
        }
    }
}

