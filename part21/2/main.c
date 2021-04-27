#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <curl/curl.h>

typedef struct {
    char *data;
    size_t length;
    size_t capacity;
} buffer_t;

static size_t callback_function(char *ptr, size_t chunk_size, size_t n, void *user_data) {
    buffer_t *buffer = user_data;
    size_t total_size = chunk_size * n;
    size_t required_capacity = buffer->length + total_size;
    if (required_capacity > buffer->capacity) {
        required_capacity *= 2;
        buffer->data = realloc(buffer->data, required_capacity);
        buffer->capacity = required_capacity;
    }
    memcpy(buffer->data + buffer->length, ptr, total_size);
    buffer->length += total_size;
    return total_size;
}

int main(int argc, char* argv[]) {
    const char* url = argv[1];
    CURL *curl = curl_easy_init();
    CURLcode res;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback_function);

    buffer_t buffer = {.data = NULL, .length = 0, .capacity = 0};

    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);

    curl_easy_setopt(curl, CURLOPT_URL, url);
    res = curl_easy_perform(curl);
    write(STDOUT_FILENO, buffer.data, buffer.length);

    free(buffer.data);
    curl_easy_cleanup(curl);
    return 0;
}
