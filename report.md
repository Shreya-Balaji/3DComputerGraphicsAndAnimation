# Cable Car Through the Alps

[One-line description, e.g. "A real-time OpenGL tech demo of a cable car travelling through a snowy mountain landscape."]

## Team Members

| Name  | Main contributions |
|-------|--------------------|
| [Srinitish Ramesh]|[Material Textures and Lighting, Cable Car Model Adjustments and Import, Cable path for Cable Car, Mountain Import and Normal Mapping (Initial Version)]     |
| [Name]|[contributions]     |
| [Name]|[contributions]     |

## Introduction

what the demo shows, the story or scene, build instructions etc.
## Features

### Minimal features

#### [Feature 1 name] ([team member name])
[What it does and how it works, 1–3 sentences.]
![[Alt text: what the image shows]](screenshots/[feature1].png)
*[Figure 1: Image Caption]*

#### Material Textures and Lighting
Material Textures and Lighting models are implemented in this demo to showcase how the body of Cable Car interacts with light. The diffuse coefficient **(kd)** and specular coefficient **(ks)** are used to showcase the base color and highlights of the model. **Shininess** is also integrated to determine the level of sharpness of these highlights.

To provide shading based on the angle in which light hits the surface, **Lambertian Diffuse Reflection** is used. To produce more realistic reflective surface **Blinn-Phong Specular reflection** is added. Finally, **Ambient Lighting** provides general illumination, ensuring that the surface is visible in areas that lack direct lighting. To test this, toggle **Enable Ambient Lighting** on and off to observe general illumination. Adjust the **Shininess** slider to see how specular highlights changes.

*Note:* These material, texture, and conventional lighting features are implemented to fulfill the assignment's minimal requirements. However, the team has decided to use **Physically Based Rendering (PBR)** with metallic and roughness, to visually enhance the cable car model.

![Cable car model without Material Textures and Lighting](./Figures/WithoutMaterialTex.png)
![Cable car model with Material Textures and Lighting](./Figures/withMaterialTex.png)

*[Figure 1: Conventional Lighting Settings Not Enabled]* *[Figure 2: Conventional Lighting Settings Enabled]*

### Extra Features
#### Minimap
![Cable car model with Material Textures and Lighting](./Figures/Minimap.png)
### ImGui Controls
| Input / UI element | Action |
|--------------------|--------|
| [key or slider] | [what it does] |

### Environment and 3D Model Setup

#### Cable Car:
The Cable Car model [[1](https://sketchfab.com/3d-models/cable-car-1621ec805f3b460faf50cd61f7bcb98f)] is a downloadable 3D model imported into the OpenGL project. It is integrated into the environment as the main object of the scene. It provides a cabin with supporting columns and pulley system for the cable. The model is slightly modified to fit the environment. 

However, the original model required some modifications to fit the requirements of the project. The support column and the pulley system needed to be adjusted so that the model could support the horizontal cable used in the scene. Blender was used to do these changes, where parts were repositioned and adjusted to achieve more suitable configuration.

![Original Cable Car Model](./Figures/InitialPosPulley.png) ![Modified Pulley system for Cable Car](./Figures/AdjustedPulley.png)

*[Figure 3: Original Cable Car Model]* *[Figure 4: Adjusted Model]*

#### Cable:
The cable is procedurally generated alongside with the Mountains using OpenGL rather than a separate imported model. It uses a cylindrical mesh consisting of 128 segments, with each of them having 8 vertices arranged in a ring. These rings are connected using triangles to form a continuous cable with small radius.
To support the looping terrain, three copies are rendered at different z-axis offsets. Their positions follow a terrain offset, which keeps the cable aligned with the looping terrain.

#### Mountain
**Current Mountain model**

The **Initial Mountain model** [[2](https://sketchfab.com/3d-models/mountain-range-01-6dff907d427f4c9f8d301075f8019c82)] was also a downloadable 3d model. This model was selected for its detailed geometry and realistic appearance.
Another important consideration was its support for normal mapping. Normal mapping enhances the surface detail of mountain and makes it appear more detailed without significant increase in geometric complexity.
However, this model didn't provide support for looping, which was important for creating an extended mountain landscape. Using it would have made it difficult to achieve this continuity, leaving gaps in the terrain.

![Initial Mountian](./Figures/InitialMountain.png)

*[Figure 5: Initial Mountain Model Used]* 



## References
[1] https://sketchfab.com/3d-models/cable-car-1621ec805f3b460faf50cd61f7bcb98f

[2] https://sketchfab.com/3d-models/mountain-range-01-6dff907d427f4c9f8d301075f8019c82
