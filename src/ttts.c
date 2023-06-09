// NOTE: must use option -pthread when compiling!
#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#define QUEUE_SIZE 8
volatile int active = 1;

// Size of buffers to store the message, host, and port respectively.
#define BUFSIZE 256
#define HOSTSIZE 100
#define PORTSIZE 10

void handler() { active = 0; }
// set up signal handlers for primary thread
// return a mask blocking those signals for worker threads
// FIXME should check whether any of these actually succeeded
void install_handlers(sigset_t *mask) {
  struct sigaction act;
  act.sa_handler = handler;
  act.sa_flags = 0;
  sigemptyset(&act.sa_mask);
  sigaction(SIGINT, &act, NULL);
  sigaction(SIGTERM, &act, NULL);
  sigemptyset(mask);
  sigaddset(mask, SIGINT);
  sigaddset(mask, SIGTERM);
}

int open_listener(char *service, int queue_size) {
  struct addrinfo hint, *info_list, *info;
  int error, sock;
  // initialize hints
  memset(&hint, 0, sizeof(struct addrinfo));
  hint.ai_family = AF_UNSPEC;
  hint.ai_socktype = SOCK_STREAM;
  hint.ai_flags = AI_PASSIVE;
  // obtain information for listening socket
  error = getaddrinfo(NULL, service, &hint, &info_list);
  if (error) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(error));
    return -1;
  }
  // attempt to create socket
  for (info = info_list; info != NULL; info = info->ai_next) {
    sock = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
    // if we could not create the socket, try the next method
    if (sock == -1)
      continue;
    // bind socket to requested port
    error = bind(sock, info->ai_addr, info->ai_addrlen);
    if (error) {
      close(sock);
      continue;
    }
    // enable listening for incoming connection requests
    error = listen(sock, queue_size);
    if (error) {
      close(sock);
      continue;
    }
    // if we got this far, we have opened the socket
    break;
  }
  freeaddrinfo(info_list);
  // info will be NULL if no method succeeded
  if (info == NULL) {
    fprintf(stderr, "Could not bind\n");
    return -1;
  }
  return sock;
}

typedef struct Player {
  int socket;
  int gameNumber; // Game the Player is in.
  char piece; // If the Player is in a game, it's piece will be either X or O.
  char name[178];
  struct Player *nextPlayer; // Linked List of Players
} Player;

struct connection_data {
  struct sockaddr_storage addr;
  socklen_t addr_len;
  int fd;
};

/* We used a struct to pass in 2 arguments at once.*/
typedef struct arguments {
  int waitingRoom[2];
  Player *listOfPlayers;
  int isActive;
} Game;

/*Counts the number of bars.*/
int numOfBars(char *buf, int bytes) {
  int numBars = 0;
  for (int i = 0; i < bytes; i++) {
    // printf("%c\n", buf[i]);
    if (buf[i] == '|')
      numBars++;
  }
  return numBars;
}

