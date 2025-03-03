#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <regex.h>
#include <omp.h>

typedef struct {
    char *memory;
    size_t size;
} MemoryBlock;

// callback for libcurl to store fetched data in memory
size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t real_size = size * nmemb;
    MemoryBlock *mem = (MemoryBlock *)userp;

    char *ptr = realloc(mem->memory, mem->size + real_size + 1);

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, real_size);
    mem->size += real_size;
    mem->memory[mem->size] = 0;

    return real_size;
}

// fetch content from a URL
char *fetch_url(const char *url) {
    CURL *curl;
    CURLcode res;
    MemoryBlock chunk = {0};

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            free(chunk.memory);
            chunk.memory = NULL;
        }
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
    return chunk.memory;
}

// trim whitespace from a string
void trim_whitespace(char *str) {
    char *end;
    while (*str == ' ' || *str == '\t' || *str == '\n' || *str == '\r') str++;
    end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) end--;
    *(end + 1) = '\0';
}

// function to parse sitemap and extract URLs
void parse_sitemap(const char *sitemap, FILE *output_file) {
    char *data = fetch_url(sitemap);

    regex_t regex;
    regmatch_t pmatch[2];
    const char *pattern = "<loc>([^<]+)</loc>";

    if (regcomp(&regex, pattern, REG_EXTENDED) != 0) {
        fprintf(stderr, "Error compiling regex\n");
        free(data);
        return;
    }

    char *cursor = data;
    while (regexec(&regex, cursor, 2, pmatch, 0) == 0) {
        size_t match_len = pmatch[1].rm_eo - pmatch[1].rm_so;
        char *url = (char *)malloc(match_len + 1);
        strncpy(url, cursor + pmatch[1].rm_so, match_len);
        url[match_len] = '\0';

        trim_whitespace(url);

        if (strstr(url, ".xml") != NULL) {
            parse_sitemap(url, output_file); // Recursively parse XML sitemaps
        }
        else if (strlen(url) > 0) {
            fprintf(output_file, "%s\n", url);
        }

        free(url);
        cursor += pmatch[0].rm_eo;
    }

    regfree(&regex);
    free(data);
}

// unified function to extract data (emails or phone numbers)
void extract_data(const char *content, FILE *output_file, const char *regex_pattern) {
    regex_t regex;
    regcomp(&regex, regex_pattern, REG_EXTENDED);

    regmatch_t match;
    const char *p = content;
    while (regexec(&regex, p, 1, &match, 0) == 0) {
        char result[256];
        snprintf(result, match.rm_eo - match.rm_so + 1, "%s", p + match.rm_so);
        printf("%s\n", result);
        fprintf(output_file, "%s\n", result);
        p += match.rm_eo;
    }

    regfree(&regex);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <sitemap_url> <-email|-phone>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *sitemap_url = argv[1];
    const char *mode = argv[2];
    const char *regex_pattern;
    const char *output_filename;

    if (strcmp(mode, "-email") == 0) {
        regex_pattern = "[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}";
        output_filename = "emails.txt";
    } else if (strcmp(mode, "-phone") == 0) {
        regex_pattern = "\\(?[0-9]{3}\\)?[-.\\s]?[0-9]{3}[-.\\s]?[0-9]{4}";
        output_filename = "phones.txt";
    } else {
        fprintf(stderr, "Invalid mode. Use -email or -phone.\n");
        return EXIT_FAILURE;
    }

    FILE *output_file = fopen("urls.txt", "w");

    // Parse the sitemap and write URLs to a file
    parse_sitemap(sitemap_url, output_file);
    fclose(output_file);

    // Open the file containing URLs
    FILE *file = fopen("urls.txt", "r");
    FILE *result_file = fopen(output_filename, "w");

    double start = omp_get_wtime();

    char url[1024];
    char **urls = NULL;
    size_t count = 0;

    // Allocate memory for URLs
    while (fgets(url, sizeof(url), file)) {
        trim_whitespace(url);

        urls = realloc(urls, (count + 1) * sizeof(char *));
        urls[count] = strdup(url);
        count++;
    }
    fclose(file);

    // Start processing URLs
    for (size_t i = 0; i < count; i++) {
        char *content = fetch_url(urls[i]);
        if (content) {
            extract_data(content, result_file, regex_pattern);
            free(content);
        }
        free(urls[i]);
    }
    free(urls);

    fclose(result_file);

    double end = omp_get_wtime();
    printf("Elapsed time: %f seconds.\n", end - start);
    
    return 0;
}
