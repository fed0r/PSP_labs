using System;
using System.Text;
using System.Net;
using System.Net.Sockets;
using System.Threading;
namespace Server
{
    class Program
    {
        const double epsilon = 1e-2;

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



        static void ThreadProc(Object obj)
        {
            Socket handler = (Socket)obj;
            string data = null;
            byte[] bytesSize = new byte[2048];

            int bytesReceive = handler.Receive(bytesSize);

            string sizeStr = Encoding.UTF8.GetString(bytesSize, 0, bytesReceive);
            int sizeArray = Convert.ToInt32(sizeStr);

            byte[] bytes = new byte[sizeof(int) * sizeArray];
            int[] array = new int[sizeArray];
            int bytesRec = handler.Receive(bytes);
            data += Encoding.UTF8.GetString(bytes, 0, bytesRec);
            Buffer.BlockCopy(bytes, 0, array, 0, bytes.Length);

            for(int i = 0; i < sizeArray; i++)
            {
                Convert.ToInt32(array[i]);
            }

            Quicksort(array, 0, array.Length - 1);

            Console.WriteLine("Отсортированный массив:");
            for (int i = 0; i < sizeArray; i++)
            {
                Console.WriteLine(array[i]);
            }

            //отправка массива назад

            byte[] msg;

            msg = new byte[array.Length * sizeof(int)];
            Buffer.BlockCopy(array, 0, msg, 0, msg.Length);
            handler.Send(msg);

            handler.Shutdown(SocketShutdown.Both);
            handler.Close();
        }

        static void Main(string[] args)
        {
            // Устанавливаем для сокета локальную конечную точку
            string host = Dns.GetHostName();

            int port = 11000;
            IPEndPoint ipEndPoint = new IPEndPoint(IPAddress.Any, port);

            // Создаем сокет Tcp/Ip
            Socket sListener = new Socket(ipEndPoint.AddressFamily, SocketType.Stream, ProtocolType.Tcp);

            // Назначаем сокет локальной конечной точке и слушаем входящие сокеты
            try
            {
                sListener.Bind(ipEndPoint);
                sListener.Listen(10);

                // Начинаем слушать соединения
                while (true)
                {
                    Console.WriteLine("Имя сервера: {0}", host);
                    Console.WriteLine("Порт: {0}", port);
                    Console.WriteLine("Ожидаем соединение через порт {0}", ipEndPoint);          

                    // Программа приостанавливается, ожидая входящее соединение
                    Socket handler = sListener.Accept();
                    Console.WriteLine("Conected");

                    //пул потоков
                   ThreadPool.QueueUserWorkItem(new WaitCallback(ThreadProc), handler);

                }
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