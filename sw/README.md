This is the source code of an Airsoft prop [KMS-ANT](http://armory.makerspace.lt) designed and sold by [Kaunas Makerspace](https://makerspace.lt).

The code is licensed under GPL-3. If you modify this code and distribute it, then you are required to publish your changes under GPL-3 license as well.

# Building

```sh
# Build and flash esphome firmware, then open log monitoring
# (requires docker or podman for esphome)
$ make flash DEVICE=/dev/ttyACM0

# Build esphome firmware
# (requires docker or podman for esphome)
$ make fw

# Compile the pc build and run integration tests
# (requires the g++ compiler)
$ make test

# Compile the pc build and run it in interactive mode
# (requires the g++ compiler)
$ make interact

# Format project's C++ code using clang-format
# (requires the clang-format utility)
$ make format
```

# Code Structure

```
src-common/                → Shared code used across all platforms (ESP32 & PC)
  ├── utilities.hpp/cpp    → Helper functions (e.g., formatting time, progress bars)
  ├── globals.hpp          → Constants, key definitions, and global state (AntGlobals)
  ├── gm_*.hpp             → Game mode classes (each game is its own class)
  ├── gm_manager.hpp       → Main controller: manages game modes & user input.

src-pc/                    → PC-only code to simulate the game (for development/debugging)
  ├── main.cpp             → Entry point: runs interactive mode & test sequences
  └── mock_esphome.hpp     → Simulates ESP32 hardware (LCD, buttons, millis)

src-esphome/               → ESPHome firmware config
```

* All hardware is managed by [ESPHome](https://esphome.io/components/), as defined in `src-esphome/config.yaml`.
* All game logic is implemented in C++ in `src-common/*`.
* The game logic code communicates with ESPHome through globals, as defined in `src-esphome/esphome-entry.hpp`.
* There is a PC build for interactive and automated testing.

Normally in ESPHome you can call custom C++ code from lambdas defined in
`config.yaml` or from included C++ files. But calling ESPHome functions from
your custom C++ code is unsupported. So we try to keep this interaction to a
minimum. When we need to activate some hardware, we set an action state bitmask
(as defined in `src-common/globals.hpp`), then in ESPHome code we checked,
executed and reset the action state.

Game modes are implemented as classes with the following interface:
```cpp
class GameModeX {
public:
  // Constructor expects `AntGlobals antg` global state.
  AntGlobals &antg;
  GameModeX(AntGlobals &antg) : antg(antg) {}

  // Reset state and prepare for a new game.
  void init();

  // Render current game state to LCD.
  void display_update(esphome::lcd_base::LCDDisplay &disp);

  // Process user input and update game logic.
  void handle_key(unsigned char key);

  // Update time-based logic (timers, progress, events).
  void clock(uint32_t now, uint32_t delta);
};
```

# Tests

There is a custom test runner `src-pc/tests/test.sh` which executes all tests
defined in `src-pc/tests/test_*`. It captures the simulated LCD output, sends
key events and checks that the LCD output is as expected (as pre-recorded). Run
it using `make test`:

```sh
# Run all tests
$ make test

# Run only the test matching the "countdown" pcre regex pattern
$ make test TEST_PATTERN=countdown

# Record new state for the "test_countdown_game" test:
$ make test TEST_SAVE=1 TEST_PATTERN='^test_countdown_game$'

# Record new state for all tests (careful):
$ make test TEST_SAVE=1
```

The tests themselves are formatted like this: the first line is the event
sequence to be executed, separated by commas. All the other lines are expected
LCD display output. Allowed events are:
* `DELAY=n` - delay `n` number of ms
* All key chars defined with the `KEY_` prefix in `src-common/globals.hpp`
* `RED`, `RED_RELEASE`, `YELLOW`, `YELLOW_RELEASE` - yellow / red button press
  & release (always remember to release the red/yellow button after pressing it
  in the test).
* `RESET`, `C_LONG` - special key sequences.

# Engineering mode

There is a special test mode to test all keypad keys & red/yellow buttons. You
can access it by entering the Settings menu and pressing `5` on the keypad. It
will exit automatically after a few seconds of inactivity.

# Siren Levels

* LOW - 87 dB
* MEDIUM - 101 dB
* HIGH - 114 dB
