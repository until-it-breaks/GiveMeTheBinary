// Shows a welcome page while also setting things up
void initializeGame();

// Sets the difficulty choosen. Will either go to next state or put the device to sleep
void setupDifficulty();

// Sets up a new round
void setupRound();

// Reads user input and checks the win condition
void processGame();

// Successful round scenario
void resolveRound();

// Unsuccessful round scenario
void showGameOver();