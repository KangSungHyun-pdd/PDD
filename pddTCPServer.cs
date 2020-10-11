using System;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.IO.Ports;  //serial 통신 선언


namespace pdd_TCPServer
{
    class ArduSerial
    {
        public SerialPort arduSerialPort = new SerialPort();   //시리얼 포트 생성

        public void ArduConnect()
        {
                arduSerialPort.PortName = "";
                arduSerialPort.BaudRate = 0;
                arduSerialPort.Open();
        }
    }
    class Program
    {
        static void Main(string[] args)
        {
            ArduSerial ArduConnect = new ArduSerial();
            ArduConnect.arduSerialPort.PortName = "COM10";
            ArduConnect.arduSerialPort.BaudRate = 9600;
            Console.WriteLine(ArduConnect.arduSerialPort.PortName);
            Console.WriteLine(ArduConnect.arduSerialPort.BaudRate);
            try
            {
                string strMsg;

                TcpListener sockServer = new TcpListener(IPAddress.Parse("192.168.100.113"), 9090);     //IP, PORT
                sockServer.Start();
                Console.WriteLine("Server start! Client connecting.....");

                TcpClient client = sockServer.AcceptTcpClient();    //client accept
                Console.WriteLine("Client connect success.");

                NetworkStream ns = client.GetStream();
                StreamReader sr = new StreamReader(ns);
                StreamWriter sw = new StreamWriter(ns);

                string welcome = "Server Connect Success!";
                sw.WriteLine(welcome);
                sw.Flush();

                Console.WriteLine(sr.GetType());
               
                while (true)
                {

                    strMsg = sr.ReadLine();

                    switch (strMsg)
                    {
                        case "a":
                            ArduConnect.arduSerialPort.Write("a");
                            break;
                        case "b":
                            ArduConnect.arduSerialPort.Write("b");
                            break;
                        case "c":
                            ArduConnect.arduSerialPort.Write("c");
                            break;
                        case "d":
                            ArduConnect.arduSerialPort.Write("d");
                            break;
                        case "exit":
                            break;
                    }

                    Console.WriteLine(strMsg);
                    sw.WriteLine(strMsg);
                    sw.Flush();
                }
                sw.Close();
                sr.Close();
                ns.Close();
                sockServer.Stop();

                Console.WriteLine("Client disconnet.");
            }

            catch (SocketException e)
            {
                Console.WriteLine(e.ToString());
                
            }
            catch(InvalidOperationException u)
            {
                Console.WriteLine(u.ToString());
            }
        }
    }
}
