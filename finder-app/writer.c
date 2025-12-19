#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

#define OPEN_SYSLOG(PID, FACILITY) openlog("writer", PID, FACILITY)
#define CLOSE_SYSLOG() closelog()

#define SYSLOGE(fmt, ...) syslog(LOG_ERR, fmt, ##__VA_ARGS__)
#define SYSLOGD(fmt, ...) syslog(LOG_DEBUG, fmt, ##__VA_ARGS__)
#define SYSLOGI(fmt, ...) syslog(LOG_INFO, fmt, ##__VA_ARGS__)
#define SYSLOGW(fmt, ...) syslog(LOG_WARNING, fmt, ##__VA_ARGS__)
#define SYSLOGV(fmt, ...) syslog(LOG_DEBUG, fmt, ##__VA_ARGS__)

#define CLOG(fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__)

#define MAX_ARG_NUM (2)

#define ARG1_FILE (1)
#define ARG2_STR (2)

int writer(const char* writeFile, const char* writeStr);
int validate_args(int argc, char* argv[]);

int main(int argc, char* argv[]) {
    // Open syslog with LOG_USER facility
    OPEN_SYSLOG(LOG_PID | LOG_CONS | LOG_PERROR, LOG_USER);

    // Validate arguments
    if (validate_args(argc, argv) != 0) {
        CLOSE_SYSLOG();
        return EXIT_FAILURE;
    }

    const char* writefile = argv[ARG1_FILE];
    const char* writestr = argv[ARG2_STR];

    // Perform the write operation
    if (writer(writefile, writestr) != 0) {
        CLOSE_SYSLOG();
        return EXIT_FAILURE;
    }

    // Close syslog
    CLOSE_SYSLOG();

    return EXIT_SUCCESS;
}

int validate_args(int argc, char* argv[]) {
    if (argc != (MAX_ARG_NUM + 1)) {
        SYSLOGE("Invalid number of arguments: %d (expected %d)", argc - 1, MAX_ARG_NUM);
        CLOG("Error: Must have exactly %d parameters\n", MAX_ARG_NUM);
        CLOG("Usage: %s <file> <string>\n", argv[0]);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int writer(const char* writeFile, const char* writeStr) {
    SYSLOGD("Writing %s to %s", writeStr, writeFile);

    FILE* file = fopen(writeFile, "w");
    if (file == NULL) {
        SYSLOGE("Failed to open file %s: %s", writeFile, strerror(errno));
        CLOG("Error: Could not open file %s: %s\n", writeFile, strerror(errno));
        return EXIT_FAILURE;
    }

    if (fprintf(file, "%s", writeStr) < 0) {
        SYSLOGE("Failed to write to file %s: %s", writeFile, strerror(errno));
        CLOG("Error: Could not write to file %s: %s\n", writeFile, strerror(errno));
        fclose(file);
        return EXIT_FAILURE;
    }

    if (fclose(file) != 0) {
        SYSLOGE("Failed to close file %s: %s", writeFile, strerror(errno));
        CLOG("Error: Could not close file %s: %s\n", writeFile, strerror(errno));
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
