#include "turtle.h"

void router(struct evhttp_request *r, void *arg) {
    const char *uri = evhttp_request_get_uri(r);

    char *static_file = (char *) malloc(strlen(_HTDOCS) + strlen(uri) + strlen(_INDEX) + 1);
    stpcpy(stpcpy(static_file, _HTDOCS), uri);

    bool file_exists = true;
    struct stat st;
    if(stat(static_file, &st) == -1) {
        file_exists = false;
        evhttp_send_error(r, HTTP_NOTFOUND, "NOT FOUND");
    }
    else {
        if(S_ISDIR(st.st_mode)) {
            strcat(static_file, _INDEX);

            if(stat(static_file, &st) == -1) {
                file_exists = false;
                evhttp_send_error(r, HTTP_NOTFOUND, "NOT FOUND");
            }
        }
    }

    if(file_exists) {
        int file_size = st.st_size;
        struct evbuffer *buffer;
        buffer = evbuffer_new();

        struct evkeyvalq *headers = evhttp_request_get_output_headers(r);
        evhttp_add_header(headers, "Server", _SIGNATURE);

        FILE *fp;
        char *php_html = (char*) malloc(2048);
        char *s = (char*) malloc(2048);

        strcpy(php_html, "");

        setenv("GATEWAY_INTERFACE", "CGI/1.1", 1);
        setenv("SCRIPT_FILENAME", static_file, 1);
        setenv("REQUEST_METHOD", "GET", 1);
        setenv("REDIRECT_STATUS", "true", 1);
        setenv("SERVER_PROTOCOL", "HTTP/1.1", 1);
        setenv("REMOTE_HOST", "127.0.0.1", 1);
        fp = popen("/usr/bin/cgi-fcgi -bind -connect 127.0.0.1:9000", "r");


        if (fp != NULL) {
            while (fgets(s, 2048, fp) != NULL) {
                realloc(php_html, sizeof(char) * (strlen(php_html) + strlen(s) + 1));
                strncat(php_html, s, 2048);
            }
            pclose(fp);
        } else {
            strcpy(php_html, "Server busy");
        }
        evbuffer_add_printf(buffer, "%s", php_html);
        evhttp_send_reply(r, HTTP_OK, "OK", buffer);
        evbuffer_free(buffer);
        if(options.verbose) fprintf(stderr, "%s\t%d\n", static_file, file_size);


        if (php_html!=NULL)
            free(php_html);

        if (s!=NULL)
            free(s);

        s = NULL; php_html = NULL;
    }
    else {
        if(options.verbose) fprintf(stderr, "%s\t%s\n", static_file, "404 Not Found");
    }

    if (static_file!=NULL)
        free(static_file);

    static_file = NULL;
}

int main(int argc, char **argv) {
    int opt;

    options.port = 8080;
    options.address = (char*)"0.0.0.0";
    options.verbose = 0;

    while((opt = getopt(argc,argv,"p:vh")) != -1) {
        switch(opt) {
            case 'p':
                options.port = atoi(optarg);
                break;
            case 'v':
                options.verbose = 1;
                break;
            case 'h':
                printf("Usage: ./turtle -p port -v[erbose] -h[elp]\n");
                exit(1);
        }
    }

    libbase = event_base_new();
    libsrvr = evhttp_new(libbase);
    evhttp_bind_socket(libsrvr, options.address, options.port);
    evhttp_set_gencb(libsrvr, router, NULL);
    event_base_dispatch(libbase);

    return 0;
}
