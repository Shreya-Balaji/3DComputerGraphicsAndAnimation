# Cable Car Through the Alps


**Cable Car Through the Alps** is a C++/OpenGL project developed by our team: it features an endlessly scrolling cable car ride through a snowy mountain landscape. The viewer can follow the cable car from several camera viewpoints, change the time of day, and adjust the lighting and materials live from a control panel. The scene is enhanced by an animated cable car (opening doors and turning wheels), a rope, falling snow and fog.

This report gives an overview of the implemented features, along with screenshots. Each team member's contribution is summarised, and described alongside the features they implemented.

The project applies the concepts taught in the lectures of 3D Computer Graphics and Animation. Knowledge from additional external sources is listed in the references.

## Team Members
| Name  | Main Contributions |
|-------|--------------------|
| Srinitish Ramesh |Material Textures and Lighting, Cable Car Model Adjustments and Import, Cable path for Cable Car, Mountain Import and Normal Mapping (Initial Version)   |
| Shreya Balaji | Multiple View Points (Orbit view, Top View, Follow View), PBR Shading, Environment Mapping (applied to whole cabin initially, optimized by Henrik later), Day and Night (Light and Sky Settings), Cabin Lights, Cable Car and Rope Shadow   |
| [Name]|[contributions]     |

## Introduction

The demo is a real-time application written in C++ with OpenGL 4.1. It is built with CMake, and also uses ImGui for the control panel (to control all features from the UI) and Assimp to load the cable car model. 

The sections below describe the implemented features. The minimal features are described first, followed by the extra features. Each feature has a short explanation of the idea used to implement it, contributions by the team member, and screenshots. A table of all keys and sliders is given in *ImGui Controls*, so every screenshot can be reproduced exactly.

## Features
### Minimal features

#### Multiple View Points (Orbit View, Follow View, Top View, Bezier View, Free Camera)
The project uses multiple camera modes to follow the path of the cable car and also view the snowy terrain around it. 

##### Orbit View
This view (as depicted in Figure 1) is used to orbit around the cable car. This is controlled by yaw, pitch and distance which can control angle around the vertical axis, angle above the horizontal axis and radius of the orbit. Pitch is limited to -85 degrees to +85 degrees to ensure the view doesn't look down exactly and flip. The minimum distance is also kept at 2 units to ensure that we don't really go into the cable car.

<p align="left">
  <img src="Figures/orbit_view.png" width="48%">
</p>

*Figure 1: Orbit View*

##### Top View
The camera hangs about 12 units above the targeted cable car and looks straight down, which gives a top view. Figure 2 shows an over view of this. Also, GLM's lookAt computes the camera's right axis as the cross product of the looking direction and the up vector (0,1,0). If the camera looks exactly down, these two are parallel making the cross product zero; hence a small offset of 0.01 is added along z, which makes them slightly non-parallel without changing the view entirely.

<p align="left">
  <img src="Figures/top_view.png" width="48%">
</p>

*Figure 2: Top View*

##### Follow View
The camera basically follows the cable car along the rope; it keeps a fixed offset which is 2.5 units up and 8 units away along the z axis. This is depicted in Figure 3 below.

<p align="left">
  <img src="Figures/follow_view.png" width="48%">
</p>

*Figure 3: Follow View*

#### Bezier View
The camera modes also include the Bezier View, which is elaborated under the Bezier Curves section.

#### Free Camera
By toggling Free Camera, you can move around the scene freely using WASD keys for movement. The free camera implementation is mostly copied from the shadow mapping homework assignment, with small changes to adapt it into the project. The previously mentioned camera angles are part of `AttachedCamera`. `CameraRig` handles switching between `AttachedCamera` and `FreeCamera`. To be able to observe the position of `AttachedCamera` from the free camera, we render an icon (`CameraIcon`), which shows the orientation and position of the attached camera (see Figure 4).

<p align="left">
  <img src="Figures/cameraicon.png" width="49%">
</p>

