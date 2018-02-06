#include "integrator.h"

void Integrator::run()
{
    Render();
}


//for path tracing
//void Integrator::Render()
//{
//    // Compute the bounds of our sample, clamping to screen's max bounds if necessary
//    // Instantiate a FilmTile to store this thread's pixel colors
//    std::vector<Point2i> tilePixels = bounds.GetPoints();
//    // For every pixel in the FilmTile:
//    for(Point2i pixel : tilePixels)
//    {
//        //Uncomment this to debug a particular pixel within this tile
////        if(pixel.x != 208 || pixel.y != 247)
////        {
////            continue;
////        }
//        Color3f pixelColor(0.f);
//        // Ask our sampler for a collection of stratified samples, then raycast through each sample
//        std::vector<Point2f> pixelSamples = sampler->GenerateStratifiedSamples();
//        for(Point2f sample : pixelSamples)
//        {
//            sample = sample + Point2f(pixel); // _sample_ is [0, 1), but it needs to be translated to the pixel's origin.
//            // Generate a ray from this pixel sample
//            Ray ray = camera->Raycast(sample);
//            // Get the L (energy) for the ray by calling Li(ray, scene, tileSampler, arena)
//            // Li is implemented by Integrator subclasses, like DirectLightingIntegrator
//            Color3f L = Li(ray, *scene, sampler, recursionLimit,Color3f(1.0f));
//            // Accumulate color in the pixel
//            pixelColor += L;
//        }
//        // Average all samples' energies
//        pixelColor /= pixelSamples.size();
//        film->SetPixelColor(pixel, glm::clamp(pixelColor, 0.f, 1.f));
//    }
//    //We're done here! All pixels have been given an averaged color.
//}


//for progressive photon mapping
void Integrator::Render()
{
    QList<PixelHitPoint> progHitPoint;//the size is equal to the total size of the pixels
    ProgressiveKdNode* rootProg = nullptr;

    std::vector<Point2i> tilePixels = bounds.GetPoints();


    for(Point2i pix :tilePixels)
    {
        //used for anti-aliasing
        std::vector<Point2f> pixelSamples = sampler->GenerateStratifiedSamples();
        Point2f sample = pixelSamples[floor(sampler->Get1D() * pixelSamples.size())];
        sample = sample * Point2f(0.2f) + Point2f(pix);
        Ray ray = camera->Raycast(sample);
        ProgressiveRayTracing(ray, *scene, pix, sampler,recursionLimit, progHitPoint);
    }

    for(int trace = 0;trace<traceTimes;trace++)
    {
        int photonsToTrace = ceil(totalNumPhoton / traceTimes);
        TraceProgressivePhotons(*scene, rootProg, sampler, recursionLimit, photonsToTrace, progHitPoint);

        for(int index = 0;index <tilePixels.size();index++)
        {
            Color3f pixelColor = progHitPoint[index].color + progHitPoint[index].indirectColor;
            Point2i pixelNum = progHitPoint[index].pixel;

            film->SetPixelColor(pixelNum, glm::clamp(pixelColor, 0.f, 1.f));
        }
    }
}


void Integrator::ClampBounds()
{
    Point2i max = bounds.Max();
    max = Point2i(std::min(max.x, film->bounds.Max().x), std::min(max.y, film->bounds.Max().y));
    bounds = Bounds2i(bounds.Min(), max);
}


