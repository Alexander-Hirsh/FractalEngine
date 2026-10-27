#version 430 core

out vec4 fragColor;
in vec2 texCoords;

uniform sampler2D screenTex;
uniform sampler2D bloomTex;

uniform bool useKuwahara;
uniform bool useBloom;
uniform float bloomStrength;

const int ANISOTROPIC_RADIUS = 2;
const int ANISOTROPIC_SECTORS = 8;
const float PI = 3.14159265359;

float luminanceAt(ivec2 pixel, ivec2 imageSize)
{
        pixel = clamp(pixel, ivec2(0), imageSize - ivec2(1));
        return dot(texelFetch(screenTex, pixel, 0).rgb, vec3(0.299, 0.587, 0.114));
}

vec2 imageGradient(ivec2 pixel, ivec2 imageSize)
{
        return vec2(
                luminanceAt(pixel + ivec2(1, 0), imageSize)
              - luminanceAt(pixel - ivec2(1, 0), imageSize),
                luminanceAt(pixel + ivec2(0, 1), imageSize)
              - luminanceAt(pixel - ivec2(0, 1), imageSize));
}

vec3 anisotropicKuwahara(ivec2 pixel, ivec2 imageSize)
{
        vec2 gradient = imageGradient(pixel, imageSize);
        vec2 gradientLeft = imageGradient(pixel + ivec2(-1, 0), imageSize);
        vec2 gradientRight = imageGradient(pixel + ivec2(1, 0), imageSize);
        vec2 gradientDown = imageGradient(pixel + ivec2(0, -1), imageSize);
        vec2 gradientUp = imageGradient(pixel + ivec2(0, 1), imageSize);

        float tensorXX = gradient.x * gradient.x;
        float tensorYY = gradient.y * gradient.y;
        float tensorXY = gradient.x * gradient.y;
        tensorXX += gradientLeft.x * gradientLeft.x + gradientRight.x * gradientRight.x
                  + gradientDown.x * gradientDown.x + gradientUp.x * gradientUp.x;
        tensorYY += gradientLeft.y * gradientLeft.y + gradientRight.y * gradientRight.y
                  + gradientDown.y * gradientDown.y + gradientUp.y * gradientUp.y;
        tensorXY += gradientLeft.x * gradientLeft.y + gradientRight.x * gradientRight.y
                  + gradientDown.x * gradientDown.y + gradientUp.x * gradientUp.y;
        float eigenRoot = sqrt(max((tensorXX - tensorYY) * (tensorXX - tensorYY)
                                 + 4.0 * tensorXY * tensorXY, 0.0));
        float eigenMax = 0.5 * (tensorXX + tensorYY + eigenRoot);
        float eigenMin = 0.5 * (tensorXX + tensorYY - eigenRoot);
        float anisotropy = (eigenMax - eigenMin) / (eigenMax + eigenMin + 1e-5);
        float gradientAngle = tensorXX + tensorYY > 1e-5
                            ? 0.5 * atan(2.0 * tensorXY, tensorXX - tensorYY)
                            : 0.0;
        float tangentAngle = gradientAngle + 0.5 * PI;
        vec2 tangent = vec2(cos(tangentAngle), sin(tangentAngle));
        vec2 normal = vec2(-tangent.y, tangent.x);

        float stretch = 1.0 + 1.5 * anisotropy;
        float majorRadius = float(ANISOTROPIC_RADIUS) * stretch;
        float minorRadius = float(ANISOTROPIC_RADIUS) / stretch;

        vec3 sectorColor[ANISOTROPIC_SECTORS];
        vec2 sectorDirections[ANISOTROPIC_SECTORS];
        float sectorWeight[ANISOTROPIC_SECTORS];
        float sectorLuminance[ANISOTROPIC_SECTORS];
        float sectorLuminanceSquared[ANISOTROPIC_SECTORS];

        for (int sector = 0; sector < ANISOTROPIC_SECTORS; ++sector)
        {
                float sectorAngle = float(sector) * (2.0 * PI / float(ANISOTROPIC_SECTORS));
                sectorDirections[sector] = vec2(cos(sectorAngle), sin(sectorAngle));
                sectorColor[sector] = vec3(0.0);
                sectorWeight[sector] = 0.0;
                sectorLuminance[sector] = 0.0;
                sectorLuminanceSquared[sector] = 0.0;
        }

        for (int y = -ANISOTROPIC_RADIUS; y <= ANISOTROPIC_RADIUS; ++y)
        {
                for (int x = -ANISOTROPIC_RADIUS; x <= ANISOTROPIC_RADIUS; ++x)
                {
                        vec2 local = vec2(x, y) / float(ANISOTROPIC_RADIUS);
                        float radiusSquared = dot(local, local);
                        if (radiusSquared > 1.0)
                                continue;

                        vec2 offset = tangent * (local.x * majorRadius)
                                    + normal * (local.y * minorRadius);
                        ivec2 samplePixel = clamp(pixel + ivec2(round(offset)),
                                                  ivec2(0), imageSize - ivec2(1));
                        vec3 sampleColor = texelFetch(screenTex, samplePixel, 0).rgb;
                        float sampleLuminance = dot(sampleColor, vec3(0.299, 0.587, 0.114));
                        float radialBase = 1.0 - radiusSquared;
                        float radialWeight = radialBase * radialBase;

                        for (int sector = 0; sector < ANISOTROPIC_SECTORS; ++sector)
                        {
                                float angularBase = max(0.0, 0.25 + dot(local, sectorDirections[sector]));
                                float angularSquared = angularBase * angularBase;
                                float angularFourth = angularSquared * angularSquared;
                                float angularWeight = angularFourth * angularFourth;
                                float weight = radialWeight * angularWeight;

                                sectorColor[sector] += sampleColor * weight;
                                sectorWeight[sector] += weight;
                                sectorLuminance[sector] += sampleLuminance * weight;
                                sectorLuminanceSquared[sector] += sampleLuminance * sampleLuminance * weight;
                        }
                }
        }

        vec3 filteredColor = vec3(0.0);
        float totalQuality = 0.0;
        for (int sector = 0; sector < ANISOTROPIC_SECTORS; ++sector)
        {
                float weight = max(sectorWeight[sector], 1e-5);
                vec3 meanColor = sectorColor[sector] / weight;
                float meanLuminance = sectorLuminance[sector] / weight;
                float variance = max(sectorLuminanceSquared[sector] / weight
                                   - meanLuminance * meanLuminance, 0.0);
                float relativeVariance = variance / (meanLuminance * meanLuminance + 0.01);
                float inverseQuality = 1.0 / (1.0 + relativeVariance);
                float qualitySquared = inverseQuality * inverseQuality;
                float quality = qualitySquared * qualitySquared;

                filteredColor += meanColor * quality;
                totalQuality += quality;
        }

        return filteredColor / max(totalQuality, 1e-5);
}

