# ASSIGNMENT 1 - COMPUTER GRAPHICS

## Building

1. `cd` into *game* folder
2. Run `make` to compile
3. Run `sample2D`

## Controls

### Turret/Cannon

- **W**, **S** to move vertically. **SPACE** to shoot. To use keyboard controls, the mouse pointer must be outside the frame.
- **MOUSE-LEFT** to click and drag vertically. **LEFT-CLICK** anywhere except on turret or buckets, and INSIDE the frame to shoot.

### Buckets

- **CTRL + LEFT** or **CTRL + RIGHT** to move RED bucket.
- **ALT + LEFT** or **ALT + RIGHT** to move GREEN bucket.
- Holding **MOUSE-LEFT** and dragging will also work on the buckets.

### General

- **P** to pause and unpause game
- **N**, **M** to increase and decrease speed of falling bricks
- **UP**, **DOWN** to increase and decrease ZOOM
- **LEFT**, **RIGHT** to PAN. To pan with the mouse use **MOUSE-RIGHT** to click and drag sideways.

## Rules

- Shoot black bricks.
- Collect red bricks in red bucket.
- Collect green bricks in green bucket.
- Don't let the bricks fall past the screen.
- Failing at any of the above results in losing a life. You begin with 9 lives.

## Notes

- Keyboard controls for the turret only work while the mouse pointer is OUTSIDE the game frame/window.
- PAN will only work when `ZOOM > x1`.

## Bonus

- Pause button
- Automatically centered panning and zooming. Basically, zooming in, panning hard left or right, and then zooming out will not cause you to lose sight of the game.