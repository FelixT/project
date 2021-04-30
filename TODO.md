#  TODO

1) Fix euclidean rhythm generation
2) Allow sample labels to be changed
3) work out precision issues: Remove casting to float when using pow10
4) dont redraw beat constantly 
5) update modifier usability 
6) modifier update dropdown fix
7) sample bpm rounding on large samples
8) improve cropping functionality
9) refactor loading state code, classes should have their own load functions
10) option of which beat to start euclidean on
11) equations
12) modifiers of modifiers 
13) Soloing
14) Delete samples & modifiers
15) Don't crash when devices change
16) Make scrollbars keep position when adding/minimising 
17) Disabling of modifiers, disabled by default
18) Remove all the std couts
19) Throw errors/don't allow the user to enter invalid params
20) Modifiers shouldnt modify themselves maybe?
21) Project settings
22) Parameters not saved: Master volume, Sine wave params
23) Change beat euclidean position to drawn text in paint func?
24) On change of euclidean parameters, set preset menu to custom
25) Custom sequencing
26) Check sample exists every time we .at(id) it in modifiers, give error if not
27) Recalculate euclidean if params modified by a modifier
28) Dispose of waveforms when deleting samples
29) Use timer callbacks for some things e.g. updating dropdown, drawing waveform, curpos etc
