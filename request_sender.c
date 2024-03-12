#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <curl/curl.h>

#define NUM_THREADS 5  // Change the number of threads as needed
#define URL "https://example.com"  // Change the URL to the desired endpoint

// Array of user agents
const char* user_agents[] = {
    "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/97.0.4692.71 Safari/537.36",
    "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/15.0 Safari/605.1.15",
    "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Edge/88.0.705.74 Safari/537.36",
    "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:97.0) Gecko/20100101 Firefox/97.0",
    "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:97.0) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/97.0.4692.99 Safari/537.36"
};

// Function to send request with libcurl
void *send_request(void *thread_id) {
    long tid = (long)thread_id;
    CURL *curl = curl_easy_init();
    if (curl) {
        CURLcode res;
        struct curl_slist *headers = NULL;
        
        // Set user-agent
        curl_easy_setopt(curl, CURLOPT_URL, URL);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, user_agents[tid % 5]); // Choose user-agent based on thread ID
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);  // Disable verbose output
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);  // Follow redirects
        
        // Set headers
        headers = curl_slist_append(headers, "Connection: keep-alive");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        
        // Perform the request
        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        
        // Clean up
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }
    pthread_exit(NULL);
}

int main() {
    pthread_t threads[NUM_THREADS];
    int rc;
    long t;

    // Initialize libcurl
    curl_global_init(CURL_GLOBAL_ALL);

    for (t = 0; t < NUM_THREADS; t++) {
        rc = pthread_create(&threads[t], NULL, send_request, (void *)t);
        if (rc) {
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }

    // Wait for all threads to finish
    for (t = 0; t < NUM_THREADS; t++) {
        pthread_join(threads[t], NULL);
    }

    // Cleanup libcurl
    curl_global_cleanup();

    return 0;
}
