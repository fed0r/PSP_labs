using System;
using System.Net.Sockets;
using System.Text;
using System.Text.RegularExpressions;

namespace Lab7PSP
{
    public class ClientObject
    {
        private TcpClient client;
        private NetworkStream stream = null;
        private string clientName = string.Empty;

        const double epsilon = 1e-2;

        public ClientObject(TcpClient client)
        {
            this.client = client;
        }

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

        public void Process()
        {
            try
            {
                NetworkStream stream = client.GetStream();

                // enter to an infinite cycle to be able to handle every change in stream
                while (true)
                {
                    while (!stream.DataAvailable) ;
                    while (client.Available < 3) ; // match against "get"

                    byte[] bytes = new byte[client.Available];
                    stream.Read(bytes, 0, client.Available);
                    string s = Encoding.UTF8.GetString(bytes);
                    

                    if (Regex.IsMatch(s, "^GET", RegexOptions.IgnoreCase))
                    {
                        Console.WriteLine("=====Handshaking from client=====\n{0}", s);
                        string swk = Regex.Match(s, "Sec-WebSocket-Key: (.*)").Groups[1].Value.Trim();
                        string swka = swk + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
                        byte[] swkaSha1 = System.Security.Cryptography.SHA1.Create().ComputeHash(Encoding.UTF8.GetBytes(swka));
                        string swkaSha1Base64 = Convert.ToBase64String(swkaSha1);

                        // HTTP/1.1 defines the sequence CR LF as the end-of-line marker
                        byte[] response = Encoding.UTF8.GetBytes(
                            "HTTP/1.1 101 Switching Protocols\r\n" +
                            "Connection: Upgrade\r\n" +
                            "Upgrade: websocket\r\n" +
                            "Sec-WebSocket-Accept: " + swkaSha1Base64 + "\r\n\r\n");

                        stream.Write(response, 0, response.Length);
                    }
                    else
                    {
                        bool fin = (bytes[0] & 0b10000000) != 0,
                            mask = (bytes[1] & 0b10000000) != 0; 

                        int opcode = bytes[0] & 0b00001111, 
                            msglen = bytes[1] - 128, 
                            offset = 2;

                        if (msglen == 126)
                        {
                            msglen = BitConverter.ToUInt16(new byte[] { bytes[3], bytes[2] }, 0);
                            offset = 4;
                        }
                        else if (msglen == 127)
                        {
                        }

                        if (msglen == 0)
                            Console.WriteLine("msglen == 0");
                        else if (mask)
                        {
                            byte[] decoded = new byte[msglen];
                            byte[] masks = new byte[4] { bytes[offset], bytes[offset + 1], bytes[offset + 2], bytes[offset + 3] };
                            offset += 4;

                            for (int i = 0; i < msglen; ++i)
                                decoded[i] = (byte)(bytes[offset + i] ^ masks[i % 4]);

                            string array = Encoding.UTF8.GetString(decoded);

                            Console.WriteLine(array);
                            string[] numbers = array.Split(' ');
                            Console.WriteLine(numbers);
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

                                newSortedArray += presortedArray[i].ToString() + "  ";

                            }
                           

                            Byte[] response = Encoding.UTF8.GetBytes("  " + newSortedArray);
                            response[0] = 0x81; // denotes this is the final message and it is in text
                            response[1] = (byte)(response.Length - 2); // payload size = message - header size
                            stream.Write(response, 0, response.Length);

                        }
                        else
                            Console.WriteLine("mask bit not set");

                        Console.WriteLine();
                    }
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
            }
            finally
            {
                if (stream != null)
                    stream.Close();
                if (client != null)
                    client.Close();
                Console.ReadKey();
            }
        }


    }
}
