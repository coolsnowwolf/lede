'
' Copyright (c) 2007, Cameron Rich
'
' All rights reserved.
'
' Redistribution and use in source and binary forms, with or without
' modification, are permitted provided that the following conditions are met:
'
' * Redistributions of source code must retain the above copyright notice,
'   this list of conditions and the following disclaimer.
' * Redistributions in binary form must reproduce the above copyright
'   notice, this list of conditions and the following disclaimer in the
'   documentation and/or other materials provided with the distribution.
' * Neither the name of the axTLS project nor the names of its
'   contributors may be used to endorse or promote products derived
'   from this software without specific prior written permission.
'
' THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
' "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
' LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
' A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
' CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
' SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
' TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
' DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY 
' OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
' NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
' THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
'

'
' Demonstrate the use of the axTLS library in VB.NET with a set of 
' command-line parameters similar to openssl. In fact, openssl clients 
' should be able to communicate with axTLS servers and visa-versa.
'
' This code has various bits enabled depending on the configuration. To enable
' the most interesting version, compile with the 'full mode' enabled.
'
' To see what options you have, run the following:
' > axssl.vbnet.exe s_server -?
' > axssl.vbnet.exe s_client -?
'
' The axtls shared library must be in the same directory or be found 
' by the OS.
'

Imports System
Imports System.Net
Imports System.Net.Sockets
Imports Microsoft.VisualBasic
Imports axTLSvb

