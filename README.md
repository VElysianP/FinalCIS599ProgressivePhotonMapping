# FinalCIS599ProgressivePhotonMapping


## 1. Rendering Results

#### 1. General View 

#### 2. Hitpoint Radius Shrinking 

## 2. Revision Based on Last Version 

[last version](https://github.com/VElysianP/ProgressivePhotonMappingCIS599)

### 1) No Kd-tree for Acceleration 

When implementing photon mapping, the hitpoints are looking for thousands and millions of photons, and therefore acceleration structure is very important. However, for progressive photon mapping, photons are searching for hitPoints whereas the number of hitPoints are decided by the number of pixels. Therefore, it is possible that it is more expensive to construct a kd-tree and do searching than simply searching in a for-loop. Additionally, it is highly possible that the photon will influence more than one hitPoints, yet kd-tree can only find one "neighboring" hitPoint result.  Therefore, there is no acceleration structure inside this project, and hitPoints searching is only implemented though for-loop.

### 2) Special Hitpoints Processing 

#### A. Specular Surfaces 

For all the specuar surfaces, it is impossible for them to gather photons and they can only reflect or transmit the light rays to display the colors of other non-specular surfaces. Therefore, they should be black when the scene is rendered with direct lighting integrator. 

#### B. No-intersection Hitpoints

There will be several cases that there are actually not hitPoints for some specific pixels. First, the ray from camera does not hit anything. Second, the ray goes into a specular material and never goes out. For these cases, there will not be any hitPoints for those pixels, and which means that this pixel will be black. 

### 3) Anti-Aliasing 

To have smoother edges, anti-aliasing is implemented. 

## 3. Limitations and Future Implementation 

The major limitation of this project is rendering right result of transmissive materials like glass ball or some clear plastic materials. The main reason lies in the first path of ray tracing. 

Because when implementing ray tracing, only one ray will be shot from the camera. Therefore, when the intersected material has more than one bxdf (especially Fresnel material), the materials have to be randomly chosen. However, the sampler in the code cannot produce huge amount of random samples especially in a single function. Therefore, when implementing ray tracing, almost all the samplers will choose specular brdf rather than btdf, and this results in the face that glass balls are turned into mirror ball, which is apparently wrong. 

To get the right effect of glass balls, all the rays are forced to go through the glass ball rather than being reflected. It can be called "Biased Progressive Photon Mapping". But this is also not correct because there will not be any reflection from the light source on those Fresnel material, there is a comparison between so-called "Biased Progressive Photon Mapping" and full-lighting path tracing integrator. 


Therefore, a unbiased sampling method is essential for choosing bxdfs for those materials with several bxdfs. 