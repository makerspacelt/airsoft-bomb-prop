# Defusal  
  
[ ] Implement beeping when bomb is armed. With time reducing the beeping should increase the speed  
[ ] After game is ended by defusing the bomb the timer should stop counting  
[ ] Implement siren  
[ ] Implement progress bar  
[ ] Implement settings input validation  
[ ] Input should clear when more than 6 numbers are inputted  
[ ] When entering code, show `ARM CODE: <digits>`
[ ] `BAD CODE` should be centered - also DISARMED like in flow pdf
[ ] `ARMED: ` should show the entered code
  
# Domination  
  
[ ] Implement siren per spec.  
[ ] Implement progress bar  
[ ] Implement settings input validation

# Zone control  
  
[ ] Implement siren per spec.  
[ ] Implement progress bar  
[ ] Implement settings input validation  

# Timer  
  
[ ] Implement siren per spec.  
[ ] Implement progress bar  
[ ] Implement settings input validation  
  
# Global  

[ ] After the game ended if the C button was pressed for 10s the same game with same parameters should start 
[ ] Implement more frequent display update for progress bars  
[ ] Different key press tones for C/D buttons (as per spec.)  
[ ] At any given point if the D + * are pressed for 10s user should be returned to main menu. UPDATE: The function is implemented on * key only because for some reason the keypad does not support multiple inputs at once
[ ] Game delay timer at first render shows 00:00 probably due to invalid sequence of update execution
[ ] `TIME LEFT: x:y` - if x or y has more digits, remove space - max entry is 999:60
