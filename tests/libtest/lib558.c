/*****************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * $Id$
 */

#include "test.h"

#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#define ENABLE_CURLX_PRINTF
#include "curlx.h"

#include "hash.h"
#include "hostip.h"

#include "memory.h"
#include "memdebug.h"


static Curl_addrinfo *fake_ai(void)
{
  Curl_addrinfo *ai;
  int ss_size;

  ss_size = sizeof (struct sockaddr_in);

  if((ai = calloc(1, sizeof(Curl_addrinfo))) == NULL)
    return NULL;

  if((ai->ai_canonname = strdup("dummy")) == NULL) {
    free(ai);
    return NULL;
  }

  if((ai->ai_addr = calloc(1, ss_size)) == NULL) {
    free(ai->ai_canonname);
    free(ai);
    return NULL;
  }

  ai->ai_family = AF_INET;
  ai->ai_addrlen = ss_size;

#if defined(ENABLE_IPV6) && defined(CURLDEBUG)
  /* For tracing purposes log a fake call to getaddrinfo */
  if(logfile)
    fprintf(logfile, "ADDR %s:%d getaddrinfo() = %p\n",
            __FILE__, __LINE__, (void *)ai);
#endif

  return ai;
}


int test(char *URL)
{
  CURL *easyh;
  struct curl_hash *hp;
  char *data_key;
  struct Curl_dns_entry *data_node;
  struct Curl_dns_entry *nodep;
  size_t key_len;
 
  (void)URL; /* not used */

  easyh = curl_easy_init();
  if(!easyh) {
    printf("easy handle init failed\n");
    return TEST_ERR_MAJOR_BAD;
  }
  printf("easy handle init OK\n");

  printf("creating hash...\n");
  hp = Curl_mk_dnscache();
  if(!hp) {
    printf("hash creation failed\n");
    return TEST_ERR_MAJOR_BAD;
  }
  printf("hash creation OK\n");

  /**/

  data_key = aprintf("%s:%d", "dummy", 0);
  if(!data_key) {
    printf("data key creation failed\n");
    return TEST_ERR_MAJOR_BAD;
  }
  key_len = strlen(data_key);

  data_node = calloc(1, sizeof(struct Curl_dns_entry));
  if(!data_node) {
    printf("data node creation failed\n");
    return TEST_ERR_MAJOR_BAD;
  }

  data_node->addr = fake_ai();
  if(!data_node->addr) {
    printf("actual data creation failed\n");
    return TEST_ERR_MAJOR_BAD;
  }

  nodep = Curl_hash_add(hp, data_key, key_len+1, (void *)data_node);
  if(!nodep) {
    printf("insertion into hash failed\n");
    return TEST_ERR_MAJOR_BAD;
  }

  free(data_key);

  /**/

  printf("destroying hash...\n");
  Curl_hash_destroy(hp);
  printf("hash destruction OK\n");

  printf("destroying easy handle...\n");
  curl_easy_cleanup(easyh);
  printf("easy handle destruction OK\n");

  curl_global_cleanup();

  return 0; /* OK */
}
