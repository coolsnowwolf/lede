/*
 * Copyright (c) 2007, Cameron Rich
 * 
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, 
 *   this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright notice, 
 *   this list of conditions and the following disclaimer in the documentation 
 *   and/or other materials provided with the distribution.
 * * Neither the name of the axTLS project nor the names of its contributors 
 *   may be used to endorse or promote products derived from this software 
 *   without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * A wrapper around the unmanaged interface to give a semi-decent C# API
 */

using System;
using System.Runtime.InteropServices;
using System.Net.Sockets;

/**
 * @defgroup csharp_api C# API.
 *
 * Ensure that the appropriate Dispose() methods are called when finished with
 * various objects - otherwise memory leaks will result.
 * @{
 */
namespace axTLS
{
    /**
     * @class SSL
     * @ingroup csharp_api 
     * @brief A representation of an SSL connection.
     */
    public class SSL
    {
        public IntPtr m_ssl;    /**< A pointer to the real SSL type */

        /**
         * @brief Store the reference to an SSL context.
         * @param ip [in] A reference to an SSL object.
         */
        public SSL(IntPtr ip)
        {
            m_ssl = ip;
        }

        /**
         * @brief Free any used resources on this connection. 
         * 
         * A "Close Notify" message is sent on this connection (if possible). 
         * It is up to the application to close the socket.
         */
        public void Dispose()
        {
            axtls.ssl_free(m_ssl);
        }

        /**
         * @brief Return the result of a handshake.
         * @return SSL_OK if the handshake is complete and ok.
         * @see ssl.h for the error code list.
         */
        public int HandshakeStatus()
        {
            return axtls.ssl_handshake_status(m_ssl);
        }

        /**
         * @brief Return the SSL cipher id.
         * @return The cipher id which is one of:
         * - SSL_AES128_SHA (0x2f)
         * - SSL_AES256_SHA (0x35)
         * - SSL_RC4_128_SHA (0x05)
         * - SSL_RC4_128_MD5 (0x04)
         */
        public byte GetCipherId()
        {
            return axtls.ssl_get_cipher_id(m_ssl);
        }

        /**
         * @brief Get the session id for a handshake. 
         * 
         * This will be a 32 byte sequence and is available after the first
         * handshaking messages are sent.
         * @return The session id as a 32 byte sequence.
         * @note A SSLv23 handshake may have only 16 valid bytes.
         */
        public byte[] GetSessionId()
        {
            IntPtr ptr = axtls.ssl_get_session_id(m_ssl);
            byte sess_id_size = axtls.ssl_get_session_id_size(m_ssl);
            byte[] result = new byte[sess_id_size];
            Marshal.Copy(ptr, result, 0, sess_id_size);
            return result;
        }

        /**
         * @brief Retrieve an X.509 distinguished name component.
         * 
         * When a handshake is complete and a certificate has been exchanged, 
         * then the details of the remote certificate can be retrieved.
         *
         * This will usually be used by a client to check that the server's 
         * common name matches the URL.
         *
         * A full handshake needs to occur for this call to work.
         *
         * @param component [in] one of:
         * - SSL_X509_CERT_COMMON_NAME
         * - SSL_X509_CERT_ORGANIZATION
         * - SSL_X509_CERT_ORGANIZATIONAL_NAME
         * - SSL_X509_CA_CERT_COMMON_NAME
         * - SSL_X509_CA_CERT_ORGANIZATION
         * - SSL_X509_CA_CERT_ORGANIZATIONAL_NAME
         * @return The appropriate string (or null if not defined)
         */
        public string GetCertificateDN(int component)
        {
            return axtls.ssl_get_cert_dn(m_ssl, component);
        }
    }

    /**
     * @class SSLUtil
     * @ingroup csharp_api 
     * @brief Some global helper functions.
     */
    public class SSLUtil
    {

        /**
         * @brief Return the build mode of the axTLS project.
         * @return The build mode is one of:
         * - SSL_BUILD_SERVER_ONLY
         * - SSL_BUILD_ENABLE_VERIFICATION
         * - SSL_BUILD_ENABLE_CLIENT
         * - SSL_BUILD_FULL_MODE
         */
        public static int BuildMode()
        {
            return axtls.ssl_get_config(axtls.SSL_BUILD_MODE);
        }

        /**
         * @brief Return the number of chained certificates that the 
         * client/server supports.
         * @return The number of supported server certificates.
         */
        public static int MaxCerts()
        {
            return axtls.ssl_get_config(axtls.SSL_MAX_CERT_CFG_OFFSET);
        }

        /**
         * @brief Return the number of CA certificates that the client/server
         * supports.
         * @return The number of supported CA certificates.
         */
        public static int MaxCACerts()
        {
            return axtls.ssl_get_config(axtls.SSL_MAX_CA_CERT_CFG_OFFSET);
        }

        /**
         * @brief Indicate if PEM is supported.
         * @return true if PEM supported.
         */
        public static bool HasPEM()
        {
            return axtls.ssl_get_config(axtls.SSL_HAS_PEM) > 0 ? true : false;
        }

