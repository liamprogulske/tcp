#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080  // Define a port number for the server
#define MAX_ITERATIONS 100  // Maximum iterations for stabilization check

// Structure to store temperature data (same as client and server)
typedef struct {
  float temperature;
  int externalIndex;  // Added to include process ID
} TemperatureData;

int main(int argc, char* argv[]) {
  if (argc != 2) {
    printf("Usage: %s <initial_temperature>\n", argv[0]);
    exit(1);
  }

  // Convert command-line argument to float
  float central_temp = atof(argv[1]);

  // ---- Socket Creation (refer to tcpserver.c for details) ----
  int server_socket;
  struct sockaddr_in address;

  // ... (socket creation, binding to address using code from tcpserver.c)

  // ---- Listen for incoming connections (same as tcpserver.c) ----
  listen(server_socket, 5);  // Listen for a maximum of 5 connections

  printf("Central process started on port %d\n", PORT);

  int valread;
  struct sockaddr_storage client_addr;
  socklen_t addr_size = sizeof(client_addr);
  char buffer[1024] = {0};

  // Array to store temperatures from external processes
  TemperatureData external_temps[4];

  int iteration = 0;  // Iteration counter for stability check

  while (1) {
    // ---- Accept incoming connection (same as tcpserver.c) ----
    int new_socket = accept(server_socket, (struct sockaddr*)&client_addr, &addr_size);
    if (new_socket < 0) {
      perror("accept failed");
      continue;  // Continue to the next iteration on accept error
    }

    // Receive temperature data from all external processes
    for (int i = 0; i < 4; i++) {
      // ---- Receive data from client (refer to tpcClient.c for details) ----
      valread = recv(new_socket, buffer, sizeof(buffer), 0);
      if (valread == 0) {
        // Handle connection closed by client
        close(new_socket);
        break;
      } else if (valread < 0) {
        perror("recv failed");
        break;  // Exit the inner loop on recv error
      }

      // ---- Parse received data and update external_temps ----
      // Assuming the client sends a TemperatureData structure:
      memcpy(&external_temps[i], buffer, sizeof(TemperatureData));  // Copy data
    }

    // Calculate new central temperature based on external temperatures
    float sum_external_temps = 0.0f;
    for (int i = 0; i < 4; i++) {
      sum_external_temps += external_temps[i].temperature;
    }
    central_temp = (2.0f * central_temp + sum_external_temps) / 6.0f;

    // Send updated central temperature to all external processes
    for (int i = 0; i < 4; i++) {
      int sent_bytes = send(new_socket, &central_temp, sizeof(central_temp), 0);
      if (sent_bytes < 0) {
        perror("send failed");
        break;  // Exit the inner loop on send error
      }
    }

    // Check for stabilization condition (basic example)
    iteration++;
    if (iteration >= MAX_ITERATIONS) {
      // System is considered stable after MAX_ITERATIONS
      break;
    }

    // Close the new socket after processing data from this connection
    close(new_socket);
  }

  // Send termination message to all external processes
  // (Replace with your implementation based on tcpserver.c)
  TemperatureData termination_message;
  termination_message.temperature = -1.0;  // Set a flag value to indicate termination
  termination_message.externalIndex = 0;  // Index not used here, but can be filled if