void Integrator::ProgressiveRayTracing(Ray cameraRay, const Scene& scene, const Point2i pixel, std::shared_ptr<Sampler> sampler, const int depth, QList<PixelHitPoint> &progHitPoint)
{
//    if((pixel.x==208)&&(pixel.y==247))
//    {
//        Point3f tempPoint;
//    }
    Intersection isec = Intersection();
    //cameraRay = scene.camera.Raycast((float)pixel.x,(float)pixel.y);
    Ray currentRay = cameraRay;
    int dep = 0;
    Color3f finalColor = Color3f(1.0f);

    //shoot ray into the scene
    while(dep < depth)
    {
        //the ray hits something
        if(scene.Intersect(currentRay,&isec))
        {
            //hits the light sources
            if(!isec.ProduceBSDF())
            {
                PixelHitPoint tempHitPoint;
                tempHitPoint.isec = isec;
                tempHitPoint.ray = currentRay;
                tempHitPoint.color = finalColor * isec.Le(-currentRay.direction);
                tempHitPoint.pixel = pixel;
                tempHitPoint.position = isec.point;
                progHitPoint.push_back(tempHitPoint);
                return;
            }
            //the hit point is not the light source
            else
            {
                Vector3f woW = -currentRay.direction;
                Vector3f wiW;
                Color3f leColor = isec.Le(woW);
                BxDFType typeBxdf;
                float specularPdf;
                Color3f fColor = isec.bsdf->Sample_f(woW,&wiW,sampler->Get2D(),&specularPdf, BSDF_ALL ,&typeBxdf);

                //specular bounce
                if((typeBxdf & BSDF_SPECULAR) != 0)
                {
                    //if the ray cannot go out of the specular object during the traces
                    //just give the color its own Le color
                    if(dep == depth)
                    {
                        PixelHitPoint tempHitPoint;
                        tempHitPoint.pixel = pixel;
                        //tempHitPoint.ray = currentRay;
                        tempHitPoint.color = finalColor * isec.Le(woW);
                        tempHitPoint.isec = Intersection();
                        tempHitPoint.position = Point3f((float)-INFINITY);
                        progHitPoint.push_back(tempHitPoint);
                        return;
                    }
                    //force transmissive material to go throgh the material rather than
                    //be reflected
                    if(isec.objectHit->GetMaterial()->JudgeTransmissiveMaterial())
                    {
                        Vector3f wo = isec.bsdf->worldToTangent * woW;
                        bool entering = CosTheta(wo) > 0;
                        float eta = entering? 1.0 / isec.bsdf->eta : isec.bsdf->eta;
                        Vector3f wi;
                        if(Refract(wo,Faceforward(Normal3f(0.0,0.0,1.0),wo),eta,&wi))
                        {
                            dep++;
                            finalColor = leColor + finalColor * fColor * AbsDot(wiW,isec.bsdf->normal) / specularPdf;
                            currentRay = isec.SpawnRay(isec.bsdf->tangentToWorld * wi);
                        }
                        else
                        {
                            dep++;
                            finalColor = leColor + finalColor * fColor * AbsDot(wiW,isec.bsdf->normal) / specularPdf;
                            currentRay = isec.SpawnRay(wiW);
                        }
                    }
                    else
                    {
                        dep++;
                        finalColor = leColor + finalColor * fColor * AbsDot(wiW,isec.bsdf->normal) / specularPdf;
                        currentRay = isec.SpawnRay(wiW);
                    }

                    continue;
                }
                //nonspecular bounce
                else
                {
                    Color3f sumColor = Color3f(0.f);
                    std::vector<Point2f> pixelSamples = sampler->GenerateStratifiedSamples();
                    for(Point2f pixSam : pixelSamples)
                    {
                        int  lightNum = std::floor(sampler->Get1D() * scene.lights.size());
                        float lightPdf;
                        Color3f chosenLightLiColor = scene.lights[lightNum]->Sample_Li(isec,sampler->Get2D(),&wiW,&lightPdf);
                        Color3f directFColor = isec.bsdf->f(woW,wiW);

                        Ray lightShadowRay = isec.SpawnRay(wiW);
                        Intersection shadowIsec = Intersection();

                        if(scene.Intersect(lightShadowRay,&shadowIsec))
                        {
                            //which means that the shadow ray cannot hit a light source
                            //or the specific light source we want
                            //if should be inside the shadow
                            if((shadowIsec.objectHit->GetLight()==nullptr)||(shadowIsec.objectHit->light != scene.lights[lightNum]))
                            {
                                sumColor += leColor;
                            }
                            else
                            {
                                if(lightPdf == 0)
                                {
                                    sumColor += leColor;
                                }
                                else
                                {
                                    lightPdf = lightPdf / scene.lights.size();
                                    sumColor += (leColor + directFColor * chosenLightLiColor * AbsDot(wiW,isec.normalGeometric)/lightPdf);
                                }
                            }

                        }
                        else
                        {
                            sumColor += leColor;
                        }
                    }
                    sumColor = sumColor / Color3f((float)pixelSamples.size());
                    PixelHitPoint tempHitPoint;
                    tempHitPoint.pixel = pixel;
                    tempHitPoint.ray = currentRay;
                    tempHitPoint.color = finalColor * sumColor;
                    tempHitPoint.isec = isec;
                    tempHitPoint.position = isec.point;
                    progHitPoint.push_back(tempHitPoint);
                    return;
                }
            }
        }
        //the ray does not hit anything
        else
        {
            PixelHitPoint tempHitPoint;
            tempHitPoint.isec = Intersection();
            //tempHitPoint.ray = currentRay;
            tempHitPoint.color = Color3f(0.0f);
            tempHitPoint.pixel = pixel;
            tempHitPoint.position = Point3f((float)-INFINITY);
            progHitPoint.push_back(tempHitPoint);

//            if(progHitPoint[progHitPoint.size()-1].isec.t == -1.0){
//                std::cout << "stop" << std::endl;
//            }
            return;
        }
    }
}

void Integrator::ProgressiveKdTree(ProgressiveKdNode* root, PixelHitPoint hitPoint, int linearIndex)
{
    root = root->InsertProgressiveKdTree(root,hitPoint,linearIndex);
}

//implemented inside ProgressivePhotonMapping
//not necessary to do anything here
void Integrator::TraceProgressivePhotons(const Scene& scene, ProgressiveKdNode* root,std::shared_ptr<Sampler> sampler, int depth, int numPhotons, QList<PixelHitPoint>& hitPoints)
{
    return;
}