void main()
{
/// ##############################################################################################
///                                     Common parameters
/// ##############################################################################################
        vec2 uv = texCoords;
        vec2 screenSize = vec2(textureSize(screenTex, 0));

        ivec2 texel = clamp(ivec2(uv * screenSize), ivec2(0), ivec2(screenSize) - ivec2(1));
        int kernelSize = 2;

        
/// ##############################################################################################
///                                     Cross blur
/// ##############################################################################################
        // vec4 avg;
        // for(int x = -kernelSize; x <= kernelSize; x++)
        // {
        //         avg += texture(screenTex, (texel + vec2(x, 0)) / screenSize);
        // }

        // for(int y = -kernelSize; y <= kernelSize; y++)
        // {
        //         avg += texture(screenTex, (texel + vec2(0, y)) / screenSize);   
        // }

        // fragColor = avg / (kernelSize * 4);

        //fragColor = texture(screenTex, uv);


/// ##############################################################################################
///                                     Base Kuwahara Filter
/// ##############################################################################################
        // vec4 sum = vec4(0.0);
        // float sumSquared = 0.0;
        // int count = 0;

        // float minVariance = 1e30;
        // vec4 bestColor = vec4(0.0);

        // for(int r = 0; r < 4; r++)
        // {
        //         bool right  = (r & 1) != 0;
        //         bool bottom = (r & 2) != 0;

        //         int x0 = right  ? 0 : -kernelSize;
        //         int x1 = right  ? kernelSize : 0;
        //         int y0 = bottom ? 0 : -kernelSize;
        //         int y1 = bottom ? kernelSize : 0;

        //         for (int y = y0; y <= y1; y++)
        //         {
        //                 for (int x = x0; x <= x1; x++)
        //                 {
        //                         vec4 color = texelFetch(screenTex, texel + ivec2(x, y), 0);

        //                         sum += color;
        //                         sumSquared += dot(color, color);
        //                         count++;
        //                 }
        //         }       
        //         vec4 mean = sum / float(count);

        //         float variance =
        //         sumSquared / float(count) - dot(mean, mean);

        //         variance = max(variance, 0.0);

        //         if (variance < minVariance)
        //         {
        //                 minVariance = variance;
        //                 bestColor = mean;
        //         }
        // }


        const float gamma = 1.1;

        vec3 sceneColor = useKuwahara ? anisotropicKuwahara(texel, ivec2(screenSize)) :
                          texture(screenTex, texCoords).rgb;

        vec3 bloomColor = texture(bloomTex, texCoords).rgb;

        if(useBloom) fragColor = vec4(pow(mix(sceneColor, bloomColor, bloomStrength), vec3(1.0/gamma)), 1.0);
        else fragColor = vec4(sceneColor, 1.0);
}