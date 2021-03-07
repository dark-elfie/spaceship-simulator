# Spaceship simulator

This project started as academic assignment (computer graphics course), after which I decided to continue working on it on my own.

----------------------------------

#### Controls

​	**W** - move forward

​	**A** - move left

​	**S** - move backward

​	**D** - move right

​	**Z** - change camera angle left (z axis)

​	**X** - change camera angle right (z axis)

Camera movement is controlled by mouse.

Other keys:

​	*when fuel or HP is equal 0*:

​	**Q** - quit

​	**R** - reset simulation

​	*when player is close to space station*:

​	**E** - refuel

---------

#### Features

- Phong lighting model for 2 light sources (different light colors, light intensity depends on distance from source),
- using objects saved as obj files,
- textures,
- normal mapping,
- spaceship movement (controls are moving the camera and player's spaceship is attached to it, camera movement based on quaternions),
- planets animations,
- fuel - player's spaceship is consuming small amount of fuel every second and with every move; player can refuel at space station (**E**); when fuel goes down to 0, spaceship's movement is blocked and player can restart simulation (**R**) or quit (**Q**),
- HP - can't be restored, small amount (1) of HP is subtracted every second if player is too close to one of stars (if they get very close, they lose 5 HP every second),
- skybox (large sphere attached to camera with space texture),
- window resize (when you do it, objects doesn't distort),
- basic HUD - HP, fuel, special messages,
- MXAA x8 (antialiasing)

----------------

#### Work in progress

- Atmosphere shader
- particle effect