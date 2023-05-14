#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <json-c/json.h>

struct MemoryStruct {
    char *memory;
    size_t size;
};

static size_t writeMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realSize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    char *ptr = realloc(mem->memory, mem->size + realSize + 1);
    if(ptr == NULL) {
        printf("Not enough memory!");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realSize);
    mem->size += realSize;
    mem->memory[mem->size] = '\0';

    return realSize;
}

int main(void) {
    CURL *curl;
    CURLcode res;

    struct MemoryStruct chunk;
    chunk.memory = malloc(1);
    chunk.size = 0;

    curl_global_init(CURL_GLOBAL_ALL);

    curl = curl_easy_init();
    if(curl) {
        char url[128];
        char city[32];

        printf("Enter city name: ");
        scanf("%s", city);

        snprintf(url, sizeof(url), "http://api.openweathermap.org/data/2.5/weather?q=%s&appid=YOUR_APP_ID", city);

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

        res = curl_easy_perform(curl);

        if(res != CURLE_OK) {
            printf("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        } else {
            struct json_object *jobj = json_tokener_parse(chunk.memory);

            struct json_object *jweather;
            json_object_object_get_ex(jobj, "weather", &jweather);

            struct json_object *jdescription;
            json_object_object_get_ex(json_object_array_get_idx(jweather, 0), "description", &jdescription);

            struct json_object *jmain;
            json_object_object_get_ex(jobj, "main", &jmain);

            struct json_object *jtemp;
            json_object_object_get_ex(jmain, "temp", &jtemp);

            struct json_object *jhumidity;
            json_object_object_get_ex(jmain, "humidity", &jhumidity);

            printf("City: %s\n", city);
            printf("Weather: %s\n", json_object_get_string(jdescription));
            printf("Temperature: %.2f\n", json_object_get_double(jtemp) - 273.15);
            printf("Humidity: %d%%\n", json_object_get_int(jhumidity));
        }

        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();

    free(chunk.memory);

    return 0;
}