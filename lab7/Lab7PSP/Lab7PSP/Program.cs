using Lab7PSP;
using System;
using System.Net;
using System.Net.Sockets;
using System.Threading;

class Server
{
    public static void Main()
    {
        string ip = "192.168.43.161";
        int port = 8080;
        var server = new TcpListener(IPAddress.Parse(ip), port);

        server.Start();
        Console.WriteLine("Server has started on {0}:{1}, Waiting for a connection...", ip, port);

        while (true)
        {
            TcpClient client = server.AcceptTcpClient();
            Console.WriteLine("A client connected.");
            ClientObject clientobject = new ClientObject(client);
            Thread clientThread = new Thread(new ThreadStart(clientobject.Process));
            clientThread.Start();
        }
        
    }
}