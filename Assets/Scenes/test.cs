using UnityEngine;
using System.Net.Sockets;
using System;
using System.IO;

public class test : MonoBehaviour
{
    private TcpClient socket;

    // Start is called before the first frame update
    void Start()
    {
        handleSocketConnect("localhost", 10086);
        readSocket();
    }

    // Update is called once per frame
    void Update()
    {
        
    }

    public bool handleSocketConnect(string hostname, int port)
    {
        socket = new TcpClient();

        //Set TCP socket to not linger and immediately release port on disconnect
        socket.LingerState = new LingerOption(true, 0);

        //Set up connection using C# TCP sockets
        try
        {
            socket.Connect(hostname, port);
        }
        catch (System.Exception e)
        {
            Debug.Log("Could not initialize socket. Error: " + e.Message);
            return false;
        }

        return handleTCHandshake();
    }

    private bool handleTCHandshake()
    {
        BinaryReader reader;
        try
        {
            reader = new BinaryReader(socket.GetStream());
        }
        catch (Exception e)
        {
            Debug.Log("Could not open stream for handshake. Error: " + e.Message);
            return false;
        }

        int type = System.Net.IPAddress.NetworkToHostOrder(reader.ReadInt32());
        int version = reader.ReadInt32();

        if (type != 4 && version != 2)
        {
            Debug.Log("IMD Handshake failed. Expected message type 4, got " + type + ". Expected version 2, got " + version);
            return false;
        }

        return true;
    }

    private void readSocket()
    {
        int dataLength = 10;
        int dataType = 0;

        if (socket == null || !socket.Connected)
            return;

        NetworkStream stream;
        try
        {
            stream = socket.GetStream();
        }
        catch (Exception e)
        {
            Debug.Log("Could not open stream to read packets. Error: " + e.Message);
            return;
        }

        if (stream.DataAvailable)
        {

            int bytesLeft = dataLength * System.Runtime.InteropServices.Marshal.SizeOf(dataType);
            int bytesRead = 0;
            byte[] packet = new byte[bytesLeft];

            try
            {
                while (bytesRead < 10)
                {
                    int read = stream.Read(packet, bytesRead, bytesLeft);
                    bytesRead += read;
                }
            }
            catch (Exception exception)
            {
                Debug.Log("TCP socket package is not complete!");
                return;
            }

            int[] result = new int[dataLength];
            for (int i = 0; i < dataLength; i++)
                result[i] = BitConverter.ToInt32(packet, i * System.Runtime.InteropServices.Marshal.SizeOf(dataType));

            string temp = "";
            for (int i = 0; i < dataLength; i++)
                temp += result[i] + "  ";
            Debug.Log(temp);
        }
    }
}
