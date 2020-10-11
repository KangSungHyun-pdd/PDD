using System;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace pdd_TCPClient
{
    class Program
    {
        static void Main(string[] args)
        {
            try
            {
                string strRecvMsg;
                string strSendMsg;

                TcpClient sockClient = new TcpClient("172.20.10.10", 9090);     //socket , connect
                NetworkStream ns = sockClient.GetStream();
                StreamReader sr = new StreamReader(ns);
                StreamWriter sw = new StreamWriter(ns);

                strRecvMsg = sr.ReadLine(); //server in success, message receive
                Console.WriteLine(strRecvMsg);

                while (true)
                {
                    strSendMsg = Console.ReadLine();

                    sw.WriteLine(strSendMsg);
                    sw.Flush();
                    if (strSendMsg == "exit")
                    {
                        break;
                    }
                    strRecvMsg = sr.ReadLine();
                    Console.WriteLine(strRecvMsg);
                }

                sr.Close();
                sw.Close();
                ns.Close();
                sockClient.Close();

                Console.WriteLine("접속 종료");
            }
            catch(SocketException e)
            {
                Console.WriteLine(e.ToString());
            }
        }
    }
}
