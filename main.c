#include <curl/curl.h>
#include <stdio.h>
#include <time.h>

#include "json.h"

#define API "https://cms.waktusolat.digital/esolatjson.php?zon=png01"

#define curl_error(c, fmt, ...)                   \
    do {                                          \
        fprintf(stderr, fmt "\n", ##__VA_ARGS__); \
        curl_easy_cleanup(c);                     \
        return EXIT_FAILURE;                      \
    } while (0)

typedef struct string {
    char* str;
    size_t len;
} string;

typedef struct PrayerTime {
    char* date;
    char* imsak;
    char* subuh;
    char* syuruk;
    char* zohor;
    char* asar;
    char* maghrib;
    char* isyak;
    char* direction;
} PrayerTime;

string* string_init() {
    string* ptr = calloc(1, sizeof(string));

    ptr->str = NULL;
    ptr->len = 0;

    return ptr;
}

size_t write_func(void* ptr, size_t size, size_t nmemb, string* s) {
    size_t new_len = s->len + (size * nmemb);

    s->str = realloc(s->str, new_len + 1);

    if (s->str == NULL) {
        fprintf(stderr, "Realloc() failed\n");
        exit(EXIT_FAILURE);
    }

    memcpy(s->str + s->len, ptr, size * nmemb);
    s->len = new_len;
    s->str[new_len] = '\0';

    return size * nmemb;
}

int get_current_day() {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    return tm.tm_mday;
}

int call_api(string* data) {
    CURL* curl = curl_easy_init();

    if (!curl) {
        curl_error(curl, "Cannot init curl");
    }

    curl_easy_setopt(curl, CURLOPT_URL, API);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_func);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, data);

    printf("Fetching...\n");
    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        curl_error(curl, "curl_easy_perform() failed: %s", curl_easy_strerror(res));
    }

    curl_easy_cleanup(curl);
    return 0;
}

PrayerTime* parse_json(string* data, int* _days) {
    json_value* json_result = json_parse(data->str, data->len);

    if (json_result == NULL) {
        fprintf(stderr, "Unable to parse data\n");
        exit(EXIT_FAILURE);
    }

    json_object_entry main_obj = json_result->u.object.values[0];
    short days = main_obj.value->u.array.length;
    *_days = days;

    PrayerTime* pt = malloc(sizeof(*pt) * days);

    for (size_t i = 0; i < days; i++) {
        pt[i] = (PrayerTime){
            .date = main_obj.value->u.array.values[i]->u.object.values[0].value->u.string.ptr,
            .imsak = main_obj.value->u.array.values[i]->u.object.values[1].value->u.string.ptr,
            .subuh = main_obj.value->u.array.values[i]->u.object.values[2].value->u.string.ptr,
            .syuruk = main_obj.value->u.array.values[i]->u.object.values[3].value->u.string.ptr,
            .zohor = main_obj.value->u.array.values[i]->u.object.values[4].value->u.string.ptr,
            .asar = main_obj.value->u.array.values[i]->u.object.values[5].value->u.string.ptr,
            .maghrib = main_obj.value->u.array.values[i]->u.object.values[6].value->u.string.ptr,
            .isyak = main_obj.value->u.array.values[i]->u.object.values[7].value->u.string.ptr,
            .direction = main_obj.value->u.array.values[i]->u.object.values[8].value->u.string.ptr,
        };
    }

    return pt;
}

int main() {
    int result;
    int days;
    string* data = string_init();

    if ((result = call_api(data)) != 0) {
        return result;
    }

    PrayerTime* pt = parse_json(data, &days);

    for (size_t i = 0; i < days; i++) {
        char* end;
        long _day = strtol(pt[i].date, &end, 10);

        printf("Tarikh: %s, Imsak: %s, Subuh: %s, Syuruk: %s, Zohor: %s, Asar: %s, Maghrib: %s, Isyak: %s",
               pt[i].date, pt[i].imsak, pt[i].subuh, pt[i].syuruk, pt[i].zohor, pt[i].asar, pt[i].maghrib, pt[i].isyak);

        if (_day == get_current_day()) {
            printf(" *\n");
        } else {
            printf("\n");
        }
    }

    free(pt);
    return 0;
}
