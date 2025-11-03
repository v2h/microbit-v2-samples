### Scratchpad

In codal-microbit-v2/source/NRF52LedMatrix.cpp:
display_irq() calls render(). display_irq() is mappped to a timer
-> it runs periodically (?)
-> there is no need to manually create a loop that calls the light check function + fiber_sleep() (?)

Look into codal-core/source/drivers/AnimatedDisplay.cpp for an example on how to emit an Event

Look into OOB_v3.cpp -> playMelody() for an example on how to play music

### Time logging
- Sun 02.11: ~1.5 hours
  -- Set up env, forking repos
  -- Skim through codebase for modules relevant to the assignment
  -- Extend `codal-microbit-v2/source/NRF52LedMatrix.cpp` to be able to emit light state

- Mon 03.11: ~30min
  -- Implement music box

- Mon 03.11: ~1 hour
  -- Read audio code
  -- Draw flow diagram