*Figure 4: Camera Icon*

#### PBR Shading
When the PBR Shading is enabled, the body of cable car uses Cook-Torrance microfacet model instead of the Blinn-Phong shader. This is used to compute how much light is reflected off the surface. Two sliders are provided to control metallic and roughness. A dull highlight is given for high roughness; metallic value basically increases the ability's surface to display properties of a metal.

For all light sources (sun, moon and a cabin light in our case), a function is defined to compute the reflected light from the parts of the cable car. The computed value is added to the ambient term.

The Cook-Torrance microfacet model is implemented for this. Some external sources and lecture content were used to gain better understanding of this can be implemented and these are added in the reference.

* **D (GGX Distribution):** It control how large and how sharp the shiny spot is. With low roughness, the cabin gets a small, bright, highlight and with high roughness, the highlight spreads into a dim and dull, matte surface.

* **G (Smith with Schlick-GGX):** It reduces the highlight where the surface is seen at a smaller angle; basically reduces light when viewed from a grazing angle.

* **F (Fresnel Term, Schlick approximation):** This gives surfaces more mirror-like appearance at grazing angles. On the cabin, this gives a brighter appearance along the edges and on surfaces seen from the side.

* **F0 (Base Reflectance):** This is indicative of how reflective the surface is when looked directly at it. As the metallic value increases, surface begins to look more like a metal because of this base reflectance value.

* **Specular Term:** This combines D, G and F (D · G · F / (4 · N·V · N·L)) into the shiny part of the surface, which is what gives the cabin its highlights and reflections. Because it uses all three terms, the highlight combines roughness, viewing angle and metallic value together, making the cabin look more metallic when metallic value is increased.

* **Diffuse Term:** This is the matte colour of the cabin and it is reduced by the Fresnel and the metallic value kD, so as the surface becomes more metallic it loses its matte colour and depends on its reflections, similar to what an actual metal does. It is also divided by π so the surface never reflects more light than it receives (energy conservation, but also makes the brightness realistic).

<p align="left">
  <img src="Figures/PBROff.png" width="48%">
  <img src="Figures/PBROn.png" width="48%">
</p>

*Figure 4.* The cable car with PBR shading off (left, Blinn-Phong) and on (right, metallic 0.25, roughness 0.25). 

<p align="left">
  <img src="Figures/PBRLow.png" width="48%">
  <img src="Figures/PBRHigh.png" width="48%">
</p>

*Figure 5.* The cable car with metallic 0.00 / roughness 0.00 (left) and metallic 0.94 / roughness 0.57 (right). 

As it can be observed in Figure 4., with Blinn-Phong (left), the cabin roof and the red tower look bright and saturated. With PBR (right), the same surfaces look duller and darker. 

In Figure 5, the metallic and roughness values are slided from low to high. At the higher metallic value, the red tower loses its bright matte paint colour and looks darker. Since the diffuse term is lower, a metal shows only its reflections. With a metallic value of 0 and a roughness of 0, the surface keeps its full paint colour.

The two sliders change the look of the cable car in the way the model predicts: metallic controls how much matte colour remains, and roughness controls how wide and strong the highlight is. In these views the highlight is probably not very easy to see, because the cabin does not directly face the sun. Ensure to have fog disabled as well while running the application.

----

#### Material Textures and Lighting
Material Textures and Lighting models are implemented in this demo to showcase how the body of Cable Car interacts with light. The diffuse coefficient **(kd)** and specular coefficient **(ks)** are used to showcase the base color and highlights of the model. **Shininess** is also integrated to determine the level of sharpness of these highlights.

To provide shading based on the angle in which light hits the surface, **Lambertian Diffuse Reflection** is used. To produce more realistic reflective surface **Blinn-Phong Specular reflection** is added. Finally, **Ambient Lighting** provides general illumination, ensuring that the surface is visible in areas that lack direct lighting. To test this, toggle **Enable Ambient Lighting** on and off to observe general illumination. Adjust the **Shininess** slider to see how specular highlights changes.