        /**
         * @brief Display the text string of the error.
         * @param error_code [in] The integer error code.
         */
        public static void DisplayError(int error_code)
        {
            axtls.ssl_display_error(error_code);
        }

        /**
         * @brief Return the version of the axTLS project.
         */
        public static string Version()
        {
            return axtls.ssl_version();
        }
    }

    /**
     * @class SSLCTX
     * @ingroup csharp_api 
     * @brief A base object for SSLServer/SSLClient.
     */
    public class SSLCTX
    {
        /**
         * @brief A reference to the real client/server context.
         */
        protected IntPtr m_ctx;

        /**
         * @brief Establish a new client/server context.
         *
         * This function is called before any client/server SSL connections are 
         * made.  If multiple threads are used, then each thread will have its 
         * own SSLCTX context. Any number of connections may be made with a 
         * single context. 
         *
         * Each new connection will use the this context's private key and 
         * certificate chain. If a different certificate chain is required, 
         * then a different context needs to be be used.
         *
         * @param options [in]  Any particular options. At present the options
         * supported are:
         * - SSL_SERVER_VERIFY_LATER (client only): Don't stop a handshake if 
         * the server authentication fails. The certificate can be 
         * authenticated later with a call to VerifyCert().
         * - SSL_CLIENT_AUTHENTICATION (server only): Enforce client 
         * authentication i.e. each handshake will include a "certificate 
         * request" message from the server.
         * - SSL_DISPLAY_BYTES (full mode build only): Display the byte 
         * sequences during the handshake.
         * - SSL_DISPLAY_STATES (full mode build only): Display the state 
         * changes during the handshake.
         * - SSL_DISPLAY_CERTS (full mode build only): Display the 
         * certificates that are passed during a handshake.
         * - SSL_DISPLAY_RSA (full mode build only): Display the RSA key 
         * details that are passed during a handshake.
         * @param num_sessions [in] The number of sessions to be used for 
         * session caching. If this value is 0, then there is no session 
         * caching.
         * @return A client/server context.
         */
        protected SSLCTX(uint options, int num_sessions)
        {
            m_ctx = axtls.ssl_ctx_new(options, num_sessions);
        }

        /**
         * @brief Remove a client/server context.
         *
         * Frees any used resources used by this context. Each connection will 
         * be sent a "Close Notify" alert (if possible).
         */
        public void Dispose()
        {
            axtls.ssl_ctx_free(m_ctx);
        }

        /**
         * @brief Read the SSL data stream.
         * @param ssl [in] An SSL object reference.
         * @param in_data [out] After a successful read, the decrypted data 
         * will be here. It will be null otherwise.
         * @return The number of decrypted bytes:
         * - if > 0, then the handshaking is complete and we are returning the 
         * number of decrypted bytes. 
         * - SSL_OK if the handshaking stage is successful (but not yet 
         * complete).  
         * - < 0 if an error.
         * @see ssl.h for the error code list.
         * @note Use in_data before doing any successive ssl calls.
         */
        public int Read(SSL ssl, out byte[] in_data)
        {
            IntPtr ptr = IntPtr.Zero;
            int ret = axtls.ssl_read(ssl.m_ssl, ref ptr);

            if (ret > axtls.SSL_OK)
            {
                in_data = new byte[ret];
                Marshal.Copy(ptr, in_data, 0, ret);
            }
            else
            {
                in_data = null;
            }

            return ret;
        }

        /**
         * @brief Write to the SSL data stream.
         * @param ssl [in] An SSL obect reference.
         * @param out_data [in] The data to be written
         * @return The number of bytes sent, or if < 0 if an error.
         * @see ssl.h for the error code list.
         */
        public int Write(SSL ssl, byte[] out_data)
        {
            return axtls.ssl_write(ssl.m_ssl, out_data, out_data.Length);
        }

        /**
         * @brief Write to the SSL data stream.
         * @param ssl [in] An SSL obect reference.
         * @param out_data [in] The data to be written
         * @param out_len [in] The number of bytes to be written
         * @return The number of bytes sent, or if < 0 if an error.
         * @see ssl.h for the error code list.
         */
        public int Write(SSL ssl, byte[] out_data, int out_len)
        {
            return axtls.ssl_write(ssl.m_ssl, out_data, out_len);
        }

        /**
         * @brief Find an ssl object based on a Socket reference.
         *
         * Goes through the list of SSL objects maintained in a client/server 
         * context to look for a socket match.
         * @param s [in] A reference to a <A HREF="http://msdn.microsoft.com/library/default.asp?url=/library/en-us/cpref/html/frlrfsystemnetsocketssocketclasstopic.asp">Socket</A> object.
         * @return A reference to the SSL object. Returns null if the object 
         * could not be found.
         */
        public SSL Find(Socket s)
        {
            int client_fd = s.Handle.ToInt32();
            return new SSL(axtls.  ssl_find(m_ctx, client_fd));
        }