Public Class axssl
    ' 
    ' do_server()
    '
    Public Sub do_server(ByVal build_mode As Integer, _
                                        ByVal args() As String)
        Dim i As Integer = 1
        Dim port As Integer = 4433
        Dim options As Integer = axtls.SSL_DISPLAY_CERTS
        Dim quiet As Boolean = False
        Dim password As String = Nothing
        Dim private_key_file As String = Nothing

        ' organise the cert/ca_cert lists 
        Dim cert_size As Integer = SSLUtil.MaxCerts()
        Dim ca_cert_size As Integer = SSLUtil.MaxCACerts()
        Dim cert(cert_size) As String
        Dim ca_cert(ca_cert_size) As String
        Dim cert_index As Integer = 0
        Dim ca_cert_index As Integer = 0

        While i < args.Length
            If args(i) = "-accept" Then
                If i >= args.Length-1
                    print_server_options(build_mode, args(i))
                End If

                i += 1
                port = Int32.Parse(args(i))
            ElseIf args(i) = "-quiet"
                quiet = True
                options = options And Not axtls.SSL_DISPLAY_CERTS
            ElseIf build_mode >= axtls.SSL_BUILD_SERVER_ONLY
                If args(i) = "-cert"
                    If i >= args.Length-1 Or cert_index >= cert_size
                        print_server_options(build_mode, args(i))
                    End If

                    i += 1
                    cert(cert_index) = args(i)
                    cert_index += 1
                ElseIf args(i) = "-key"
                    If i >= args.Length-1
                        print_server_options(build_mode, args(i))
                    End If

                    i += 1
                    private_key_file = args(i)
                    options = options Or axtls.SSL_NO_DEFAULT_KEY
                ElseIf args(i) = "-pass"
                    If i >= args.Length-1
                        print_server_options(build_mode, args(i))
                    End If

                    i += 1
                    password = args(i)
                ElseIf build_mode >= axtls.SSL_BUILD_ENABLE_VERIFICATION
                    If args(i) = "-verify" Then
                        options = options Or axtls.SSL_CLIENT_AUTHENTICATION
                    ElseIf args(i) = "-CAfile"
                        If i >= args.Length-1 Or _
                                    ca_cert_index >= ca_cert_size Then
                            print_server_options(build_mode, args(i))
                        End If

                        i += 1
                        ca_cert(ca_cert_index) = args(i)
                        ca_cert_index += 1
                    ElseIf build_mode = axtls.SSL_BUILD_FULL_MODE
                        If args(i) = "-debug" Then
                            options = options Or axtls.SSL_DISPLAY_BYTES
                        ElseIf args(i) = "-state"
                            options = options Or axtls.SSL_DISPLAY_STATES
                        ElseIf args(i) = "-show-rsa"
                            options = options Or axtls.SSL_DISPLAY_RSA
                        Else
                            print_server_options(build_mode, args(i))
                        End If
                    Else
                        print_server_options(build_mode, args(i))
                    End If
                Else
                    print_server_options(build_mode, args(i))
                End If
            End If

            i += 1
        End While

        ' Create socket for incoming connections
        Dim ep As IPEndPoint = New IPEndPoint(IPAddress.Any, port)
        Dim server_sock As TcpListener = New TcpListener(ep)
        server_sock.Start()      

        '*********************************************************************
        ' This is where the interesting stuff happens. Up until now we've
        ' just been setting up sockets etc. Now we do the SSL handshake.
        '*********************************************************************/
        Dim ssl_ctx As SSLServer = New SSLServer(options, _
                axtls.SSL_DEFAULT_SVR_SESS)

        If ssl_ctx Is Nothing Then
            Console.Error.WriteLine("Error: Server context is invalid")
            Environment.Exit(1)
        End If

        If private_key_file <> Nothing Then
            Dim obj_type As Integer = axtls.SSL_OBJ_RSA_KEY

            If private_key_file.EndsWith(".p8") Then
                obj_type = axtls.SSL_OBJ_PKCS8
            Else If (private_key_file.EndsWith(".p12"))
                obj_type = axtls.SSL_OBJ_PKCS12
            End If

            If ssl_ctx.ObjLoad(obj_type, private_key_file, _
                                            password) <> axtls.SSL_OK Then
                Console.Error.WriteLine("Error: Private key '" & _
                        private_key_file & "' is undefined.")
                Environment.Exit(1)
            End If
        End If

        For i = 0 To cert_index-1
            If ssl_ctx.ObjLoad(axtls.SSL_OBJ_X509_CERT, _
                            cert(i), Nothing) <> axtls.SSL_OK Then
                Console.WriteLine("Certificate '" & cert(i) & _
                        "' is undefined.")
                Environment.Exit(1)
            End If
        Next

        For i = 0 To ca_cert_index-1
            If ssl_ctx.ObjLoad(axtls.SSL_OBJ_X509_CACERT, _
                            ca_cert(i), Nothing) <> axtls.SSL_OK Then
                Console.WriteLine("Certificate '" & ca_cert(i) & _
                        "' is undefined.")
                Environment.Exit(1)
            End If
        Next

        Dim buf As Byte() = Nothing
        Dim res As Integer
        Dim ssl As SSL

        While 1
            If Not quiet Then
                Console.WriteLine("ACCEPT")
            End If

            Dim client_sock As Socket = server_sock.AcceptSocket()

            ssl = ssl_ctx.Connect(client_sock)

            ' do the actual SSL handshake 
            While 1
                res = ssl_ctx.Read(ssl, buf)
                If  res <> axtls.SSL_OK Then
                    Exit While
                End If

                ' check when the connection has been established 
                If ssl.HandshakeStatus() = axtls.SSL_OK
                    Exit While
                End If

                ' could do something else here 
            End While

            If res = axtls.SSL_OK Then  ' connection established and ok
                If Not quiet
                    display_session_id(ssl)
                    display_cipher(ssl)
                End If

                ' now read (and display) whatever the client sends us
                While 1
                    ' keep reading until we get something interesting 
                    While 1
                        res = ssl_ctx.Read(ssl, buf)
                        If res <> axtls.SSL_OK Then
                            Exit While
                        End If

                        ' could do something else here
                    End While

                    If res < axtls.SSL_OK
                        If Not quiet
                            Console.WriteLine("CONNECTION CLOSED")
                        End If

                        Exit While
                    End If

                    ' convert to String 
                    Dim str(res) As Char
                    For i = 0 To res-1
                        str(i) = Chr(buf(i))
                    Next

                    Console.Write(str)
                End While
            ElseIf Not quiet
                SSLUtil.DisplayError(res)
            End If

            ' client was disconnected or the handshake failed. */
            ssl.Dispose()
            client_sock.Close()
        End While

        ssl_ctx.Dispose()
    End Sub

    ' 
    ' do_client()
    '
    Public Sub do_client(ByVal build_mode As Integer, _
                                    ByVal args() As String)

        If build_mode < axtls.SSL_BUILD_ENABLE_CLIENT Then
            print_client_options(build_mode, args(1))
        End If

        Dim i As Integer = 1
        Dim res As Integer
        Dim port As Integer = 4433
        Dim quiet As Boolean = False
        Dim password As String = Nothing
        Dim reconnect As Integer = 0
        Dim private_key_file As String = Nothing
        Dim hostname As String = "127.0.0.1"

        ' organise the cert/ca_cert lists
        Dim ssl As SSL = Nothing
        Dim cert_size As Integer = SSLUtil.MaxCerts()
        Dim ca_cert_size As Integer = SSLUtil.MaxCACerts()
        Dim cert(cert_size) As String
        Dim ca_cert(ca_cert_size) As String
        Dim cert_index As Integer = 0
        Dim ca_cert_index As Integer = 0

        Dim options As Integer = _
                    axtls.SSL_SERVER_VERIFY_LATER Or axtls.SSL_DISPLAY_CERTS
        Dim session_id As Byte() = Nothing

        While i < args.Length
            If args(i) = "-connect" Then
                Dim host_port As String

                If i >= args.Length-1
                    print_client_options(build_mode, args(i))
                End If

                i += 1
                host_port = args(i)

                Dim index_colon As Integer = host_port.IndexOf(":"C)
                If index_colon < 0 Then 
                    print_client_options(build_mode, args(i))
                End If

                hostname = New String(host_port.ToCharArray(), _
                        0, index_colon)
                port = Int32.Parse(New String(host_port.ToCharArray(), _
                            index_colon+1, host_port.Length-index_colon-1))
            ElseIf args(i) = "-cert"
                If i >= args.Length-1 Or cert_index >= cert_size Then
                    print_client_options(build_mode, args(i))
                End If

                i += 1
                cert(cert_index) = args(i)
                cert_index += 1
            ElseIf args(i) = "-key"
                If i >= args.Length-1
                    print_client_options(build_mode, args(i))
                End If

                i += 1
                private_key_file = args(i)
                options = options Or axtls.SSL_NO_DEFAULT_KEY
            ElseIf args(i) = "-CAfile"
                If i >= args.Length-1 Or ca_cert_index >= ca_cert_size
                    print_client_options(build_mode, args(i))
                End If

                i += 1
                ca_cert(ca_cert_index) = args(i)
                ca_cert_index += 1
            ElseIf args(i) = "-verify"
                options = options And Not axtls.SSL_SERVER_VERIFY_LATER
            ElseIf args(i) = "-reconnect"
                reconnect = 4
            ElseIf args(i) = "-quiet"
                quiet = True
                options = options And  Not axtls.SSL_DISPLAY_CERTS
            ElseIf args(i) = "-pass"
                If i >= args.Length-1
                    print_client_options(build_mode, args(i))
                End If

                i += 1
                password = args(i)
            ElseIf build_mode = axtls.SSL_BUILD_FULL_MODE
                If args(i) = "-debug" Then
                    options = options Or axtls.SSL_DISPLAY_BYTES
                ElseIf args(i) = "-state"
                    options = options Or axtls.SSL_DISPLAY_STATES
                ElseIf args(i) = "-show-rsa"
                    options = options Or axtls.SSL_DISPLAY_RSA
                Else
                    print_client_options(build_mode, args(i))
                End If
            Else    ' don't know what this is 
                print_client_options(build_mode, args(i))
            End If

            i += 1
        End While

        'Dim hostInfo As IPHostEntry = Dns.Resolve(hostname)
        Dim hostInfo As IPHostEntry = Dns.GetHostEntry(hostname)
        Dim  addresses As IPAddress() = hostInfo.AddressList
        Dim ep As IPEndPoint = New IPEndPoint(addresses(0), port) 
        Dim client_sock As Socket = New Socket(AddressFamily.InterNetwork, _
                SocketType.Stream, ProtocolType.Tcp)
        client_sock.Connect(ep)

        If Not client_sock.Connected Then
            Console.WriteLine("could not connect")
            Environment.Exit(1)
        End If

        If Not quiet Then
            Console.WriteLine("CONNECTED")
        End If

        '*********************************************************************
        ' This is where the interesting stuff happens. Up until now we've
        ' just been setting up sockets etc. Now we do the SSL handshake.
        '*********************************************************************/
        Dim ssl_ctx As SSLClient = New SSLClient(options, _
                axtls.SSL_DEFAULT_CLNT_SESS)

        If ssl_ctx Is Nothing Then
            Console.Error.WriteLine("Error: Client context is invalid")
            Environment.Exit(1)
        End If

        If private_key_file <> Nothing Then
            Dim obj_type As Integer = axtls.SSL_OBJ_RSA_KEY

            If private_key_file.EndsWith(".p8") Then
                obj_type = axtls.SSL_OBJ_PKCS8
            Else If (private_key_file.EndsWith(".p12"))
                obj_type = axtls.SSL_OBJ_PKCS12
            End If

            If ssl_ctx.ObjLoad(obj_type, private_key_file, _
                                            password) <> axtls.SSL_OK Then
                Console.Error.WriteLine("Error: Private key '" & _
                        private_key_file & "' is undefined.")
                Environment.Exit(1)
            End If
        End If

        For i = 0 To cert_index-1
            If ssl_ctx.ObjLoad(axtls.SSL_OBJ_X509_CERT, _
                            cert(i), Nothing) <> axtls.SSL_OK Then
                Console.WriteLine("Certificate '" & cert(i) & _
                        "' is undefined.")
                Environment.Exit(1)
            End If
        Next

        For i = 0 To ca_cert_index-1
            If ssl_ctx.ObjLoad(axtls.SSL_OBJ_X509_CACERT, _
                            ca_cert(i), Nothing) <> axtls.SSL_OK Then
                Console.WriteLine("Certificate '" & ca_cert(i) & _
                        "' is undefined.")
                Environment.Exit(1)
            End If
        Next

        ' Try session resumption?
        If reconnect > 0 Then
            While reconnect > 0
                reconnect -= 1
                ssl = ssl_ctx.Connect(client_sock, session_id)

                res = ssl.HandshakeStatus()
                If res <> axtls.SSL_OK Then
                    If Not quiet Then
                        SSLUtil.DisplayError(res)
                    End If

                    ssl.Dispose()
                    Environment.Exit(1)
                End If

                display_session_id(ssl)
                session_id = ssl.GetSessionId()

                If reconnect > 0 Then
                    ssl.Dispose()
                    client_sock.Close()
                    
                    ' and reconnect
                    client_sock = New Socket(AddressFamily.InterNetwork, _
                        SocketType.Stream, ProtocolType.Tcp)
                    client_sock.Connect(ep)
                End If
            End While
        Else
            ssl = ssl_ctx.Connect(client_sock, Nothing)
        End If

        ' check the return status 
        res = ssl.HandshakeStatus()
        If res <> axtls.SSL_OK Then
            If Not quiet Then
                SSLUtil.DisplayError(res)
            End If

            Environment.Exit(1)
        End If

        If Not quiet Then
            Dim common_name As String = _
                ssl.GetCertificateDN(axtls.SSL_X509_CERT_COMMON_NAME)

            If common_name <> Nothing
                Console.WriteLine("Common Name:" & _
                        ControlChars.Tab & ControlChars.Tab & _
                        ControlChars.Tab & common_name)
            End If

            display_session_id(ssl)
            display_cipher(ssl)
        End If

        While (1)
            Dim user_input As String = Console.ReadLine()

            If user_input = Nothing Then
                Exit While
            End If

            Dim buf(user_input.Length+1) As Byte
            buf(buf.Length-2) = Asc(ControlChars.Lf) ' add the carriage return
            buf(buf.Length-1) = 0                    ' null terminate 

            For i = 0 To user_input.Length-1
                buf(i) = Asc(user_input.Chars(i))
            Next

            res = ssl_ctx.Write(ssl, buf, buf.Length)
            If res < axtls.SSL_OK Then
                If Not quiet Then
                    SSLUtil.DisplayError(res)
                End If

                Exit While
            End If
        End While

        ssl_ctx.Dispose()
    End Sub

    '
    ' Display what cipher we are using
    '
    Private Sub display_cipher(ByVal ssl As SSL)
        Console.Write("CIPHER is ")

        Select ssl.GetCipherId()
            Case axtls.SSL_AES128_SHA
                Console.WriteLine("AES128-SHA")

            Case axtls.SSL_AES256_SHA
                Console.WriteLine("AES256-SHA")

            Case axtls.SSL_RC4_128_SHA
                Console.WriteLine("RC4-SHA")

            Case axtls.SSL_RC4_128_MD5
                Console.WriteLine("RC4-MD5")

            Case Else
                Console.WriteLine("Unknown - " & ssl.GetCipherId())
        End Select
    End Sub

    '
    ' Display what session id we have.
    '
    Private Sub display_session_id(ByVal ssl As SSL)
        Dim session_id As Byte() = ssl.GetSessionId()

        If session_id.Length > 0 Then
            Console.WriteLine("-----BEGIN SSL SESSION PARAMETERS-----")
            Dim b As Byte
            For Each b In session_id
                Console.Write("{0:x02}", b)
            Next

            Console.WriteLine()
            Console.WriteLine("-----END SSL SESSION PARAMETERS-----")
        End If
    End Sub

    ' 
    ' We've had some sort of command-line error. Print out the basic options.
    '
    Public Sub print_options(ByVal options As String)
        Console.WriteLine("axssl: Error: '" & options & _
                "' is an invalid command.")
        Console.WriteLine("usage: axssl.vbnet [s_server|s_client|" & _
                "version] [args ...]")
        Environment.Exit(1)
    End Sub

    ' 
    ' We've had some sort of command-line error. Print out the server options.
    '
    Private Sub print_server_options(ByVal build_mode As Integer, _
                                    ByVal options As String)
        Dim cert_size As Integer = SSLUtil.MaxCerts()
        Dim ca_cert_size As Integer = SSLUtil.MaxCACerts()

        Console.WriteLine("unknown option " & options)
        Console.WriteLine("usage: s_server [args ...]")
        Console.WriteLine(" -accept arg" & ControlChars.Tab & _
                "- port to accept on (default is 4433)")
        Console.WriteLine(" -quiet" & ControlChars.Tab & ControlChars.Tab & _
                "- No server output")
        If build_mode >= axtls.SSL_BUILD_SERVER_ONLY 
            Console.WriteLine(" -cert arg" & ControlChars.Tab & _
               "- certificate file to add (in addition to default) to chain -")
            Console.WriteLine(ControlChars.Tab & ControlChars.Tab & _
                    "  Can repeat up to " & cert_size & " times")
            Console.WriteLine(" -key arg" & ControlChars.Tab & _
                        "- Private key file to use")
            Console.WriteLine(" -pass" & ControlChars.Tab & ControlChars.Tab & _
                    "- private key file pass phrase source")
        End If

        If build_mode >= axtls.SSL_BUILD_ENABLE_VERIFICATION
            Console.WriteLine(" -verify" & ControlChars.Tab & _
                    "- turn on peer certificate verification")
            Console.WriteLine(" -CAfile arg" & ControlChars.Tab & _
                    "- Certificate authority")
            Console.WriteLine(ControlChars.Tab & ControlChars.Tab & _
                    "  Can repeat up to " & ca_cert_size & " times")
        End If

        If build_mode = axtls.SSL_BUILD_FULL_MODE
            Console.WriteLine(" -debug" & _
                    ControlChars.Tab & ControlChars.Tab & _
                    "- Print more output")
            Console.WriteLine(" -state" & _
                    ControlChars.Tab & ControlChars.Tab & _
                    "- Show state messages")
            Console.WriteLine(" -show-rsa" & _
                    ControlChars.Tab & "- Show RSA state")
        End If

        Environment.Exit(1)
    End Sub

    '
    ' We've had some sort of command-line error. Print out the client options.
    '
    Private Sub print_client_options(ByVal build_mode As Integer, _
                                                ByVal options As String)
        Dim cert_size As Integer = SSLUtil.MaxCerts()
        Dim ca_cert_size As Integer = SSLUtil.MaxCACerts()

        Console.WriteLine("unknown option " & options)

        If build_mode >= axtls.SSL_BUILD_ENABLE_CLIENT Then
            Console.WriteLine("usage: s_client [args ...]")
            Console.WriteLine(" -connect host:port - who to connect to " & _
                    "(default is localhost:4433)")
            Console.WriteLine(" -verify" & ControlChars.Tab & _
                    "- turn on peer certificate verification")
            Console.WriteLine(" -cert arg" & ControlChars.Tab & _
                    "- certificate file to use")
            Console.WriteLine(ControlChars.Tab & ControlChars.Tab & _
                    "  Can repeat up to " & cert_size & " times")
            Console.WriteLine(" -key arg" & ControlChars.Tab & _
                    "- Private key file to use")
            Console.WriteLine(" -CAfile arg" & ControlChars.Tab & _
                    "- Certificate authority")
            Console.WriteLine(ControlChars.Tab & ControlChars.Tab & _
                    "  Can repeat up to " & ca_cert_size & " times")
            Console.WriteLine(" -quiet" & _
                    ControlChars.Tab & ControlChars.Tab & "- No client output")
            Console.WriteLine(" -pass" & ControlChars.Tab & _
                    ControlChars.Tab & _
                    "- private key file pass phrase source")
            Console.WriteLine(" -reconnect" & ControlChars.Tab & _
                    "- Drop and re-make the " & _
                    "connection with the same Session-ID")

            If build_mode = axtls.SSL_BUILD_FULL_MODE Then
                Console.WriteLine(" -debug" & _
                        ControlChars.Tab & ControlChars.Tab & _
                        "- Print more output")
                Console.WriteLine(" -state" & _
                        ControlChars.Tab & ControlChars.Tab & _
                        "- Show state messages")
                Console.WriteLine(" -show-rsa" & ControlChars.Tab & _
                        "- Show RSA state")
            End If
        Else 
            Console.WriteLine("Change configuration to allow this feature")
        End If

        Environment.Exit(1)
    End Sub

End Class

Public Module MyMain
    Function Main(ByVal args() As String) As Integer
        Dim runner As axssl = New axssl()

        If args.Length = 1 And args(0) = "version" Then
           Console.WriteLine("axssl.vbnet " & SSLUtil.Version())
            Environment.Exit(0)
        End If

        If args.Length < 1 
            runner.print_options("")
        ElseIf args(0) <> "s_server" And args(0) <> "s_client"
            runner.print_options(args(0))
        End If

        Dim build_mode As Integer = SSLUtil.BuildMode()

        If args(0) = "s_server" Then
            runner.do_server(build_mode, args)
        Else
            runner.do_client(build_mode, args)
        End If
    End Function
End Module