int valid(char *buf, int bytes, Player *possiblePlayer) {
  if (bytes == 0)
    return 0;
  // const char s[2] = "|";

  int numBars = numOfBars(buf, bytes);

  int numBytes = bytes; // place holder
  // printf("Num Bytes: %d", numBytes);

  char *token;

  token = strtok_r(buf, "|", &buf);

  // printf("Token: %ld\n", strlen(token+1));

  // traverse through the buf and see how many | signs are in the message

  // printf("NUMOFBARS: %d\n", numBars);

  numBytes = numBytes - 5;

  // printf("Num Bytes After: %d\n", numBytes);

  // printf("%c", buf[numBytes-3]);

  if (strcmp(token, "PLAY") == 0) {

    if (numBars == 3 && buf[numBytes - 2] == '|') {

      token = strtok_r(buf, "|", &buf); // get second token

      int x = atoi(token);

      // If x > BUFSIZE
      if (x > BUFSIZE) {
        return -4; // Too long to store name error message
      }

      if (x != 0) {
        // printf("HEllo");
        token = strtok_r(buf, "|", &buf); // get third token

        memcpy(possiblePlayer ->name, token, strlen(token) + 1);

        int length = 0;

        while (*token != '\0') {
          length++;
          token++;
        }

        length++;

        if (x == length) {
          return 1;
          // Valid
        }

        else {
          return -1;
          // Invalid
        }
      }

      else {
        return 0;
        // Invalid
      }

    }

    else {
      return 0;
      // Invalid
    }
  }

  else if (strcmp(token, "MOVE") == 0) {

    if (numBars == 4 && buf[numBytes - 2] == '|') {

      token = strtok_r(buf, "|", &buf); // get second token

      // printf("Token 2\n: %s", buf);

      int x = atoi(token);

      if (x == 6) {

        token = strtok_r(buf, "|", &buf); // get third token

        // printf("Token 3\n: %s", buf);

        // numBytes-=3;

        if (strcmp(token, "X") == 0 ||
            strcmp(token, "O") == 0) { // if third token is X or O

          // numBytes-=2;

          token = strtok_r(buf, "|", &buf); // get fourth token

          // printf("Token 4: %s", token);

          int length = 0;

          char *ptr = token;

          // printf("%s", ptr);

          while (*ptr != '\0') {
            length++;
            ptr++;
          }

          if (length == 3) {
            if (token[1] == ',' &&
                (token[0] == '1' || token[0] == '2' || token[0] == '3') &&
                (token[2] == '1' || token[2] == '2' || token[2] == '3')) {
              // valid
              // printf("Hello");
              return 1;

            }

            else {
              return -10;

              // invalid
            }

          }

          else
            return 0; // Invalid
        }

        else
          return -2; // Invalid
      }

      else
        return -1; // Invalid
    } else
      return 0; // Invalid
  }

  // else if token1 == MOVE
  // if there are 4 parallel bars and ends with parallel bar
  // get second tolen
  // if second token is a size length
  // if second token using atoi == 6
  // get third token
  // if third token == "X" or third token == "O"
  // if length of fourth token is 3 and first char is 1-9, second char is comma
  // and third char is 1-9 valid

  // else
  // invalid
  // else
  // invalid

  // else
  // invalid

  // else
  // invalid

  // else
  // invalid

  else if (strcmp(token, "RSGN") == 0) {
    if (numBars == 2 && buf[numBytes - 2] == '|') {
      token = strtok_r(buf, "|", &buf); // get second token

      int x = atoi(token);

      // printf("Number: %d", x);

      if (x == 0)
        return 1; // Valid
      else
        return -1; // Invalid
    } else
      return 0; // Invalid
  }

  // else if token1 == RSGN
  // if there are 2 parallel bars and ends with parallel bar
  // if second token is 0

  // else
  // invalid
  // else
  // invalid

  else if (strcmp(token, "DRAW") == 0) {
    if (numBars == 3 && buf[numBytes - 2] == '|') {
      token = strtok_r(buf, "|", &buf); // get second token

      int x = atoi(token);

      if (x == 2) {
        token = strtok_r(buf, "|", &buf); // get third token
        // printf("Hello");
        if (strcmp(token, "S") == 0 || strcmp(token, "A") == 0 ||
            strcmp(token, "S") == 0) {

          return 1;
          // valid
        }
      } else
        return 0;
    } else
      return 0;
  }
  // else if token1 == DRAW
  // if there are three parallel bars and ends with parallel bar
  // if second token is 2
  // if third token is S or A or R

  // else
  // invalid

  // else
  // invalid
  // else
  // invalid
  else {
    return 0;
    // invalid
  }
  return 0;
  // else
  // invalid
  //*END OF PSEUDOCODE*
}

void insertPlayer(Player **players, Player *playerToInsert) {
  if (*players == NULL) {
    *players = playerToInsert;
  } else {
    Player *current = *players;
    while (current->nextPlayer != NULL) {
      current = current->nextPlayer;
    }
    current->nextPlayer = playerToInsert;
  }
}

void deletePlayer(Player **players, int socketNum) {
  // Have a prev and cur player pointer(cur is the first player, prev is null)
  Player *prev = NULL;
  Player *current = *players;
  // While current is not out of bounds and it's socket is not the socketNum you
  // are looking for
  while (current != NULL && current->socket != socketNum) {
    // Update the pointers to point to the next node(s).
    prev = prev->nextPlayer;
    current = current->nextPlayer;
  }
  // If prev is NULL(Deleting the first player)
  if (prev == NULL) {
    // Get next player.
    Player *next = current->nextPlayer;
    // Delete/Free current player.
    free(current);
    // Update first player to be next player as the new first player.
    *players = next;
  } else {
    // Else
    // Prev's next is current's next
    prev->nextPlayer = current->nextPlayer;
    // Delete current
    free(current);
  }
}

// Checks to see if the name exists in the linked list of players.
int doesPlayerExist(Player *player, char *playerName) {
  // Have a current ptr to player.
  Player *current = player;
  // While cur doesn't equal to null and it's name doesn't equal to the
  // playerName passed in
  if (current != NULL) fprintf(stdout, "Current playerrrrrrrr: %s\n", current->name);
  fprintf(stdout, "Current playerrrrrrrr: %s\n", playerName);
  while (current != NULL) {
    if (strcmp(current->name, playerName) == 0) return 1;
    current = current->nextPlayer;
  }
  return 0;
}