        /**
         * @brief Authenticate a received certificate.
         * 
         * This call is usually made by a client after a handshake is complete 
         * and the context is in SSL_SERVER_VERIFY_LATER mode.
         * @param ssl [in] An SSL object reference.
         * @return SSL_OK if the certificate is verified.
         */
        public int VerifyCert(SSL ssl)
        {
            return axtls.ssl_verify_cert(ssl.m_ssl);
        }

        /**
         * @brief Force the client to perform its handshake again.
         *
         * For a client this involves sending another "client hello" message.
         * For the server is means sending a "hello request" message.
         *
         * This is a blocking call on the client (until the handshake 
         * completes).
         * @param ssl [in] An SSL object reference.
         * @return SSL_OK if renegotiation instantiation was ok
         */
        public int Renegotiate(SSL ssl)
        {
            return axtls.ssl_renegotiate(ssl.m_ssl);
        }

        /**
         * @brief Load a file into memory that is in binary DER or ASCII PEM 
         * format.
         *
         * These are temporary objects that are used to load private keys,
         * certificates etc into memory.
         * @param obj_type [in] The format of the file. Can be one of:
         * - SSL_OBJ_X509_CERT (no password required)
         * - SSL_OBJ_X509_CACERT (no password required)
         * - SSL_OBJ_RSA_KEY (AES128/AES256 PEM encryption supported)
         * - SSL_OBJ_P8 (RC4-128 encrypted data supported)
         * - SSL_OBJ_P12 (RC4-128 encrypted data supported)
         *
         * PEM files are automatically detected (if supported).
         * @param filename [in] The location of a file in DER/PEM format.
         * @param password [in] The password used. Can be null if not required.
         * @return SSL_OK if all ok
         */
        public int ObjLoad(int obj_type, string filename, string password)
        {
            return axtls.ssl_obj_load(m_ctx, obj_type, filename, password);
        }

        /**
         * @brief Transfer binary data into the object loader.
         *
         * These are temporary objects that are used to load private keys,
         * certificates etc into memory.
         * @param obj_type [in] The format of the memory data.
         * @param data [in] The binary data to be loaded.
         * @param len [in] The amount of data to be loaded.
         * @param password [in] The password used. Can be null if not required.
         * @return SSL_OK if all ok
         */
        public int ObjLoad(int obj_type, byte[] data, int len, string password)
        {
            return axtls.ssl_obj_memory_load(m_ctx, obj_type, 
                                            data, len, password);
        }
    }

    /**
     * @class SSLServer
     * @ingroup csharp_api 
     * @brief The server context.
     *
     * All server connections are started within a server context.
     */
    public class SSLServer : SSLCTX
    {
        /**
         * @brief Start a new server context.
         * 
         * @see SSLCTX for details.
         */
        public SSLServer(uint options, int num_sessions) :
                            base(options, num_sessions) {}

        /**
         * @brief Establish a new SSL connection to an SSL client.
         *
         * It is up to the application to establish the initial socket 
         * connection.
         *
         * Call Dispose() when the connection is to be removed.
         * @param s [in] A reference to a <A HREF="http://msdn.microsoft.com/library/default.asp?url=/library/en-us/cpref/html/frlrfsystemnetsocketssocketclasstopic.asp">Socket</A> object.
         * @return An SSL object reference.
         */
        public SSL Connect(Socket s)
        {
            int client_fd = s.Handle.ToInt32();
            return new SSL(axtls.ssl_server_new(m_ctx, client_fd));
        }
    }

    /**
     * @class SSLClient
     * @ingroup csharp_api
     * @brief The client context.
     *
     * All client connections are started within a client context.
     */
    public class SSLClient : SSLCTX
    {
        /**
         * @brief Start a new client context.
         * 
         * @see SSLCTX for details.
         */
        public SSLClient(uint options, int num_sessions) :
                        base(options, num_sessions) {}

        /**
         * @brief Establish a new SSL connection to an SSL server.
         *
         * It is up to the application to establish the initial socket 
         * connection.
         *
         * This is a blocking call - it will finish when the handshake is 
         * complete (or has failed).
         *
         * Call Dispose() when the connection is to be removed.
         * @param s [in] A reference to a <A HREF="http://msdn.microsoft.com/library/default.asp?url=/library/en-us/cpref/html/frlrfsystemnetsocketssocketclasstopic.asp">Socket</A> object.
         * @param session_id [in] A 32 byte session id for session resumption. 
         * This can be null if no session resumption is not required.
         * @return An SSL object reference. Use SSL.handshakeStatus() to check 
         * if a handshake succeeded.
         */
        public SSL Connect(Socket s, byte[] session_id)
        {
            int client_fd = s.Handle.ToInt32();
            byte sess_id_size = (byte)(session_id != null ? 
                                session_id.Length : 0);
            return new SSL(axtls.ssl_client_new(m_ctx, client_fd, session_id,
                        sess_id_size));
        }
    }
}
/** @} */
