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
' A wrapper around the unmanaged Integererface to give a semi-decent VB.NET API
'

Imports System
Imports System.Runtime.InteropServices
Imports System.Net.Sockets
Imports axTLSvb

Namespace axTLSvb
    Public Class SSL
        Public m_ssl As IntPtr

        Public Sub New(ByRef ip As IntPtr)
            m_ssl = ip
        End Sub

        Public Sub Dispose()
            axtls.ssl_free(m_ssl)
        End Sub

        Public Function HandshakeStatus() As Integer
            Return axtls.ssl_handshake_status(m_ssl)
        End Function

        Public Function GetCipherId() As Byte
            Return axtls.ssl_get_cipher_id(m_ssl)
        End Function

        Public Function GetSessionId() As Byte()
            Dim ptr As IntPtr = axtls.ssl_get_session_id(m_ssl)
            Dim sess_id_size As Integer = axtls.ssl_get_session_id_size(m_ssl)
            Dim result(sess_id_size-1) As Byte
            Marshal.Copy(ptr, result, 0, sess_id_size)
            Return result
        End Function

        Public Function GetCertificateDN(component As Integer) As String
            Return axtls.ssl_get_cert_dn(m_ssl, component)
        End Function
    End Class

    Public Class SSLUtil
        Private dummy As Integer    ' need something here

        Public Shared Function BuildMode() As Integer
            Return axtls.ssl_get_config(axtls.SSL_BUILD_MODE)
        End Function

        Public Shared Function MaxCerts() As Integer
            Return axtls.ssl_get_config(axtls.SSL_MAX_CERT_CFG_OFFSET)
        End Function

        Public Shared Function MaxCACerts() As Integer
            Return axtls.ssl_get_config(axtls.SSL_MAX_CA_CERT_CFG_OFFSET)
        End Function

        Public Shared Function HasPEM() As Boolean
            If axtls.ssl_get_config(axtls.SSL_HAS_PEM) > 0 Then
                Return True
            Else
                Return False
            End If
        End Function
        
        Public Shared Sub DisplayError(ByVal error_code As Integer)
            axtls.ssl_display_error(error_code)
        End Sub

        Public Shared Function Version() As String
            Return axtls.ssl_version()
        End Function
    End Class

    Public Class SSLCTX
        Protected m_ctx As IntPtr

        Protected Sub New(ByVal options As Integer, _
                ByVal num_sessions As Integer)
            m_ctx = axtls.ssl_ctx_new(options, num_sessions)
        End Sub

        Public Sub Dispose()
            axtls.ssl_ctx_free(m_ctx)
        End Sub

        Public Function Read(ByVal ssl As SSL, ByRef in_data As Byte()) As Integer
            Dim ptr As IntPtr = IntPtr.Zero
            Dim ret as Integer = axtls.ssl_read(ssl.m_ssl, ptr)

            If ret > axtls.SSL_OK Then
                ReDim in_data(ret)
                Marshal.Copy(ptr, in_data, 0, ret)
            Else
                in_data = Nothing
            End If

            Return ret
        End Function

        Public Function Write(ByVal ssl As SSL, _
                ByVal data As Byte(), len As Integer) As Integer
            Return axtls.ssl_write(ssl.m_ssl, data, len)
        End Function

        Public Function Find(ByVal s As Socket) As SSL
            Dim client_fd As Integer = s.Handle.ToInt32()
            Return New SSL(axtls.ssl_find(m_ctx, client_fd))
        End Function

        Public Function VerifyCert(ByVal ssl As SSL) As Integer
            Return axtls.ssl_verify_cert(ssl.m_ssl)
        End Function

        Public Function Renegotiate(ByVal ssl As SSL) As Integer
            Return axtls.ssl_renegotiate(ssl.m_ssl)
        End Function

        Public Function ObjLoad(ByVal obj_type As Integer, _
                ByVal filename As String, _
                password As String) As Integer
            Return axtls.ssl_obj_load(m_ctx, obj_type, filename, password)
        End Function

        Public Function ObjLoad(ByVal obj_type As Integer, _
                ByVal data As Byte(), ByVal len As Integer, _
                password As String) As Integer
            Return axtls.ssl_obj_memory_load( _
                    m_ctx, obj_type, data, len, password)
        End Function
    End Class

    Public Class SSLServer 
            Inherits SSLCTX

        Public Sub New(ByVal options As Integer, _
                ByVal num_sessions As Integer)
            MyBase.New(options, num_sessions)
        End Sub

        Public Function Connect(ByVal s As Socket) As SSL
            Dim client_fd As Integer = s.Handle.ToInt32()
            Return New SSL(axtls.ssl_server_new(m_ctx, client_fd))
        End Function
    End Class

    Public Class SSLClient 
            Inherits SSLCTX

        Public Sub New(ByVal options As Integer, _
                ByVal num_sessions As Integer)
            MyBase.New(options, num_sessions)
        End Sub

        Public Function Connect(ByVal s As Socket, _
                                ByVal session_id As Byte()) As SSL
            Dim client_fd As Integer = s.Handle.ToInt32()
            Dim sess_id_size As Byte
            If session_id is Nothing Then
                sess_id_size = 0
            Else
                sess_id_size = session_id.Length
            End If

            Return New SSL(axtls.ssl_client_new(m_ctx, client_fd, session_id, _
                       sess_id_size))
        End Function

    End Class
End Namespace