*Note:* These material, texture, and conventional lighting features are implemented to fulfill the assignment's minimal requirements. However, the team has decided to use **Physically Based Rendering (PBR)** with metallic and roughness, to visually enhance the cable car model.

<p align="left">
  <img src="Figures/WithoutMaterialTex.png" width="48%">
  <img src="Figures/WithMaterialTex.png" width="48%">
</p>
*Figure 6: Conventional Lighting Settings Not Enabled (left) and Conventional Lighting Settings Enabled (right)*

### Terrain and Normal Mapping
The terrain consists of 3 identical tiles in a row, which is being scrolled under the cable car. When the car surpasses 1 tile length from the center, it is moved back to the center (`Terrain::update`). Since the tiles are identical, the reset is seamless.

The terrain tile is generated using the `terrainMesh` function. The terrain tile is a grid with the dimensions `CELLS`x`CELLS`. The `terrainVertex` function translates a point on the grid into a world space position (a mesh vertex). The x and z coordinates are scaled by `TILE_SIZE` and the y coordinate is calculated as follows: 
1. convert the x and z coordinates into angles between 0 and 2π (use modulus so that coordinates match at the edges)
2. subtract 10 to so that the terrain is below the cable car
3. `ridge` ensures that there is a valley through the middle
4. `peaks` creates the mountains along the valley using a combination of sine and cosine functions. `peaks` is a linear blend of `leftPeaks` and `rightPeaks`, using the x coordinate, so that on the left side of the valley `leftPeaks` is more dominant, and on the right it's `rightPeaks`. 
5. add a sine function to vary the valley bed geometry.

