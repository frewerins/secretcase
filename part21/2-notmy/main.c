#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char*  data;
    size_t length;
} buffer_t;

static size_t
callback_function(char* ptr, size_t chunk_size, size_t nmemb, void* user_data) {
    buffer_t* buffer = user_data;
    size_t total_size = chunk_size * nmemb;

    memcpy(buffer->data, ptr, total_size);
    buffer->length += total_size;

    char* begin = strstr(buffer->data, "<title>");
    if (begin != NULL) {
        begin += 7;
        char* end = strstr(buffer->data, "</title>");

        size_t cur = 0;
        char* title = malloc(end - begin + 1);

        while (begin != end) {
            title[cur++] = *begin;
            ++begin;
        }
        title[cur] = '\0';
        printf("%s\n", title);
    }

    return total_size;
}

int main(int argc, char *argv[]) {
    const char* url = argv[1];

    CURL *curl = curl_easy_init();
    if(curl) {
        CURLcode res;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback_function);

        buffer_t buffer;
        buffer.data = calloc(100*1024*1024, 1);
        buffer.length = 0;
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);

        curl_easy_setopt(curl, CURLOPT_URL, url);
        res = curl_easy_perform(curl);

        free(buffer.data);
        curl_easy_cleanup(curl);
    }
	return 0;
}