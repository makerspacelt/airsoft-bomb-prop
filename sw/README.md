# Defusal  
  
[x] Implement beeping when bomb is armed. With time reducing the beeping should increase the speed  
[x] After game is ended by defusing the bomb the timer should stop counting  
[x] Implement siren  
[x] Implement progress bar  
[x] Implement settings input validation  
[x] Input should clear when more than 6 numbers are inputted  
[x] When entering code, show `ARM CODE: <digits>`
[x] `BAD CODE` should be centered - also DISARMED like in flow pdf
[x] `ARMED: ` should show the entered code
[x] D to go back from setup
[ ] If time runs out without arming, should it still say EXPLODED?
[ ] Create integration tests

# Domination  
  
[x] Implement siren per spec.  
[x] Implement progress bar  
[x] Implement settings input validation
[x] D to go back from setup

# Zone control  
  
[x] Implement siren per spec.  
[x] Implement progress bar  

# Timer  
  
[x] Implement siren per spec.  
[x] Implement settings input validation  
[x] D to go back from setup

# Mertvek

[ ] TODO
  
# Global  

[x] After the game ended if the C button was pressed for 10s the same game with same parameters should start 
[x] Implement more frequent display update for progress bars  
[x] Different key press tones for C/D buttons (as per spec.)  
[x] At any given point if the D + * are pressed for 10s user should be returned to main menu. UPDATE: The function is implemented on * key only because for some reason the keypad does not support multiple inputs at once
[x] Game delay timer at first render shows 00:00 probably due to invalid sequence of update execution
[x] `TIME LEFT: x:y` - if x or y has more digits, remove space - max entry is 999:60
[ ] OTA
[ ] ESP NOW
[ ] github CI
