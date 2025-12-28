#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <syslog.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>

#define PORT 9000
#define DATA_FILE "/var/tmp/aesdsocketdata"
#define BUFFER_SIZE 1024

static int sockfd = -1;
static int clientfd = -1;
static volatile sig_atomic_t caught_signal = 0;

void signal_handler(int signo) {
    syslog(LOG_INFO, "Caught signal, exiting");
    caught_signal = 1;
    
    // Close sockets
    if (clientfd >= 0) {
        close(clientfd);
        clientfd = -1;
    }
    if (sockfd >= 0) {
        close(sockfd);
        sockfd = -1;
    }
    
    // Remove data file
    unlink(DATA_FILE);
}

int setup_signals() {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = signal_handler;
    
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        syslog(LOG_ERR, "Failed to setup SIGINT handler: %s", strerror(errno));
        return -1;
    }
    
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        syslog(LOG_ERR, "Failed to setup SIGTERM handler: %s", strerror(errno));
        return -1;
    }
    
    return 0;
}

int become_daemon() {
    pid_t pid = fork();
    
    if (pid < 0) {
        syslog(LOG_ERR, "Fork failed: %s", strerror(errno));
        return -1;
    }
    
    if (pid > 0) {
        // Parent process exits
        exit(0);
    }
    
    // Child process continues as daemon
    if (setsid() < 0) {
        syslog(LOG_ERR, "setsid failed: %s", strerror(errno));
        return -1;
    }
    
    // Change working directory to root
    if (chdir("/") < 0) {
        syslog(LOG_ERR, "chdir failed: %s", strerror(errno));
        return -1;
    }
    
    // Close standard file descriptors
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    
    return 0;
}

int send_file_content(int client_sock) {
    FILE *fp = fopen(DATA_FILE, "r");
    if (fp == NULL) {
        syslog(LOG_ERR, "Failed to open file for reading: %s", strerror(errno));
        return -1;
    }
    
    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
        ssize_t bytes_sent = send(client_sock, buffer, bytes_read, 0);
        if (bytes_sent < 0) {
            syslog(LOG_ERR, "Failed to send data: %s", strerror(errno));
            fclose(fp);
            return -1;
        }
    }
    
    fclose(fp);
    return 0;
}

int handle_client(int client_sock, struct sockaddr_in *client_addr) {
    char ip_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(client_addr->sin_addr), ip_str, INET_ADDRSTRLEN);
    
    syslog(LOG_INFO, "Accepted connection from %s", ip_str);
    
    // Open file for appending
    FILE *fp = fopen(DATA_FILE, "a");
    if (fp == NULL) {
        syslog(LOG_ERR, "Failed to open file for writing: %s", strerror(errno));
        return -1;
    }
    
    char *line_buffer = NULL;
    size_t line_buffer_size = 0;
    size_t line_length = 0;
    char recv_buffer[BUFFER_SIZE];
    
    while (!caught_signal) {
        ssize_t bytes_received = recv(client_sock, recv_buffer, sizeof(recv_buffer), 0);
        
        if (bytes_received < 0) {
            syslog(LOG_ERR, "Failed to receive data: %s", strerror(errno));
            break;
        }
        
        if (bytes_received == 0) {
            // Client closed connection
            break;
        }
        
        // Process received data
        for (ssize_t i = 0; i < bytes_received; i++) {
            // Expand buffer if needed
            if (line_length >= line_buffer_size) {
                size_t new_size = line_buffer_size + BUFFER_SIZE;
                char *new_buffer = realloc(line_buffer, new_size);
                if (new_buffer == NULL) {
                    syslog(LOG_ERR, "Failed to allocate memory: %s", strerror(errno));
                    free(line_buffer);
                    fclose(fp);
                    return -1;
                }
                line_buffer = new_buffer;
                line_buffer_size = new_size;
            }
            
            line_buffer[line_length++] = recv_buffer[i];
            
            // Check for newline
            if (recv_buffer[i] == '\n') {
                // Write to file
                if (fwrite(line_buffer, 1, line_length, fp) != line_length) {
                    syslog(LOG_ERR, "Failed to write to file: %s", strerror(errno));
                    free(line_buffer);
                    fclose(fp);
                    return -1;
                }
                fflush(fp);
                
                // Send file content back to client
                if (send_file_content(client_sock) < 0) {
                    free(line_buffer);
                    fclose(fp);
                    return -1;
                }
                
                // Reset line buffer
                line_length = 0;
            }
        }
    }
    
    free(line_buffer);
    fclose(fp);
    
    syslog(LOG_INFO, "Closed connection from %s", ip_str);
    return 0;
}

int main(int argc, char *argv[]) {
    bool daemon_mode = false;
    
    // Parse command line arguments
    if (argc > 1 && strcmp(argv[1], "-d") == 0) {
        daemon_mode = true;
    }
    
    openlog("aesdsocket", LOG_PID | LOG_CONS, LOG_USER);
    
    // Setup signal handlers
    if (setup_signals() < 0) {
        closelog();
        return -1;
    }
    
    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        syslog(LOG_ERR, "Failed to create socket: %s", strerror(errno));
        closelog();
        return -1;
    }
    
    // Set socket options to reuse address
    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        syslog(LOG_ERR, "Failed to set socket options: %s", strerror(errno));
        close(sockfd);
        closelog();
        return -1;
    }
    
    // Bind socket
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        syslog(LOG_ERR, "Failed to bind socket: %s", strerror(errno));
        close(sockfd);
        closelog();
        return -1;
    }
    
    // Become daemon if requested
    if (daemon_mode) {
        if (become_daemon() < 0) {
            close(sockfd);
            closelog();
            return -1;
        }
    }
    
    // Listen for connections
    if (listen(sockfd, 5) < 0) {
        syslog(LOG_ERR, "Failed to listen on socket: %s", strerror(errno));
        close(sockfd);
        unlink(DATA_FILE);
        closelog();
        return -1;
    }
    
    // Accept connections in a loop
    while (!caught_signal) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        
        clientfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_addr_len);
        if (clientfd < 0) {
            if (caught_signal) {
                break;
            }
            syslog(LOG_ERR, "Failed to accept connection: %s", strerror(errno));
            continue;
        }
        
        handle_client(clientfd, &client_addr);
        
        close(clientfd);
        clientfd = -1;
    }
    
    // Cleanup
    if (sockfd >= 0) {
        close(sockfd);
    }
    unlink(DATA_FILE);
    closelog();
    
    return 0;
}
