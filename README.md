---


---

<h1 id="finalcis599progressivephotonmapping">FinalCIS599ProgressivePhotonMapping</h1>
<h2 id="algorithm">1. Algorithm</h2>
<h3 id="data-structure">1) Data Structure</h3>
<p>Based on the data structure in the paper <em>Progressive Photon Mapping</em> , some revisions are made to store some temporary data for hitPoint radius shrink and also separation of direct color and indirect color.</p>
<p>The original data structure in the paper <em>Progressive Photon Mapping</em> is:</p>
<pre><code>struct hitpoint {
    position x    //Hit location
    normal n    //Normal at x
    vector v      //Ray direction
    integer BRDF     //BRDF index
    float x,y      //Pixel location
    color wgt     //Pixel weight
    float R     //Current photon radius
    integer N     //Accumulated photon count
    color c     //Accumulated reflected flux
}
</code></pre>
<p>However, according to <em>Physically Based Rendering Technique</em>, it would be better to separate direct lighting and indirect lighting. Therefore, there are two Color3f components in this data structure. Also, the paper mentioned radius shrinking and photon elimination for each hitPoints. Therefore, extra data structure are essential to store the original photon amount and color and also new photon amount as well as color in this data structure. And the following data structure represent the data structure in this project:</p>
<pre><code>struct PixelHitPoint{
//intersection in the scene contains all of the information
    Intersection isec;
//the coming firection of ray
    Ray ray; 
//the corresponding pixel of the hitpoint
    Point2i pixel; 
//the hitpoint position in world coordinate in the scene
//if there is no hitpoint in the scene, the position would be Point3f(-INFINITY)
    Point3f position; 
//the number of photons
    int numPhotons = 0; 
//the number of new Photons
    int numNewPhotons = 0; 
//the direct lighting color 
    Color3f color;
//the tempColor storage for flux correction
    Color3f newColor = Color3f(0.0f);
//the accumulated indirect color for that pixel
    Color3f indirectColor = Color3f(0.0f);
// the radius that we will check for the final color result
    float radius = 0.5f;
//for shrinking the radius for better results
//the initial value would be rewritten after the first trace of photons
    float density = 1.0f; 
}
</code></pre>
<h3 id="integratorrender-and-progressiveintegrator">2) Integrator::Render() and ProgressiveIntegrator()</h3>
<h4 id="general-algorithm-workflow">General Algorithm Workflow</h4>
<p><img src="https://lh3.googleusercontent.com/fo9gyXNbCQJP9XGfaaUxcPAHikoDXyneVIMJRwY2K-u7ins8b2Mkc7tvODT5GwHt74G-eNDPpRuJ" alt="enter image description here"></p>
<h4 id="implementation-details">Implementation Details</h4>
<p>A. Transmissive Material<br>
For transmissive material especially Fresnel material with more than one BxDFs, because BSDF will not always choose BTDF that go inside the geometry, all the rays are forced to penetrate this transmissive material.</p>
<p>Also, if the ray cannot go out from the transmissive material after specific number of traces defined, this hitPoint will have no intersection point with the scene, and therefore the intersection point inside the scene would be Point3f(-INFINITY), and t in Intersection class would be -1.</p>
<p>B. Pure Specular Material<br>
For pure specular material, if the ray hit pure specular material, the ray would be bounced based on the material property, and the while loop will end if the ray hits non-specular material, light source or it cannot hit anything as well as it cannot go out from the geometry as stated in the last part.</p>
<p>However, if the ray goes through the specular material and finally hits a matte surface, the color of the specular should be recorded in direct lighting color to correctly display the color of the specular material.</p>
<p>C. Light Source<br>
If the hitPoint is on the light, the direct color of the hitPoint would be the color of the light source, and no photon will accumulate in this hitPoint, which means that indirect lighting color would be 0.</p>
<p>D. Direct Lighting Integrator<br>
The first trace are calculated through direct lighting integrator rather then photon accumulation as suggested by Physically Based Rendering Technique to boost the computation efficiency. However it will cause problems like BxDF selection and sharp edges.</p>
<p>E. Photon Accumulation<br>
There is no photon data structure inside this project, photons are shot through several for-loops and the intersection point and color would be recorded in hitPoint structure. When calculating LTE, the final color do not have to divide the pdf calculated by BSDF::Sample_f because the indirect color will be averaged by the total amount of the photons or new photons according to the paper, and therefore, the photons are actually not sampled, no dividing by pdf is needed.</p>
<p>Additionally, no photon can be accumulated on specular material, only reflection or refraction color can be displayed on those surfaces.</p>
<p>F. HitPoints Radius Shrinking<br>
The radius of the hitPoints should change based on the calculate method provided by the paper. An accumulation ratio 0 &lt; alpha &lt; 1 should be decided to determine how many new photons does a hitPoint would accept after each photon trace.</p>
<h2 id="rendering-results">2. Rendering Results</h2>
<h3 id="general-view">1). General View</h3>
<h4 id="cornell-box">Cornell Box</h4>
<p>Initial step: direct lighting</p>
<p><img src="https://lh3.googleusercontent.com/-i5Buppn2lJeLGwu5hnt5Habhln4ZCkG9kxDnLahYWRWA9RfJOTlykQctpsIP8vnmbjNBxpN8x0t" alt="enter image description here"></p>
<p>500,000 photons with 5 traces</p>
<p><img src="https://lh3.googleusercontent.com/Chtr-ZJEZ6FgTRE30hUn5M3qp6qlXgik41TaZUBIPjOeC00KCaQcAyB77Td6psMvpN7a-UA42wjX" alt="enter image description here"></p>
<p>2,000,000 photons with 20 traces</p>
<p><img src="https://lh3.googleusercontent.com/Mo5zB90xkJOfeR4G0e88rqnng8YywuEWrrVkWNsAtdZRsj0wH6izm3H1lnB0P5whIB4YIK3R0Die" alt="enter image description here"></p>
<h4 id="cornell-box-with-two-light-sources">Cornell Box with two light sources</h4>
<p>Initial step: direct lighting</p>
<p><img src="https://lh3.googleusercontent.com/EDHEsowIbAXFqlEJzMPw-NDrZUvb46v1IEkEm5GGs90xhBJf3R1D-4Kzp0X8AFEe2Z9z_-hjfle0" alt="enter image description here"></p>
<p>500,000 photons with 5 traces</p>
<p><img src="https://lh3.googleusercontent.com/pK11DKTyWR0JkmKJKLfM9-r9oS32DVe9ke1-fw2z3UjuVZpzjqizT8UqvEEegbbPIN8awavWV1O4" alt="enter image description here"></p>
<p>2000000 photons with 10 traces</p>
<p><img src="https://lh3.googleusercontent.com/YtSYC1PChk_K1v87gM5AQqUNJ1aIbBkwKjTTH6ggRAvqr-OzINu1ujTGbfz9Kht3_ymU45t-yai1" alt="enter image description here"></p>
<h3 id="hitpoint-radius-shrinking">2). Hitpoint Radius Shrinking</h3>
<p>Comparison: Fulling Lighting 900 samples per pixel</p>
<p><img src="https://lh3.googleusercontent.com/-vPlfGKOR5tG-D2HljEcfA7BVNM93hX6OghjYNfmz-0ZBk94rGIpOwwIHg6l9uwF0K7ChWvHprTm" alt="enter image description here"></p>
<p>Initial state: Direct Lighting</p>
<p><img src="https://lh3.googleusercontent.com/tWNv9LIyHZ2DKgmg5g5dFRcd8ouVjRpawuPdShkTv-0FloPdA-VI9TCtayj9pQvHJKs1frmP9Rbq" alt="enter image description here"></p>
<p>10,000 photons with 1 traces</p>
<p><img src="https://lh3.googleusercontent.com/BKLDz-EgEthUMinza90beu5sn8V_Maix6T09qblbbIaRggjGexgaiA1JRp6vAg3eAmgjWX5RfPMT" alt="enter image description here"></p>
<p>30,000 photons with 3 traces</p>
<p><img src="https://lh3.googleusercontent.com/yDiA-jPZ_rYTCbHHoNEFL0mWZcs3O_TLRhz0T213td-HC5hPthJxAT-R00QyyoxgDKQu2KOO7kPt" alt="enter image description here"></p>
<p>100,000 photons with 10 traces</p>
<p><img src="https://lh3.googleusercontent.com/KBWcLjz8kh6nfZVIKOVgHcBvPtKMZ6GwM4NPFOvVzxiGgo3z5VwqVxREZJj2N-gi7Sd9w7Yg8F8z" alt="enter image description here"></p>
<p>500,000 photons with 5 traces</p>
<p><img src="https://lh3.googleusercontent.com/0rBO2NsrbkCAnBRgrleRBsV1iEmIaysB32A712QOHd-6AdND4LNKhJyHER2KRiE_K_t05yXH_N7_" alt="enter image description here"></p>
<p>2,000,000 photons with 20 traces</p>
<p><img src="https://lh3.googleusercontent.com/SaLVTXoSXnOUAsnHIqY3qr_GGNR0P5tbh5Ti41L460R5qSmuhfjCVxTvUz73Y4HqYO_Qn4wwO_GZ" alt="enter image description here"></p>
<h3 id="photon-view">3). Photon View</h3>
<h3 id="realistic-camera">4). Realistic Camera</h3>
<h2 id="revision-based-on-last-version">3. Revision Based on Last Version</h2>
<p><a href="https://github.com/VElysianP/ProgressivePhotonMappingCIS599">last version</a></p>
<h3 id="no-kd-tree-for-acceleration">1) No Kd-tree for Acceleration</h3>
<p>When implementing photon mapping, the hitpoints are looking for thousands and millions of photons, and therefore acceleration structure is very important. However, for progressive photon mapping, photons are searching for hitPoints whereas the number of hitPoints are decided by the number of pixels. Therefore, it is possible that it is more expensive to construct a kd-tree and do searching than simply searching in a for-loop. Additionally, it is highly possible that the photon will influence more than one hitPoints, yet kd-tree can only find one “neighboring” hitPoint result.  Therefore, there is no acceleration structure inside this project, and hitPoints searching is only implemented though for-loop.</p>
<h3 id="special-hitpoints-processing">2) Special Hitpoints Processing</h3>
<h4 id="a.-specular-surfaces">A. Specular Surfaces</h4>
<p>For all the specuar surfaces, it is impossible for them to gather photons and they can only reflect or transmit the light rays to display the colors of other non-specular surfaces. Therefore, they should be black when the scene is rendered with direct lighting integrator.</p>
<h4 id="b.-no-intersection-hitpoints">B. No-intersection Hitpoints</h4>
<p>There will be several cases that there are actually not hitPoints for some specific pixels. First, the ray from camera does not hit anything. Second, the ray goes into a specular material and never goes out. For these cases, there will not be any hitPoints for those pixels, and which means that this pixel will be black.</p>
<h3 id="anti-aliasing">3) Anti-Aliasing</h3>
<p>To have smoother edges, anti-aliasing is implemented.</p>
<h2 id="limitations-and-future-implementation">4. Limitations and Future Implementation</h2>
<h3 id="limitation">1) Limitation</h3>
<p>The major limitation of this project is rendering right result of transmissive materials like glass ball or some clear plastic materials. The main reason lies in the first path of ray tracing.</p>
<p>Because when implementing ray tracing, only one ray will be shot from the camera. Therefore, when the intersected material has more than one bxdf (especially Fresnel material), the materials have to be randomly chosen. However, the sampler in the code cannot produce huge amount of random samples especially in a single function. Therefore, when implementing ray tracing, almost all the samplers will choose specular brdf rather than btdf, and this results in the face that glass balls are turned into mirror ball, which is apparently wrong.</p>
<p>To get the right effect of glass balls, all the rays are forced to go through the glass ball rather than being reflected. It can be called “Biased Progressive Photon Mapping”. But this is also not correct because there will not be any reflection from the light source on those Fresnel material, there is a comparison between so-called “Biased Progressive Photon Mapping” and full-lighting path tracing integrator.</p>
<p>Full-lighting-integrator with 900 samples per pixel with 5 traces</p>
<p><img src="https://lh3.googleusercontent.com/-FwWKK7vmZXo/WnIjc3XuIgI/AAAAAAAABhg/lAF85vzPPtYIEW2asR7hflu2VpQgR_jeQCLcBGAs/s0/rendered_images_11.png" alt="enter image description here" title="rendered_images_11.png"></p>
<p>“Biased” Progressive Photon Mapping with 500,000 photons with 5 traces</p>
<p><img src="https://lh3.googleusercontent.com/-Ald0-Dtt4MA/WnIVUhqhDJI/AAAAAAAABhA/u60ZR7vLQe8oankxduTSINpGk4RaZVR7wCLcBGAs/s0/rendered_images62.png" alt="enter image description here" title="rendered_images62.png"></p>
<p>Therefore, a unbiased sampling method is essential for choosing bxdfs for those materials with several bxdfs.</p>
<h3 id="comparison-between-path-tracing-and-progressive-photon-mapping">2) Comparison Between Path Tracing and Progressive Photon Mapping</h3>
<h4 id="advantage">Advantage</h4>
<p>Better global illumination effects</p>
<p>Smooth rendered image (But with sharp edges)</p>
<p>Less noise</p>
<h4 id="disadvantage">Disadvantage</h4>
<p>Biased Bxdf selection</p>
<p>Parameter selection has great influence on the result</p>
<p>Time consuming</p>

