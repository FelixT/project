MainComponent
    -> Contains vector of Samples
    -> Contains vector of Modifiers
    
    -> getNextAudioBlock - every audio block
        -> redraw beat count
        
        -> call updateParams() on each modifier 
            -> if slidersChanged [i.e. not by the user], repaint() modifier [async]
                -> if slidersChanged, make them represent these new values i.e. call ->update()
                -> refresh select dropdown items
                -> make select dropdown represent selectedID 
                -> show controls for the selected mode + draw euclidean pattern
            -> update roundedInterval & roundedStep
        -> call updateBuffers() on each sample
            -> if slidersChanged, repaint() sample [async]
                -> if slidersChanged, make them represent these new values i.e. call ->update()
                -> if collapsed hide items, otherwise show em
            -> repaint waveform component
        
        -> for each individual output audio sample
            -> perform modifier actions [tick()]
            -> sample->updateParams, do some lil calculations
            -> sample->getValue
    
    
    ->


slidersChanged is used when the sliders/parameters have been modified by the program rather than the user, hence need to be redrawn with the new values at next repaint (i.e. when loading/with modifiers)