void beginMessage(Player *playerInGame){
    char buffer[45] = "BEGN|"; // Begin message
    char pieceString[2];
    pieceString[0] = playerInGame -> piece;
    pieceString[1] = '\0';
    strcat(buffer, pieceString);            // Add piece
    strcat(buffer, "|");                 // Add bar
    strcat(buffer, playerInGame->name); // Add name
    strcat(buffer, "|");  // Add bar
    //Send the begin message back.               
    write(playerInGame ->socket, buffer, strlen(buffer) + 1);
}

Player* applyPieceToPlayer(Player *listOfPlayers, int socketNum, char piece) {
  // Current pointer points to the head
  Player *currentPlayer = listOfPlayers;
  // While the current pointer isn't null and not contain the socket you are
  // looking for
  while (currentPlayer != NULL && currentPlayer->socket != socketNum) {
    // Update the pointer to point to the next node.
    currentPlayer = currentPlayer->nextPlayer;
  }
  // Apply the piece to the player.
  currentPlayer->piece = piece;
  return currentPlayer;
}

void *playGame(void *arg) {
  // Cast the argument to be a Game.
  Game *game = (Game *)arg;
  // Get the waiting room
  fprintf(stderr, "Socket 1: %d\n", game ->waitingRoom[0]);
  fprintf(stderr, "Socket 2: %d\n", game ->waitingRoom[1]);
  // Create a method that takes in a linked list of players, socketNum, and X to
  // apply a piece to the player
  //Get both players(The below two methods should return the player.)
  Player *player1 = applyPieceToPlayer(game->listOfPlayers, game->waitingRoom[0], 'X');
  Player *player2 = applyPieceToPlayer(game->listOfPlayers, game->waitingRoom[1], 'O');
  //Create a separate method for begin
  //Then call it on both players.
  beginMessage(player1);
  beginMessage(player2);
  //Start the game(While loop for game)
  while(1){
    
  }
  return NULL;
}

