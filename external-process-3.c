#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080  // Same port number as the central process
#define SERVER_IP "localhost"  // IP address of the central process

// Structure to store temperature data (same as central process)
typedef struct {
  float temperature;
  int externalIndex;  
} TemperatureData;

int main(int argc, char* argv[]) {
  if (argc != 3) {
    printf("Usage: %s <process_id> <initial_temperature>\n", argv[0]);
    exit(1);
  }

  // Get process ID and initial temperature from arguments
  int process_id = 3;  
  float external_temp = atof(argv[2]);

  int client_socket;
  struct sockaddr_in server_address;

  // ---- Create a TCP socket (Fill in the details) ----
  client_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (client_socket == 0) {
    perror("Socket creation failed");
    exit(1);
  }

  // ---- Configure server address (Fill in the details) ----
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(PORT);
  inet_pton(AF_INET, SERVER_IP, &server_address.sin_addr);

  // ---- Connect to the server (Fill in the details) ----
  int connection = connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address));
  if (connection < 0) {
    perror("Connection failed");
    exit(1);
  }

  printf("External process %d connected to server\n", process_id);

  while (1) {
    // Send current temperature and process ID to the server
    TemperatureData data = {external_temp, process_id};  // Include process ID
    send(client_socket, &data, sizeof(data), 0);

    // Receive updated central temperature from the server
    float central_temp;
    recv(client_socket, &central_temp, sizeof(central_temp), 0);

    // Calculate new external temperature based on formula
    external_temp = (3 * external_temp + 2 * central_temp) / 5;

    // Print current state for debugging (optional)
    printf("Process %d: Temp = %.2f, Central Temp = %.2f\n", process_id, external_temp, central_temp);

    // Check for termination message from server
    TemperatureData received_data;
    recv(client_socket, &received_data, sizeof(received_data), 0);
    if (received_data.temperature == -1.0) {
      // Termination flag received
      break;
    }
  }

  // Close the socket
  close(client_socket);

  return 0;
}
