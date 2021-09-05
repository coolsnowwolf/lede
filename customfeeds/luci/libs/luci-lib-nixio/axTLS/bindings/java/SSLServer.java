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
 * @class SSLServer
 * @ingroup java_api 
 * @brief The server context.
 *
 * All server connections are started within a server context.
 */
public class SSLServer extends SSLCTX
{
    /**
     * @brief Start a new server context.
     * 
     * @see SSLCTX for details.
     */
    public SSLServer(int options, int num_sessions)
    {
        super(options, num_sessions);
    }

    /**
     * @brief Establish a new SSL connection to an SSL client.
     *
     * It is up to the application to establish the initial socket connection.
     *
     * Call dispose() when the connection is to be removed.
     * @param s [in] A reference to a <A HREF="http://java.sun.com/j2se/1.4.2/docs/api">Socket</A> object.
     * @return An SSL object reference.
     */
    public SSL connect(Socket s)
    {
        int client_fd = axtlsj.getFd(s);
        return new SSL(axtlsj.ssl_server_new(m_ctx, client_fd));
    }
}
