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

import java.io.*;
import java.util.*;

/**
 * @class SSLUtil
 * @ingroup java_api 
 * @brief Some global helper functions.
 *
 */
public class SSLUtil
{
    /**
     * @brief Load up the ddl/shared library 
     */
    static
    {
        System.loadLibrary("axtlsj");
    }

    /**
     * @brief Return the build mode of the axTLS project.
     * @return The build mode is one of:
     * - SSL_BUILD_SERVER_ONLY
     * - SSL_BUILD_ENABLE_VERIFICATION
     * - SSL_BUILD_ENABLE_CLIENT
     * - SSL_BUILD_FULL_MODE
     */
    public static int buildMode()
    {
        return axtlsj.ssl_get_config(axtlsj.SSL_BUILD_MODE);
    }

    /**
     * @brief Return the number of chained certificates that the client/server 
     * supports.
     * @return The number of supported client/server certificates.
     */
    public static int maxCerts()
    {
        return axtlsj.ssl_get_config(axtlsj.SSL_MAX_CERT_CFG_OFFSET);
    }

    /**
     * @brief Return the number of CA certificates that the client/server
     * supports.
     * @return The number of supported CA certificates.
     */
    public static int maxCACerts()
    {
        return axtlsj.ssl_get_config(axtlsj.SSL_MAX_CA_CERT_CFG_OFFSET);
    }

    /**
     * @brief Indicate if PEM is supported.
     * @return true if PEM supported.
     */
    public static boolean hasPEM()
    {
        return axtlsj.ssl_get_config(axtlsj.SSL_HAS_PEM) > 0 ? true : false;
    }

    /**
     * @brief Display the text string of the error.
     * @param error_code [in] The integer error code.
     * @see ssl.h for the error code list.
     */
    public static void displayError(int error_code)
    {
        axtlsj.ssl_display_error(error_code);
    }

    /**
     * @brief Return the version of the axTLS project.
     */
    public static String version()
    {
        return axtlsj.ssl_version();
    }
}

