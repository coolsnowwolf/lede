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

/*
 * A wrapper around the unmanaged interface to give a semi-decent Java API
 */

package axTLSj;

import java.net.*;

/**
 * @class SSLCTX
 * @ingroup java_api 
 * @brief A base object for SSLServer/SSLClient.
 */
public class SSLCTX
{
    /**
     * A reference to the real client/server context.
     */
    protected int m_ctx;

    /**
     * @brief Establish a new client/server context.
     *
     * This function is called before any client/server SSL connections are 
     * made.  If multiple threads are used, then each thread will have its 
     * own SSLCTX context. Any number of connections may be made with a single 
     * context. 
     *
     * Each new connection will use the this context's private key and 
     * certificate chain. If a different certificate chain is required, then a 
     * different context needs to be be used.
     *
     * @param options [in]  Any particular options. At present the options
     * supported are:
     * - SSL_SERVER_VERIFY_LATER (client only): Don't stop a handshake if the 
     * server authentication fails. The certificate can be authenticated later 
     * with a call to verifyCert().
     * - SSL_CLIENT_AUTHENTICATION (server only): Enforce client authentication
     * i.e. each handshake will include a "certificate request" message from 
     * the server.
     * - SSL_DISPLAY_BYTES (full mode build only): Display the byte sequences
     * during the handshake.
     * - SSL_DISPLAY_STATES (full mode build only): Display the state changes
     * during the handshake.
     * - SSL_DISPLAY_CERTS (full mode build only): Display the certificates that
     * are passed during a handshake.
     * - SSL_DISPLAY_RSA (full mode build only): Display the RSA key details 
     * that are passed during a handshake.
     *
     * @param num_sessions [in] The number of sessions to be used for session
     * caching. If this value is 0, then there is no session caching.
     * 
     * If this option is null, then the default internal private key/
     * certificate pair is used (if CONFIG_SSL_USE_DEFAULT_KEY is set). 
     * 
     * The resources used by this object are automatically freed.  
     * @return A client/server context.
     */
    protected SSLCTX(int options, int num_sessions)
    {
        m_ctx = axtlsj.ssl_ctx_new(options, num_sessions);
    }

    /**
     * @brief Remove a client/server context.
     *
     * Frees any used resources used by this context. Each connection will be 
     * sent a "Close Notify" alert (if possible).
     */
    public void dispose()
    {
        axtlsj.ssl_ctx_free(m_ctx);
    }

    /**
     * @brief Read the SSL data stream.
     * @param ssl [in] An SSL object reference.
     * @param rh [out] After a successful read, the decrypted data can be 
     * retrieved with rh.getData(). It will be null otherwise.
     * @return The number of decrypted bytes:
     * - if > 0, then the handshaking is complete and we are returning the 
     * number of decrypted bytes. 
     * - SSL_OK if the handshaking stage is successful (but not yet complete).  
     * - < 0 if an error.
     * @see ssl.h for the error code list.
     * @note Use rh before doing any successive ssl calls.
     */
    public int read(SSL ssl, SSLReadHolder rh)
    {
        return axtlsj.ssl_read(ssl.m_ssl, rh);
    }

    /**
     * @brief Write to the SSL data stream.
     * @param ssl [in] An SSL obect reference.
     * @param out_data [in] The data to be written
     * @return The number of bytes sent, or if < 0 if an error.
     * @see ssl.h for the error code list.
     */
    public int write(SSL ssl, byte[] out_data)
    {
        return axtlsj.ssl_write(ssl.m_ssl, out_data, out_data.length);
    }

    /**
     * @brief Write to the SSL data stream.
     * @param ssl [in] An SSL obect reference.
     * @param out_data [in] The data to be written
     * @param out_len [in] The number of bytes to be written
     * @return The number of bytes sent, or if < 0 if an error.
     * @see ssl.h for the error code list.
     */
    public int write(SSL ssl, byte[] out_data, int out_len)
    {
        return axtlsj.ssl_write(ssl.m_ssl, out_data, out_len);
    }

    /**
     * @brief Find an ssl object based on a Socket reference.
     *
     * Goes through the list of SSL objects maintained in a client/server 
     * context to look for a socket match.
     * @param s [in] A reference to a <A HREF="http://java.sun.com/j2se/1.4.2/docs/api">Socket</A> object.
     * @return A reference to the SSL object. Returns null if the object 
     * could not be found.
     */
    public SSL find(Socket s)
    {
        int client_fd = axtlsj.getFd(s);
        return new SSL(axtlsj.ssl_find(m_ctx, client_fd));
    }

    /**
     * @brief Authenticate a received certificate.
     * 
     * This call is usually made by a client after a handshake is complete 
     * and the context is in SSL_SERVER_VERIFY_LATER mode.
     * @param ssl [in] An SSL object reference.
     * @return SSL_OK if the certificate is verified.
     */
    public int verifyCert(SSL ssl)
    {
        return axtlsj.ssl_verify_cert(ssl.m_ssl);
    }

    /**
     * @brief Force the client to perform its handshake again.
     *
     * For a client this involves sending another "client hello" message.
     * For the server is means sending a "hello request" message.
     *
     * This is a blocking call on the client (until the handshake completes).
     * @param ssl [in] An SSL object reference.
     * @return SSL_OK if renegotiation instantiation was ok
     */
    public int renegotiate(SSL ssl)
    {
        return axtlsj.ssl_renegotiate(ssl.m_ssl);
    }

    /**
     * @brief Load a file into memory that is in binary DER or ASCII PEM format.
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
    public int objLoad(int obj_type, String filename, String password)
    {
        return axtlsj.ssl_obj_load(m_ctx, obj_type, filename, password);
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

    public int objLoad(int obj_type, byte[] data, int len, String password)
    {
        return axtlsj.ssl_obj_memory_load(m_ctx, obj_type, data, len, password);
    }
}
