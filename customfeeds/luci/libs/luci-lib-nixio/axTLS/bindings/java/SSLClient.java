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
 * @class SSLClient
 * @ingroup java_api 
 * @brief The client context.
 *
 * All client connections are started within a client context.
 */
public class SSLClient extends SSLCTX
{
    /**
     * @brief Start a new client context.
     * 
     * @see SSLCTX for details.
     */
    public SSLClient(int options, int num_sessions)
    {
        super(options, num_sessions);
    }

    /**
     * @brief Establish a new SSL connection to an SSL server.
     *
     * It is up to the application to establish the initial socket connection.
     *
     * This is a blocking call - it will finish when the handshake is 
     * complete (or has failed).
     *
     * Call dispose() when the connection is to be removed.
     * @param s [in] A reference to a <A HREF="http://java.sun.com/j2se/1.4.2/docs/api">Socket</A> object.
     * @param session_id [in] A 32 byte session id for session resumption. This 
     * can be null if no session resumption is not required.
     * @return An SSL object reference. Use SSL.handshakeStatus() to check 
     * if a handshake succeeded.
     */
    public SSL connect(Socket s, byte[] session_id)
    {
        int client_fd = axtlsj.getFd(s);
        byte sess_id_size = (byte)(session_id != null ? 
                                session_id.length : 0);
        return new SSL(axtlsj.ssl_client_new(m_ctx, client_fd, session_id,
                        sess_id_size));
    }
}
