/*
 * ssl-utils.C: SSL utility functions for protocol check module
 *
 * Version: $Revision: 0.5 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2003/04/09 19:54:24 $
 * MT-Level: Unsafe, except as noted
 *
 * Copyright (c) 2002 - 2003
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: ssl-utils.C,v $
 * Revision 0.5  2003/04/09 19:54:24  benno
 * dlog and wlog Debuggers
 *
 * Revision 0.4  2003/01/29 01:26:49  benno
 * IOTF
 *
 * Revision 0.3  2002/04/04 21:02:32  benno
 * copyright
 *
 * Revision 0.2  2002/04/03 21:46:07  benno
 * rcsify date
 *
 * Revision 0.1  2002/04/03 21:44:16  benno
 * initial revision
 *
 */

#include "protocol.H"

#if defined(WITH_SSL)
// Global SSL context
SSL_CTX *ctx = NULL;

// Multithreaded locking helpers
static pthread_mutex_t *lock_cs = NULL;
static long *lock_count = NULL;

#if defined(DEBUG)
BIO *bio_stderr = NULL;
#endif

void posix_locking_callback(int mode, int type, char *file, int line)
{
  // Callback routine to make OpenSSL thread safe.

  // Returns: Nothing.
  
  if(mode & CRYPTO_LOCK)
  {
    pthread_mutex_lock(&(lock_cs[type]));
    lock_count[type]++;
  }
  else
    pthread_mutex_unlock(&(lock_cs[type]));
}

void ssl_discontinue()
{
  // Clean up and discontinue SSL.

  // Returns: Nothing.
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "ssl_discontinue()");
#endif

  if(lock_cs)
  {
    CRYPTO_set_locking_callback(NULL);

    for(int i = 0;i < CRYPTO_num_locks();i++)
      pthread_mutex_destroy(&(lock_cs[i]));

    free(lock_cs);
    lock_cs = NULL;
    
    free(lock_count);
    lock_count = NULL;
  }

  if(ctx)
  {
    SSL_CTX_free(ctx);
    ctx = NULL;
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "ssl_discontinue()");
#endif
}

bool ssl_initialize()
{
  // Perform SSL initializations.

  // Returns: true if fully successful, false otherwise.

  bool r = false;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MAJTRC, "ssl_initialize()");
#endif

  SSL_METHOD *ssl_method = SSLv23_method();
  
#if defined(DEBUG)
  bio_stderr = BIO_new_fp(stderr, BIO_NOCLOSE);
#endif

  // OpenSSL requires some randomness in order to initialize.
  // This isn't the best solution, but should be adequate for these
  // purposes.
  RAND_load_file("/etc/passwd", -1);
	    
  OpenSSL_add_ssl_algorithms();
  SSL_load_error_strings();

  ctx = SSL_CTX_new(ssl_method);

  if(ctx)
  {
    SSL_CTX_set_options(ctx, 0);
    SSL_CTX_set_session_cache_mode(ctx, SSL_SESS_CACHE_NO_AUTO_CLEAR |
				        SSL_SESS_CACHE_SERVER);

    lock_cs = (pthread_mutex_t *)malloc(CRYPTO_num_locks()
					* sizeof(pthread_mutex_t));
    lock_count = (long *)malloc(CRYPTO_num_locks() * sizeof(long));

    if(lock_cs && lock_count)
    {
      for(int i = 0;i < CRYPTO_num_locks();i++)
      {
	lock_count[i] = 0;
	pthread_mutex_init(&(lock_cs[i]), NULL);
      }
      
      CRYPTO_set_id_callback((unsigned long (*)())pthread_self);
      CRYPTO_set_locking_callback((void (*)(int, int, const char *, int))
				  posix_locking_callback);
      
      r = true;
    }
    else
      wlog->warn("ssl_initialize failed to allocate lock structures");
  }
  else
  {
#if defined(DEBUG)
    dlog->log_progress(DEBUG_ANY, "SSL CTX new failed");
    ERR_print_errors(bio_stderr);  // XXX ?!
#endif
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MAJTRC, "ssl_initialize = %s", IOTF(r));
#endif
  
  return(r);
}

SSL *ssl_session_connect(int sd)
{
  // Create a new SSL session for the already opened socket <sd>.  This
  // function is thread safe only if ssl_initialize was successfully
  // called.

  // Returns: A new SSL struct that should be passed to ssl_session_disconnect
  // when no longer required.

  SSL *r = NULL;
  
#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "ssl_session_connect(%d)", sd);
#endif

  if(sd > -1 && ctx)
  {
    r = SSL_new(ctx);

    if(r)
    {
      BIO *sbio = BIO_new_socket(sd, BIO_NOCLOSE);
      SSL_set_bio(r, sbio, sbio);
      SSL_set_connect_state(r);
    }
  }
  
#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "ssl_session_connect = %d", r);
#endif
  
  return(r);
}

void ssl_session_disconnect(SSL *con)
{
  // Terminate the SSL session described in <con>.  The underlying socket
  // is not closed.  This method is thread safe only if SSL was allocated
  // by ssl_session_connect.

  // Returns: Nothing.

#if defined(DEBUG)
  dlog->log_entry(DEBUG_MINTRC, "ssl_session_disconnect(%d)", con);
#endif
  
  if(con)
  {
    SSL_shutdown(con);
    SSL_free(con);
  }

#if defined(DEBUG)
  dlog->log_exit(DEBUG_MINTRC, "ssl_session_disconnect()");
#endif
}
#endif // WITH_SSL