int main(int argc, char **argv) {
  sigset_t mask;
  struct connection_data *con;
  char *service = argc == 2 ? argv[1] : "15000";
  install_handlers(&mask);
  int listener = open_listener(service, QUEUE_SIZE);
  if (listener < 0)
    exit(EXIT_FAILURE);
  printf("Listening for incoming connections on %s\n", service);
  // Initialize the list of Players here
  Player *players = NULL;
  int currentNumOfPlayers = 0; // Current number of players.
  int waitingRoom[2]; // When there are two players ready to play a game.

  // There can only be a maximum of 100 games(No more than that).

  while (currentNumOfPlayers != 2) {
    con = (struct connection_data *)malloc(sizeof(struct connection_data));
    con->addr_len = sizeof(struct sockaddr_storage);
    Player *possiblePlayer = malloc(sizeof(Player));
    char buf[BUFSIZE + 1], host[HOSTSIZE], port[PORTSIZE];
    possiblePlayer -> socket = accept(listener, (struct sockaddr *)&con->addr, &con->addr_len);
    if (possiblePlayer -> socket < 0) {
      strerror(errno);
      free(con);
      free(possiblePlayer);
    } else {
      // Create a possiblePlayer for that socket.
      possiblePlayer->nextPlayer = NULL;
      fprintf(stdout, "%d", possiblePlayer->socket);
      Player* cuy = players;
      while(cuy != NULL){
        fprintf(stdout, "Player name: %s\n", cuy ->name);
        cuy = cuy -> nextPlayer;
      }
      // Set up the data.
      // Memset host and port to be 0.
      memset(host, 0, HOSTSIZE);
      memset(port, 0, PORTSIZE);
      int bytes, error;
      error = getnameinfo((struct sockaddr *)&con->addr, con->addr_len, host,
                          HOSTSIZE, port, PORTSIZE, NI_NUMERICSERV);
      if (error) {
        fprintf(stderr, "getnameinfo: %s\n", gai_strerror(error));
        strcpy(host, "??");
        strcpy(port, "??");
      }
      printf("Connection from %s:%s\n", host, port);

      // Read data
      bytes = read(possiblePlayer->socket, buf, BUFSIZE);
      buf[bytes] = '\0';
      fprintf(stdout, "Number of Bytes Read: %d\n", bytes);

      char buf2[BUFSIZE];
      memset(buf2, 0, BUFSIZE);

      memcpy(buf2, buf, strlen(buf) + 1);

      // Checking for play message to be valid.
      int isValid = valid(buf, bytes, possiblePlayer);
      fprintf(stdout, "%s\n", possiblePlayer -> name);

      // If the message is valid = 1
      if (isValid == 1) {
        // Have a function that goes through the linked list to see if a name
        // exists Inputs: Name of the possible player and the linked list of
        // players
        // If so
        if (doesPlayerExist(players, possiblePlayer->name)) {
          // Invl message
          send(possiblePlayer->socket,
               "INVL|33|Player already exists in the game|",
               strlen("INVL|33|Player already exists in the game|"), 0);
          fprintf(stdout, "Yor:%s\n", players->name);
          // Disconnect the possible player
          close(possiblePlayer->socket);
          free(possiblePlayer);
        } else { // Else
          // Send WAIT|0|
          send(possiblePlayer->socket, "WAIT|0|", strlen("WAIT|0|"), 0);
          // Add possible player to linked list.
          insertPlayer(&players, possiblePlayer);
          fprintf(stdout, "%s\n", players->name);
          // Add to waiting room
          waitingRoom[currentNumOfPlayers % 2] = possiblePlayer->socket;
          // Increment total numnber of players by 1.
          currentNumOfPlayers++;
        }
      }
      // Else
      else {
        // Store the corresponding invl message to the buffer (Look at the
        // return values) Make sure you include the error message for the return
        // value of -4.
        if (isValid == 0) {
          send(possiblePlayer->socket, "INVL|23|!Formatted Incorrectly|",
               strlen("INVL|23|!Formatted Incorrectly|"), 0);
        } else if (isValid == -10) {
          send(possiblePlayer->socket, "INVL|21|!Move is not allowed|",
               strlen("INVL|21|!Move is not allowed|"), 0);
        } else if (isValid == -1) {
          send(possiblePlayer->socket, "INVL|18|!Length not equal|",
               strlen("INVL|18|!Length not equal|"), 0);
        } else if (isValid == -2) {
          send(possiblePlayer->socket, "INVL|12|!Not X or O|",
               strlen("INVL|12|!Not X or O|"), 0);
        } else if (isValid == -4) {
          send(possiblePlayer->socket, "INVL|18|!Name is too long|",
               strlen("INVL|18|!Name is too long|"), 0);
        }
        // Close the client socket.
        close(con->fd);
        // Delete the client associated with the socket.
        free(possiblePlayer);
      }

      // If there are an even amount of players
      if (currentNumOfPlayers > 0 && currentNumOfPlayers % 2 == 0) {
        // Create an argument struct
        // Takes in the argument waitroom and the player linked list (which is
        // the game struct)
        Game *game = malloc(sizeof(Game));
        game->listOfPlayers = players;
        memcpy(game->waitingRoom, waitingRoom, 2 * sizeof(int));
        // Create a playGame method.
        // Start a game using the pthread create function
        pthread_t thread;
        if (pthread_create(&thread, NULL, &playGame, game) != 0) {
          // Delete the two clients
          deletePlayer(&players, game->waitingRoom[0]);
          deletePlayer(&players, game->waitingRoom[1]);
          // Send invl message to both clients.
          send(game->waitingRoom[0],
               "INVL|46|Could not create a game due to network issue.|",
               strlen("INVL|46|Could not create a game due to network issue.|"),
               0);
          send(game->waitingRoom[1],
               "INVL|46|Could not create a game due to network issue.|",
               strlen("INVL|46|Could not create a game due to network issue.|"),
               0);
          // Close the sockets
          close(game->waitingRoom[0]);
          close(game->waitingRoom[1]);
          // Free the game
          free(game);
          // Reset the waitingRoom
          memset(waitingRoom, 0, 2 * sizeof(int));
          // Continue
          free(con);
          continue;
        }
        // Waits for the game to be over.
        pthread_join(thread, NULL);
        // Delete the two clients
        deletePlayer(&players, game->waitingRoom[0]);
        deletePlayer(&players, game->waitingRoom[1]);
        // Close the sockets
        close(game->waitingRoom[0]);
        close(game->waitingRoom[1]);
        // Free the game
        free(game);
        // Reset the waitingRoom
        memset(waitingRoom, 0, 2 * sizeof(int));
        // Continue
        free(con);
        continue;
      }
      free(con);
   }
  }
    // Free the linked list of players.
    Player *current = players;
    while (current != NULL) {
      Player *next = current->nextPlayer;
      free(current);
      current = next;
    }
  return EXIT_SUCCESS;
}