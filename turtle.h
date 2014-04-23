/* original: http://abhinavsingh.com/blog/2009/12/how-to-build-a-custom-static-file-serving-http-server-using-libevent-in-c/ */
/* slightly hacked, added php-fpm passthrough with cgi client */
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <getopt.h>
#include <sys/stat.h>
#include <event2/event.h>
#include <event2/http.h>
#include <event2/buffer.h>
#define _SIGNATURE "turtle 1"
#define _HTDOCS "/var/www/static"
#define _INDEX "/index.php"

struct evhttp *libsrvr;
struct event_base *libbase;
struct _options {
    int port;
    char *address;
    int verbose;
} options;