The terrain is covered with a snowy texture [[9](https://polyhaven.com/a/snow_02)]. The snow consists of a diffuse texture and a normal map. Figure 7 shows the difference between rendering the terrain with and without the normal map. There is a problem with using just one texture repeated multiple times, that you can easily notice the repetition. To reduce repetition, we used two-scale texture blending [[10](https://www.versluis.com/2023/05/how-to-use-macro-variations-for-unreal-engine-landscape-materials/)]. In `basic.frag`, we define a `broadUV` coordinate, which scales at 25% speed and has an arbitrary offset. If two-scale texture blending is enabled, we create a mix, with 70% of the original sample and 30% of the broad sample. The same is done with the tangent normal. The difference blending makes can be seen in Figure 8 . The downside of blending is that the texture loses some of its crispness and becomes more smooth.  

<p align="left">
  <img src="Figures/normal_map_off.png" width="48%">
  <img src="Figures/normal_map_on.png" width="48%">
</p>

*Figure 7: Normal Mapping disbled (left) and enabled (right)*

<p align="left">
  <img src="Figures/blendingdisabled.png" width="48%">
  <img src="Figures/blendingenabled.png" width="48%">
</p>

*Figure 8: Two-scale texture blending disabled (left) and enabled (right)*

### Environment Mapping
Environment mapping lets the cabin windows reflect their surroundings, a snowy terrain in our case. 
The basic idea is that the cabin takes a picture of sorts in all direction and each window shows a part of that picture that a surface like mirror would reflect towards the viewing direction.<br>
For our implementation, the sky and terrain is rendered six time in all directions into a cube map. A blurrier mip is also built from it for less shiny surfaces. For optimization purposes, the environment map is not updated every frame, but 20 times per second. The view direction is mirrored and then used to look up the cube map, which gives the colour of the reflected sky and terrain. Figure 7 depicts the environment mapping enabled and disabled scenarios in our application.<br> 
The terrain part of the environment map is generated based on a simplified version of the terrain (`m_reflectionMesh`), with fewer cells and very basic shading (`m_reflectionShader`). Using a simplified mesh, however, created a pixelated reflection. To make it look better, we used PCF with a 3x3 filter kernel (see Figure 8). Finally, alpha blending is used to blend the reflection with what's visible behind the window. Window opacity is configurable in the GUI. 

<p align="left">
  <img src="Figures/env_map_off.png" width="48%">
  <img src="Figures/env_map_on.png" width="48%">
</p>

*Figure 7: Environment Mapping Disabled (left) and Environment Mapping Enabled (right)*


<p align="left">
  <img src="Figures/pcf_off.png" width="48%">
  <img src="Figures/pcf_on.png" width="48%">
</p>

*Figure 8: PCF disabled (left) and PCF enabled (right) at maximum window opacity*

**Note:** The initial version of the environment mapping was applied to the whole cabin. Henrik later restricted it to the windows, reduced the capture rate, and added a lighter terrain draw for the capture.

### Bezier Curves

The project contains a class called `BezierCurve`, which represents a cubic Bezier curve. For simplicity, since the curves used here are not large, the curve always contains 100 segments. The class contains a function `evaluate`, which evaluates the curve at a specified t in (0,1), using the standard formula. The `draw` function draws the curve using `GL_LINE_STRIP`.<br> 
There is also `CompositeBezierCurve`, which contains an array of cubic Bezier curves, which are assumed to be connecting and in order. The composite curve evaluation is done using a t value in (0, `m_curves.size()`)). The integer part of t refers to the index of the curve and the fractional part is used to evaluate the position for that curve. The exception is the last point of the last curve.<br> 
In the project, we are moving a camera along a curve at constant speed (see Figure 9). The curve we use is a closed loop, so we calculate the position using `fmod`. The curve of the camera is defined using 3 points and shared 3 handles. The 3 points are the joins between curves and the handles are used to calculate the points on either side of each join. Using this method, the tangent doesn't change when moving across the join. To keep the speed constant, we use reparametrization, which was talked about in the lecture. We construct an arc length table with 200 samples per curve. For a given distance, we find the tightest bounds in the sample table and linearly interpolate the corresponding t value between them. The rotation of the camera is not constant, since it is configured to always look at the cable car.  

<p align="left">
  <img src="Figures/bezier_curve.png" width="48%">
  <img src="Figures/bezier_curve_2.png" width="48%">
</p>

*Figure 9: Camera moving along the Bezier curve*

### Hierarchical transformations
The cable car has 3 moving parts: the doors, the door windows and the pulley wheels. The model did not initially have a proper component hierarchy. The components were coupled together based on material, not physical hierarchy. So I separated the doors, windows and pulley wheels all into separate components in Blender. <br>
The transformations are implemented using the `m_nodeWorldOffsets` table. The table keeps track of offsets for child components. This is used by the recursive `drawNode` function, which looks for offsets for the current node, applies them, and then draws each child node. Due to the recursive nature of `drawNode`, the transforms are performed in the correct order.<br> 
The rotation of the pulley wheels is implemented similarly, however, the rotations need to be performed relative to the childs local origin. In addition to the transformation, `m_nodeRotations` contains the center parameter, which is the node's local origin. 
Setting the node rotation for a node includes finding the node's local origin, and calculating the transformation, which is translating it to the global origin, using the local origin, performing the rotation, and translating it back. `drawNode` uses this analogously.<br>
The pulley wheels spin at the speed specified in the GUI, the doors can be opened using a slider, and the windows can be toggled into a semi-open position (see Figures 10, 11). 

<p align="left">
  <img src="Figures/open_doors.png" width="48%">
</p>

*Figure 10: Doors fully open*

<p align="left">
  <img src="Figures/open_doors_windows.png" width="48%">
</p>

*Figure 11: Doors and windows semi open*

#### 
### Extra Features

### ImGui Controls
| Input / UI element | Action |
|--------------------|--------|
| [key or slider] | [what it does] |

### Environment and 3D Model Setup

#### Cable Car:
The Cable Car model [[1](https://sketchfab.com/3d-models/cable-car-1621ec805f3b460faf50cd61f7bcb98f)] is a downloadable 3D model imported into the OpenGL project. It is integrated into the environment as the main object of the scene. It provides a cabin with supporting columns and pulley system for the cable. The model is slightly modified to fit the context. Blender was used to adjust the support column and pulley system, so that horizontal cable can be fit with the model. 

<p align="left">
  <img src="Figures/InitialPosPulley.png" width="48%">
  <img src="Figures/AdjustedPulley.png" width="48%">
</p>

*Figure 8: Original Cable Car Model (left) and Adjusted Model (right) *

#### Mountain
**Current Mountain model**

The **Initial Mountain model** [[2](https://sketchfab.com/3d-models/mountain-range-01-6dff907d427f4c9f8d301075f8019c82)] was also a downloadable 3d model. This model was selected for its detailed geometry and realistic appearance, as well as its support for normal mapping. However, this model didn't provide support for looping, which was important for creating an extended mountain landscape.

<p align="left">
  <img src="Figures/InitialMountain.png" width="48%">
</p>
*Figure 9: Initial Mountain Model Used* 

### Snowfall
In the project, we also have a snowfall particle effect (see Figures 10, 11). The particle effect is created within a box around the cable car. Particles instantiated in random locations around the box with random sizes. A particle is essentially a `vec4`, which contains its position in the x, y and z parameters, and size in the w parameter. They are drawn to on the screen using `GL_POINTS`.<br>
The particles horizontal displacement is dependent on wind speed and vertical displacement on the fall speed. When a particle goes outside the box, it is reset to the top. The particles are shaded as circles with the outline fading into transparency. `snow.vert` defines the `edgeFade` parameter based on the position of the particle. If the particle is nearing an edge, it starts fading. This makes it so the snowflakes smoothly appear and disappear at the edges. 


<p align="left">
  <img src="Figures/light_snowfall.png" width="48%">
</p>

*Figure 10: Light snow*

<p align="left">
  <img src="Figures/heavy_snow.png" width="48%">
</p>

*Figure 11: Heavy snow*

### Fog 
To conceal the terrain resets and limited background geometry, we implemented a fog post processing effect. First the scene is rendered, then distant pixels are faded, creating a foggy effect. <br>
`Fog::begin` creates a frame buffer that stores the scene's color and depth. Then `fog.frag` reads the pixel's depth, and uses the inverse projection matrix to find its distance from the camera. Then we calculate the fog amount based on the distance from the camera, and blend the scene color together with the fog color. For the fog color, we use the sky's color. Different levels of fog are shown in Figures 12-14.


<p align="left">
  <img src="Figures/no_fog.png" width="48%">
</p>

*Figure 12: No fog*

<p align="left">
  <img src="Figures/light_fog.png" width="48%">
</p>

*Figure 13: Light fog*

<p align="left">
  <img src="Figures/dense_fog.png" width="48%">
</p>

*Figure 14: Dense fog*

## References
[1] https://sketchfab.com/3d-models/cable-car-1621ec805f3b460faf50cd61f7bcb98f
[2] https://sketchfab.com/3d-models/mountain-range-01-6dff907d427f4c9f8d301075f8019c82
[3] https://stackoverflow.com/questions/4770735/shader-sun-position-glsl - idea taken for moving sun
[4] https://community.khronos.org/t/day-night-cycle-light/57752- inspiration taken for sunlight affecting sky color
[5] https://github.com/ocornut/imgui/issues/4213- discussions referred for imgui controls
[6] https://learnopengl.com/PBR/Theory- formula for pbr shaders (also in lecture slides)
[7] https://learnopengl.com/Advanced-OpenGL/Cubemaps- to understand cubemaps for environment mapping
[8] https://medium.com/@arda.yildirim158/environment-mapping-ceng469-hw2-eaeb3745f137- for environment mapping, used a discussion from this
[9] https://polyhaven.com/a/snow_02 - this is public domain, no credit necessary
[10] https://www.versluis.com/2023/05/how-to-use-macro-variations-for-unreal-engine-landscape-materials/
